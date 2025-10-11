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

#include "LLVMModule.hpp"
#include "Runtime/LLIConfig.hpp"
#include "Static/Transforms/BarrierResultCheckerPass.hpp"
#include "Static/Transforms/BisimilarityCheckerPass.hpp"
#include "Static/Transforms/CallInfoCollectionPass.hpp"
#include "Static/Transforms/CodeCondenserPass.hpp"
#include "Static/Transforms/ConfirmationAnnotationPass.hpp"
#include "Static/Transforms/DeclareInternalsPass.hpp"
#include "Static/Transforms/DefineLibcFunsPass.hpp"
#include "Static/Transforms/EliminateAnnotationsPass.hpp"
#include "Static/Transforms/EliminateCASPHIsPass.hpp"
#include "Static/Transforms/EliminateCastsPass.hpp"
#include "Static/Transforms/EliminateRedundantInstPass.hpp"
#include "Static/Transforms/EliminateUnusedCodePass.hpp"
#include "Static/Transforms/EscapeCheckerPass.hpp"
#include "Static/Transforms/FunctionInlinerPass.hpp"
#include "Static/Transforms/IntrinsicLoweringPass.hpp"
#include "Static/Transforms/LoadAnnotationPass.hpp"
#include "Static/Transforms/LocalSimplifyCFGPass.hpp"
#include "Static/Transforms/LoopJumpThreadingPass.hpp"
#include "Static/Transforms/LoopUnrollPass.hpp"
#include "Static/Transforms/MDataCollectionPass.hpp"
#include "Static/Transforms/MMDetectorPass.hpp"
#include "Static/Transforms/PromoteMemIntrinsicPass.hpp"
#include "Static/Transforms/PropagateAssumesPass.hpp"
#include "Static/Transforms/RustPrepPass.hpp"
#include "Static/Transforms/SpinAssumePass.hpp"
#include "Support/Error.hpp"
#include "Support/SExprVisitor.hpp"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/PassPlugin.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/DeadArgumentElimination.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/JumpThreading.h>
#include <llvm/Transforms/Scalar/SROA.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

#include <filesystem>
namespace fs = std::filesystem;

namespace LLVMModule {

auto parseLLVMModule(const std::string &filename, const std::unique_ptr<llvm::LLVMContext> &ctx)
	-> std::unique_ptr<llvm::Module>
{
	llvm::SMDiagnostic err;

	auto mod = llvm::parseIRFile(filename, err, *ctx);
	if (!mod) {
		err.print(filename.c_str(), llvm::dbgs());
		ERROR("Could not parse LLVM IR!\n");
	}
	return std::move(mod);
}

/**
 * Used to link a collection of modules into a single module
 */
auto linkAllModules(std::vector<std::unique_ptr<llvm::Module>> modules) -> std::unique_ptr<Module>
{
	Linker linker(*modules[0]);
	for (size_t i = 1; i < modules.size(); ++i) {
		if (linker.linkInModule(std::move(modules[i]))) {
			ERROR("Could not link the LLVM IR!\n");
		}
	}

	return std::move(modules[0]);
}

/**
 * Used for Rust-builds. We link all produced llvm-ir files in the target-directory together.
 */
auto parseLinkAllLLVMModules(const fs::path &dirname, const std::unique_ptr<llvm::LLVMContext> &ctx)
	-> std::unique_ptr<llvm::Module>
{
	llvm::SMDiagnostic err;

	/* Search for all *.bc files under "dir / target / * / deps /" */
	std::vector<fs::path> bc_files;
	for (const auto &dir : fs::directory_iterator(dirname / "target")) {
		if (!fs::is_directory(dir)) {
			continue;
		}

		auto deps_dir = dir.path() / "deps";
		if (!fs::exists(deps_dir)) {
			continue;
		}

		for (const auto &file : fs::directory_iterator(deps_dir)) {
			if (file.path().extension() != ".bc") {
				continue;
			}

			bc_files.push_back(file.path());
		}
	}

	/* Parse all modules */
	std::vector<std::unique_ptr<Module>> modules;
	for (auto bc_file : bc_files) {
		std::unique_ptr<Module> module = parseIRFile(bc_file.string(), err, *ctx);
		if (!module) {
			err.print(bc_file.c_str(), llvm::dbgs());
			ERROR("Could not parse LLVM IR!\n");
		}
		modules.push_back(std::move(module));
	}

	/* Link them all together */
	return linkAllModules(std::move(modules));
}

auto cloneModule(const std::unique_ptr<llvm::Module> &mod,
		 const std::unique_ptr<llvm::LLVMContext> &ctx) -> std::unique_ptr<llvm::Module>
{
	/* Roundtrip the module to a stream and then back into the new context */
	std::string str;
	llvm::raw_string_ostream stream(str);

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
}

void initializeAnnotationInfo(ModuleInfo &MI, PassModuleInfo &PI)
{
	using Transformer = SExprTransformer<llvm::Value *>;
	Transformer tr;

	for (auto &kv : PI.annotInfo.annotMap) {
		MI.annotInfo.annotMap[MI.idInfo.VID.at(kv.first)] = std::make_pair(
			kv.second.first, tr.transform(&*kv.second.second, [&](llvm::Value *v) {
				return MI.idInfo.VID.at(v);
			}));
	}
}

void initializeModuleInfo(ModuleInfo &MI, PassModuleInfo &PI)
{
	MI.collectIDs();
	initializeVariableInfo(MI, PI);
	initializeAnnotationInfo(MI, PI);
	MI.determinedMM = PI.determinedMM;
	MI.barrierResultsUsed = PI.barrierResultsUsed;
}

auto transformLLVMModule(llvm::Module &mod, ModuleInfo &MI, const LLIConfig *conf) -> bool
{
	PassModuleInfo PI;

	/* NOTE: The order between the analyses, the builder and the managers matters */

	/* First, register the analyses that we are about to use.
	 * We also use an (unused) pass builder to load default analyses */
	llvm::LoopAnalysisManager lam;
	llvm::CGSCCAnalysisManager cgam;
	llvm::FunctionAnalysisManager fam;
	llvm::ModuleAnalysisManager mam;

	mam.registerPass([&] { return MDataInfo(); });
	mam.registerPass([&] { return BarrierResultAnalysis(); });
	mam.registerPass([&] { return MMAnalysis(); });
	mam.registerPass([&] { return CallAnalysis(); });
	mam.registerPass([&] { return EscapeAnalysis(); });
	fam.registerPass([&] { return BisimilarityAnalysis(); });
	fam.registerPass([&] { return LoadAnnotationAnalysis(); });

	llvm::PassBuilder pb;
	pb.registerModuleAnalyses(mam);
	pb.registerCGSCCAnalyses(cgam);
	pb.registerFunctionAnalyses(fam);
	pb.registerLoopAnalyses(lam);
	pb.crossRegisterProxies(lam, fam, cgam, mam);

	/* Then create two pass managers: a basic one and one that
	runs some loop passes */
	llvm::ModulePassManager basicOptsMGR;

	basicOptsMGR.addPass(DeclareInternalsPass());
	basicOptsMGR.addPass(DefineLibcFunsPass());
	basicOptsMGR.addPass(MDataCollectionPass(PI));
	if (conf->rust)
		basicOptsMGR.addPass(RustPrepPass());
	if (conf->inlineFunctions)
		basicOptsMGR.addPass(FunctionInlinerPass());
	{
		llvm::FunctionPassManager fpm;
		/* Run after the inliner because it might generate new memcpys */
		fpm.addPass(PromoteMemIntrinsicPass());
		fpm.addPass(IntrinsicLoweringPass());
		if (conf->castElimination)
			fpm.addPass(EliminateCastsPass());
#if LLVM_VERSION_MAJOR < 14
		fpm.addPass(SROA());
#elif LLVM_VERSION_MAJOR < 16
		fpm.addPass(SROAPass());
#else
		fpm.addPass(SROAPass(SROAOptions::PreserveCFG));
#endif
		fpm.addPass(PromotePass()); // Mem2Reg
		basicOptsMGR.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
	}
	basicOptsMGR.addPass(DeadArgumentEliminationPass());
	{
		llvm::FunctionPassManager fpm;
		fpm.addPass(LocalSimplifyCFGPass());
		fpm.addPass(EliminateAnnotationsPass({.annotHelper = conf->helper,
						      .annotConf = conf->confirmation,
						      .annotFinal = conf->finalWrite}));
		fpm.addPass(EliminateRedundantInstPass());
		basicOptsMGR.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
	}

	if (!conf->bam)
		basicOptsMGR.addPass(BarrierResultCheckerPass(PI));
	if (conf->mmDetector)
		basicOptsMGR.addPass(MMDetectorPass(PI));

	auto preserved = basicOptsMGR.run(mod, mam);

	llvm::ModulePassManager loopOptsMGR;

	{
		llvm::FunctionPassManager fpm;
		fpm.addPass(EliminateCASPHIsPass());
		fpm.addPass(llvm::JumpThreadingPass());
		fpm.addPass(EliminateUnusedCodePass());
		if (conf->codeCondenser && !conf->liveness)
			fpm.addPass(CodeCondenserPass());
		if (conf->loopJumpThreading)
			fpm.addPass(createFunctionToLoopPassAdaptor(LoopJumpThreadingPass()));
		loopOptsMGR.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
	}
	if (conf->spinAssume)
		loopOptsMGR.addPass(SpinAssumePass(conf->liveness));
	if (conf->unroll.has_value())
		loopOptsMGR.addPass(
			createModuleToFunctionPassAdaptor(createFunctionToLoopPassAdaptor(
				LoopUnrollPass(*conf->unroll, conf->noUnrollFuns))));
	preserved.intersect(loopOptsMGR.run(mod, mam));

	/* Run annotation passes last so that the module is stable */
	{
		llvm::FunctionPassManager fpm;
		if (conf->assumePropagation)
			fpm.addPass(PropagateAssumesPass());
		if (conf->confirmation)
			fpm.addPass(ConfirmationAnnotationPass());
		if (conf->loadAnnot)
			fpm.addPass(LoadAnnotationPass(PI.annotInfo));
		basicOptsMGR.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(fpm)));
	}

	preserved.intersect(basicOptsMGR.run(mod, mam));

	initializeModuleInfo(MI, PI);

	assert(!llvm::verifyModule(mod, &llvm::dbgs()));
	return true;
}

void printLLVMModule(llvm::Module &mod, const std::string &out)
{
	auto flags =
#if LLVM_VERSION_MAJOR < 13
		llvm::sys::fs::F_None;
#else
		llvm::sys::fs::OF_None;
#endif
	std::error_code errs;
	auto os = std::make_unique<llvm::raw_fd_ostream>(out.c_str(), errs, flags);

	/* TODO: Do we need an exception? If yes, properly handle it */
	if (errs) {
		WARN("Failed to write transformed module to file " + out + ": " + errs.message());
		return;
	}
	mod.print(*os, nullptr);
}

} // namespace LLVMModule
