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

#ifndef GENMC_FUNCTION_INLINER_PASS_HPP
#define GENMC_FUNCTION_INLINER_PASS_HPP

#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>

#include <llvm/IR/Module.h>

using namespace llvm;

class FunctionInlinerPass : public PassInfoMixin<FunctionInlinerPass> {
public:
	auto run(Module &M, ModuleAnalysisManager &AM) -> PreservedAnalyses;
};

#endif /* GENMC_FUNCTION_INLINER_PASS_HPP */
