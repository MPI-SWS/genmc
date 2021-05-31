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

#include "LLVMModule.hpp"
#include "Error.hpp"
#include "Passes.hpp"
#include <llvm/InitializePasses.h>
#if defined(HAVE_LLVM_PASSMANAGER_H)
# include <llvm/PassManager.h>
#elif defined(HAVE_LLVM_IR_PASSMANAGER_H)
# include <llvm/IR/PassManager.h>
#endif
#if defined(HAVE_LLVM_IR_LEGACYPASSMANAGER_H) && defined(LLVM_PASSMANAGER_TEMPLATE)
# include <llvm/IR/LegacyPassManager.h>
#endif
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#if defined(HAVE_LLVM_TRANSFORMS_UTILS_H)
#include <llvm/Transforms/Utils.h>
#endif

#ifdef LLVM_PASSMANAGER_TEMPLATE
# define PassManager llvm::legacy::PassManager
#else
# define PassManager llvm::PassManager
#endif

/* TODO: Move explanation comments to *.hpp files. */
namespace LLVMModule {
/* Global variable to handle the LLVM context */
	llvm::LLVMContext *globalContext = nullptr;

/* Returns the LLVM context */
	llvm::LLVMContext &getLLVMContext(void)
	{
		if (!globalContext)
			globalContext = new llvm::LLVMContext();
		return *globalContext;
	}

/*
 * Destroys the LLVM context. This function should be called explicitly
 * when we are done managing the LLVM data.
 */
	void destroyLLVMContext(void)
	{
		delete globalContext;
	}

/* Returns the LLVM module corresponding to the source code stored in src. */
	std::unique_ptr<llvm::Module> getLLVMModule(std::string &filename, std::string &src)
	{
		llvm::MemoryBuffer *buf;
		llvm::SMDiagnostic err;

#ifdef LLVM_GETMEMBUFFER_RET_PTR
		buf = llvm::MemoryBuffer::getMemBuffer(src, "", false);
#else
		buf = llvm::MemoryBuffer::getMemBuffer(src, "", false).release();
#endif
#ifdef LLVM_PARSE_IR_MEMBUF_PTR
		auto mod = llvm::ParseIR(buf, err, getLLVMContext());
#else
		auto mod = llvm::parseIR(buf->getMemBufferRef(), err, getLLVMContext()).release();
#endif
		if (!mod) {
			err.print(filename.c_str(), llvm::dbgs());
			ERROR("Could not parse LLVM IR!\n");
		}
		return std::unique_ptr<llvm::Module>(mod);
	}

	bool transformLLVMModule(llvm::Module &mod, const Config *conf, ModuleInfo &MI)
	{
		llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
		PassManager OptPM, BndPM;
		bool modified;

		llvm::initializeCore(Registry);
		llvm::initializeScalarOpts(Registry);
		llvm::initializeObjCARCOpts(Registry);
		llvm::initializeVectorization(Registry);
		llvm::initializeIPO(Registry);
		llvm::initializeAnalysis(Registry);
#ifdef HAVE_LLVM_INITIALIZE_IPA
		llvm::initializeIPA(Registry);
#endif
		llvm::initializeTransformUtils(Registry);
		llvm::initializeInstCombine(Registry);
		llvm::initializeInstrumentation(Registry);
		llvm::initializeTarget(Registry);

		OptPM.add(createDeclareInternalsPass());
		OptPM.add(createDefineLibcFunsPass());
		OptPM.add(createMDataCollectionPass(MI.varInfo, MI.fsInfo));
		OptPM.add(createPromoteMemIntrinsicPass());
		OptPM.add(createIntrinsicLoweringPass(mod));
		OptPM.add(llvm::createPromoteMemoryToRegisterPass());
		OptPM.add(llvm::createDeadArgEliminationPass());

		modified = OptPM.run(mod);

		BndPM.add(createBisimilarityCheckerPass());
		if (conf->codeCondenser && !conf->checkLiveness)
			BndPM.add(createCodeCondenserPass());
		if (conf->loopJumpThreading)
			BndPM.add(createLoopJumpThreadingPass());
		BndPM.add(createCallInfoCollectionPass());
		if (conf->spinAssume)
			BndPM.add(createSpinAssumePass(conf->checkLiveness));
		if (conf->unroll >= 0)
			BndPM.add(createLoopUnrollPass(conf->unroll));

		modified |= BndPM.run(mod);

		/* The last pass we run is the load-annotation pass */
		if (conf->loadAnnot)
			OptPM.add(createLoadAnnotationPass(MI.annotInfo));
		modified |= OptPM.run(mod);

		assert(!llvm::verifyModule(mod, &llvm::dbgs()));
		return modified;
	}

	void printLLVMModule(llvm::Module &mod, const std::string &out)
	{
		PassManager PM;
#ifdef LLVM_RAW_FD_OSTREAM_ERR_STR
		std::string errs;
#else
		std::error_code errs;
#endif
#ifdef HAVE_LLVM_SYS_FS_OPENFLAGS
		llvm::raw_ostream *os = new llvm::raw_fd_ostream(out.c_str(), errs,
								 llvm::sys::fs::F_None);
#else
		llvm::raw_ostream *os = new llvm::raw_fd_ostream(out.c_str(), errs, 0);
#endif

		/* TODO: Do we need an exception? If yes, properly handle it */
#ifdef LLVM_RAW_FD_OSTREAM_ERR_STR
		if (errs.size()) {
			delete os;
			WARN("Failed to write transformed module to file "
			     + out + ": " + errs);
			return;
		}
#else
		if (errs) {
			delete os;
			WARN("Failed to write transformed module to file "
			     + out + ": " + errs.message());
			return;
		}
#endif
		PM.add(llvm::createPrintModulePass(*os));
		PM.run(mod);
		return;
	}

}
