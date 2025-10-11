//Same test as in treiber-stack-dynamic

use std::sync::atomic::AtomicU32;

#[path = "../treiber_stack_hp.rs"]
mod treiber_stack;
use treiber_stack::*;

pub const MAX_NODES : usize = 0xff;
pub const MAX_THREADS: usize = 32;


pub fn thread_w(args: (usize, &TreiberStack<u32>, &[AtomicU32 ; MAX_THREADS])) {
    let (pid, stack, x) = args;
    let _pid = pid as u32;
    x[pid].store(_pid + 42, std::sync::atomic::Ordering::Relaxed);
    stack.push(_pid);
}

pub fn thread_r(args: (usize, &TreiberStack<u32>, &[AtomicU32 ; MAX_THREADS])) {
    let (pid, stack, x) = args;
    let idx = stack.pop();
    if idx.is_some() {
        let _b = x[pid].load(core::sync::atomic::Ordering::Relaxed);
    }
}

pub fn main() {
    let s: TreiberStack<u32> = TreiberStack::new();
    let x: [AtomicU32; MAX_THREADS] = std::array::from_fn(|_| AtomicU32::new(0));
    let h1 = unsafe { std::thread::spawn_f_args(thread_w, (0usize, &s, &x)) };
    let h2 = unsafe { std::thread::spawn_f_args(thread_w, (1usize, &s, &x)) };
    let h3 = unsafe { std::thread::spawn_f_args(thread_r, (2usize, &s, &x)) };
    h1.join().unwrap();
    h2.join().unwrap();
    h3.join().unwrap();

}