#![allow(dead_code)]

pub use std::*; //Re-export everything in std

pub mod thread;
pub mod sync;
pub mod genmc;
mod genmc_allocator;


pub const NULL: *const usize = 0x0usize as *const usize;