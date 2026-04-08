///!Simple Rust-Implementation of Treiber-Stack tests given in treiber-stack-dynamic for C.
use std::{mem::MaybeUninit, ptr, sync::atomic::{AtomicPtr, Ordering}};
use std::{alloc::{alloc, Layout}, ffi::c_void, genmc::{__VERIFIER_hp_alloc, __VERIFIER_hp_protect, __VERIFIER_hp_retire, __VERIFIER_hp_free, __VERIFIER_hp_t}};
use std::sync::atomic::Ordering::*;

pub const MAX_NODES : usize = 0xff;
pub const MAX_THREADS: usize = 32;

#[repr(C)]
pub struct Node {
    pub value: u32,
    pub next: AtomicPtr<Node>
}

#[repr(C)]
pub struct MyStack {
    pub top: AtomicPtr<Node>,
    //nodes: MaybeUninit<[Node; MAX_NODES + 1]> => not used..
}

impl Node {
    pub fn new() -> *mut Node {
        let ptr = unsafe { alloc(Layout::new::<Node>()) };
        ptr as *mut Node
    }

    pub fn reclaim(node: *mut Node) {
        __VERIFIER_hp_retire(node);
    }
}

impl MyStack {
    pub const fn init_stack() -> MyStack {
        //let ns: [MaybeUninit<Node>; MAX_NODES + 1] = unsafe { MaybeUninit::uninit().assume_init() }; //Important!
        let m = MyStack {
            top: AtomicPtr::new(ptr::null_mut()),
            //nodes: ns
        };
        //m.top.store(ptr::null_mut(), SeqCst);

        m
    }


    pub fn push(&self, val: u32) {
        let node: *mut Node = Node::new();
        unsafe { (*node).value = val };

        loop {
            let top: *mut Node = self.top.load(Acquire);
            unsafe { (*node).next.store(top, Relaxed) };
            if self.top.compare_exchange(top, node, Release, Relaxed).is_ok() {
                break;
            }
        }
    }

    pub fn pop(&self) -> u32 {
        let mut top_: *mut Node = ptr::null_mut();
        let mut val: u32 = 0;

        let hp: *mut __VERIFIER_hp_t = __VERIFIER_hp_alloc();
        loop {
            top_ = __VERIFIER_hp_protect(hp, &(self.top));
            if top_.is_null() {
                __VERIFIER_hp_free(hp);
                return 0;
            }

            let next: *mut Node = unsafe { (*top_).next.load(Ordering::Relaxed) };
            if self.top.compare_exchange(top_, next, Ordering::Release, Ordering::Relaxed).is_ok() {
                break;
            }
        }
        val = unsafe { (*top_).value } ;
        /* Reclaim the used slot */
        Node::reclaim(top_);
        __VERIFIER_hp_free(hp);
        val
    }
}