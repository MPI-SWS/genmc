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

#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include "DriverGraphEnumAPI.hpp"

#include <string>
#include <vector>

enum class ModelType { rc11, imm, lkmm };
enum class SchedulePolicy { ltr, wf, random };
#ifdef ENABLE_GENMC_DEBUG
enum class VerbosityLevel { V0, V1, V2, V3 };
#endif

struct Config {

public:
	/*** General syntax ***/
	std::vector<std::string> cflags;
	std::string inputFile;

	/*** Exploration options ***/
	ModelType model;
	bool isDepTrackingModel;
	CoherenceType coherence;
	unsigned int threads;
	bool LAPOR;
	bool symmetryReduction;
	CheckConsType checkConsType;
	ProgramPoint checkConsPoint;
	bool checkLiveness;
	bool printErrorTrace;
	std::string dotFile;
	bool disableRaceDetection;
	bool disableBAM;
	bool disableStopOnSystemError;

	/*** Persistency options ***/
	bool persevere;
	ProgramPoint checkPersPoint;
	unsigned int blockSize;
	unsigned int maxFileSize;
	JournalDataFS journalData;
	bool disableDelalloc;

	/*** Transformation options ***/
	int unroll;
	bool castElimination;
	bool loopJumpThreading;
	bool spinAssume;
	bool codeCondenser;
	bool loadAnnot;

	/*** Debugging options ***/
	bool inputFromBitcodeFile;
	bool printExecGraphs;
	bool prettyPrintExecGraphs;
	SchedulePolicy schedulePolicy;
	std::string randomScheduleSeed;
	bool printRandomScheduleSeed;
	std::string transformFile;
	std::string programEntryFun;
	unsigned int warnOnGraphSize;
#ifdef ENABLE_GENMC_DEBUG
	bool validateExecGraphs;
	bool countDuplicateExecs;
	VerbosityLevel vLevel;
#endif

	/* Parses the CLI options and initialized the respective fields */
	void getConfigOptions(int argc, char **argv);

private:
	void checkConfigOptions() const;
	void saveConfigOptions();
};

#endif /* __CONFIG_HPP__ */
