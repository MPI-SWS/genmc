//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_fifo_ordering

#[path = "../ms_queue_hp.rs"]
mod ms_queue_hp;
use ms_queue_hp::*;

pub fn main() {
    let queue = Queue::new();

    // Enqueue elements
    for i in 0..100 {
        queue.push(i);
    }

    // Verify FIFO order
    for i in 0..100 {
        assert_eq!(queue.try_pop(), Some(i));
    }
}