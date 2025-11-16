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

#include "Runtime/Interpreter.h"
#include "Runtime/LLIConfig.hpp"
#include "Static/LLVMModule.hpp"
#include "Support/Error.hpp"
#include "Support/Logger.hpp"
#include "Support/ThreadPool.hpp"
#include "Verification/Config.hpp"
#include "Verification/GenMCDriver.hpp"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/DynamicLibrary.h>

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <unistd.h>

namespace fs = std::filesystem;

enum class InputLanguage : std::uint8_t { clang, cargo, rust, llvmir };

// NOLINTBEGIN

/*******************************************************************************
 **                     Command-line arguments
 ******************************************************************************/

/*** Command-line argument categories ***/

static llvm::cl::OptionCategory clGeneral("Exploration Options");
static llvm::cl::OptionCategory clTransformation("Transformation Options");
static llvm::cl::OptionCategory clDebugging("Debugging Options");

/*** General syntax ***/

static llvm::cl::list<std::string> clCFLAGS(llvm::cl::Positional, llvm::cl::ZeroOrMore,
					    llvm::cl::desc("-- [compiler flags]"));

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

static llvm::cl::opt<bool> clDisableHelper("disable-helper", llvm::cl::cat(clGeneral),
					   llvm::cl::desc("Disable helping pattern optimization"));

static llvm::cl::opt<bool>
	clConfirmation("confirmation", llvm::cl::cat(clGeneral),
		       llvm::cl::desc("Enable confirmation pattern optimization"));

static llvm::cl::opt<bool> clDisableFinalWrite("disable-final-write", llvm::cl::cat(clGeneral),
					       llvm::cl::desc("Disable final write optimization"));

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

static llvm::cl::opt<std::string>
	clCollectLinSpec("collect-lin-spec", llvm::cl::init(""), llvm::cl::value_desc("file"),
			 llvm::cl::cat(clGeneral),
			 llvm::cl::desc("Collect linearizability specification into a file"));

static llvm::cl::opt<std::string>
	clCheckLinSpec("check-lin-spec", llvm::cl::init(""), llvm::cl::value_desc("file"),
		       llvm::cl::cat(clGeneral),
		       llvm::cl::desc("Check implementation refinement of specification file"));

static llvm::cl::opt<bool>
	clDotPrintOnlyClientEvents("dot-print-only-client-events", llvm::cl::cat(clGeneral),
				   llvm::cl::desc("Omit library events in the DOT file"));

static llvm::cl::opt<unsigned int> clMaxExtSize(
	"max-hint-size", llvm::cl::init(std::numeric_limits<unsigned int>::max()),
	llvm::cl::cat(clDebugging),
	llvm::cl::desc("Limit the number of edges in hints to be considered (for debugging)"));

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

static llvm::cl::opt<std::string> clOutputLlvmBefore(
	"output-llvm-before", llvm::cl::init(""), llvm::cl::value_desc("file"),
	llvm::cl::cat(clDebugging),
	llvm::cl::desc("Output the LLVM code to file before applying transformations"));
static llvm::cl::opt<std::string> clOutputLlvmAfter(
	"output-llvm-after", llvm::cl::init(""), llvm::cl::value_desc("file"),
	llvm::cl::cat(clDebugging),
	llvm::cl::desc("Output the LLVM code to file after applying transformations"));

static llvm::cl::opt<bool> clSkipGenmcStdBuild(
	"skip-genmc-std-build", llvm::cl::cat(clDebugging),
	llvm::cl::desc("Don't build the genmc-std crate while verifying .rs files "
		       "if a build is already present"));

static llvm::cl::list<std::string> clExtraInput(
	"extra-input", llvm::cl::value_desc("file"), llvm::cl::cat(clDebugging),
	llvm::cl::desc("Build this file seperately and link the result into the build"));

static llvm::cl::opt<unsigned int>
	clWarnOnGraphSize("warn-on-graph-size", llvm::cl::init(1024), llvm::cl::value_desc("N"),
			  llvm::cl::cat(clDebugging),
			  llvm::cl::desc("Warn about graphs larger than N"));
static llvm::cl::opt<SchedulePolicy> clSchedulePolicy(
	"schedule-policy", llvm::cl::cat(clDebugging), llvm::cl::init(SchedulePolicy::WF),
	llvm::cl::desc("Choose the scheduling policy:"),
	llvm::cl::values(clEnumValN(SchedulePolicy::LTR, "ltr", "Left-to-right"),
			 clEnumValN(SchedulePolicy::WF, "wf", "Writes-first (default)"),
			 clEnumValN(SchedulePolicy::WFR, "wfr", "Writes-first-random"),
			 clEnumValN(SchedulePolicy::Arbitrary, "arbitrary", "Arbitrary")));

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

static llvm::cl::opt<VerbosityLevel> clVLevel(
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

static llvm::cl::opt<bool> clRelincheDebug("relinche-debug", llvm::cl::cat(clDebugging),
					   llvm::cl::desc("Enable debug printing for Relinche"));
#endif /* ENABLE_GENMC_DEBUG */

// NOLINTEND

static void printVersion(llvm::raw_ostream &s)
{
	s << PACKAGE_NAME " (" PACKAGE_URL "):\n"
	  << "  " PACKAGE_NAME " v" PACKAGE_VERSION
#ifdef GIT_COMMIT
	  << " (commit #" GIT_COMMIT ")"
#endif
	  << "\n  Built with LLVM " LLVM_VERSION " (" LLVM_BUILDMODE ")\n";
}

static auto determineLang(const std::string &inputFile) -> InputLanguage
{
	std::filesystem::path inputFilePath(inputFile);

	if (inputFilePath.extension() == ".toml")
		return InputLanguage::cargo;
	if (inputFilePath.extension() == ".rs")
		return InputLanguage::rust;
	if (inputFilePath.extension() == ".ll")
		return InputLanguage::llvmir;
	return InputLanguage::clang; /* default: C/C++ */
}

/* Check whether files exist and if settings are correct for the given file type. */
static void checkLLIConfig(const LLIConfig &lliConfig)
{
	/* Make sure -skip-genmc-std-build is used only on Rust builds */
	if (lliConfig.skipGenmcStdBuild && !lliConfig.rust) {
		ERROR("-skip-genmc-std-build used on non-Rust input.");
	}
}

static void saveConfigOptions(Config &conf, LLIConfig &lliConfig)
{
	/* LLI */
	lliConfig.cflags.insert(lliConfig.cflags.end(), clCFLAGS.begin(), clCFLAGS.end());
	lliConfig.inputFile = std::move(clInputFile);
	lliConfig.threads = clThreads;
	lliConfig.disableStopOnSystemError = clDisableStopOnSystemError;
	lliConfig.collectLinSpec = clCollectLinSpec.empty()
					   ? std::nullopt
					   : std::optional(clCollectLinSpec.getValue());
	lliConfig.checkLinSpec = clCheckLinSpec.empty() ? std::nullopt
							: std::optional(clCheckLinSpec.getValue());

	lliConfig.unroll = clLoopUnroll >= 0 ? std::optional(clLoopUnroll.getValue())
					     : std::nullopt;
	lliConfig.noUnrollFuns.insert(clNoUnrollFuns.begin(), clNoUnrollFuns.end());
	lliConfig.loopJumpThreading = !clDisableLoopJumpThreading;
	lliConfig.castElimination = !clDisableCastElimination;
	lliConfig.inlineFunctions = !clDisableFunctionInliner;
	lliConfig.spinAssume = !clDisableSpinAssume;
	lliConfig.codeCondenser = !clDisableCodeCondenser;
	lliConfig.loadAnnot = !clDisableLoadAnnot;
	lliConfig.assumePropagation = !clDisableAssumePropagation;
	lliConfig.mmDetector = !clDisableMMDetector;
	lliConfig.helper = !clDisableHelper;
	lliConfig.confirmation = clConfirmation; /* could be two options: pass + annot */
	lliConfig.finalWrite = !clDisableFinalWrite;
	lliConfig.liveness = clCheckLiveness;
	lliConfig.bam = !clDisableBAM;
	auto lang = determineLang(lliConfig.inputFile);
	lliConfig.rust = lang == InputLanguage::rust || lang == InputLanguage::cargo;

	lliConfig.outputLlvmBefore = std::move(clOutputLlvmBefore);
	lliConfig.outputLlvmAfter = std::move(clOutputLlvmAfter);
	lliConfig.skipGenmcStdBuild = clSkipGenmcStdBuild;
	for (const auto extraInput : clExtraInput) {
		auto extraLang = determineLang(extraInput);
		lliConfig.extraInput.insert(extraInput);
		lliConfig.rust |= extraLang == InputLanguage::rust ||
				  extraLang == InputLanguage::cargo;
	}
	lliConfig.programEntryFun = std::move(clProgramEntryFunction);
	lliConfig.isDepTrackingModel = (clModelType == ModelType::IMM);

	/* Exploration */
	conf.dotFile = std::move(clDotGraphFile);
	conf.model = clModelType;
	conf.estimate = !clDisableEstimation;
	conf.estimationMax = clEstimationMax;
	conf.estimationMin = clEstimationMin;
	conf.sdThreshold = clEstimationSdThreshold;
	conf.isDepTrackingModel = lliConfig.isDepTrackingModel;

	conf.bound = clBound >= 0 ? std::optional(clBound.getValue()) : std::nullopt;
	conf.boundType = clBoundType.getNumOccurrences() > 0 ? clBoundType : BoundType::none;
	conf.LAPOR = clLAPOR;
	conf.symmetryReduction = !clDisableSymmetryReduction;
	conf.helper = !clDisableHelper;
	conf.confirmation = clConfirmation;
	conf.finalWrite = !clDisableFinalWrite;
	conf.printErrorTrace = clPrintErrorTrace;
	conf.checkLiveness = clCheckLiveness;
	conf.instructionCaching = !clDisableInstructionCaching;
	conf.disableRaceDetection = clDisableRaceDetection;
	conf.disableBAM = clDisableBAM;
	conf.ipr = !clDisableIPR;

	conf.warnUnfreedMemory = !clDisableWarnUnfreedMemory;
	conf.collectLinSpec = lliConfig.collectLinSpec;
	conf.checkLinSpec = lliConfig.checkLinSpec;
	conf.dotPrintOnlyClientEvents = clDotPrintOnlyClientEvents;
	conf.maxExtSize = clMaxExtSize;

	/* Save debugging options */
	conf.warnOnGraphSize = clWarnOnGraphSize;
	conf.schedulePolicy = clSchedulePolicy;
	conf.printRandomScheduleSeed = clPrintArbitraryScheduleSeed;
	conf.randomScheduleSeed = std::move(clArbitraryScheduleSeed);
	conf.printExecGraphs = clPrintExecGraphs;
	conf.printBlockedExecs = clPrintBlockedExecs;
#ifdef ENABLE_GENMC_DEBUG
	conf.validateExecGraphs = clValidateExecGraphs;
	conf.countDuplicateExecs = clCountDuplicateExecs;
	conf.countMootExecs = clCountMootExecs;
	conf.printEstimationStats = clPrintEstimationStats;
	conf.boundsHistogram = clBoundsHistogram;
	conf.relincheDebug = clRelincheDebug;
#endif
}

static void adjustConfig(const ModuleInfo &modInfo, Config &conf)
{
	/* Warn if BAM is enabled and barrier results might be used */
	if (!conf.disableBAM && modInfo.barrierResultsUsed.has_value() &&
	    *modInfo.barrierResultsUsed == BarrierRetResult::Used) {
		LOG(VerbosityLevel::Warning,
		    "Could not determine whether barrier_wait() result is used. Use -disable-bam "
		    "if the order in which threads reach the barrier matters.");
	}

	/* Perhaps override the MM under which verification will take place */
	if (modInfo.determinedMM.has_value() && isStrongerThan(*modInfo.determinedMM, conf.model)) {
		conf.model = *modInfo.determinedMM;
		conf.isDepTrackingModel = (conf.model == ModelType::IMM);
		LOG(VerbosityLevel::Tip,
		    "Automatically adjusting memory model to {}. You can disable this behavior "
		    "with -disable-mm-detector.",
		    conf.model);
	}
}

static void parseConfig(int argc, char **argv, Config &conf, LLIConfig &lliConfig)
{
	/* Option categories printed */
	const llvm::cl::OptionCategory *cats[] = {&clGeneral, &clDebugging, &clTransformation};

	llvm::cl::SetVersionPrinter(printVersion);

	/* Hide unrelated LLVM options and parse user configuration */
	llvm::cl::HideUnrelatedOptions(cats);
	llvm::cl::ParseCommandLineOptions(argc, argv,
					  "GenMC -- "
					  "Model Checking for C programs");

	/* Save the config options and do some sanity-checks. */
	saveConfigOptions(conf, lliConfig);
	checkLLIConfig(lliConfig);

	std::vector<std::string> warnings;
	auto status = conf.validate(warnings);
	for (const auto &w : warnings)
		WARN("{}", w);
	if (auto *errors = std::get_if<ConfigErrorList>(&status); errors) {
		for (const auto &e : *errors)
			LOG(VerbosityLevel::Error, "{}", e);
		exit(EUSER);
	}

	/* Set (global) log state */
	logLevel = clVLevel;

	llvm::cl::ResetAllOptionOccurrences();
	clInputFile.removeArgument();
}

/*******************************************************************************
 **                         Compilation
 ******************************************************************************/

static auto getOutFilename(const LLIConfig & /* lliConfig */) -> std::string
{
	static char filenameTemplate[] = "/tmp/__genmc.ll.XXXXXX";
	static bool createdFilename = false;

	if (!createdFilename) {
		close(mkstemp(filenameTemplate));
		createdFilename = true;
	}
	return {filenameTemplate};
}

static auto buildCompilationArgs(const LLIConfig &lliConfig) -> std::string
{
	std::string args;

	args += " -fno-discard-value-names";
	args += " -Xclang";
	args += " -disable-O0-optnone";
	/* We may be linking with Rust which uses dwarf-4 */
	args += lliConfig.rust ? " -gdwarf-4" : " -g";
	for (const auto &f : lliConfig.cflags)
		args += " " + f;
	args += " -I" SRC_INCLUDE_DIR;
	args += " -I" INCLUDE_DIR;
	args += " -S -emit-llvm";
	args += " -o " + getOutFilename(lliConfig);
	args += " " + lliConfig.inputFile;

	return args;
}

static auto compileInput(const LLIConfig &lliConfig, const std::unique_ptr<llvm::LLVMContext> &ctx,
			 std::unique_ptr<llvm::Module> &module) -> bool
{
	const auto *path = CLANGPATH;
	auto command = path + buildCompilationArgs(lliConfig);
	if (std::system(command.c_str()) != 0)
		return false;

	module = LLVMModule::parseLLVMModule(getOutFilename(lliConfig), ctx);
	return true;
}

/**
 * Clean + build a given Cargo crate
 */
static auto buildCargoProject(std::string &cargoPath, const std::filesystem::path &projectPath,
			      bool skipBuild = false) -> bool
{
	if (skipBuild)
		return true;

	/* Save current working directory */
	const std::filesystem::path origWD = std::filesystem::current_path();

	/* Clean + build the cargo project */
	std::filesystem::current_path(projectPath);
	std::string cleanCommand = cargoPath + std::string(" clean");
	if (std::system(cleanCommand.c_str()) != 0)
		WARN("'cargo clean' failed");

	std::string buildCommand =
		std::string("RUSTFLAGS=\"--emit=llvm-bc\" ") + cargoPath + std::string(" build");
	if (std::system(buildCommand.c_str()) != 0) {
		WARN("'cargo build' failed");
		return false;
	}

	/* Restore original working directory to allow relative pathnames */
	std::filesystem::current_path(origWD);
	return true;
}

/**
 * Compilation for *.rs input types.
 * Strategy: Build genmc-std as library cargo project separately, then include the produced
 * .rlib file as a prelude (--extern) when building our .rs file with rustc. Link the
 * llvm-ir output files produced by genmc-std into it seperately.
 */
static auto compileRustInput(const LLIConfig &lliConfig,
			     const std::unique_ptr<llvm::LLVMContext> &ctx,
			     std::unique_ptr<llvm::Module> &module) -> bool
{
#ifndef RUSTC_PATH
	ERROR("Rust not linked, use cmake with '-DENABLE_RUST=ON' and rebuild GenMC to use "
	      "Rust.");
#define RUSTC_PATH ""
#define CARGO_PATH ""
#endif
	auto rustcPath = std::string(RUSTC_PATH);
	auto cargoPath = std::string(CARGO_PATH);

	std::filesystem::path pathGenmcStd(std::string(RUST_DIR) + "/genmc-std");
	std::filesystem::path pathDebug = pathGenmcStd / "target" / "debug";
	std::filesystem::path pathRlib = pathDebug / "libgenmc_std.rlib";

	/* Build genmc-std, skip if a build exists that we're allowed to use */
	bool skipBuild = lliConfig.skipGenmcStdBuild && std::filesystem::exists(pathRlib);
	bool buildSuccessful =
		buildCargoProject(cargoPath, RUST_DIR + std::string("/genmc-std"), skipBuild);
	if (!buildSuccessful)
		return false;

	/* Build args for rustc */
	std::string args;
	args += " --emit=llvm-bc";
	args += " -Copt-level=0";
	for (const auto &f : lliConfig.cflags)
		args += " " + f;
	args += " --extern std=" + pathRlib.string();
	args += " -o rustc_out.bc";
	args += " " + lliConfig.inputFile;

	/* Build the rustc file */
	std::string rustcCommand = rustcPath + args;
	if (std::system(rustcCommand.c_str()) != 0) {
		WARN("'rustc' failed");
		return false;
	}

	/* Link genmc-std's llvm-ir output into it (otherwise unused functions like
	 * genmc__rust_alloc are not present in the module for use in later LLVM-Passes) */
	std::string mvCommand = std::string("mv -f rustc_out.bc ") + (pathDebug / "deps").string();
	if (std::system(mvCommand.c_str()) != 0) {
		WARN("Copying the LLVM-IR output failed");
		return false;
	}
	module = LLVMModule::parseLinkAllLLVMModules(pathGenmcStd, ctx);

	return true;
}

/**
 * Compilation for Cargo-crate input types.
 */
static auto compileCargoInput(const LLIConfig &lliConfig,
			      const std::unique_ptr<llvm::LLVMContext> &ctx,
			      std::unique_ptr<llvm::Module> &module) -> bool
{
#ifndef RUSTC_PATH
	ERROR("Rust not linked, use cmake with '-DENABLE_RUST=ON' and rebuild GenMC to use Rust.");
#define CARGO_PATH ""
#endif
	std::string cargoPath = std::string(CARGO_PATH);
	std::filesystem::path inputFilePath(lliConfig.inputFile);

	/* Save current working directory */
	const std::filesystem::path origWD = std::filesystem::current_path();

	/* Clean + build the cargo project */
	bool buildSuccessful = buildCargoProject(cargoPath, inputFilePath.parent_path());
	if (!buildSuccessful)
		return false;

	/* Restore original working directory to allow relative pathnames */
	std::filesystem::current_path(origWD);

	/* Parse & Link all the LLVM-BC output files */
	module = LLVMModule::parseLinkAllLLVMModules(inputFilePath.parent_path(), ctx);
	return true;
}

static auto compileToModule(const LLIConfig &lliConfig,
			    const std::unique_ptr<llvm::LLVMContext> &ctx)
	-> std::unique_ptr<llvm::Module>
{
	/* Make sure filename is a regular file */
	if (!fs::is_regular_file(lliConfig.inputFile))
		ERROR("Input file is neither a regular file, nor a directory!");

	std::unique_ptr<llvm::Module> module;
	auto lang = determineLang(lliConfig.inputFile);
	if (lang == InputLanguage::llvmir) {
		module = LLVMModule::parseLLVMModule(lliConfig.inputFile, ctx);
	} else if (lang == InputLanguage::clang && !compileInput(lliConfig, ctx, module)) {
		std::exit(ECOMPILE);
	} else if (lang == InputLanguage::cargo && !compileCargoInput(lliConfig, ctx, module)) {
		std::exit(ECOMPILE);
	} else if (lang == InputLanguage::rust && !compileRustInput(lliConfig, ctx, module)) {
		std::exit(ECOMPILE);
	}
	return std::move(module);
}

static void transformInput(const LLIConfig &lliConfig, llvm::Module &module, ModuleInfo &modInfo)
{
	if (!lliConfig.outputLlvmBefore.empty())
		LLVMModule::printLLVMModule(module, lliConfig.outputLlvmBefore);

	LLVMModule::transformLLVMModule(module, modInfo, &lliConfig);
	if (!lliConfig.outputLlvmAfter.empty())
		LLVMModule::printLLVMModule(module, lliConfig.outputLlvmAfter);
}

/*******************************************************************************
 **                       Running/result printing
 ******************************************************************************/

static auto durationToMill(const std::chrono::high_resolution_clock::duration &duration)
{
	static constexpr long double secToMillFactor = 1e-3L;
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() *
	       secToMillFactor;
}

static auto getElapsedSecs(const std::chrono::high_resolution_clock::time_point &begin)
	-> long double
{
	return durationToMill(std::chrono::high_resolution_clock::now() - begin);
}

static void printEstimationResults(const std::shared_ptr<const Config> &conf,
				   const std::chrono::high_resolution_clock::time_point &begin,
				   const VerificationResult &res)
{
	PRINT(VerbosityLevel::Error, "{}", res.message);
	PRINT(VerbosityLevel::Error, "{}.\n",
	      (!res.status.has_value() ? "*** Estimation complete"
				       : "\n*** Estimation unsuccessful"));

	long long mean = std::llround(res.estimationMean);
	long long sd = std::llround(std::sqrt(res.estimationVariance));
	auto meanTimeSecs = getElapsedSecs(begin) / (res.explored + res.exploredBlocked);
	PRINT(VerbosityLevel::Error, "Total executions estimate: {} (+- {})\n", mean, sd);
	PRINT(VerbosityLevel::Error, "Time to completion estimate: {:.2f}s\n", meanTimeSecs * mean);
	GENMC_DEBUG(if (conf->printEstimationStats) {
		PRINT(VerbosityLevel::Error, "Estimation moot: {}\n", res.exploredMoot);
		PRINT(VerbosityLevel::Error, "Estimation blocked: {}\n", res.exploredBlocked);
		PRINT(VerbosityLevel::Error, "Estimation complete: {}\n", res.explored);
	});
}

static void printVerificationResults(const std::shared_ptr<const Config> &conf,
				     const VerificationResult &res)
{
	PRINT(VerbosityLevel::Error, "{}", res.message);
	PRINT(VerbosityLevel::Error, "{}.\n",
	      (!res.status.has_value() ? "*** Verification complete.\nNo errors were detected"
				       : "\n*** Verification unsuccesful"));

	PRINT(VerbosityLevel::Error, "Number of complete executions explored: {}", res.explored);
	GENMC_DEBUG(if (conf->countDuplicateExecs)
			    PRINT(VerbosityLevel::Error, " ({} duplicates)", res.duplicates););
	if (res.boundExceeding) {
		BUG_ON(conf->boundType == BoundType::round);
		PRINT(VerbosityLevel::Error, " ({} exceeded bound)", res.boundExceeding);
	}
	if (res.exploredBlocked != 0U) {
		PRINT(VerbosityLevel::Error, "\nNumber of blocked executions seen: {}",
		      res.exploredBlocked);
	}
	GENMC_DEBUG(
		if (conf->countMootExecs) {
			PRINT(VerbosityLevel::Error, " (+ {} mooted)", res.exploredMoot);
		};
		if (conf->boundsHistogram) {
			PRINT(VerbosityLevel::Error, "\nBounds histogram:");
			auto executions = 0u;
			for (auto i = 0u; i < res.exploredBounds.size(); i++) {
				executions += res.exploredBounds[i];
				PRINT(VerbosityLevel::Error, " {}", executions);
			}
			if (!executions)
				PRINT(VerbosityLevel::Error, " 0");
		});
	if (conf->checkLinSpec) {
		PRINT(VerbosityLevel::Error, "\nNumber of checked hints: {}",
		      res.relincheResult.hintsChecked);
		GENMC_DEBUG(PRINT(VerbosityLevel::Error, "\nRelinche time: {:.2f}s",
				  durationToMill(res.relincheResult.analysisTime)););
	}
}

static void calculateHintsAndSaveSpec(const std::shared_ptr<const Config> &conf,
				      const VerificationResult &res)
{
	auto spec = std::move(*res.specification);
	spec.calculateHints();

	PRINT(VerbosityLevel::Error, "\n*** Specification analysis complete.\n");
	PRINT(VerbosityLevel::Error, "Number of collected outcomes: {} (synchronizations: {})\n",
	      spec.getNumOutcomes(), spec.getNumObservations());
	PRINT(VerbosityLevel::Error, "Number of hints found: {}", spec.getNumHints());

	const auto &filePath = *conf->collectLinSpec;
	std::ofstream specFile(filePath, std::ios::trunc);
	if (!specFile) {
		std::error_code ec = std::make_error_code(std::io_errc::stream);
		handleFSError(ec, "Failed to open specification file: " + filePath);
	}
	serialize(specFile, spec);
}

static auto createExecutionContext(const ExecutionGraph &g) -> std::vector<ThreadInfo>
{
	std::vector<ThreadInfo> tis;
	for (auto i = 1U; i < g.getNumThreads(); i++) { // skip main
		const auto *bLab = g.getFirstThreadLabel(i);
		BUG_ON(!bLab);
		tis.push_back(bLab->getThreadInfo());
	}
	return tis;
}

void run(GenMCDriver *driver, llvm::Interpreter *EE)
{
	driver->setInterpCallbacks(EE->getCallbacks());
	do {
		auto errorReplay = driver->handleExecutionStart();
		if (!errorReplay && driver->runFromCache()) {
			driver->handleExecutionEnd();
			continue;
		}
		EE->reset();
		EE->setExecutionContext(createExecutionContext(driver->getExec().getGraph()));
		EE->runMain(errorReplay);
		driver->handleExecutionEnd();
	} while (!driver->done());
}

auto estimate(const LLIConfig &lliConfig, std::shared_ptr<const Config> conf,
	      const std::unique_ptr<llvm::Module> &mod, const std::unique_ptr<ModuleInfo> &modInfo)
	-> VerificationResult
{
	auto estCtx = std::make_unique<llvm::LLVMContext>();
	auto newmod = LLVMModule::cloneModule(mod, estCtx);
	auto newMI = modInfo->clone(*newmod);
	auto driver = GenMCDriver::create(conf, nullptr,
					  GenMCDriver::EstimationMode{conf->estimationMax});
	std::string buf;
	auto EE = llvm::Interpreter::create(std::move(newmod), std::move(newMI), &*driver,
					    &lliConfig, driver->getExec().getAllocator(), &buf);
	run(&*driver, &*EE);
	return std::move(driver->getResult());
}

auto verify(const LLIConfig &lliConfig, std::shared_ptr<const Config> conf,
	    std::unique_ptr<llvm::Module> mod, std::unique_ptr<ModuleInfo> modInfo)
	-> VerificationResult
{
	/* Spawn a single or multiple drivers depending on the configuration */
	if (lliConfig.threads == 1) {
		auto driver = GenMCDriver::create(conf, nullptr, GenMCDriver::VerificationMode{});
		std::string buf;
		auto EE = llvm::Interpreter::create(std::move(mod), std::move(modInfo), &*driver,
						    &lliConfig, driver->getExec().getAllocator(),
						    &buf);
		run(&*driver, &*EE);
		return std::move(driver->getResult());
	}

	std::vector<std::future<VerificationResult>> futures;
	{
		/* Then, fire up the drivers */
		ThreadPool pool(lliConfig, conf, mod, modInfo, run);
		futures = pool.waitForTasks();
	}

	VerificationResult res;
	for (auto &f : futures) {
		res += f.get();
	}
	return res;
}

auto main(int argc, char **argv) -> int
{
	std::ios::sync_with_stdio(false);

	auto begin = std::chrono::high_resolution_clock::now();
	auto conf = std::make_shared<Config>();
	LLIConfig lliConfig;

	parseConfig(argc, argv, *conf, lliConfig);

	PRINT(VerbosityLevel::Error,
	      PACKAGE_NAME " v" PACKAGE_VERSION " (LLVM " LLVM_VERSION ")\n"
			   "Copyright (C) 2024 MPI-SWS. All rights reserved.\n\n");

	/* Make sure we can resolve symbols in the program. We use 0
	 * as an argument in order to load the program, not a library. This
	 * is useful as it allows the executions of external functions in the
	 * user code. */
	std::string errorStr;
	if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr, &errorStr)) {
		WARN("Could not resolve symbols in the program: {}", errorStr);
	}

	auto ctx = std::make_unique<llvm::LLVMContext>(); // *dtor after module's*
	auto moduleUP = compileToModule(lliConfig, ctx);

	/* Handle option "-extra-input"*/
	std::vector<std::unique_ptr<llvm::Module>> modules;
	modules.push_back(std::move(moduleUP));
	for (const auto extraInput : lliConfig.extraInput) {
		LLIConfig confNew;

		confNew.inputFile = extraInput;
		confNew.cflags = {};
		std::unique_ptr<llvm::Module> linkModule = compileToModule(confNew, ctx);

		modules.push_back(std::move(linkModule));
	}
	moduleUP = LLVMModule::linkAllModules(std::move(modules));

	PRINT(VerbosityLevel::Error, "*** Compilation complete.\n");

	/* Perform the necessary transformations */
	auto modInfo = std::make_unique<ModuleInfo>(*moduleUP);
	transformInput(lliConfig, *moduleUP, *modInfo);
	adjustConfig(*modInfo, *conf);
	PRINT(VerbosityLevel::Error, "*** Transformation complete.\n");

	/* Estimate the state space */
	if (conf->estimate) {
		LOG(VerbosityLevel::Tip, "Estimating state-space size. For better performance, you "
					 "can use --disable-estimation.");
		auto res = estimate(lliConfig, conf, moduleUP, modInfo);
		printEstimationResults(conf, begin, res);
		if (res.status.has_value())
			return EVERIFY;
	}

	/* Go ahead and try to verify */
	auto res = verify(lliConfig, conf, std::move(moduleUP), std::move(modInfo));
	printVerificationResults(conf, res);

	/* Serialize spec if in analysis mode */
	if (conf->collectLinSpec)
		calculateHintsAndSaveSpec(conf, res);

	PRINT(VerbosityLevel::Error, "\nTotal wall-clock time: {:.2f}s\n", getElapsedSecs(begin));

	/* TODO: Check globalContext.destroy() and llvm::shutdown() */
	return !res.status.has_value() ? 0 : EVERIFY;
}
