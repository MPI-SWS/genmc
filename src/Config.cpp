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
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <thread>

/*** Command-line argument categories ***/

static llvm::cl::OptionCategory clGeneral("Exploration Options");
static llvm::cl::OptionCategory clPersistency("Persistency Options");
static llvm::cl::OptionCategory clTransformation("Transformation Options");
static llvm::cl::OptionCategory clDebugging("Debugging Options");


/*** General syntax ***/

static llvm::cl::list<std::string>
clCFLAGS(llvm::cl::Positional, llvm::cl::ZeroOrMore, llvm::cl::desc("-- [CFLAGS]"));

static llvm::cl::opt<std::string>
clInputFile(llvm::cl::Positional, llvm::cl::Required, llvm::cl::desc("<input file>"));


/*** Exploration options ***/

static llvm::cl::opt<ModelType>
clModelType(llvm::cl::values(
		    clEnumValN(ModelType::rc11, "rc11", "RC11 memory model"),
		    clEnumValN(ModelType::imm,  "imm",  "IMM memory model"),
		    clEnumValN(ModelType::lkmm, "lkmm", "LKMM memory model")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		    , NULL
#endif
		    ),
	    llvm::cl::cat(clGeneral),
	    llvm::cl::init(ModelType::rc11),
	    llvm::cl::desc("Choose model type:"));

static llvm::cl::opt<CoherenceType>
clCoherenceType(llvm::cl::values(
			clEnumValN(CoherenceType::mo, "mo", "Track modification order"),
			clEnumValN(CoherenceType::wb, "wb", "Calculate writes-before")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
			, NULL
#endif
			),
		llvm::cl::cat(clGeneral),
		llvm::cl::init(CoherenceType::mo),
		llvm::cl::desc("Choose coherence type:"));

static llvm::cl::opt<unsigned int>
clThreads("nthreads", llvm::cl::cat(clGeneral), llvm::cl::init(1),
	      llvm::cl::desc("Number of threads to be used in the exploration"));

static llvm::cl::opt<bool>
clLAPOR("lapor", llvm::cl::cat(clGeneral),
	llvm::cl::desc("Enable Lock-Aware Partial Order Reduction (LAPOR)"));

static llvm::cl::opt<bool>
clSymmetryReduction("sr", llvm::cl::cat(clGeneral),
		    llvm::cl::desc("Enable Symmetry Reduction"));

static llvm::cl::opt<bool>
clHelper("helper", llvm::cl::cat(clGeneral),
	 llvm::cl::desc("Enable Helper for CDs verification"));

static llvm::cl::opt<bool>
clPrintErrorTrace("print-error-trace", llvm::cl::cat(clGeneral),
		  llvm::cl::desc("Print error trace"));

static llvm::cl::opt<std::string>
clDotGraphFile("dump-error-graph", llvm::cl::init(""), llvm::cl::value_desc("file"),
	       llvm::cl::cat(clGeneral),
	       llvm::cl::desc("Dump an error graph to a file (DOT format)"));

static llvm::cl::opt<CheckConsType>
clCheckConsType("check-consistency-type", llvm::cl::init(CheckConsType::slow), llvm::cl::cat(clGeneral),
		llvm::cl::desc("Type of (configurable) consistency checks"),
		llvm::cl::values(
			clEnumValN(CheckConsType::slow, "slow", "Approximation check"),
			clEnumValN(CheckConsType::full, "full", "Full checks")
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

static llvm::cl::opt<bool>
clCheckLiveness("check-liveness", llvm::cl::cat(clGeneral),
		llvm::cl::desc("Check for liveness violations"));

static llvm::cl::opt<bool>
clDisableRaceDetection("disable-race-detection", llvm::cl::cat(clGeneral),
		     llvm::cl::desc("Disable race detection"));

static llvm::cl::opt<bool>
clDisableBAM("disable-bam", llvm::cl::cat(clGeneral),
	     llvm::cl::desc("Disable optimized barrier handling (BAM)"));
static llvm::cl::opt<bool>
clDisableStopOnSystemError("disable-stop-on-system-error", llvm::cl::cat(clGeneral),
			   llvm::cl::desc("Do not stop verification on system errors"));


/*** Persistency options ***/

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


/*** Transformation options ***/

static llvm::cl::opt<int>
clLoopUnroll("unroll", llvm::cl::init(-1), llvm::cl::value_desc("N"),
	     llvm::cl::cat(clTransformation),
	     llvm::cl::desc("Unroll loops N times"));

static llvm::cl::list<std::string>
clNoUnrollFuns("no-unroll", llvm::cl::value_desc("fun_name"),
	       llvm::cl::cat(clTransformation),
	       llvm::cl::desc("Do not unroll this function"));


static llvm::cl::opt<bool>
clDisableLoopJumpThreading("disable-loop-jump-threading", llvm::cl::cat(clTransformation),
			   llvm::cl::desc("Disable loop-jump-threading transformation"));

static llvm::cl::opt<bool>
clDisableCastElimination("disable-cast-elimination", llvm::cl::cat(clTransformation),
			   llvm::cl::desc("Disable cast-elimination transformation"));
static llvm::cl::opt<bool>
clDisableSpinAssume("disable-spin-assume", llvm::cl::cat(clTransformation),
		    llvm::cl::desc("Disable spin-assume transformation"));

static llvm::cl::opt<bool>
clDisableCodeCondenser("disable-code-condenser", llvm::cl::cat(clTransformation),
		       llvm::cl::desc("Disable code-condenser transformation"));

static llvm::cl::opt<bool>
clDisableLoadAnnot("disable-load-annotation", llvm::cl::cat(clTransformation),
		   llvm::cl::desc("Disable load-annotation transformation"));

static llvm::cl::opt<bool>
clDisableAssumePropagation("disable-assume-propagation", llvm::cl::cat(clTransformation),
			   llvm::cl::desc("Disable assume-propagation transformation"));

static llvm::cl::opt<bool>
clDisableConfirmAnnot("disable-confirmation-annotation", llvm::cl::cat(clTransformation),
		      llvm::cl::desc("Disable confirmation-annotation transformation"));


/*** Debugging options ***/

static llvm::cl::opt<std::string>
clProgramEntryFunction("program-entry-function", llvm::cl::init("main"),
		       llvm::cl::value_desc("fun_name"), llvm::cl::cat(clDebugging),
		       llvm::cl::desc("Function used as program entrypoint (default: main())"));

static llvm::cl::opt<bool>
clInputFromBitcodeFile("input-from-bitcode-file", llvm::cl::cat(clDebugging),
		       llvm::cl::desc("The input file contains LLVM bitcode"));

static llvm::cl::opt<std::string>
clTransformFile("transform-output", llvm::cl::init(""),	llvm::cl::value_desc("file"),
		llvm::cl::cat(clDebugging),
		llvm::cl::desc("Output the transformed LLVM code to file"));
static llvm::cl::opt<unsigned int>
clWarnOnGraphSize("warn-on-graph-size", llvm::cl::init(42042), llvm::cl::value_desc("N"),
		  llvm::cl::cat(clDebugging), llvm::cl::desc("Warn about graphs larger than N"));
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


#ifdef ENABLE_GENMC_DEBUG
static llvm::cl::opt<bool>
clPrintBlockedExecs("print-blocked-execs", llvm::cl::cat(clDebugging),
		    llvm::cl::desc("Print blocked execution graphs"));

static llvm::cl::opt<bool>
clColorAccesses("color-accesses", llvm::cl::cat(clDebugging),
		llvm::cl::desc("Color accesses depending on revisitability"));

static llvm::cl::opt<bool>
clValidateExecGraphs("validate-exec-graphs", llvm::cl::cat(clDebugging),
		     llvm::cl::desc("Validate the execution graphs in each step"));

static llvm::cl::opt<bool>
clCountDuplicateExecs("count-duplicate-execs", llvm::cl::cat(clDebugging),
		      llvm::cl::desc("Count duplicate executions (adds runtime overhead)"));

llvm::cl::opt<VerbosityLevel>
clVLevel(llvm::cl::cat(clDebugging), llvm::cl::init(VerbosityLevel::V0),
	 llvm::cl::desc("Choose verbosity level:"),
	 llvm::cl::values(
		 clEnumValN(VerbosityLevel::V0, "v0", "No verbosity"),
		 clEnumValN(VerbosityLevel::V1, "v1", "Print stamps on executions"),
		 clEnumValN(VerbosityLevel::V2, "v2", "Print restricted executions"),
		 clEnumValN(VerbosityLevel::V3, "v3", "Print execution after each instruction")
#ifdef LLVM_CL_VALUES_NEED_SENTINEL
		 , NULL
#endif
		 ));
#endif /* ENABLE_GENMC_DEBUG */


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
	if (clCheckLiveness && clCoherenceType != CoherenceType::mo) {
		ERROR("-check-liveness can only be used with -mo.\n");
	}
	if (clLAPOR && clModelType == ModelType::lkmm) {
		ERROR("LAPOR usage is temporarily disabled under LKMM.\n");
	}
	if (clLAPOR && clCheckConsPoint < ProgramPoint::step) {
		WARN("LAPOR requires pointwise consistency steps.\n");
	}
	if (clLAPOR) {
		ERROR("LAPOR is temporarily disabled.\n");
	}
	if (clHelper && clSchedulePolicy == SchedulePolicy::random) {
		ERROR("Helper cannot be used with -schedule-policy=random.\n");
	}
	if (clHelper && clCoherenceType != CoherenceType::mo) {
		ERROR("Helper can only be used with -mo.\n");
	}

	/* Check debugging options */
	if (clSchedulePolicy != SchedulePolicy::random && clPrintRandomScheduleSeed) {
		WARN("--print-random-schedule-seed used without -schedule-policy=random.\n");
	}
	if (clSchedulePolicy != SchedulePolicy::random && clRandomScheduleSeed != "") {
		WARN("--random-schedule-seed used without -schedule-policy=random.\n");
	}

	/* Make sure filename is a regular file */
	if (!llvm::sys::fs::is_regular_file(clInputFile))
		ERROR("Input file is not a regular file!\n");
}

void Config::saveConfigOptions()
{
	/* General syntax */
	cflags.insert(cflags.end(), clCFLAGS.begin(), clCFLAGS.end());
	inputFile = clInputFile;

	/* Save exploration options */
	dotFile = clDotGraphFile;
	model = clModelType;
	isDepTrackingModel = (model == ModelType::imm || model == ModelType::lkmm);
	coherence = clCoherenceType;
	threads = clThreads;
	LAPOR = clLAPOR;
	symmetryReduction = clSymmetryReduction;
	helper = clHelper;
	printErrorTrace = clPrintErrorTrace;
	checkConsType = clCheckConsType;
	checkConsPoint = (LAPOR ? ProgramPoint::step : clCheckConsPoint);
	checkLiveness = clCheckLiveness;
	disableRaceDetection = clDisableRaceDetection;
	disableBAM = clDisableBAM;
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
	noUnrollFuns.insert(clNoUnrollFuns.begin(), clNoUnrollFuns.end());
	loopJumpThreading = !clDisableLoopJumpThreading;
	castElimination = !clDisableCastElimination;
	spinAssume = !clDisableSpinAssume;
	codeCondenser = !clDisableCodeCondenser;
	loadAnnot = !clDisableLoadAnnot;
	assumePropagation = !clDisableAssumePropagation;
	confirmAnnot = !clDisableConfirmAnnot;

	/* Save debugging options */
	programEntryFun = clProgramEntryFunction;
	warnOnGraphSize = clWarnOnGraphSize;
	schedulePolicy = clSchedulePolicy;
	printRandomScheduleSeed = clPrintRandomScheduleSeed;
	randomScheduleSeed = clRandomScheduleSeed;
	printExecGraphs = clPrintExecGraphs;
	inputFromBitcodeFile = clInputFromBitcodeFile;
	transformFile = clTransformFile;
#ifdef ENABLE_GENMC_DEBUG
	printBlockedExecs = clPrintBlockedExecs;
	colorAccesses = clColorAccesses;
	validateExecGraphs = clValidateExecGraphs;
	countDuplicateExecs = clCountDuplicateExecs;
	vLevel = clVLevel;
#endif
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
	llvm::cl::ResetAllOptionOccurrences();
	clInputFile.removeArgument();
#endif
}
