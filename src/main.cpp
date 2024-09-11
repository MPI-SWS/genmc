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

#include "Config/Config.hpp"
#include "Static/LLVMModule.hpp"
#include "Support/Error.hpp"
#include "Verification/DriverFactory.hpp"
#include "config.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <set>
#include <unistd.h>

auto getOutFilename(const std::shared_ptr<const Config> & /*conf*/) -> std::string
{
	static char filenameTemplate[] = "/tmp/__genmc.ll.XXXXXX";
	static bool createdFilename = false;

	if (!createdFilename) {
		close(mkstemp(filenameTemplate));
		createdFilename = true;
	}
	return {filenameTemplate};
}

auto buildCompilationArgs(const std::shared_ptr<const Config> &conf) -> std::string
{
	std::string args;

	args += " -fno-discard-value-names";
	args += " -Xclang";
	args += " -disable-O0-optnone";
	args += " -g"; /* Compile with -g to get debugging mdata */
	for (const auto &f : conf->cflags)
		args += " " + f;
	args += " -I" SRC_INCLUDE_DIR;
	args += " -I" INCLUDE_DIR;
	auto inodeFlag = " -D__CONFIG_GENMC_INODE_DATA_SIZE=" + std::to_string(conf->maxFileSize);
	args += " " + inodeFlag;
	args += " -S -emit-llvm";
	args += " -o " + getOutFilename(conf);
	args += " " + conf->inputFile;

	return args;
}

auto compileInput(const std::shared_ptr<const Config> &conf,
		  const std::unique_ptr<llvm::LLVMContext> &ctx,
		  std::unique_ptr<llvm::Module> &module) -> bool
{
	const auto *path = CLANGPATH;
	auto command = path + buildCompilationArgs(conf);
	if (std::system(command.c_str()) != 0)
		return false;

	module = LLVMModule::parseLLVMModule(getOutFilename(conf), ctx);
	return true;
}

void transformInput(const std::shared_ptr<Config> &conf, llvm::Module &module, ModuleInfo &modInfo)
{
	LLVMModule::transformLLVMModule(module, modInfo, conf);
	if (!conf->transformFile.empty())
		LLVMModule::printLLVMModule(module, conf->transformFile);

	/* Perhaps override the MM under which verification will take place */
	if (conf->mmDetector && modInfo.determinedMM.has_value() &&
	    isStrongerThan(*modInfo.determinedMM, conf->model)) {
		conf->model = *modInfo.determinedMM;
		conf->isDepTrackingModel = (conf->model == ModelType::IMM);
		LOG(VerbosityLevel::Tip)
			<< "Automatically adjusting memory model to " << conf->model
			<< ". You can disable this behavior with -disable-mm-detector.\n";
	}
}

auto getElapsedSecs(const std::chrono::high_resolution_clock::time_point &begin) -> long double
{
	static constexpr long double secToMillFactor = 1e-3L;
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now - begin).count() *
	       secToMillFactor;
}

void printEstimationResults(const std::shared_ptr<const Config> &conf,
			    const std::chrono::high_resolution_clock::time_point &begin,
			    const GenMCDriver::Result &res)
{
	PRINT(VerbosityLevel::Error) << res.message;
	PRINT(VerbosityLevel::Error)
		<< (res.status == VerificationError::VE_OK ? "*** Estimation complete.\n"
							   : "*** Estimation unsuccessful.\n");

	auto mean = std::llround(res.estimationMean);
	auto sd = std::llround(std::sqrt(res.estimationVariance));
	auto meanTimeSecs = getElapsedSecs(begin) / (res.explored + res.exploredBlocked);
	PRINT(VerbosityLevel::Error)
		<< "Total executions estimate: " << mean << " (+- " << sd << ")\n"
		<< "Time to completion estimate: " << llvm::format("%.2Lf", meanTimeSecs * mean)
		<< "s\n";
	GENMC_DEBUG(if (conf->printEstimationStats) PRINT(VerbosityLevel::Error)
			    << "Estimation moot: " << res.exploredMoot << "\n"
			    << "Estimation blocked: " << res.exploredBlocked << "\n"
			    << "Estimation complete: " << res.explored << "\n";);
}

void printVerificationResults(const std::shared_ptr<const Config> &conf,
			      const std::chrono::high_resolution_clock::time_point &begin,
			      const GenMCDriver::Result &res)
{
	PRINT(VerbosityLevel::Error) << res.message;
	PRINT(VerbosityLevel::Error)
		<< (res.status == VerificationError::VE_OK
			    ? "*** Verification complete.\nNo errors were detected.\n"
			    : "*** Verification unsuccessful.\n");

	PRINT(VerbosityLevel::Error) << "Number of complete executions explored: " << res.explored;
	GENMC_DEBUG(PRINT(VerbosityLevel::Error)
			    << ((conf->countDuplicateExecs)
					? " (" + std::to_string(res.duplicates) + " duplicates)"
					: ""););
	if (res.boundExceeding) {
		BUG_ON(conf->boundType == BoundType::round);
		PRINT(VerbosityLevel::Error)
			<< " (" + std::to_string(res.boundExceeding) + " exceeded bound)";
	}
	if (res.exploredBlocked != 0U) {
		PRINT(VerbosityLevel::Error)
			<< "\nNumber of blocked executions seen: " << res.exploredBlocked;
	}
	GENMC_DEBUG(
		if (conf->countMootExecs) {
			PRINT(VerbosityLevel::Error) << " (+ " << res.exploredMoot << " mooted)";
		};
		if (conf->boundsHistogram) {
			PRINT(VerbosityLevel::Error) << "\nBounds histogram:";
			auto executions = 0u;
			for (auto i = 0u; i < res.exploredBounds.size(); i++) {
				executions += res.exploredBounds[i];
				PRINT(VerbosityLevel::Error) << " " << executions;
			}
			if (!executions)
				PRINT(VerbosityLevel::Error) << " 0";
		});
	PRINT(VerbosityLevel::Error)
		<< "\nTotal wall-clock time: " << llvm::format("%.2Lf", getElapsedSecs(begin))
		<< "s\n";
}

auto main(int argc, char **argv) -> int
{
	auto begin = std::chrono::high_resolution_clock::now();
	auto conf = std::make_shared<Config>();

	conf->getConfigOptions(argc, argv);

	PRINT(VerbosityLevel::Error)
		<< PACKAGE_NAME " v" PACKAGE_VERSION << " (LLVM " LLVM_VERSION ")\n"
		<< "Copyright (C) 2024 MPI-SWS. All rights reserved.\n\n";

	auto ctx = std::make_unique<llvm::LLVMContext>(); // *dtor after module's*
	std::unique_ptr<llvm::Module> module;
	if (conf->inputFromBitcodeFile) {
		module = LLVMModule::parseLLVMModule(conf->inputFile, ctx);
	} else if (!compileInput(conf, ctx, module)) {
		return ECOMPILE;
	}
	PRINT(VerbosityLevel::Error) << "*** Compilation complete.\n";

	/* Perform the necessary transformations */
	auto modInfo = std::make_unique<ModuleInfo>(*module);
	transformInput(conf, *module, *modInfo);
	PRINT(VerbosityLevel::Error) << "*** Transformation complete.\n";

	/* Estimate the state space */
	if (conf->estimate) {
		LOG(VerbosityLevel::Tip) << "Estimating state-space size. For better performance, "
					    "you can use --disable-estimation.\n";
		auto res = GenMCDriver::estimate(conf, module, modInfo);
		printEstimationResults(conf, begin, res);
		if (res.status != VerificationError::VE_OK)
			return EVERIFY;
	}

	/* Go ahead and try to verify */
	auto res = GenMCDriver::verify(conf, std::move(module), std::move(modInfo));
	printVerificationResults(conf, begin, res);

	/* TODO: Check globalContext.destroy() and llvm::shutdown() */
	return res.status == VerificationError::VE_OK ? 0 : EVERIFY;
}
