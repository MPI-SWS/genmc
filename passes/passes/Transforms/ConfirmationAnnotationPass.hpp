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

#ifndef GENMC_CONFIRMATION_ANNOTATION_PASS_HPP
#define GENMC_CONFIRMATION_ANNOTATION_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class ConfirmationAnnotationPass : public PassInfoMixin<ConfirmationAnnotationPass> {
public:
	auto run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses;
};

#endif /* GENMC_CONFIRMATION_ANNOTATION_PASS_HPP */
