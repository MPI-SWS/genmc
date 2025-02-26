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

#ifndef GENMC_LLVM_UTILS_HPP
#define GENMC_LLVM_UTILS_HPP

#include "ADT/VSet.hpp"
#include "ExecutionGraph/EventAttr.hpp"
#include "Support/Error.hpp"
#include "config.h"
#include <llvm/IR/CFG.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <string>

using TerminatorInst = llvm::Instruction;

#if LLVM_VERSION_MAJOR < 17
#define GLOBALS(M) (M).getGlobalList()
#else
#define GLOBALS(M) (M).globals()
#endif

#if LLVM_VERSION_MAJOR < 11

#include <llvm/IR/CallSite.h>

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
	CallInstWrapper(llvm::CallBase *CB) : CB(CB) {}
	CallInstWrapper(llvm::CallBase &CB) : CB(&CB) {}
	CallInstWrapper(llvm::CallInst *CI) : CB(CI) {}

	/* Getters emulation */
	auto getCalledOperand() -> llvm::Value * { return CB->getCalledOperand(); }
	auto getCalledFunction() -> llvm::Function * { return CB->getCalledFunction(); }
	auto arg_size() const -> size_t { return CB->arg_size(); }
	auto arg_begin() -> llvm::User::op_iterator { return CB->arg_begin(); }
	auto arg_end() -> llvm::User::op_iterator { return CB->arg_end(); }

	auto operator&() -> llvm::Instruction * { return CB; }

private:
	llvm::CallBase *CB;
};
#endif

/*
 * Returns true if o1 and o2 are the same ordering as far as a load
 * operation is concerned. This catches cases where e.g.,
 * o1 is acq_rel and o2 is acq.
 * */
auto areSameLoadOrdering(llvm::AtomicOrdering o1, llvm::AtomicOrdering o2) -> bool;

/*
 * Strips all kinds of casts from val (including trunc, zext ops, etc)
 */
auto stripCasts(llvm::Value *val) -> llvm::Value *;

/*
 * Strips all casts and GEPs from val
 */
auto stripCastsGEPs(llvm::Value *val) -> llvm::Value *;

/*
 * Strips all casts and constant operations from val
 */
auto stripCastsConstOps(llvm::Value *val) -> llvm::Value *;

/*
 * Returns the name of the function ci calls
 */
auto getCalledFunOrStripValName(const llvm::CallInst &ci) -> std::string;

/*
 * Returns true if its argument is an intrinsic call that does
 * not have any side-effects.
 */
auto isIntrinsicCallNoSideEffects(const llvm::Instruction &i) -> bool;

// FIXME: Change name
/*
 * Returns true if i1 depends on o2
 */
auto isDependentOn(const llvm::Instruction *i1, const llvm::Instruction *i2) -> bool;

/*
 * If EI extracts its value from an integer CAS instruction, returns said CAS;
 * otherwise returns nullptr.
 */
auto extractsFromCAS(llvm::ExtractValueInst *ei) -> llvm::AtomicCmpXchgInst *;

/*
 * Returns true if its argument has side-effects.
 * Clean intrinsic functions (e.g., assert, assume) are considered side-effect-free.
 * Calls to non-intrinsic functions are considered to produce side-effects
 * unless a clean list "cleanFuns" is provided.
 */
auto hasSideEffects(const llvm::Instruction *i, const VSet<llvm::Function *> *cleanFuns = nullptr)
	-> bool;

/*
 * Returns true if I allocates memory.
 * If a list of allocFuns is provided, then these are also considered as allocating instructions.
 */
auto isAlloc(const llvm::Instruction *i, const VSet<llvm::Function *> *allocFuns = nullptr) -> bool;

/*
 * Annotates I by setting the metadata TYPE to VALUE
 */
void annotateInstruction(llvm::Instruction *i, const std::string &type, uint64_t value);

/*
 * Given the terminator TERM of a block B and a successor SUCC of B,
 * tries to make B directly jump to SUCC's successor, if SUCC is an
 * empty block with an unconditional jump.
 * Returns the destination block if it succeeded, and null otherwise.
 */
auto tryThreadSuccessor(llvm::BranchInst *term, llvm::BasicBlock *succ) -> llvm::BasicBlock *;

/*
 * Extracts the write attribute from an (annotated) instruction.
 * Returns NONE if the instruction is not annotated.
 */
inline WriteAttr getWriteAttr(llvm::Instruction &I)
{
	auto *md = I.getMetadata("genmc.attr");
	if (!md)
		return WriteAttr::None;

	auto *op = llvm::dyn_cast<llvm::ConstantAsMetadata>(md->getOperand(0));
	BUG_ON(!op);

	auto flags = llvm::dyn_cast<llvm::ConstantInt>(op->getValue())->getZExtValue();
	return static_cast<WriteAttr>(flags);
}

namespace details {
template <typename F>
void foreachInBackPathTo(llvm::BasicBlock *curr, llvm::BasicBlock *to,
			 llvm::SmallVector<llvm::BasicBlock *, 4> &path, F &&fun)
{
	path.push_back(curr);
	if (curr == to) {
		for (auto *bb : path)
			std::for_each(bb->rbegin(), bb->rend(), fun);
		path.pop_back();
		return;
	}

	for (auto *pred : predecessors(curr))
		if (std::find(path.begin(), path.end(), pred) == path.end())
			foreachInBackPathTo(pred, to, path, fun);
	path.pop_back();
	return;
}
} // namespace details

/*
 * Executes FUN for all instructions from FROM to TO.
 * TO needs to be a predecessor of FROM, otherwise FUN is not applied.
 * FUN is applied in reverse iteration order within a block.
 */

template <typename F>
void foreachInBackPathTo(llvm::BasicBlock *from, llvm::BasicBlock *to, F &&fun)
{
	llvm::SmallVector<llvm::BasicBlock *, 4> path;
	::details::foreachInBackPathTo(from, to, path, fun);
}

/*
 * LLVM Utilities for older LLVM versions
 */

void replaceUsesWithIf(llvm::Value *Old, llvm::Value *New,
		       llvm::function_ref<bool(llvm::Use &U)> ShouldReplace);

#endif /* GENMC_LLVM_UTILS_HPP */
