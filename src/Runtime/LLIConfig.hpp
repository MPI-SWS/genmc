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

#ifndef GENMC_LLI_CONFIG_HPP
#define GENMC_LLI_CONFIG_HPP

#include "ADT/VSet.hpp"

#include <optional>
#include <string>
#include <vector>

struct LLIConfig {
	/*** Compilation options ***/
	std::vector<std::string> cflags;
	std::string inputFile;

	/*** Exploration options ***/
	unsigned int threads{};
	bool disableStopOnSystemError{};
	bool isDepTrackingModel{};
	std::optional<std::string> collectLinSpec;
	std::optional<std::string> checkLinSpec;

	/*** Transformation options ***/
	std::optional<unsigned> unroll;
	VSet<std::string> noUnrollFuns;
	bool castElimination{};
	bool inlineFunctions{};
	bool loopJumpThreading{};
	bool spinAssume{};
	bool codeCondenser{};
	bool loadAnnot{};
	bool assumePropagation{};
	bool mmDetector{};
	bool helper{};
	bool confirmation{};
	bool finalWrite{};
	bool liveness{};
	bool bam{};
	bool rust{};

	/*** Debugging options ***/
	std::string outputLlvmBefore;
	std::string outputLlvmAfter;
	bool skipGenmcStdBuild{};
	VSet<std::string> extraInput;
	std::string programEntryFun;
};

#endif /* GENMC_LLI_CONFIG_HPP */
