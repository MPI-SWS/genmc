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

#include "Interpreter.h"
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopPass.h>

#include <string>
#include <unordered_map>
#include <vector>

class MDataCollectionPass : public llvm::ModulePass {

public:
	static char ID;
	llvm::VariableInfo &VI;
	llvm::FsInfo &FI;

	MDataCollectionPass(llvm::VariableInfo &VI, llvm::FsInfo &FI)
		: llvm::ModulePass(ID), VI(VI), FI(FI), collected(false) {}

	virtual bool runOnModule(llvm::Module &M);

protected:

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	void collectVarName(llvm::Module &M, unsigned int ptr, llvm::Type *typ,
			    llvm::DIType *dit, std::string nameBuilder,
			    std::vector<std::pair<unsigned int, std::string > > &names);
#else
	void collectVarName(unsigned int ptr, unsigned int typeSize,
			    llvm::Type *typ, std::string nameBuilder,
			    std::vector<std::pair<unsigned int, std::string > > &names);

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

	/* Maps allocas to the metadata of the variable allocated */
	std::unordered_map<llvm::AllocaInst *, llvm::DILocalVariable *> allocaMData;

	/* Whether we have collected metadata or not */
	bool collected;
};

#endif /* __MDATA_COLLECTION_PASS_HPP__ */
