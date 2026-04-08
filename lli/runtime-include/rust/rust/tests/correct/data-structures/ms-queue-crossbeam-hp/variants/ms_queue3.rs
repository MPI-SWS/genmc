//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_memory_safety
// Failing

#[path = "../ms_queue_hp.rs"]
mod ms_queue_hp;
use ms_queue_hp::*;

use std::sync::Arc;
use std::thread::spawn;

pub fn main() {
    let queue = Arc::new(Queue::new());

    // Multiple threads accessing the same memory
    let handles: Vec<_> = (0..3)
        .map(|i| {
            let queue = Arc::clone(&queue);
            spawn(move || {
                queue.push(i);
                let _ = queue.try_pop();
            })
        })
        .collect();

    for handle in handles {
        handle.join().unwrap();
    }
}