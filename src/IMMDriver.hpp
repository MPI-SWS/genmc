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
	IMMDriver(std::shared_ptr<const Config> conf, std::unique_ptr<llvm::Module> mod,
		  std::unique_ptr<ModuleInfo> MI);

	void updateLabelViews(EventLabel *lab, const EventDeps *deps) override;
	Event findDataRaceForMemAccess(const MemAccessLabel *mLab) override;
	void changeRf(Event read, Event store) override;
	void updateStart(Event create, Event start) override;
	bool updateJoin(Event join, Event childLast) override;
	void initConsCalculation() override;

private:

	DepView getDepsAsView(const EventDeps *deps);
	View calcBasicHbView(Event e) const;
	DepView calcPPoView(Event e, const EventDeps *deps); /* not const */
	void updateRelView(DepView &pporf, EventLabel *lab);
	void calcFenceRelRfPoBefore(Event last, View &v);
	void updateReadViewsFromRf(DepView &pporf, View &hb, const ReadLabel *lab);

	void calcBasicViews(EventLabel *lab, const EventDeps *deps);
	void calcReadViews(ReadLabel *lab, const EventDeps *deps);
	void calcWriteViews(WriteLabel *lab, const EventDeps *deps);
	void calcWriteMsgView(WriteLabel *lab);
	void calcRMWWriteMsgView(WriteLabel *lab);
	void calcFenceViews(FenceLabel *lab, const EventDeps *deps);
	void calcJoinViews(ThreadJoinLabel *lab, const EventDeps *deps);
	void calcStartViews(ThreadStartLabel *lab);
	void calcLockLAPORViews(LockLabelLAPOR *lab, const EventDeps *deps);
};

#endif /* __IMM_WB_DRIVER_HPP__ */
