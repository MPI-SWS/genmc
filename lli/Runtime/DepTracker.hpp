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

#ifndef GENMC_DEP_TRACKER_HPP
#define GENMC_DEP_TRACKER_HPP

#include "genmc/Execution/DepInfo.hpp"
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
	const DepInfo *getDataDeps(unsigned int tid, llvm::Value *i) { return &dataDeps[tid][i]; };

	/* Returns the address dependencies collected so far for tid */
	const DepInfo *getAddrPoDeps(unsigned int tid) { return &addrPoDeps[tid]; };

	/* Returns the control dependencies collected so far for tid */
	const DepInfo *getCtrlDeps(unsigned int tid) { return &ctrlDeps[tid]; };

	/* Updates data dependencies of dst, as it is dependent on src */
	void updateDataDeps(unsigned int tid, llvm::Value *dst, llvm::Value *src)
	{
		dataDeps[tid][dst].update(dataDeps[tid][src]);
	};
	void updateDataDeps(unsigned int tid, llvm::Value *dst, DepInfo e)
	{
		dataDeps[tid][dst].update(e);
	};

	/* Adds the dependencies from src to the address dependencies */
	void updateAddrPoDeps(unsigned int tid, llvm::Value *src)
	{
		addrPoDeps[tid].update(dataDeps[tid][src]);
	};

	/* Adds the dependencies from src to the control dependencies */
	void updateCtrlDeps(unsigned int tid, llvm::Value *src)
	{
		ctrlDeps[tid].update(dataDeps[tid][src]);
	};
	void updateCtrlDeps(unsigned int tid, Event e) { ctrlDeps[tid].update(DepInfo(e)); };

	/* Clears the dependencies calculated for thread TID */
	void clearDeps(unsigned int tid)
	{
		dataDeps[tid].clear();
		addrPoDeps[tid].clear();
		ctrlDeps[tid].clear();
	};

private:
	/* The data dependencies of each instruction are
	 * stored in a map (per thread) */
	std::unordered_map<unsigned int, std::unordered_map<llvm::Value *, DepInfo>> dataDeps;

	/* Since {addr, ctrl} are forwards-closed under po, we just
	 * keep a DepInfo item for these */
	std::unordered_map<unsigned int, DepInfo> addrPoDeps;
	std::unordered_map<unsigned int, DepInfo> ctrlDeps;
};

struct DepTrackerCloner {
	DepTracker *operator()(const DepTracker &x) const { return new DepTracker(x); }
	// DepTracker *operator()(DepTracker &&x) const { return new DepTracker(std::move(x)); }
};

#endif /* GENMC_DEP_TRACKER_HPP */
