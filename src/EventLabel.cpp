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

EventLabel *EventLabel::castFromDskAccessLabel (const DskAccessLabel *D)
{
	EventLabel::EventLabelKind DK = D->getEventLabelKind();
	switch (DK) {
	case EventLabel::EventLabelKind::EL_DskRead:
		return static_cast<DskReadLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskWrite:
		return static_cast<DskWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskMdWrite:
		return static_cast<DskMdWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskJnlWrite:
		return static_cast<DskJnlWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskDirWrite:
		return static_cast<DskDirWriteLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskSync:
		return static_cast<DskSyncLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskFsync:
		return static_cast<DskFsyncLabel *>(const_cast<DskAccessLabel *>(D));
	case EventLabel::EventLabelKind::EL_DskPbarrier:
		return static_cast<DskPbarrierLabel *>(const_cast<DskAccessLabel *>(D));
	default:
		BUG();
	}
}

DskAccessLabel *EventLabel::castToDskAccessLabel(const EventLabel *E)
{
	EventLabel::EventLabelKind EK = E->getKind();
	switch (EK) {
	case EventLabel::EventLabelKind::EL_DskRead:
		return static_cast<DskReadLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskWrite:
		return static_cast<DskWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskMdWrite:
		return static_cast<DskMdWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskJnlWrite:
		return static_cast<DskJnlWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskDirWrite:
		return static_cast<DskDirWriteLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskSync:
		return static_cast<DskSyncLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskFsync:
		return static_cast<DskFsyncLabel *>(const_cast<EventLabel *>(E));
	case EventLabel::EventLabelKind::EL_DskPbarrier:
		return static_cast<DskPbarrierLabel *>(const_cast<EventLabel *>(E));
	default:
		BUG();
	}
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& s,
			      const EventLabel::EventLabelKind k)
{
	switch (k) {
	case EventLabel::EL_Read:
	case EventLabel::EL_LibRead:
		s << "R";
		break;
	case EventLabel::EL_FaiRead:
	case EventLabel::EL_FaiWrite:
		s << "U";
		break;
	case EventLabel::EL_CasRead:
	case EventLabel::EL_CasWrite:
		s << "C";
		break;
	case EventLabel::EL_Write:
	case EventLabel::EL_LibWrite:
		s << "W";
		break;
	case EventLabel::EL_Fence:
		s << "F";
		break;
	case EventLabel::EL_ThreadCreate:
		s << "TC";
		break;
	case EventLabel::EL_ThreadJoin:
		s << "TJ";
		break;
	case EventLabel::EL_ThreadStart:
		s << "B";
		break;
	case EventLabel::EL_ThreadFinish:
		s << "E";
		break;
	case EventLabel::EL_Malloc:
		s << "M";
		break;
	case EventLabel::EL_Free:
		s << "D";
		break;
	case EventLabel::EL_LockLabelLAPOR:
		s << "LL";
		break;
	case EventLabel::EL_UnlockLabelLAPOR:
		s << "LU";
	case EventLabel::EL_DskOpen:
		s << "FO";
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
		s << "DF";
		break;
	case EventLabel::EL_DskSync:
		s << "DS";
		break;
	case EventLabel::EL_DskPbarrier:
		s << "PB";
		break;
	default:
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
	default : return s;
	}
	return s;
}

#define PRINT_RF(s, e)				\
do {					        \
	if (e.isInitializer())			\
		s << "INIT";			\
	else					\
		s << e;				\
} while (0)

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const EventLabel &lab)
{
	s << lab.getPos() << ": ";

	switch (lab.getKind()) {
	case EventLabel::EL_Empty: {
		s << "EMPTY";
		break;
	}
	case EventLabel::EL_Read: {
		auto &rLab = static_cast<const ReadLabel&>(lab);
		s << rLab.getKind() << rLab.getOrdering() << " [";
		PRINT_RF(s, rLab.getRf());
		s << "]";
		break;
	}
	case EventLabel::EL_FaiRead: {
		auto &rLab = static_cast<const FaiReadLabel&>(lab);
		s << rLab.getKind() << rLab.getOrdering() << " [";
		PRINT_RF(s, rLab.getRf());
		s << "]";
		break;
	}
	case EventLabel::EL_CasRead: {
		auto &rLab = static_cast<const CasReadLabel&>(lab);
		s << rLab.getKind() << rLab.getOrdering() << " [";
		PRINT_RF(s, rLab.getRf());
		s << "]";
		break;
	}
	case EventLabel::EL_LibRead: {
		auto &rLab = static_cast<const LibReadLabel&>(lab);
		s << rLab.getKind() << rLab.getOrdering() << " ("
		  << rLab.getFunctionName() << ") [";
		PRINT_RF(s, rLab.getRf());
		s << "]";
		break;
	}
	case EventLabel::EL_Write: {
		auto &wLab = static_cast<const WriteLabel&>(lab);
		s << wLab.getKind() << wLab.getOrdering() << " " << wLab.getVal().IntVal;
		break;
	}
	case EventLabel::EL_FaiWrite: {
		auto &wLab = static_cast<const FaiWriteLabel&>(lab);
		s << wLab.getKind() << wLab.getOrdering() << " "
		  << wLab.getVal().IntVal;
		break;
	}
	case EventLabel::EL_CasWrite: {
		auto &wLab = static_cast<const CasWriteLabel&>(lab);
		s << wLab.getKind() << wLab.getOrdering() << " "
		  << wLab.getVal().IntVal << "";
		break;
	}
	case EventLabel::EL_LibWrite: {
		auto &wLab = static_cast<const LibWriteLabel&>(lab);
		s << wLab.getKind() << wLab.getOrdering() << " ("
		  << wLab.getFunctionName() << ") " << wLab.getVal().IntVal;
		break;
	}
	case EventLabel::EL_Fence: {
		auto &fLab = static_cast<const FenceLabel&>(lab);
		s << fLab.getKind() << fLab.getOrdering();
		break;
	}
	case EventLabel::EL_DskFsync: {
		auto &fLab = static_cast<const DskSyncLabel&>(lab);
		s << fLab.getKind();
		break;
	}
	case EventLabel::EL_DskSync: {
		auto &fLab = static_cast<const DskSyncLabel&>(lab);
		s << fLab.getKind();
		break;
	}
	case EventLabel::EL_ThreadCreate: {
		auto &cLab = static_cast<const ThreadCreateLabel&>(lab);
		s << cLab.getKind() << " [forks " << cLab.getChildId() << "]";
		break;
	}
	case EventLabel::EL_ThreadJoin: {
		auto &jLab = static_cast<const ThreadJoinLabel&>(lab);
		s << jLab.getKind();
		break;
	}
	case EventLabel::EL_ThreadStart: {
		auto &bLab = static_cast<const ThreadStartLabel&>(lab);
		s << bLab.getKind();
		break;
	}
	case EventLabel::EL_ThreadFinish: {
		auto &eLab = static_cast<const ThreadFinishLabel&>(lab);
		s << eLab.getKind();
		break;
	}
	case EventLabel::EL_Malloc: {
		auto &bLab = static_cast<const MallocLabel&>(lab);
		s << bLab.getKind();
		break;
	}
	case EventLabel::EL_Free: {
		auto &bLab = static_cast<const FreeLabel&>(lab);
		s << bLab.getKind();
		break;
	}
	case EventLabel::EL_LockLabelLAPOR: {
		auto &lLab = static_cast<const LockLabelLAPOR&>(lab);
		s << lLab.getKind();
		break;
	}
	case EventLabel::EL_UnlockLabelLAPOR: {
		auto &uLab = static_cast<const UnlockLabelLAPOR&>(lab);
		s << uLab.getKind();
		break;
	}
	case EventLabel::EL_DskOpen: {
		auto &bLab = static_cast<const DskOpenLabel&>(lab);
		s << bLab.getKind() << " (";
		s << bLab.getFileName() << ", ";
		s << bLab.getFd().IntVal.getLimitedValue() << ")";
		break;
	}
	case EventLabel::EL_DskRead: {
		auto &rLab = static_cast<const DskReadLabel&>(lab);
		s << rLab.getKind() << " [";
		PRINT_RF(s, rLab.getRf());
		s << "]";
		break;
	}
	case EventLabel::EL_DskWrite: {
		auto &wLab = static_cast<const DskWriteLabel&>(lab);
		s << wLab.getKind() << " " << wLab.getVal().IntVal;
		break;
	}
	case EventLabel::EL_DskPbarrier: {
		auto &pLab = static_cast<const DskPbarrierLabel&>(lab);
		s << pLab.getKind();
		break;
	}
	default:
		s << "UNKNOWN";
		break;
	}
	return s;
}
