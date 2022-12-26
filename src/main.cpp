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
#include "DriverFactory.hpp"
#include "Error.hpp"
#include "LLVMModule.hpp"

#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>

std::string getOutFilename(const std::shared_ptr<const Config> &conf)
{
	return "/tmp/__genmc.ll";
}

std::string
buildCompilationArgs(const std::shared_ptr<const Config> &conf)
{
	std::string args;

	args += " -fno-discard-value-names";
#ifdef HAVE_CLANG_DISABLE_OPTNONE
	args += " -Xclang";
	args += " -disable-O0-optnone";
#endif
	args += " -g"; /* Compile with -g to get debugging mdata */
	for (auto &f : conf->cflags)
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

bool compileInput(const std::shared_ptr<const Config> &conf,
		  const std::unique_ptr<llvm::LLVMContext> &ctx,
		  std::unique_ptr<llvm::Module> &module)
{
	auto path = CLANGPATH;
	auto command = path + buildCompilationArgs(conf);
	if (std::system(command.c_str()))
		return false;

	module = LLVMModule::parseLLVMModule(getOutFilename(conf), ctx);
	return true;
}

void printResults(const std::shared_ptr<const Config> &conf,
		  const std::chrono::high_resolution_clock::time_point &begin,
		  const GenMCDriver::Result &res)
{
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

	if (res.status == GenMCDriver::Status::VS_OK)
		llvm::outs() << "No errors were detected.\n";
	else
		llvm::outs() << res.message;

	llvm::outs() << "Number of complete executions explored: " << res.explored;
	GENMC_DEBUG(
		llvm::outs() << ((conf->countDuplicateExecs) ?
				 " (" + std::to_string(res.duplicates) + " duplicates)" : "");
		);
	if (res.exploredBlocked) {
		llvm::outs() << "\nNumber of blocked executions seen: " << res.exploredBlocked;
	}
	if (res.exploredMoot) {
		llvm::outs() << " (" << res.exploredMoot << " mooted)";
	}
	llvm::outs() << "\nTotal wall-clock time: "
		     << llvm::format("%.2f", elapsed.count() * 1e-3)
		     << "s\n";
}

int main(int argc, char **argv)
{
	auto begin = std::chrono::high_resolution_clock::now();
	auto ctx = std::make_unique<llvm::LLVMContext>();
	auto conf = std::make_shared<Config>();

	conf->getConfigOptions(argc, argv);
	if (conf->inputFromBitcodeFile) {
		auto mod = LLVMModule::parseLLVMModule(conf->inputFile, ctx);
		auto res = GenMCDriver::verify(conf, std::move(mod));
		printResults(conf, begin, res);
		return res.status == GenMCDriver::Status::VS_OK ? 0 : EVERIFY;
	}

	std::unique_ptr<llvm::Module> module;
	if (!compileInput(conf, ctx, module))
		return ECOMPILE;

	auto res = GenMCDriver::verify(conf, std::move(module));
	printResults(conf, begin, res);

	/* TODO: Check globalContext.destroy() and llvm::shutdown() */
	return res.status == GenMCDriver::Status::VS_OK ? 0 : EVERIFY;
}
