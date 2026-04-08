//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_concurrent_operations
//LOOP_BOUND2
// 400s

#[path = "../seg_queue.rs"]
mod seg_queue;
use seg_queue::*;

fn producer(queue: &SegQueue<u32>) {
    queue.push(1);
    queue.push(2);
}

fn consumer(queue: &SegQueue<u32>) {
    let _ = queue.pop();
    let _ = queue.pop();
}

pub fn main() {
    let q: SegQueue<u32> = SegQueue::new();
    let producer1 = unsafe { std::thread::spawn_f_args(producer, &q) };
    let producer2 = unsafe { std::thread::spawn_f_args(producer, &q) };
    let consumer1 = unsafe { std::thread::spawn_f_args(consumer, &q) };
    let consumer2 = unsafe { std::thread::spawn_f_args(consumer, &q) };

    producer1.join().unwrap();
    producer2.join().unwrap();
    consumer1.join().unwrap();
    consumer2.join().unwrap();

    drop(q);
}