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

#ifndef __DRIVER_HANDLER_DISPATCHER_HPP__
#define __DRIVER_HANDLER_DISPATCHER_HPP__

#include "Error.hpp"
#include "EventLabel.hpp"
#include "GenMCDriver.hpp"
#include "LabelVisitor.hpp"

/*******************************************************************************
 **                           DriverHandlerDispatcher Class
 ******************************************************************************/

/*
 * Calls the appropriate Driver handler for a particular label kind.
 */
class DriverHandlerDispatcher : public LabelVisitor<DriverHandlerDispatcher> {

public:
	DriverHandlerDispatcher(GenMCDriver *driver) : driver(driver) {}

	void visitBlockLabel(const BlockLabel &lab)
	{
		driver->handleBlock(std::unique_ptr<BlockLabel>(
			static_cast<BlockLabel *>(lab.clone().release())));
	}

	void visitOptionalLabel(const OptionalLabel &lab)
	{
		driver->handleOptional(std::unique_ptr<OptionalLabel>(
			static_cast<OptionalLabel *>(lab.clone().release())));
	}

	void visitReadLabel(const ReadLabel &lab)
	{
		driver->handleLoad(std::unique_ptr<ReadLabel>(
			static_cast<ReadLabel *>(lab.clone().release())));
	}

	void visitDskReadLabel(const DskReadLabel &lab)
	{
		driver->handleDskRead(std::unique_ptr<DskReadLabel>(
			static_cast<DskReadLabel *>(lab.clone().release())));
	}

	void visitWriteLabel(const WriteLabel &lab)
	{
		driver->handleStore(std::unique_ptr<WriteLabel>(
			static_cast<WriteLabel *>(lab.clone().release())));
	}

	void visitDskWriteLabel(const DskWriteLabel &lab)
	{
		driver->handleDskWrite(std::unique_ptr<DskWriteLabel>(
			static_cast<DskWriteLabel *>(lab.clone().release())));
	}

	void visitFenceLabel(const FenceLabel &lab)
	{
		driver->handleFence(std::unique_ptr<FenceLabel>(
			static_cast<FenceLabel *>(lab.clone().release())));
	}

	void visitCLFlushLabel(const CLFlushLabel &lab)
	{
		driver->handleCLFlush(std::unique_ptr<CLFlushLabel>(
			static_cast<CLFlushLabel *>(lab.clone().release())));
	}

	void visitDskFsyncLabel(const DskFsyncLabel &lab)
	{
		driver->handleDskFsync(std::unique_ptr<DskFsyncLabel>(
			static_cast<DskFsyncLabel *>(lab.clone().release())));
	}

	void visitDskSyncLabel(const DskSyncLabel &lab)
	{
		driver->handleDskSync(std::unique_ptr<DskSyncLabel>(
			static_cast<DskSyncLabel *>(lab.clone().release())));
	}

	void visitDskPbarrierLabel(const DskPbarrierLabel &lab)
	{
		driver->handleDskPbarrier(std::unique_ptr<DskPbarrierLabel>(
			static_cast<DskPbarrierLabel *>(lab.clone().release())));
	}

	void visitThreadCreateLabel(const ThreadCreateLabel &lab)
	{
		driver->handleThreadCreate(std::unique_ptr<ThreadCreateLabel>(
			static_cast<ThreadCreateLabel *>(lab.clone().release())));
	}

	void visitThreadJoinLabel(const ThreadJoinLabel &lab)
	{
		driver->handleThreadJoin(std::unique_ptr<ThreadJoinLabel>(
			static_cast<ThreadJoinLabel *>(lab.clone().release())));
	}

	void visitThreadFinishLabel(const ThreadFinishLabel &lab)
	{
		driver->handleThreadFinish(std::unique_ptr<ThreadFinishLabel>(
			static_cast<ThreadFinishLabel *>(lab.clone().release())));
	}

	void visitMallocLabel(const MallocLabel &lab)
	{
		driver->handleMalloc(std::unique_ptr<MallocLabel>(
			static_cast<MallocLabel *>(lab.clone().release())));
	}

	void visitFreeLabel(const FreeLabel &lab)
	{
		driver->handleFree(std::unique_ptr<FreeLabel>(
			static_cast<FreeLabel *>(lab.clone().release())));
	}

	void visitHpProtectLabel(const HpProtectLabel &lab)
	{
		driver->handleHpProtect(std::unique_ptr<HpProtectLabel>(
			static_cast<HpProtectLabel *>(lab.clone().release())));
	}

	void visitLoopBeginLabel(const LoopBeginLabel &lab)
	{
		driver->handleLoopBegin(std::unique_ptr<LoopBeginLabel>(
			static_cast<LoopBeginLabel *>(lab.clone().release())));
	}

	void visitSpinStartLabel(const SpinStartLabel &lab)
	{
		driver->handleSpinStart(std::unique_ptr<SpinStartLabel>(
			static_cast<SpinStartLabel *>(lab.clone().release())));
	}

	void visitFaiZNESpinEndLabel(const FaiZNESpinEndLabel &lab)
	{
		driver->handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel>(
			static_cast<FaiZNESpinEndLabel *>(lab.clone().release())));
	}

	void visitLockZNESpinEndLabel(const LockZNESpinEndLabel &lab)
	{
		driver->handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel>(
			static_cast<LockZNESpinEndLabel *>(lab.clone().release())));
	}

	void visitThreadKillLabel(const ThreadKillLabel &lab)
	{
		driver->handleThreadKill(std::unique_ptr<ThreadKillLabel>(
			static_cast<ThreadKillLabel *>(lab.clone().release())));
	}

	void visitHelpingCasLabel(const HelpingCasLabel &lab)
	{
		driver->handleHelpingCas(std::unique_ptr<HelpingCasLabel>(
			static_cast<HelpingCasLabel *>(lab.clone().release())));
	}

	void visitDskOpenLabel(const DskOpenLabel &lab)
	{
		driver->handleDskOpen(std::unique_ptr<DskOpenLabel>(
			static_cast<DskOpenLabel *>(lab.clone().release())));
	}

	/* Generic handlers */

	void visitEventLabel(const EventLabel &lab) { BUG(); }

protected:
	GenMCDriver *driver;
};

#endif /* __DRIVER_HANDLER_DISPATCHER_HPP__ */
