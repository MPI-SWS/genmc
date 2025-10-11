//Source: https://github.com/computersforpeace/model-checker-benchmarks/blob/master/mpmc-queue/mpmc-queue.cc
// Unfreed memory due to __VERIFIER_hp_free not being taken into account fully

#[path = "../ms_queue_hp.rs"]
mod ms_queue_hp;
use ms_queue_hp::*;

fn thread_a(q: &Queue<u32>) { //Producer
	q.push(1);
}

fn thread_b(q: &Queue<u32>) { //Consumer
	while q.try_pop().is_some() {
	}
}

fn thread_c(q: &Queue<u32>) { //Producer and Consumer
	q.push(1);

	while q.try_pop().is_some() {
	}
}


pub fn main() {
	let q: Queue<u32> = Queue::new();

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