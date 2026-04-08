//Same test as in treiber-stack-dynamic
// SegFault

use std::sync::atomic::AtomicU32;

#[path = "../treiber_stack_hp.rs"]
mod treiber_stack;
use treiber_stack::*;

pub const MAX_NODES : usize = 0xff;
pub const MAX_THREADS: usize = 32;


pub fn thread_w(pid: usize, stack: std::sync::Arc<TreiberStack<u32>>, x: std::sync::Arc<[AtomicU32 ; MAX_THREADS]>) {
    let _pid = pid as u32;
    x[pid].store(_pid + 42, std::sync::atomic::Ordering::Relaxed);
    stack.push(_pid);
}

pub fn thread_r(pid: usize, stack: std::sync::Arc<TreiberStack<u32>>, x: std::sync::Arc<[AtomicU32 ; MAX_THREADS]>) {
    let idx = stack.pop();
    if idx.is_some() {
        let _b = x[pid].load(core::sync::atomic::Ordering::Relaxed);
    }
}

pub fn main() {
    let s: TreiberStack<u32> = TreiberStack::new();
    let x: [AtomicU32; MAX_THREADS] = std::array::from_fn(|_| AtomicU32::new(0));

    let s_1 = std::sync::Arc::new(s);
    let s_2 = s_1.clone();
    let s_3 = s_1.clone();

    let x_1 = std::sync::Arc::new(x);
    let x_2 = x_1.clone();
    let x_3 = x_1.clone();


    let h1 = std::thread::spawn(|| thread_w(0usize, s_1, x_1));
    let h2 = std::thread::spawn(|| thread_w(1usize, s_2, x_2));
    let h3 = std::thread::spawn(|| thread_r(2usize, s_3, x_3));
    h1.join().unwrap();
    h2.join().unwrap();
    h3.join().unwrap();

}