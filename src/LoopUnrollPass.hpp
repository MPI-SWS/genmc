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

#ifndef __LOOP_UNROLL_PASS_HPP__
#define __LOOP_UNROLL_PASS_HPP__

#include "VSet.hpp"
#include "config.h"
#include <llvm/Pass.h>
#ifdef LLVM_PASS_GETPASSNAME_IS_STRINGREF
#include <llvm/ADT/StringRef.h>
#endif
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Transforms/Utils/Cloning.h>

class LoopUnrollPass : public llvm::LoopPass {

public:
	static char ID;

	LoopUnrollPass(unsigned int depth, const VSet<std::string> &noUnrollFuns = {})
		: llvm::LoopPass(ID), unrollDepth(depth), noUnroll(noUnrollFuns){};

	bool shouldUnroll(llvm::Loop *l) const
	{
		return !noUnroll.count((*l->block_begin())->getParent()->getName().str());
	}

	virtual llvm::StringRef getPassName() const { return "LoopUnrollPass"; }
	virtual void getAnalysisUsage(llvm::AnalysisUsage &au) const;
	virtual bool runOnLoop(llvm::Loop *l, llvm::LPPassManager &LPM);

private:
	unsigned int unrollDepth;
	VSet<std::string> noUnroll;
};

#endif /* __LOOP_UNROLL_PASS_HPP__ */
