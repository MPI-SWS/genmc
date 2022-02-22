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

#ifndef __LOAD_ANNOTATION_PASS_HPP__
#define __LOAD_ANNOTATION_PASS_HPP__

#include "ModuleInfo.hpp"
#include <llvm/Pass.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;

class LoadAnnotationPass : public FunctionPass {

public:
	static char ID;
	AnnotationInfo<Instruction *, Value *> &LAI;

	LoadAnnotationPass(AnnotationInfo<Instruction *, Value *> &LAI)
		: FunctionPass(ID), LAI(LAI) {};

	virtual void getAnalysisUsage(AnalysisUsage &AU) const;
	virtual bool runOnFunction(Function &F);

private:
	/*
	 * Returns the source loads of an assume statement, that is,
	 * loads the result of which is used in the assume.
	 */
	std::vector<Instruction *> getSourceLoads(CallInst *assm) const;

	/*
	 * Given an assume's source loads, returns the annotatable ones.
	 */
	std::vector<Instruction *>
	filterAnnotatableFromSource(CallInst *assm, const std::vector<Instruction *> &source) const;

	/*
	 * Returns all of ASSM's annotatable loads
	 */
	std::vector<Instruction *>
	getAnnotatableLoads(CallInst *assm) const;
};

#endif /* __LOAD_ANNOTATION_PASS_HPP__ */
