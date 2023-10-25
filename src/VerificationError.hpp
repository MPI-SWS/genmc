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

#ifndef __VERIFICATION_ERROR_HPP__
#define __VERIFICATION_ERROR_HPP__

#include "Error.hpp"

#include <config.h>
#include <string>
#include <unordered_map>

/* Some basic system error codes for the user -- should match include/errno.h */
enum class SystemError {
	SE_EPERM   = 1,
	SE_ENOENT  = 2,
	SE_EIO     = 5,
	SE_EBADF   = 9,
	SE_ENOMEM  = 12,
	SE_EEXIST  = 17,
	SE_EINVAL  = 22,
	SE_EMFILE  = 24,
	SE_ENFILE  = 23,
	SE_ETXTBSY = 26,
	SE_EFBIG   = 27,
	SE_ESPIPE  = 29,
};

/* Different errors that might be encountered during verification.
 * Public to enable the interpreter utilize it */
enum class VerificationError {
	VE_OK,
	VE_Safety,
	VE_Recovery,
	VE_Liveness,
	VE_RaceNotAtomic,
	VE_WWRace,
	VE_RaceFreeMalloc,
	VE_FreeNonMalloc,
	VE_DoubleFree,
	VE_Allocation,

	VE_InvalidAccessBegin,
	VE_UninitializedMem,
	VE_AccessNonMalloc,
	VE_AccessFreed,
	VE_InvalidAccessEnd,

	VE_InvalidCreate,
	VE_InvalidJoin,
	VE_InvalidUnlock,
	VE_InvalidBInit,
	VE_InvalidRecoveryCall,
	VE_InvalidTruncate,
	VE_Annotation,
	VE_MixedSize,
	VE_SystemError,
};

inline bool isHardError(VerificationError err)
{
	return err != VerificationError::VE_OK && err != VerificationError::VE_WWRace;
}

/* For compilers that do not have a recent enough lib{std}c++ */
#ifndef STDLIBCPP_SUPPORTS_ENUM_MAP_KEYS
struct EnumClassHash {
	template <typename T>
	std::size_t operator()(T t) const {
		return static_cast<std::size_t>(t);
	}
};
#define ENUM_HASH(t) EnumClassHash
#else
#define ENUM_HASH(t) std::hash<t>
#endif

extern SystemError systemErrorNumber; // just to inform the driver
extern const std::unordered_map<SystemError, std::string, ENUM_HASH(SystemError)> errorList;

llvm::raw_ostream& operator<<(llvm::raw_ostream &s, const VerificationError &st);

#endif /* __VERIFICATION_ERROR_HPP__ */
