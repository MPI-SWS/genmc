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

#include "LLVMUtils.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/ValueHandle.h>

#include <unordered_set>

using namespace llvm;

bool areSameLoadOrdering(AtomicOrdering o1, AtomicOrdering o2)
{
	return o1 == o2 ||
	       (o1 == AtomicOrdering::Acquire && o2 == AtomicOrdering::AcquireRelease) ||
	       (o1 == AtomicOrdering::AcquireRelease && o2 == AtomicOrdering::Acquire) ||
	       (o1 == AtomicOrdering::Monotonic && o2 == AtomicOrdering::Release) ||
	       (o1 == AtomicOrdering::Release && o2 == AtomicOrdering::Monotonic);
}

Value *stripCasts(Value *val)
{
	while (isa<CastInst>(val))
		val = dyn_cast<CastInst>(val)->getOperand(0);
	return val;
}

Value *stripCastsGEPs(Value *val)
{
	while (true) {
		if (auto *ci = dyn_cast<CastInst>(val))
			val = ci->getOperand(0);
		else if (auto *gepi = dyn_cast<GetElementPtrInst>(val))
			val = gepi->getPointerOperand();
		else
			break;
	}
	return val;
}

Value *getNonConstantOp(const Instruction *i)
{
	if (isa<Constant>(i->getOperand(1)))
		return i->getOperand(0);
	if (isa<Constant>(i->getOperand(0)))
		return i->getOperand(1);
	return nullptr;
}

/*
 * If V is a binop/cmpop, and one of the operators of V is a constant,
 * returns the other operator of V.
 * If both operators of V are non-const, returns nullptr.
 */
Value *getNonConstOpFromBinopOrCmp(const Value *v)
{
	if (auto *bop = dyn_cast<BinaryOperator>(v)) {
		return getNonConstantOp(bop);
	} else if (auto *cop = dyn_cast<CmpInst>(v)) {
		return getNonConstantOp(cop);
	}
	return nullptr;
}

Value *stripCastsConstOps(Value *val)
{
	while (true) {
		if (auto *ci = dyn_cast<CastInst>(val)) {
			val = ci->getOperand(0);
		} else if (auto *v = getNonConstOpFromBinopOrCmp(val)) {
			val = v;
		} else {
			break;
		}
	}
	return val;
}

std::string getCalledFunOrStripValName(const CallInst &ci)
{
	if (auto *fun = ci.getCalledFunction())
		return fun->getName().str();
	return CallInstWrapper(const_cast<CallInst *>(&ci))
		.getCalledOperand()
		->stripPointerCasts()
		->getName()
		.str();
}

bool isIntrinsicCallNoSideEffects(const Instruction &i)
{
	auto *ci = dyn_cast<CallInst>(&i);
	if (!ci)
		return false;

	return isCleanInternalFunction(getCalledFunOrStripValName(*ci));
}

AtomicCmpXchgInst *extractsFromCAS(ExtractValueInst *extract)
{
	if (!extract->getType()->isIntegerTy() || extract->getNumIndices() > 1)
		return nullptr;
	return dyn_cast<AtomicCmpXchgInst>(extract->getAggregateOperand());
}

bool isDependentOn(const Instruction *i1, const Instruction *i2, VSet<const Instruction *> chain)
{
	if (!i1 || !i2 || chain.find(i1) != chain.end())
		return false;

	for (auto &u : i1->operands()) {
		if (auto *i = dyn_cast<Instruction>(u.get())) {
			chain.insert(i1);
			if (i == i2 || isDependentOn(i, i2, chain))
				return true;
			chain.erase(i1);
		}
	}
	return false;
}

bool isDependentOn(const Instruction *i1, const Instruction *i2)
{
	VSet<const Instruction *> chain;
	return isDependentOn(i1, i2, chain);
}

bool hasSideEffects(const Instruction *i, const VSet<Function *> *cleanFuns /* = nullptr */)
{
	if (isa<AllocaInst>(i))
		return true;
	if (i->mayHaveSideEffects()) {
		if (auto *ci = dyn_cast<CallInst>(i)) {
			auto name = getCalledFunOrStripValName(*ci);
			if (isInternalFunction(name))
				return !isCleanInternalFunction(name);
			if (!cleanFuns)
				return true;
			CallInstWrapper CW(const_cast<CallInst *>(ci));
			const auto *fun =
				dyn_cast<Function>(CW.getCalledOperand()->stripPointerCasts());
			if (!fun || !cleanFuns->count(const_cast<Function *>(fun)))
				return true;
		} else if (!isa<LoadInst>(i) && !isa<FenceInst>(i)) {
			return true;
		}
	}
	return false;
}

bool isAlloc(const Instruction *i, const VSet<Function *> *allocFuns /* = nullptr */)
{
	auto *si = i->stripPointerCasts();
	if (isa<AllocaInst>(si))
		return true;

	auto *ci = dyn_cast<CallInst>(si);
	if (!ci)
		return false;

	if (isAllocFunction(getCalledFunOrStripValName(*ci)))
		return true;

	CallInstWrapper CW(const_cast<CallInst *>(ci));
	const auto *fun = dyn_cast<Function>(CW.getCalledOperand()->stripPointerCasts());
	return allocFuns && allocFuns->count(const_cast<Function *>(fun));
}

void annotateInstruction(llvm::Instruction *i, const std::string &type, uint64_t value)
{
	auto &ctx = i->getContext();
	auto *md = i->getMetadata(type);

	/* If there are already metadata, accumulate */
	uint64_t mValue = value;
	if (md) {
		auto old = dyn_cast<ConstantInt>(
				   dyn_cast<ConstantAsMetadata>(md->getOperand(0))->getValue())
				   ->getZExtValue();
		mValue |= old;
	}

	auto *node =
		MDNode::get(ctx, ConstantAsMetadata::get(ConstantInt::get(ctx, APInt(64, mValue))));
	i->setMetadata(type, node);
	return;
}

BasicBlock *tryThreadSuccessor(BranchInst *term, BasicBlock *succ)
{
	auto *succTerm = dyn_cast<BranchInst>(succ->getTerminator());
	if (!succTerm || succTerm != &*succ->begin() || succTerm->isConditional())
		return nullptr;

	/* If there are PHIs that depend on SUCC be conservative and
	 * do not transform, as B might jump to DESTBB too */
	auto *destBB = succTerm->getSuccessor(0);
	if (isa<PHINode>(&*destBB->begin()))
		return nullptr;

	for (auto i = 0u; i < term->getNumSuccessors(); i++) {
		if (term->getSuccessor(i) == succ) {
			term->setSuccessor(i, destBB);
			return destBB;
		}
	}
	return nullptr;
}

#if LLVM_VERSION_MAJOR < 9

void DetatchDeadBlocks(ArrayRef<BasicBlock *> BBs,
		       // SmallVectorImpl<DominatorTree::UpdateType> *Updates,
		       bool KeepOneInputPHIs)
{
	for (auto *BB : BBs) {
		// Loop through all of our successors and make sure they know that one
		// of their predecessors is going away.
		SmallPtrSet<BasicBlock *, 4> UniqueSuccessors;
		for (BasicBlock *Succ : successors(BB)) {
			Succ->removePredecessor(BB, KeepOneInputPHIs);
			// if (Updates && UniqueSuccessors.insert(Succ).second)
			//   Updates->push_back({DominatorTree::Delete, BB, Succ});
		}

		// Zap all the instructions in the block.
		while (!BB->empty()) {
			Instruction &I = BB->back();
			// If this instruction is used, replace uses with an arbitrary value.
			// Because control flow can't get here, we don't care what we replace the
			// value with.  Note that since this block is unreachable, and all values
			// contained within it must dominate their uses, that all uses will
			// eventually be removed (they are themselves dead).
			if (!I.use_empty())
				I.replaceAllUsesWith(UndefValue::get(I.getType()));
			BB->getInstList().pop_back();
		}
		new UnreachableInst(BB->getContext(), BB);
		assert(BB->getInstList().size() == 1 && isa<UnreachableInst>(BB->getTerminator()) &&
		       "The successor list of BB isn't empty before "
		       "applying corresponding DTU updates.");
	}
}

void DeleteDeadBlocks(ArrayRef<BasicBlock *> BBs, DomTreeUpdater *DTU, bool KeepOneInputPHIs)
{
	// #ifndef NDEBUG
	// 	// Make sure that all predecessors of each dead block is also dead.
	// 	SmallPtrSet<BasicBlock *, 4> Dead(BBs.begin(), BBs.end());
	// 	assert(Dead.size() == BBs.size() && "Duplicating blocks?");
	// 	for (auto *BB : Dead)
	// 		for (BasicBlock *Pred : predecessors(BB))
	// 			assert(Dead.count(Pred) && "All predecessors must be dead!");
	// #endif

	// SmallVector<DominatorTree::UpdateType, 4> Updates;
	DetatchDeadBlocks(BBs, // DTU ? &Updates : nullptr,
			  KeepOneInputPHIs);

	// if (DTU)
	//   DTU->applyUpdatesPermissive(Updates);

	for (BasicBlock *BB : BBs)
		// if (DTU)
		//   DTU->deleteBB(BB);
		// else
		BB->eraseFromParent();
}

void DeleteDeadBlock(BasicBlock *BB, DomTreeUpdater *DTU /* = nullptr */,
		     bool KeepOneInputPHIs /* = false */)
{
	DeleteDeadBlocks({BB}, DTU, KeepOneInputPHIs);
}

bool EliminateUnreachableBlocks(Function &F, DomTreeUpdater *DTU /* = nullptr */,
				bool KeepOneInputPHIs /* = false */)
{
	df_iterator_default_set<BasicBlock *> Reachable;

	// Mark all reachable blocks.
	for (BasicBlock *BB : depth_first_ext(&F, Reachable))
		(void)BB /* Mark all reachable blocks */;

	// Collect all dead blocks.
	std::vector<BasicBlock *> DeadBlocks;
	for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I)
		if (!Reachable.count(&*I)) {
			BasicBlock *BB = &*I;
			DeadBlocks.push_back(BB);
		}

	// Delete the dead blocks.
	DeleteDeadBlocks(DeadBlocks, DTU, KeepOneInputPHIs);

	return !DeadBlocks.empty();
}

#endif /* LLVM_VERSION_MAJOR < 9 */

void replaceUsesWithIf(Value *Old, Value *New, llvm::function_ref<bool(Use &U)> ShouldReplace)
{
	// assert(New && "Value::replaceUsesWithIf(<null>) is invalid!");
	// assert(New->getType() == old->getType() &&
	//        "replaceUses of value with new value of different type!");

	SmallVector<TrackingVH<Constant>, 8> Consts;
	SmallPtrSet<Constant *, 8> Visited;

	for (auto UI = Old->use_begin(), E = Old->use_end(); UI != E;) {
		Use &U = *UI;
		++UI;
		if (!ShouldReplace(U))
			continue;
		// Must handle Constants specially, we cannot call replaceUsesOfWith on a
		// constant because they are uniqued.
		if (auto *C = dyn_cast<Constant>(U.getUser())) {
			if (!isa<GlobalValue>(C)) {
				if (Visited.insert(C).second)
					Consts.push_back(TrackingVH<Constant>(C));
				continue;
			}
		}
		U.set(New);
	}

	while (!Consts.empty()) {
		// FIXME: handleOperandChange() updates all the uses in a given Constant,
		//        not just the one passed to ShouldReplace
		Consts.pop_back_val()->handleOperandChange(Old, New);
	}
}
