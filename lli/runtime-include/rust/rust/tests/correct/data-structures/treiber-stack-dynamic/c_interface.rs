#![no_main]

mod my_stack;
pub use my_stack::*;
pub use std::ffi::*;
use std::sync::atomic::Ordering::*;
use std::sync::atomic::AtomicPtr;

#[no_mangle]
pub extern "C" fn init_stack(stack: *mut MyStack, _num_threads: c_int) {
    unsafe {
        (*stack).top = AtomicPtr::new(std::ptr::null_mut());
        (*stack).top.store(std::ptr::null_mut(), SeqCst);
    }
}

#[no_mangle]
pub extern "C" fn clear_stack(stack: &MyStack, _num_threads: c_int) {
    loop {
        let top_: *mut Node = stack.top.load(SeqCst);
        if top_.is_null() {
            return;
        }
        let next_: *mut Node = unsafe { (*top_).next.load(SeqCst) };
        unsafe { std::alloc::dealloc(top_ as *mut u8, std::alloc::Layout::new::<Node>()) };
        stack.top.store(next_, SeqCst);
    }
}

#[no_mangle]
pub extern "C" fn push(stack: &MyStack, val: c_uint) {
    stack.push(val);
}

#[no_mangle]
pub extern "C" fn pop(stack: &MyStack) -> c_uint {
    stack.pop()
}