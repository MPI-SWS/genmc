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
#include "Error.hpp"
#include "MDataCollectionPass.hpp"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace llvm;

#ifdef LLVM_EXECUTIONENGINE_DATALAYOUT_PTR
# define GET_TYPE_ALLOC_SIZE(M, x)		\
	(M).getDataLayout()->getTypeAllocSize((x))
#else
# define GET_TYPE_ALLOC_SIZE(M, x)		\
	(M).getDataLayout().getTypeAllocSize((x))
#endif

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
void MDataCollectionPass::collectVarName(Module &M, unsigned int ptr, Type *typ,
					 DIType *dit, std::string nameBuilder,
					 std::vector<std::pair<unsigned int, std::string > > &names)
{
	if(!isa<CompositeType>(typ)) {
		names.push_back(std::make_pair(ptr, nameBuilder));
		return;
	}

	unsigned int offset = 0;
	if (SequentialType *AT = dyn_cast<SequentialType>(typ)) {
		if (auto *dict = dyn_cast<DICompositeType>(dit)) {
			unsigned int elemSize = GET_TYPE_ALLOC_SIZE(M, AT->getElementType());
			for (auto i = 0u; i < AT->getNumElements(); i++) {
				if (!dict->getBaseType()) {
					collectVarName(M, ptr + offset,
						       AT->getElementType(), dict,
						       nameBuilder + "[" +
						       std::to_string(i) + "]",
						       names);
				} else if (auto *dibt = dyn_cast<DIType>(dict->getBaseType())) {
					collectVarName(M, ptr + offset,
						       AT->getElementType(), dibt,
						       nameBuilder + "[" +
						       std::to_string(i) + "]",
						       names);
				}
				offset += elemSize;
			}
		}
	} else if (StructType *ST = dyn_cast<StructType>(typ)) {
		if (auto *dict = dyn_cast<DICompositeType>(dit)) {
			auto dictElems = dict->getElements();
			auto eb = ST->element_begin();
			for (auto it = ST->element_begin(); it != ST->element_end(); ++it) {
				unsigned int elemSize = GET_TYPE_ALLOC_SIZE(M, *it);
				auto didt = dictElems[it - eb];
				if (auto *dit = dyn_cast<DIDerivedType>(didt)) {
					if (auto ditb = dyn_cast<DIType>(dit->getBaseType()))
						collectVarName(M, ptr + offset, *it,
							       ditb, nameBuilder + "."
							       + dit->getName().str(),
							       names);
				}
				offset += elemSize;
			}
		}
	}
	return;
}
#else
void MDataCollectionPass::collectVarName(unsigned int ptr, unsigned int typeSize,
					 llvm::Type *typ, std::string nameBuilder,
					 std::vector<std::pair<unsigned int, std::string > > &names)
{
	if (auto *AT = dyn_cast<ArrayType>(typ)) {
		unsigned int elemTypeSize = typeSize / AT->getArrayNumElements();
		for (auto i = 0u, s = 0u; s < typeSize; i++, s += elemTypeSize) {
			std::string name = nameBuilder + "[" + std::to_string(i) + "]";
			names.push_back(std::make_pair(ptr + s, name));
		}
	} else {
		names.push_back(std::make_pair(ptr, nameBuilder));
	}
}
#endif

void MDataCollectionPass::collectGlobalInfo(Module &M)
{
	for (auto &v : M.getGlobalList()) {
#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
		if (!v.getMetadata("dbg"))
			continue;

		BUG_ON(!isa<DIGlobalVariableExpression>(v.getMetadata("dbg")));
		auto *dive = static_cast<DIGlobalVariableExpression *>(v.getMetadata("dbg"));
		auto dit = dive->getVariable()->getType();

		/* Check whether it is a global pointer */
		if (auto ditc = dyn_cast<DIType>(dit))
			collectVarName(M, 0, v.getType()->getElementType(),
				       ditc, v.getName(), VI.globalInfo[&v]);
#else
		unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, v.getType()->getElementType());
		collectVarName(0, typeSize, v.getType()->getElementType(),
			       v.getName(), VI.globalInfo[&v]);
#endif
		std::sort(VI.globalInfo[&v].begin(), VI.globalInfo[&v].end());
	}
}

void MDataCollectionPass::runOnBasicBlock(BasicBlock &BB, Module &M)
{
	for (auto it = BB.begin(); it != BB.end();  ++it) {
		if (auto *DD = dyn_cast<DbgDeclareInst>(&*it)) {
#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
			auto dit = DD->getVariable()->getType();

			auto *v = DD->getAddress();
			BUG_ON(!isa<PointerType>(v->getType()));
		        auto *vt = static_cast<PointerType *>(v->getType());

			if (auto ditc = dyn_cast<DIType>(dit))
				collectVarName(M, 0, vt->getElementType(),
					       ditc, v->getName(), VI.localInfo[v]);
#else
			auto *v = DD->getAddress();
			BUG_ON(!isa<PointerType>(v->getType()));
		        auto *vt = static_cast<PointerType *>(v->getType());

			unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, vt->getElementType());
			collectVarName(0, typeSize, vt->getElementType(), v->getName(), VI.localInfo[v]);
#endif
			std::sort(VI.localInfo[v].begin(), VI.localInfo[v].end());
		}
	}
	return;
}

bool MDataCollectionPass::runOnModule(Module &M)
{
	if (collected)
		return false;

	/* First, get type information for all global variables */
	collectGlobalInfo(M);

	/* Scan through the instructions and lower intrinsic calls */
	for (auto &F : M)
		for (auto &BB : F)
			runOnBasicBlock(BB, M);

	collected = true;
	return false;
}

char MDataCollectionPass::ID = 42;
