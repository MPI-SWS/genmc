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

#ifndef GENMC_LABEL_VISITOR_HPP
#define GENMC_LABEL_VISITOR_HPP

#include "ExecutionGraph/EventLabel.hpp"
#include "Support/Error.hpp"
#include <llvm/Support/Casting.h>

/*******************************************************************************
 **                           LabelVisitor Class
 ******************************************************************************/

/*
 * Visitor for EventLabel objects (CRTP-style).
 */

template <typename Subclass> class LabelVisitor {

public:
	void visit(const EventLabel *lab) { return visit(*lab); }
	void visit(const std::unique_ptr<EventLabel> &lab) { return visit(*lab); }
	void visit(const std::shared_ptr<EventLabel> &lab) { return visit(*lab); }

	void visit(const EventLabel &lab)
	{
		switch (lab.getKind()) {
#define HANDLE_LABEL(NUM, NAME)                                                                    \
	case EventLabel::NAME:                                                                     \
		return static_cast<Subclass *>(this)->visit##NAME##Label(                          \
			static_cast<const NAME##Label &>(lab));
#include "ExecutionGraph/EventLabel.def"
		default:
			BUG();
		}
	}

#define DELEGATE_LABEL(TO_CLASS)                                                                   \
	static_cast<Subclass *>(this)->visit##TO_CLASS(static_cast<const TO_CLASS &>(lab))

	void visitThreadStartLabel(const ThreadStartLabel &lab)
	{
		return DELEGATE_LABEL(EventLabel);
	}
	void visitInitLabel(const InitLabel &lab) { return DELEGATE_LABEL(ThreadStartLabel); }

	void visitSpinloopBlockLabel(const SpinloopBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitFaiZNEBlockLabel(const FaiZNEBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitLockZNEBlockLabel(const LockZNEBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitHelpedCASBlockLabel(const HelpedCASBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitConfirmationBlockLabel(const ConfirmationBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitLockNotAcqBlockLabel(const LockNotAcqBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitLockNotRelBlockLabel(const LockNotRelBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitBarrierBlockLabel(const BarrierBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitErrorBlockLabel(const ErrorBlockLabel &lab) { return DELEGATE_LABEL(BlockLabel); }
	void visitUserBlockLabel(const UserBlockLabel &lab) { return DELEGATE_LABEL(BlockLabel); }
	void visitJoinBlockLabel(const JoinBlockLabel &lab) { return DELEGATE_LABEL(BlockLabel); }
	void visitReadOptBlockLabel(const ReadOptBlockLabel &lab)
	{
		return DELEGATE_LABEL(BlockLabel);
	}
	void visitThreadKillLabel(const ThreadKillLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitThreadFinishLabel(const ThreadFinishLabel &lab)
	{
		return DELEGATE_LABEL(EventLabel);
	}

	void visitReadLabel(const ReadLabel &lab) { return DELEGATE_LABEL(MemAccessLabel); }
	void visitBWaitReadLabel(const BWaitReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitCondVarWaitReadLabel(const CondVarWaitReadLabel &lab)
	{
		return DELEGATE_LABEL(ReadLabel);
	}
	void visitSpeculativeReadLabel(const SpeculativeReadLabel &lab)
	{
		return DELEGATE_LABEL(ReadLabel);
	}
	void visitConfirmingReadLabel(const ConfirmingReadLabel &lab)
	{
		return DELEGATE_LABEL(ReadLabel);
	}
	void visitCasReadLabel(const CasReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitLockCasReadLabel(const LockCasReadLabel &lab)
	{
		return DELEGATE_LABEL(CasReadLabel);
	}
	void visitTrylockCasReadLabel(const TrylockCasReadLabel &lab)
	{
		return DELEGATE_LABEL(CasReadLabel);
	}
	void visitHelpedCasReadLabel(const HelpedCasReadLabel &lab)
	{
		return DELEGATE_LABEL(CasReadLabel);
	}
	void visitConfirmingCasReadLabel(const ConfirmingCasReadLabel &lab)
	{
		return DELEGATE_LABEL(CasReadLabel);
	}
	void visitFaiReadLabel(const FaiReadLabel &lab) { return DELEGATE_LABEL(ReadLabel); }
	void visitNoRetFaiReadLabel(const NoRetFaiReadLabel &lab)
	{
		return DELEGATE_LABEL(FaiReadLabel);
	}
	void visitBIncFaiReadLabel(const BIncFaiReadLabel &lab)
	{
		return DELEGATE_LABEL(FaiReadLabel);
	}

	void visitWriteLabel(const WriteLabel &lab) { return DELEGATE_LABEL(MemAccessLabel); }
	void visitUnlockWriteLabel(const UnlockWriteLabel &lab)
	{
		return DELEGATE_LABEL(WriteLabel);
	}
	void visitBInitWriteLabel(const BInitWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitBDestroyWriteLabel(const BDestroyWriteLabel &lab)
	{
		return DELEGATE_LABEL(WriteLabel);
	}
	void visitCondVarInitWriteLabel(const CondVarInitWriteLabel &lab)
	{
		return DELEGATE_LABEL(WriteLabel);
	}
	void visitCondVarSignalWriteLabel(const CondVarSignalWriteLabel &lab)
	{
		return DELEGATE_LABEL(WriteLabel);
	}
	void visitCondVarBcastWriteLabel(const CondVarBcastWriteLabel &lab)
	{
		return DELEGATE_LABEL(WriteLabel);
	}
	void visitCondVarDestroyWriteLabel(const CondVarDestroyWriteLabel &lab)
	{
		return DELEGATE_LABEL(WriteLabel);
	}
	void visitCasWriteLabel(const CasWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitLockCasWriteLabel(const LockCasWriteLabel &lab)
	{
		return DELEGATE_LABEL(CasWriteLabel);
	}
	void visitTrylockCasWriteLabel(const TrylockCasWriteLabel &lab)
	{
		return DELEGATE_LABEL(CasWriteLabel);
	}
	void visitHelpedCasWriteLabel(const HelpedCasWriteLabel &lab)
	{
		return DELEGATE_LABEL(CasWriteLabel);
	}
	void visitConfirmingCasWriteLabel(const ConfirmingCasWriteLabel &lab)
	{
		return DELEGATE_LABEL(CasWriteLabel);
	}
	void visitFaiWriteLabel(const FaiWriteLabel &lab) { return DELEGATE_LABEL(WriteLabel); }
	void visitNoRetFaiWriteLabel(const NoRetFaiWriteLabel &lab)
	{
		return DELEGATE_LABEL(FaiWriteLabel);
	}
	void visitBIncFaiWriteLabel(const BIncFaiWriteLabel &lab)
	{
		return DELEGATE_LABEL(FaiWriteLabel);
	}

	void visitFenceLabel(const FenceLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	// void visitSubFenceLabel(const SubFenceLabel &lab) { return DELEGATE_LABEL(FenceLabel); }

	void visitMallocLabel(const MallocLabel &lab) { return DELEGATE_LABEL(EventLabel); }

	void visitFreeLabel(const FreeLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitHpRetireLabel(const HpRetireLabel &lab) { return DELEGATE_LABEL(FreeLabel); }

	void visitThreadCreateLabel(const ThreadCreateLabel &lab)
	{
		return DELEGATE_LABEL(EventLabel);
	}
	void visitThreadJoinLabel(const ThreadJoinLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitHpProtectLabel(const HpProtectLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitHelpingCasLabel(const HelpingCasLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitOptionalLabel(const OptionalLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitLoopBeginLabel(const LoopBeginLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitSpinStartLabel(const SpinStartLabel &lab) { return DELEGATE_LABEL(EventLabel); }
	void visitFaiZNESpinEndLabel(const FaiZNESpinEndLabel &lab)
	{
		return DELEGATE_LABEL(EventLabel);
	}
	void visitLockZNESpinEndLabel(const LockZNESpinEndLabel &lab)
	{
		return DELEGATE_LABEL(EventLabel);
	}
	void visitEmptyLabel(const EmptyLabel &lab) { return DELEGATE_LABEL(EventLabel); }

	/* Matchers for abstract classes */

	/*
	 * If none of the above matched, propagate to the next level.
	 * Override the ones below to handle a bunch of cases.
	 */
	void visitMemAccessLabel(const MemAccessLabel &lab) { return DELEGATE_LABEL(EventLabel); }

	/* Similar to the above, but for blocked labels */
	void visitBlockLabel(const BlockLabel &lab) { return DELEGATE_LABEL(EventLabel); }

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

template <typename Subclass> class LabelPrinterBase : public LabelVisitor<Subclass> {

public:
	using FmterT = std::function<std::string(SAddr)>;
	using GetterT = std::function<SVal(const ReadLabel &)>;

	LabelPrinterBase()
		: out(buf), fmtFun([&](const SAddr &saddr) {
			  std::string buf;
			  llvm::raw_string_ostream s(buf);
			  s << saddr;
			  return s.str();
		  }),
		  valFun()
	{}
	LabelPrinterBase(FmterT addrFmter, GetterT readValGetter)
		: out(buf), fmtFun(addrFmter), valFun(readValGetter)
	{}

#define IMPLEMENT_INTEGER_PRINT(OS, TY)                                                            \
	case AType::Signed:                                                                        \
		OS << val.getSigned();                                                             \
		break;                                                                             \
	case AType::Unsigned:                                                                      \
		OS << val.get();                                                                   \
		break;

#define IMPLEMENT_POINTER_PRINT(OS, TY)                                                            \
	case AType::Pointer:                                                                       \
		OS << val.getPointer();                                                            \
		break;

	void printVal(const SVal &val, AType atyp)
	{
		switch (atyp) {
			IMPLEMENT_INTEGER_PRINT(out, atyp);
			IMPLEMENT_POINTER_PRINT(out, atyp);
		default:
			PRINT_BUGREPORT_INFO_ONCE("val-printing",
						  "Unhandled type for value predicate!\n");
		}
	}

	void visitReadLabel(const ReadLabel &lab)
	{
		DELEGATE_LABEL(MemAccessLabel);
		out << " (" << fmtFun(lab.getAddr());
		if (valFun) {
			out << ", ";
			printVal(valFun(lab), lab.getType());
		}
		out << ")";
	}

	void visitWriteLabel(const WriteLabel &lab)
	{
		DELEGATE_LABEL(MemAccessLabel);
		out << " (" << fmtFun(lab.getAddr()) << ", ";
		printVal(lab.getVal(), lab.getType());
		out << ")";
	}

	void visitFenceLabel(const FenceLabel &lab)
	{
		DELEGATE_LABEL(EventLabel);
		out << lab.getOrdering();
	}

	void visitMallocLabel(const MallocLabel &lab)
	{
		DELEGATE_LABEL(EventLabel);
		out << " " << lab.getName();
	}

	void visitThreadCreateLabel(const ThreadCreateLabel &lab)
	{
		DELEGATE_LABEL(EventLabel);
		out << " [thread " << lab.getChildId() << "]";
	}

	void visitThreadJoinLabel(const ThreadJoinLabel &lab)
	{
		DELEGATE_LABEL(EventLabel);
		out << " [thread " << lab.getChildId() << "]";
	}

	/* Generic handlers */

	void visitMemAccessLabel(const MemAccessLabel &lab)
	{
		DELEGATE_LABEL(EventLabel);
		out << lab.getOrdering();
	}

	void visitEventLabel(const EventLabel &lab)
	{
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

	LabelPrinter() : LabelPrinterBase(){};
	LabelPrinter(FmterT fmter, GetterT getter) : LabelPrinterBase(fmter, getter) {}

	std::string toString(const EventLabel &lab)
	{
		buf.clear();
		this->visit(lab);
		return out.str();
	}

	/* Helper to print read RFs */
	void printRf(const ReadLabel &lab)
	{
		if (!lab.getRf())
			out << "[BOTTOM]";
		else if (lab.getRf()->getPos().isInitializer())
			out << "[INIT]";
		else
			out << "[" << lab.getRf()->getPos() << "]";
	}

	void visitReadLabel(const ReadLabel &lab)
	{
		LabelPrinterBase::visitReadLabel(lab);
		out << " ";
		printRf(lab);
	}
};

class DotPrinter : public LabelPrinterBase<DotPrinter> {

public:
	using FmterT = LabelPrinterBase<DotPrinter>::FmterT;
	using GetterT = LabelPrinterBase<DotPrinter>::GetterT;

	DotPrinter() : LabelPrinterBase(){};
	DotPrinter(FmterT fmter, GetterT getter) : LabelPrinterBase(fmter, getter) {}

	std::string toString(const EventLabel &lab)
	{
		buf.clear();
		this->visit(lab);
		return out.str();
	}

	/* Helpers to print orderings as exponent */
	void printOrdering(const EventLabel &lab)
	{
		out << "<SUP>" << lab.getOrdering() << "</SUP>";
	}

	void visitFenceLabel(const FenceLabel &lab)
	{
		visitEventLabel(lab);
		printOrdering(lab);
	}

	void visitThreadCreateLabel(const ThreadCreateLabel &lab) { visitEventLabel(lab); }

	void visitThreadJoinLabel(const ThreadJoinLabel &lab) { visitEventLabel(lab); }

	/* Generic handlers */

	void visitMemAccessLabel(const EventLabel &lab)
	{
		visitEventLabel(lab);
		printOrdering(lab);
	}

	void visitEventLabel(const EventLabel &lab) { out << lab.getKind(); }
};

#endif /* GENMC_LABEL_VISITOR_HPP */
