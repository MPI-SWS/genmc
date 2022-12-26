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

#ifndef __PASSES_HPP__
#define __PASSES_HPP__

#include "config.h"
#include "VSet.hpp"
#include <llvm/Pass.h>
#include <string>

template<typename K, typename V>
struct AnnotationInfo;
struct PassModuleInfo;
struct ModuleInfo;

/*
 * Finds all bisimilarity pairs in a function.
 */
llvm::FunctionPass *createBisimilarityCheckerPass();

/*
 * Collects function call information (e.g., whether a function has side effects)
 * in a given module.
 */
llvm::ModulePass *createCallInfoCollectionPass();

/*
 * Condenses the code by merging bisimilar points.
 */
llvm::FunctionPass *createCodeCondenserPass();

/*
 * Declares GenMC functions.
 */
llvm::ModulePass *createDeclareInternalsPass();

/*
 * Defines some commonly used libc functions (currently replaces then with no-ops).
 */
llvm::ModulePass *createDefineLibcFunsPass();

/*
 * Lowers intrinsic calls.
 */
llvm::ModulePass *createIntrinsicLoweringPass(llvm::Module &M);

/*
 * Collects annotation information for a function's load instructions.
 */
llvm::FunctionPass *createLoadAnnotationPass(AnnotationInfo<llvm::Instruction *, llvm::Value *> &AI);

/*
 * Unrolls a loop N times, unless the loop is in a function present in NOUNROLLFUNS
 */
llvm::Pass *createLoopUnrollPass(int n, const VSet<std::string> &noUnrollFuns = {});

/*
 * Performs jump-threading for some simple classes of loops.
 */
llvm::Pass *createLoopJumpThreadingPass();

/*
 * Collects naming information about a module's variables and fs calls.
 * Makes sure the module information is up-to-date before doing so.
 */
llvm::ModulePass *createMDataCollectionPass(PassModuleInfo *PI);

/*
 * Promotes memcpy() and memset() calls to a series of loads and stores.
 */
llvm::ModulePass *createPromoteMemIntrinsicPass();

/*
 * Transforms certain classes of spinloops to assume() statements.
 */
llvm::Pass *createSpinAssumePass(bool markStarts = false);

/*
 * Eliminates certain form of PHIs stemming from CASes
 */
llvm::Pass *createEliminateCASPHIsPass();

/*
 * Eliminates certain form of casts
 */
llvm::Pass *createEliminateCastsPass();

/*
 * Eliminates GenMC's internal annotations
 */
llvm::Pass *createEliminateAnnotationsPass();

/*
 * Eliminates some unused code that does not affect the verification result
 */
llvm::Pass *createEliminateUnusedCodePass();

/*
 * Collects possible escape points for a given function
 */
llvm::Pass *createEscapeCheckerPass(bool loadsEscape = true);

/*
 * Annotates confirming reads
 */
llvm::FunctionPass *createConfirmationAnnotationPass();

/*
 * Propagates assume(0)s upwards.
 */
llvm::Pass *createPropagateAssumesPass();

/*
 * Performs some minor simplifications to the CFG
 */
llvm::Pass *createLocalSimplifyCFGPass();

/*
 * Eliminates redundant instructions
 */
llvm::Pass *createEliminateRedundantInstPass();

/*
 * Inlines all functions of a module
 */
llvm::ModulePass *createFunctionInlinerPass();

#endif /* __PASSES_HPP__ */
