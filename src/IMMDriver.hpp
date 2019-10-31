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

#ifndef __IMM_MO_DRIVER_HPP__
#define __IMM_MO_DRIVER_HPP__

#include "GenMCDriver.hpp"

class IMMDriver : public GenMCDriver {

public:

	IMMDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod,
		  std::vector<Library> &granted, std::vector<Library> &toVerify,
		  clock_t start)
		: GenMCDriver(std::move(conf), std::move(mod), granted, toVerify, start) {};

	/* Creates a label for a plain read to be added to the graph */
	std::unique_ptr<ReadLabel>
	createReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			const llvm::GenericValue *ptr, const llvm::Type *typ,
			Event rf) override;

	/* Creates a label for a FAI read to be added to the graph */
	std::unique_ptr<FaiReadLabel>
	createFaiReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *ptr, const llvm::Type *typ,
			   Event rf, llvm::AtomicRMWInst::BinOp op,
			   llvm::GenericValue &&opValue) override;

	/* Creates a label for a CAS read to be added to the graph */
	std::unique_ptr<CasReadLabel>
	createCasReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *ptr, const llvm::Type *typ,
			   Event rf, const llvm::GenericValue &expected,
			   const llvm::GenericValue &swap,
			   bool isLock = false) override;

	/* Creates a label for a library read to be added to the graph */
	std::unique_ptr<LibReadLabel>
	createLibReadLabel(int tid, int index, llvm::AtomicOrdering ord,
			   const llvm::GenericValue *ptr, const llvm::Type *typ,
			   Event rf, std::string functionName)  override;

	/* Creates a label for a plain write to be added to the graph */
	std::unique_ptr<WriteLabel>
	createStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			 const llvm::GenericValue *ptr, const llvm::Type *typ,
			 const llvm::GenericValue &val,
			 bool isUnlock = false) override;

	/* Creates a label for a FAI write to be added to the graph */
	std::unique_ptr<FaiWriteLabel>
	createFaiStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val) override;

	/* Creates a label for a CAS write to be added to the graph */
	std::unique_ptr<CasWriteLabel>
	createCasStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    const llvm::GenericValue &val,
			    bool isLock = false) override;

	/* Creates a label for a library write to be added to the graph */
	std::unique_ptr<LibWriteLabel>
	createLibStoreLabel(int tid, int index, llvm::AtomicOrdering ord,
			    const llvm::GenericValue *ptr, const llvm::Type *typ,
			    llvm::GenericValue &val, std::string functionName,
			    bool isInit) override;

	/* Creates a label for a fence to be added to the graph */
	std::unique_ptr<FenceLabel>
	createFenceLabel(int tid, int index, llvm::AtomicOrdering ord) override;


	/* Creates a label for a malloc event to be added to the graph */
	std::unique_ptr<MallocLabel>
	createMallocLabel(int tid, int index, const void *addr,
			  unsigned int size, bool isLocal = false) override;

	/* Creates a label for a free event to be added to the graph */
	std::unique_ptr<FreeLabel>
	createFreeLabel(int tid, int index, const void *addr) override;

	/* Creates a label for the creation of a thread to be added to the graph */
	std::unique_ptr<ThreadCreateLabel>
	createTCreateLabel(int tid, int index, int cid) override;

	/* Creates a label for the join of a thread to be added to the graph */
	std::unique_ptr<ThreadJoinLabel>
	createTJoinLabel(int tid, int index, int cid) override;

	/* Creates a label for the start of a thread to be added to the graph */
	std::unique_ptr<ThreadStartLabel>
	createStartLabel(int tid, int index, Event tc) override;

	/* Creates a label for the end of a thread to be added to the graph */
	std::unique_ptr<ThreadFinishLabel>
	createFinishLabel(int tid, int index) override;

	/* Since there is no concept of race in IMM, always returns INIT */
	Event findDataRaceForMemAccess(const MemAccessLabel *mLab) override;

	std::vector<Event> getStoresToLoc(const llvm::GenericValue *addr) override;

	std::vector<Event> getRevisitLoads(const WriteLabel *lab) override;

	void changeRf(Event read, Event store) override;

	bool updateJoin(Event join, Event childLast) override;

	bool isExecutionValid() override;

private:

	View calcBasicHbView(Event e) const;
	View calcBasicPorfView(Event e) const;
	DepView calcPPoView(Event e); /* not const */
	void calcBasicReadViews(ReadLabel *lab);
	void calcBasicWriteViews(WriteLabel *lab);
	void calcWriteMsgView(WriteLabel *lab);
	void calcRMWWriteMsgView(WriteLabel *lab);
	void calcBasicFenceViews(FenceLabel *lab);
	void calcFenceRelRfPoBefore(Event last, View &v);

	std::vector<Event> collectAllEvents();
	void fillMatrixFromView(const Event e, const DepView &v,
				Matrix2D<Event> &matrix);
	Matrix2D<Event> getARMatrix();
};

#endif /* __IMM_WB_DRIVER_HPP__ */
