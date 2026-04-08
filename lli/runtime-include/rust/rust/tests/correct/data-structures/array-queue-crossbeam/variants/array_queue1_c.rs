//Source: https://github.com/computersforpeace/model-checker-benchmarks/blob/master/mpmc-queue/mpmc-queue.cc
//LOOP_BOUND5
// Failing

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;

use std::sync::Arc;

fn thread_a(q: &ArrayQueue<u32>) { //Producer
	q.push(1).unwrap();
}

fn thread_b(q: &ArrayQueue<u32>) { //Consumer
	while q.pop().is_some() {
	}
}

fn thread_c(q: &ArrayQueue<u32>) { //Producer and Consumer
	q.push(1).unwrap();

	while q.pop().is_some() {
	}
}


pub fn main() {
	let q = ArrayQueue::new(5);
    let queue = Arc::new(q);

    let queue_clone = Arc::clone(&queue);
    let h1 = std::thread::spawn(move || {
        queue_clone.push(1).unwrap();
    });

    let queue_clone = Arc::clone(&queue);
    let h2 = std::thread::spawn(move || {
        while queue_clone.pop().is_some() {}
    });

    let queue_clone = Arc::clone(&queue);
    let h3 = std::thread::spawn(move || {
        queue_clone.push(1).unwrap();
        while queue_clone.pop().is_some() {}
    });

	h1.join().unwrap();
	h2.join().unwrap();
	h3.join().unwrap();
}