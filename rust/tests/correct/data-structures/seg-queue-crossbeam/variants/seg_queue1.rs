//Source: https://github.com/computersforpeace/model-checker-benchmarks/blob/master/mpmc-queue/mpmc-queue.cc
//LOOP_BOUND5

#[path = "../seg_queue.rs"]
mod seg_queue;
use seg_queue::*;

fn thread_a(q: &SegQueue<u32>) { //Producer
	q.push(1);
}

fn thread_b(q: &SegQueue<u32>) { //Consumer
	let mut loop_counter = 0;
	while q.pop().is_some() {
		std::genmc::__VERIFIER_assume(loop_counter < 2);
		loop_counter += 1;
	}
}

fn thread_c(q: &SegQueue<u32>) { //Producer and Consumer
	q.push(1);

	let mut loop_counter = 0;
	while q.pop().is_some() {
		std::genmc::__VERIFIER_assume(loop_counter < 2);
		loop_counter += 1;
	}
}


pub fn main() {
	let q: SegQueue<u32> = SegQueue::new();

	//Notice: We can pass &q directly here as ownership is
	//not moved, Rust thinks those functions don't do anything
	//Make sure q is used (alive after joining), such that it
	//doesn't get dropped too early.
	let h1 = unsafe { std::thread::spawn_f_args(thread_a, &q) };
	let h2 = unsafe { std::thread::spawn_f_args(thread_b, &q) };
	let h3 = unsafe { std::thread::spawn_f_args(thread_c, &q) };

	h1.join().unwrap();
	h2.join().unwrap();
	h3.join().unwrap();

	drop(q)
}