//Source: https://github.com/computersforpeace/model-checker-benchmarks/blob/master/ms-queue/main.c
//LOOP_BOUND5

#[path = "../seg_queue.rs"]
mod seg_queue;
use seg_queue::*;

pub static mut INPUT: [u32; 2] = [0, 0];
pub static mut OUTPUT: [u32; 2] = [0, 0];


fn main_task(pid: usize, QUEUE: std::sync::Arc<SegQueue<u32>>) {
	unsafe {
		if pid == 1 {
			INPUT[0] = 17;
			QUEUE.push(INPUT[0]);
			match QUEUE.pop() {
				Some(v) => {OUTPUT[0] = v;},
				None => {OUTPUT[0] = 0;}
			}
		} else {
			INPUT[1] = 37;
			QUEUE.push(INPUT[1]);
			match QUEUE.pop() {
				Some(v) => {OUTPUT[1] = v;},
				None => {OUTPUT[1] = 0;}
			}
		}
	}
}


pub fn main() {
	let q: SegQueue<u32> = SegQueue::new();

	unsafe { //Otherwise: getLocInitVal Error
		INPUT[0] = 0;
		INPUT[1] = 1;
		OUTPUT[0] = 0;
		OUTPUT[1] = 1;
	}

	let q_1 = std::sync::Arc::new(q);
	let q_2 = q_1.clone();
	let h1 = std::thread::spawn(move || main_task(0usize, q_1));
	let h2 = std::thread::spawn(move || main_task(1usize, q_2));

	h1.join().unwrap();
	h2.join().unwrap();


	unsafe {
		let in_sum = INPUT[0] + INPUT[1];
		let out_sum = OUTPUT[0] + OUTPUT[1];
		assert!(in_sum == out_sum);
	}
}