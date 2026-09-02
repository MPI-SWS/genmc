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

#include "LLVMUtils.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/Support/ActionEnums.hpp"
#include "passes/InternalFunctions.hpp"

#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/ValueHandle.h>
#include <llvm/Support/AtomicOrdering.h>
#include <llvm/Support/Casting.h>

#include <cstdint>
#include <string>

using namespace llvm;

auto areSameLoadOrdering(AtomicOrdering ord1, AtomicOrdering ord2) -> bool
{
	return ord1 == ord2 ||
	       (ord1 == AtomicOrdering::Acquire && ord2 == AtomicOrdering::AcquireRelease) ||
	       (ord1 == AtomicOrdering::AcquireRelease && ord2 == AtomicOrdering::Acquire) ||
	       (ord1 == AtomicOrdering::Monotonic && ord2 == AtomicOrdering::Release) ||
	       (ord1 == AtomicOrdering::Release && ord2 == AtomicOrdering::Monotonic);
}

auto stripCasts(Value *val) -> Value *
{
	while (isa<CastInst>(val))
		val = dyn_cast<CastInst>(val)->getOperand(0);
	return val;
}

auto stripCastsGEPs(Value *val) -> Value *
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

static auto getNonConstantOp(const Instruction *i) -> Value *
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
static auto getNonConstOpFromBinopOrCmp(const Value *v) -> Value *
{
	if (const auto *bop = dyn_cast<BinaryOperator>(v)) {
		return getNonConstantOp(bop);
	}
	if (const auto *cop = dyn_cast<CmpInst>(v)) {
		return getNonConstantOp(cop);
	}
	return nullptr;
}

auto stripCastsConstOps(Value *val) -> Value *
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

auto getCalledFunOrStripValName(const CallInst &ci) -> std::string
{
	if (auto *fun = ci.getCalledFunction())
		return fun->getName().str();
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	return CallInstWrapper(const_cast<CallInst *>(&ci))
		.getCalledOperand()
		->stripPointerCasts()
		->getName()
		.str();
}

auto isIntrinsicCallNoSideEffects(const Instruction &i) -> bool
{
	const auto *ci = dyn_cast<CallInst>(&i);
	if (!ci)
		return false;

	return isCleanInternalFunction(getCalledFunOrStripValName(*ci));
}

auto extractsFromCAS(ExtractValueInst *extract) -> AtomicCmpXchgInst *
{
	if (!extract->getType()->isIntegerTy() || extract->getNumIndices() > 1)
		return nullptr;
	return dyn_cast<AtomicCmpXchgInst>(extract->getAggregateOperand());
}

static auto isDependentOn(const Instruction *i1, const Instruction *i2,
			  VSet<const Instruction *> chain) -> bool
{
	if (!i1 || !i2 || chain.contains(i1))
		return false;

	for (const auto &use : i1->operands()) {
		if (auto *i = dyn_cast<Instruction>(use.get())) {
			chain.insert(i1);
			if (i == i2 || isDependentOn(i, i2, chain))
				return true;
			chain.erase(i1);
		}
	}
	return false;
}

auto isDependentOn(const Instruction *i1, const Instruction *i2) -> bool
{
	const VSet<const Instruction *> chain;
	return isDependentOn(i1, i2, chain);
}

auto hasSideEffects(const Instruction *i, const VSet<Function *> *cleanFuns /* = nullptr */) -> bool
{
	if (isa<AllocaInst>(i))
		return true;
	if (i->mayHaveSideEffects()) {
		if (const auto *ci = dyn_cast<CallInst>(i)) {
			auto name = getCalledFunOrStripValName(*ci);
			if (isInternalFunction(name))
				return !isCleanInternalFunction(name);
			if (!cleanFuns)
				return true;
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
			CallInstWrapper callWrapper(const_cast<CallInst *>(ci));
			const auto *fun = dyn_cast<Function>(
				callWrapper.getCalledOperand()->stripPointerCasts());
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
			if (!fun || !cleanFuns->contains(const_cast<Function *>(fun)))
				return true;
		} else if (!isa<LoadInst>(i) && !isa<FenceInst>(i)) {
			return true;
		}
	}
	return false;
}

auto isAlloc(const Instruction *i, const VSet<Function *> *allocFuns /* = nullptr */) -> bool
{
	const auto *si = i->stripPointerCasts();
	if (isa<AllocaInst>(si))
		return true;

	const auto *ci = dyn_cast<CallInst>(si);
	if (!ci)
		return false;

	if (isAllocFunction(getCalledFunOrStripValName(*ci)))
		return true;

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	CallInstWrapper callWrapper(const_cast<CallInst *>(ci));
	const auto *fun = dyn_cast<Function>(callWrapper.getCalledOperand()->stripPointerCasts());
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
	return allocFuns && allocFuns->contains(const_cast<Function *>(fun));
}

auto hasLoadSemantics(llvm::Instruction *I) -> bool
{
	/* Overapproximate with function calls some of which might be modeled as loads */
	auto *ci = llvm::dyn_cast<llvm::CallInst>(I);
	return llvm::isa<llvm::LoadInst>(I) || llvm::isa<llvm::AtomicCmpXchgInst>(I) ||
	       llvm::isa<llvm::AtomicRMWInst>(I) ||
	       (ci && ci->getCalledFunction() &&
		hasGlobalLoadSemantics(ci->getCalledFunction()->getName().str()));
}

auto getInstKind(llvm::Instruction *I) -> ActionKind
{
	return hasLoadSemantics(I) ? ActionKind::Load : ActionKind::NonLoad;
}

void annotateInstruction(llvm::Instruction *i, const std::string &type, uint64_t value)
{
	auto &ctx = i->getContext();
	auto *metadata = i->getMetadata(type);

	/* If there are already metadata, accumulate */
	uint64_t mValue = value;
	if (metadata) {
		auto old = cast<ConstantInt>(
				   cast<ConstantAsMetadata>(metadata->getOperand(0))->getValue())
				   ->getZExtValue();
		mValue |= old;
	}

	auto *node =
		MDNode::get(ctx, ConstantAsMetadata::get(ConstantInt::get(ctx, APInt(64, mValue))));
	i->setMetadata(type, node);
}

auto tryThreadSuccessor(BranchInst *term, BasicBlock *succ) -> BasicBlock *
{
	auto *succTerm = dyn_cast<BranchInst>(succ->getTerminator());
	if (!succTerm || succTerm != &*succ->begin() || succTerm->isConditional())
		return nullptr;

	/* If there are PHIs that depend on SUCC be conservative and
	 * do not transform, as B might jump to DESTBB too */
	auto *destBB = succTerm->getSuccessor(0);
	if (isa<PHINode>(&*destBB->begin()))
		return nullptr;

	for (auto i = 0U; i < term->getNumSuccessors(); i++) {
		if (term->getSuccessor(i) == succ) {
			term->setSuccessor(i, destBB);
			return destBB;
		}
	}
	return nullptr;
}

void replaceUsesWithIf(Value *Old, Value *New, llvm::function_ref<bool(Use &U)> ShouldReplace)
{
	// assert(New && "Value::replaceUsesWithIf(<null>) is invalid!");
	// assert(New->getType() == old->getType() &&
	//        "replaceUses of value with new value of different type!");

	SmallVector<TrackingVH<Constant>, 8> Consts;
	SmallPtrSet<Constant *, 8> Visited;

	for (auto UI = Old->use_begin(), end = Old->use_end(); UI != end;) {
		Use &U = *UI;
		++UI;
		if (!ShouldReplace(U))
			continue;
		// Must handle Constants specially, we cannot call replaceUsesOfWith on a
		// constant because they are uniqued.
		if (auto *constant = dyn_cast<Constant>(U.getUser())) {
			if (!isa<GlobalValue>(constant)) {
				if (Visited.insert(constant).second)
					Consts.push_back(TrackingVH<Constant>(constant));
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
