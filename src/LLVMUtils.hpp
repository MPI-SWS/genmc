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

#ifndef __LLVM_UTILS_HPP__
#define __LLVM_UTILS_HPP__

#include "config.h"
#include "Error.hpp"
#include "EventAttr.hpp"
#include "VSet.hpp"
#ifdef LLVM_HAVE_ELIMINATE_UNCREACHABLE_BLOCKS
#include <llvm/Analysis/DomTreeUpdater.h>
#endif
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CFG.h>
#include <string>

#ifdef LLVM_HAS_TERMINATORINST
 typedef llvm::TerminatorInst TerminatorInst;
#else
 typedef llvm::Instruction TerminatorInst;
#endif

/*
 * Returns true if o1 and o2 are the same ordering as far as a load
 * operation is concerned. This catches cases where e.g.,
 * o1 is acq_rel and o2 is acq.
 * */
bool areSameLoadOrdering(llvm::AtomicOrdering o1, llvm::AtomicOrdering o2);

/*
 * Strips all kinds of casts from val (including trunc, zext ops, etc)
 */
llvm::Value *stripCasts(llvm::Value *val);

/*
 * Strips all casts and GEPs from val
 */
llvm::Value *stripCastsGEPs(llvm::Value *val);

/*
 * Strips all casts and constant operations from val
 */
llvm::Value *stripCastsConstOps(llvm::Value *val);

/*
 * Returns the name of the function ci calls
 */
std::string getCalledFunOrStripValName(const llvm::CallInst &ci);

/*
 * Returns true if its argument is an intrinsic call that does
 * not have any side-effects.
 */
bool isIntrinsicCallNoSideEffects(const llvm::Instruction &i);

// FIXME: Change name
/*
 * Returns true if i1 depends on o2
 */
bool isDependentOn(const llvm::Instruction *i1, const llvm::Instruction *i2);

/*
 * If EI extracts its value from an integer CAS instruction, returns said CAS;
 * otherwise returns nullptr.
 */
llvm::AtomicCmpXchgInst *extractsFromCAS(llvm::ExtractValueInst *ei);

/*
 * Returns true if its argument has side-effects.
 * Clean intrinsic functions (e.g., assert, assume) are considered side-effect-free.
 * Calls to non-intrinsic functions are considered to produce side-effects
 * unless a clean list "cleanFuns" is provided.
 */
bool hasSideEffects(const llvm::Instruction *i, const VSet<llvm::Function *> *cleanFuns = nullptr);

/*
 * Returns true if I allocates memory.
 * If a list of allocFuns is provided, then these are also considered as allocating instructions.
 */
bool isAlloc(const llvm::Instruction *i, const VSet<llvm::Function *> *allocFuns = nullptr);

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
llvm::BasicBlock *tryThreadSuccessor(llvm::BranchInst *term, llvm::BasicBlock *succ);

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
	template<typename F>
	void foreachInBackPathTo(llvm::BasicBlock *curr,
				 llvm::BasicBlock *to,
				 llvm::SmallVector<llvm::BasicBlock *, 4> &path,
				 F&& fun)
	{
		path.push_back(curr);
		if (curr == to) {
			for (auto *bb : path)
				std::for_each(bb->rbegin(), bb->rend(), fun);
			path.pop_back();
			return;
		}

		for (auto *pred: predecessors(curr))
			if (std::find(path.begin(), path.end(), pred) == path.end())
				foreachInBackPathTo(pred, to, path, fun);
		path.pop_back();
		return;
	}
}

/*
 * Executes FUN for all instructions from FROM to TO.
 * TO needs to be a predecessor of FROM, otherwise FUN is not applied.
 * FUN is applied in reverse iteration order within a block.
 */

template<typename F>
void foreachInBackPathTo(llvm::BasicBlock *from, llvm::BasicBlock *to, F&& fun)
{
	llvm::SmallVector<llvm::BasicBlock *, 4> path;
	details::foreachInBackPathTo(from, to, path, fun);
}

/*
 * LLVM Utilities for older LLVM versions
 */

/* Copy portions of LLVM code for older LLVM versions */
#ifndef LLVM_HAVE_ELIMINATE_UNREACHABLE_BLOCKS

using namespace llvm;

#ifndef LLVM_HAVE_DF_ITERATOR_DEFAULT_SET
template <typename NodeRef, unsigned SmallSize=8>
struct df_iterator_default_set : public SmallPtrSet<NodeRef, SmallSize> {
  typedef SmallPtrSet<NodeRef, SmallSize>  BaseSet;
  typedef typename BaseSet::iterator iterator;
  std::pair<iterator,bool> insert(NodeRef N) { return BaseSet::insert(N) ; }
  template <typename IterT>
  void insert(IterT Begin, IterT End) { BaseSet::insert(Begin,End); }

  void completed(NodeRef) { }
};
#endif

namespace llvm {
	class DomTreeUpdater;
}

void DetatchDeadBlocks(
	ArrayRef<BasicBlock *> BBs,
	// SmallVectorImpl<DominatorTree::UpdateType> *Updates,
	bool KeepOneInputPHIs);

void DeleteDeadBlocks(ArrayRef <BasicBlock *> BBs, DomTreeUpdater *DTU,
		      bool KeepOneInputPHIs);

void DeleteDeadBlock(BasicBlock *BB, DomTreeUpdater *DTU = nullptr,
		     bool KeepOneInputPHIs = false);

bool EliminateUnreachableBlocks(Function &F, DomTreeUpdater *DTU = nullptr,
				bool KeepOneInputPHIs = false);

#endif /* !LLVM_HAVE_ELIMINATE_UNREACHABLE_BLOCKS */

void replaceUsesWithIf(llvm::Value *Old, llvm::Value *New, llvm::function_ref<bool(llvm::Use &U)> ShouldReplace);

#endif /* __LLVM_UTILS_HPP__ */
