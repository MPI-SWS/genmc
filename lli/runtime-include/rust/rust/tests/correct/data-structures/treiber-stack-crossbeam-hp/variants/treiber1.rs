//Source: https://github.com/nidhugg/nidhugg/blob/master/tests/smoke/C-tests/treiber_pop.inc
// SegFault

#[path = "../treiber_stack_hp.rs"]
mod treiber_stack;
use treiber_stack::*;


pub fn thread(stack: std::sync::Arc<TreiberStack<u32>>) {
    let v = stack.pop();
    if v.is_some() {
        let v = v.unwrap();
        assert!(v >= 0 && v < 5);
    }
}


pub fn main() {
    let s: TreiberStack<u32> = TreiberStack::new();
    s.push(0);
    s.push(1);
    s.push(2);
    s.push(3);

    let s_1 = std::sync::Arc::new(s);
    let s_2 = s_1.clone();
    let s_3 = s_1.clone();
    let s_4 = s_1.clone();

    let h1 = std::thread::spawn(|| thread(s_1));
    let h2 = std::thread::spawn(|| thread(s_2));
    let h3 = std::thread::spawn(|| thread(s_3));
    let h4 = std::thread::spawn(|| thread(s_4));

    h1.join().unwrap();
    h2.join().unwrap();
    h3.join().unwrap();
    h4.join().unwrap();
}