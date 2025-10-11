#[path = "../common.rs"]
mod common;
pub use common::*;

use std::convert::TryInto;

pub fn main() {
    let _i: i32 = 0;
    let _in_sum: u32 = 0;
    let _out_sum: u32 = 0;

    let num_threads = READERS + WRITERS + RDWR;
    unsafe { init_queue(&MY_QUEUE, num_threads.try_into().unwrap()) };

    let mut param = 0;
    for i in 0..WRITERS {
        let _h = unsafe { std::thread::spawn_f_args(threadW, param + i) };
    }
    //(0..WRITERS).map(|i| unsafe { std::thread::spawn_f_args(threadW, param + i) });

    param += WRITERS;
    for i in 0..READERS {
        let _h = unsafe { std::thread::spawn_f_args(threadR, param + i) };
    }
    //(0..READERS).map(|i| unsafe { std::thread::spawn_f_args(threadR, param + i) });

    param += READERS;
    for i in 0..RDWR {
        let _h = unsafe { std::thread::spawn_f_args(threadRW, param + i) };
    }
    //(0..READERS).map(|i| unsafe { std::thread::spawn_f_args(threadRW, param + i) });
}