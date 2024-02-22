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

#include "VerificationError.hpp"
#include "Error.hpp"

const std::unordered_map<SystemError, std::string, ENUM_HASH(SystemError)> errorList = {
	{SystemError::SE_EPERM, "Operation not permitted"},
	{SystemError::SE_ENOENT, "No such file or directory"},
	{SystemError::SE_EIO, "Input/output error"},
	{SystemError::SE_EBADF, "Bad file descriptor"},
	{SystemError::SE_ENOMEM, "Cannot allocate memory"},
	{SystemError::SE_EEXIST, "File exists"},
	{SystemError::SE_EINVAL, "Invalid argument"},
	{SystemError::SE_EMFILE, "Too many open files"},
	{SystemError::SE_ENFILE, "Too many open files in system"},
	{SystemError::SE_EFBIG, "File too large"},
};

SystemError systemErrorNumber;

llvm::raw_ostream &operator<<(llvm::raw_ostream &s, const VerificationError &st)
{
	switch (st) {
	case VerificationError::VE_OK:
		return s << "OK";
	case VerificationError::VE_Safety:
		return s << "Safety violation";
	case VerificationError::VE_Recovery:
		return s << "Recovery error";
	case VerificationError::VE_Liveness:
		return s << "Liveness violation";
	case VerificationError::VE_RaceNotAtomic:
		return s << "Non-atomic race";
	case VerificationError::VE_WWRace:
		return s << "Write-write race";
	case VerificationError::VE_RaceFreeMalloc:
		return s << "Malloc-free race";
	case VerificationError::VE_FreeNonMalloc:
		return s << "Attempt to free non-allocated memory";
	case VerificationError::VE_DoubleFree:
		return s << "Double-free error";
	case VerificationError::VE_Allocation:
		return s << "Allocation error";
	case VerificationError::VE_UninitializedMem:
		return s << "Attempt to read from uninitialized memory";
	case VerificationError::VE_AccessNonMalloc:
		return s << "Attempt to access non-allocated memory";
	case VerificationError::VE_AccessFreed:
		return s << "Attempt to access freed memory";
	case VerificationError::VE_InvalidCreate:
		return s << "Invalid create() operation";
	case VerificationError::VE_InvalidJoin:
		return s << "Invalid join() operation";
	case VerificationError::VE_InvalidUnlock:
		return s << "Invalid unlock() operation";
	case VerificationError::VE_InvalidBInit:
		return s << "Invalid barrier_init() operation";
	case VerificationError::VE_InvalidRecoveryCall:
		return s << "Invalid function call during recovery";
	case VerificationError::VE_InvalidTruncate:
		return s << "Invalid file truncation";
	case VerificationError::VE_Annotation:
		return s << "Annotation error";
	case VerificationError::VE_MixedSize:
		return s << "Mixed-size accesses";
	case VerificationError::VE_SystemError:
		return s << errorList.at(systemErrorNumber);
	default:
		return s << "Uknown status";
	}
}
