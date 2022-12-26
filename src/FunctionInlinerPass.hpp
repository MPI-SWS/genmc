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

#ifndef __FUNCTION_INLINER_PASS_HPP__
#define __FUNCTION_INLINER_PASS_HPP__

#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

class FunctionInlinerPass : public llvm::ModulePass {

public:
	FunctionInlinerPass() : llvm::ModulePass(ID) {}

	virtual bool runOnModule(llvm::Module &M);
	void getAnalysisUsage(llvm::AnalysisUsage &au) const;

	static char ID;

private:

};

#endif /* __FUNCTION_INLINER_PASS_HPP__ */
