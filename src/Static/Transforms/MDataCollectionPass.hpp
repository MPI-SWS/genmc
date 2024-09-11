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

#ifndef GENMC_MDATA_COLLECTION_PASS_HPP
#define GENMC_MDATA_COLLECTION_PASS_HPP

#include "Static/ModuleInfo.hpp"
#include "Support/NameInfo.hpp"
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
	void collectLocalInfo(llvm::DbgDeclareInst *DD, llvm::Module &M);

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

	void collectFilename(const std::string &name) { PMI.filenames.insert(name); }

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
	PassModuleInfo &PMI;
};

#endif /* GENMC_MDATA_COLLECTION_PASS_HPP */
