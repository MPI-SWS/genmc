#![allow(non_snake_case)]
//! Calls to __rust_alloc & co. will be overwritten with calls
//! to genmc__rust_alloc & co. instead in the RustPrepPass.
//!
//! __rust_alloc & co. are called from std::alloc::alloc(...) & co.
//! and are defined in the same file (alloc.rs).
//! See there for Rusts documentation of these functions.
//!
//! These functions don't have a function body, instead calls there
//! will be rerouted towards the GlobalAllocator. We imitate the GlobalAllocator
//! by forwarding those calls to our own __genmc_rust_alloc & co. in the RustPrepPass.
//!
//! Implementation of __genmc_rust_alloc & co. is
//! inspired by the standard GlobalAllocator for Unix Systems,
//! see in Rusts open-source codebase for STD: std->sys->alloc->unix.rs
use core::ffi::*;


//These don't need to have their bodies removed in a LLVM-Pass, as they're not called directly anyways.
extern "C" {
    pub fn __VERIFIER_malloc_aligned(align: usize, size: usize) -> *mut u8;
    pub fn __VERIFIER_malloc(size: usize) -> *mut u8;
    pub fn __VERIFIER_free(ptr: *mut c_void);
}



#[no_mangle]
fn genmc__rust_alloc(size: usize, align: usize) -> *mut u8 { //__rust_alloc calls will be forwarded here
    unsafe {
        if align <= 16 && align <= size {
            return __VERIFIER_malloc(size) as *mut u8;
        }
        __VERIFIER_malloc_aligned(align, size) as *mut u8
    }
}
#[no_mangle]
fn genmc__rust_dealloc(ptr: *mut u8, _size: usize, _align: usize) { //__rust_dealloc calls will be forwarded here
    unsafe {
        __VERIFIER_free(ptr as *mut c_void)
    }
}
#[no_mangle]
fn genmc__rust_realloc(_ptr: *mut u8, _old_size: usize, _align: usize, _new_size: usize) -> *mut u8 { //__rust_realloc calls will be forwarded here
    panic!("Memory reallocation not supported!")
}
#[no_mangle]
fn genmc__rust_alloc_zeroed(size: usize, align: usize) -> *mut u8 { //__rust_alloc_zeroed calls will be forwarded here
    let ptr = genmc__rust_alloc(size, align);
    unsafe {
        std::ptr::write_bytes(ptr, 0x0u8, size);
    }
    ptr
}
