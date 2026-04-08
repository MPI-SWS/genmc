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

#ifndef GENMC_ELIMINATE_ANNOTATIONS_PASS_HPP
#define GENMC_ELIMINATE_ANNOTATIONS_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class EliminateAnnotationsPass : public PassInfoMixin<EliminateAnnotationsPass> {
public:
	struct AnnotationOptions {
		bool annotHelper{};
		bool annotConf{};
		bool annotFinal{};
	};

	EliminateAnnotationsPass(AnnotationOptions options) : options_(options) {}

	auto run(Function &F, FunctionAnalysisManager &FAM) -> PreservedAnalyses;

private:
	[[nodiscard]] auto getOptions() const -> const AnnotationOptions & { return options_; }

	AnnotationOptions options_;
};

#endif /* GENMC_ELIMINATE_ANNOTATIONS_PASS_HPP */
