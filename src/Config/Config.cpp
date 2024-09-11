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
#include "Config/Config.hpp"
#include "Support/Error.hpp"
#include "Support/Logger.hpp"
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

static llvm::cl::list<std::string> clCFLAGS(llvm::cl::Positional, llvm::cl::ZeroOrMore,
					    llvm::cl::desc("-- [CFLAGS]"));

static llvm::cl::opt<std::string> clInputFile(llvm::cl::Positional, llvm::cl::Required,
					      llvm::cl::desc("<input file>"));

/*** Exploration options ***/

static llvm::cl::opt<ModelType> clModelType(
	llvm::cl::values(clEnumValN(ModelType::SC, "sc", "SC memory model"),
			 clEnumValN(ModelType::TSO, "tso", "TSO memory model"),
			 clEnumValN(ModelType::RA, "ra", "RA+RLX memory model"),
			 clEnumValN(ModelType::RC11, "rc11", "RC11 memory model (default)"),
			 clEnumValN(ModelType::IMM, "imm", "IMM memory model")),
	llvm::cl::cat(clGeneral), llvm::cl::init(ModelType::RC11),
	llvm::cl::desc("Choose model type:"));

static llvm::cl::opt<bool> clDisableEstimation(
	"disable-estimation", llvm::cl::cat(clGeneral),
	llvm::cl::desc("Do not estimate the state-space size before verifying the program"));

static llvm::cl::opt<unsigned int>
	clThreads("nthreads", llvm::cl::cat(clGeneral), llvm::cl::init(1),
		  llvm::cl::desc("Number of threads to be used in the exploration"));

static llvm::cl::opt<int>
	clBound("bound", llvm::cl::cat(clGeneral), llvm::cl::init(-1), llvm::cl::value_desc("N"),
		llvm::cl::desc("Do not explore executions exceeding given bound"));

static llvm::cl::opt<BoundType>
	clBoundType("bound-type", llvm::cl::cat(clGeneral), llvm::cl::init(BoundType::round),
		    llvm::cl::desc("Choose type for -bound:"),
		    llvm::cl::values(clEnumValN(BoundType::context, "context", "Context bound"),
				     clEnumValN(BoundType::round, "round", "Round-robin bound")));

#ifdef ENABLE_GENMC_DEBUG
static llvm::cl::opt<bool> clBoundsHistogram("bounds-histogram", llvm::cl::cat(clDebugging),
					     llvm::cl::desc("Produce bounds histogram"));
#endif /* ifdef ENABLE_GENMC_DEBUG */

static llvm::cl::opt<bool>
	clLAPOR("lapor", llvm::cl::cat(clGeneral),
		llvm::cl::desc("Enable Lock-Aware Partial Order Reduction (LAPOR)"));

static llvm::cl::opt<bool> clDisableSymmetryReduction("disable-sr", llvm::cl::cat(clGeneral),
						      llvm::cl::desc("Disable symmetry reduction"));

static llvm::cl::opt<bool> clHelper("helper", llvm::cl::cat(clGeneral),
				    llvm::cl::desc("Enable Helper for CDs verification"));

static llvm::cl::opt<bool> clPrintErrorTrace("print-error-trace", llvm::cl::cat(clGeneral),
					     llvm::cl::desc("Print error trace"));

static llvm::cl::opt<std::string>
	clDotGraphFile("dump-error-graph", llvm::cl::init(""), llvm::cl::value_desc("file"),
		       llvm::cl::cat(clGeneral),
		       llvm::cl::desc("Dump an error graph to a file (DOT format)"));

static llvm::cl::opt<bool> clCheckLiveness("check-liveness", llvm::cl::cat(clGeneral),
					   llvm::cl::desc("Check for liveness violations"));

static llvm::cl::opt<bool> clDisableInstructionCaching(
	"disable-instruction-caching", llvm::cl::cat(clGeneral),
	llvm::cl::desc("Disable instruction caching (pure stateless exploration)"));

static llvm::cl::opt<bool> clDisableRaceDetection("disable-race-detection",
						  llvm::cl::cat(clGeneral),
						  llvm::cl::desc("Disable race detection"));

static llvm::cl::opt<bool> clDisableBAM("disable-bam", llvm::cl::cat(clGeneral),
					llvm::cl::desc("Disable optimized barrier handling (BAM)"));
static llvm::cl::opt<bool> clDisableIPR("disable-ipr", llvm::cl::cat(clGeneral),
					llvm::cl::desc("Disable in-place revisiting"));
static llvm::cl::opt<bool>
	clDisableStopOnSystemError("disable-stop-on-system-error", llvm::cl::cat(clGeneral),
				   llvm::cl::desc("Do not stop verification on system errors"));
static llvm::cl::opt<bool>
	clDisableWarnUnfreedMemory("disable-warn-on-unfreed-memory", llvm::cl::cat(clGeneral),
				   llvm::cl::desc("Do not warn about unfreed memory"));

/*** Persistency options ***/

static llvm::cl::opt<bool> clPersevere("persevere", llvm::cl::cat(clPersistency),
				       llvm::cl::desc("Enable persistency checks (Persevere)"));

static llvm::cl::opt<unsigned int> clBlockSize("block-size", llvm::cl::cat(clPersistency),
					       llvm::cl::init(2),
					       llvm::cl::desc("Block size (in bytes)"));

static llvm::cl::opt<unsigned int> clMaxFileSize("max-file-size", llvm::cl::cat(clPersistency),
						 llvm::cl::init(64),
						 llvm::cl::desc("Maximum file size (in bytes)"));

static llvm::cl::opt<JournalDataFS> clJournalData(
	"journal-data", llvm::cl::cat(clPersistency), llvm::cl::init(JournalDataFS::ordered),
	llvm::cl::desc("Specify the journaling mode for file data:"),
	llvm::cl::values(clEnumValN(JournalDataFS::writeback, "writeback",
				    "Data ordering not preserved"),
			 clEnumValN(JournalDataFS::ordered, "ordered", "Data before metadata"),
			 clEnumValN(JournalDataFS::journal, "journal", "Journal data")));

static llvm::cl::opt<bool> clDisableDelalloc("disable-delalloc", llvm::cl::cat(clPersistency),
					     llvm::cl::desc("Do not model delayed allocation"));

/*** Transformation options ***/

static llvm::cl::opt<int> clLoopUnroll("unroll", llvm::cl::init(-1), llvm::cl::value_desc("N"),
				       llvm::cl::cat(clTransformation),
				       llvm::cl::desc("Unroll loops N times"));

static llvm::cl::list<std::string> clNoUnrollFuns("no-unroll", llvm::cl::value_desc("fun_name"),
						  llvm::cl::cat(clTransformation),
						  llvm::cl::desc("Do not unroll this function"));

static llvm::cl::opt<bool>
	clDisableLoopJumpThreading("disable-loop-jump-threading", llvm::cl::cat(clTransformation),
				   llvm::cl::desc("Disable loop-jump-threading transformation"));

static llvm::cl::opt<bool>
	clDisableCastElimination("disable-cast-elimination", llvm::cl::cat(clTransformation),
				 llvm::cl::desc("Disable cast-elimination transformation"));

static llvm::cl::opt<bool>
	clDisableFunctionInliner("disable-function-inliner", llvm::cl::cat(clTransformation),
				 llvm::cl::desc("Disable function-inlining transformation"));

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

static llvm::cl::opt<bool> clDisableMMDetector("disable-mm-detector",
					       llvm::cl::cat(clTransformation),
					       llvm::cl::desc("Disable MM detector pass"));

/*** Debugging options ***/

static llvm::cl::opt<unsigned int> clEstimationMax(
	"estimation-max", llvm::cl::init(1000), llvm::cl::value_desc("N"),
	llvm::cl::cat(clDebugging),
	llvm::cl::desc("Number of maximum allotted rounds for state-space estimation"));

static llvm::cl::opt<unsigned int> clEstimationMin(
	"estimation-min", llvm::cl::init(10), llvm::cl::value_desc("N"), llvm::cl::cat(clDebugging),
	llvm::cl::desc("Number of minimum alloted round for state-space estimation"));

static llvm::cl::opt<unsigned int> clEstimationSdThreshold(
	"estimation-threshold", llvm::cl::init(10), llvm::cl::value_desc("N"),
	llvm::cl::cat(clDebugging),
	llvm::cl::desc("Deviation threshold % under which estimation is deemed good enough"));

static llvm::cl::opt<std::string> clProgramEntryFunction(
	"program-entry-function", llvm::cl::init("main"), llvm::cl::value_desc("fun_name"),
	llvm::cl::cat(clDebugging),
	llvm::cl::desc("Function used as program entrypoint (default: main())"));

static llvm::cl::opt<bool>
	clInputFromBitcodeFile("input-from-bitcode-file", llvm::cl::cat(clDebugging),
			       llvm::cl::desc("The input file contains LLVM bitcode"));

static llvm::cl::opt<std::string>
	clTransformFile("transform-output", llvm::cl::init(""), llvm::cl::value_desc("file"),
			llvm::cl::cat(clDebugging),
			llvm::cl::desc("Output the transformed LLVM code to file"));
static llvm::cl::opt<unsigned int>
	clWarnOnGraphSize("warn-on-graph-size", llvm::cl::init(1024), llvm::cl::value_desc("N"),
			  llvm::cl::cat(clDebugging),
			  llvm::cl::desc("Warn about graphs larger than N"));
llvm::cl::opt<SchedulePolicy> clSchedulePolicy(
	"schedule-policy", llvm::cl::cat(clDebugging), llvm::cl::init(SchedulePolicy::wf),
	llvm::cl::desc("Choose the scheduling policy:"),
	llvm::cl::values(clEnumValN(SchedulePolicy::ltr, "ltr", "Left-to-right"),
			 clEnumValN(SchedulePolicy::wf, "wf", "Writes-first (default)"),
			 clEnumValN(SchedulePolicy::wfr, "wfr", "Writes-first-random"),
			 clEnumValN(SchedulePolicy::arbitrary, "arbitrary", "Arbitrary")));

static llvm::cl::opt<bool> clPrintArbitraryScheduleSeed(
	"print-schedule-seed", llvm::cl::cat(clDebugging),
	llvm::cl::desc("Print the seed used for arbitrary scheduling"));

static llvm::cl::opt<std::string>
	clArbitraryScheduleSeed("schedule-seed", llvm::cl::init(""), llvm::cl::value_desc("seed"),
				llvm::cl::cat(clDebugging),
				llvm::cl::desc("Seed to be used for arbitrary scheduling"));

static llvm::cl::opt<bool> clPrintExecGraphs("print-exec-graphs", llvm::cl::cat(clDebugging),
					     llvm::cl::desc("Print explored execution graphs"));

static llvm::cl::opt<bool> clPrintBlockedExecs("print-blocked-execs", llvm::cl::cat(clDebugging),
					       llvm::cl::desc("Print blocked execution graphs"));

llvm::cl::opt<VerbosityLevel> clVLevel(
	llvm::cl::cat(clDebugging), llvm::cl::init(VerbosityLevel::Tip),
	llvm::cl::desc("Choose verbosity level:"),
	llvm::cl::values(clEnumValN(VerbosityLevel::Quiet, "v0", "Quiet (no logging)"),
			 clEnumValN(VerbosityLevel::Error, "v1", "Print errors only"),
			 clEnumValN(VerbosityLevel::Warning, "v2", "Print warnings"),
			 clEnumValN(VerbosityLevel::Tip, "v3", "Print tips (default)")
#ifdef ENABLE_GENMC_DEBUG
				 ,
			 clEnumValN(VerbosityLevel::Debug1, "v4", "Print revisits considered"),
			 clEnumValN(VerbosityLevel::Debug2, "v5",
				    "Print graph after each memory access"),
			 clEnumValN(VerbosityLevel::Debug3, "v6", "Print rf options considered")
#endif /* ifdef ENABLE_GENMC_DEBUG */
				 ));

#ifdef ENABLE_GENMC_DEBUG
static llvm::cl::opt<bool> clPrintStamps("print-stamps", llvm::cl::cat(clDebugging),
					 llvm::cl::desc("Print stamps in execution graphs"));

static llvm::cl::opt<bool>
	clColorAccesses("color-accesses", llvm::cl::cat(clDebugging),
			llvm::cl::desc("Color accesses depending on revisitability"));

static llvm::cl::opt<bool>
	clValidateExecGraphs("validate-exec-graphs", llvm::cl::cat(clDebugging),
			     llvm::cl::desc("Validate the execution graphs in each step"));

static llvm::cl::opt<bool>
	clCountDuplicateExecs("count-duplicate-execs", llvm::cl::cat(clDebugging),
			      llvm::cl::desc("Count duplicate executions (adds runtime overhead)"));

static llvm::cl::opt<bool> clCountMootExecs("count-moot-execs", llvm::cl::cat(clDebugging),
					    llvm::cl::desc("Count moot executions"));

static llvm::cl::opt<bool> clPrintEstimationStats("print-estimation-stats",
						  llvm::cl::cat(clDebugging),
						  llvm::cl::desc("Prints estimations statistics"));
#endif /* ENABLE_GENMC_DEBUG */

void printVersion(llvm::raw_ostream &s)
{
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
	if (clLAPOR) {
		ERROR("LAPOR is temporarily disabled.\n");
	}
	if (clHelper && clSchedulePolicy == SchedulePolicy::arbitrary) {
		ERROR("Helper cannot be used with -schedule-policy=arbitrary.\n");
	}
	if (clModelType == ModelType::IMM && (!clDisableIPR || !clDisableSymmetryReduction)) {
		WARN("In-place revisiting and symmetry reduction have no effect under IMM\n");
		clDisableSymmetryReduction = true;
		clDisableIPR = true;
	}

	/* Check debugging options */
	if (clSchedulePolicy != SchedulePolicy::arbitrary && clPrintArbitraryScheduleSeed) {
		WARN("--print-schedule-seed used without -schedule-policy=arbitrary.\n");
	}
	if (clSchedulePolicy != SchedulePolicy::arbitrary && !clArbitraryScheduleSeed.empty()) {
		WARN("--schedule-seed used without -schedule-policy=arbitrary.\n");
	}

	/* Check bounding options */
	if (clBound != -1 && clModelType != ModelType::SC) {
		ERROR("Bounding can only be used with --sc.\n");
	}
	GENMC_DEBUG(ERROR_ON(clBound != -1 && clBoundsHistogram,
			     "Bounds histogram cannot be used when bounding.\n"););
	if (clBound == -1 && clBoundType.getNumOccurrences() > 0) {
		WARN("--bound-type used without --bound.\n");
	}

	/* Sanitize bounding options */
	bool bounding = (clBound != -1);
	GENMC_DEBUG(bounding |= clBoundsHistogram;);
	if (bounding && (clLAPOR || clHelper || !clDisableBAM || !clDisableSymmetryReduction ||
			 !clDisableIPR || clSchedulePolicy != SchedulePolicy::ltr)) {
		WARN("LAPOR/Helper/BAM/SR/IPR have no effect when --bound is used. Scheduling "
		     "defaults to LTR.\n");
		clLAPOR = clHelper = false;
		clDisableBAM = clDisableSymmetryReduction = clDisableIPR = true;
		clSchedulePolicy = SchedulePolicy::ltr;
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
	estimate = !clDisableEstimation;
	estimationMax = clEstimationMax;
	estimationMin = clEstimationMin;
	sdThreshold = clEstimationSdThreshold;
	isDepTrackingModel = (model == ModelType::IMM);
	threads = clThreads;
	bound = clBound >= 0 ? std::optional(clBound.getValue()) : std::nullopt;
	boundType = clBoundType;
	LAPOR = clLAPOR;
	symmetryReduction = !clDisableSymmetryReduction;
	helper = clHelper;
	printErrorTrace = clPrintErrorTrace;
	checkLiveness = clCheckLiveness;
	instructionCaching = !clDisableInstructionCaching;
	disableRaceDetection = clDisableRaceDetection;
	disableBAM = clDisableBAM;
	ipr = !clDisableIPR;
	disableStopOnSystemError = clDisableStopOnSystemError;
	warnUnfreedMemory = !clDisableWarnUnfreedMemory;

	/* Save persistency options */
	persevere = clPersevere;
	blockSize = clBlockSize;
	maxFileSize = clMaxFileSize;
	journalData = clJournalData;
	disableDelalloc = clDisableDelalloc;

	/* Save transformation options */
	unroll = clLoopUnroll >= 0 ? std::optional(clLoopUnroll.getValue()) : std::nullopt;
	noUnrollFuns.insert(clNoUnrollFuns.begin(), clNoUnrollFuns.end());
	loopJumpThreading = !clDisableLoopJumpThreading;
	castElimination = !clDisableCastElimination;
	inlineFunctions = !clDisableFunctionInliner;
	spinAssume = !clDisableSpinAssume;
	codeCondenser = !clDisableCodeCondenser;
	loadAnnot = !clDisableLoadAnnot;
	assumePropagation = !clDisableAssumePropagation;
	confirmAnnot = !clDisableConfirmAnnot;
	mmDetector = !clDisableMMDetector;

	/* Save debugging options */
	programEntryFun = clProgramEntryFunction;
	warnOnGraphSize = clWarnOnGraphSize;
	schedulePolicy = clSchedulePolicy;
	printRandomScheduleSeed = clPrintArbitraryScheduleSeed;
	randomScheduleSeed = clArbitraryScheduleSeed;
	printExecGraphs = clPrintExecGraphs;
	printBlockedExecs = clPrintBlockedExecs;
	inputFromBitcodeFile = clInputFromBitcodeFile;
	transformFile = clTransformFile;
	vLevel = clVLevel;
#ifdef ENABLE_GENMC_DEBUG
	printStamps = clPrintStamps;
	colorAccesses = clColorAccesses;
	validateExecGraphs = clValidateExecGraphs;
	countDuplicateExecs = clCountDuplicateExecs;
	countMootExecs = clCountMootExecs;
	printEstimationStats = clPrintEstimationStats;
	boundsHistogram = clBoundsHistogram;
#endif
	/* Set (global) log state */
	logLevel = vLevel;
}

void Config::getConfigOptions(int argc, char **argv)
{
	/* Option categories printed */
	const llvm::cl::OptionCategory *cats[] = {&clGeneral, &clDebugging, &clTransformation,
						  &clPersistency};

	llvm::cl::SetVersionPrinter(printVersion);

	/* Hide unrelated LLVM options and parse user configuration */
	llvm::cl::HideUnrelatedOptions(cats);
	llvm::cl::ParseCommandLineOptions(argc, argv,
					  "GenMC -- "
					  "Model Checking for C programs");

	/* Sanity-check config options and then appropriately save them */
	checkConfigOptions();
	saveConfigOptions();

	llvm::cl::ResetAllOptionOccurrences();
	clInputFile.removeArgument();
}
