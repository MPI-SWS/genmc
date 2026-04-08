//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_fifo_ordering
//LOOP_BOUND5

#[path = "../seg_queue.rs"]
mod seg_queue;
use seg_queue::*;

pub fn main() {
    let queue = SegQueue::new();

    // Enqueue elements
    for i in 0..100 {
        queue.push(i);
    }

    // Verify FIFO order
    for i in 0..100 {
        assert_eq!(queue.pop(), Some(i));
    }
}