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

#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/Consistency/ConsistencyChecker.hpp"
#include "genmc/Execution/ExecutionGraph.hpp"
#include "genmc/Support/ModuleVarID.hpp"
#include "genmc/Support/SExprVisitor.hpp"

SVal EventLabel::getAccessValue(const AAccess &access) const
{
	const auto &g = *getParent();

	VERIFY(getPos().isInitializer() || genmc::isa<MemAccessLabel>(this));
	const auto *rLab = genmc::dyn_cast<ReadLabel>(this);
	return g.resolveAccessValue(rLab ? rLab->getRf() : this, access);
}

SVal EventLabel::getReturnValue() const
{
	if (auto *rLab = genmc::dyn_cast<ReadLabel>(this)) {
		VERIFY(rLab->getRf());
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
	UNREACHABLE();
}

void InitLabel::addReader(ReadLabel *rLab)
{
	ASSERT(std::find_if(rf_begin(rLab->getAddr()), rf_end(rLab->getAddr()),
			    [rLab](ReadLabel &oLab) { return oLab.getPos() == rLab->getPos(); }) ==
	       rf_end(rLab->getAddr()));
	initRfs[rLab->getAddr()].push_back(*rLab);
}

void MethodBeginLabel::addPred(MethodEndLabel *predLab)
{
	/* Add to predecessor list */
	auto preds = lin_preds();
	ASSERT(std::ranges::find_if(preds, [predLab](auto &endLab) {
		       return endLab->getPos() == predLab->getPos();
	       }) == std::ranges::end(preds));
	linPreds_.push_back(predLab);

	/* Add THIS as a successor of PREDLAB */
	predLab->addSuccNoCascade(this);
}

void MethodEndLabel::addSucc(MethodBeginLabel *succLab)
{
	/* Add to successors list */
	auto succs = lin_succs();
	ASSERT(std::ranges::find_if(succs, [succLab](auto &begLab) {
		       return begLab->getPos() == succLab->getPos();
	       }) == std::ranges::end(succs));
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
	using Evaluator = SExprEvaluator<ModuleVarID>;
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
		// VERIFY(getParent()->containsLab(oldRfLab));
		if (auto *oldLab = genmc::dyn_cast<WriteLabel>(oldRfLab))
			oldLab->removeReader([&](ReadLabel &oLab) { return &oLab == this; });
		else if (auto *oldLab = genmc::dyn_cast<InitLabel>(oldRfLab))
			oldLab->removeReader(getAddr(),
					     [&](ReadLabel &oLab) { return &oLab == this; });
		else
			UNREACHABLE();
	}

	/* If this read is now reading from bottom, nothing else to do */
	if (!rfLab)
		return;

	/* Otherwise, add it to the write's reader list... */
	if (auto *wLab = genmc::dyn_cast<WriteLabel>(rfLab)) {
		wLab->addReader(this);
	} else if (auto *iLab = genmc::dyn_cast<InitLabel>(rfLab)) {
		iLab->addReader(this);
	} else
		UNREACHABLE();

	/* and adjust the max flag */
	this->setAddedMax(getParent()->co_max(this->getAddr()) == rfLab);
}

void WriteLabel::addCo(EventLabel *predLab)
{
	auto &g = *getParent();
	auto *predLabW = genmc::dyn_cast<WriteLabel>(predLab);
	auto &coh = g.coherence[getAddr()];
	coh.insert(predLabW ? ++ExecutionGraph::co_iterator(*predLabW) : coh.begin(), *this);
}

void WriteLabel::moveCo(EventLabel *predLab)
{
	auto &g = *getParent();
	g.coherence[getAddr()].remove(*this);
	addCo(predLab);
	this->setAddedMax(this == &g.coherence[getAddr()].back());
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
		UNREACHABLE();
	}
}
