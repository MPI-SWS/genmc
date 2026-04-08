use std::sync::Mutex;

pub static COUNTER: Mutex<i64> = Mutex::new(0);

// Check to see if values are set correctly.
fn main() {
    { // Set the value
        let mut v = COUNTER.lock().unwrap();
        *v += 100;
    } // Should unlock here automatically

    { // Check the value, throw an error if it's wrong
        let mut v = COUNTER.lock().unwrap();
        assert!(*v == 100);
    }
}