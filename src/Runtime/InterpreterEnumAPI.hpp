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

#ifndef GENMC_INTERPRETER_ENUM_API_HPP
#define GENMC_INTERPRETER_ENUM_API_HPP

#include "ExecutionGraph/Event.hpp"
#include "ExecutionGraph/LoadAnnotation.hpp"

#include <string>
#include <unordered_map>

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

/* Modeled functions */
enum class InternalFunctions : std::int8_t {
#define HANDLE_FUNCTION(NUM, FUN, NAME) NAME = NUM,
#include "Runtime/InternalFunction.def"
};

extern const std::unordered_map<std::string, InternalFunctions> internalFunNames;

inline bool isInternalFunction(const std::string &name) { return internalFunNames.count(name); }

inline bool isCleanInternalFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	auto code =
		static_cast<std::underlying_type_t<InternalFunctions>>(internalFunNames.at(name));
	return
#define FIRST_PURE_FUNCTION(NUM) code >= NUM &&
#define LAST_PURE_FUNCTION(NUM) code <= NUM;
#include "Runtime/InternalFunction.def"
}

inline bool isErrorFunction(const std::string &name)
{
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::AssertFail;
}

inline bool isAssumeFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	return internalFunNames.at(name) == InternalFunctions::Assume;
}

inline bool isAllocFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	auto code =
		static_cast<std::underlying_type_t<InternalFunctions>>(internalFunNames.at(name));
	return
#define FIRST_ALLOC_FUNCTION(NUM) code >= NUM &&
#define LAST_ALLOC_FUNCTION(NUM) code <= NUM;
#include "Runtime/InternalFunction.def"
}

inline bool isMutexCode(InternalFunctions code)
{
	auto codeI = static_cast<std::underlying_type_t<InternalFunctions>>(code);
	return
#define FIRST_MUTEX_FUNCTION(NUM) codeI >= NUM &&
#define LAST_MUTEX_FUNCTION(NUM) codeI <= NUM;
#include "Runtime/InternalFunction.def"
}

inline bool isCondVarCode(InternalFunctions code)
{
	auto codeI = static_cast<std::underlying_type_t<InternalFunctions>>(code);
	return
#define FIRST_CONDVAR_FUNCTION(NUM) codeI >= NUM &&
#define LAST_CONDVAR_FUNCTION(NUM) codeI <= NUM;
#include "Runtime/InternalFunction.def"
}

inline bool hasGlobalLoadSemantics(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	using IF = InternalFunctions;
	auto code = internalFunNames.at(name);
	return code == IF::MutexLock || code == IF::MutexTrylock || code == IF::CondVarWait;
}

/* Should match our internal definitions */

#define GENMC_ATTR_LOCAL 	0x00000001
#define GENMC_ATTR_FINAL 	0x00000002

#define GENMC_KIND_NONVR 	0x00010000
#define GENMC_KIND_HELPED 	0x00020000
#define GENMC_KIND_HELPING 	0x00040000
#define GENMC_KIND_SPECUL 	0x00080000
#define GENMC_KIND_CONFIRM 	0x00100000
#define GENMC_KIND_PLOCK 	0x00200000
#define GENMC_KIND_BARRIER 	0x00400000

#define GENMC_ATTR(flags) ((flags) & (0x0000ffff))
#define GENMC_KIND(flags) ((flags) & (0xffff0000))

#endif /* GENMC_INTERPRETER_ENUM_API_HPP */
