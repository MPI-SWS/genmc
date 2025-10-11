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

#ifndef GENMC_CONFIG_HPP
#define GENMC_CONFIG_HPP

#include "Verification/MemoryModel.hpp"

#include <cstdint>
#include <optional>
#include <string>

enum class SchedulePolicy : std::uint8_t { LTR, WF, WFR, Arbitrary };
enum class BoundType : std::uint8_t { context, round, none };

struct Config {
	/*** Exploration options ***/
	ModelType model{};
	bool estimate{};
	bool isDepTrackingModel{};
	std::optional<unsigned int> bound;
	BoundType boundType{};
	bool LAPOR{};
	bool symmetryReduction{};
	bool helper{};
	bool confirmation{};
	bool finalWrite{};
	bool checkLiveness{};
	bool printErrorTrace{};
	std::string dotFile;
	bool instructionCaching{};
	bool disableRaceDetection{};
	bool disableBAM{};
	bool ipr{};
	bool warnUnfreedMemory{};
	std::optional<std::string> collectLinSpec;
	std::optional<std::string> checkLinSpec;
	unsigned int maxExtSize{};
	bool dotPrintOnlyClientEvents{};
	bool replayCompletedThreads{};

	/*** Debugging options ***/
	unsigned int estimationMax{};
	unsigned int estimationMin{};
	unsigned int sdThreshold{};
	bool printExecGraphs{};
	bool printBlockedExecs{};
	SchedulePolicy schedulePolicy{};
	std::string randomScheduleSeed;
	bool printRandomScheduleSeed{};
	unsigned int warnOnGraphSize{};
#ifdef ENABLE_GENMC_DEBUG
	bool printStamps{};
	bool colorAccesses{};
	bool validateExecGraphs{};
	bool countDuplicateExecs{};
	bool countMootExecs{};
	bool printEstimationStats{};
	bool boundsHistogram{};
	bool relincheDebug{};
#endif
};

/* Check validity of config options. */
void checkConfig(Config &conf);

#endif /* GENMC_CONFIG_HPP */
