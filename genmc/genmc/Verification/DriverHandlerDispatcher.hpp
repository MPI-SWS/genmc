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

#include "genmc/Execution/EventLabel.hpp"
#include "genmc/Execution/LabelVisitor.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Verification/GenMCDriver.hpp"

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
			genmc::cast<BlockLabel>(lab.clone().release())));
	}
	void visitThreadKillLabel(const ThreadKillLabel &lab)
	{
		driver->handleThreadKill(std::unique_ptr<ThreadKillLabel>(
			genmc::cast<ThreadKillLabel>(lab.clone().release())));
	}
	void visitThreadFinishLabel(const ThreadFinishLabel &lab)
	{
		driver->handleThreadFinish(std::unique_ptr<ThreadFinishLabel>(
			genmc::cast<ThreadFinishLabel>(lab.clone().release())));
	}

	void visitReadLabel(const ReadLabel &lab)
	{
		if (lab.isNotAtomic()) {
			driver->handleNALoad(lab.getPos(), lab.getAddr(), lab.getSize());
		} else {
			driver->handleLoad(std::unique_ptr<ReadLabel>(
						   genmc::cast<ReadLabel>(lab.clone().release())),
					   std::nullopt);
		}
	}

	void visitWriteLabel(const WriteLabel &lab)
	{
		if (lab.isNotAtomic()) {
			driver->handleNAStore(lab.getPos(), lab.getAddr(), lab.getSize(),
					      lab.getVal());
		} else {
			driver->handleStore(std::unique_ptr<WriteLabel>(
						    genmc::cast<WriteLabel>(lab.clone().release())),
					    std::nullopt);
		}
	}

	void visitFenceLabel(const FenceLabel &lab)
	{
		driver->handleFence(std::unique_ptr<FenceLabel>(
			genmc::cast<FenceLabel>(lab.clone().release())));
	}

	void visitOptionalLabel(const OptionalLabel &lab)
	{
		driver->handleOptional(std::unique_ptr<OptionalLabel>(
			genmc::cast<OptionalLabel>(lab.clone().release())));
	}

	void visitMallocLabel(const MallocLabel &lab)
	{
		driver->handleMalloc(lab.getPos(), lab.getSize(), lab.getAlignment(),
				     lab.getStorageDuration(), lab.getStorageType(),
				     lab.getAddressSpace(), lab.getNameInfo(), lab.getName(),
				     lab.getDeps());
	}

	void visitFreeLabel(const FreeLabel &lab)
	{
		driver->handleFree(lab.getPos(), lab.getAddr(), lab.getDeps());
	}
	void visitHpRetireLabel(const HpRetireLabel &lab)
	{
		driver->handleRetire(lab.getPos(), lab.getAddr(), lab.getDeps());
	}

	void visitThreadCreateLabel(const ThreadCreateLabel &lab)
	{
		driver->handleThreadCreate(std::unique_ptr<ThreadCreateLabel>(
			genmc::cast<ThreadCreateLabel>(lab.clone().release())));
	}
	void visitThreadJoinLabel(const ThreadJoinLabel &lab)
	{
		driver->handleThreadJoin(std::unique_ptr<ThreadJoinLabel>(
			genmc::cast<ThreadJoinLabel>(lab.clone().release())));
	}
	void visitHpProtectLabel(const HpProtectLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<HpProtectLabel>(
			genmc::cast<HpProtectLabel>(lab.clone().release())));
	}
	void visitHelpingCasLabel(const HelpingCasLabel &lab)
	{
		driver->handleHelpingCas(std::unique_ptr<HelpingCasLabel>(
			genmc::cast<HelpingCasLabel>(lab.clone().release())));
	}
	void visitLoopBeginLabel(const LoopBeginLabel &lab)
	{
		driver->handleLoopBegin(std::unique_ptr<LoopBeginLabel>(
			genmc::cast<LoopBeginLabel>(lab.clone().release())));
	}
	void visitSpinStartLabel(const SpinStartLabel &lab)
	{
		driver->handleSpinStart(std::unique_ptr<SpinStartLabel>(
			genmc::cast<SpinStartLabel>(lab.clone().release())));
	}
	void visitFaiZNESpinEndLabel(const FaiZNESpinEndLabel &lab)
	{
		driver->handleFaiZNESpinEnd(std::unique_ptr<FaiZNESpinEndLabel>(
			genmc::cast<FaiZNESpinEndLabel>(lab.clone().release())));
	}
	void visitLockZNESpinEndLabel(const LockZNESpinEndLabel &lab)
	{
		driver->handleLockZNESpinEnd(std::unique_ptr<LockZNESpinEndLabel>(
			genmc::cast<LockZNESpinEndLabel>(lab.clone().release())));
	}

	void visitMethodBeginLabel(const MethodBeginLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<MethodBeginLabel>(
			genmc::cast<MethodBeginLabel>(lab.clone().release())));
	}

	void visitMethodEndLabel(const MethodEndLabel &lab)
	{
		driver->handleDummy(std::unique_ptr<MethodEndLabel>(
			genmc::cast<MethodEndLabel>(lab.clone().release())));
	}

	/* Start,Init etc should never be handled here */

	void visitEventLabel(const EventLabel & /*lab*/) { UNREACHABLE(); }

protected:
	// NOLINTNEXTLINE(cppcoreguidelines-non-private-member-variables-in-classes)
	GenMCDriver *driver;
};

#endif /* GENMC_DRIVER_HANDLER_DISPATCHER_HPP */
