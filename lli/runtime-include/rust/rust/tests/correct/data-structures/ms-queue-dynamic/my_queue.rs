#![no_main]
//! NOTE: Must disable PostDominator Check in EliminateAnnotationsPass::findMatchingEnd
//! for this to work

use std::ffi::*;
use std::sync::atomic::AtomicPtr;
use std::sync::atomic::Ordering::*;
use std::genmc::{__VERIFIER_hp_retire, __VERIFIER_hp_t, __VERIFIER_hp_alloc, __VERIFIER_hp_free, __VERIFIER_hp_protect, __VERIFIER_annotate_begin, __VERIFIER_annotate_end};

#[repr(C)]
pub struct Node {
    value: c_uint,
    next: AtomicPtr<Node>
}


#[repr(C)]
pub struct Queue {
    head: AtomicPtr<Node>,
    tail: AtomicPtr<Node>
}

pub const fn empty_queue() -> Queue {
    Queue {
        head: AtomicPtr::new(std::ptr::null_mut()),
        tail: AtomicPtr::new(std::ptr::null_mut())
    }
}


#[no_mangle]
pub fn new_node() -> *mut Node {
    unsafe { std::alloc::alloc(std::alloc::Layout::new::<Node>()) as *mut Node }
}

#[no_mangle]
pub fn reclaim(node: *mut Node) {
    __VERIFIER_hp_retire(node);
}

#[no_mangle]
pub fn init_queue(q: &Queue, _num_threads: c_int) {
    let dummy = new_node();
    unsafe {
        (*dummy).next = AtomicPtr::default();
        (*dummy).value = 0;
    }
    q.head.store(dummy, SeqCst);
    q.tail.store(dummy, SeqCst);
}

#[no_mangle]
pub fn clear_queue(q: &Queue, _num_threads: c_int) {
    let mut next: *mut Node = std::ptr::null_mut();
    loop {
        let head: *mut Node = q.head.load(SeqCst);
        if head.is_null() {
            break;
        }

        next = unsafe { (*head).next.load(SeqCst) };
        unsafe { std::alloc::dealloc(head as *mut u8, std::alloc::Layout::new::<Node>()) };
        q.head.store(next, SeqCst);
    }
}

#[no_mangle]
pub fn enqueue(q: &Queue, val: c_uint) {
    let mut tail: *mut Node = std::ptr::null_mut();
    let mut next: *mut Node = std::ptr::null_mut();

    let node: *mut Node = new_node();
    unsafe { (*node).value = val };
    unsafe { (*node).next = AtomicPtr::default() };
    //Maybe set node.next to NULL?

    let hp: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
    loop {
        tail = __VERIFIER_hp_protect(hp, &q.tail);
        next = unsafe { (*tail).next.load(Acquire) };
        if tail != q.tail.load(Acquire) {
            continue;
        }

        if next.is_null() {
            let res = __VERIFIER_final_CAS!(
                unsafe { (*tail).next.compare_exchange(next, node, Release, Relaxed).is_ok() }
            );
            if res {
                break;
            }
        }
        else {
            __VERIFIER_helping_CAS!(
                q.tail.compare_exchange(tail, next, Release, Relaxed).is_ok()
            );
        }
    }
    __VERIFIER_helped_CAS!(
        q.tail.compare_exchange(tail, node, Release, Relaxed).is_ok()
    );
    __VERIFIER_hp_free(hp);
}


#[no_mangle]
pub fn dequeue(q: &Queue, retval: *mut c_uint) -> bool {
    let mut head: *mut Node = std::ptr::null_mut();
    let mut tail: *mut Node = std::ptr::null_mut();
    let mut next: *mut Node = std::ptr::null_mut();
    let mut ret: c_uint = 0;

    let hp_head: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
    let hp_next: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
    loop {
        head = __VERIFIER_hp_protect(hp_head, &q.head);
        tail = q.tail.load(Acquire);
        next = __VERIFIER_hp_protect(hp_next, unsafe { &((*head).next) });

        if q.head.load(Acquire) != head {
            continue;
        }
        if head == tail {
            if next.is_null() {
                __VERIFIER_hp_free(hp_head);
                __VERIFIER_hp_free(hp_next);
                return false;
            }
            __VERIFIER_helping_CAS!(
                q.tail.compare_exchange(tail, next, Release, Relaxed).is_ok()
            );
        }
        else {
            ret = unsafe { (*next).value };
            if q.head.compare_exchange(head, next, Release, Relaxed).is_ok() {
                break;
            }
        }
    }
    unsafe { *retval = ret };
    reclaim(head);
    __VERIFIER_hp_free(hp_head);
    __VERIFIER_hp_free(hp_next);
    return true;
}