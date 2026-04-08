pub mod my_queue;
pub use my_queue::*;

pub const MAX_THREADS: usize = 32;
pub const DEFAULT_READERS: u32 = 0;
pub const DEFAULT_WRITERS: u32 = 2;
pub const DEFAULT_RDWR: u32 = 0;

pub const READERS: u32 = DEFAULT_READERS;
pub const WRITERS: u32 = DEFAULT_WRITERS;
pub const RDWR: u32 = DEFAULT_RDWR;


pub static mut MY_QUEUE: Queue = empty_queue();
pub static mut PARAM: [u32; MAX_THREADS] = [0; MAX_THREADS];
pub static mut INPUT: [u32; MAX_THREADS] = [0; MAX_THREADS];
pub static mut OUTPUT: [u32; MAX_THREADS] = [0; MAX_THREADS];

pub static mut TID: usize = 0;

pub fn set_thread_num(i: usize) {
    // We don't yet have thread local storage in Rust
    //unsafe { TID = i };
}

pub fn get_thread_num() -> usize {
    return unsafe { TID };
}

pub static mut SUCC: [bool; MAX_THREADS] = [false; MAX_THREADS];

pub fn threadW(param: u32) {
    let _val: u32 = 0;
    let pid: usize = param as usize;

    set_thread_num(pid);

    unsafe { INPUT[pid] = param * 10 };
    unsafe { enqueue(&MY_QUEUE, INPUT[pid]) };
}

pub fn threadR(param: u32) {
    let _val: u32 = 0;
    let pid: usize = param as usize;

    set_thread_num(pid);

    unsafe { SUCC[pid] = dequeue(&MY_QUEUE, &mut (OUTPUT[pid])) };
}

pub fn threadRW(param: u32) {
    let _val: u32 = 0;
    let pid: usize = param as usize;

    set_thread_num(pid);

    unsafe { INPUT[pid] = param * 10 };
    unsafe { enqueue(&MY_QUEUE, INPUT[pid]) };
    unsafe { SUCC[pid] = dequeue(&MY_QUEUE, &mut (OUTPUT[pid])) };
}