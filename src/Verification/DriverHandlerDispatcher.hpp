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

#ifndef GENMC_DRIVER_HANDLER_DISPATCHER_HPP
#define GENMC_DRIVER_HANDLER_DISPATCHER_HPP

#include "ExecutionGraph/EventLabel.hpp"
#include "ExecutionGraph/LabelVisitor.hpp"
#include "Support/Error.hpp"
#include "Verification/GenMCDriver.hpp"

/*******************************************************************************
 **                           DriverHandlerDispatcher Class
 ******************************************************************************/

/**
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
	void visitThreadKillLabel(const ThreadKillLabel &lab)
	{
		driver->handleThreadKill(std::unique_ptr<ThreadKillLabel>(
			static_cast<ThreadKillLabel *>(lab.clone().release())));
	}
	void visitThreadFinishLabel(const ThreadFinishLabel &lab)
	{
		driver->handleThreadFinish(std::unique_ptr<ThreadFinishLabel>(
			static_cast<ThreadFinishLabel *>(lab.clone().release())));
	}

	void visitReadLabel(const ReadLabel &lab)
	{
		driver->handleLoad(
			std::unique_ptr<ReadLabel>(static_cast<ReadLabel *>(lab.clone().release())),
			std::nullopt);
	}

	void visitWriteLabel(const WriteLabel &lab)
	{
		driver->handleStore(std::unique_ptr<WriteLabel>(
					    static_cast<WriteLabel *>(lab.clone().release())),
				    std::nullopt);
	}

	void visitFenceLabel(const FenceLabel &lab)
	{
		driver->handleFence(std::unique_ptr<FenceLabel>(
			static_cast<FenceLabel *>(lab.clone().release())));
	}

	void visitOptionalLabel(const OptionalLabel &lab)
	{
		driver->handleOptional(std::unique_ptr<OptionalLabel>(
			static_cast<OptionalLabel *>(lab.clone().release())));
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
	void visitHpProtectLabel(const HpProtectLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<HpProtectLabel>(
			static_cast<HpProtectLabel *>(lab.clone().release())));
	}
	void visitHelpingCasLabel(const HelpingCasLabel &lab)
	{
		driver->handleHelpingCas(std::unique_ptr<HelpingCasLabel>(
			static_cast<HelpingCasLabel *>(lab.clone().release())));
	}
	void visitLoopBeginLabel(const LoopBeginLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<LoopBeginLabel>(
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

	void visitMethodBeginLabel(const MethodBeginLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<MethodBeginLabel>(
			static_cast<MethodBeginLabel *>(lab.clone().release())));
	}

	void visitMethodEndLabel(const MethodEndLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<MethodEndLabel>(
			static_cast<MethodEndLabel *>(lab.clone().release())));
	}

	/* Start,Init etc should never be handled here */

	void visitEventLabel(const EventLabel &lab) { BUG(); }

protected:
	GenMCDriver *driver;
};

#endif /* GENMC_DRIVER_HANDLER_DISPATCHER_HPP */
