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

#ifndef __DEP_TRACKER_HPP__
#define __DEP_TRACKER_HPP__

#include "DepInfo.hpp"
#include <llvm/IR/Instruction.h>

#include <unordered_map>

/*******************************************************************************
 **                            DepTracker Class
 ******************************************************************************/

/*
 * A class to be used from the Interpreter to track dependencies between
 * instructions.
 */
class DepTracker {

public:
	/* Returns data dependencies for instruction i in thread tid */
	const DepInfo *getDataDeps(unsigned int tid, llvm::Value *i) {
		return &dataDeps[tid][i];
	};

	/* Returns the address dependencies collected so far for tid */
	const DepInfo *getAddrPoDeps(unsigned int tid) {
		return &addrPoDeps[tid];
	};

	/* Returns the control dependencies collected so far for tid */
	const DepInfo *getCtrlDeps(unsigned int tid) {
		return &ctrlDeps[tid];
	};

	/* Updates data dependencies of dst, as it is dependent on src */
	void updateDataDeps(unsigned int tid, llvm::Value *dst, llvm::Value *src) {
		dataDeps[tid][dst].update(dataDeps[tid][src]);
	};
	void updateDataDeps(unsigned int tid, llvm::Value *dst, DepInfo e) {
		dataDeps[tid][dst].update(e);
	};

	/* Adds the dependencies from src to the address dependencies */
	void updateAddrPoDeps(unsigned int tid, llvm::Value *src) {
		addrPoDeps[tid].update(dataDeps[tid][src]);
	};

	/* Adds the dependencies from src to the control dependencies */
	void updateCtrlDeps(unsigned int tid, llvm::Value *src) {
		ctrlDeps[tid].update(dataDeps[tid][src]);
	};

	/* Clears the dependencies calculated for thread TID */
	void clearDeps(unsigned int tid) {
		dataDeps[tid].clear();
		addrPoDeps[tid].clear();
		ctrlDeps[tid].clear();
	};

private:
	/* The data dependencies of each instruction are
	 * stored in a map (per thread) */
	std::unordered_map<unsigned int,
			   std::unordered_map<llvm::Value *, DepInfo>> dataDeps;

	/* Since {addr, ctrl} are forwards-closed under po, we just
	 * keep a DepInfo item for these */
	std::unordered_map<unsigned int, DepInfo> addrPoDeps;
	std::unordered_map<unsigned int, DepInfo> ctrlDeps;
};

#endif /* __DEP_TRACKER_HPP__ */
