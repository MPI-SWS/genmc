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

#ifndef GENMC_DECLARE_INTERNALS_PASS_HPP
#define GENMC_DECLARE_INTERNALS_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class DeclareInternalsPass : public PassInfoMixin<DeclareInternalsPass> {
public:
	auto run(Module &M, ModuleAnalysisManager &AM) -> PreservedAnalyses;
};

#endif /* GENMC_DECLARE_INTERNALS_PASS_HPP */
