/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef GENMC_CONFIG_HPP
#define GENMC_CONFIG_HPP

#include "ADT/VSet.hpp"
#include "Config/MemoryModel.hpp"
#include "Config/Verbosity.hpp"
#include "Runtime/InterpreterEnumAPI.hpp"

#include <cstdint>
#include <optional>
#include <string>

enum class SchedulePolicy : std::uint8_t { ltr, wf, wfr, arbitrary };
enum class BoundType : std::uint8_t { context, round };

struct Config {
	/*** General syntax ***/
	std::vector<std::string> cflags;
	std::string inputFile;

	/*** Exploration options ***/
	ModelType model{};
	bool estimate{};
	bool isDepTrackingModel{};
	unsigned int threads{};
	std::optional<unsigned int> bound;
	BoundType boundType{};
	bool LAPOR{};
	bool symmetryReduction{};
	bool helper{};
	bool checkLiveness{};
	bool printErrorTrace{};
	std::string dotFile;
	bool instructionCaching{};
	bool disableRaceDetection{};
	bool disableBAM{};
	bool ipr{};
	bool disableStopOnSystemError{};
	bool warnUnfreedMemory{};
	std::optional<std::string> collectLinSpec;
	std::optional<std::string> checkLinSpec;
	unsigned int maxExtSize{};
	bool dotPrintOnlyClientEvents{};

	/*** Persistency options ***/
	bool persevere{};
	unsigned int blockSize{};
	unsigned int maxFileSize{};
	JournalDataFS journalData{};
	bool disableDelalloc{};

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
	bool confirmAnnot{};
	bool mmDetector{};

	/*** Debugging options ***/
	unsigned int estimationMax{};
	unsigned int estimationMin{};
	unsigned int sdThreshold{};
	bool inputFromBitcodeFile{};
	bool printExecGraphs{};
	bool printBlockedExecs{};
	SchedulePolicy schedulePolicy{};
	std::string randomScheduleSeed;
	bool printRandomScheduleSeed{};
	std::string transformFile;
	std::string programEntryFun;
	unsigned int warnOnGraphSize{};
	VerbosityLevel vLevel{};
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

/* Parses CLI options and initializes a Config object */
void parseConfig(int argc, char **argv, Config &conf);

#endif /* GENMC_CONFIG_HPP */
