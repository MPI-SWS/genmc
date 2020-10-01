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
#include "Error.hpp"
#include <llvm/ADT/ArrayRef.h>	// needed for 3.5
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

/* Command-line argument categories */

static llvm::cl::OptionCategory clGeneral("Exploration Options");
static llvm::cl::OptionCategory clPersistency("Persistency Options");
static llvm::cl::OptionCategory clTransformation("Transformation Options");
static llvm::cl::OptionCategory clDebugging("Debugging Options");


/* General syntax */

llvm::cl::list<std::string>
clCFLAGS(llvm::cl::Positional, llvm::cl::ZeroOrMore, llvm::cl::desc("-- [CFLAGS]"));
static llvm::cl::opt<std::string>
clInputFile(llvm::cl::Positional, llvm::cl::Required, llvm::cl::desc("<input file>"));


/* Exploration options */

llvm::cl::opt<ModelType>
clModelType(llvm::cl::values(
		    clEnumValN(ModelType::rc11, "rc11", "RC11 memory model"),
		    clEnumValN(ModelType::imm, "imm",       "IMM model")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ),
	    llvm::cl::cat(clGeneral),
	    llvm::cl::init(ModelType::rc11),
	    llvm::cl::desc("Choose model type:"));
llvm::cl::opt<CoherenceType>
clCoherenceType(llvm::cl::values(
			clEnumValN(CoherenceType::mo, "mo", "Track modification order"),
			clEnumValN(CoherenceType::wb, "wb", "Calculate writes-before")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
			, NULL
#endif
			),
		llvm::cl::cat(clGeneral),
		llvm::cl::init(CoherenceType::wb),
		llvm::cl::desc("Choose coherence type:"));
llvm::cl::opt<bool>
clLAPOR("lapor", llvm::cl::cat(clGeneral),
	llvm::cl::desc("Enable Lock-Aware Partial Order Reduction (LAPOR)"));
llvm::cl::opt<bool>
clSymmetryReduction("sr", llvm::cl::cat(clGeneral),
		    llvm::cl::desc("Enable Symmetry Reduction"));
static llvm::cl::opt<bool>
clPrintErrorTrace("print-error-trace", llvm::cl::cat(clGeneral),
		  llvm::cl::desc("Print error trace"));
static llvm::cl::opt<std::string>
clDotGraphFile("dump-error-graph", llvm::cl::init(""), llvm::cl::value_desc("file"),
	       llvm::cl::cat(clGeneral),
	       llvm::cl::desc("Dumps an error graph to a file (DOT format)"));
static llvm::cl::opt<CheckConsType>
clCheckConsType("check-consistency-type", llvm::cl::init(CheckConsType::approx), llvm::cl::cat(clGeneral),
		llvm::cl::desc("Type of consistency checks"),
		llvm::cl::values(
			clEnumValN(CheckConsType::approx, "approx", "Approximate checks"),
			clEnumValN(CheckConsType::full,   "full",   "Full checks")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ));
static llvm::cl::opt<ProgramPoint>
clCheckConsPoint("check-consistency-point", llvm::cl::init(ProgramPoint::error), llvm::cl::cat(clGeneral),
		 llvm::cl::desc("Points at which consistency is checked"),
		 llvm::cl::values(
			 clEnumValN(ProgramPoint::error, "error", "At errors only"),
			 clEnumValN(ProgramPoint::exec,  "exec",  "At the end of each execution"),
			 clEnumValN(ProgramPoint::step,  "step",  "At each program step")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ));
static llvm::cl::opt<std::string>
clLibrarySpecsFile("library-specs", llvm::cl::init(""), llvm::cl::value_desc("file"),
		   llvm::cl::cat(clGeneral),
		   llvm::cl::desc("Check for library correctness"));
static llvm::cl::opt<bool>
clDisableRaceDetection("disable-race-detection", llvm::cl::cat(clGeneral),
		     llvm::cl::desc("Disable race detection"));
static llvm::cl::opt<bool>
clDisableStopOnSystemError("disable-stop-on-system-error", llvm::cl::cat(clGeneral),
			   llvm::cl::desc("Do not stop verification on system errors"));


/* Persistency options */

static llvm::cl::opt<bool>
clPersevere("persevere", llvm::cl::cat(clPersistency),
	    llvm::cl::desc("Enable persistency checks (Persevere)"));
static llvm::cl::opt<ProgramPoint>
clCheckPersPoint("check-persistency-point", llvm::cl::init(ProgramPoint::step), llvm::cl::cat(clPersistency),
		 llvm::cl::desc("Points at which persistency is checked"),
		 llvm::cl::values(
			 clEnumValN(ProgramPoint::error, "error", "At errors only"),
			 clEnumValN(ProgramPoint::exec,  "exec",  "At the end of each execution"),
			 clEnumValN(ProgramPoint::step,  "step",  "At each program step")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ));
static llvm::cl::opt<unsigned int>
clBlockSize("block-size", llvm::cl::cat(clPersistency), llvm::cl::init(2),
	      llvm::cl::desc("Block size (in bytes)"));
static llvm::cl::opt<unsigned int>
clMaxFileSize("max-file-size", llvm::cl::cat(clPersistency), llvm::cl::init(64),
	      llvm::cl::desc("Maximum file size (in bytes)"));
static llvm::cl::opt<JournalDataFS>
clJournalData("journal-data", llvm::cl::cat(clPersistency), llvm::cl::init(JournalDataFS::ordered),
	      llvm::cl::desc("Specify the journaling mode for file data:"),
	      llvm::cl::values(
		      clEnumValN(JournalDataFS::writeback, "writeback", "Data ordering not preserved"),
		      clEnumValN(JournalDataFS::ordered,   "ordered",   "Data before metadata"),
		      clEnumValN(JournalDataFS::journal,   "journal",   "Journal data")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		      , NULL
#endif
		      ));
static llvm::cl::opt<bool>
clDisableDelalloc("disable-delalloc", llvm::cl::cat(clPersistency),
		  llvm::cl::desc("Do not model delayed allocation"));


/* Transformation options */

static llvm::cl::opt<int>
clLoopUnroll("unroll", llvm::cl::init(-1), llvm::cl::value_desc("N"),
	     llvm::cl::cat(clTransformation),
	     llvm::cl::desc("Unroll loops N times"));
static llvm::cl::opt<bool>
clDisableSpinAssume("disable-spin-assume", llvm::cl::cat(clTransformation),
		    llvm::cl::desc("Disable spin-assume transformation"));


/* Debugging options */

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
llvm::cl::opt<SchedulePolicy>
clSchedulePolicy("schedule-policy", llvm::cl::cat(clDebugging), llvm::cl::init(SchedulePolicy::wf),
		 llvm::cl::desc("Choose the scheduling policy:"),
		 llvm::cl::values(
			 clEnumValN(SchedulePolicy::ltr,     "ltr",      "Left-to-right"),
			 clEnumValN(SchedulePolicy::wf,      "wf",       "Writes-first (default)"),
			 clEnumValN(SchedulePolicy::random,  "random",   "Random")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
			 , NULL
#endif
			 ));
static llvm::cl::opt<bool>
clPrintRandomScheduleSeed("print-random-schedule-seed", llvm::cl::cat(clDebugging),
			     llvm::cl::desc("Print the seed used for randomized scheduling"));
static llvm::cl::opt<std::string>
clRandomScheduleSeed("random-schedule-seed", llvm::cl::init(""),
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

#ifdef LLVM_SETVERSIONPRINTER_NEEDS_ARG
void printVersion(llvm::raw_ostream &s)
{
#else
void printVersion()
{
	auto &s = llvm::outs();
#endif
	s << PACKAGE_NAME " (" PACKAGE_URL "):\n"
	  << "  " PACKAGE_NAME " v" PACKAGE_VERSION
#ifdef GIT_COMMIT
	  << " (commit #" GIT_COMMIT ")"
#endif
	  << "\n  Built with LLVM " LLVM_VERSION " (" LLVM_BUILDMODE ")\n";
}

void Config::checkConfigOptions() const
{
	/* Check exploration options */
	if (clLAPOR && clCoherenceType == CoherenceType::mo) {
		WARN("LAPOR usage with -mo is experimental.\n");
	}

	/* Check debugging options */
	if (schedulePolicy != SchedulePolicy::random && clPrintRandomScheduleSeed) {
		WARN("--print-random-schedule-seed used without -schedule-policy=random.\n");
	}
	if (schedulePolicy != SchedulePolicy::random && clRandomScheduleSeed != "") {
		WARN("--random-schedule-seed used without -schedule-policy=random.\n");
	}
}

void Config::saveConfigOptions()
{
	/* General syntax */
	cflags.insert(cflags.end(), clCFLAGS.begin(), clCFLAGS.end());
	inputFile = clInputFile;

	/* Save exploration options */
	specsFile = clLibrarySpecsFile;
	dotFile = clDotGraphFile;
	model = clModelType;
	isDepTrackingModel = (model == ModelType::imm);
	coherence = clCoherenceType;
	LAPOR = clLAPOR;
	symmetryReduction = clSymmetryReduction;
	printErrorTrace = clPrintErrorTrace;
	checkConsType = clCheckConsType;
	checkConsPoint = clCheckConsPoint;
	disableRaceDetection = clDisableRaceDetection;
	disableStopOnSystemError = clDisableStopOnSystemError;

	/* Save persistency options */
	persevere = clPersevere;
	checkPersPoint = clCheckPersPoint;
	blockSize = clBlockSize;
	maxFileSize = clMaxFileSize;
	journalData = clJournalData;
	disableDelalloc = clDisableDelalloc;

	/* Save transformation options */
	unroll = clLoopUnroll;
	spinAssume = !clDisableSpinAssume;

	/* Save debugging options */
	programEntryFun = clProgramEntryFunction;
	validateExecGraphs = clValidateExecGraphs;
	schedulePolicy = clSchedulePolicy;
	printRandomScheduleSeed = clPrintRandomScheduleSeed;
	randomScheduleSeed = clRandomScheduleSeed;
	printExecGraphs = clPrintExecGraphs;
	prettyPrintExecGraphs = clPrettyPrintExecGraphs;
	countDuplicateExecs = clCountDuplicateExecs;
	inputFromBitcodeFile = clInputFromBitcodeFile;
	transformFile = clTransformFile;
}

void Config::getConfigOptions(int argc, char **argv)
{
	/* Option categories printed */
	const llvm::cl::OptionCategory *cats[] =
		{&clGeneral, &clDebugging, &clTransformation, &clPersistency};

	llvm::cl::SetVersionPrinter(printVersion);

	/* Hide unrelated LLVM options and parse user configuration */
#ifdef LLVM_HAS_HIDE_UNRELATED_OPTS
	llvm::cl::HideUnrelatedOptions(cats);
#endif
	llvm::cl::ParseCommandLineOptions(argc, argv, "GenMC -- "
					  "Model Checking for C programs");

	/* Sanity-check config options and then appropriately save them */
	checkConfigOptions();
	saveConfigOptions();

#ifdef LLVM_HAS_RESET_COMMANDLINE_PARSER
	llvm::cl::ResetCommandLineParser();
#endif
}
