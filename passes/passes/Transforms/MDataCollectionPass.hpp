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

#ifndef GENMC_MDATA_COLLECTION_PASS_HPP
#define GENMC_MDATA_COLLECTION_PASS_HPP

#include "genmc/Support/NameInfo.hpp"
#include "passes/ModuleInfo.hpp"

#include <llvm/Analysis/LoopPass.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>

#include <string>
#include <unordered_map>
#include <vector>

using namespace llvm;

class MDataInfo : public AnalysisInfoMixin<MDataInfo> {

public:
	using Result = PassModuleInfo;
	auto run(Module &M, ModuleAnalysisManager &AM) -> Result;

	auto collectMData(Module &M) -> MDataInfo::Result;

private:
	/* Collects name info for a global variable */
	void collectGlobalInfo(llvm::GlobalVariable &v, llvm::Module &M);

	/* Collects name info for a stack variable */
	void collectLocalInfo(llvm::DbgDeclareInst *dbgDecl, llvm::Module &M);

	/*
	 * Collects name info about global variables w/ private linkage
	 * used in memory intrinsics
	 */
	void collectMemCpyInfo(llvm::MemCpyInst *MI, llvm::Module &M);

	/* Check whether the respective information exist */
	auto hasGlobalInfo(llvm::Value *gv) const -> bool
	{
		return PMI.varInfo.globalInfo.contains(gv);
	}
	auto hasLocalInfo(llvm::Value *lv) const -> bool
	{
		return PMI.varInfo.localInfo.contains(lv);
	}
	auto hasInternalInfo(const std::string &key) const -> bool
	{
		return PMI.varInfo.internalInfo.contains(key);
	}

	/* Getters for collected naming info */
	auto getGlobalInfo(llvm::Value *gv) -> std::shared_ptr<NameInfo> &
	{
		return PMI.varInfo.globalInfo[gv];
	}
	auto getLocalInfo(llvm::Value *lv) -> std::shared_ptr<NameInfo> &
	{
		return PMI.varInfo.localInfo[lv];
	}
	auto getInternalInfo(const std::string &key) -> std::shared_ptr<NameInfo> &
	{
		return PMI.varInfo.internalInfo[key];
	}

	friend AnalysisInfoMixin<MDataInfo>;
	static inline AnalysisKey Key;

	/* Maps allocas to the metadata of the variable allocated */
	std::unordered_map<llvm::AllocaInst *, llvm::DILocalVariable *> allocaMData;

	PassModuleInfo PMI;
};

class MDataCollectionPass : public AnalysisInfoMixin<MDataCollectionPass> {
public:
	MDataCollectionPass(PassModuleInfo &PMI) : PMI(PMI) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses;

private:
	MDataInfo info{};

	/* We have to extract the necessary information out of this pass.
	 * If we try to get them in another pass (e.g., w/ getAnalysis()),
	 * then a new instance of this pass may be created (e.g., if the pass
	 * gets invalidated), and we will lose all the data we have collected.  */
	PassModuleInfo &PMI; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
};

#endif /* GENMC_MDATA_COLLECTION_PASS_HPP */
