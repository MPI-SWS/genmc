use std::sync::Mutex;

pub static COUNTER: Mutex<usize> = Mutex::new(0);
pub const N: usize = 10;

fn thread_1() {
    for _ in 0..N {
        let lock = COUNTER.try_lock();

        if lock.is_ok() {
            panic!("Something went wrong");
        }
    }
}

// All try_locks by other threads should fail, as the lock is already held.
fn main() {
    let v = COUNTER.lock().unwrap(); //Acquire the mutex

    // No other thread should be successfull in try_lock
    let h1 = std::thread::spawn(|| thread_1());
    let h2 = std::thread::spawn(|| thread_1());
    let h3 = std::thread::spawn(|| thread_1());
    h1.join().unwrap();
    h2.join().unwrap();
    h3.join().unwrap();

    // Check the values
    assert!(*v == 0);
}