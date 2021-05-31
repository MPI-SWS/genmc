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
#include "SExprVisitor.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>

using namespace llvm;

void InstAnnotator::reset()
{
	statusMap.clear();
	annotMap.clear();
	return;
}

std::unique_ptr<SExpr> generateOperandExpr(Value *op)
{
	if (auto *c = dyn_cast<Constant>(op)) {
		BUG_ON(!c->getType()->isIntegerTy());
		if (isa<UndefValue>(c))
			return ConcreteExpr::create(APInt(c->getType()->getIntegerBitWidth(), 42));
		return ConcreteExpr::create(c->getUniqueInteger());
	}
	return RegisterExpr::create(op);
}

std::unique_ptr<SExpr> generateInstExpr(Instruction *curr)
{
	/*
	 * Next, we try to generate an annotation for a whole bunch of instructions,
	 * apart from function calls, memory instructions, and some pointer casts.
	 * For the cases we do not handle, we simply return false.
	 */

#define HANDLE_INST(op, ...)			\
	case Instruction::op:			\
		return op##Expr::create(__VA_ARGS__)

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

		HANDLE_INST(Add, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Sub, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Mul, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(UDiv, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(SDiv, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(URem, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(And, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Or, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Xor, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(Shl, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(LShr, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));
		HANDLE_INST(AShr, curr->getType(),
			    generateOperandExpr(curr->getOperand(0)),
			    generateOperandExpr(curr->getOperand(1)));

	case Instruction::ICmp: {
		llvm::CmpInst *cmpi = llvm::dyn_cast<llvm::CmpInst>(curr);
		switch(cmpi->getPredicate()) {
		case CmpInst::ICMP_EQ :
			return EqExpr::create(generateOperandExpr(curr->getOperand(0)),
					      generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_NE :
			return NeExpr::create(generateOperandExpr(curr->getOperand(0)),
					      generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_UGT :
			return UgtExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_UGE :
			return UgeExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_ULT :
			return UltExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_ULE :
			return UleExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SGT :
			return SgtExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SGE :
			return SgeExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SLT :
			return SltExpr::create(generateOperandExpr(curr->getOperand(0)),
					       generateOperandExpr(curr->getOperand(1)));
		case CmpInst::ICMP_SLE :
			return SleExpr::create(generateOperandExpr(curr->getOperand(0)),
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
	return ConcreteExpr::createTrue();
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

std::unique_ptr<SExpr>
InstAnnotator::propagateAnnotFromSucc(Instruction *curr, Instruction *succ)
{
	auto succExp = annotMap[succ]->clone();
	auto substitutor = SExprRegSubstitutor();

	PHINode *succPhi = nullptr;
	for (auto iit = succ;
	     (succPhi = dyn_cast<PHINode>(iit)) && curr->getParent() != succ->getParent();
	     iit = iit->getNextNode()) {
		auto phiOp = generateOperandExpr(succPhi->getIncomingValueForBlock(curr->getParent()));
		succExp = substitutor.substitute(succExp.get(), iit, phiOp.get());
	}

	if (isa<BranchInst>(curr) || (isa<PHINode>(curr) && curr->getParent() == succ->getParent()))
		return succExp;

	auto currOp = generateInstExpr(curr);
	return substitutor.substitute(succExp.get(), curr, currOp.get());
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
			annotMap[succ] = ConcreteExpr::createTrue();
	}

	statusMap[curr] = InstAnnotator::left;

	/* If we cannot get past this instruction, return either TRUE or the assumed expression */
	if (succs.empty()) {
		if (auto *ci = dyn_cast<CallInst>(curr)) {
			if (isAssumeFunction(getCalledFunOrStripValName(*ci))) {
				annotMap[curr] = generateOperandExpr(ci->getOperand(0));
				return;
			}
		}
		annotMap[curr] = ConcreteExpr::createTrue();
		return;
	}
	/* If this is a branch instruction, create a select expression */
	if (succs.size() == 2) {
		auto cond = dyn_cast<BranchInst>(curr)->getCondition();
		annotMap[curr] = SelectExpr::create(RegisterExpr::create(cond),
						     propagateAnnotFromSucc(curr, succs[0]),
						     propagateAnnotFromSucc(curr, succs[1]));
		return;
	}
	/* At this point we know there is just one successor: substitute */
	annotMap[curr] = propagateAnnotFromSucc(curr, succs[0]);
	return;
}

std::unique_ptr<SExpr> InstAnnotator::annotate(LoadInst *curr)
{
	/* Reset DFS data */
	reset();

	for (auto &i : instructions(curr->getParent()->getParent()))
		statusMap[&i] = InstAnnotator::unseen;
	annotateDFS(curr->getNextNode());
	return std::move(annotMap[curr->getNextNode()]);
}

std::unique_ptr<SExpr> InstAnnotator::annotateBBCond(BasicBlock *bb, BasicBlock *pred /* = nullptr */)
{
	auto *bi = dyn_cast<BranchInst>(bb->getTerminator());
	if (!bi)
		return ConcreteExpr::createFalse();
	if (bi->isUnconditional())
		return ConcreteExpr::createTrue();

	/* Reset data */
	reset();

	/* Propagate jump condition backwards to the beginning of the basic block */
	annotMap[bi] = generateOperandExpr(bi->getCondition());
	for (auto irit = ++bb->rbegin(); irit != bb->rend(); ++irit) {
		annotMap[&*irit] = propagateAnnotFromSucc(&*irit, irit->getNextNode());
	}

	/* If a predecessor is given substitute Φ values too */
	if (pred)
		return propagateAnnotFromSucc(pred->getTerminator(), &*bb->begin());
	return std::move(annotMap[&*bb->begin()]);
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

std::unique_ptr<SExpr>
InstAnnotator::propagateAnnotFromSuccInLoop(Instruction *curr, Instruction *succ,
					    const VSet<BasicBlock *> &backedgePaths, Loop *l)
{
	auto succExp = annotMap[succ]->clone();
	auto substitutor = SExprRegSubstitutor();

	PHINode *succPhi = nullptr;
	for (auto iit = succ;
	     (succPhi = dyn_cast<PHINode>(iit)) && curr->getParent() != succ->getParent();
	     iit = iit->getNextNode()) {
		auto phiOp = generateOperandExpr(succPhi->getIncomingValueForBlock(curr->getParent()));
		succExp = substitutor.substitute(succExp.get(), iit, phiOp.get());
	}

	if (isa<BranchInst>(curr) || (isa<PHINode>(curr) && curr->getParent() == succ->getParent()) ||
	    isa<AtomicCmpXchgInst>(curr) || isa<ExtractValueInst>(curr) || isa<LoadInst>(curr) ||
	    isa<CallInst>(curr))
		return succExp;

	/* Transform assume()s into disjunctions */
	if (auto *ci = dyn_cast<CallInst>(curr)) {
		BUG_ON(!isAssumeFunction(getCalledFunOrStripValName(*ci)));
		return ConjunctionExpr::create(std::move(generateOperandExpr(ci->getOperand(0))),
					       std::move(succExp));
	}

	auto currOp = generateInstExpr(curr);
	return substitutor.substitute(succExp.get(), curr, currOp.get());
}

void InstAnnotator::annotateCASWithBackedgeCondDFS(Instruction *curr,
						   const VSet<BasicBlock *> &backedgePaths,
						   Loop *l,
						   const VSet<llvm::Function *> *cleanSet)
{
	statusMap[curr] = InstAnnotator::entered;

	std::vector<Instruction *> succs = getNextOrBranchSuccessorsInLoop(curr, backedgePaths, l, cleanSet);
	BUG_ON(succs.size() > 2);

	for (auto *succ : succs) {
		if (statusMap[succ] == InstAnnotator::unseen)
			annotateCASWithBackedgeCondDFS(succ, backedgePaths, l, cleanSet);
		else if (statusMap[succ] == InstAnnotator::entered)
			annotMap[succ] = ConcreteExpr::createTrue();
	}

	statusMap[curr] = InstAnnotator::left;

	/*
	 * If we cannot get past this instruction (meaning we either exited the loop or
	 * traversed the backedge), return FALSE or TRUE (respectively)
	 */
	if (succs.empty()) {
		if (!backedgePaths.count(curr->getParent())) {
			annotMap[curr] = ConcreteExpr::createFalse();
			return;
		} else if (curr->getParent() == l->getHeader()) {
			annotMap[curr] = ConcreteExpr::createTrue();
			return;
		}
		BUG();
	}

	/* If this is a branch instruction, create a select expression */
	if (succs.size() == 2) {
		auto cond = dyn_cast<BranchInst>(curr)->getCondition();
		annotMap[curr] = SelectExpr::create(RegisterExpr::create(cond),
						    propagateAnnotFromSuccInLoop(curr, succs[0], backedgePaths, l),
						    propagateAnnotFromSuccInLoop(curr, succs[1], backedgePaths, l));
		return;
	}
	/* At this point we know there is just one successor: substitute */
	annotMap[curr] = propagateAnnotFromSuccInLoop(curr, succs[0], backedgePaths, l);
	return;
}

std::unique_ptr<SExpr> InstAnnotator::annotateCASWithBackedgeCond(AtomicCmpXchgInst *curr,
								  BasicBlock *latch,
								  Loop *l,
								  const VSet<llvm::Function *> *cleanSet)
{
	/* Reset DFS data */
	reset();

	/* Collect backedge paths */
	VSet<BasicBlock *> backedgePaths;
	foreachInBackPathTo(latch, l->getHeader(), [&](Instruction &i){ backedgePaths.insert(i.getParent()); });

	for (auto &i : instructions(curr->getParent()->getParent()))
		statusMap[&i] = InstAnnotator::unseen;
	annotateCASWithBackedgeCondDFS(curr->getNextNode(), backedgePaths, l, cleanSet);
	return std::move(annotMap[curr->getNextNode()]);
}
