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

enum class ModelType { rc11, imm };
enum class SchedulePolicy { ltr, wf, random };

struct Config {

public:
	/* General syntax */
	std::vector<std::string> cflags;
	std::string inputFile;

	/* Exploration options */
	ModelType model;
	bool isDepTrackingModel;
	CoherenceType coherence;
	bool LAPOR;
	bool symmetryReduction;
	CheckConsType checkConsType;
	ProgramPoint checkConsPoint;
	bool printErrorTrace;
	std::string dotFile;
	bool disableRaceDetection;
	bool disableStopOnSystemError;
	std::string specsFile;

	/* Persistency options */
	bool persevere;
	ProgramPoint checkPersPoint;
	unsigned int blockSize;
	unsigned int maxFileSize;
	JournalDataFS journalData;
	bool disableDelalloc;

	/* Transformation options */
	int unroll;
	bool spinAssume;

	/* Debugging options */
	bool countDuplicateExecs;
	bool inputFromBitcodeFile;
	bool printExecGraphs;
	bool prettyPrintExecGraphs;
	SchedulePolicy schedulePolicy;
	std::string randomScheduleSeed;
	bool printRandomScheduleSeed;
	std::string transformFile;
	std::string programEntryFun;
	bool validateExecGraphs;

	void getConfigOptions(int argc, char **argv);

private:
	void checkConfigOptions() const;
	void saveConfigOptions();
};

#endif /* __CONFIG_HPP__ */
