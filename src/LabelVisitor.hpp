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

#ifndef __LABEL_VISITOR_HPP__
#define __LABEL_VISITOR_HPP__

#include "Error.hpp"
#include "EventLabel.hpp"
#include <llvm/Support/Casting.h>

/*******************************************************************************
 **                           LabelVisitor Class
 ******************************************************************************/

/*
 * Visitor for EventLabel objects (CRTP-style).
 */

template<typename Subclass>
class LabelVisitor {

public:
	/* Gets a variadic arguments because some labels have the extension suffix */
#define VISIT_LABEL(NAME, ...)					\
	case EventLabel::EL_##NAME##__VA_ARGS__:		\
		return static_cast<Subclass *>(this)->		\
		visit##NAME##Label##__VA_ARGS__(static_cast<const NAME##Label##__VA_ARGS__&>(lab))

	void visit(const EventLabel *lab) { return visit(*lab); }
	void visit(const std::unique_ptr<EventLabel> &lab) { return visit(*lab); }
	void visit(const std::shared_ptr<EventLabel> &lab) { return visit(*lab); }

	void visit(const EventLabel &lab) {
		switch(lab.getKind()) {
			VISIT_LABEL(Empty);
			VISIT_LABEL(Block);
			VISIT_LABEL(Optional);
			VISIT_LABEL(ThreadStart);
			VISIT_LABEL(Init);
			VISIT_LABEL(ThreadFinish);
			VISIT_LABEL(ThreadCreate);
			VISIT_LABEL(ThreadJoin);
			VISIT_LABEL(ThreadKill);
			VISIT_LABEL(LoopBegin);
			VISIT_LABEL(SpinStart);
			VISIT_LABEL(FaiZNESpinEnd);
			VISIT_LABEL(LockZNESpinEnd);
			VISIT_LABEL(Read);
			VISIT_LABEL(BWaitRead);
			VISIT_LABEL(SpeculativeRead);
			VISIT_LABEL(ConfirmingRead);
			VISIT_LABEL(FaiRead);
			VISIT_LABEL(NoRetFaiRead);
			VISIT_LABEL(BIncFaiRead);
			VISIT_LABEL(CasRead);
			VISIT_LABEL(LockCasRead);
			VISIT_LABEL(TrylockCasRead);
			VISIT_LABEL(HelpedCasRead);
			VISIT_LABEL(ConfirmingCasRead);
			VISIT_LABEL(DskRead);
			VISIT_LABEL(Write);
			VISIT_LABEL(UnlockWrite);
			VISIT_LABEL(BInitWrite);
			VISIT_LABEL(BDestroyWrite);
			VISIT_LABEL(FaiWrite);
			VISIT_LABEL(NoRetFaiWrite);
			VISIT_LABEL(BIncFaiWrite);
			VISIT_LABEL(CasWrite);
			VISIT_LABEL(LockCasWrite);
			VISIT_LABEL(TrylockCasWrite);
			VISIT_LABEL(HelpedCasWrite);
			VISIT_LABEL(ConfirmingCasWrite);
			VISIT_LABEL(DskWrite);
			VISIT_LABEL(DskMdWrite);
			VISIT_LABEL(DskJnlWrite);
			VISIT_LABEL(DskDirWrite);
			VISIT_LABEL(Fence);
			VISIT_LABEL(DskFsync);
			VISIT_LABEL(DskSync);
			VISIT_LABEL(DskPbarrier);
			VISIT_LABEL(SmpFence, LKMM);
			VISIT_LABEL(RCUSync, LKMM);
			VISIT_LABEL(Malloc);
			VISIT_LABEL(Free);
			VISIT_LABEL(HpRetire);
			VISIT_LABEL(HpProtect);
			VISIT_LABEL(Lock, LAPOR);
			VISIT_LABEL(Unlock, LAPOR);
			VISIT_LABEL(HelpingCas);
			VISIT_LABEL(DskOpen);
			VISIT_LABEL(RCULock, LKMM);
			VISIT_LABEL(RCUUnlock, LKMM);
			VISIT_LABEL(CLFlush);
		default:
			BUG();
		}
	}

#define DELEGATE_LABEL(TO_CLASS)				\
	static_cast<Subclass *>(this)->				\
	visit##TO_CLASS(static_cast<const TO_CLASS&>(lab))

	void visitEmptyLabel(const EmptyLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitBlockLabel(const BlockLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitOptionalLabel(const OptionalLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitInitLabel(const InitLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitThreadStartLabel(const ThreadStartLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitThreadFinishLabel(const ThreadFinishLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitThreadCreateLabel(const ThreadCreateLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitThreadJoinLabel(const ThreadJoinLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitThreadKillLabel(const ThreadKillLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitLoopBeginLabel(const LoopBeginLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitSpinStartLabel(const SpinStartLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitFaiZNESpinEndLabel(const FaiZNESpinEndLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitLockZNESpinEndLabel(const LockZNESpinEndLabel &lab) { return DELEGATE_LABEL(EventLabel); }

	void visitReadLabel(const ReadLabel &lab) { return DELEGATE_LABEL(MemAccessLabel); }
	void visitBWaitReadLabel(const BWaitReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitSpeculativeReadLabel(const SpeculativeReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitConfirmingReadLabel(const ConfirmingReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitDskReadLabel(const DskReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }

	void visitFaiReadLabel(const FaiReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitNoRetFaiReadLabel(const NoRetFaiReadLabel &lab) { return DELEGATE_LABEL(FaiReadLabel); }
	void visitBIncFaiReadLabel(const BIncFaiReadLabel &lab) { return DELEGATE_LABEL(FaiReadLabel); }

	void visitCasReadLabel(const CasReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitLockCasReadLabel(const LockCasReadLabel &lab) { return DELEGATE_LABEL(CasReadLabel); }
	void visitTrylockCasReadLabel(const TrylockCasReadLabel &lab) { return DELEGATE_LABEL(CasReadLabel); }
	void visitHelpedCasReadLabel(const HelpedCasReadLabel &lab) { return DELEGATE_LABEL(CasReadLabel); }
	void visitConfirmingCasReadLabel(const ConfirmingCasReadLabel &lab) { return DELEGATE_LABEL(CasReadLabel); }

	void visitWriteLabel(const WriteLabel &lab) { return DELEGATE_LABEL(MemAccessLabel); }
	void visitUnlockWriteLabel(const UnlockWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitBInitWriteLabel(const BInitWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitBDestroyWriteLabel(const BDestroyWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }

	void visitFaiWriteLabel(const FaiWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitNoRetFaiWriteLabel(const NoRetFaiWriteLabel &lab) { return DELEGATE_LABEL(FaiWriteLabel); }
	void visitBIncFaiWriteLabel(const BIncFaiWriteLabel &lab) { return DELEGATE_LABEL(FaiWriteLabel); }

	void visitCasWriteLabel(const CasWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitLockCasWriteLabel(const LockCasWriteLabel &lab) { return DELEGATE_LABEL(CasWriteLabel); }
	void visitTrylockCasWriteLabel(const TrylockCasWriteLabel &lab) { return DELEGATE_LABEL(CasWriteLabel); }
	void visitHelpedCasWriteLabel(const HelpedCasWriteLabel &lab) { return DELEGATE_LABEL(CasWriteLabel); }
	void visitConfirmingCasWriteLabel(const ConfirmingCasWriteLabel &lab) { return DELEGATE_LABEL(CasWriteLabel); }

	void visitDskWriteLabel(const DskWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitDskMdWriteLabel(const DskMdWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitDskJnlWriteLabel(const DskJnlWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitDskDirWriteLabel(const DskDirWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }

	void visitFenceLabel(const FenceLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitDskFsyncLabel(const DskFsyncLabel &lab) { return DELEGATE_LABEL(FenceLabel); }
	void visitDskSyncLabel(const DskSyncLabel &lab) { return DELEGATE_LABEL(FenceLabel); }
	void visitDskPbarrierLabel(const DskPbarrierLabel &lab) { return DELEGATE_LABEL(FenceLabel); }
	void visitSmpFenceLabelLKMM(const SmpFenceLabelLKMM &lab) { return DELEGATE_LABEL(FenceLabel); }
	void visitRCUSyncLabelLKMM(const RCUSyncLabelLKMM &lab) { return DELEGATE_LABEL(FenceLabel); }

	void visitMallocLabel(const MallocLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitFreeLabel(const FreeLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitHpRetireLabel(const HpRetireLabel &lab) { return DELEGATE_LABEL(FreeLabel); }

	void visitHpProtectLabel(const HpProtectLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitLockLabelLAPOR(const LockLabelLAPOR &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitUnlockLabelLAPOR(const UnlockLabelLAPOR &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitHelpingCasLabel(const HelpingCasLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitDskOpenLabel(const DskOpenLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitRCULockLabelLKMM(const RCULockLabelLKMM &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitRCUUnlockLabelLKMM(const RCUUnlockLabelLKMM &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitCLFlushLabel(const CLFlushLabel &lab) { return DELEGATE_LABEL(EventLabel); }

	/*
	 * If none of the above matched, propagate to the next level.
	 * Override the ones below to handle a bunch of cases.
	 */
	void visitMemAccessLabel(const MemAccessLabel &lab) { return DELEGATE_LABEL(EventLabel); }

	/*
	 * If no one else could handle this particular instruction,
	 * call the generic handler
	 */
	void visitEventLabel(const EventLabel &lab) { return; }

};


/*******************************************************************************
 **                           LabelPrinter Class
 ******************************************************************************/

/*
 * Prints a label to a string.
 */

template<typename Subclass>
class LabelPrinterBase : public LabelVisitor<Subclass> {

public:
	using FmterT = std::function<std::string(SAddr)>;
	using GetterT = std::function<SVal(const ReadLabel&)>;

	LabelPrinterBase()
		: out(buf),
		  fmtFun([&](const SAddr &saddr){
				      std::string buf;
				      llvm::raw_string_ostream s(buf);
				      s << saddr;
				      return s.str();
			      }),
		  valFun() {}
	LabelPrinterBase(FmterT addrFmter, GetterT readValGetter)
			 : out(buf), fmtFun(addrFmter), valFun(readValGetter) {}

#define IMPLEMENT_INTEGER_PRINT(OS, TY)			\
	case AType::Signed:				\
		OS << val.getSigned();			\
		break;					\
	case AType::Unsigned:				\
		OS << val.get();			\
		break;

#define IMPLEMENT_POINTER_PRINT(OS, TY)			\
	case AType::Pointer:				\
		OS << val.getPointer();			\
		break;

	void printVal(const SVal &val, AType atyp) {
		switch (atyp) {
			IMPLEMENT_INTEGER_PRINT(out, atyp);
			IMPLEMENT_POINTER_PRINT(out, atyp);
		default:
			PRINT_BUGREPORT_INFO_ONCE("val-printing",
						  "Unhandled type for value predicate!\n");
		}
	}

	void visitBlockLabel(const BlockLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << " [" << lab.getType() << "]";
	}

	void visitReadLabel(const ReadLabel &lab) {
		DELEGATE_LABEL(MemAccessLabel);
		out << " (" << fmtFun(lab.getAddr());
		if (valFun) {
			out << ", ";
			printVal(valFun(lab), lab.getType());
		}
		out << ")";
	}

	void visitWriteLabel(const WriteLabel &lab) {
		DELEGATE_LABEL(MemAccessLabel);
		out << " (" << fmtFun(lab.getAddr()) << ", ";
		printVal(lab.getVal(), lab.getType());
		out << ")";
	}

	void visitFenceLabel(const FenceLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << lab.getOrdering();
	}

	void visitSmpFenceLabelLKMM(const SmpFenceLabelLKMM &lab) {
		DELEGATE_LABEL(EventLabel);
		out << lab.getType();
	}

	void visitCLFlushLabel(const CLFlushLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << " " << fmtFun(lab.getAddr());
	}

	void visitThreadCreateLabel(const ThreadCreateLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << " [thread " << lab.getChildId() << "]";
	}

	void visitThreadJoinLabel(const ThreadJoinLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << " [thread " << lab.getChildId() << "]";
	}

	void visitDskOpenLabel(const DskOpenLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << " (" << lab.getFileName() << ", " << lab.getFd() << ")";
	}

	void visitMallocLabel(const MallocLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << " " << lab.getName();
	}

	/* Generic handlers */

	void visitMemAccessLabel(const MemAccessLabel &lab) {
		DELEGATE_LABEL(EventLabel);
		out << lab.getOrdering();
	}

	void visitEventLabel(const EventLabel &lab) {
		out << lab.getPos() << ": " << lab.getKind();
	}

protected:
	std::string buf;
	llvm::raw_string_ostream out;
	FmterT fmtFun;
	GetterT valFun;
};


class LabelPrinter : public LabelPrinterBase<LabelPrinter> {

public:
	using FmterT = LabelPrinterBase<LabelPrinter>::FmterT;
	using GetterT = LabelPrinterBase<LabelPrinter>::GetterT;

	LabelPrinter() : LabelPrinterBase() {};
	LabelPrinter(FmterT fmter, GetterT getter) : LabelPrinterBase(fmter, getter) {}

	std::string toString(const EventLabel &lab) {
		buf.clear();
		this->visit(lab);
		return out.str();
	}

	/* Helper to print read RFs */
	void printRf(const ReadLabel &lab) {
		if (!lab.getRf())
			out << "[BOTTOM]";
		else if (lab.getRf()->getPos().isInitializer())
			out << "[INIT]";
		else
			out << "[" << lab.getRf()->getPos() << "]";
	}

	void visitReadLabel(const ReadLabel &lab) {
		LabelPrinterBase::visitReadLabel(lab);
		out << " ";
		printRf(lab);
	}
};

class DotPrinter : public LabelPrinterBase<DotPrinter> {

public:
	using FmterT = LabelPrinterBase<DotPrinter>::FmterT;
	using GetterT = LabelPrinterBase<DotPrinter>::GetterT;

	DotPrinter() : LabelPrinterBase() {};
	DotPrinter(FmterT fmter, GetterT getter) : LabelPrinterBase(fmter, getter) {}

	std::string toString(const EventLabel &lab) {
		buf.clear();
		this->visit(lab);
		return out.str();
	}

	/* Helpers to print orderings as exponent */
	void printOrdering(const EventLabel &lab) {
		out << "<SUP>" << lab.getOrdering() << "</SUP>";
	}
	void printFenceType(const SmpFenceLabelLKMM &lab) {
		out << "<SUP>" << lab.getType() << "</SUP>";
	}

	void visitFenceLabel(const FenceLabel &lab) {
		visitEventLabel(lab);
		printOrdering(lab);
	}

	void visitSmpFenceLabelLKMM(const SmpFenceLabelLKMM &lab) {
		visitEventLabel(lab);
		printFenceType(lab);
	}

	void visitThreadCreateLabel(const ThreadCreateLabel &lab) {
		visitEventLabel(lab);
	}

	void visitThreadJoinLabel(const ThreadJoinLabel &lab) {
		visitEventLabel(lab);
	}

	/* Generic handlers */

	void visitMemAccessLabel(const EventLabel &lab) {
		visitEventLabel(lab);
		printOrdering(lab);
	}

	void visitEventLabel(const EventLabel &lab) {
		out << lab.getKind();
	}
};

#endif /* __LABEL_VISITOR_HPP__ */
