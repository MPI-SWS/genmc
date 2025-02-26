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

#include "EliminateCastsPass.hpp"
#include "Static/LLVMUtils.hpp"
#include "config.h"

#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/PromoteMemToReg.h>

using namespace llvm;

#if LLVM_VERSION_MAJOR >= 12
#define IS_LIFETIME_START_OR_END(i) (i)->isLifetimeStartOrEnd()
#if LLVM_VERSION_MAJOR >= 11
#define IS_DROPPABLE(i) (i)->isDroppable()
#else
#define IS_DROPPABLE(i) isDroppable(i)
#endif
#define ONLY_USED_BY_MARKERS_OR_DROPPABLE(i) onlyUsedByLifetimeMarkersOrDroppableInsts(i)
#else
#define IS_LIFETIME_START_OR_END(i)                                                                \
	(i->getIntrinsicID() != Intrinsic::lifetime_start &&                                       \
	 i->getIntrinsicID() != Intrinsic::lifetime_end)
#define IS_DROPPABLE(i) false
#define ONLY_USED_BY_MARKERS_OR_DROPPABLE(i) onlyUsedByLifetimeMarkers(i)
#endif

#if LLVM_VERSION_MAJOR >= 11
#define GET_INST_ALIGN(i) i->getAlign()
#else
#define GET_INST_ALIGN(i) i->getAlignment()
#endif

#define GET_INST_SYNC_SCOPE(i) i->getSyncScopeID()

/* Opaque pointers should render this pass obsolete */
#if LLVM_VERSION_MAJOR <= 14

static auto haveSameSizePointees(const Type *p1, const Type *p2, const DataLayout &DL) -> bool
{
	const auto *pTy1 = dyn_cast<PointerType>(p1);
	const auto *pTy2 = dyn_cast<PointerType>(p2);

	return pTy1 && pTy2 &&
	       DL.getTypeAllocSize(pTy1->getElementType()) ==
		       DL.getTypeAllocSize(pTy2->getElementType());
}

static auto isUnpromotablePureHelper(User *u, std::vector<Instruction *> &aliases) -> bool;

/* Return whether u is pure; if it is, it will be returned in ALIASES with its aliases */
static auto isUserPure(User *u, std::vector<Instruction *> &aliases) -> bool
{
	/* We only allow direct, non-volatile loads and stores, as well as "pure"
	 * bitcasts (only used by direct, non-volatile loads and stores).
	 * (Atomicity qualifiers are not important as they have no meaning for local
	 * allocas.) */
	if (auto *li = dyn_cast<LoadInst>(u)) {
		return !li->isVolatile();
	}
	if (auto *si = dyn_cast<StoreInst>(u)) {
		/* Don't allow a store OF the U, only INTO the U */
		return !si->isVolatile() && si->getOperand(0) != aliases.back();
	}
	if (auto *ii = dyn_cast<IntrinsicInst>(u)) {
		return IS_LIFETIME_START_OR_END(ii) || IS_DROPPABLE(ii);
	}
	if (auto *bci = dyn_cast<BitCastInst>(u)) {
		if (ONLY_USED_BY_MARKERS_OR_DROPPABLE(bci))
			return true;
		if (!haveSameSizePointees(bci->getSrcTy(), bci->getDestTy(),
					  bci->getModule()->getDataLayout()))
			return false;
		aliases.push_back(bci);
		return isUnpromotablePureHelper(bci, aliases);
	}
	if (auto *gepi = dyn_cast<GetElementPtrInst>(u)) {
		return gepi->hasAllZeroIndices() && ONLY_USED_BY_MARKERS_OR_DROPPABLE(gepi);
	}
	if (auto *asci = dyn_cast<AddrSpaceCastInst>(u)) {
		return onlyUsedByLifetimeMarkers(asci);
	}
	/* All other cases are not safe*/
	return false;
}

static auto isUnpromotablePureHelper(User *i, std::vector<Instruction *> &aliases) -> bool
{
	return std::all_of(i->users().begin(), i->users().end(),
			   [&aliases](User *u) { return isUserPure(u, aliases); });
}

static auto isUnpromotablePure(Instruction *i, std::vector<Instruction *> &aliases) -> bool
{
	aliases.push_back(i);
	auto pure = isUnpromotablePureHelper(i, aliases);
	if (!pure)
		aliases.clear();
	return pure;
}

static auto isEliminableCastPair(const CastInst *CI1, const CastInst *CI2, const DataLayout &DL)
	-> Instruction::CastOps
{
	Type *SrcTy = CI1->getSrcTy();
	Type *MidTy = CI1->getDestTy();
	Type *DstTy = CI2->getDestTy();

	Instruction::CastOps firstOp = CI1->getOpcode();
	Instruction::CastOps secondOp = CI2->getOpcode();
	Type *SrcIntPtrTy = SrcTy->isPtrOrPtrVectorTy() ? DL.getIntPtrType(SrcTy) : nullptr;
	Type *MidIntPtrTy = MidTy->isPtrOrPtrVectorTy() ? DL.getIntPtrType(MidTy) : nullptr;
	Type *DstIntPtrTy = DstTy->isPtrOrPtrVectorTy() ? DL.getIntPtrType(DstTy) : nullptr;
	unsigned Res = CastInst::isEliminableCastPair(firstOp, secondOp, SrcTy, MidTy, DstTy,
						      SrcIntPtrTy, MidIntPtrTy, DstIntPtrTy);

	/* We don't want to form an inttoptr or ptrtoint that converts to an integer
	 * type that differs from the pointer size */
	if ((Res == Instruction::IntToPtr && SrcTy != DstIntPtrTy) ||
	    (Res == Instruction::PtrToInt && DstTy != SrcIntPtrTy))
		Res = 0;

	return Instruction::CastOps(Res);
}

static auto isLoadCastedFromSameSrcType(const LoadInst *li, CastInst &ci) -> Value *
{
	auto *M = li->getModule();
	auto &DL = M->getDataLayout();
	auto *typ = dyn_cast<PointerType>(ci.getOperand(0)->getType());
	BUG_ON(!typ);

	/* We capture two basic cases for now:
	 * 1) LI loading from a bitcast (const or not), and
	 * 2) LI loading from a constexpr */
	if (auto *lci = dyn_cast<BitCastInst>(li->getPointerOperand())) {
		auto *srcTy = lci->getSrcTy();
		auto *dstTy = lci->getDestTy();
		if (srcTy == typ && haveSameSizePointees(srcTy, dstTy, DL))
			return lci->getOperand(0);
		return nullptr;
	}
	if (auto *lce = dyn_cast<ConstantExpr>(li->getPointerOperand())) {
		if (lce->isCast() && lce->getOpcode() == Instruction::CastOps::BitCast) {
			auto *srcTy = lce->getOperand(0)->getType();
			if (srcTy == typ->getElementType())
				return lce->getOperand(0);
		}
		return nullptr;
	}
	return nullptr;
}

static void replaceAndMarkDelete(Instruction *toDel, Value *toRepl,
				 VSet<Instruction *> *deleted = nullptr)
{
	if (toRepl && toDel->hasNUsesOrMore(1))
		toDel->replaceAllUsesWith(toRepl);
	BUG_ON(toDel->hasNUsesOrMore(1));
	toDel->eraseFromParent();
	if (deleted)
		deleted->insert(toDel);
}

static auto castToType(Value *a, Type *typ, Instruction *insertBefore) -> Instruction *
{
	if (a->getType()->isPointerTy() && typ->isPointerTy())
		return new BitCastInst(a, typ, "", insertBefore);
	if (typ->isPointerTy())
		return new IntToPtrInst(a, typ, "", insertBefore);

	BUG_ON(!a->getType()->isPointerTy());
	return new PtrToIntInst(a, typ, "", insertBefore);
}

static void transformStoredBitcast(CastInst &ci, StoreInst *si, VSet<Instruction *> &deleted)
{
	auto *origPTy = dyn_cast<PointerType>(ci.getOperand(0)->getType());
	BUG_ON(!origPTy);

	/* Case 1: SI's value comes from a load that (a) comes from a similar cast, and
	 * (b) the load result is only used in SI, then can simply get rid of the load */
	auto *li = dyn_cast<LoadInst>(si->getValueOperand());
	Value *lsrc = nullptr;
	if (li && (lsrc = isLoadCastedFromSameSrcType(li, ci)) && li->hasNUses(1)) {
		/* If the load came from cast operand, check if we have to delete the cast too */
		if (auto *lci = dyn_cast<BitCastInst>(li->getPointerOperand()))
			if (lci->hasNUses(0))
				replaceAndMarkDelete(lci, nullptr);

		if (auto *clsrc = dyn_cast<Constant>(lsrc)) /* ensure type-compatibility */
			lsrc = ConstantExpr::getPointerCast(clsrc, origPTy);
		auto *load = new LoadInst(origPTy->getElementType(), lsrc, li->getName(),
					  li->isVolatile(), GET_INST_ALIGN(li), li->getOrdering(),
					  GET_INST_SYNC_SCOPE(li), si);
		auto *store = new StoreInst(load, ci.getOperand(0), si->isVolatile(),
					    GET_INST_ALIGN(si), si->getOrdering(),
					    GET_INST_SYNC_SCOPE(si), si);

		replaceAndMarkDelete(si, store);
		replaceAndMarkDelete(li, load);
		return;
	}

	/* Case 2: SI's value comes from a different instruction. It's safe to cast that
	 * value to CI's source type because, instead of casting the type of the address,
	 * we will now cast the type of the value (this _has_ to be a pointer type) */
	auto *cast = castToType(si->getValueOperand(), origPTy->getElementType(), si);
	auto *store = new StoreInst(cast, ci.getOperand(0), si->isVolatile(), GET_INST_ALIGN(si),
				    si->getOrdering(), GET_INST_SYNC_SCOPE(si), si);
	replaceAndMarkDelete(si, store);
}

static void transformLoadedBitcast(CastInst &ci, LoadInst *li, VSet<Instruction *> &deleted)
{

	auto *load = new LoadInst(dyn_cast<PointerType>(ci.getSrcTy())->getElementType(),
				  ci.getOperand(0), li->getName(), li->isVolatile(),
				  GET_INST_ALIGN(li), li->getOrdering(), GET_INST_SYNC_SCOPE(li),
				  li);
	auto *cast = castToType(load, li->getType(), li);
	replaceAndMarkDelete(li, cast, &deleted);
}

/* Performs some common transformations on the cast and deletes it, if necessary.
 * Returns true if the transformation succeeded (and the caller should delete CI),
 * and sets TRANSFORMED to its replacement. */
static auto commonCastTransforms(CastInst &ci, std::vector<Instruction *> &aliases,
				 VSet<Instruction *> &deleted) -> bool
{
	const DataLayout &DL = ci.getModule()->getDataLayout();
	Value *src = ci.getOperand(0);
	Type *typ = ci.getType();

	/* Eliminate zero-use casts */
	if (ci.hasNUses(0)) {
		replaceAndMarkDelete(&ci, nullptr, &deleted);
		return true;
	}

	/* Eliminate same-type casts */
	if (ci.getSrcTy() == ci.getDestTy()) {
		replaceAndMarkDelete(&ci, ci.getOperand(0), &deleted);
		return true;
	}

	/* Try to eliminate a cast of a cast */
	if (auto *csrc = dyn_cast<CastInst>(src)) { /* A->B->C cast */
		if (Instruction::CastOps newOpc = isEliminableCastPair(csrc, &ci, DL)) {
			/* The first cast (csrc) is eliminable so we need to fix up or replace
			 * the second cast (ci). csrc will then have a good chance of being dead */
			auto *res = CastInst::Create(newOpc, csrc->getOperand(0), typ);
			/* Point debug users of the dying cast to the new one. */
			// if (CSrc->hasOneUse())
			// 	replaceAllDbgUsesWith(*CSrc, *Res, ci, DT);
			res->takeName(&ci);
			ci.getParent()->getInstList().insert(ci.getIterator(), res);
			replaceAndMarkDelete(&ci, res, &deleted);
			aliases.push_back(res);
			return true;
		}
	}

	/* Try to eliminate unnecessary casts */
	if (std::all_of(ci.user_begin(), ci.user_end(), [&](const User *u) {
		    return isa<LoadInst>(u) ||
			   (isa<StoreInst>(u) && &ci != dyn_cast<StoreInst>(u)->getValueOperand());
	    })) {
		/* We know there is at least one use because otherwise the
		 * bitcast would have been deleted previously */
		auto *cusr = *ci.user_begin();
		/* dyn_cast_or_null<> because we might delete users */
		if (auto *si = dyn_cast_or_null<StoreInst>(cusr))
			transformStoredBitcast(ci, si, deleted);
		else if (auto *li = dyn_cast_or_null<LoadInst>(cusr))
			transformLoadedBitcast(ci, li, deleted);
		else
			BUG();
		/* We will not transform all users in one go because
		 * messing with one user might break iterators */
		return true;
	}
	return false;
}

static auto eliminateAllocaCasts(std::vector<Instruction *> &aliases) -> bool
{
	if (aliases.empty())
		return false;

	bool eliminated = false;
	bool changed = true;

	VSet<Instruction *> deleted;
	while (changed) {
		changed = false;

		/* Skip the allocation itself */
		BUG_ON(!llvm::isa<AllocaInst>(aliases[0]));
		for (auto i = 1u; i < aliases.size(); i++) {
			/* If we have already deleted the instruction, skip */
			if (deleted.count(aliases[i]))
				continue;

			auto *bci = llvm::dyn_cast<BitCastInst>(aliases[i]);
			BUG_ON(!bci);

			bool transformed = commonCastTransforms(*bci, aliases, deleted);
			if (transformed) {
				changed = true;
				eliminated = true;
			}
		}
	}
	return eliminated;
}

static auto eliminateCasts(Function &F, DominatorTree &DT) -> bool
{
	auto &eBB = F.getEntryBlock();
	bool changed = true;

	while (changed) {
		changed = false;
		/* Find allocas that are safe to promote (skip terminator) */
		for (auto it = eBB.begin(), e = --eBB.end(); it != e; ++it) {
			/* An alloca is promotable if all its users are "pure" */
			if (auto *ai = dyn_cast<AllocaInst>(it)) {
				std::vector<Instruction *> aliases;
				if (isUnpromotablePure(ai, aliases)) {
					changed = eliminateAllocaCasts(aliases);
				}
			}
		}
	}
	return changed;
}
#else  /* LLVM_VERSION_MAJOR <= 14 */
static auto isUserPure(User *u, AllocaInst *ai, const DataLayout &DL, std::vector<Type *> &useTypes)
	-> bool
{
	if (auto *li = dyn_cast<LoadInst>(u)) {
		useTypes.push_back(li->getType());
		return !li->isVolatile();
	} else if (auto *si = dyn_cast<StoreInst>(u)) {
		useTypes.push_back(si->getValueOperand()->getType());
		/* Don't allow a store OF the U, only INTO the U */
		return !si->isVolatile() && si->getValueOperand() != ai;
	} else if (auto *ii = dyn_cast<IntrinsicInst>(u)) {
		return IS_LIFETIME_START_OR_END(ii) || IS_DROPPABLE(ii);
	} else if (auto *gepi = dyn_cast<GetElementPtrInst>(u)) {
		return gepi->hasAllZeroIndices() && ONLY_USED_BY_MARKERS_OR_DROPPABLE(gepi);
	} else if (auto *asci = dyn_cast<AddrSpaceCastInst>(u)) {
		return onlyUsedByLifetimeMarkers(asci);
	}
	/* All other cases are not safe*/
	return false;
}

static auto isPromotable(AllocaInst *ai) -> bool
{
	auto &DL = ai->getModule()->getDataLayout();
	std::vector<Type *> useTypes;

	if (!ai->getAllocatedType()->isIntOrPtrTy() ||
	    std::any_of(ai->users().begin(), ai->users().end(),
			[&](User *u) { return !isUserPure(u, ai, DL, useTypes); }))
		return false;
	return std::all_of(useTypes.begin(), useTypes.end(), [&ai, &DL](auto *typ) {
		return DL.getTypeAllocSize(typ) == DL.getTypeAllocSize(ai->getAllocatedType()) &&
		       typ->isIntOrPtrTy();
	});
}

static auto introduceAllocaCasts(AllocaInst *ai) -> bool
{
	for (auto *u : ai->users()) {
		if (auto *li = dyn_cast<LoadInst>(u)) {
			if (li->getType() == ai->getAllocatedType())
				continue;
			auto *prevType = li->getType();
			li->mutateType(ai->getAllocatedType());
			auto opc = CastInst::getCastOpcode(li, false, prevType, false);
			auto *res = CastInst::Create(opc, li, prevType, "",
						     li->getNextNonDebugInstruction());
			replaceUsesWithIf(li, res, [&](Use &u) {
				auto *us = dyn_cast<Instruction>(u.getUser());
				return us && us != res;
			});
		} else if (auto *si = dyn_cast<StoreInst>(u)) {
			if (si->getValueOperand()->getType() == ai->getAllocatedType())
				continue;
			auto opc = CastInst::getCastOpcode(si->getValueOperand(), false,
							   ai->getAllocatedType(), false);
			auto *res = CastInst::Create(opc, si->getValueOperand(),
						     ai->getAllocatedType(), "", si);
			si->setOperand(0, res);
		}
	}
	return false;
}

static auto introduceCasts(Function &F, DominatorTree &DT) -> bool
{
	auto &eBB = F.getEntryBlock();
	auto modified = false;

	auto changed = true;
	while (changed) {
		changed = false;
		/* Find allocas that are safe to promote (skip terminator) */
		for (auto it = eBB.begin(), e = --eBB.end(); it != e; ++it) {
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
#endif /* LLVM_VERSION_MAJOR <= 14 */

auto EliminateCastsPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
#if LLVM_VERSION_MAJOR <= 14
	auto modified = eliminateCasts(F, DT);
#else
	auto modified = introduceCasts(F, DT);
#endif
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::none();
}
