//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_concurrent_operations
//LOOP_BOUND1 (800s)
// Failing

#[path = "../seg_queue.rs"]
mod seg_queue;
use seg_queue::*;

use std::sync::Arc;
use std::thread::spawn;

pub fn main() {
    let q: SegQueue<u32> = SegQueue::new();
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
        let _ = queue_clone.pop();
        let _ = queue_clone.pop();
    });

    let queue_clone = Arc::clone(&queue);
    let consumer2 = spawn(move || {
        let _ = queue_clone.pop();
        let _ = queue_clone.pop();
    });

    producer1.join().unwrap();
    producer2.join().unwrap();
    consumer1.join().unwrap();
    consumer2.join().unwrap();
}