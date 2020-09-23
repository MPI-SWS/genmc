/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#ifndef __WORK_SET_HPP__
#define __WORK_SET_HPP__

#include "EventLabel.hpp"
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

class WorkItem;

/*
 * WorkSet class - Represents the set of work items for one particular event
 */
class WorkSet {

protected:
	using WorkSetT = std::vector<std::unique_ptr<WorkItem> >;

public:
	/* Iterators */
	typedef WorkSetT::iterator iterator;
	typedef WorkSetT::const_iterator const_iterator;
	iterator begin() { return wset_.begin(); }
	iterator end()   { return wset_.end(); }
	const_iterator cbegin() const { return wset_.cbegin(); }
	const_iterator cend() const   { return wset_.cend(); }

	/* Returns whether this workset is empty */
	bool empty() const { return wset_.empty(); }

	/* Adds an item to the workset */
	void add(std::unique_ptr<WorkItem> item) {
		wset_.push_back(std::move(item));
		return;
	}

	/* Returns the next item to examine for this workset */
	std::unique_ptr<WorkItem> getNext() {
		auto i = std::move(wset_.back());
		wset_.pop_back();
		return i;
	}

	/* Overloaded operators */
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const WorkSet &wset);

private:
	/* The workset of an event */
	WorkSetT wset_;
};


/*
 * WorkItem class (abstract) - Represents an item belonging to the workset of an event
 */
class WorkItem {

public:
	/* LLVM-style RTTI discriminator */
	enum Kind {
		WI_RevBegin,
		WI_FRev,
		WI_FRevLib,
		WI_FRevLast,
		WI_BRev,
		WI_RevLast,
		WI_MO,
		WI_MOLib,
		WI_MOLast
	};

protected:
	/* Constructors */
	WorkItem() = delete;
	WorkItem(Kind k, Event p) : kind(k), pos(p) {}

public:
	/* Returns the kind of this item */
	Kind getKind() const { return kind; }

	/* Returns the event for which we are exploring an alternative exploration option */
	Event getPos() const { return pos; }

	/* Destructor and printing facilities */
	virtual ~WorkItem() {}
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const WorkItem &item);

private:
	Kind kind;
	Event pos;
};

llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const WorkItem::Kind k);

/*
 * RevItem class (abstract) - Represents the various revisit kinds (forward, backward, etc)
 */
class RevItem : public WorkItem {

protected:
	/* Constructors */
	RevItem() = delete;
	RevItem(Kind k, Event p, Event r) : WorkItem(k, p), rev(r) {}

public:
	/* Returns the event performing the revisit */
	Event getRev() const { return rev; }

	static bool classof(const WorkItem *item) {
		return item->getKind() >= WI_RevBegin && item->getKind() <= WI_RevLast;
	}

private:
	Event rev;
};

/*
 * FRevItem class - Represents a forward revisit
 */
class FRevItem : public RevItem {

protected:
	FRevItem(Kind k, Event p, Event r) : RevItem(k, p, r) {}

public:
	FRevItem(Event p, Event r) : RevItem(WI_FRev, p, r) {}

	static bool classof(const WorkItem *item) {
		return item->getKind() >= WI_FRev && item->getKind() <= WI_FRevLast;
	}
};


/*
 * FRevLibItem class - Represents a forward revisit for libraries
 */
class FRevLibItem : public FRevItem {

public:
	FRevLibItem(Event p, Event r) : FRevItem(WI_FRevLib, p, r) {}

	static bool classof(const WorkItem *item) {
		return item->getKind() == WI_FRevLib;
	}
};


/*
 * BRevItem class - Represents a backward revisit
 */
class BRevItem : public RevItem {

public:
	BRevItem(Event p, Event r,
		 std::vector<std::unique_ptr<EventLabel> > &&prefix,
		 std::vector<std::pair<Event, Event> > &&moPlacings)
		: RevItem(WI_BRev, p, r),
		  prefix(std::move(prefix)),
		  moPlacings(std::move(moPlacings)) {}

	/* Returns (releases) the prefix of the revisiting event */
	std::vector<std::unique_ptr<EventLabel> > &&getPrefixRel() {
		return std::move(prefix);
	}

	/* Returns (but does not release) the prefix of the revisiting event */
	const std::vector<std::unique_ptr<EventLabel> > &getPrefixNoRel() const {
		return prefix;
	}

	/* Returns (releases) the coherence placing in the prefix */
	std::vector<std::pair<Event, Event> > &&getMOPlacingsRel() {
		return std::move(moPlacings);
	}

	static bool classof(const WorkItem *item) {
		return item->getKind() == WI_BRev;
	}

private:
	std::vector<std::unique_ptr<EventLabel> >  prefix;
	std::vector<std::pair<Event, Event> >  moPlacings;
};


/*
 * MOItem class - Represents an alternative MO position for a store
 * (Used by drivers that track MO only)
 */
class MOItem : public WorkItem {

protected:
	MOItem(Kind k, Event p, int moPos) : WorkItem(k, p), moPos(moPos) {}

public:
	MOItem(Event p, int moPos) : WorkItem(WI_MO, p), moPos(moPos) {}

	/* Returns the new MO position of the event for which
	 * we are exploring alternative exploration options */
	int getMOPos() const { return moPos; }

	static bool classof(const WorkItem *item) {
		return item->getKind() >= WI_MO && item->getKind() <= WI_MOLast;
	}

private:
	int moPos;
};


/*
 * MOLibItem class - Represents an alternative MO position for a library store
 * (Used by libraries that track MO only)
 */
class MOLibItem : public MOItem {

public:
	MOLibItem(Event p, int moPos) : MOItem(WI_MOLib, p, moPos) {}

	static bool classof(const WorkItem *item) {
		return item->getKind() == WI_MOLib;
	}
};

#endif /* __WORK_SET_HPP__ */
