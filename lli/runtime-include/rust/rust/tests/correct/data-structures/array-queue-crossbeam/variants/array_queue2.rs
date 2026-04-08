//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_concurrent_operations
//LOOP_BOUND2 (1000s)

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;


fn producer(queue: &ArrayQueue<u32>) {
    queue.push(1).unwrap();
    queue.push(2).unwrap();
}

fn consumer(queue: &ArrayQueue<u32>) {
    let _ = queue.pop();
    let _ = queue.pop();
}

pub fn main() {
    let q: ArrayQueue<u32> = ArrayQueue::new(5);
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