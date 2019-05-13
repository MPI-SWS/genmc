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

#include "config.h"
#include "Config.hpp"
#include <llvm/ADT/ArrayRef.h>	// needed for 3.5
#include <llvm/Support/CommandLine.h>

/* Command-line argument categories */
static llvm::cl::OptionCategory clGeneral("Exploration Options");
static llvm::cl::OptionCategory clTransformation("Transformation Options");
static llvm::cl::OptionCategory clDebugging("Debugging Options");

/* Command-line argument types, default values and descriptions */
llvm::cl::list<std::string>
clCFLAGS(llvm::cl::Positional, llvm::cl::ZeroOrMore, llvm::cl::desc("-- [CFLAGS]"));
static llvm::cl::opt<std::string>
clInputFile(llvm::cl::Positional, llvm::cl::Required, llvm::cl::desc("<input file>"));

llvm::cl::opt<ModelType>
clModelType(llvm::cl::values(
		    clEnumValN(ModelType::weakra, "weakra", "Weak RA model"),
		    clEnumValN(ModelType::mo, "mo",         "MO model"),
		    clEnumValN(ModelType::wb, "wb",         "WB model")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ),
	    llvm::cl::cat(clGeneral),
	    llvm::cl::init(ModelType::wb),
	    llvm::cl::desc("Choose model type:"));
static llvm::cl::opt<bool>
clPrintErrorTrace("print-error-trace", llvm::cl::cat(clGeneral),
		  llvm::cl::desc("Print error trace"));
static llvm::cl::opt<std::string>
clDotGraphFile("dump-error-graph", llvm::cl::init(""), llvm::cl::value_desc("file"),
	       llvm::cl::cat(clGeneral),
	       llvm::cl::desc("Dumps an error graph to a file (DOT format)"));
static llvm::cl::opt<CheckPSCType>
clCheckPscAcyclicity("check-psc-acyclicity", llvm::cl::init(CheckPSCType::nocheck), llvm::cl::cat(clGeneral),
		     llvm::cl::desc("Check whether PSC is acyclic at the end of each execution"),
		     llvm::cl::values(
			     clEnumValN(CheckPSCType::nocheck, "none", "Disable PSC checks"),
			     clEnumValN(CheckPSCType::weak,    "weak", "Check PSC-weak"),
			     clEnumValN(CheckPSCType::wb,      "wb",   "Check PSC-wb"),
			     clEnumValN(CheckPSCType::full,    "full", "Check complete PSC")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ));
static llvm::cl::opt<bool>
clCheckWbAcyclicity("check-wb-acyclicity", llvm::cl::cat(clGeneral),
		     llvm::cl::desc("Check whether WB is acyclic at the end of each execution"));
static llvm::cl::opt<std::string>
clLibrarySpecsFile("library-specs", llvm::cl::init(""), llvm::cl::value_desc("file"),
		   llvm::cl::cat(clGeneral),
		   llvm::cl::desc("Check for library correctness"));
static llvm::cl::opt<bool>
clDisableRaceDetection("disable-race-detection", llvm::cl::cat(clGeneral),
		     llvm::cl::desc("Disable race detection"));

static llvm::cl::opt<int>
clLoopUnroll("unroll", llvm::cl::init(-1), llvm::cl::value_desc("N"),
	     llvm::cl::cat(clTransformation),
	     llvm::cl::desc("Unroll loops N times"));
static llvm::cl::opt<bool>
clDisableSpinAssume("disable-spin-assume", llvm::cl::cat(clTransformation),
		    llvm::cl::desc("Disable spin-assume transformation"));

static llvm::cl::opt<std::string>
clProgramEntryFunction("program-entry-function", llvm::cl::init("main"),
		       llvm::cl::value_desc("fun_name"), llvm::cl::cat(clDebugging),
		       llvm::cl::desc("Function used as program entrypoint (default: main())"));
static llvm::cl::opt<bool>
clInputFromBitcodeFile("input-from-bitcode-file", llvm::cl::cat(clDebugging),
		       llvm::cl::desc("Read LLVM bitcode directly from file"));
static llvm::cl::opt<std::string>
clTransformFile("transform-output", llvm::cl::init(""),	llvm::cl::value_desc("file"),
		llvm::cl::cat(clDebugging),
		llvm::cl::desc("Output the transformed LLVM code to file"));
static llvm::cl::opt<bool>
clValidateExecGraphs("validate-exec-graphs", llvm::cl::cat(clDebugging),
		     llvm::cl::desc("Validate the execution graphs in each step"));
static llvm::cl::opt<bool>
clRandomizeSchedule("randomize-schedule", llvm::cl::cat(clDebugging),
		    llvm::cl::desc("Execute threads in random order"));
static llvm::cl::opt<bool>
clPrintRandomizeScheduleSeed("print-randomize-schedule-seed", llvm::cl::cat(clDebugging),
			     llvm::cl::desc("Print the seed used for randomized scheduling"));
static llvm::cl::opt<std::string>
clRandomizeScheduleSeed("randomize-schedule-seed", llvm::cl::init(""),
			llvm::cl::value_desc("seed"), llvm::cl::cat(clDebugging),
			llvm::cl::desc("Seed to be used for randomized scheduling"));
static llvm::cl::opt<bool>
clPrintExecGraphs("print-exec-graphs", llvm::cl::cat(clDebugging),
		  llvm::cl::desc("Print explored execution graphs"));
static llvm::cl::opt<bool>
clPrettyPrintExecGraphs("pretty-print-exec-graphs", llvm::cl::cat(clDebugging),
			llvm::cl::desc("Pretty-print explored execution graphs"));
static llvm::cl::opt<bool>
clCountDuplicateExecs("count-duplicate-execs", llvm::cl::cat(clDebugging),
		      llvm::cl::desc("Count duplicate executions (adds runtime overhead)"));

void Config::getConfigOptions(int argc, char **argv)
{
	const llvm::cl::OptionCategory *cats[] =
		{&clGeneral, &clDebugging, &clTransformation};

	/* Hide unrelated LLVM options and parse user configuration */
#ifdef LLVM_HAS_HIDE_UNRELATED_OPTS
	llvm::cl::HideUnrelatedOptions(cats);
#endif
	llvm::cl::ParseCommandLineOptions(argc, argv,
					  "GenMC -- Model Checking for C programs");
#ifdef LLVM_HAS_RESET_COMMANDLINE_PARSER
	llvm::cl::ResetCommandLineParser();
#endif

	/* Save general options */
	cflags.insert(cflags.end(), clCFLAGS.begin(), clCFLAGS.end());
	inputFile = clInputFile;
	specsFile = clLibrarySpecsFile;
	dotFile = clDotGraphFile;
	model = clModelType;
	printErrorTrace = clPrintErrorTrace;
	checkPscAcyclicity = clCheckPscAcyclicity;
	checkWbAcyclicity = clCheckWbAcyclicity;
	disableRaceDetection = clDisableRaceDetection;

	/* Save transformation options */
	unroll = clLoopUnroll;
	spinAssume = !clDisableSpinAssume;

	/* Save debugging options */
	programEntryFun = clProgramEntryFunction;
	validateExecGraphs = clValidateExecGraphs;
	randomizeSchedule = clRandomizeSchedule;
	printRandomizeScheduleSeed = clPrintRandomizeScheduleSeed;
	randomizeScheduleSeed = clRandomizeScheduleSeed;
	printExecGraphs = clPrintExecGraphs;
	prettyPrintExecGraphs = clPrettyPrintExecGraphs;
	countDuplicateExecs = clCountDuplicateExecs;
	inputFromBitcodeFile = clInputFromBitcodeFile;
	transformFile = clTransformFile;
}
