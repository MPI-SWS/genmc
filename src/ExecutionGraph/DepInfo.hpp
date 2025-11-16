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

#ifndef GENMC_DEP_INFO_HPP
#define GENMC_DEP_INFO_HPP

#include "ADT/VSet.hpp"
#include "Event.hpp"

#include <format>

/*******************************************************************************
 **                             DepInfo Class
 ******************************************************************************/

/**
 * A class to model the dependencies (of some kind) of an event. Each DepInfo
 * objects holds a collection of events on which some events depend on. In
 * principle, such an object should be used for each dependency kind of
 * a particular event.
 */
class DepInfo {

protected:
	using Set = VSet<Event>;

public:
	/** Constructors */
	DepInfo() = default;
	DepInfo(Event e) : set_({e}) {}

	/** Updates this object based on the dependencies of dep (union) */
	void update(const DepInfo &dep) { set_.insert(dep.set_); }

	/** Clears all the stored dependencies */
	void clear() { set_.clear(); }

	/** Returns true if e is contained in the dependencies */
	[[nodiscard]] auto contains(Event e) const -> bool { return set_.count(e); }

	/** Returns true if there are no dependencies */
	[[nodiscard]] auto empty() const -> bool { return set_.empty(); }

	/** Iterators */
	using const_iterator = typename Set::const_iterator;

	[[nodiscard]] auto begin() const -> const_iterator { return set_.begin(); };
	[[nodiscard]] auto end() const -> const_iterator { return set_.end(); };

	friend struct std::formatter<DepInfo>;

private:
	/** The actual container for the dependencies */
	Set set_;
};

/** Make `DebInfo` formattable with `std::format`. */
template <> struct std::formatter<DepInfo> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const DepInfo &dep, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}", dep.set_);
	}
};

/*******************************************************************************
 **                             EventDeps Class
 ******************************************************************************/

/**
 * Packs together all the dependencies of a given event.
 *
 * Dependencies have one of the following types:
 *     addr, data, ctrl, addr;po, cas.
 * Models are free to ignore some of these if they are of no use.
 */
struct EventDeps {

	DepInfo addr;
	DepInfo data;
	DepInfo ctrl;
	DepInfo addrPo;
	DepInfo cas;
};

#endif /* GENMC_DEP_INFO_HPP */
