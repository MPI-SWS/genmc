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

#ifndef __LKMM_DRIVER_HPP__
#define __LKMM_DRIVER_HPP__

#include "GenMCDriver.hpp"

class LKMMDriver : public GenMCDriver {

public:

	LKMMDriver(std::unique_ptr<Config> conf, std::unique_ptr<llvm::Module> mod, clock_t start);

	void updateLabelViews(EventLabel *lab) override;
	Event findDataRaceForMemAccess(const MemAccessLabel *mLab) override;
	std::vector<Event> getStoresToLoc(const llvm::GenericValue *addr) override;
	std::vector<Event> getRevisitLoads(const WriteLabel *lab) override;
	void changeRf(Event read, Event store) override;
	void updateStart(Event create, Event start) override;
	bool updateJoin(Event join, Event childLast) override;
	void initConsCalculation() override;

private:

	View calcBasicHbView(Event e) const;
	DepView calcFenceView(const MemAccessLabel *lab) const;
	DepView calcPPoView(EventLabel *lab); /* not const */
	void updateRelView(DepView &pporf, const EventLabel *lab);
	void updateReadViewsFromRf(DepView &pporf, View &hb, ReadLabel *rLab);

	void calcBasicViews(EventLabel *lab);
	void calcReadViews(ReadLabel *lab);
	void calcWriteViews(WriteLabel *lab);
	void calcWriteMsgView(WriteLabel *lab);
	void calcRMWWriteMsgView(WriteLabel *lab);
	void updateRmbFenceView(DepView &pporf, SmpFenceLabelLKMM *lab);
	void updateWmbFenceView(DepView &pporf, SmpFenceLabelLKMM *lab);
	void updateMbFenceView(DepView &pporf, SmpFenceLabelLKMM *fLab);
	void calcFenceRelRfPoBefore(Event last, View &v);
	void calcFenceViews(FenceLabel *lab);
	void calcJoinViews(ThreadJoinLabel *lab);
	void calcStartViews(ThreadStartLabel *lab);

	bool areInPotentialRace(const MemAccessLabel *labA, const MemAccessLabel *labB);
	std::vector<Event> findPotentialRacesForNewLoad(const ReadLabel *rLab);
	std::vector<Event> findPotentialRacesForNewStore(const WriteLabel *wLab);

	bool isAcqPoOrPoRelBefore(Event a, Event b);
	bool isFenceBefore(Event a, Event b);

	std::vector<Event> getOverwrites(const MemAccessLabel *lab);
	std::vector<Event> getMarkedWritePreds(const EventLabel *lab);
	std::vector<Event> getMarkedWriteSuccs(const EventLabel *lab);
	std::vector<Event> getMarkedReadPreds(const EventLabel *lab);
	std::vector<Event> getMarkedReadSuccs(const EventLabel *lab);
	std::vector<Event> getStrongFenceSuccs(Event e);

	bool isVisConnected(Event a, Event b);
	bool isVisBefore(Event a, Event b);

	bool isWWVisBefore(const MemAccessLabel *labA, const MemAccessLabel *labB);
	bool isWRVisBefore(const MemAccessLabel *labA, const MemAccessLabel *labB);
	bool isRWXbBefore(const MemAccessLabel *labA, const MemAccessLabel *labB);
	bool isWRXbBefore(const MemAccessLabel *labA, const MemAccessLabel *labB);

	bool isValidWWRace(const WriteLabel *labA, const WriteLabel *labB);
	bool isValidWRRace(const WriteLabel *labA, const ReadLabel *labB);
	bool isValidRWRace(const ReadLabel *labA, const WriteLabel *labB);
	bool isValidRace(Event a, Event b);
	bool isRaceIncoherent(Event a, Event b);
};

#endif /* __LKMM_DRIVER_HPP__ */
