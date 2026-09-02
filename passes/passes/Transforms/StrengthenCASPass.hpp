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

#ifndef GENMC_STRENGTHEN_CAS_PASS_HPP
#define GENMC_STRENGTHEN_CAS_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

/** Strengthens weak CASes connected to spinloops into strong ones. Weak
 * CASes that cannot be strengthened are left intact, with a warning that a
 * strong CAS would enable more effective verification.
 *
 * Must run after the annotation passes, so that the spinloop assumes
 * have been turned into load/CAS annotations. */
class StrengthenCASPass : public PassInfoMixin<StrengthenCASPass> {
public:
	auto run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses;
};

#endif /* GENMC_STRENGTHEN_CAS_PASS_HPP */
