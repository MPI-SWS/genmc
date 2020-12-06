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

#ifndef __INTERPRETER_ENUM_API_HPP__
#define __INTERPRETER_ENUM_API_HPP__

#include <config.h>
#include <string>
#include <unordered_map>

/* Types of allocations in the interpreter */
enum class AddressSpace { AS_User, AS_Internal };

/* Storage types */
enum class Storage { ST_Static, ST_Automatic, ST_Heap, ST_StorageLast };

/* Modeled functions */
enum class InternalFunctions {
	FN_AssertFail,
	FN_EndLoop,
	FN_Assume,
	FN_NondetInt,
	FN_Malloc,
	FN_MallocAligned,
	FN_Free,
	FN_ThreadSelf,
	FN_ThreadCreate,
	FN_ThreadJoin,
	FN_ThreadExit,
	FN_MutexInit,
	FN_MutexLock,
	FN_MutexUnlock,
	FN_MutexTrylock,
	FN_OpenFS,
	FN_CreatFS,
	FN_RenameFS,
	FN_LinkFS,
	FN_UnlinkFS,
	FN_TruncateFS,
	FN_LastInodeFS,
	FN_WriteFS,
	FN_PwriteFS,
	FN_PersBarrierFS,
	FN_LastInvRecFS,
	FN_ReadFS,
	FN_PreadFS,
	FN_FsyncFS,
	FN_SyncFS,
	FN_LseekFS,
	FN_CloseFS,
	FN_LastFS,
};

#define IS_INTERNAL_FUNCTION(name) internalFunNames.count(name)
#define IS_FS_FN_CODE(code)						\
	(code >= InternalFunctions::FN_OpenFS && code <= InternalFunctions::FN_LastFS)
#define IS_FS_INODE_FN_CODE(code)					\
	(code >= InternalFunctions::FN_OpenFS && code <= InternalFunctions::FN_LastInodeFS)
#define IS_FS_INVALID_REC_CODE(code)					\
	(code >= InternalFunctions::FN_CreatFS && code <= InternalFunctions::FN_LastInvRecFS)

extern const std::unordered_map<std::string, InternalFunctions> internalFunNames;

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

#endif /* __INTERPRETER_ENUM_API_HPP__ */
