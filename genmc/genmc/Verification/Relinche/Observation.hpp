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

#ifndef GENMC_OBSERVATION_HPP
#define GENMC_OBSERVATION_HPP

#include "genmc/ADT/IndexedMap.hpp"
#include "genmc/ADT/VSet.hpp"
#include "genmc/Support/Error.hpp"
#include "genmc/Support/Hash.hpp"

#include <format>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

class ExecutionGraph;
class ConsistencyChecker;
class MethodBeginLabel;
class MethodEndLabel;
class EventLabel;
class Specification;

/** Summarization of a method call in the execution graph */
struct MethodCall {
	/** An opaque type for MethodCall ids */
	class Id {
	public:
		explicit Id() = default;
		explicit Id(uint64_t id) : id_(id) {}

		/* This is a hole in the typesystem and should not be abused */
		[[nodiscard]] auto value() const -> uint64_t { return id_; }

		auto operator<=>(const Id &other) const = default;

	private:
		/* Internal convention: the id of a call object is its index in
		 * the parent Observation::ops_ */
		uint64_t id_{};
	};

	/* for diagnostics */
	using ThreadKindAndCopyIx = std::pair<int16_t, int16_t>; // (kind of thread, number of copy)

	Id id; /**< Invariant: the id of a call will always be its index in the parent observation
		*/
	std::string name;
	int32_t argVal;
	int32_t retVal;

	/* Dynamic information (not (de)serializable) */
	MethodBeginLabel *beginLab = nullptr;
	MethodEndLabel *endLab = nullptr;
	ThreadKindAndCopyIx thdKC; /**< thread ID for mpc.c */

	auto operator<=>(const MethodCall &other) const
	{
		return std::tie(name, argVal, retVal) <=>
		       std::tie(other.name, other.argVal, other.retVal);
	}

	auto operator==(const MethodCall &other) const -> bool { return operator<=>(other) == 0; }
};

/** This class represents the projection of execution graph into client.
 * It stores a collection of method calls (ops), along with client enforced synchronization (rfs)
 * and library-induced synchronization (hb).
 * Each call is associated with a unique id: these ids form a contiguous range starting from 0.
 */
class Observation {

public:
	using CallEdge = std::pair<MethodCall::Id, MethodCall::Id>;
	using PermutationMap = genmc::IndexedMap<MethodCall::Id>;

	Observation(ExecutionGraph &graph, const ConsistencyChecker *consChecker);

	[[nodiscard]] auto ops() const { return std::views::all(ops_); }
	[[nodiscard]] auto ops() { return std::views::all(ops_); }

	[[nodiscard]] auto rfs() const { return std::views::all(rfs_); }
	[[nodiscard]] auto rfs() { return std::views::all(rfs_); }

	[[nodiscard]] auto hb() const { return std::views::all(hb_); }
	[[nodiscard]] auto hb() { return std::views::all(hb_); }

	[[nodiscard]] auto getNumOps() const { return ops_.size(); }

	[[nodiscard]] auto ids() const
	{
		return ops() | std::views::transform([](auto &call) { return call.id; });
	}

	[[nodiscard]] auto getCall(MethodCall::Id id) const -> const MethodCall &
	{
		return ops_[id.value()];
	}

	/** Whether THIS is refined by OTHER */
	[[nodiscard]] auto isRefinedBy(const Observation &other) const -> bool
	{
		return *this == other && hb_.subsetOf(other.hb_);
	}

	[[nodiscard]] auto hasSameHb(const Observation &other) const -> bool
	{
		return hb_ == other.hb_;
	}

	[[nodiscard]] auto calculateHbDiff(const Observation &other) const -> VSet<CallEdge>
	{
		return hb_.diff(other.hb_);
	}

	auto applyHbPermutation(const PermutationMap &pMap) -> Observation
	{
		Observation newObs(*this);

		std::vector<Observation::CallEdge> newHb;
		for (const auto &item : hb())
			newHb.emplace_back(pMap[item.first.value()], pMap[item.second.value()]);
		newObs.hb_ = std::move(newHb);
		return newObs;
	}

	// TODO: replace CallIdx with const OpCall *
	[[nodiscard]] auto getContainingCallId(const EventLabel &lab) const
		-> std::optional<MethodCall::Id>;

	auto operator==(const Observation &other) const -> bool
	{
		return ops_ == other.ops_ && rfs_ == other.rfs_;
	}

	auto operator<(const Observation &other) const -> bool
	{
		return std::tie(ops_, rfs_, hb_) < std::tie(other.ops_, other.rfs_, other.hb_);
	}

	friend struct std::formatter<Observation>;

private:
	Observation() = default;

	friend void serialize(std::ostream &os, const Specification &spec);
	friend auto deserialize(std::istream &is) -> Specification;

	std::vector<MethodCall> ops_;	// Ordered operation calls
	VSet<std::pair<int, int>> rfs_; // read-from of the client side
	VSet<CallEdge> hb_;		// synchronization between operation calls
};

template <> struct std::hash<Observation> {
	auto operator()(const Observation &obs) const -> std::size_t
	{
		std::size_t hash = 0;

		for (const auto &call : obs.ops()) {
			hash_combine(hash, call.retVal);
		}
		for (const auto &[from, to] : obs.rfs()) {
			hash_combine(hash, from);
			hash_combine(hash, to);
		}
		return hash;
	}
};

/**** Formatting methods. ****/

/** Make `MethodCall::Id` formattable with `std::format`. */
template <> struct std::formatter<MethodCall::Id> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const MethodCall::Id &id, std::format_context &ctx) const
	{
		return std::format_to(ctx.out(), "{}", id.value());
	}
};

/** Make `MethodCall` formattable with `std::format`. */
template <> struct std::formatter<MethodCall> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const MethodCall &call, std::format_context &ctx) const
	{
		auto argStr = call.argVal == 0 ? std::string{} : std::to_string(call.argVal);
		auto retStr = call.retVal == 0 ? std::string{"⊥"} : std::to_string(call.retVal);
		return std::format_to(ctx.out(), "{}: {}({}) -> {}", call.id, call.name, argStr,
				      retStr);
	}
};

/** Make `Observation` formattable with `std::format`. */
template <> struct std::formatter<Observation> {
	constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

	auto format(const Observation &obs, std::format_context &ctx) const
	{
		auto out = ctx.out();
		out = std::format_to(out, "Ops:\n{}\n", obs.ops_);
		out = std::format_to(out, "RF:\n{}\n", obs.rfs_);
		out = std::format_to(out, "HB:\n{}\n", obs.hb_);
		return out;
	}
};

#endif /* GENMC_OBSERVATION_HPP */
