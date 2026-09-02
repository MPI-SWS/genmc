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

#include "EliminateCastsPass.hpp"
#include "passes/LLVMUtils.hpp"

#include <llvm/Analysis/ValueTracking.h>
#include <llvm/Config/llvm-config.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/Casting.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>

#include <algorithm>
#include <vector>

using namespace llvm;

#define IS_LIFETIME_START_OR_END(i) (i)->isLifetimeStartOrEnd()
#define IS_DROPPABLE(i) (i)->isDroppable()
#define ONLY_USED_BY_MARKERS_OR_DROPPABLE(i) onlyUsedByLifetimeMarkersOrDroppableInsts(i)

static auto isUserPure(User *usr, AllocaInst *ai, std::vector<Type *> &useTypes) -> bool
{
	if (auto *li = dyn_cast<LoadInst>(usr)) {
		useTypes.push_back(li->getType());
		return !li->isVolatile();
	}
	if (auto *si = dyn_cast<StoreInst>(usr)) {
		useTypes.push_back(si->getValueOperand()->getType());
		/* Don't allow a store OF the U, only INTO the U */
		return !si->isVolatile() && si->getValueOperand() != ai;
	}
	if (auto *ii = dyn_cast<IntrinsicInst>(usr)) {
		return IS_LIFETIME_START_OR_END(ii) || IS_DROPPABLE(ii);
	}
	if (auto *gepi = dyn_cast<GetElementPtrInst>(usr)) {
		return gepi->hasAllZeroIndices() && ONLY_USED_BY_MARKERS_OR_DROPPABLE(gepi);
	}
	if (auto *asci = dyn_cast<AddrSpaceCastInst>(usr)) {
		return onlyUsedByLifetimeMarkers(asci);
	}
	/* All other cases are not safe*/
	return false;
}

static auto isPromotable(AllocaInst *ai) -> bool
{
	const auto &DL = ai->getModule()->getDataLayout();
	std::vector<Type *> useTypes;

	if (!ai->getAllocatedType()->isIntOrPtrTy() ||
	    std::any_of(ai->users().begin(), ai->users().end(),
			[&](User *usr) { return !isUserPure(usr, ai, useTypes); }))
		return false;
	return std::ranges::all_of(useTypes, [&ai, &DL](auto *typ) {
		return DL.getTypeAllocSize(typ) == DL.getTypeAllocSize(ai->getAllocatedType()) &&
		       typ->isIntOrPtrTy();
	});
}

static auto introduceAllocaCasts(AllocaInst *ai) -> bool
{
	for (auto *usr : ai->users()) {
		if (auto *li = dyn_cast<LoadInst>(usr)) {
			if (li->getType() == ai->getAllocatedType())
				continue;
			auto *prevType = li->getType();
			li->mutateType(ai->getAllocatedType());
			auto opc = CastInst::getCastOpcode(li, false, prevType, false);
			auto *res = CastInst::Create(opc, li, prevType, "",
						     std::next(li->getIterator()));
			replaceUsesWithIf(li, res, [&](Use &use) {
				auto *userInst = dyn_cast<Instruction>(use.getUser());
				return userInst && userInst != res;
			});
		} else if (auto *si = dyn_cast<StoreInst>(usr)) {
			if (si->getValueOperand()->getType() == ai->getAllocatedType())
				continue;
			auto opc = CastInst::getCastOpcode(si->getValueOperand(), false,
							   ai->getAllocatedType(), false);
			auto *res = CastInst::Create(opc, si->getValueOperand(),
						     ai->getAllocatedType(), "", si->getIterator());
			si->setOperand(0, res);
		}
	}
	return false;
}

static auto introduceCasts(Function &F) -> bool
{
	auto &eBB = F.getEntryBlock();
	auto modified = false;

	auto changed = true;
	while (changed) {
		changed = false;
		/* Find allocas that are safe to promote (skip terminator) */
		for (auto it = eBB.begin(), end = --eBB.end(); it != end; ++it) {
			/* An alloca is promotable if all its users are "pure" */
			if (auto *ai = dyn_cast<AllocaInst>(it)) {
				if (isPromotable(ai)) {
					changed |= introduceAllocaCasts(ai);
					modified |= changed;
				}
			}
		}
	}
	return modified;
}

auto EliminateCastsPass::run(Function &F, FunctionAnalysisManager & /*FAM*/) -> PreservedAnalyses
{
	auto modified = introduceCasts(F);
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::none();
}
