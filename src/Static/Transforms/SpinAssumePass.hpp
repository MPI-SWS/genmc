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

#ifndef GENMC_SPIN_ASSUME_PASS_HPP
#define GENMC_SPIN_ASSUME_PASS_HPP

#include "ADT/VSet.hpp"

#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

/** Convert spinloops to assume statements */
class SpinAssumePass : public PassInfoMixin<SpinAssumePass> {
public:
	SpinAssumePass(bool markStarts = false) : markStarts_(markStarts) {}

	auto run(Module &M, ModuleAnalysisManager &MAM) -> PreservedAnalyses;

private:
	/** Whether we should mark spinloop starts */
	bool markStarts_{};
};

#endif /* GENMC_SPIN_ASSUME_PASS_HPP */
