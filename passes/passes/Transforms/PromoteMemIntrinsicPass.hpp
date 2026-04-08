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

#ifndef GENMC_PROMOTE_MEMINTRINSIC_PASS_HPP
#define GENMC_PROMOTE_MEMINTRINSIC_PASS_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class PromoteMemIntrinsicPass : public PassInfoMixin<PromoteMemIntrinsicPass> {
public:
	auto run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses;
};

#endif /* GENMC_PROMOTE_MEMINTRINSIC_PASS_HPP */
