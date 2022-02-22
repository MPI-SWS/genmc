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

#ifndef __PROPAGATE_ASSUMES_PASS_HPP__
#define __PROPAGATE_ASSUMES_PASS_HPP__

#include <llvm/Pass.h>

class PropagateAssumesPass : public llvm::FunctionPass {

public:
	static char ID;

	PropagateAssumesPass() : llvm::FunctionPass(ID) {}

	bool runOnFunction(llvm::Function &F) override;

	void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

#endif /* __PROPAGATE_ASSUMES_PASS_HPP__ */
