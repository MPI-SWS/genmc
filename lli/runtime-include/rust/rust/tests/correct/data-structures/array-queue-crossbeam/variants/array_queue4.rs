//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_fifo_ordering
//LOOP_BOUND5

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;


pub fn main() {
    let queue = ArrayQueue::new(100);

    // Enqueue elements
    for i in 0..100 {
        queue.push(i).unwrap();
    }

    // Verify FIFO order
    for i in 0..100 {
        assert_eq!(queue.pop(), Some(i));
    }
}