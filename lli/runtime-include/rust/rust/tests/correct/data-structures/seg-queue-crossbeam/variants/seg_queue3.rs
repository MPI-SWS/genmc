//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_memory_safety
//LOOP_BOUND2
// 250s

#[path = "../seg_queue.rs"]
mod seg_queue;
use seg_queue::*;

use std::sync::Arc;
use std::thread::spawn;

pub fn main() {
    let queue = Arc::new(SegQueue::new());

    // Multiple threads accessing the same memory
    let handles: Vec<_> = (0..3)
        .map(|i| {
            let queue = Arc::clone(&queue);
            spawn(move || {
                queue.push(i);
                let _ = queue.pop();
            })
        })
        .collect();

    for handle in handles {
        handle.join().unwrap();
    }
}