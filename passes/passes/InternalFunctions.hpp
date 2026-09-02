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

#ifndef GENMC_INTERNAL_FUNCTIONS_HPP
#define GENMC_INTERNAL_FUNCTIONS_HPP

#include <cstdint>
#include <string>
#include <unordered_map>

/* Modeled functions */
enum class InternalFunctions : std::int8_t {
#define HANDLE_FUNCTION(NUM, FUN, NAME) NAME = NUM,
#include "passes/InternalFunction.def"
};

extern const std::unordered_map<std::string, InternalFunctions> internalFunNames;

inline auto isInternalFunction(const std::string &name) -> bool
{
	return internalFunNames.contains(name);
}

inline auto isCleanInternalFunction(const std::string &name) -> bool
{
	if (!isInternalFunction(name))
		return false;

	auto code =
		static_cast<std::underlying_type_t<InternalFunctions>>(internalFunNames.at(name));
	return
#define FIRST_PURE_FUNCTION(NUM) code >= NUM &&
#define LAST_PURE_FUNCTION(NUM) code <= NUM;
#include "passes/InternalFunction.def"
}

inline auto isErrorFunction(const std::string &name) -> bool
{
	return isInternalFunction(name) &&
	       internalFunNames.at(name) == InternalFunctions::AssertFail;
}

inline auto isAssumeFunction(const std::string &name) -> bool
{
	if (!isInternalFunction(name))
		return false;

	return internalFunNames.at(name) == InternalFunctions::Assume;
}

inline auto isAllocFunction(const std::string &name) -> bool
{
	if (!isInternalFunction(name))
		return false;

	auto code =
		static_cast<std::underlying_type_t<InternalFunctions>>(internalFunNames.at(name));
	return
#define FIRST_ALLOC_FUNCTION(NUM) code >= NUM &&
#define LAST_ALLOC_FUNCTION(NUM) code <= NUM;
#include "passes/InternalFunction.def"
}

inline auto isAllocCode(InternalFunctions code) -> bool
{
	auto codeI = static_cast<std::underlying_type_t<InternalFunctions>>(code);
	return
#define FIRST_ALLOC_FUNCTION(NUM) codeI >= NUM &&
#define LAST_ALLOC_FUNCTION(NUM) codeI <= NUM;
#include "passes/InternalFunction.def"
}

inline auto isMutexCode(InternalFunctions code) -> bool
{
	auto codeI = static_cast<std::underlying_type_t<InternalFunctions>>(code);
	return
#define FIRST_MUTEX_FUNCTION(NUM) codeI >= NUM &&
#define LAST_MUTEX_FUNCTION(NUM) codeI <= NUM;
#include "passes/InternalFunction.def"
}

inline auto isCondVarCode(InternalFunctions code) -> bool
{
	auto codeI = static_cast<std::underlying_type_t<InternalFunctions>>(code);
	return
#define FIRST_CONDVAR_FUNCTION(NUM) codeI >= NUM &&
#define LAST_CONDVAR_FUNCTION(NUM) codeI <= NUM;
#include "passes/InternalFunction.def"
}

inline auto hasGlobalLoadSemantics(const std::string &name) -> bool
{
	if (!isInternalFunction(name))
		return false;

	using IF = InternalFunctions;
	auto code = internalFunNames.at(name);
	return code == IF::MutexLock || code == IF::MutexTrylock || code == IF::CondVarWait;
}

/* Annotation attribute flags — should match runtime definitions */

#define GENMC_ATTR_LOCAL 0x00000001
#define GENMC_ATTR_FINAL 0x00000002

#define GENMC_KIND_NONVR 0x00010000
#define GENMC_KIND_HELPED 0x00020000
#define GENMC_KIND_HELPING 0x00040000
#define GENMC_KIND_SPECUL 0x00080000
#define GENMC_KIND_CONFIRM 0x00100000
#define GENMC_KIND_PLOCK 0x00200000
#define GENMC_KIND_BARRIER 0x00400000

#define GENMC_ATTR(flags) ((flags) & (0x0000ffff))
#define GENMC_KIND(flags) ((flags) & (0xffff0000))

#endif /* GENMC_INTERNAL_FUNCTIONS_HPP */
