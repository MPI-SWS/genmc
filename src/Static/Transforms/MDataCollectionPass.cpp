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

#include "MDataCollectionPass.hpp"
#include "Static/LLVMUtils.hpp"
#include "Support/Error.hpp"
#include "config.h"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

using namespace llvm;

void collectVarName(Module &M, unsigned int ptr, Type *typ, DIType *dit, std::string nameBuilder,
		    NameInfo &info)
{
	if (!isa<StructType>(typ) && !isa<ArrayType>(typ) && !isa<VectorType>(typ)) {
		info.addOffsetInfo(ptr, nameBuilder);
		return;
	}

	unsigned int offset = 0;
	if (auto *AT = dyn_cast<ArrayType>(typ)) {
		auto *newDit = dit;
		if (auto *dict = dyn_cast<DICompositeType>(dit)) {
			newDit = (!dict->getBaseType())
					 ? dict
					 : llvm::dyn_cast<DIType>(dict->getBaseType());
		}
		auto elemSize = M.getDataLayout().getTypeAllocSize(AT->getElementType());
		for (auto i = 0u; i < AT->getNumElements(); i++) {
			collectVarName(M, ptr + offset, AT->getElementType(), newDit,
				       nameBuilder + "[" + std::to_string(i) + "]", info);
			offset += elemSize;
		}
	} else if (auto *ST = dyn_cast<StructType>(typ)) {
		DINodeArray dictElems;

		/* Since this is a struct type, the metadata should yield a
		 * composite type, or a derived type that will eventually
		 * yield a composite type. */
		if (auto *dict = dyn_cast<DICompositeType>(dit)) {
			dictElems = dict->getElements();
		}
		if (auto *dict = dyn_cast<DIDerivedType>(dit)) {
			DIType *dbt = dict;
			while (auto *dbtc = dyn_cast<DIDerivedType>(dbt))
				dbt = dyn_cast<DIType>(dbtc->getBaseType());

			if (auto *dbtc = dyn_cast<DICompositeType>(dbt))
				dictElems = dbtc->getElements();
			else {
				/* Take some precautions, in case we got this
				 * metadata thing all wrong... */
				PRINT_BUGREPORT_INFO_ONCE("struct-mdata",
							  "Cannot get variable naming info");
				return;
			}
		}

		/* It can be dictElems.size() < ST->getNumElements(), e.g., for va_arg */
		auto i = 0u;
		auto minSize = std::min(dictElems.size(), ST->getNumElements());
		for (auto it = ST->element_begin(); i < minSize; ++it, ++i) {
			auto elemSize = M.getDataLayout().getTypeAllocSize(*it);
			auto didt = dictElems[i];
			if (auto *dit = dyn_cast<DIDerivedType>(didt)) {
				if (auto ditb = dyn_cast<DIType>(dit->getBaseType()))
					collectVarName(M, ptr + offset, *it, ditb,
						       nameBuilder + "." + dit->getName().str(),
						       info);
			}
			offset += elemSize;
		}
	} else {
		BUG();
	}
}

auto collectVarName(Module &M, Type *typ, DIType *dit) -> NameInfo
{
	NameInfo info;
	collectVarName(M, 0, typ, dit, "", info);
	return info;
}

void MDataInfo::collectGlobalInfo(GlobalVariable &v, Module &M)
{
	/* If we already have data for the variable, skip */
	if (hasGlobalInfo(&v))
		return;

	auto &info = getGlobalInfo(&v);
	info = std::make_shared<NameInfo>();

	if (!v.getMetadata("dbg"))
		return;

	BUG_ON(!isa<DIGlobalVariableExpression>(v.getMetadata("dbg")));
	auto *dive = static_cast<DIGlobalVariableExpression *>(v.getMetadata("dbg"));
	auto dit = dive->getVariable()->getType();

	/* Check whether it is a global pointer */
	if (auto *ditc = dyn_cast<DIType>(dit))
		*info = collectVarName(M, v.getValueType(), ditc);
}

void MDataInfo::collectLocalInfo(DbgDeclareInst *DD, Module &M)
{
	/* Skip if it's not an alloca or we don't have data */
	auto *v = dyn_cast<AllocaInst>(DD->getAddress());
	if (!v)
		return;

	if (hasLocalInfo(v))
		return;

	auto &info = getLocalInfo(v);
	info = std::make_shared<NameInfo>();

	/* Store alloca's metadata, in case it's used in memcpy */
	allocaMData[v] = DD->getVariable();

	auto dit = DD->getVariable()->getType();
	if (auto *ditc = dyn_cast<DIType>(dit))
		*info = collectVarName(M, v->getAllocatedType(), ditc);
}

void MDataInfo::collectMemCpyInfo(MemCpyInst *mi, Module &M)
{
	/*
	 * Only mark global variables w/ private linkage that will
	 * otherwise go undetected by this pass
	 */
	auto *src = dyn_cast<GlobalVariable>(mi->getSource());
	if (!src)
		return;

	if (hasGlobalInfo(src))
		return;

	auto &info = getGlobalInfo(src);
	info = std::make_shared<NameInfo>();

	/*
	 * Since there will be no metadata for variables with private linkage,
	 * we do a small hack and take the metadata of memcpy()'s dest for
	 * memcpy()'s source
	 * The type of the dest is guaranteed to be a pointer
	 */
	auto *dst = dyn_cast<AllocaInst>(mi->getDest());
	BUG_ON(!dst);

	if (allocaMData.count(dst) == 0)
		return; /* We did our best, but couldn't get a name for it... */
	auto dit = allocaMData[dst]->getType();
	if (auto *ditc = dyn_cast<DIType>(dit))
		*info = collectVarName(M, dst->getAllocatedType(), ditc);
}

auto MDataInfo::run(Module &M, ModuleAnalysisManager &AM) -> Result
{
	/* First, get type information for user's global variables */
	for (auto &v : GLOBALS(M))
		collectGlobalInfo(v, M);

	/* Then for all local variables and some other special cases */
	for (auto &F : M) {
		for (auto &I : instructions(F)) {
			if (auto *dd = dyn_cast<DbgDeclareInst>(&I))
				collectLocalInfo(dd, M);
			if (auto *mi = dyn_cast<MemCpyInst>(&I))
				collectMemCpyInfo(mi, M);
		}
	}
	return PMI;
}

auto MDataCollectionPass::run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses
{
	PMI = MAM.getResult<MDataInfo>(M);
	return PreservedAnalyses::all();
}
