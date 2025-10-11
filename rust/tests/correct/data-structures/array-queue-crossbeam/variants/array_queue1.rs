//Source: https://github.com/computersforpeace/model-checker-benchmarks/blob/master/mpmc-queue/mpmc-queue.cc
//LOOP_BOUND5

#[path = "../array_queue.rs"]
mod array_queue;
use array_queue::*;

fn thread_a(q: &ArrayQueue<u32>) { //Producer
	q.push(1).unwrap();
}

fn thread_b(q: &ArrayQueue<u32>) { //Consumer
	let mut loop_counter = 0;
	while q.pop().is_some() {
		std::genmc::__VERIFIER_assume(loop_counter < 2);
		loop_counter += 1;
	}
}

fn thread_c(q: &ArrayQueue<u32>) { //Producer and Consumer
	q.push(1).unwrap();

	let mut loop_counter = 0;
	while q.pop().is_some() {
		std::genmc::__VERIFIER_assume(loop_counter < 2);
		loop_counter += 1;
	}
}


pub fn main() {
	let q = ArrayQueue::new(5);

	let h1 = unsafe { std::thread::spawn_f_args(thread_a, &q) };
	let h2 = unsafe { std::thread::spawn_f_args(thread_b, &q) };
	let h3 = unsafe { std::thread::spawn_f_args(thread_c, &q) };

	h1.join().unwrap();
	h2.join().unwrap();
	h3.join().unwrap();

	drop(q)
}