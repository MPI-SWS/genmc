//Source: https://github.com/nidhugg/nidhugg/blob/master/tests/smoke/C-tests/treiber_pop.inc

#[path = "../treiber_stack_hp.rs"]
mod treiber_stack;
use treiber_stack::*;


pub fn thread(stack: &TreiberStack<u32>) {
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

    let h1 = unsafe { std::thread::spawn_f_args(thread, &s) };
    let h2 = unsafe { std::thread::spawn_f_args(thread, &s) };
    let h3 = unsafe { std::thread::spawn_f_args(thread, &s) };
    let h4 = unsafe { std::thread::spawn_f_args(thread, &s) };

    h1.join().unwrap();
    h2.join().unwrap();
    h3.join().unwrap();
    h4.join().unwrap();

    drop(s);
}