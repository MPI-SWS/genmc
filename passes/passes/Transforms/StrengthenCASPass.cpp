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

#include "StrengthenCASPass.hpp"
#include "passes/Transforms/LoadAnnotationPass.hpp"

#include "genmc/Execution/LoadAnnotation.hpp"
#include "genmc/Support/Error.hpp"

#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>

#include <format>
#include <string>

using namespace llvm;

static auto sourceLocation(const Instruction *inst) -> std::string
{
	if (const auto &dl = inst->getDebugLoc())
		return std::format("{}:{}", dl->getFilename().str(), dl.getLine());
	return inst->getFunction()->getName().str();
}

auto StrengthenCASPass::run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses
{
	const auto &lai = FAM.getResult<LoadAnnotationAnalysis>(F);
	auto modified = false;

	for (auto &inst : instructions(F)) {
		auto *casi = dyn_cast<AtomicCmpXchgInst>(&inst);
		if (!casi || !casi->isWeak())
			continue;

		/* If the weak CAS is annotated, make it strong */
		auto it = lai.annotMap.find(casi);
		if (it != lai.annotMap.end() && it->second.first == AssumeType::Spinloop) {
			casi->setWeak(false);
			modified = true;
			continue;
		}

		/* NB: keep this message free of the words the test driver greps for in
		 * GenMC's output ("explored", "blocked", "wall-clock"). */
		WARN("Weak CAS at {} could not be strengthened to a strong "
		     "one. Strong CASes enable more effective verification.",
		     sourceLocation(casi));
	}
	return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
