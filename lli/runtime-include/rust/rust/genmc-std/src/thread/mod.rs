//! This file provides implementations for spawning and joining
//! threads using GenMC. The implementation is strongly modelled
//! after the implementation in Rusts STD. Spawning / Joining
//! fullfills the same type requirements as in Rusts STD, thus
//! this can be easily swapped in for checking preexisting code.
//! Yielding and Sleeping are no-ops in this implementation.
//!
//! It includes a light-weight alternatives for spawning threads
//! using functions instead of closures: spawn_f & spawn_f_args.
//!
//! Some additional, non-STD helper functions were added for JoinHandle:
//! - Join Non-Consuming
//! - Join by Thread ID
//! - Get Thread ID
//! - Create a Dummy-JoinHandle
pub use std::thread::*; //Re-export everything
pub mod scoped;
pub use scoped::{scope};
pub mod genmc;

use std::{marker::PhantomData, mem::{ManuallyDrop, MaybeUninit}, time::Duration};
use genmc::*;


pub struct JoinHandle<T>(JoinInner<T>);
struct JoinInner<T> {
    native: ThreadNative,
    ret_type: PhantomData<T>
}
unsafe impl<T> Send for JoinHandle<T> {}
unsafe impl<T> Sync for JoinHandle<T> {}

impl<T> JoinInner<T> {
    pub fn join(self) -> Result<T> {
        unsafe { self.join_keep() }
    }

    /// Custom (non-std) function: Join non-consuming
    pub unsafe fn join_keep(&self) -> Result<T> {
        let t_id: u64 = self.native.id;

        let retval: *mut T = __VERIFIER_join(t_id) as *mut T; // Get the pointer to the return-value
        // Using a Box here allows for threads returning non-Sized elements,
        // like the unit type for threads returning nothing: ().
        // These 0-byte types get instantiated by Box without reading from memory.
        let val = Box::from_raw(retval);
        std::thread::Result::Ok(*val)
    }
}

impl<T> JoinHandle<T> {
    /// Joins the Thread represented by this JoinHandle.
    /// It retrieves a pointer to the the return value of the thread
    /// and returns the value.
    pub fn join(self) -> Result<T> {
        self.0.join()
    }

    /// Custom (non_std) function: Join non-consuming
    pub unsafe fn join_keep(&self) -> Result<T> {
        self.0.join_keep()
    }

    /// Custom (non_std) function: Join by tid
    pub unsafe fn join_tid(t_id: u64) -> Result<T> {
        let inner = JoinInner {
            native: ThreadNative { id: t_id },
            ret_type: PhantomData::<T>
        };
        inner.join()
    }

    /// Custom (non_std) function: Get the tid
    pub unsafe fn get_tid(&self) -> u64 {
        self.0.native.id
    }

    /// Custom (non_std) function: Get a dummy JoinHandle
    pub const unsafe fn dummy() -> JoinHandle<T> {
        JoinHandle(JoinInner { native: ThreadNative { id: std::u64::MAX }, ret_type: PhantomData::<T> })
    }
}


// Pass `f` in `MaybeUninit` because actually that closure might *run longer than the lifetime of `F`*.
// See <https://github.com/rust-lang/rust/issues/101983> for more details.
// To prevent leaks we use a wrapper that drops its contents.
#[repr(transparent)]
struct MaybeDangling<T>(MaybeUninit<T>);
impl<T> MaybeDangling<T> {
    fn new(x: T) -> Self {
        MaybeDangling(MaybeUninit::new(x))
    }
    fn into_inner(self) -> T {
        // Make sure we don't drop.
        let this = ManuallyDrop::new(self);
        // SAFETY: we are always initialized.
        unsafe { this.0.assume_init_read() }
    }
}
impl<T> Drop for MaybeDangling<T> {
    //Drop will be called by Box after this value is consumed (i.e. f is called)
    fn drop(&mut self) {
        // SAFETY: we are always initialized.
        unsafe { self.0.assume_init_drop() };
    }
}

pub fn spawn<F, T>(f: F) -> JoinHandle<T>
where
    F: FnOnce() -> T,
    F: Send + 'static,
    T: Send + 'static,
{
    let f = MaybeDangling::new(f);
    let main = move || -> *mut u8 {
        let f = f.into_inner();

        let try_res = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
            f()
        }));
        let res = try_res.expect("Something went wrong here");

        // Execute f, allocate the return value on the heap
        let ret = Box::into_raw(Box::new(res)) as *mut u8;

        // Return the pointer to the return value of f to thread_start.
        // Can't __VERIFIER_exit here as it would cause memory-leakage.
        ret
    };

    let main = Box::new(main);
    // Lifetime change: The caller must guarantee that the passed-in lifetime is long enough
    // for the lifetime of the thread.
    let main =
        unsafe { Box::from_raw(Box::into_raw(main) as *mut (dyn FnOnce() -> *mut u8 + Send + 'static)) };

    JoinHandle::<T>(JoinInner::<T> {
        native: unsafe { ThreadNative::new(main) },
        ret_type: PhantomData::<T>
    })
}


/// Lightweight alternative to using closures. Runs a function
/// without any arguments in a new thread. See spawn_f_args for
/// more details and for passing arguments.
///
/// Example use: Concurrent access on static items
/// ```
/// static QUEUE: Queue<u32> = Queue::new::<u32>();
///
/// fn my_thread() {
///	    QUEUE.push(1);
/// }
///
/// pub fn main() {
///	    let h1 = unsafe { std::thread::spawn_f(my_thread) };
///	    let h2 = unsafe { std::thread::spawn_f(my_thread) };
///	    let h3 = unsafe { std::thread::spawn_f(my_thread) };
///
///	    h1.join().unwrap();
///	    h2.join().unwrap();
///	    h3.join().unwrap();
///
/// }
/// ```
pub unsafe fn spawn_f<T>(f: fn() -> T) -> JoinHandle<T>
where T: Send + 'static {
    unsafe {
        JoinHandle::<T>(JoinInner::<T> {
            native: ThreadNative::new_f::<T>(f),
            ret_type: PhantomData::<T>
        })
    }
}

/// Lightweight alternative to using closures. Runs a function
/// in a new thread. This non-STD interface does not take ownership
/// like a closure does and this aspect must be handled carefully
/// to avoid dropping prematurely. See an example:
///
/// ```
/// fn my_thread(q: &Queue<u32>) {
///	    q.push(1);
/// }
///
/// pub fn main() {
///	    let q: Queue<u32> = Queue::new();
///
///	    //Notice: We can pass &q directly here as ownership is not moved
///	    //Make sure q is used (alive after joining), such that it
///	    //doesn't get dropped too early.
///	    let h1 = unsafe { std::thread::spawn_f_args(my_thread, &q) };
///	    let h2 = unsafe { std::thread::spawn_f_args(my_thread, &q) };
///	    let h3 = unsafe { std::thread::spawn_f_args(my_thread, &q) };
///
///	    h1.join().unwrap();
///	    h2.join().unwrap();
///	    h3.join().unwrap();
///
///	    drop(q)
/// }
/// ```
///
/// Why is this useful? Accessing through std::sync::Arc will induce
/// more synchronization, thus causing more executions to be explored
/// by GenMC. spawn_f_args allows for multiple threads to still access
/// the same underlying datastructure of the client application to be
/// verified, with maximal parallelism.
pub unsafe fn spawn_f_args<A, T>(f: fn(A) -> T, arg: A) -> JoinHandle<T>
where
    T: Send + 'static,
    A: Copy
{
    unsafe {
        JoinHandle::<T>(JoinInner::<T> {
            native: ThreadNative::new_f_args::<A, T>(f, arg),
            ret_type: PhantomData::<T>
        })
    }
}

struct ThreadNative {
    id: __VERIFIER_thread_t
}
unsafe impl Send for ThreadNative {}
unsafe impl Sync for ThreadNative {}
impl ThreadNative {
    /// Builder used by spawn_f
    unsafe fn new_f<T: Send + 'static>(f: fn() -> T) -> ThreadNative {
        ThreadNative {
            id: __VERIFIER_spawn(thread_start_f::<T> as *const c_void, f as *mut c_void)
        }
    }

    /// Builder used by spawn_f_args
    unsafe fn new_f_args<A: Copy, T: Send + 'static>(f: fn(A) -> T, arg: A) -> ThreadNative {
        let __arg = Box::into_raw(Box::new(ThreadStartArgs {f, arg}));
        ThreadNative {
            id: __VERIFIER_spawn(thread_start_f_args::<A, T> as *const c_void, __arg as *mut c_void)
        }
    }

    /// Builder used by spawn
    unsafe fn new(main: Box<dyn FnOnce() -> *mut u8>) -> ThreadNative {
        // main is a fat pointer (holds two pointers, one to the code of the closure and one to the data captured),
        // thus we can't pass it directly as we'd loose the second field.
        // => Pass a pointer to main instead
        let p = Box::into_raw(Box::new(main));
        ThreadNative {
            id: __VERIFIER_spawn(thread_start as *const c_void, p as *mut c_void)
        }
    }
}

/// Starting point for a thread with spawn_f.
/// Executes the function, writes the return-value to the heap
/// and returns a ptr to the return-value that will be
/// retrieved by the JoinHandle.
fn thread_start_f<T: Send + 'static>(f: fn() -> T) {
    let ret = Box::new(f());
    __VERIFIER_thread_exit(Box::into_raw(ret) as *mut c_void);
}


struct ThreadStartArgs<A,T> {
    f: fn (A) -> T,
    arg: A
}
/// Starting point for a thread with spawn_f_args.
/// Similar to thread_start_f, with the difference that the arguments-struct
/// must be dropped after executing the function.
fn thread_start_f_args<A: Copy, T: Send + 'static>(c: *mut ThreadStartArgs<A,T>) {
    unsafe {
        let b = Box::new(((*c).f)((*c).arg));
        drop(Box::from_raw(c));
        __VERIFIER_thread_exit(Box::into_raw(b) as *mut c_void);
    }
}

/// Starting point for a function with spawn.
/// Receives a pointer "main" to the closure, the closure
/// is itself a fat pointer, here represented as a struct:
///
/// main -> struct { ptr1, ptr2 }
/// ptr1 -> struct { arguments to the closure }
/// ptr2 -> vtable -> function code of the closure
///
/// The Box gets consumed when executing the underlying closure
/// due to the use of FnOnce(). thus, the heap-allocated memory
/// gets freed at that point.
fn thread_start(main: *mut c_void) -> *mut c_void {
    unsafe {
        if main.is_null() {
            panic!("Something went horribly wrong");
        }
        //Executing the thread
        let f = Box::from_raw(main as *mut Box<dyn FnOnce() -> *mut u8>);
        let ret = f();
        //Both outer and inner Box of f as well as arguments passed to f get dropped at this point => No memory leakage happening

        //Pass a pointer to the return value of f to GenMC
        __VERIFIER_thread_exit(ret as *mut c_void);
    }
    crate::NULL as *mut c_void
}

pub fn yield_now() {
    //Do nothing
}

pub fn sleep(_dur: Duration) {
    //Do nothing
}