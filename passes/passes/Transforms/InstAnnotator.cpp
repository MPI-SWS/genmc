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

#include "InstAnnotator.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/SExpr.hpp"
#include "genmc/Support/SExprVisitor.hpp"
#include "genmc/Support/SVal.hpp"
#include "passes/InternalFunctions.hpp"
#include "passes/LLVMUtils.hpp"
#include "passes/ModuleInfo.hpp"

#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Casting.h>

#include <utility>
#include <vector>

using namespace llvm;

void InstAnnotator::reset()
{
	statusMap.clear();
	annotMap.clear();
}

auto InstAnnotator::getAnnotMapKey(Value *i) -> Value * { return i; }

auto InstAnnotator::getAnnot(Instruction *i) -> const InstAnnotator::IRExpr *
{
	return annotMap[getAnnotMapKey(i)].get();
}

auto InstAnnotator::releaseAnnot(Instruction *i) -> InstAnnotator::IRExprUP
{
	return std::move(annotMap[getAnnotMapKey(i)]);
}

void InstAnnotator::setAnnot(Instruction *i, InstAnnotator::IRExprUP annot)
{
	annotMap[getAnnotMapKey(i)] = std::move(annot);
}

auto InstAnnotator::generateOperandExpr(Module *mod, Value *op) -> InstAnnotator::IRExprUP
{
	const auto &DL = mod->getDataLayout();
	constexpr SVal concUndefVal(42);

	/* First, check if the expression is a constant */
	if (auto *constant = dyn_cast<Constant>(op)) {
		auto *typ = constant->getType();
		if (typ->isIntegerTy()) {
			auto bitWidth = typ->getIntegerBitWidth();
			if (isa<UndefValue>(constant))
				return ConcreteExpr<Value *>::create(bitWidth, concUndefVal);
			auto *ci = dyn_cast<ConstantInt>(constant);
			VERIFY(ci); /* will fire for ConstantExpr (being deprecated) */
			return ConcreteExpr<Value *>::create(bitWidth, SVal(ci->getLimitedValue()));
		}
		if (typ->isPointerTy()) {
			auto bitWidth = DL.getPointerTypeSizeInBits(typ);
			if (isa<UndefValue>(constant))
				return ConcreteExpr<Value *>::create(bitWidth, concUndefVal);
			VERIFY(isa<ConstantPointerNull>(constant)); /* will fire for GlobalValue */
			return ConcreteExpr<Value *>::create(bitWidth, SVal(0));
		}
		ERROR("Only integer and null constants currently allowed in assume() "
		      "expressions.");
	}

	/* Otherwise, it has to be an instruction or an argument */
	VERIFY(isa<Instruction>(op) || isa<Argument>(op));
	return RegisterExpr<Value *>::create(DL.getTypeAllocSizeInBits(op->getType()),
					     getAnnotMapKey(op));
}

auto InstAnnotator::generateInstExpr(Instruction *curr) -> InstAnnotator::IRExprUP
{
	/*
	 * Next, we try to generate an annotation for a whole bunch of instructions,
	 * apart from function calls, memory instructions, and some pointer casts.
	 * For the cases we do not handle, we simply return false.
	 */
#define HANDLE_INST(op, ...)                                                                       \
	case Instruction::op:                                                                      \
		return op##Expr<Value *>::create(__VA_ARGS__)

	auto *mod = curr->getModule();
	switch (curr->getOpcode()) {
		HANDLE_INST(ZExt, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)));
		HANDLE_INST(SExt, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)));
		HANDLE_INST(Trunc, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)));

		HANDLE_INST(Select, generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)),
			    generateOperandExpr(mod, curr->getOperand(2)));

		HANDLE_INST(Add, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(Sub, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(Mul, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(UDiv, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(SDiv, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(URem, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(And, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(Or, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(Xor, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(Shl, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(LShr, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));
		HANDLE_INST(AShr, curr->getType()->getPrimitiveSizeInBits(),
			    generateOperandExpr(mod, curr->getOperand(0)),
			    generateOperandExpr(mod, curr->getOperand(1)));

	/* Special case for extracts --- only CAS extracts allowed; */
	case Instruction::ExtractValue: {
		auto *extract = cast<ExtractValueInst>(curr);
		auto *cas = extractsFromCAS(extract);
		if (!cas)
			break;
		/* Hack: If it extracts the value read, just return a register expr;
		 * the types won't match but we don't care about that since we won't
		 * annotate above the CAS anyway */
		if (*extract->idx_begin() == 0)
			return generateOperandExpr(mod, cas);
		return EqExpr<Value *>::create(generateOperandExpr(mod, cas),
					       generateOperandExpr(mod, cas->getCompareOperand()));
	}

	case Instruction::BitCast: {
		return generateOperandExpr(mod, curr->getOperand(0));
	}

	case Instruction::PtrToInt:
	case Instruction::IntToPtr: {
		auto *castinst = cast<CastInst>(curr);
		auto *mod = curr->getParent()->getParent()->getParent();
		const auto &DL = mod->getDataLayout();
		auto srcSize = DL.getTypeAllocSize(castinst->getSrcTy());
		auto dstSize = DL.getTypeAllocSize(castinst->getDestTy());
		if (srcSize > dstSize) {
			return TruncExpr<Value *>::create(
				castinst->getDestTy()->getPrimitiveSizeInBits(),
				generateOperandExpr(mod, castinst->getOperand(0)));
		}
		if (srcSize < dstSize) {
			return ZExtExpr<Value *>::create(
				castinst->getDestTy()->getPrimitiveSizeInBits(),
				generateOperandExpr(mod, castinst->getOperand(0)));
		}
		return generateOperandExpr(mod, castinst->getOperand(0));
	}

	case Instruction::ICmp: {
		auto *cmpi = llvm::cast<llvm::CmpInst>(curr);
		switch (cmpi->getPredicate()) {
		case CmpInst::ICMP_EQ:
			return EqExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_NE:
			return NeExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_UGT:
			return UgtExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_UGE:
			return UgeExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_ULT:
			return UltExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_ULE:
			return UleExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_SGT:
			return SgtExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_SGE:
			return SgeExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_SLT:
			return SltExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
		case CmpInst::ICMP_SLE:
			return SleExpr<Value *>::create(
				generateOperandExpr(mod, curr->getOperand(0)),
				generateOperandExpr(mod, curr->getOperand(1)));
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

static auto getNextOrBranchSuccessors(Instruction *i) -> std::vector<Instruction *>
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

auto InstAnnotator::propagateAnnotFromSucc(Instruction *curr, Instruction *succ)
	-> InstAnnotator::IRExprUP
{
	auto succExp = getAnnot(succ)->clone();
	auto substitutor = SExprRegSubstitutor<Value *>();

	const PHINode *succPhi = nullptr;
	for (auto *iit = succ;
	     (succPhi = dyn_cast<PHINode>(iit)) && curr->getParent() != succ->getParent();
	     iit = iit->getNextNode()) {
		auto phiOp = generateOperandExpr(
			curr->getModule(), succPhi->getIncomingValueForBlock(curr->getParent()));
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
	VERIFY(succs.size() <= 2);

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
				setAnnot(curr,
					 generateOperandExpr(curr->getModule(), ci->getOperand(0)));
				return;
			}
		}
		setAnnot(curr, ConcreteExpr<Value *>::createTrue());
		return;
	}
	/* If this is a branch instruction, create a select expression */
	if (succs.size() == 2) {
		auto *cond = cast<BranchInst>(curr)->getCondition();
		auto regExp = RegisterExpr<Value *>::create(
			cond->getType()->getPrimitiveSizeInBits(), getAnnotMapKey(cond));
		setAnnot(curr, SelectExpr<Value *>::create(std::move(regExp),
							   propagateAnnotFromSucc(curr, succs[0]),
							   propagateAnnotFromSucc(curr, succs[1])));
		return;
	}
	/* At this point we know there is just one successor: substitute */
	setAnnot(curr, propagateAnnotFromSucc(curr, succs[0]));
}

auto InstAnnotator::annotate(Instruction *curr) -> InstAnnotator::IRExprUP
{
	/* Reset DFS data + prepare new exploration */
	reset();
	for (auto &i : instructions(curr->getParent()->getParent()))
		statusMap[&i] = InstAnnotator::unseen;

	/* The load annotation will be the expression from its successor to the assume */
	VERIFY(isa<LoadInst>(curr) || isa<AtomicCmpXchgInst>(curr));
	annotateDFS(curr->getNextNode());
	return releaseAnnot(curr->getNextNode());
}

auto InstAnnotator::annotateBBCond(BasicBlock *bb, BasicBlock *pred /* = nullptr */)
	-> InstAnnotator::IRExprUP
{
	auto *bi = dyn_cast<BranchInst>(bb->getTerminator());
	if (!bi)
		return ConcreteExpr<Value *>::createFalse();
	if (bi->isUnconditional())
		return ConcreteExpr<Value *>::createTrue();

	/* Reset data */
	reset();

	/* Propagate jump condition backwards to the beginning of the basic block */
	setAnnot(bi, generateOperandExpr(bi->getModule(), bi->getCondition()));
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

static auto getNextOrBranchSuccessorsInLoop(Instruction *i, const VSet<BasicBlock *> &backedgePaths,
					    Loop *l, const VSet<llvm::Function *> *cleanSet)
	-> std::vector<Instruction *>
{
	std::vector<Instruction *> succs;

	/*
	 * The points beyond which we cannot annotate are different when it comes
	 * to backedges: CASes are ignored (they should fail and will be checked later),
	 * while loop headers or blocks outside the loop have no successors
	 */
	if (!backedgePaths.contains(i->getParent()) || i == &*l->getHeader()->begin())
		return succs;

	/* Sanity checks for side-effects: only CASes and effect-free calls are allowed */
	auto *ci = llvm::dyn_cast<CallInst>(i);
	auto inCleanSet = ci && cleanSet && cleanSet->contains(ci->getCalledFunction());
	VERIFY(!hasSideEffects(i) || isa<AtomicCmpXchgInst>(i) || inCleanSet);

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

auto InstAnnotator::propagateAnnotFromSuccInLoop(Instruction *curr, Instruction *succ,
						 const VSet<BasicBlock *> & /*backedgePaths*/,
						 Loop * /*l*/) -> InstAnnotator::IRExprUP
{
	auto *mod = curr->getModule();
	auto succExp = getAnnot(succ)->clone();
	auto substitutor = SExprRegSubstitutor<Value *>();

	const PHINode *succPhi = nullptr;
	for (auto *iit = succ;
	     (succPhi = dyn_cast<PHINode>(iit)) && curr->getParent() != succ->getParent();
	     iit = iit->getNextNode()) {
		auto phiOp = generateOperandExpr(
			mod, succPhi->getIncomingValueForBlock(curr->getParent()));
		succExp = substitutor.substitute(succExp.get(), getAnnotMapKey(iit), phiOp.get());
	}

	if (isa<BranchInst>(curr) ||
	    (isa<PHINode>(curr) && curr->getParent() == succ->getParent()) ||
	    isa<AtomicCmpXchgInst>(curr) || isa<ExtractValueInst>(curr) || isa<LoadInst>(curr) ||
	    isa<CallInst>(curr))
		return succExp;

	/* Transform assume()s into disjunctions */
	if (auto *ci = dyn_cast<CallInst>(curr)) {
		VERIFY(isAssumeFunction(getCalledFunOrStripValName(*ci)));
		return ConjunctionExpr<Value *>::create(generateOperandExpr(mod, ci->getOperand(0)),
							std::move(succExp));
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
	VERIFY(succs.size() <= 2);

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
		if (!backedgePaths.contains(curr->getParent())) {
			setAnnot(curr, ConcreteExpr<Value *>::createFalse());
			return;
		}
		if (curr->getParent() == l->getHeader()) {
			setAnnot(curr, ConcreteExpr<Value *>::createTrue());
			return;
		}
		UNREACHABLE();
	}

	/* If this is a branch instruction, create a select expression */
	if (succs.size() == 2) {
		auto *cond = cast<BranchInst>(curr)->getCondition();
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
}

auto InstAnnotator::annotateCASWithBackedgeCond(AtomicCmpXchgInst *curr, BasicBlock *latch, Loop *l,
						const VSet<llvm::Function *> *cleanSet)
	-> InstAnnotator::IRExprUP
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
