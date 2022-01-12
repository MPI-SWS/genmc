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
#include "CallInstWrapper.hpp"
#include "Error.hpp"
#include "MDataCollectionPass.hpp"
#include "LLVMUtils.hpp"
#include <llvm/ADT/Twine.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DebugInfo.h>
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

void MDataCollectionPass::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
	au.setPreservesAll();
}

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
void MDataCollectionPass::collectVarName(Module &M, unsigned int ptr, Type *typ,
					 DIType *dit, std::string nameBuilder, NameInfo &info)
{
	if(!isa<StructType>(typ) && !isa<ArrayType>(typ) && !isa<VectorType>(typ)) {
		info.addOffsetInfo(ptr, nameBuilder);
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
				       nameBuilder + "[" + std::to_string(i) + "]", info);
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
						       info);
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
					 llvm::Type *typ, std::string nameBuilder, NameInfo &info)
{
	if (auto *AT = dyn_cast<ArrayType>(typ)) {
		unsigned int elemTypeSize = typeSize / AT->getArrayNumElements();
		for (auto i = 0u, s = 0u; s < typeSize; i++, s += elemTypeSize) {
			std::string name = nameBuilder + "[" + std::to_string(i) + "]";
			info.addOffsetInfo(ptr + s, name);
		}
	} else {
		info.addOffsetInfo(ptr, nameBuilder);
	}
}
#endif

void MDataCollectionPass::collectGlobalInfo(GlobalVariable &v, Module &M)
{
	/* If we already have data for the variable, skip */
	auto &info = getGlobalInfo(&v);
	if (!info.empty())
		return;

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	if (!v.getMetadata("dbg"))
		return;

	BUG_ON(!isa<DIGlobalVariableExpression>(v.getMetadata("dbg")));
	auto *dive = static_cast<DIGlobalVariableExpression *>(v.getMetadata("dbg"));
	auto dit = dive->getVariable()->getType();

	/* Check whether it is a global pointer */
	if (auto ditc = dyn_cast<DIType>(dit))
		collectVarName(M, 0, v.getType()->getElementType(), ditc, "", info);
#else
	unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, v.getType()->getElementType());
	collectVarName(0, typeSize, v.getType()->getElementType(), "", info);
#endif
	return;
}

void MDataCollectionPass::collectLocalInfo(DbgDeclareInst *DD, Module &M)
{
	/* Skip if it's not an alloca or we don't have data */
	auto *v = dyn_cast<AllocaInst>(DD->getAddress());
	if (!v)
		return;

	auto &info = getLocalInfo(v);
	if (!info.empty())
		return;

	auto *vt = dyn_cast<PointerType>(v->getType());
	BUG_ON(!vt);

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	/* Store alloca's metadata, in case it's used in memcpy */
	allocaMData[v] = DD->getVariable();

	auto dit = DD->getVariable()->getType();
	if (auto ditc = dyn_cast<DIType>(dit))
		collectVarName(M, 0, vt->getElementType(), ditc, "", info);
#else
	unsigned int typeSize =
		GET_TYPE_ALLOC_SIZE(M, vt->getElementType());
	collectVarName(0, typeSize, vt->getElementType(), "", info);
#endif
	return;
}

void MDataCollectionPass::collectMemCpyInfo(MemCpyInst *mi, Module &M)
{
	/*
	 * Only mark global variables w/ private linkage that will
	 * otherwise go undetected by this pass
	 */
	auto *src = dyn_cast<GlobalVariable>(mi->getSource());
	if (!src)
		return;
	auto &info = getGlobalInfo(src);
	if (!info.empty())
		return;

	/*
	 * Since there will be no metadata for variables with private linkage,
	 * we do a small hack and take the metadata of memcpy()'s dest for
	 * memcpy()'s source
	 * The type of the dest is guaranteed to be a pointer
	 */
	auto *dst = dyn_cast<AllocaInst>(mi->getDest());
	BUG_ON(!dst);
	auto *dstTyp = dyn_cast<PointerType>(dst->getType());

#ifdef LLVM_HAS_GLOBALOBJECT_GET_METADATA
	if (allocaMData.count(dst) == 0)
		return; /* We did our best, but couldn't get a name for it... */
	auto dit = allocaMData[dst]->getType();
	if (auto ditc = dyn_cast<DIType>(dit))
		collectVarName(M, 0, dstTyp->getElementType(), ditc, "", info);
#else
	unsigned int typeSize = GET_TYPE_ALLOC_SIZE(M, dstTyp->getElementType());
	collectVarName(0, typeSize, dstTyp->getElementType(), "", info);
#endif
	return;
}

/* We need to take special care so that these internal types
 * actually match the ones used by the ExecutionEngine */
void MDataCollectionPass::collectInternalInfo(Module &M)
{
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
	getInternalInfo("file").addOffsetInfo(offset, ".inode");
	getInternalInfo("file").addOffsetInfo((offset += voidPtrByteWidth), ".count");
	getInternalInfo("file").addOffsetInfo((offset += intByteWidth), ".flags");
	getInternalInfo("file").addOffsetInfo((offset += intByteWidth), ".pos_lock");
	getInternalInfo("file").addOffsetInfo((offset += intByteWidth), ".pos");

	/* struct inode */
	offset = 0;
	getInternalInfo("inode").addOffsetInfo(offset, ".lock");
	getInternalInfo("inode").addOffsetInfo((offset += intByteWidth), ".i_size");
	getInternalInfo("inode").addOffsetInfo((offset += intByteWidth), ".i_transaction");
	getInternalInfo("inode").addOffsetInfo((offset += intByteWidth), ".i_disksize");
	getInternalInfo("inode").addOffsetInfo((offset += intByteWidth), ".data");
	return;
}

bool isSyscallWPathname(CallInst *CI)
{
	/* Use getCalledValue() to deal with indirect invocations too */
	auto name = getCalledFunOrStripValName(*CI);
	if (!isInternalFunction(name))
		return false;

	auto icode = internalFunNames.at(name);
	return isFsInodeCode(icode);
}

void MDataCollectionPass::initializeFilenameEntry(Value *v)
{
	if (auto *CE = dyn_cast<ConstantExpr>(v)) {
		auto filename = dyn_cast<ConstantDataArray>(
			dyn_cast<GlobalVariable>(CE->getOperand(0))->
			getInitializer())->getAsCString().str();
		collectFilename(filename);
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
	initializeFilenameEntry(CI->getArgOperand(0));

	/* For some syscalls we capture the second argument as well */
	auto fCode = internalFunNames.at(F->getName().str());
	if (fCode == InternalFunctions::FN_RenameFS ||
	    fCode == InternalFunctions::FN_LinkFS) {
		initializeFilenameEntry(CI->getArgOperand(1));
	}
	return;
}

bool MDataCollectionPass::runOnModule(Module &M)
{
	if (!PI)
		return false;

	/* First, get type information for user's global variables */
	for (auto &v : M.getGlobalList())
		collectGlobalInfo(v, M);

	/* Then for all local variables and some other special cases */
	for (auto &F : M) {
		for (auto it = inst_iterator(F), ei = inst_end(F); it != ei; ++it) {
			if (auto *dd = dyn_cast<DbgDeclareInst>(&*it))
				collectLocalInfo(dd, M);
			if (auto *mi = dyn_cast<MemCpyInst>(&*it))
				collectMemCpyInfo(mi, M);
			if (auto *ci = dyn_cast<CallInst>(&*it)) {
				if (isSyscallWPathname(ci))
					collectFilenameInfo(ci, M);
			}
		}
	}

	/* Finally, collect internal type information */
	collectInternalInfo(M);
	return false;
}

ModulePass *createMDataCollectionPass(PassModuleInfo *PI)
{
	auto *P = new MDataCollectionPass();
	P->setPassModuleInfo(PI);
	return P;
}

char MDataCollectionPass::ID = 42;
static llvm::RegisterPass<MDataCollectionPass> P("mdata-collection",
						 "Collections various metadata of a module.");
