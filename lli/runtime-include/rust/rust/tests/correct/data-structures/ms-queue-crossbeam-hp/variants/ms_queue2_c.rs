//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_concurrent_operations
// Failing

#[path = "../ms_queue_hp.rs"]
mod ms_queue_hp;
use ms_queue_hp::*;

use std::sync::Arc;
use std::thread::spawn;

pub fn main() {
    let q: Queue<u32> = Queue::new();
    let queue = Arc::new(q);

    // Producer threads
    let queue_clone = Arc::clone(&queue);
    let producer1 = spawn(move || {
        queue_clone.push(1);
        queue_clone.push(2);
    });

    let queue_clone = Arc::clone(&queue);
    let producer2 = spawn(move || {
        queue_clone.push(3);
        queue_clone.push(4);
    });

    // Consumer threads
    let queue_clone = Arc::clone(&queue);
    let consumer1 = spawn(move || {
        let _ = queue_clone.try_pop();
        let _ = queue_clone.try_pop();
    });

    let queue_clone = Arc::clone(&queue);
    let consumer2 = spawn(move || {
        let _ = queue_clone.try_pop();
        let _ = queue_clone.try_pop();
    });

    producer1.join().unwrap();
    producer2.join().unwrap();
    consumer1.join().unwrap();
    consumer2.join().unwrap();
}