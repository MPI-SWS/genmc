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

#ifndef __REVISIT_HPP__
#define __REVISIT_HPP__

#include "EventLabel.hpp"

/*
 * Revisit class (abstract) - Represents a revisit operation
 */
class Revisit {

public:
	/* LLVM-style RTTI discriminator */
	enum Kind {
		RV_ReadBegin,
		RV_FRev,
		RV_FRevLast,
		RV_BRev,
		RV_BRevHelper,
		RV_BRevLast,
		RV_ReadLast,
		RV_MO,
		RV_Opt,
	};

protected:
	/* Constructors */
	Revisit() = delete;
	Revisit(Kind k, Event p) : kind(k), pos(p) {}

public:
	/* Returns the kind of this item */
	Kind getKind() const { return kind; }

	/* Returns the event for which we are exploring an alternative exploration option */
	Event getPos() const { return pos; }

	/* Destructor and printing facilities */
	virtual ~Revisit() {}
	friend llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const Revisit &item);

private:
	Kind kind;
	Event pos;
};

llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs, const Revisit::Kind k);

/*
 * ReadRevisit class (abstract) - Represents various read revisit kinds (forward, backward, etc)
 */
class ReadRevisit : public Revisit {

protected:
	/* Constructors */
	ReadRevisit() = delete;
	ReadRevisit(Kind k, Event p, Event r) : Revisit(k, p), rev(r) {}

public:
	/* Returns the event performing the revisit */
	Event getRev() const { return rev; }

	static bool classof(const Revisit *item) {
		return item->getKind() >= RV_ReadBegin && item->getKind() <= RV_ReadLast;
	}

private:
	Event rev;
};

/*
 * ForwardRevisit class - Represents a forward revisit
 */
class ForwardRevisit : public ReadRevisit {

protected:
	ForwardRevisit(Kind k, Event p, Event r, bool maximal = false)
		: ReadRevisit(k, p, r), maximal(maximal) {}

public:
	ForwardRevisit(Event p, Event r, bool maximal = false) : ForwardRevisit(RV_FRev, p, r, maximal) {}

	bool isMaximal() const { return maximal; }

	static bool classof(const Revisit *item) {
		return item->getKind() >= RV_FRev && item->getKind() <= RV_FRevLast;
	}

private:
	bool maximal;
};


/*
 * BackwardRevisit class - Represents a backward revisit
 */
class BackwardRevisit : public ReadRevisit {

protected:
	BackwardRevisit(Kind k, Event p, Event r,
		 std::vector<std::unique_ptr<EventLabel> > &&prefix,
		 std::vector<std::pair<Event, Event> > &&moPlacings)
		: ReadRevisit(k, p, r),
		  prefix(std::move(prefix)),
		  moPlacings(std::move(moPlacings)) {}

public:
	BackwardRevisit(Event p, Event r,
		 std::vector<std::unique_ptr<EventLabel> > &&prefix,
		 std::vector<std::pair<Event, Event> > &&moPlacings)
		: BackwardRevisit(RV_BRev, p, r, std::move(prefix), std::move(moPlacings)) {}
	BackwardRevisit(Event p, Event r)
		: BackwardRevisit(p, r, {}, {}) {}
	BackwardRevisit(const ReadLabel *rLab, const WriteLabel *wLab)
		: BackwardRevisit(rLab->getPos(), wLab->getPos()) {}

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

	static bool classof(const Revisit *item) {
		return item->getKind() >= RV_BRev && item->getKind() <= RV_BRevLast;
	}

private:
	std::vector<std::unique_ptr<EventLabel> >  prefix;
	std::vector<std::pair<Event, Event> >  moPlacings;
};


/*
 * BackwardRevisitHELPER class - Represents an optimized BR performed by Helper
 */
class BackwardRevisitHELPER : public BackwardRevisit {

public:
	BackwardRevisitHELPER(Event p, Event r, Event m)
		: BackwardRevisit(RV_BRevHelper, p, r, {}, {}), mid(m) {}
	BackwardRevisitHELPER(const ReadLabel *rLab, const WriteLabel *wLab, const WriteLabel *mLab)
		: BackwardRevisitHELPER(rLab->getPos(), wLab->getPos(), mLab->getPos()) {}

	/* Returns the intermediate write participating in the revisit */
	Event getMid() const { return mid; }

	static bool classof(const Revisit *item) {
		return item->getKind() == RV_BRevHelper;
	}

private:
	Event mid;
};

/*
 * WriteRevisit class - Represents an alternative MO position for a store
 * (Used by drivers that track MO only)
 */
class WriteRevisit : public Revisit {

protected:
	WriteRevisit(Kind k, Event p, int moPos) : Revisit(k, p), moPos(moPos) {}

public:
	WriteRevisit(Event p, int moPos) : Revisit(RV_MO, p), moPos(moPos) {}

	/* Returns the new MO position of the event for which
	 * we are exploring alternative exploration options */
	int getMOPos() const { return moPos; }

	static bool classof(const Revisit *item) {
		return item->getKind() == RV_MO;
	}

private:
	int moPos;
};


/*
 * OptionalRevisit class - Represents the revisit of an optional block
 */
class OptionalRevisit : public Revisit {

public:
	OptionalRevisit(Event p) : Revisit(RV_Opt, p) {}

	static bool classof(const Revisit *item) {
		return item->getKind() == RV_Opt;
	}
};

#endif /* __REVISIT_HPP__ */
