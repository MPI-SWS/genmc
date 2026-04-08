//! NOTE: Removed Cache Padding, using custom Garbage Collector
//!
//! Michael-Scott lock-free queue.
//!
//! Usable with any number of producers and consumers.
//!
//! Michael and Scott.  Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue
//! Algorithms.  PODC 1996.  <http://dl.acm.org/citation.cfm?id=248106>
//!
//! Simon Doherty, Lindsay Groves, Victor Luchangco, and Mark Moir. 2004b. Formal Verification of a
//! Practical Lock-Free Queue Algorithm. <https://doi.org/10.1007/978-3-540-30232-2_7>

use std::mem::MaybeUninit;
use std::sync::atomic::Ordering::{Acquire, Relaxed, Release};

use std::sync::atomic::AtomicPtr;
use std::genmc::{__VERIFIER_hp_retire, __VERIFIER_hp_t, __VERIFIER_hp_alloc, __VERIFIER_hp_free, __VERIFIER_hp_protect};


// The representation here is a singly-linked list, with a sentinel node at the front. In general
// the `tail` pointer may lag behind the actual tail. Non-sentinel nodes are either all `Data` or
// all `Blocked` (requests for data from blocked threads).
#[derive(Debug)]
pub struct Queue<T> {
    head: AtomicPtr<Node<T>>,
    tail: AtomicPtr<Node<T>>,
}

pub struct Node<T> {
    /// The slot in which a value of type `T` can be stored.
    ///
    /// The type of `data` is `MaybeUninit<T>` because a `Node<T>` doesn't always contain a `T`.
    /// For example, the sentinel node in a queue never contains a value: its slot is always empty.
    /// Other nodes start their life with a push operation and contain a value until it gets popped
    /// out. After that such empty nodes get added to the collector for destruction.
    data: MaybeUninit<T>,

    next: AtomicPtr<Node<T>>,
}

// Any particular `T` should never be accessed concurrently, so no need for `Sync`.
unsafe impl<T: Send> Sync for Queue<T> {}
unsafe impl<T: Send> Send for Queue<T> {}

impl<T> Queue<T> {
    /// Create a new, empty queue.
    pub fn new() -> Self {
        let q = Self {
            head: AtomicPtr::default(),
            tail: AtomicPtr::default(),
        };
        let sentinel_b = Box::new(Node {
            data: MaybeUninit::uninit(),
            next: AtomicPtr::default(),
        });
        let sentinel = Box::into_raw(sentinel_b);
        q.head.store(sentinel, Relaxed);
        q.tail.store(sentinel, Relaxed);
        q
    }

    /// Attempts to atomically place `n` into the `next` pointer of `onto`, and returns `true` on
    /// success. The queue's `tail` pointer may be updated.
    #[inline(always)]
    fn push_internal(
        &self,
        onto: *mut Node<T>,
        new: *mut Node<T>,
    ) -> bool {
        // is `onto` the actual tail?
        let next = unsafe { (*onto).next.load(Acquire) };
        if !next.is_null() {
            // if not, try to "help" by moving the tail pointer forward
            let _ = self
                .tail
                .compare_exchange(onto, next, Release, Relaxed);
            false
        } else {
            // looks like the actual tail; attempt to link in `n`
            let result = unsafe { (*onto)
                .next
                .compare_exchange(std::ptr::null_mut(), new, Release, Relaxed)
                .is_ok() };
            if result {
                // try to move the tail pointer forward
                let _ = self
                    .tail
                    .compare_exchange(onto, new, Release, Relaxed);
            }
            result
        }
    }

    /// Adds `t` to the back of the queue, possibly waking up threads blocked on `pop`.
    pub fn push(&self, t: T) {
        let new = Box::into_raw(Box::new(Node {
            data: MaybeUninit::new(t),
            next: AtomicPtr::default(),
        }));

        let hp: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
        loop {
            // We push onto the tail, so we'll start optimistically by looking there first.
            //let tail = self.tail.load(Acquire);
            let tail = __VERIFIER_hp_protect(hp, &self.tail); // Load Acquire + protect the resulting address

            // Attempt to push onto the `tail` snapshot; fails if `tail.next` has changed.
            if self.push_internal(tail, new) {
                break;
            }
        }
        __VERIFIER_hp_free(hp);
    }

    /// Attempts to pop a data node. `Ok(None)` if queue is empty; `Err(())` if lost race to pop.
    #[inline(always)]
    fn pop_internal(&self, hp_head: *mut __VERIFIER_hp_t, hp_next: *mut __VERIFIER_hp_t) -> Result<Option<T>, ()> {
        //let head = self.head.load(Acquire);
        //let h = unsafe { head };
        let h = __VERIFIER_hp_protect(hp_head, &self.head); // Load Acquire + protect the resulting address
        //let next = unsafe { (*h).next.load(Acquire) };
        let next = __VERIFIER_hp_protect(hp_next, unsafe { &(*h).next }); // Load Acquire + protect the resulting address
            // => Should not be freed by another thread also popping before we read the result
        if next.is_null() {
            Ok(None)
        }
        else {
            self.head
                    .compare_exchange(h, next, Release, Relaxed)
                    .map(|_| {
                        let tail = self.tail.load(Relaxed);
                        // Advance the tail so that we don't retire a pointer to a reachable node.
                        if h == tail {
                            let _ = self
                                .tail
                                .compare_exchange(tail, next, Release, Relaxed);
                        }
                        //guard.defer_destroy(head);
                        __VERIFIER_hp_retire(h); // Destroy head once no other thread has a reference to it
                        Some(unsafe { (*next).data.assume_init_read() })
                    })
                    .map_err(|_| ())
        }
    }

    /// Attempts to pop a data node, if the data satisfies the given condition. `Ok(None)` if queue
    /// is empty or the data does not satisfy the condition; `Err(())` if lost race to pop.
    #[inline(always)]
    fn pop_if_internal<F>(&self, condition: F, hp: *mut __VERIFIER_hp_t) -> Result<Option<T>, ()>
    where
        T: Sync,
        F: Fn(&T) -> bool,
    {
        //let head = self.head.load(Acquire);
        //let h = unsafe { head };
        let h = __VERIFIER_hp_protect(hp, &self.head); // Load Acquire + protect the resulting address
        let next = unsafe { (*h).next.load(Acquire) };
        if next.is_null() {
            Ok(None)
        }
        else {
            if condition(unsafe { &(*(*next).data.as_ptr()) }) {
                return self.head
                    .compare_exchange(h, next, Release, Relaxed)
                    .map(|_| {
                        let tail = self.tail.load(Relaxed);
                        // Advance the tail so that we don't retire a pointer to a reachable node.
                        if h == tail {
                            let _ = self
                                .tail
                                .compare_exchange(tail, next, Release, Relaxed);
                        }
                        //guard.defer_destroy(head);
                        __VERIFIER_hp_retire(h); // Destroy head once no other thread has a reference to it
                        Some(unsafe { (*next).data.assume_init_read() })
                    })
                    .map_err(|_| ());
            }
            Ok(None)
        }
    }

    /// Attempts to dequeue from the front.
    ///
    /// Returns `None` if the queue is observed to be empty.
    pub fn try_pop(&self) -> Option<T> {
        let hp_head: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
        let hp_next: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
        loop {
            if let Ok(head) = self.pop_internal(hp_head, hp_next) {
                __VERIFIER_hp_free(hp_head);
                __VERIFIER_hp_free(hp_next);
                return head;
            }
        }
    }

    /// Attempts to dequeue from the front, if the item satisfies the given condition.
    ///
    /// Returns `None` if the queue is observed to be empty, or the head does not satisfy the given
    /// condition.
    pub fn try_pop_if<F>(&self, condition: F) -> Option<T>
    where
        T: Sync,
        F: Fn(&T) -> bool,
    {
        let hp: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
        loop {
            if let Ok(head) = self.pop_if_internal(&condition, hp) {
                return head;
            }
        }
    }
}

impl<T> Drop for Queue<T> {
    fn drop(&mut self) {
        while self.try_pop().is_some() {}

        // Destroy the remaining sentinel node.
        let sentinel = self.head.load(Relaxed);
        drop(unsafe { Box::from_raw(sentinel) });
    }
}
