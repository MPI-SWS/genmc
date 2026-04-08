use std::{mem::MaybeUninit, sync::atomic::AtomicU32};
use std::thread::JoinHandle;

pub mod my_stack;
use self::my_stack::{MAX_THREADS, MyStack};

const MAXREADERS: usize = 3;
const MAXWRITERS: usize = 3;
const MAXRDWR:    usize = 3;

const DEFAULT_READERS: usize = 1;
const DEFAULT_WRITERS: usize = 2;

const DEFAULT_RDWR: usize = 0;

pub const READERS: usize = DEFAULT_READERS;
pub const WRITERS: usize = DEFAULT_WRITERS;
pub const RDWR: usize = DEFAULT_RDWR;

pub const NUM_THREADS: usize = READERS + WRITERS + RDWR;

pub static STACK: MyStack = MyStack::init_stack();
//pub static mut THREADS: MaybeUninit<[JoinHandle<()>; MAX_THREADS]> = unsafe { MaybeUninit::uninit() };
pub static mut PARAM: [usize; MAX_THREADS] = [0 ; MAX_THREADS];

pub static mut X: [AtomicU32; MAX_THREADS] = [AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0),
AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0),
AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0),
AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0), AtomicU32::new(0)];

#[allow(non_snake_case)]
pub fn threadW(pid: usize) {
    let _pid = pid as u32;
    unsafe {X.get(pid).expect("")}.store(_pid + 42, core::sync::atomic::Ordering::Relaxed);
    STACK.push(_pid);
}

#[allow(non_snake_case)]
pub fn threadR(pid: usize) {
    let idx = STACK.pop();
    if idx != 0 {
        let _b = unsafe{X.get(pid).expect("")}.load(core::sync::atomic::Ordering::Relaxed);
    }
}

#[allow(non_snake_case)]
pub fn threadRW(pid: usize) {
    let _pid = pid as u32;
    unsafe{X.get(pid).expect("")}.store(_pid + 42, core::sync::atomic::Ordering::Relaxed);
    STACK.push(_pid);

    let idx = STACK.pop();
    if idx != 0 {
        let _b = unsafe{X.get(pid).expect("")}.load(core::sync::atomic::Ordering::Relaxed);
    }
}


#[allow(non_snake_case)]
pub fn threadW0() {
    threadW(0usize);
}
#[allow(non_snake_case)]
pub fn threadW1() {
    threadW(1usize);
}
#[allow(non_snake_case)]
pub fn threadR2() {
    threadR(2usize);
}