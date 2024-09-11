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

#include "config.h"

#include "ADT/VSet.hpp"
#include "LoopUnrollPass.hpp"
#include "Static/Transforms/DeclareInternalsPass.hpp"
#include "Support/Error.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>

#if LLVM_VERSION_MAJOR < 8
typedef llvm::TerminatorInst TerminatorInst;
#else
typedef llvm::Instruction TerminatorInst;
#endif

#if LLVM_VERSION_MAJOR >= 11
#define GET_LOADINST_ARG(val)
#else
#define GET_LOADINST_ARG(val) (val)->getType()->getPointerElementType(),
#endif

#if LLVM_VERSION_MAJOR >= 11
#define GET_BOUND_ALLOCA_ALIGN_ARG(val) llvm::Align(val)
#else
#define GET_BOUND_ALLOCA_ALIGN_ARG(val) val
#endif

using namespace llvm;

/*
 * Returns a PHI node the only purpose of which is to serve as the
 * bound variable for this loop.  It does not set up the arguments for
 * the PHI node.
 */
auto createBoundInit(Loop *l) -> PHINode *
{
	Function *parentFun = (*l->block_begin())->getParent();
	Type *int32Typ = Type::getInt32Ty(parentFun->getContext());

	PHINode *bound = PHINode::Create(int32Typ, 0, "bound.val", &*l->getHeader()->begin());
	return bound;
}

/*
 * Returns an instruction which decrements the bound variable for this loop (BOUNDVAL).
 * The returned value should be later checked in order to bound the loop.
 */
auto createBoundDecrement(Loop *l, PHINode *boundVal) -> BinaryOperator *
{
	Function *parentFun = (*l->block_begin())->getParent();
	Type *int32Typ = Type::getInt32Ty(parentFun->getContext());

	Value *minusOne = ConstantInt::get(int32Typ, -1, true);
	Instruction *pt = &*l->getHeader()->getFirstInsertionPt();
	return BinaryOperator::CreateNSW(Instruction::Add, boundVal, minusOne,
					 l->getName() + ".bound.dec", pt);
}

void addBoundCmpAndSpinEndBefore(Loop *l, PHINode *val, BinaryOperator *decVal)
{
	Function *parentFun = (*l->block_begin())->getParent();
	Function *endLoopFun = parentFun->getParent()->getFunction("__VERIFIER_kill_thread");
	Type *int32Typ = Type::getInt32Ty(parentFun->getContext());

	Value *zero = ConstantInt::get(int32Typ, 0);
	Value *cmp =
		new ICmpInst(decVal, ICmpInst::ICMP_EQ, val, zero, l->getName() + ".bound.cmp");

	BUG_ON(!endLoopFun);
	CallInst::Create(endLoopFun, {cmp}, "", decVal);
}

auto LoopUnrollPass::run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR,
			 LPMUpdater &U) -> PreservedAnalyses
{
	if (!shouldUnroll(&L))
		return PreservedAnalyses::all();

	PHINode *val = createBoundInit(&L);
	BinaryOperator *dec = createBoundDecrement(&L, val);
	Type *int32Typ = Type::getInt32Ty((*L.block_begin())->getParent()->getContext());

	/* Adjust incoming values for the bound variable */
	for (BasicBlock *bb : predecessors(L.getHeader()))
		val->addIncoming(L.contains(bb) ? (Value *)dec
						: (Value *)ConstantInt::get(int32Typ, unrollDepth_),
				 bb);

	/* Finally, compare the bound and block if it reaches zero */
	addBoundCmpAndSpinEndBefore(&L, val, dec);
	return PreservedAnalyses::none();
}
