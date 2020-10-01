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
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
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
	if(!isa<StructType>(typ) && !isa<ArrayType>(typ) && !isa<VectorType>(typ)) {
		names.push_back(std::make_pair(ptr, nameBuilder));
		return;
	}

	unsigned int offset = 0;
	if (ArrayType *AT = dyn_cast<ArrayType>(typ)) {
		auto *newDit = dit;
		if (auto *dict = dyn_cast<DICompositeType>(dit)) {
			newDit = (!dict->getBaseType()) ? dict :
				llvm::dyn_cast<DIType>(dict->getBaseType());
		}
		unsigned int elemSize = GET_TYPE_ALLOC_SIZE(M, AT->getElementType());
		for (auto i = 0u; i < AT->getNumElements(); i++) {
			collectVarName(M, ptr + offset, AT->getElementType(), newDit,
				       nameBuilder + "[" + std::to_string(i) + "]", names);
			offset += elemSize;
		}
	} else if (StructType *ST = dyn_cast<StructType>(typ)) {
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
				WARN_ONCE("struct-mdata", "Could not get " \
					  "naming info for variable!\n" \
					  "Please submit a bug report to " \
					  PACKAGE_BUGREPORT "\n");
				return;
			}
		}

		/* It can be dictElems.size() < ST->getNumElements(), e.g., for va_arg */
		auto i = 0u;
		auto minSize = std::min(dictElems.size(), ST->getNumElements());
		for (auto it = ST->element_begin(); i < minSize; ++it, ++i) {
			unsigned int elemSize = GET_TYPE_ALLOC_SIZE(M, *it);
			auto didt = dictElems[i];
			if (auto *dit = dyn_cast<DIDerivedType>(didt)) {
				if (auto ditb = dyn_cast<DIType>(dit->getBaseType()))
					collectVarName(M, ptr + offset, *it, ditb,
						       nameBuilder + "." + dit->getName().str(),
						       names);
			}
			offset += elemSize;
		}
	} else {
		BUG();
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

void MDataCollectionPass::collectGlobalInfo(GlobalVariable &v, Module &M)
{
#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	if (!v.getMetadata("dbg"))
		return;

	BUG_ON(!isa<DIGlobalVariableExpression>(v.getMetadata("dbg")));
	auto *dive = static_cast<DIGlobalVariableExpression *>(v.getMetadata("dbg"));
	auto dit = dive->getVariable()->getType();

	/* Check whether it is a global pointer */
	if (auto ditc = dyn_cast<DIType>(dit)) {
		collectVarName(M, 0, v.getType()->getElementType(),
			       ditc, "", VI.globalInfo[&v]);
	}
#else
	unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, v.getType()->getElementType());
	collectVarName(0, typeSize, v.getType()->getElementType(),
		       "", VI.globalInfo[&v]);
#endif

	std::sort(VI.globalInfo[&v].begin(), VI.globalInfo[&v].end());
	std::unique(VI.globalInfo[&v].begin(), VI.globalInfo[&v].end());
	return;
}

void MDataCollectionPass::collectLocalInfo(DbgDeclareInst *DD, Module &M)
{
	auto *v = DD->getAddress();
	BUG_ON(!isa<PointerType>(v->getType()));
	auto *vt = static_cast<PointerType *>(v->getType());

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	/* Store alloca's metadata, in case it's used in memcpy */
	allocaMData[dyn_cast<AllocaInst>(v)] = DD->getVariable();

	auto dit = DD->getVariable()->getType();
	if (auto ditc = dyn_cast<DIType>(dit))
		collectVarName(M, 0, vt->getElementType(),
			       ditc, "", VI.localInfo[v]);
#else
	unsigned int typeSize =
		GET_TYPE_ALLOC_SIZE(M, vt->getElementType());
	collectVarName(0, typeSize, vt->getElementType(),
		       "", VI.localInfo[v]);
#endif

	std::sort(VI.localInfo[v].begin(), VI.localInfo[v].end());
	std::unique(VI.localInfo[v].begin(), VI.localInfo[v].end());
	return;
}

void MDataCollectionPass::collectMemCpyInfo(MemCpyInst *MI, Module &M)
{
	/*
	 * Only mark global variables w/ private linkage that will
	 * otherwise go undetected by this pass
	 */
	auto *src = dyn_cast<GlobalVariable>(MI->getSource());
	if (!src || VI.globalInfo[src].size())
		return;

	/*
	 * Since there will be no metadata for variables with private linkage,
	 * we do a small hack and take the metadata of memcpy()'s dest for
	 * memcpy()'s source
	 * The type of the dest is guaranteed to be a pointer
	 */
	auto *dst = dyn_cast<AllocaInst>(MI->getDest());
	BUG_ON(!dst);
	auto *dstTyp = dyn_cast<PointerType>(dst->getType());

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	if (allocaMData.count(dst) == 0)
		return; /* We did our best, but couldn't get a name for it... */
	auto dit = allocaMData[dst]->getType();
	if (auto ditc = dyn_cast<DIType>(dit))
		collectVarName(M, 0, dstTyp->getElementType(),
			       ditc, "", VI.globalInfo[src]);
#else
	unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, dstTyp->getElementType());
	collectVarName(0, typeSize, dstTyp->getElementType(),
		       "", VI.globalInfo[src]);
#endif

	std::sort(VI.globalInfo[src].begin(), VI.globalInfo[src].end());
	std::unique(VI.globalInfo[src].begin(), VI.globalInfo[src].end());
	return;
}

/* We need to take special care so that these internal types
 * actually match the ones used by the ExecutionEngine */
void MDataCollectionPass::collectInternalInfo(Module &M)
{
	using IT = VariableInfo::InternalType;

	/* We need to find out the size of an integer and the size of a pointer
	 * in this platform. HACK: since all types can be safely converted to
	 * void *, we take the size of a void * to see how many bytes are
	 * necessary to represent a pointer, and get the integer type from
	 * main()'s return type... */
	auto *main = M.getFunction("main");
	if (!main || !main->getReturnType()->isIntegerTy()) {
		WARN_ONCE("internal-mdata", "Could not get "	\
			  "naming info for internal variable. "	\
			  "(Does main() return int?)\n"		\
			  "Please submit a bug report to "	\
			  PACKAGE_BUGREPORT "\n");
		return;
	}

	auto *voidPtrTyp = Type::getVoidTy(M.getContext())->getPointerTo();
	unsigned int voidPtrByteWidth = GET_TYPE_ALLOC_SIZE(M, voidPtrTyp);

	auto *intTyp = main->getReturnType();
	unsigned int intByteWidth = GET_TYPE_ALLOC_SIZE(M, intTyp);

	/* struct file */
	unsigned int offset = 0;
	VI.internalInfo["file"].push_back(
		std::make_pair(offset, ".inode"));
	VI.internalInfo["file"].push_back(
		std::make_pair((offset += voidPtrByteWidth), ".count"));
	VI.internalInfo["file"].push_back(
		std::make_pair((offset += intByteWidth), ".flags"));
	VI.internalInfo["file"].push_back(
		std::make_pair((offset += intByteWidth), ".pos_lock"));
	VI.internalInfo["file"].push_back(
		std::make_pair((offset += intByteWidth), ".pos"));

	/* struct inode */
	offset = 0;
	VI.internalInfo["inode"].push_back(
		std::make_pair(offset, ".lock"));
	VI.internalInfo["inode"].push_back(
		std::make_pair((offset += intByteWidth), ".i_size"));
	VI.internalInfo["inode"].push_back(
		std::make_pair((offset += intByteWidth), ".i_transaction"));
	VI.internalInfo["inode"].push_back(
		std::make_pair((offset += intByteWidth), ".i_disksize"));
	VI.internalInfo["inode"].push_back(
		std::make_pair((offset += intByteWidth), ".data"));
	return;
}

bool isSyscallWPathname(CallInst *CI)
{
	/* Use getCalledValue() to deal with indirect invocations too */
	auto name = CallInstWrapper(CI).getCalledOperand()->getName().str();
	if (!IS_INTERNAL_FUNCTION(name))
		return false;

	auto icode = internalFunNames.at(name);
	return IS_FS_INODE_FN_CODE(icode);
}

void initializeFilenameEntry(FsInfo &FI, Value *v)
{
	if (auto *CE = dyn_cast<ConstantExpr>(v)) {
		auto filename = dyn_cast<ConstantDataArray>(
			dyn_cast<GlobalVariable>(CE->getOperand(0))->
			getInitializer())->getAsCString().str();
		FI.nameToInodeAddr[filename] = (char *) 0xdeadbeef;
	} else
		ERROR("Non-constant expression in filename\n");
	return;
}

void MDataCollectionPass::collectFilenameInfo(CallInst *CI, Module &M)
{
	auto *F = CI->getCalledFunction();
	auto ai = CI->arg_begin();

	/* Fetch the first argument of the syscall as a string.
	 * We simply initialize the entries in the map; they will be
	 * populated with actual addresses from the EE */
	initializeFilenameEntry(FI, CI->getArgOperand(0));

	/* For some syscalls we capture the second argument as well */
	auto fCode = internalFunNames.at(F->getName().str());
	if (fCode == InternalFunctions::FN_RenameFS ||
	    fCode == InternalFunctions::FN_LinkFS) {
		initializeFilenameEntry(FI, CI->getArgOperand(1));
	}
	return;
}

bool MDataCollectionPass::runOnModule(Module &M)
{
	if (collected)
		return false;

	/* First, get type information for user's global variables */
	for (auto &v : M.getGlobalList())
		collectGlobalInfo(v, M);

	/* Then for all local variables and some other special cases */
	for (auto &F : M) {
		for (auto it = inst_iterator(F), ei = inst_end(F); it != ei; ++it) {
			if (auto *DD = dyn_cast<DbgDeclareInst>(&*it))
				collectLocalInfo(DD, M);
			if (auto *MI = dyn_cast<MemCpyInst>(&*it))
				collectMemCpyInfo(MI, M);
			if (auto *CI = dyn_cast<CallInst>(&*it)) {
				if (isSyscallWPathname(CI))
					collectFilenameInfo(CI, M);
			}
		}
	}

	/* Finally, collect internal type information */
	collectInternalInfo(M);

	collected = true;
	return false;
}

char MDataCollectionPass::ID = 42;
