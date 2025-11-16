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

#include "LoadAnnotationPass.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"
#include "Static/LLVMUtils.hpp"
#include "Static/Transforms/InstAnnotator.hpp"
#include "Support/Error.hpp"
#include "Support/SExpr.hpp"
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>

#include <algorithm>

using namespace llvm;

/* Helper for getSourceLoads() -- see below */
void calcSourceLoads(Instruction *i, VSet<PHINode *> phis, std::vector<Instruction *> &source)
{
	if (!i)
		return;

	/* Don't go past stores or allocas (CASes are OK) */
	if (isa<StoreInst>(i) || isa<AtomicRMWInst>(i) || isa<AllocaInst>(i))
		return;

	/* If we reached a (source) load, collect it */
	if (isa<LoadInst>(i) || isa<AtomicCmpXchgInst>(i)) {
		source.push_back(i);
		return;
	}

	/* If this is an already encountered Φ, don't go into circles */
	if (auto *phi = dyn_cast<PHINode>(i)) {
		if (phis.count(phi))
			return;
		phis.insert(phi);
	}

	/* Otherwise, recurse */
	for (auto &u : i->operands()) {
		if (auto *pi = dyn_cast<Instruction>(u.get())) {
			calcSourceLoads(pi, phis, source);
		} else if (auto *c = dyn_cast<Constant>(u.get())) {
			if (auto *phi = dyn_cast<PHINode>(i)) {
				auto *term = phi->getIncomingBlock(u)->getTerminator();
				if (auto *bi = dyn_cast<BranchInst>(term))
					if (bi->isConditional())
						calcSourceLoads(
							dyn_cast<Instruction>(bi->getCondition()),
							phis, source);
			}
		}
	}
}

/*
 * Returns the source loads of an assume statement, that is,
 * loads the result of which is used in the assume.
 */
auto getSourceLoads(CallInst *assm) -> std::vector<Instruction *>
{
	VSet<PHINode *> phis;
	std::vector<Instruction *> source;

	if (auto *arg = dyn_cast<Instruction>(assm->getOperand(0)))
		calcSourceLoads(arg, phis, source);
	std::sort(source.begin(), source.end());
	source.erase(std::unique(source.begin(), source.end()), source.end());
	return source;
}

/*
 * Given an assume's source loads, returns the annotatable ones.
 */
auto filterAnnotatableFromSource(CallInst *assm, const std::vector<Instruction *> &source)
	-> std::vector<Instruction *>
{
	std::vector<Instruction *> result;

	/* Collect candidates for which the path to the assume is clear */
	for (auto *li : source) {
		auto assumeFound = false;
		auto loadFound = false;
		auto sideEffects = false;
		foreachInBackPathTo(assm->getParent(), li->getParent(), [&](Instruction &i) {
			/* wait until we find the assume */
			if (!assumeFound) {
				assumeFound |= (dyn_cast<CallInst>(&i) == assm);
				return;
			}
			/* we should stop when the load is found */
			if (assumeFound && !loadFound) {
				sideEffects |= (hasSideEffects(&i) && &i != li); /* also CASes */
				sideEffects |= (isa<LoadInst>(&i) && &i != li);
			}
			if (!loadFound) {
				loadFound |= (&i == li);
				if (loadFound) {
					if (!sideEffects)
						result.push_back(li);
					/* reset for next path */
					assumeFound = false;
					loadFound = false;
					sideEffects = false;
				}
			}
		});
	}
	result.erase(std::unique(result.begin(), result.end()), result.end());
	return result;
}

/*
 * Returns all of ASSM's annotatable loads
 */
auto getAnnotatableLoads(CallInst *assm) -> std::vector<Instruction *>
{
	if (!isAssumeFunction(getCalledFunOrStripValName(*assm)))
		return {}; /* yet another check... */

	auto sourceLoads = getSourceLoads(assm);
	return filterAnnotatableFromSource(assm, sourceLoads);
}

static auto extractAssumeArgument(CallInst *assume) -> uint64_t
{
	auto *arg = dyn_cast<Constant>(assume->getArgOperand(1));
	BUG_ON(!arg || !arg->getType()->isIntegerTy());

	return arg->getUniqueInteger().getLimitedValue();
}

auto LoadAnnotationAnalysis::run(Function &F, FunctionAnalysisManager &FAM) -> Result
{
	InstAnnotator annotator;

	for (auto &i : instructions(F)) {
		auto *call = llvm::dyn_cast<llvm::CallInst>(&i);
		if (call && isAssumeFunction(getCalledFunOrStripValName(*call))) {
			auto loads = getAnnotatableLoads(call);
			for (auto *l : loads) {
				auto type = AssumeType(extractAssumeArgument(call));
				result_.annotMap[l] = std::make_pair(type, annotator.annotate(l));
			}
		}
	}
	return result_;
}

auto LoadAnnotationPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	LAI_ = FAM.getResult<LoadAnnotationAnalysis>(F);
	return PreservedAnalyses::all();
}
