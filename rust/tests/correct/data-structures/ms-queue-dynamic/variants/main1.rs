#[path = "../my_queue.rs"]
mod my_queue;
use my_queue::*;


fn thread_a(q: &Queue) { //Producer
    enqueue(q, 1);
}

fn thread_b(q: &Queue) { //Consumer
    let mut val: u32 = 0;
	while dequeue(q, (&mut val) as *mut u32) {
	}
}

fn thread_c(q: &Queue) { //Producer and Consumer
	enqueue(q, 1);

	let mut val: u32 = 0;
	while dequeue(q, (&mut val) as *mut u32) {
	}
}


pub fn main() {
	let q: Queue = empty_queue();
    init_queue(&q, 3);

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

	clear_queue(&q, 3);
}