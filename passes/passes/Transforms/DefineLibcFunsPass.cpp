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

#include "DefineLibcFunsPass.hpp"
#include "genmc/Support/Error.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Passes/PassPlugin.h>

using namespace llvm;

void replaceFunWithNop(Module &M, std::string name)
{
	auto *F = M.getFunction(name);
	if (!F || !F->isDeclaration())
		return;

	Value *res = nullptr;
	Type *retTy = F->getReturnType();
	if (retTy->isIntegerTy())
		res = ConstantInt::get(retTy, 0);
	else if (retTy->isVoidTy())
		res = nullptr;
	else if (retTy->isPointerTy())
		res = ConstantPointerNull::get(dyn_cast<PointerType>(retTy));
	else
		WARN("Could not add definition for {}!\n", name);

	auto *BB = BasicBlock::Create(F->getContext(), "", F);
	ReturnInst::Create(F->getContext(), res, BB);
}

auto DefineLibcFunsPass::run(Module &M, ModuleAnalysisManager &AM) -> PreservedAnalyses
{
	replaceFunWithNop(M, "fclose");
	replaceFunWithNop(M, "fopen");
	replaceFunWithNop(M, "fflush");
	replaceFunWithNop(M, "fprintf");
	return PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
auto getDefineLibcFunsPluginInfo() -> PassPluginLibraryInfo
{
	return {LLVM_PLUGIN_API_VERSION, "DefineLibcFuns", LLVM_VERSION_STRING,
		[](PassBuilder &PB) {
			PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MPM,
				   ArrayRef<PassBuilder::PipelineElement>) {
					if (Name == "define-libc-funs") {
						MPM.addPass(DefineLibcFunsPass());
						return true;
					}
					return false;
				});
		}};
}
