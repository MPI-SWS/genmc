//! This file provides the thread-module specific interfaces found
//! in genmc.h and genmc_internal.h. See there or the manual for
//! documentation of these interfaces.
//!
//! Bodies of all internal __VERIFIER_* functions (same as genmc_internal.h)
//! will be removed again and the function marked as "extern" during the
//! RustPrepPass.
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
pub use core::ffi::*;

pub type __VERIFIER_thread_t = u64;


#[no_mangle]
#[inline(always)]
pub fn __VERIFIER_spawn(
    __start_routine: *const c_void,
    __arg: *mut c_void,
) -> __VERIFIER_thread_t {
    __VERIFIER_thread_create(crate::NULL as *const __VERIFIER_attr_t, __start_routine, __arg)
}

#[no_mangle]
#[inline(always)]
pub fn __VERIFIER_spawn_symmetric(
    __start_routine: *const c_void,
    __arg: *mut c_void,
    __th: __VERIFIER_thread_t
) -> __VERIFIER_thread_t {
    __VERIFIER_thread_create_symmetric(crate::NULL as *const __VERIFIER_attr_t, __start_routine as *mut c_void, __arg, __th)
}

#[no_mangle]
#[inline(always)]
pub fn __VERIFIER_join(__th: __VERIFIER_thread_t) -> *mut c_void {
    __VERIFIER_thread_join(__th)
}

#[no_mangle]
#[inline(always)]
pub fn __VERIFIER_join_symmetric(__th: __VERIFIER_thread_t) {
    __VERIFIER_join(__th);
}


/* Internal types & interfaces */

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub(crate) struct __VERIFIER_attr_t {
    pub __private: i32,
}


#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_thread_create(
    __attr: *const __VERIFIER_attr_t,
    __start_routine: *const c_void,
    __arg: *mut c_void,
) -> __VERIFIER_thread_t {0}

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_thread_create_symmetric(
    __attr: *const __VERIFIER_attr_t,
    __start_routine: *const c_void,
    __arg: *mut c_void,
    __th: __VERIFIER_thread_t,
) -> __VERIFIER_thread_t {0}

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_thread_exit(__retval: *mut c_void) {}

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_thread_join(__th: __VERIFIER_thread_t) -> *mut c_void {crate::NULL as *mut c_void}

#[no_mangle]
#[inline(never)]
pub(crate) extern "C" fn __VERIFIER_thread_self() -> __VERIFIER_thread_t {0}