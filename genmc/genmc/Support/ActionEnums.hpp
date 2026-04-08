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

#ifndef GENMC_ACTION_ENUMS_HPP
#define GENMC_ACTION_ENUMS_HPP

#include "genmc/Execution/Event.hpp"

#include <cstdint>

enum class ActionKind : std::uint8_t { Load, NonLoad };

struct Action {
	Action(ActionKind kind, Event event) : kind(kind), event(event) {}
	Action(Action &&) = default;
	Action(const Action &other) = default;

	auto operator=(const Action &) -> Action & = default;
	auto operator=(Action &&) -> Action & = default;

	~Action() = default;

	ActionKind kind;
	Event event;
};

/* Types of allocations in the interpreter */
enum class AddressSpace : std::uint8_t { AS_User, AS_Internal };

/* Storage duration */
enum class StorageDuration : std::uint8_t { SD_Static, SD_Automatic, SD_Heap, SD_StorageLast };

/* Storage types */
enum class StorageType : std::uint8_t { ST_Volatile, ST_Durable, ST_StorageLast };

#endif /* GENMC_ACTION_ENUMS_HPP */
