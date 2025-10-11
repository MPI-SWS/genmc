#![allow(unused_imports)]

use std::{mem::MaybeUninit, sync::atomic::AtomicI32};
use std::thread::JoinHandle;

#[path = "../common.rs"]
mod common;
use common::{threadW, threadR, threadRW, NUM_THREADS, PARAM, WRITERS, READERS, RDWR, X, threadW0, threadW1, threadR2};
use common::my_stack::MAX_THREADS;


pub fn main() {
    let mut i = 0;

    let mut threads: [MaybeUninit<JoinHandle<()>>; MAX_THREADS] = std::array::from_fn(|_| MaybeUninit::uninit());

    //num_threads: Done statically

    //X[1].write(AtomicI32::new(0)); //Not needed => Static initialization
    //X[2].write(AtomicI32::new(0));

    //init stack: Done statically
    for j in 0..NUM_THREADS {
        unsafe {PARAM[j] = j};
    }
    for _ in 0..WRITERS {
        let pj = unsafe {PARAM[i]};
        threads[i].write(std::thread::spawn(move || {threadW(pj)}));
        i += 1;
    }
    for _ in 0..READERS {
        let pj = unsafe {PARAM[i]};
        threads[i].write(std::thread::spawn(move || {threadR(pj)}));
        i += 1;
    }
    for _ in 0..RDWR {
        let pj = unsafe {PARAM[i]};
        threads[i].write(std::thread::spawn(move || {threadRW(pj)}));
        i += 1;
    }

    for j in 0..NUM_THREADS {
        unsafe { threads[j].assume_init_mut().join_keep().unwrap() };
    }

    //let h1 = std::thread::spawn_f(threadW0);
    //let h2 = std::thread::spawn_f(threadW1);
    //let h3 = std::thread::spawn_f(threadR2);
    /*let h1 = std::thread::spawn_f_args(threadW, 0usize);
    let h2 = std::thread::spawn_f_args(threadW, 1usize);
    let h3 = std::thread::spawn_f_args(threadR, 2usize);
    h1.join().unwrap();
    h2.join().unwrap();
    h3.join().unwrap();*/

    /*std::thread::scope(|s| {
        s.spawn(|| {threadW(0usize)});
        s.spawn(|| {threadR(1usize)});
        //s.spawn(|| {threadR(2usize)});
        //let mut i = 0;
        /*for j in 0..WRITERS {
            let pj = unsafe {PARAM[i]};
            s.spawn(move || {threadW(pj)});
            i += 1;
        }
        for j in 0..READERS {
            let pj = unsafe {PARAM[i]};
            s.spawn(move || {threadR(pj)});
            i += 1;
        }*/
        /*for j in 0..RDWR {
            let pj = unsafe {PARAM[i]};
            s.spawn(move || {threadRW(pj)});
            i += 1;
        }*/
        6
    });*/
}