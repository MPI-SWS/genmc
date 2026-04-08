use std::sync::Mutex;

pub static COUNTER: Mutex<i64> = Mutex::new(0);
pub const N: usize = 10;

// Test with multiple try-locks
fn main() {
    let mut success = 0usize;
    let mut failure = 0usize;

    for _ in 0..N {
        let mut lock = COUNTER.try_lock();
        if let Ok(ref mut mutex) = lock {
            **mutex += 1;
            success += 1;
        } else {
            failure += 1;
        }
    }

    // Check the values
    let v = COUNTER.lock().unwrap();
    assert!(success == 10 && failure == 0 && *v == 10);
}