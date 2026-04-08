//! This file provides implementations for spawning and joining scoped
//! threads using GenMC. The implementation is strongly modelled after
//! the implementation in Rusts STD. We count the number of running threads
//! and return when that counter reaches 0. We use __VERIFIER_assume for that
//! instead of the busy-waiting used in Rusts STD.
//!
//! Scoped threads have two lifetimes: 'env and 'scope
//! 'scope: Lifetime during which scoped threads may still be running
//! and new threads may still be spawned. It ends when all scoped threads
//! are joined.
//! 'env: Lifetime of everything borrowed by threads in the scope. It must
//! outlast the lifetime of 'scope.
use crate::thread::{JoinInner, Result};
use crate::genmc::__VERIFIER_assume;
use crate::marker::PhantomData;
use crate::sync::Arc;
use crate::sync::atomic::{AtomicUsize, Ordering};


pub struct Scope<'env> {
    num_running_threads: Arc<AtomicUsize>,
    env: PhantomData<&'env mut &'env ()>,
}


pub struct ScopedJoinHandle<'scope, T> {
    join_inner: JoinInner<T>,
    /// Borrows the parent scope with lifetime `'scope`.
    _marker: PhantomData<&'scope ()>,
}


pub fn scope<'env, F, T>(f: F) -> T
where
    F: FnOnce(&Scope<'env>) -> T,
{
    let scope = Scope {
        num_running_threads: Arc::new(AtomicUsize::new(0)),
        env: PhantomData,
    };

    let result = f(&scope);

    // Wait until all the threads are finished.
    let running_threads = scope.num_running_threads.load(Ordering::Acquire);
    __VERIFIER_assume(running_threads == 0);

    result
}


impl<'env> Scope<'env> {
    pub fn spawn<'scope, F, T>(&'scope self, f: F) -> ScopedJoinHandle<'scope, T>
    where
        F: FnOnce() -> T,
        F: Send + 'env,
        T: Send + 'env,
    {
        self.builder()
            .spawn(f)
            .expect("failed to spawn scoped thread")
    }

    pub fn builder<'scope>(&'scope self) -> ScopedThreadBuilder<'scope, 'env> {
        ScopedThreadBuilder {
            scope: self
        }
    }

    fn increment_num_running_threads(&self) {
        self.num_running_threads.fetch_add(1, Ordering::Relaxed);
    }
}

fn decrement_num_running_threads(num_running_threads: &Arc<AtomicUsize>) {
    num_running_threads.fetch_sub(1, Ordering::Release);
}


pub struct ScopedThreadBuilder<'scope, 'env> {
    scope: &'scope Scope<'env>
}

impl<'scope, 'env> ScopedThreadBuilder<'scope, 'env> {
    pub fn spawn<F, T>(self, f: F) -> std::io::Result<ScopedJoinHandle<'scope, T>>
    where
        F: FnOnce() -> T,
        F: Send + 'env,
        T: Send + 'env,
    {
        self.scope.increment_num_running_threads();
        let num_running_threads = self.scope.num_running_threads.clone();

        let f = super::MaybeDangling::new(f);
        let main = move || -> *mut u8 {
            let f = f.into_inner();

            // Execute f, allocate the return value on the heap
            let ret = Box::into_raw(Box::new(f())) as *mut u8;

            // Safe to decrease here as the values captured by this scoped
            // spawn are not used anymore, thus other threads can use them
            // again, even if this thread continues to run.
            decrement_num_running_threads(&num_running_threads);

            // Return the pointer to the return value of f to thread_start.
            // Can't __VERIFIER_exit here as it would cause memory-leakage.
            ret
        };

        let main: Box<dyn FnOnce() -> *mut u8 + Send + 'env> = Box::new(main);
        // Lifetime change: The caller must guarantee that the passed-in lifetime is long enough
        // for the lifetime of the thread.
        let main: Box<dyn FnOnce() -> *mut u8 + Send + 'static> = unsafe { std::mem::transmute(main) };

        Ok(
            ScopedJoinHandle::<T> {
                join_inner: JoinInner::<T> {
                    // Same as for standard threads
                    native: unsafe { super::ThreadNative::new(main) },
                    ret_type: PhantomData::<T>
                },
                _marker: PhantomData
            }
        )
    }
}


// We call the same Join as for standard threads
impl<'scope, T> ScopedJoinHandle<'scope, T> {
    pub fn join(self) -> Result<T> {
        self.join_inner.join()
    }
}
