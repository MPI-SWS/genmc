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

#ifndef GENMC_VECTOR_CLOCK_HPP
#define GENMC_VECTOR_CLOCK_HPP

#include "genmc/Execution/Event.hpp"

#include <format>
#include <memory>

/*******************************************************************************
 **                        VectorClock Class (Abstract)
 ******************************************************************************/

class View;
class DepView;
class EventLabel;

/**
 * An abstract class for modeling vector clocks. Contains the bare
 * minimum that all different types of vector clocks (e.g., plain ones
 * or dependency tracking ones) should have. Vector clocks are supposed
 * to work with events.
 */
class VectorClock {

public:
	VectorClock(const VectorClock &) = default;
	VectorClock(VectorClock &&) = default;
	auto operator=(const VectorClock &) -> VectorClock & = default;
	auto operator=(VectorClock &&) -> VectorClock & = default;

	/** Discriminator for LLVM-style RTTI (dyn_cast<> et al).
	 * It is public to allow clients perform a switch() on it */
	enum VectorClockKind : std::uint8_t { // NOLINT(cppcoreguidelines-use-enum-class)
		VC_View,
		VC_DepView,
	};

protected:
	VectorClock(VectorClockKind kind) : kind(kind) {}

public:
	virtual ~VectorClock() = default;

	/** Returns the kind of this vector clock */
	[[nodiscard]] auto getKind() const -> VectorClockKind { return kind; }

	/** Returns the size of this vector clock */
	[[nodiscard]] virtual auto size() const -> int = 0;

	/** Returns true if this vector clock is empty */
	[[nodiscard]] auto empty() const -> bool;

	virtual void clear() = 0;

	/** Returns true if this clock contains e */
	[[nodiscard]] virtual auto contains(Event e) const -> bool = 0;
	auto contains(const EventLabel *lab) const -> bool;

	/** Updates the clock based on another clock **of the same kind** */
	virtual auto update(const View &v) -> View & = 0;
	virtual auto update(const DepView &v) -> DepView & = 0;
	virtual auto update(const VectorClock &v) -> VectorClock & = 0;

	/** Ensures event E is included in the clock */
	virtual auto updateIdx(Event e) -> VectorClock & = 0;

	[[nodiscard]] virtual auto getMax(int thread) const -> int = 0;
	[[nodiscard]] auto getMax(Event e) const -> int;

	virtual void setMax(Event e) = 0;

	/** Clones a VectorClock */
	[[nodiscard]] auto clone() const -> std::unique_ptr<VectorClock>;

	/** Formatting facilities */
	virtual auto formatData(std::format_context &ctx) const
		-> std::format_context::iterator = 0;

private:
	/** The kind of this VectorClock */
	VectorClockKind kind;
};

/** Helper cloner class */
struct VectorClockCloner {
	auto operator()(const VectorClock &clock) const -> VectorClock *
	{
		return clock.clone().release();
	}
	// VectorClock *operator()(VectorClock &&x) const { return new VectorClock(std::move(x)); }
};

/** Make `VectorClock` formattable with `std::format`. */
template <> struct std::formatter<VectorClock> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const VectorClock &vc, std::format_context &ctx) const
	{
		return vc.formatData(ctx);
	}
};

#endif /* GENMC_VECTOR_CLOCK_HPP */
