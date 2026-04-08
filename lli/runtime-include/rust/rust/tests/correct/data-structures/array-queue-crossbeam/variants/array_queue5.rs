//Source: https://github.com/tokio-rs/tokio/blob/master/tokio/src/sync/tests/loom_mpsc.rs
//len_nonzero_after_send & nonempty_after_send
//LOOP_BOUND5

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;

fn thread(queue: &ArrayQueue<i32>) {
    queue.push(0);
}

pub fn main() {
    let queue = ArrayQueue::new(2);

    let handle = unsafe { std::thread::spawn_f_args(thread, &queue) };
    queue.push(1);

    assert!(!queue.is_empty());
    assert!(queue.len() != 0);

    handle.join().unwrap();
}