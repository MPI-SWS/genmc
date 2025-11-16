/*
 * GenMC -- Generic Model Checking.
 *
 * This project is dual-licensed under the Apache License 2.0 and the MIT License.
 * You may choose to use, distribute, or modify this software under either license.
 *
 * Apache License 2.0:
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * MIT License:
 *     https://opensource.org/licenses/MIT
 */

#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "Static/ModuleID.hpp"
#include "Support/SExprVisitor.hpp"

SVal EventLabel::getAccessValue(const AAccess &access) const
{
	/* Special case for initializer */
	if (getPos().isInitializer())
		return getParent()->getInitVal(access);

	/* Assumes rf is already set */
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(this)) {
		return rLab->getRf()->getAccessValue(access);
	}
	auto *wLab = genmc::dyn_cast<WriteLabel>(this);
	BUG_ON(!wLab);
	return wLab->getVal();
}

SVal EventLabel::getReturnValue() const
{
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(this)) {
		BUG_ON(!rLab->getRf());
		return getAccessValue(rLab->getAccess());
	}
	if (auto *tsLab = genmc::dyn_cast<ThreadStartLabel>(this)) {
		return tsLab->getThreadInfo().arg;
	}
	if (auto *jLab = genmc::dyn_cast<ThreadJoinLabel>(this)) {
		auto *eLab = genmc::dyn_cast<ThreadFinishLabel>(
			jLab->getParent()->getLastThreadLabel(jLab->getChildId()));
		return eLab->getRetVal();
	}
	if (auto *oLab = genmc::dyn_cast<OptionalLabel>(this)) {
		return SVal(oLab->isExpanded());
	}
	BUG();
}

void InitLabel::addReader(ReadLabel *rLab)
{
	BUG_ON(std::find_if(rf_begin(rLab->getAddr()), rf_end(rLab->getAddr()),
			    [rLab](ReadLabel &oLab) { return oLab.getPos() == rLab->getPos(); }) !=
	       rf_end(rLab->getAddr()));
	initRfs[rLab->getAddr()].push_back(*rLab);
}

void MethodBeginLabel::addPred(MethodEndLabel *predLab)
{
	/* Add to predecessor list */
	auto preds = lin_preds();
	BUG_ON(std::ranges::find_if(preds, [predLab](auto &endLab) {
		       return endLab->getPos() == predLab->getPos();
	       }) != std::ranges::end(preds));
	linPreds_.push_back(predLab);

	/* Add THIS as a successor of PREDLAB */
	predLab->addSuccNoCascade(this);
}

void MethodEndLabel::addSucc(MethodBeginLabel *succLab)
{
	/* Add to successors list */
	auto succs = lin_succs();
	BUG_ON(std::ranges::find_if(succs, [succLab](auto &begLab) {
		       return begLab->getPos() == succLab->getPos();
	       }) != std::ranges::end(succs));
	linSuccs_.push_back(succLab);

	/* Add THIS as a predecessor of SUCCLAB */
	succLab->addPredNoCascade(this);
}

bool WriteLabel::isRMW() const
{
	return CasWriteLabel::classofKind(getKind()) || FaiWriteLabel::classofKind(getKind());
}

bool ReadLabel::isRMW() const
{
	if (!CasReadLabel::classofKind(getKind()) && !FaiReadLabel::classofKind(getKind()))
		return false;

	auto &g = *getParent();
	auto *nLab = genmc::dyn_cast_if_present<WriteLabel>(g.po_imm_succ(this));
	return nLab && nLab->isRMW() && nLab->getAddr() == getAddr();
}

bool WriteLabel::isEffectful() const
{
	auto &g = *getParent();
	auto *xLab = genmc::dyn_cast<FaiWriteLabel>(this);
	auto *rLab = genmc::dyn_cast<FaiReadLabel>(g.po_imm_pred(this));
	if (!xLab || rLab->getOp() != RMWBinOp::Xchg)
		return true;

	return rLab->getAccessValue(rLab->getAccess()) != xLab->getVal();
}

bool WriteLabel::isObservable() const
{
	if (isAtLeastRelease() || !getAddr().isDynamic())
		return true;

	auto &g = *getParent();
	auto wpreds = g.po_preds(this);
	auto mLabIt = std::ranges::find_if(wpreds, [this](auto &lab) {
		if (auto *aLab = genmc::dyn_cast<MallocLabel>(&lab)) {
			if (aLab->contains(this->getAddr()))
				return true;
		}
		return false;
	});
	if (mLabIt == std::ranges::end(wpreds))
		return true;

	for (auto &lab : std::ranges::subrange(std::ranges::begin(wpreds), mLabIt)) {
		if (lab.isAtLeastRelease())
			return true;
		/* The location must not be read (loop counter) */
		if (auto *rLab = genmc::dyn_cast<ReadLabel>(&lab))
			if (rLab->getAddr() == getAddr())
				return true;
	}
	return false;
}

bool ReadLabel::valueMakesRMWSucceed(const SVal &val) const
{
	if (FaiReadLabel::classofKind(getKind()))
		return true;
	if (!CasReadLabel::classofKind(getKind()))
		return false;
	auto *casLab = static_cast<const CasReadLabel *>(this);
	return val == casLab->getExpected();
}

bool ReadLabel::valueMakesAssumeSucceed(const SVal &val) const
{
	using Evaluator = SExprEvaluator<ModuleID::ID>;
	return getAnnot() && Evaluator().evaluate(&*getAnnot()->expr, val);
}

void ReadLabel::setRf(EventLabel *rfLab)
{
	/* Remember old rf before setting */
	auto *oldRfLab = getRf();
	setRfNoCascade(rfLab);

	/*
	 * Delete the read from the readers list of oldRf.
	 * We need to ensure that the old label we were reading from still exists
	 * (not just the position; it might have been replaced). */
	if (oldRfLab) {
		// BUG_ON(!getParent()->containsLab(oldRfLab));
		if (auto *oldLab = genmc::dyn_cast<WriteLabel>(oldRfLab))
			oldLab->removeReader([&](ReadLabel &oLab) { return &oLab == this; });
		else if (auto *oldLab = genmc::dyn_cast<InitLabel>(oldRfLab))
			oldLab->removeReader(getAddr(),
					     [&](ReadLabel &oLab) { return &oLab == this; });
		else
			BUG();
	}

	/* If this read is now reading from bottom, nothing else to do */
	if (!rfLab)
		return;

	/* Otherwise, add it to the write's reader list */
	if (auto *wLab = genmc::dyn_cast<WriteLabel>(rfLab)) {
		wLab->addReader(this);
	} else if (auto *iLab = genmc::dyn_cast<InitLabel>(rfLab)) {
		iLab->addReader(this);
	} else
		BUG();
}

void WriteLabel::addCo(EventLabel *predLab)
{
	auto &g = *getParent();
	auto *predLabW = genmc::dyn_cast<WriteLabel>(predLab);
	g.coherence[getAddr()].insert(
		predLabW ? ++ExecutionGraph::co_iterator(*predLabW) : g.co_begin(getAddr()), *this);
}

void WriteLabel::moveCo(EventLabel *predLab)
{
	auto &g = *getParent();
	g.coherence[getAddr()].remove(*this);
	addCo(predLab);
}

auto BlockLabel::createAssumeBlock(Event pos, AssumeType type) -> std::unique_ptr<BlockLabel>
{
	switch (type) {
	case AssumeType::User:
		return UserBlockLabel::create(pos);
	case AssumeType::Barrier:
		return BarrierBlockLabel::create(pos);
	case AssumeType::Spinloop:
		return SpinloopBlockLabel::create(pos);
	default:
		BUG();
	}
}
