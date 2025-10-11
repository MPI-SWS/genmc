//Source: https://github.com/nidhugg/nidhugg/blob/master/tests/smoke/C-tests/treiber_pop.inc
// Just with 2 consumers instead of 4

#[path = "../treiber_stack_hp.rs"]
mod treiber_stack;
use treiber_stack::*;

pub fn consumer(s: &TreiberStack<u32>) {
    let _v = s.pop();
}

pub fn main() {
    let s: TreiberStack<u32> = TreiberStack::new();
    s.push(1);
    s.push(2);

    let h1 = unsafe { std::thread::spawn_f_args(consumer, &s) };
    let h2 = unsafe { std::thread::spawn_f_args(consumer, &s) };

    h1.join().unwrap();
    h2.join().unwrap();

    drop(s);
}