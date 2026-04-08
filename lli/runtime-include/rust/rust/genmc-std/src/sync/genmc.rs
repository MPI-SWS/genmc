//! This file provides the thread-module specific interfaces found
//! in genmc.h and genmc_internal.h. See there or the manual for
//! documentation of these interfaces.
//!
//! Condvars is not included.
//!
//! All functions in this file are internal, thus bodies of all
//! __VERIFIER_* functions in this file will be removed again
//! and the function marked as "extern" during the RustPrepPass.
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

pub(crate) mod mutex {
    #[repr(C)]
    #[derive(Debug, Copy, Clone)]
    pub struct __VERIFIER_mutex_t {
        pub __private: i32,
    }

    pub type __VERIFIER_mutexattr_t = i64;

    #[macro_export]
    macro_rules! __VERIFIER_MUTEX_INITIALIZER {() => {0};}

    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_mutex_init(
        __mutex: *mut __VERIFIER_mutex_t,
        __mutexattr: *const __VERIFIER_mutexattr_t,
    ) -> i32 {0}
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_mutex_destroy(
        __mutex: *mut __VERIFIER_mutex_t,
    ) -> i32 {0}
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_mutex_trylock(
        __mutex: *mut __VERIFIER_mutex_t,
    ) -> i32 {0}
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_mutex_lock(
        __mutex: *mut __VERIFIER_mutex_t,
    ) -> i32 {0}
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_mutex_unlock(
        __mutex: *mut __VERIFIER_mutex_t,
    ) -> i32 {0}
}


pub(crate) mod barrier {
    /* Data types and variables */
    #[repr(C)]
    #[derive(Debug, Copy, Clone)]
    pub struct __VERIFIER_barrier_t {
        pub __private: i32,
    }

    pub type __VERIFIER_barrierattr_t = i32;

    /* barrier functions */
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_barrier_init(
        __barrier: *mut __VERIFIER_barrier_t,
        __attr: *const __VERIFIER_barrierattr_t,
        __count: u32,
    ) -> i32 {0}
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_barrier_destroy(
        __barrier: *mut __VERIFIER_barrier_t,
    ) -> i32 {0}
    #[no_mangle]
    #[inline(never)]
    pub extern "C" fn __VERIFIER_barrier_wait(
        __barrier: *mut __VERIFIER_barrier_t,
    ) -> i32 {0}
}