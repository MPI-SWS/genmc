// Test to check scoped threads
fn main() {
    let mut a0 = 30;
    let mut a1 = 30;
    let b = vec![1, 2];

    std::thread::scope(|s| {
        s.spawn(|| {
            a0 += b[0];
        });

        s.spawn(|| {
            a1 += b[1];
        });
    });

    // Can still use a0, a1 and b here as we still have ownership
    // Check the values
    assert!(a0 == 31 && a1 == 32 && b[0] == 1 && b[1] == 2);
}