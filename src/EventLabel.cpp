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

#include "EventLabel.hpp"
#include "LabelVisitor.hpp"

llvm::raw_ostream& operator<<(llvm::raw_ostream& s,
			      const EventLabel::EventLabelKind k)
{
	switch (k) {
	case EventLabel::EL_Empty:
		s << "EMPTY";
		break;
	case EventLabel::EL_Block:
		s << "BLOCK";
		break;
	case EventLabel::EL_ThreadKill:
		s << "KILL";
		break;
	case EventLabel::EL_Optional:
		s << "OPTIONAL";
		break;
	case EventLabel::EL_LoopBegin:
		s << "LOOP_BEGIN";
		break;
	case EventLabel::EL_SpinStart:
		s << "SPIN_START";
		break;
	case EventLabel::EL_FaiZNESpinEnd:
	case EventLabel::EL_LockZNESpinEnd:
		s << "ZNE_SPIN_END";
		break;
	case EventLabel::EL_Read:
	case EventLabel::EL_BWaitRead:
	case EventLabel::EL_SpeculativeRead:
	case EventLabel::EL_ConfirmingRead:
		s << "R";
		break;
	case EventLabel::EL_FaiRead:
	case EventLabel::EL_BIncFaiRead:
	case EventLabel::EL_NoRetFaiRead:
		s << "UR";
		break;
	case EventLabel::EL_FaiWrite:
	case EventLabel::EL_BIncFaiWrite:
	case EventLabel::EL_NoRetFaiWrite:
		s << "UW";
		break;
	case EventLabel::EL_CasRead:
	case EventLabel::EL_LockCasRead:
	case EventLabel::EL_TrylockCasRead:
	case EventLabel::EL_HelpedCasRead:
	case EventLabel::EL_ConfirmingCasRead:
		s << "CR";
		break;
	case EventLabel::EL_CasWrite:
	case EventLabel::EL_LockCasWrite:
	case EventLabel::EL_TrylockCasWrite:
	case EventLabel::EL_HelpedCasWrite:
	case EventLabel::EL_ConfirmingCasWrite:
		s << "CW";
		break;
	case EventLabel::EL_HelpingCas:
		s << "HELPING_CAS";
		break;
	case EventLabel::EL_Write:
	case EventLabel::EL_BInitWrite:
	case EventLabel::EL_BDestroyWrite:
	case EventLabel::EL_UnlockWrite:
		s << "W";
		break;
	case EventLabel::EL_Fence:
	case EventLabel::EL_SmpFenceLKMM:
		s << "F";
		break;
	case EventLabel::EL_ThreadCreate:
		s << "THREAD_CREATE";
		break;
	case EventLabel::EL_ThreadJoin:
		s << "THREAD_JOIN";
		break;
	case EventLabel::EL_ThreadStart:
		s << "THREAD_START";
		break;
	case EventLabel::EL_ThreadFinish:
		s << "THREAD_END";
		break;
	case EventLabel::EL_Malloc:
		s << "MALLOC";
		break;
	case EventLabel::EL_Free:
		s << "FREE";
		break;
	case EventLabel::EL_HpRetire:
		s << "HP_RETIRE";
		break;
	case EventLabel::EL_HpProtect:
		s << "HP_PROTECT";
		break;
	case EventLabel::EL_LockLAPOR:
		s << "LAPOR_LOCK";
		break;
	case EventLabel::EL_UnlockLAPOR:
		s << "LAPOR_UNLOCK";
		break;
	case EventLabel::EL_DskOpen:
		s << "FOPEN";
		break;
	case EventLabel::EL_DskRead:
		s << "DR";
		break;
	case EventLabel::EL_DskWrite:
	case EventLabel::EL_DskMdWrite:
	case EventLabel::EL_DskDirWrite:
	case EventLabel::EL_DskJnlWrite:
		s << "DW";
		break;
	case EventLabel::EL_DskFsync:
		s << "FSYNC";
		break;
	case EventLabel::EL_DskSync:
		s << "SYNC";
		break;
	case EventLabel::EL_DskPbarrier:
		s << "PERSISTENCY_BARRIER";
		break;
	case EventLabel::EL_RCULockLKMM:
		s << "RCU_LOCK";
		break;
	case EventLabel::EL_RCUUnlockLKMM:
		s << "RCU_UNLOCK";
		break;
	case EventLabel::EL_RCUSyncLKMM:
		s << "RCU_SYNC";
		break;
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-label-type",
					  "Cannot print label type");
		s << "UNKNOWN";
		break;
	}
	return s;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const llvm::AtomicOrdering o)
{
	switch (o) {
	case llvm::AtomicOrdering::NotAtomic : return s << "na";
	case llvm::AtomicOrdering::Unordered : return s << "un";
	case llvm::AtomicOrdering::Monotonic : return s << "rlx";
	case llvm::AtomicOrdering::Acquire   : return s << "acq";
	case llvm::AtomicOrdering::Release   : return s << "rel";
	case llvm::AtomicOrdering::AcquireRelease : return s << "ar";
	case llvm::AtomicOrdering::SequentiallyConsistent : return s << "sc";
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-ordering-type", "Cannot print ordering");
		return s;
	}
	return s;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const SmpFenceType t)
{
	switch (t) {
	case SmpFenceType::MB : return s << "mb";
	case SmpFenceType::WMB : return s << "wmb";
	case SmpFenceType::RMB : return s << "rmb";
	case SmpFenceType::MBBA: return s << "ba";
	case SmpFenceType::MBAA: return s << "aa";
	case SmpFenceType::MBAS: return s << "as";
	case SmpFenceType::MBAUL: return s << "aul";
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-fence-type", "Cannot print fence type");
	}
	return s;
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const EventLabel &lab)
{
	s << LabelPrinter().toString(lab);
	return s;
}
