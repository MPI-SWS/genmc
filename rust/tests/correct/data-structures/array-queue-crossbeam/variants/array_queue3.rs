//Source: https://github.com/sujalsin/concurrent-verification/blob/main/src/main.rs
//test_memory_safety
//LOOP_BOUND4

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;

fn thread(args: (&ArrayQueue<i32>, i32)) {
    let (queue, id) = args;
    assert!( queue.push(id).is_ok() );
    let _ = queue.pop();
}

pub fn main() {
    let queue = ArrayQueue::new(5);

    let handles: Vec<_> = (0..3)
        .map(|i| {
            unsafe { std::thread::spawn_f_args(thread, (&queue, i)) }
        }).collect();

    for handle in handles {
        handle.join().unwrap();
    }
}