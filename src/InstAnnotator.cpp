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

#include "InstAnnotator.hpp"
#include "InterpreterEnumAPI.hpp"
#include "LLVMUtils.hpp"
#include "ModuleInfo.hpp"
#include "SExprVisitor.hpp"
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Module.h>

using namespace llvm;

void InstAnnotator::reset()
{
	statusMap.clear();
	annotMap.clear();
	return;
}

Value *InstAnnotator::getAnnotMapKey(Value *i) const { return i; }

const InstAnnotator::IRExpr *InstAnnotator::getAnnot(Instruction *i)
{
	return annotMap[getAnnotMapKey(i)].get();
}

InstAnnotator::IRExprUP InstAnnotator::releaseAnnot(Instruction *i)
{
	return std::move(annotMap[getAnnotMapKey(i)]);
}

void InstAnnotator::setAnnot(Instruction *i, InstAnnotator::IRExprUP annot)
{
	annotMap[getAnnotMapKey(i)] = std::move(annot);
}

InstAnnotator::IRExprUP InstAnnotator::generateOperandExpr(Value *op)
{
	if (auto *c = dyn_cast<Constant>(op)) {
		if (isa<UndefValue>(c)) {
			return ConcreteExpr<Value *>::create(c->getType()->getIntegerBitWidth(),
							     42);
		} else if (c->getType()->isIntegerTy()) {
			auto v = c->getUniqueInteger();
			return ConcreteExpr<Value *>::create(v.getBitWidth(), v.getLimitedValue());
		} else if (isa<ConstantPointerNull>(c)) {
			BUG_ON(!op->getType()->isPointerTy());
			auto iIt =
				std::find_if(op->user_begin(), op->user_end(), [](const User *u) {
					return llvm::isa<Instruction>(u);
				});
			BUG_ON(iIt == op->user_end());
			auto *mod =
				dyn_cast<Instruction>(*iIt)->getParent()->getParent()->getParent();
			auto &DL = mod->getDataLayout();
			return ConcreteExpr<Value *>::create(
				DL.getTypeAllocSizeInBits(op->getType()), 0);
		}
		ERROR("Only integer and null constants currently allowed in assume() "
		      "expressions.\n");
	}
	BUG_ON(!isa<Instruction>(op) && !isa<Argument>(op));
	Module *mod = nullptr;
	if (auto *i = dyn_cast<Instruction>(op))
		mod = i->getParent()->getParent()->getParent();
	else if (auto *a = dyn_cast<Argument>(op))
		mod = a->getParent()->getParent();
	auto &DL = mod->getDataLayout();
	return RegisterExpr<Value *>::create(DL.getTypeAllocSizeInBits(op->getType()),
					     getAnnotMapKey(op));
}

InstAnnotator::IRExprUP InstAnnotator::generateInstExpr(Instruction *curr)
{
	/*
	 * Next, we try to generate an annotation for a whole bunch of instructions,
	 * apart from function calls, memory instructions, and some pointer casts.
	 * For the cases we do not handle, we simply return false.
	 */

#define HANDLE_INST(op, ...)                                                                       \
	case Instruction::op:                                                                      \
		return op##Expr<Value *>::create(__VA_ARGS__)

	switch (curr->getOpcode()) {
		HANDLE_INST(ZExt, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)));
		HANDLE_INST(SExt, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)));
		HANDLE_INST(Trunc, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)));

		HANDLE_INST(Select, generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)),
			    generateOperandExpr(curr->getOperand(2)));

		HANDLE_INST(Add, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Sub, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Mul, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(UDiv, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(SDiv, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(URem, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(And, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Or, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Xor, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Shl, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(LShr, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(AShr, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));

	/* Special case for extracts --- only CAS extracts allowed; */
	case Instruction::ExtractValue: {
		auto *extract = dyn_cast<ExtractValueInst>(curr);
		auto *cas = extractsFromCAS(extract);
		if (!cas)
			break;
		/* Hack: If it extracts the value read, just return a register expr;
		 * the types won't match but we don't care about that since we won't
		 * annotate above the CAS anyway */
		if (*extract->idx_begin() == 0)
			return generateOperandExpr(cas);
		return EqExpr<Value *>::create(generateOperandExpr(cas),
					       generateOperandExpr(cas->getCompareOperand()));
	}

	case Instruction::BitCast: {
		return generateOperandExpr(curr->getOperand(0));
	}

	case Instruction::PtrToInt:
	case Instruction::IntToPtr: {
		auto *castinst = dyn_cast<CastInst>(curr);
		auto *mod = curr->getParent()->getParent()->getParent();
		auto &DL = mod->getDataLayout();
		auto srcSize = DL.getTypeAllocSize(castinst->getSrcTy());
		auto dstSize = DL.getTypeAllocSize(castinst->getDestTy());
		if (srcSize > dstSize) {
			return TruncExpr<Value *>::create(
				castinst->getDestTy()->getPrimitiveSizeInBits(),
				generateOperandExpr(castinst->getOperand(0)));
		} else if (srcSize < dstSize) {
			return ZExtExpr<Value *>::create(
				castinst->getDestTy()->getPrimitiveSizeInBits(),
				generateOperandExpr(castinst->getOperand(0)));
		} else {
			return generateOperandExpr(castinst->getOperand(0));
		}
	}

	case Instruction::ICmp: {
		llvm::CmpInst *cmpi = llvm::dyn_cast<llvm::CmpInst>(curr);
		switch (cmpi->getPredicate()) {
		case CmpInst::ICMP_EQ:
			return EqExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
						       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_NE:
			return NeExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
						       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_UGT:
			return UgtExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_UGE:
			return UgeExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_ULT:
			return UltExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_ULE:
			return UleExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SGT:
			return SgtExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SGE:
			return SgeExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SLT:
			return SltExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SLE:
			return SleExpr<Value *>::create(generateOperandExpr(curr->getOperand(0)),
							generateOperandExpr(curr->getOperand(1)));
		default:
			/* Unsupported compare predicate; quit */
			break;
		}
	}
	default:
		/* Don't know how to annotate this */
		break;
	}
	return ConcreteExpr<Value *>::createTrue();
}

std::vector<Instruction *> getNextOrBranchSuccessors(Instruction *i)
{
	std::vector<Instruction *> succs;

	/*
	 * Do not return any successors for points beyond which we
	 * cannot annotate (even though the CFG does have edges we can follow)
	 */
	if (isa<LoadInst>(i) || hasSideEffects(i))
		return succs;
	if (auto *ci = dyn_cast<CallInst>(i)) {
		if (isAssumeFunction(getCalledFunOrStripValName(*ci)) ||
		    isErrorFunction(getCalledFunOrStripValName(*ci)))
			return succs;
	}

	if (i->getNextNode())
		succs.push_back(i->getNextNode());
	else if (auto *bi = dyn_cast<BranchInst>(i)) {
		if (bi->isUnconditional()) {
			succs.push_back(&*bi->getSuccessor(0)->begin());
		} else {
			succs.push_back(&*bi->getSuccessor(0)->begin());
			succs.push_back(&*bi->getSuccessor(1)->begin());
		}
	}
	return succs;
}

InstAnnotator::IRExprUP InstAnnotator::propagateAnnotFromSucc(Instruction *curr, Instruction *succ)
{
	auto succExp = getAnnot(succ)->clone();
	auto substitutor = SExprRegSubstitutor<Value *>();

	PHINode *succPhi = nullptr;
	for (auto iit = succ;
	     (succPhi = dyn_cast<PHINode>(iit)) && curr->getParent() != succ->getParent();
	     iit = iit->getNextNode()) {
		auto phiOp =
			generateOperandExpr(succPhi->getIncomingValueForBlock(curr->getParent()));
		succExp = substitutor.substitute(succExp.get(), getAnnotMapKey(iit), phiOp.get());
	}

	if (isa<BranchInst>(curr) || (isa<PHINode>(curr) && curr->getParent() == succ->getParent()))
		return succExp;

	auto currOp = generateInstExpr(curr);
	return substitutor.substitute(succExp.get(), getAnnotMapKey(curr), currOp.get());
}

void InstAnnotator::annotateDFS(Instruction *curr)
{
	statusMap[curr] = InstAnnotator::entered;

	std::vector<Instruction *> succs = getNextOrBranchSuccessors(curr);
	BUG_ON(succs.size() > 2);

	for (auto *succ : succs) {
		if (statusMap[succ] == InstAnnotator::unseen)
			annotateDFS(succ);
		else if (statusMap[succ] == InstAnnotator::entered)
			setAnnot(succ, ConcreteExpr<Value *>::createTrue());
	}

	statusMap[curr] = InstAnnotator::left;

	/* If we cannot get past this instruction, return either TRUE or the assumed expression */
	if (succs.empty()) {
		if (auto *ci = dyn_cast<CallInst>(curr)) {
			if (isAssumeFunction(getCalledFunOrStripValName(*ci))) {
				setAnnot(curr, generateOperandExpr(ci->getOperand(0)));
				return;
			}
		}
		setAnnot(curr, ConcreteExpr<Value *>::createTrue());
		return;
	}
	/* If this is a branch instruction, create a select expression */
	if (succs.size() == 2) {
		auto cond = dyn_cast<BranchInst>(curr)->getCondition();
		auto regExp = RegisterExpr<Value *>::create(
			cond->getType()->getPrimitiveSizeInBits(), getAnnotMapKey(cond));
		setAnnot(curr, SelectExpr<Value *>::create(std::move(regExp),
							   propagateAnnotFromSucc(curr, succs[0]),
							   propagateAnnotFromSucc(curr, succs[1])));
		return;
	}
	/* At this point we know there is just one successor: substitute */
	setAnnot(curr, propagateAnnotFromSucc(curr, succs[0]));
	return;
}

InstAnnotator::IRExprUP InstAnnotator::annotate(Instruction *curr)
{
	/* Reset DFS data + prepare new exploration */
	reset();
	for (auto &i : instructions(curr->getParent()->getParent()))
		statusMap[&i] = InstAnnotator::unseen;

	/* The load annotation will be the expression from its successor to the assume */
	BUG_ON(!isa<LoadInst>(curr) && !isa<AtomicCmpXchgInst>(curr));
	annotateDFS(curr->getNextNode());
	return releaseAnnot(curr->getNextNode());
}

InstAnnotator::IRExprUP InstAnnotator::annotateBBCond(BasicBlock *bb,
						      BasicBlock *pred /* = nullptr */)
{
	auto *bi = dyn_cast<BranchInst>(bb->getTerminator());
	if (!bi)
		return ConcreteExpr<Value *>::createFalse();
	if (bi->isUnconditional())
		return ConcreteExpr<Value *>::createTrue();

	/* Reset data */
	reset();

	/* Propagate jump condition backwards to the beginning of the basic block */
	setAnnot(bi, generateOperandExpr(bi->getCondition()));
	for (auto irit = ++bb->rbegin(); irit != bb->rend(); ++irit) {
		annotMap[&*irit] = irit->mayReadOrWriteMemory()
					   ? ConcreteExpr<Value *>::createFalse()
					   : propagateAnnotFromSucc(&*irit, irit->getNextNode());
	}

	/* If a predecessor is given substitute Φ values too */
	if (pred)
		return propagateAnnotFromSucc(pred->getTerminator(), &*bb->begin());
	return releaseAnnot(&*bb->begin());
}

std::vector<Instruction *> getNextOrBranchSuccessorsInLoop(Instruction *i,
							   const VSet<BasicBlock *> &backedgePaths,
							   Loop *l,
							   const VSet<llvm::Function *> *cleanSet)
{
	std::vector<Instruction *> succs;

	/*
	 * The points beyond which we cannot annotate are different when it comes
	 * to backedges: CASes are ignored (they should fail and will be checked later),
	 * while loop headers or blocks outside the loop have no successors
	 */
	if (!backedgePaths.count(i->getParent()) || i == &*l->getHeader()->begin())
		return succs;

	/* Sanity checks for side-effects: only CASes and effect-free calls are allowed */
	auto *ci = llvm::dyn_cast<CallInst>(i);
	auto inCleanSet = ci && cleanSet && cleanSet->count(ci->getCalledFunction());
	BUG_ON(hasSideEffects(i) && !isa<AtomicCmpXchgInst>(i) && !inCleanSet);

	/* Find successors */
	if (i->getNextNode())
		succs.push_back(i->getNextNode());
	else if (auto *bi = dyn_cast<BranchInst>(i)) {
		if (bi->isUnconditional()) {
			succs.push_back(&*bi->getSuccessor(0)->begin());
		} else {
			succs.push_back(&*bi->getSuccessor(0)->begin());
			succs.push_back(&*bi->getSuccessor(1)->begin());
		}
	}
	return succs;
}

InstAnnotator::IRExprUP
InstAnnotator::propagateAnnotFromSuccInLoop(Instruction *curr, Instruction *succ,
					    const VSet<BasicBlock *> &backedgePaths, Loop *l)
{
	auto succExp = getAnnot(succ)->clone();
	auto substitutor = SExprRegSubstitutor<Value *>();

	PHINode *succPhi = nullptr;
	for (auto iit = succ;
	     (succPhi = dyn_cast<PHINode>(iit)) && curr->getParent() != succ->getParent();
	     iit = iit->getNextNode()) {
		auto phiOp =
			generateOperandExpr(succPhi->getIncomingValueForBlock(curr->getParent()));
		succExp = substitutor.substitute(succExp.get(), getAnnotMapKey(iit), phiOp.get());
	}

	if (isa<BranchInst>(curr) ||
	    (isa<PHINode>(curr) && curr->getParent() == succ->getParent()) ||
	    isa<AtomicCmpXchgInst>(curr) || isa<ExtractValueInst>(curr) || isa<LoadInst>(curr) ||
	    isa<CallInst>(curr))
		return succExp;

	/* Transform assume()s into disjunctions */
	if (auto *ci = dyn_cast<CallInst>(curr)) {
		BUG_ON(!isAssumeFunction(getCalledFunOrStripValName(*ci)));
		return ConjunctionExpr<Value *>::create(
			std::move(generateOperandExpr(ci->getOperand(0))), std::move(succExp));
	}

	auto currOp = generateInstExpr(curr);
	return substitutor.substitute(succExp.get(), getAnnotMapKey(curr), currOp.get());
}

void InstAnnotator::annotateCASWithBackedgeCondDFS(Instruction *curr,
						   const VSet<BasicBlock *> &backedgePaths, Loop *l,
						   const VSet<llvm::Function *> *cleanSet)
{
	statusMap[curr] = InstAnnotator::entered;

	std::vector<Instruction *> succs =
		getNextOrBranchSuccessorsInLoop(curr, backedgePaths, l, cleanSet);
	BUG_ON(succs.size() > 2);

	for (auto *succ : succs) {
		if (statusMap[succ] == InstAnnotator::unseen)
			annotateCASWithBackedgeCondDFS(succ, backedgePaths, l, cleanSet);
		else if (statusMap[succ] == InstAnnotator::entered)
			setAnnot(succ, ConcreteExpr<Value *>::createTrue());
	}

	statusMap[curr] = InstAnnotator::left;

	/*
	 * If we cannot get past this instruction (meaning we either exited the loop or
	 * traversed the backedge), return FALSE or TRUE (respectively)
	 */
	if (succs.empty()) {
		if (!backedgePaths.count(curr->getParent())) {
			setAnnot(curr, ConcreteExpr<Value *>::createFalse());
			return;
		} else if (curr->getParent() == l->getHeader()) {
			setAnnot(curr, ConcreteExpr<Value *>::createTrue());
			return;
		}
		BUG();
	}

	/* If this is a branch instruction, create a select expression */
	if (succs.size() == 2) {
		auto cond = dyn_cast<BranchInst>(curr)->getCondition();
		auto regExp = RegisterExpr<Value *>::create(
			cond->getType()->getPrimitiveSizeInBits(), getAnnotMapKey(cond));
		setAnnot(curr,
			 SelectExpr<Value *>::create(
				 std::move(regExp),
				 propagateAnnotFromSuccInLoop(curr, succs[0], backedgePaths, l),
				 propagateAnnotFromSuccInLoop(curr, succs[1], backedgePaths, l)));
		return;
	}
	/* At this point we know there is just one successor: substitute */
	setAnnot(curr, propagateAnnotFromSuccInLoop(curr, succs[0], backedgePaths, l));
	return;
}

InstAnnotator::IRExprUP
InstAnnotator::annotateCASWithBackedgeCond(AtomicCmpXchgInst *curr, BasicBlock *latch, Loop *l,
					   const VSet<llvm::Function *> *cleanSet)
{
	/* Reset DFS data */
	reset();

	/* Collect backedge paths */
	VSet<BasicBlock *> backedgePaths;
	foreachInBackPathTo(latch, l->getHeader(),
			    [&](Instruction &i) { backedgePaths.insert(i.getParent()); });

	for (auto &i : instructions(curr->getParent()->getParent()))
		statusMap[&i] = InstAnnotator::unseen;
	annotateCASWithBackedgeCondDFS(curr->getNextNode(), backedgePaths, l, cleanSet);
	return releaseAnnot(curr->getNextNode());
}
