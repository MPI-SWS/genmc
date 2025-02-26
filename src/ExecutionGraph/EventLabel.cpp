/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/ExecutionGraph.hpp"
#include "ExecutionGraph/LabelVisitor.hpp"
#include "Static/ModuleID.hpp"
#include "Support/SExprVisitor.hpp"

SVal EventLabel::getAccessValue(const AAccess &access) const
{
	/* Special case for initializer */
	if (getPos().isInitializer())
		return getParent()->getInitVal(access);

	/* Assumes rf is already set */
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(this)) {
		return rLab->getRf()->getAccessValue(access);
	}
	auto *wLab = llvm::dyn_cast<WriteLabel>(this);
	BUG_ON(!wLab);
	return wLab->getVal();
}

SVal EventLabel::getReturnValue() const
{
	if (auto *rLab = llvm::dyn_cast<ReadLabel>(this)) {
		BUG_ON(!rLab->getRf());
		return getAccessValue(rLab->getAccess());
	}
	if (auto *tsLab = llvm::dyn_cast<ThreadStartLabel>(this)) {
		return tsLab->getThreadInfo().arg;
	}
	if (auto *jLab = llvm::dyn_cast<ThreadJoinLabel>(this)) {
		auto *eLab = llvm::dyn_cast<ThreadFinishLabel>(
			jLab->getParent()->getLastThreadLabel(jLab->getChildId()));
		return eLab->getRetVal();
	}
	if (auto *oLab = llvm::dyn_cast<OptionalLabel>(this)) {
		return SVal(oLab->isExpanded());
	}
	BUG();
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
	auto *nLab = llvm::dyn_cast_or_null<WriteLabel>(g.po_imm_succ(this));
	return nLab && nLab->isRMW() && nLab->getAddr() == getAddr();
}

bool WriteLabel::isEffectful() const
{
	auto &g = *getParent();
	auto *xLab = llvm::dyn_cast<FaiWriteLabel>(this);
	auto *rLab = llvm::dyn_cast<FaiReadLabel>(g.po_imm_pred(this));
	if (!xLab || rLab->getOp() != llvm::AtomicRMWInst::BinOp::Xchg)
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
		if (auto *aLab = llvm::dyn_cast<MallocLabel>(&lab)) {
			if (aLab->contains(this->getAddr()))
				return true;
		}
		return false;
	});
	if (mLabIt == std::ranges::end(wpreds))
		return true;

	auto *mLab = &*mLabIt;
	for (auto j = mLab->getIndex() + 1; j < getIndex(); j++) {
		auto *lab = g.getEventLabel(Event(getThread(), j));
		if (lab->isAtLeastRelease())
			return true;
		/* The location must not be read (loop counter) */
		if (auto *rLab = llvm::dyn_cast<ReadLabel>(lab))
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
	return getAnnot() && Evaluator().evaluate(getAnnot(), val);
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
	if (oldRfLab && getParent()->containsPos(oldRfLab->getPos())) {
		BUG_ON(!getParent()->containsLab(oldRfLab));
		if (auto *oldLab = llvm::dyn_cast<WriteLabel>(oldRfLab))
			oldLab->removeReader([&](ReadLabel &oLab) { return &oLab == this; });
		else if (auto *oldLab = llvm::dyn_cast<InitLabel>(oldRfLab))
			oldLab->removeReader(getAddr(),
					     [&](ReadLabel &oLab) { return &oLab == this; });
		else
			BUG();
	}

	/* If this read is now reading from bottom, nothing else to do */
	if (!rfLab)
		return;

	/* Otherwise, add it to the write's reader list */
	if (auto *wLab = llvm::dyn_cast<WriteLabel>(rfLab)) {
		wLab->addReader(this);
	} else if (auto *iLab = llvm::dyn_cast<InitLabel>(rfLab)) {
		iLab->addReader(this);
	} else
		BUG();
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const EventLabel::EventLabelKind k)
{
	switch (k) {
	case EventLabel::ThreadStart:
		s << "THREAD_START";
		break;
	case EventLabel::Init:
		s << "INIT";
		break;
	case EventLabel::JoinBlock:
		s << "BLOCK[join]";
		break;
	case EventLabel::SpinloopBlock:
		s << "BLOCK[spinloop]";
		break;
	case EventLabel::FaiZNEBlock:
		s << "BLOCK[Fai-zne]";
		break;
	case EventLabel::LockZNEBlock:
		s << "BLOCK[Lock-zne]";
		break;
	case EventLabel::HelpedCASBlock:
		s << "BLOCK[helped-cas]";
		break;
	case EventLabel::ConfirmationBlock:
		s << "BLOCK[conf]";
		break;
	case EventLabel::LockNotAcqBlock:
		s << "BLOCK[lock-unacq]";
		break;
	case EventLabel::LockNotRelBlock:
		s << "BLOCK[lock-unrel]";
		break;
	case EventLabel::BarrierBlock:
		s << "BLOCK[barrier]";
		break;
	case EventLabel::ErrorBlock:
		s << "BLOCK[error]";
		break;
	case EventLabel::UserBlock:
		s << "BLOCK[user]";
		break;
	case EventLabel::ReadOptBlock:
		s << "BLOCK[read-opt]";
		break;
	case EventLabel::ThreadKill:
		s << "KILL";
		break;
	case EventLabel::ThreadFinish:
		s << "THREAD_END";
		break;
	case EventLabel::Read:
	case EventLabel::BWaitRead:
	case EventLabel::CondVarWaitRead:
	case EventLabel::SpeculativeRead:
	case EventLabel::ConfirmingRead:
		s << "R";
		break;
	case EventLabel::CasRead:
	case EventLabel::LockCasRead:
	case EventLabel::TrylockCasRead:
	case EventLabel::HelpedCasRead:
	case EventLabel::ConfirmingCasRead:
		s << "CR";
		break;
	case EventLabel::FaiRead:
	case EventLabel::BIncFaiRead:
	case EventLabel::NoRetFaiRead:
		s << "UR";
		break;
	case EventLabel::Write:
	case EventLabel::BInitWrite:
	case EventLabel::BDestroyWrite:
	case EventLabel::CondVarInitWrite:
	case EventLabel::CondVarSignalWrite:
	case EventLabel::CondVarBcastWrite:
	case EventLabel::CondVarDestroyWrite:
	case EventLabel::UnlockWrite:
		s << "W";
		break;
	case EventLabel::CasWrite:
	case EventLabel::LockCasWrite:
	case EventLabel::TrylockCasWrite:
	case EventLabel::HelpedCasWrite:
	case EventLabel::ConfirmingCasWrite:
		s << "CW";
		break;
	case EventLabel::FaiWrite:
	case EventLabel::BIncFaiWrite:
	case EventLabel::NoRetFaiWrite:
		s << "UW";
		break;
	case EventLabel::Fence:
		s << "F";
		break;
	case EventLabel::Malloc:
		s << "MALLOC";
		break;
	case EventLabel::Free:
		s << "FREE";
		break;
	case EventLabel::HpRetire:
		s << "HP_RETIRE";
		break;
	case EventLabel::ThreadCreate:
		s << "THREAD_CREATE";
		break;
	case EventLabel::ThreadJoin:
		s << "THREAD_JOIN";
		break;
	case EventLabel::HelpingCas:
		s << "HELPING_CAS";
		break;
	case EventLabel::HpProtect:
		s << "HP_PROTECT";
		break;
	case EventLabel::Optional:
		s << "OPTIONAL";
		break;
	case EventLabel::LoopBegin:
		s << "LOOP_BEGIN";
		break;
	case EventLabel::SpinStart:
		s << "SPIN_START";
		break;
	case EventLabel::FaiZNESpinEnd:
	case EventLabel::LockZNESpinEnd:
		s << "ZNE_SPIN_END";
		break;
		break;
	case EventLabel::Empty:
		s << "EMPTY";
		break;
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-label-type", "Cannot print label type");
		s << "UNKNOWN";
		break;
	}
	return s;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const llvm::AtomicOrdering o)
{
	switch (o) {
	case llvm::AtomicOrdering::NotAtomic:
		return s << "na";
	case llvm::AtomicOrdering::Unordered:
		return s << "un";
	case llvm::AtomicOrdering::Monotonic:
		return s << "rlx";
	case llvm::AtomicOrdering::Acquire:
		return s << "acq";
	case llvm::AtomicOrdering::Release:
		return s << "rel";
	case llvm::AtomicOrdering::AcquireRelease:
		return s << "ar";
	case llvm::AtomicOrdering::SequentiallyConsistent:
		return s << "sc";
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-ordering-type", "Cannot print ordering");
		return s;
	}
	return s;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const EventLabel &lab)
{
	s << LabelPrinter().toString(lab);
	return s;
}
