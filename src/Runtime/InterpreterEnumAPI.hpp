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

#ifndef GENMC_INTERPRETER_ENUM_API_HPP
#define GENMC_INTERPRETER_ENUM_API_HPP

#include <config.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <unordered_map>

/* Pers: Journaling mount options */
enum class JournalDataFS { writeback, ordered, journal };

/* Types of allocations in the interpreter */
enum class AddressSpace { AS_User, AS_Internal };

/* Storage duration */
enum class StorageDuration { SD_Static, SD_Automatic, SD_Heap, SD_StorageLast };

/* Storage types */
enum class StorageType { ST_Volatile, ST_Durable, ST_StorageLast };

/* Modeled functions */
enum class InternalFunctions {
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

inline bool isSpinEndFunction(const std::string &name)
{
	return isInternalFunction(name) && internalFunNames.at(name) == InternalFunctions::SpinEnd;
}

inline bool isAssumeFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	auto code =
		static_cast<std::underlying_type_t<InternalFunctions>>(internalFunNames.at(name));
	return
#define FIRST_ASSUME_FUNCTION(NUM) code >= NUM &&
#define LAST_ASSUME_FUNCTION(NUM) code <= NUM;
#include "Runtime/InternalFunction.def"
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

inline bool isBarrierCode(InternalFunctions code)
{
	auto codeI = static_cast<std::underlying_type_t<InternalFunctions>>(code);
	return
#define FIRST_BARRIER_FUNCTION(NUM) codeI >= NUM &&
#define LAST_BARRIER_FUNCTION(NUM) codeI <= NUM;
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
	return code == IF::MutexLock || code == IF::MutexTrylock || code == IF::BarrierWait ||
	       code == IF::CondVarWait;
}

/* Should match our internal definitions */

#define GENMC_ATTR_LOCAL 0x00000001
#define GENMC_ATTR_FINAL 0x00000002

#define GENMC_KIND_NONVR 0x00010000
#define GENMC_KIND_HELPED 0x00020000
#define GENMC_KIND_HELPING 0x00040000
#define GENMC_KIND_SPECUL 0x00080000
#define GENMC_KIND_CONFIRM 0x00100000
#define GENMC_KIND_PLOCK 0x00200000

#define GENMC_ATTR(flags) ((flags) & (0x0000ffff))
#define GENMC_KIND(flags) ((flags) & (0xffff0000))

#endif /* GENMC_INTERPRETER_ENUM_API_HPP */
