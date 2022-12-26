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
#include "SExprVisitor.hpp"
#include <llvm/InitializePasses.h>
#if defined(HAVE_LLVM_BITCODE_READERWRITER_H)
# include <llvm/Bitcode/ReaderWriter.h>
#else
# include <llvm/Bitcode/BitcodeReader.h>
# include <llvm/Bitcode/BitcodeWriter.h>
#endif
#if defined(HAVE_LLVM_PASSMANAGER_H)
# include <llvm/PassManager.h>
#elif defined(HAVE_LLVM_IR_PASSMANAGER_H)
# include <llvm/IR/PassManager.h>
#endif
#if defined(HAVE_LLVM_IR_LEGACYPASSMANAGER_H)
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
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#if defined(HAVE_LLVM_TRANSFORMS_UTILS_H)
#include <llvm/Transforms/Utils.h>
#endif

#define PassManager llvm::legacy::PassManager

namespace LLVMModule {

	std::unique_ptr<llvm::Module>
	parseLLVMModule(const std::string &filename, const std::unique_ptr<llvm::LLVMContext> &ctx)
	{
		llvm::SMDiagnostic err;

		auto mod = llvm::parseIRFile(filename, err, *ctx);
		if (!mod) {
			err.print(filename.c_str(), llvm::dbgs());
			ERROR("Could not parse LLVM IR!\n");
		}
		return std::move(mod);
	}

	std::unique_ptr<llvm::Module>
	cloneModule(const std::unique_ptr<llvm::Module> &mod,
		    const std::unique_ptr<llvm::LLVMContext> &ctx)
	{
		/* Roundtrip the module to a stream and then back into the new context */
		std::string str;
		llvm::raw_string_ostream  stream(str);

		llvm::WriteBitcodeToFile(*mod, stream);

		llvm::StringRef ref(stream.str());
		std::unique_ptr<llvm::MemoryBuffer> buf(llvm::MemoryBuffer::getMemBuffer(ref));

		return std::move(llvm::parseBitcodeFile(buf->getMemBufferRef(), *ctx).get());
	}

	void initializeVariableInfo(ModuleInfo &MI, PassModuleInfo &PI)
	{
		for (auto &kv : PI.varInfo.globalInfo)
			MI.varInfo.globalInfo[MI.idInfo.VID.at(kv.first)] = kv.second;
		for (auto &kv : PI.varInfo.localInfo) {
			if (MI.idInfo.VID.count(kv.first))
				MI.varInfo.localInfo[MI.idInfo.VID.at(kv.first)] = kv.second;
		}
		MI.varInfo.internalInfo = PI.varInfo.internalInfo;
		return;
	}

	void initializeAnnotationInfo(ModuleInfo &MI, PassModuleInfo &PI)
	{
		using Transformer = SExprTransformer<llvm::Value *>;
		Transformer tr;

		for (auto &kv : PI.annotInfo.annotMap) {
			MI.annotInfo.annotMap[MI.idInfo.VID.at(kv.first)] =
				tr.transform(&*kv.second, [&](llvm::Value *v){ return MI.idInfo.VID.at(v); });
		}
	}

	void initializeFsInfo(ModuleInfo &MI, PassModuleInfo &PI)
	{
		MI.fsInfo.filenames.insert(PI.filenames.begin(), PI.filenames.end());
		return;
	}

	void initializeModuleInfo(ModuleInfo &MI, PassModuleInfo &PI)
	{
		MI.collectIDs();
		initializeVariableInfo(MI, PI);
		initializeAnnotationInfo(MI, PI);
		initializeFsInfo(MI, PI);
		return;
	}

	bool transformLLVMModule(llvm::Module &mod, ModuleInfo &MI,
				 const std::shared_ptr<const Config> &conf)
	{
		llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
		PassModuleInfo PI;
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
		OptPM.add(createMDataCollectionPass(&PI));
		if (conf->inlineFunctions)
			OptPM.add(createFunctionInlinerPass());
		OptPM.add(createPromoteMemIntrinsicPass());
		OptPM.add(createIntrinsicLoweringPass(mod));
		if (conf->castElimination)
			OptPM.add(createEliminateCastsPass());
		OptPM.add(llvm::createPromoteMemoryToRegisterPass());
		OptPM.add(llvm::createDeadArgEliminationPass());
		OptPM.add(createLocalSimplifyCFGPass());
		OptPM.add(createEliminateAnnotationsPass());
		OptPM.add(createEliminateRedundantInstPass());

		modified = OptPM.run(mod);

		BndPM.add(createEliminateCASPHIsPass());
		BndPM.add(llvm::createJumpThreadingPass());
		BndPM.add(createEliminateUnusedCodePass());
		BndPM.add(createBisimilarityCheckerPass());
		if (conf->codeCondenser && !conf->checkLiveness)
			BndPM.add(createCodeCondenserPass());
		if (conf->loopJumpThreading)
			BndPM.add(createLoopJumpThreadingPass());
		BndPM.add(createCallInfoCollectionPass());
		BndPM.add(createEscapeCheckerPass());
		if (conf->spinAssume)
			BndPM.add(createSpinAssumePass(conf->checkLiveness));
		if (conf->unroll >= 0)
			BndPM.add(createLoopUnrollPass(conf->unroll, conf->noUnrollFuns));

		modified |= BndPM.run(mod);

		/* Run annotation passes last so that the module is stable */
		if (conf->assumePropagation)
			OptPM.add(createPropagateAssumesPass());
		if (conf->confirmAnnot)
			OptPM.add(createConfirmationAnnotationPass());
		if (conf->loadAnnot)
			OptPM.add(createLoadAnnotationPass(PI.annotInfo));
		modified |= OptPM.run(mod);

		initializeModuleInfo(MI, PI);

		assert(!llvm::verifyModule(mod, &llvm::dbgs()));
		return modified;
	}

	void printLLVMModule(llvm::Module &mod, const std::string &out)
	{
		PassManager PM;
		std::error_code errs;

		auto flags =
#if LLVM_VERSION_MAJOR < 13
			llvm::sys::fs::F_None;
#else
			llvm::sys::fs::OF_None;
#endif

		auto os = std::make_unique<llvm::raw_fd_ostream>(out.c_str(), errs, flags);

		/* TODO: Do we need an exception? If yes, properly handle it */
		if (errs) {
			WARN("Failed to write transformed module to file "
			     + out + ": " + errs.message());
			return;
		}

		PM.add(llvm::createPrintModulePass(*os));
		PM.run(mod);
		return;
	}

}
