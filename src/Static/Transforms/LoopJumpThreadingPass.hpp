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

#ifndef GENMC_LOOP_THREAD_JUMPING_PASS_HPP
#define GENMC_LOOP_THREAD_JUMPING_PASS_HPP

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

class LoopJumpThreadingPass : public PassInfoMixin<LoopJumpThreadingPass> {
public:
	auto run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR, LPMUpdater &U)
		-> PreservedAnalyses;
};

#endif /* GENMC_LOOP_THREAD_JUMPING_PASS_HPP */
