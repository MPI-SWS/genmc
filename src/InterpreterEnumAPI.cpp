/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include "InterpreterEnumAPI.hpp"
#include "Error.hpp"

const std::unordered_map<std::string, InternalFunctions> internalFunNames = {
	{"__VERIFIER_assert_fail", InternalFunctions::FN_AssertFail},
	{"__VERIFIER_opt_begin", InternalFunctions::FN_OptBegin},
	{"__VERIFIER_loop_begin", InternalFunctions::FN_LoopBegin},
	{"__VERIFIER_spin_start", InternalFunctions::FN_SpinStart},
	{"__VERIFIER_spin_end", InternalFunctions::FN_SpinEnd},
	{"__VERIFIER_faiZNE_spin_end", InternalFunctions::FN_FaiZNESpinEnd},
	{"__VERIFIER_lockZNE_spin_end", InternalFunctions::FN_LockZNESpinEnd},
	{"__VERIFIER_kill_thread", InternalFunctions::FN_KillThread},
	{"__VERIFIER_assume", InternalFunctions::FN_Assume},
	{"__VERIFIER_nondet_int", InternalFunctions::FN_NondetInt},
	{"__VERIFIER_malloc", InternalFunctions::FN_Malloc},
	{"__VERIFIER_malloc_aligned", InternalFunctions::FN_MallocAligned},
	{"__VERIFIER_palloc", InternalFunctions::FN_PMalloc},
	{"__VERIFIER_free", InternalFunctions::FN_Free},
	{"__VERIFIER_thread_self", InternalFunctions::FN_ThreadSelf},
	{"__VERIFIER_thread_create", InternalFunctions::FN_ThreadCreate},
	{"__VERIFIER_thread_create_symmetric", InternalFunctions::FN_ThreadCreateSymmetric},
	{"__VERIFIER_thread_join", InternalFunctions::FN_ThreadJoin},
	{"__VERIFIER_thread_exit", InternalFunctions::FN_ThreadExit},
	{"__VERIFIER_atexit", InternalFunctions::FN_AtExit},
	{"__VERIFIER_mutex_init", InternalFunctions::FN_MutexInit},
	{"__VERIFIER_mutex_lock", InternalFunctions::FN_MutexLock},
	{"__VERIFIER_mutex_unlock", InternalFunctions::FN_MutexUnlock},
	{"__VERIFIER_mutex_trylock", InternalFunctions::FN_MutexTrylock},
	{"__VERIFIER_mutex_destroy", InternalFunctions::FN_MutexDestroy},
	{"__VERIFIER_barrier_init", InternalFunctions::FN_BarrierInit},
	{"__VERIFIER_barrier_wait", InternalFunctions::FN_BarrierWait},
	{"__VERIFIER_barrier_destroy", InternalFunctions::FN_BarrierDestroy},
	{"__VERIFIER_hazptr_alloc", InternalFunctions::FN_HazptrAlloc},
	{"__VERIFIER_hazptr_protect", InternalFunctions::FN_HazptrProtect},
	{"__VERIFIER_hazptr_clear", InternalFunctions::FN_HazptrClear},
	{"__VERIFIER_hazptr_free", InternalFunctions::FN_HazptrFree},
	{"__VERIFIER_hazptr_retire", InternalFunctions::FN_HazptrRetire},
	{"__VERIFIER_openFS", InternalFunctions::FN_OpenFS},
	{"__VERIFIER_closeFS", InternalFunctions::FN_CloseFS},
	{"__VERIFIER_creatFS", InternalFunctions::FN_CreatFS},
	{"__VERIFIER_renameFS", InternalFunctions::FN_RenameFS},
	{"__VERIFIER_linkFS", InternalFunctions::FN_LinkFS},
	{"__VERIFIER_unlinkFS", InternalFunctions::FN_UnlinkFS},
	{"__VERIFIER_truncateFS", InternalFunctions::FN_TruncateFS},
	{"__VERIFIER_readFS", InternalFunctions::FN_ReadFS},
	{"__VERIFIER_preadFS", InternalFunctions::FN_PreadFS},
	{"__VERIFIER_writeFS", InternalFunctions::FN_WriteFS},
	{"__VERIFIER_pwriteFS", InternalFunctions::FN_PwriteFS},
	{"__VERIFIER_fsyncFS", InternalFunctions::FN_FsyncFS},
	{"__VERIFIER_syncFS", InternalFunctions::FN_SyncFS},
	{"__VERIFIER_lseekFS", InternalFunctions::FN_LseekFS},
	{"__VERIFIER_pbarrier", InternalFunctions::FN_PersBarrierFS},
	{"__VERIFIER_annotate_begin", InternalFunctions::FN_AnnotateBegin},
	{"__VERIFIER_annotate_end", InternalFunctions::FN_AnnotateEnd},
	{"__VERIFIER_lkmm_fence", InternalFunctions::FN_SmpFenceLKMM},
	{"__VERIFIER_rcu_read_lock", InternalFunctions::FN_RCUReadLockLKMM},
	{"__VERIFIER_rcu_read_unlock", InternalFunctions::FN_RCUReadUnlockLKMM},
	{"__VERIFIER_synchronize_rcu", InternalFunctions::FN_SynchronizeRCULKMM},
	{"__VERIFIER_clflush", InternalFunctions::FN_CLFlush},
	/* Some C++ calls */
	{"_Znwm", InternalFunctions::FN_Malloc},
	{"_ZdlPv", InternalFunctions::FN_Free},
};

llvm::raw_ostream& operator<<(llvm::raw_ostream& s, const BlockageType &b)
{
	switch (b) {
	case BlockageType::ThreadJoin:
		s <<  "join";
		break;
	case BlockageType::Spinloop:
		s <<  "spinloop";
		break;
	case BlockageType::FaiZNESpinloop:
		s <<  "FAI-ZNE";
		break;
	case BlockageType::LockZNESpinloop:
		s <<  "lock-ZNE";
		break;
	case BlockageType::HelpedCas:
		s <<  "helped-CAS";
		break;
	case BlockageType::Confirmation:
		s <<  "confirmation";
		break;
	case BlockageType::ReadOptBlock:
		s <<  "read-opt";
		break;
	case BlockageType::LockNotAcq:
		s <<  "lock-unacq";
		break;
	case BlockageType::LockNotRel:
		s <<  "lock-unrel";
		break;
	case BlockageType::Barrier:
		s <<  "barrier";
		break;
	case BlockageType::Cons:
		s <<  "cons";
		break;
	case BlockageType::Error:
		s <<  "error";
		break;
	case BlockageType::User:
		s <<  "user";
		break;
	default:
		PRINT_BUGREPORT_INFO_ONCE("print-blockage-type",
					  "Cannot print blockage type");
		s << "???";
	}
	return s;
}
