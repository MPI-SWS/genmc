use std::sync::Mutex;

pub static COUNTER: Mutex<i64> = Mutex::new(0);

// Check for locking and automatic unlocking multiple times
fn main() {
    let mut i = 0;
    loop {
        if i == 10 {
            break;
        }

        let mut v = COUNTER.lock().unwrap();
        *v += 1;

        i += 1;
    }

    //Check the values
    let v = COUNTER.lock().unwrap();
    assert!(*v == 10);
}