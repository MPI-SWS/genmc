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
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <memory>

#include <cstdlib>
#include <chrono>
#include <fstream>
#include <set>

using namespace clang;
using namespace clang::driver;

std::string getExecutablePath(const char *Argv0)
{
	/*
	 * This just needs to be some symbol in the binary; C++ doesn't
	 * allow taking the address of ::main however.
	 */
	void *MainAddr = (void*) (intptr_t) getExecutablePath;
	return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
}

llvm::opt::ArgStringList filterCC1Args(const llvm::opt::ArgStringList &ccArgs)
{
	std::set<std::string> ignoredArgs = {"-discard-value-names"};
	llvm::opt::ArgStringList newCcArgs;

	for (auto &arg : ccArgs) {
		if (ignoredArgs.count(arg) == 0) {
			newCcArgs.push_back(arg);
		}
	}
	return newCcArgs;
}

int main(int argc, char **argv)
{
	auto begin = std::chrono::high_resolution_clock::now();
	auto conf = std::make_shared<Config>();

	conf->getConfigOptions(argc, argv);
	if (conf->inputFromBitcodeFile) {
		auto ctx = LLVM_MAKE_UNIQUE<llvm::LLVMContext>();
		auto mod = LLVMModule::parseLLVMModule(conf->inputFile, ctx);
		BUG(); // FIXME
		// std::unique_ptr<GenMCDriver> driver =
		// 	DriverFactory::create(std::move(conf), std::move(mod), start);
		// driver->run();
		/* TODO: Check globalContext.destroy() and llvm::shutdown() */
		return 0;
	}

	void *MainAddr = (void*) (intptr_t) getExecutablePath;
	std::string Path = CLANGPATH;
	IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
	TextDiagnosticPrinter *DiagClient =
		new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

	IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
	DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

	// Use ELF on windows for now.
	std::string TripleStr = llvm::sys::getProcessTriple();
	llvm::Triple T(TripleStr);
	if (T.isOSBinFormatCOFF())
		T.setObjectFormat(llvm::Triple::ELF);

	Driver TheDriver(Path, T.str(), Diags);
	TheDriver.setTitle("clang interpreter");
	TheDriver.setCheckInputsExist(false);

	SmallVector<const char *, 16> Args;//(argv, argv + argc);
	Args.push_back("-fsyntax-only");
#ifdef HAVE_CLANG_DISABLE_OPTNONE
	Args.push_back("-Xclang");
	Args.push_back("-disable-O0-optnone");
#endif
	Args.push_back("-g"); /* Compile with -g to get debugging mdata */
	for (auto &f : conf->cflags)
		Args.push_back(f.c_str());
	Args.push_back("-I" SRC_INCLUDE_DIR);
	Args.push_back("-I" INCLUDE_DIR);
	auto inodeFlag = "-D__CONFIG_GENMC_INODE_DATA_SIZE=" + std::to_string(conf->maxFileSize);
	Args.push_back(inodeFlag.c_str());
	Args.push_back(conf->inputFile.c_str());

	std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));
	if (!C)
		return ECOMPILE;

	const driver::JobList &Jobs = C->getJobs();
#ifdef CLANG_LIST_TYPE_JOB_PTR
	const driver::Command &Cmd = *cast<driver::Command>(*Jobs.begin());
#else
	const driver::Command &Cmd = (*Jobs.begin());
#endif
	const llvm::opt::ArgStringList &CCArgs = Cmd.getArguments();
	const llvm::opt::ArgStringList FCCArgs = filterCC1Args(CCArgs);
#ifdef CLANG_COMPILER_INVOCATION_PTR
	CompilerInvocation *CI
#else
	std::shared_ptr<CompilerInvocation> CI
#endif
		(new CompilerInvocation);

#ifdef CLANG_CREATE_FROM_ARGS_ARRAY_REF
	CompilerInvocation::CreateFromArgs(*CI, FCCArgs, Diags);
#else
	CompilerInvocation::CreateFromArgs(*CI,
					   const_cast<const char **>(FCCArgs.data()),
					   const_cast<const char **>(FCCArgs.data()) +
					   FCCArgs.size(), Diags);
#endif

	// Show the invocation, with -v.
	if (CI->getHeaderSearchOpts().Verbose) {
		llvm::errs() << "clang invocation:\n";
		Jobs.Print(llvm::errs(), "\n", true);
		llvm::errs() << "\n";
	}

	CompilerInstance Clang;
	Clang.setInvocation(CI);

	// Create the compilers actual diagnostics engine.
	Clang.createDiagnostics();
	if (!Clang.hasDiagnostics())
		return ECOMPILE;

	// Infer the builtin include path if unspecified.
	if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
	    Clang.getHeaderSearchOpts().ResourceDir.empty())
		Clang.getHeaderSearchOpts().ResourceDir =
			CompilerInvocation::GetResourcesPath(argv[0], MainAddr);

	// Create and execute the frontend to generate an LLVM bitcode module.
	std::unique_ptr<CodeGenAction> Act(new EmitLLVMOnlyAction());
	if (!Clang.ExecuteAction(*Act))
		return ECOMPILE;

	auto res = GenMCDriver::verify(conf, std::move(Act->takeModule()));

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
	llvm::outs() << "\n";
	if (res.exploredBlocked) {
		llvm::outs() << "Number of blocked executions seen: " << res.exploredBlocked
			     << "\n";
	}
	llvm::outs() << "Total wall-clock time: "
		     << llvm::format("%.2f", elapsed.count() * 1e-3)
		     << "s\n";

	/* TODO: Check globalContext.destroy() and llvm::shutdown() */

	return res.status == GenMCDriver::Status::VS_OK ? 0 : EVERIFY;
}
