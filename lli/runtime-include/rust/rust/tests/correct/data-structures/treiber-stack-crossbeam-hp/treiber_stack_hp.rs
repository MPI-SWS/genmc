//! Here we model crossbeams (now removed) implementation of the Treiber-Stack.
//! Please refer to crossbeams repo for any further information.
//!
//! Using the Pull-Request where it this was removed as reference:
//! https://github.com/crossbeam-rs/crossbeam/pull/301/files
//!
//! Changes applied for our purpose of modeling:
//! - We use GenMC's hazard pointers for protecting memory from premature freeing
//!   (i.e. freeing while another thread still holds a reference to it)
//!   instead of crossbeams Garbage Collector.
//! - We don't use Guards as we don't need thread pinning
//! - We use AtomicPtr directly instead of Crossbeam's "Atomic" wrapper, since we don't
//!   need the Guards or "Shared" / "Owned" pointers anymore without the Garbage Collector.
//! - We removed Cache Padding
//! - compare_and_set(a, b, Release) = compare_exchange(a, b, Release, Relaxed), see removed:
//!     https://github.com/crossbeam-rs/crossbeam/commit/d3187be4ae1d9fc88666d872fda8a311d4c5d5b6
//!
//! Thus, this model is self-contained (no dependency on other crossbeam tools)
use std::mem::ManuallyDrop;
use std::ptr;
use std::sync::atomic::Ordering::{Acquire, Relaxed, Release};

use std::genmc::{__VERIFIER_hp_retire, __VERIFIER_hp_t, __VERIFIER_hp_alloc, __VERIFIER_hp_free, __VERIFIER_hp_protect};
use std::sync::atomic::AtomicPtr;

/// Treiber's lock-free stack.
///
/// Usable with any number of producers and consumers.
#[derive(Debug)]
#[repr(C)]
pub struct TreiberStack<T> {
    pub head: AtomicPtr<Node<T>>,
}

#[derive(Debug)]
#[repr(C)]
pub struct Node<T> {
    data: ManuallyDrop<T>,
    next: AtomicPtr<Node<T>>,
}

impl<T> TreiberStack<T> {
    /// Creates a new, empty stack.
    pub const fn new() -> TreiberStack<T> { //OWN: const, was not const
        TreiberStack {
            head: AtomicPtr::new(std::ptr::null_mut()),
        }
    }

    /// Pushes a value on top of the stack.
    pub fn push(&self, t: T) {
        let mut n = Box::into_raw(Box::new(Node {
            data: ManuallyDrop::new(t),
            next: AtomicPtr::default(),
        }));

        loop {
            let head = self.head.load(Relaxed);
            unsafe { (*n).next.store(head, Relaxed) };

            match self.head.compare_exchange(head, n, Release, Relaxed) {
                Ok(_) => break,
                Err(_) => break, //n = n,
            }
        }
    }

    /// Attempts to pop the top element from the stack.
    ///
    /// Returns `None` if the stack is empty.
    pub fn pop(&self) -> Option<T> {
        let hp: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();

        loop {
            let head = __VERIFIER_hp_protect(hp, &self.head); // Load Acquire + Protect the resulting address

            if head.is_null() {
                __VERIFIER_hp_free(hp);
                return None;
            }
            else {
                let next = unsafe { (*head).next.load(Relaxed) };

                if self
                    .head
                    .compare_exchange(head, next, Release, Relaxed)
                    .is_ok()
                {
                    let res = unsafe { Some(ManuallyDrop::into_inner(ptr::read( &(*head).data ))) };
                    __VERIFIER_hp_retire(head); // Drop once no other thread holds this reference (instead of defer_destroy)
                    __VERIFIER_hp_free(hp);
                    return res;
                }
            }
        }
    }

    /// Returns `true` if the stack is empty.
    pub fn is_empty(&self) -> bool {
        self.head.load(Acquire).is_null()
    }
}

impl<T> Drop for TreiberStack<T> {
    fn drop(&mut self) {
        while self.pop().is_some() {}
    }
}