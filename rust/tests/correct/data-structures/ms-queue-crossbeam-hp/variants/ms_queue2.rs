//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_concurrent_operations
// 450s

#[path = "../ms_queue_hp.rs"]
mod ms_queue_hp;
use ms_queue_hp::*;

fn producer(queue: &Queue<u32>) {
    queue.push(1);
    queue.push(2);
}

fn consumer(queue: &Queue<u32>) {
    let _ = queue.try_pop();
    let _ = queue.try_pop();
}

pub fn main() {
    let q: Queue<u32> = Queue::new();
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