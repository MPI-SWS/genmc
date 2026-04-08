//! This file preovides the same interfaces as genmc.h and genmc_internal in C.
//! Note that we moved some definitions into their respective modules
//! (e.g. thread, sync) where they're used for clarity.
//!
//! See genmc.h/genmc_internal.h or the manual for documentation on these
//! functions and types.
//!
//! Bodies of all internal __VERIFIER_* functions (same as genmc_internal.h)
//! will be removed again and the function marked as "extern" during the
//! RustPrepPass.
//!
//! Interfaces for persistent storage, condvar and RCU are not included.
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(unused_imports)]
use std::sync::atomic::AtomicPtr;
pub use core::ffi::*;

#[repr(i8)]
enum __VERIFIER_assume_t {
    GENMC_ASSUME_USER = 0,
	GENMC_ASSUME_BARRIER = 1,
	GENMC_ASSUME_SPINLOOP = 2,
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_assume(cond: bool) {
    __VERIFIER_assume_internal(cond, __VERIFIER_assume_t::GENMC_ASSUME_USER as i8);
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_assume_internal(_cond: bool, _typ: i8) {}

#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_nondet_int() -> i32 {0}


#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_loop_begin() {}
#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_spin_start() {}
#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_spin_end(cond: bool) {
    __VERIFIER_assume_internal(cond, __VERIFIER_assume_t::GENMC_ASSUME_SPINLOOP as i8);
}


#[macro_export]
macro_rules! __VERIFIER_local_write {
    ($s:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_ATTR_LOCAL)):
        $s;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_ATTR_LOCAL));
    };
}
#[macro_export]
macro_rules! __VERIFIER_final_write {
    ($s:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_ATTR_FINAL));
        $s;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_ATTR_FINAL));
    };
}
#[macro_export]
macro_rules! __VERIFIER_helped_CAS {
    ($c:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_KIND_HELPED));
        $c;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_KIND_HELPED));
    };
}
#[macro_export]
macro_rules! __VERIFIER_helping_CAS {
    ($c:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_KIND_HELPING));
        $c;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_KIND_HELPING));
    };
}
#[macro_export]
macro_rules! __VERIFIER_speculative_read {
    ($c:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_KIND_SPECUL));
        let __ret = $c;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_KIND_SPECUL));
        __ret
    };
}
#[macro_export]
macro_rules! __VERIFIER_confirming_read {
    ($c:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_KIND_CONFIRM));
        let __ret = $c;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_KIND_CONFIRM));
        __ret
    };
}
#[macro_export]
macro_rules! __VERIFIER_confirming_CAS {
    ($c:expr) => {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_KIND_CONFIRM));
        let __ret = $c;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_KIND_CONFIRM));
        __ret
    };
}
#[macro_export]
macro_rules! __VERIFIER_final_CAS {
    ($c:expr) => {
        {
        __VERIFIER_annotate_begin(genmc_opcode!(GENMC_ATTR_FINAL));
        let __ret = $c;
        __VERIFIER_annotate_end(genmc_opcode!(GENMC_ATTR_FINAL));
        __ret
        }
    };
}


#[macro_export]
macro_rules! __VERIFIER_optional {
    ($x:expr) => {
        if __VERIFIER_opt_begin() {
            $x;
            __VERIFIER_assume(false);
        }
    };
}



pub type __VERIFIER_hp_t = __VERIFIER_hazptr_t;

#[no_mangle]
#[inline(always)]
pub extern "C" fn __VERIFIER_hp_alloc() -> *mut __VERIFIER_hp_t {
    __VERIFIER_hazptr_alloc()
}

#[inline(always)]
pub extern "C" fn __VERIFIER_hp_protect<T>(hp: *mut __VERIFIER_hp_t, p: &AtomicPtr<T>) -> *mut T {
    let _p_ = p.load(std::sync::atomic::Ordering::Acquire);
    __VERIFIER_hazptr_protect(hp, _p_ as *mut c_void);
    _p_
}

#[no_mangle]
#[inline(always)]
pub extern "C" fn __VERIFIER_hp_clear(hp: *mut __VERIFIER_hp_t) {
    __VERIFIER_hazptr_clear(hp)
}

#[no_mangle]
#[inline(always)]
pub extern "C" fn __VERIFIER_hp_free(hp: *mut __VERIFIER_hp_t) {
    __VERIFIER_hazptr_free(hp)
}

#[inline(never)]
pub extern "C" fn __VERIFIER_hp_retire<T>(p: *mut T) {
    __VERIFIER_hazptr_retire(p as *mut c_void)
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_method_begin(_name: *const c_char, _argVal: u32) {}
#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_method_end(_name: *const c_char, _argVal: u32) {}



/* Internal types & interfaces */

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_assert_fail(_arg1: *const c_char, _arg2: *const c_char, _arg3: i32) {}

pub(crate) use crate::genmc_allocator::*;

#[no_mangle]
#[inline(never)]
///Paramater must be a pointer to a function. See use of __VERIFIER_thread_spawn as reference
pub(crate) extern "C" fn __VERIFIER_atexit(
    _func: *mut c_void
) -> i32 {0}


#[macro_export]
macro_rules! genmc_opcode {
    (GENMC_ATTR_LOCAL) => {0x00000001};
    (GENMC_ATTR_FINAL) => {0x00000002};

    (GENMC_KIND_NONVR) => {0x00010000};
    (GENMC_KIND_HELPED) => {0x00020000};
    (GENMC_KIND_HELPING) => {0x00040000};
    (GENMC_KIND_SPECUL) => {0x00080000};
    (GENMC_KIND_CONFIRM) => {0x00100000};
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_annotate_begin(_mask: i32) {}
#[no_mangle]
#[inline(never)]
pub extern "C" fn __VERIFIER_annotate_end(_mask: i32) {}

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_opt_begin() -> bool {false}


#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct __VERIFIER_hazptr_t { pub(crate) __dummy: *mut c_void}

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_hazptr_alloc() -> *mut __VERIFIER_hazptr_t {crate::NULL as *mut __VERIFIER_hazptr_t}
#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_hazptr_protect(_hp: *mut __VERIFIER_hazptr_t, _p: *mut c_void) {}
#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_hazptr_clear(_hp: *mut __VERIFIER_hazptr_t) {}
#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_hazptr_free(_hp: *mut __VERIFIER_hazptr_t) {}
#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_hazptr_retire(_p: *mut c_void) {}