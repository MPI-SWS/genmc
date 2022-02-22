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

#include <llvm/Support/raw_ostream.h>
#include <config.h>
#include <string>
#include <unordered_map>

/* The different reasons a thread might block */
enum class BlockageType {
	NotBlocked,
	ThreadJoin,
	Spinloop,
	FaiZNESpinloop,
	LockZNESpinloop,
	HelpedCas,
	Confirmation,
	ReadOptBlock,
	LockNotAcq,
	LockNotRel,
	Barrier,
	Cons,
	Error,
	User,
};

/* Pers: Journaling mount options */
enum class JournalDataFS { writeback, ordered, journal };

/* Types of allocations in the interpreter */
enum class AddressSpace { AS_User, AS_Internal };

/* Storage types */
enum class Storage { ST_Static, ST_Automatic, ST_Heap, ST_StorageLast };

/* Modeled functions -- (CAUTION: Order matters) */
enum class InternalFunctions {
	FN_AssertFail,
	FN_OptBegin,
	FN_LoopBegin,
	FN_SpinStart,

	FN_SpinEnd,
	FN_FaiZNESpinEnd,
	FN_LockZNESpinEnd,
	FN_Assume,
	/* Assume calls */
	FN_KillThread,
	FN_NondetInt,
	FN_ThreadSelf,
	FN_AnnotateRead,
	FN_AnnotateWrite,
	FN_AnnotateCas,
	FN_AnnotateFai,
	FN_HazptrProtect,
	FN_HazptrClear,
	FN_NoSideEffectsLast,
	/* No side effects */
	FN_ThreadCreate,
	FN_ThreadJoin,
	FN_ThreadExit,
	FN_Malloc,
	FN_MallocAligned,
	FN_HazptrAlloc,
	FN_MallocLast,
	FN_Free,
	FN_HazptrFree,
	FN_HazptrRetire,
	FN_MutexInit,
	FN_MutexLock,
	FN_MutexUnlock,
	FN_MutexTrylock,
	FN_MutexDestroy,
	FN_BarrierInit,
	FN_BarrierWait,
	FN_BarrierDestroy,

	FN_OpenFS,
	FN_CreatFS,
	FN_RenameFS,
	FN_LinkFS,
	FN_UnlinkFS,
	FN_TruncateFS,
	FN_LastInodeFS,
	/* Inode ops */
	FN_WriteFS,
	FN_PwriteFS,
	FN_PersBarrierFS,
	FN_LastInvRecFS,
	/* Invalid rec ops */
	FN_ReadFS,
	FN_PreadFS,
	FN_FsyncFS,
	FN_SyncFS,
	FN_LseekFS,
	FN_CloseFS,
	FN_LastFS,
	/* FS ops */

	FN_SmpFenceLKMM,
	FN_RCUReadLockLKMM,
	FN_RCUReadUnlockLKMM,
	FN_SynchronizeRCULKMM,
	/* LKMM ops */
};

extern const std::unordered_map<std::string, InternalFunctions> internalFunNames;

inline bool isInternalFunction(const std::string &name)
{
	return internalFunNames.count(name);
}

inline bool isCleanInternalFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	auto &code = internalFunNames.at(name);
	return code >= InternalFunctions::FN_AssertFail && code <= InternalFunctions::FN_NoSideEffectsLast;
}

inline bool isErrorFunction(const std::string &name)
{
	return isInternalFunction(name) && internalFunNames.at(name) == InternalFunctions::FN_AssertFail;
}

inline bool isAssumeFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	auto &code = internalFunNames.at(name);
	return code >= InternalFunctions::FN_SpinEnd && code <= InternalFunctions::FN_Assume;
}

inline bool isAllocFunction(const std::string &name)
{
	if (!isInternalFunction(name))
		return false;

	auto &code = internalFunNames.at(name);
	return code >= InternalFunctions::FN_Malloc && code <= InternalFunctions::FN_MallocLast;
}

inline bool isMutexCode(InternalFunctions code)
{
	return (code >= InternalFunctions::FN_MutexInit && code <= InternalFunctions::FN_MutexDestroy);
}

inline bool isBarrierCode(InternalFunctions code)
{
	return (code >= InternalFunctions::FN_BarrierInit && code <= InternalFunctions::FN_BarrierDestroy);
}

inline bool isFsCode(InternalFunctions code)
{
	return (code >= InternalFunctions::FN_OpenFS && code <= InternalFunctions::FN_LastFS);
}

inline bool isFsInodeCode(InternalFunctions code)
{
	return (code >= InternalFunctions::FN_OpenFS && code <= InternalFunctions::FN_LastInodeFS);
}

inline bool isFsInvalidRecCode(InternalFunctions code)
{
	return (code >= InternalFunctions::FN_CreatFS && code <= InternalFunctions::FN_LastInvRecFS);
}

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

/* Should match our internal definitions */

#define GENMC_ATTR_LOCAL   0x00000001
#define GENMC_ATTR_FINAL   0x00000002

#define GENMC_KIND_NONVR   0x00010000
#define GENMC_KIND_HELPED  0x00020000
#define GENMC_KIND_HELPING 0x00040000
#define GENMC_KIND_SPECUL  0x00080000
#define GENMC_KIND_CONFIRM 0x00100000

#define GENMC_ATTR(flags) ((flags) & (0x0000ffff))
#define GENMC_KIND(flags) ((flags) & (0xffff0000))

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

extern llvm::raw_ostream& operator<<(llvm::raw_ostream& rhs,
				     const BlockageType &b);

#endif /* __INTERPRETER_ENUM_API_HPP__ */
