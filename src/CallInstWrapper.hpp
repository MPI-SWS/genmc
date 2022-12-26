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

#ifndef __CALL_INST_WRAPPER_HPP__
#define __CALL_INST_WRAPPER_HPP__

#include "config.h"
#include <llvm/IR/Instructions.h>

#if LLVM_VERSION_MAJOR < 11
#include <llvm/IR/CallSite.h>
#endif


using namespace llvm;

/*
 * The class llvm::CallSite was removed in LLVM 11 in favor of
 * llvm::CallBase. To maintain compatibility with older versions that
 * still utilize this class CallInstWrapper can be used instead; it
 * boils down to either llvm::CallSite or llvm::CallBase.
 */

#if LLVM_VERSION_MAJOR < 11
class CallInstWrapper {

public:
	/* Constructors */
	CallInstWrapper() : CS() {}
	CallInstWrapper(CallSite CS) : CS(CS) {}
	CallInstWrapper(CallInst *CI) : CS(CI) {}

	/* Getters emulation */
	llvm::Value *getCalledOperand() { return CS.getCalledValue(); }
	llvm::Function *getCalledFunction() { return CS.getCalledFunction(); }
	size_t arg_size() const { return CS.arg_size(); }
	CallSite::arg_iterator arg_begin() { return CS.arg_begin(); }
	CallSite::arg_iterator arg_end() { return CS.arg_end(); }

	/* Operators to use instead of member functions like
	 * getInstruction() that do not exist in CallBase */
	Instruction *operator&() { return CS.getInstruction(); }

private:
	CallSite CS;
};

#else

class CallInstWrapper {

public:
	/* Constructors */
	CallInstWrapper() : CB(nullptr) {}
	CallInstWrapper(CallBase *CB) : CB(CB) {}
	CallInstWrapper(CallBase &CB) : CB(&CB) {}
	CallInstWrapper(CallInst *CI) : CB(CI) {}

	/* Getters emulation */
	llvm::Value *getCalledOperand() { return CB->getCalledOperand(); }
	llvm::Function *getCalledFunction() { return CB->getCalledFunction(); }
	size_t arg_size() const { return CB->arg_size(); }
	User::op_iterator arg_begin() { return CB->arg_begin(); }
	User::op_iterator arg_end() { return CB->arg_end(); }

	Instruction *operator&() { return CB; }

private:
	CallBase *CB;
};
#endif

#endif /* __CALL_INST_WRAPPER__ */
