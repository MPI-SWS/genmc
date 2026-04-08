//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_memory_safety
// Failing

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;

use std::sync::Arc;
use std::thread::spawn;

pub fn main() {
    let queue = Arc::new(ArrayQueue::new(5));

    // Multiple threads accessing the same memory
    let handles: Vec<_> = (0..3)
        .map(|i| {
            let queue = Arc::clone(&queue);
            spawn(move || {
                assert!( queue.push(i).is_ok() );
                let _ = queue.pop();
            })
        })
        .collect();

    for handle in handles {
        handle.join().unwrap();
    }
}