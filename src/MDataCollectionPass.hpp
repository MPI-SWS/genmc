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

#ifndef __MDATA_COLLECTION_PASS_HPP__
#define __MDATA_COLLECTION_PASS_HPP__

#include "NameInfo.hpp"
#include "ModuleInfo.hpp"
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopPass.h>

#include <string>
#include <unordered_map>
#include <vector>

class MDataCollectionPass : public llvm::ModulePass {

public:
	static char ID;

	MDataCollectionPass() : llvm::ModulePass(ID) {}

	void setPassModuleInfo(PassModuleInfo *I) { PI = I; }

	virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
	virtual bool runOnModule(llvm::Module &M) override;

protected:

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	void collectVarName(llvm::Module &M, unsigned int ptr, llvm::Type *typ,
			    llvm::DIType *dit, std::string nameBuilder, NameInfo &names);
#else
	void collectVarName(unsigned int ptr, unsigned int typeSize,
			    llvm::Type *typ, std::string nameBuilder, NameInfo &names);

#endif

	/* Collects name info for a global variable */
	void collectGlobalInfo(llvm::GlobalVariable &v, llvm::Module &M);

	/* Collects name info for a stack variable */
	void collectLocalInfo(llvm::DbgDeclareInst *DD, llvm::Module &M);

	/* Collect name info for internal types */
	void collectInternalInfo(llvm::Module &M);

	/* Collect info about the files used */
	void collectFilenameInfo(llvm::CallInst *DD, llvm::Module &M);

	/*
	 * Collects name info about global variables w/ private linkage
	 * used in memory intrinsics
	 */
	void collectMemCpyInfo(llvm::MemCpyInst *MI, llvm::Module &M);

	/* Marks V's name as used in the module.
	 * Reports an error if the name is not a constant  */
	void initializeFilenameEntry(llvm::Value *v);

	/* Check whether the respective information exist */
	bool hasGlobalInfo(llvm::Value *gv) const { return PI->varInfo.globalInfo[gv].get(); }
	bool hasLocalInfo(llvm::Value *lv) const { return PI->varInfo.localInfo[lv].get(); }
	bool hasInternalInfo(const std::string &key) const { return PI->varInfo.internalInfo[key].get(); }

	/* Getters for collected naming info */
	std::shared_ptr<NameInfo> &getGlobalInfo(llvm::Value *gv) { return PI->varInfo.globalInfo[gv]; }
	std::shared_ptr<NameInfo> &getLocalInfo(llvm::Value *lv) { return PI->varInfo.localInfo[lv]; }
	std::shared_ptr<NameInfo> &getInternalInfo(const std::string &key) { return PI->varInfo.internalInfo[key]; }

	void collectFilename(const std::string &name) { PI->filenames.insert(name); }

private:
	/* Maps allocas to the metadata of the variable allocated */
	std::unordered_map<llvm::AllocaInst *, llvm::DILocalVariable *> allocaMData;

	/* We have to extract the necessary information out of this pass.
	 * If we try to get them in another pass (e.g., w/ getAnalysis()),
	 * then a new instance of this pass may be created (e.g., if the pass
	 * gets invalidated), and we will lose all the data we have collected.  */
	PassModuleInfo *PI = nullptr;
};

#endif /* __MDATA_COLLECTION_PASS_HPP__ */
