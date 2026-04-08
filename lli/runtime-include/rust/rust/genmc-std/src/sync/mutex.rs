//! This file provides implementations for using Mutexes in Rust with GenMC.
//! It fullfills the same type-requirements as std::sync::Mutex & std::sync::MutexGuard
//! in Rusts STD, thus it can be swapped in to pre-existing code without any changes.
//!
//! This Mutex is always considered not poisoned as GenMC does not model failures.
//! TODOs are left in places that need to be extended if we decide to model failures in the future.
//!
//! Locking the mutex returns a MutexGuard, the mutex is automatically unlocked when the MutexGuard
//! is dropped.
use std::cell::UnsafeCell;
use std::fmt;
use std::ops::{Deref, DerefMut};
use std::sync::LockResult;
use std::sync::TryLockResult;
use std::sync::TryLockError;

use crate::sync::genmc::mutex::*;

//TODO: Model failures in GenMC

//Structs
pub struct Mutex<T: ?Sized> {
    inner: __VERIFIER_mutex_t,
    data: UnsafeCell<T>,
}

pub struct MutexGuard<'a, T: ?Sized + 'a> {
    lock: &'a Mutex<T>,
}


//Mutex implementations
unsafe impl<T: ?Sized + Send> Send for Mutex<T> {}
unsafe impl<T: ?Sized + Send> Sync for Mutex<T> {}

impl<T> Mutex<T> {
    #[inline]
    pub const fn new(t: T) -> Mutex<T> {
        let mutex: __VERIFIER_mutex_t = __VERIFIER_mutex_t {__private: 0};
        //Can't call __VERIFIER_mutex_init here as function must be const, but no need to init/destroy the mutex at all

        Mutex {
            inner: mutex,
            data: UnsafeCell::new(t)
        }
    }
}

impl<T: ?Sized> Mutex<T> {
    pub fn lock(&self) -> LockResult<MutexGuard<'_, T>> {
        unsafe {
            let ptr: *const __VERIFIER_mutex_t = &self.inner as *const __VERIFIER_mutex_t; //&raw const self.inner;
            let _status: i32 = __VERIFIER_mutex_lock(ptr as *mut __VERIFIER_mutex_t);
            //TODO: use _status to model failures in GenMC

            LockResult::Ok(MutexGuard::new(self))
        }
    }

    pub fn try_lock(&self) -> TryLockResult<MutexGuard<'_, T>> {
        unsafe {
            let ptr: *const __VERIFIER_mutex_t = &self.inner as *const __VERIFIER_mutex_t; //&raw const self.inner;
            let status: i32 = __VERIFIER_mutex_trylock(ptr as *mut __VERIFIER_mutex_t);

            if status == 0 {
                Ok(MutexGuard::new(self)) //No ? needed for Error-Propagation, as MutexGuard::new can't throw an error
            } else {
                Err(TryLockError::WouldBlock)
            }
        }
    }

    #[inline]
    pub fn is_poisoned(&self) -> bool {
        //TODO: Implement poisoned to model failures in GenMC
        false //Always false as can't panic on __VERIFIER_mutex_lock
    }

    #[inline]
    pub fn clear_poison(&self) {
    }

    pub fn get_mut(&mut self) -> LockResult<&mut T> {
        let data = self.data.get_mut();
        LockResult::Ok(data)
    }

    pub fn into_inner(self) -> LockResult<T>
    where
        T: Sized,
    {
        let data = self.data.into_inner();
        LockResult::Ok(data)
    }
}

impl<T> From<T> for Mutex<T> {
    fn from(t: T) -> Self {
        Mutex::new(t)
    }
}

impl<T: ?Sized + Default> Default for Mutex<T> {
    fn default() -> Mutex<T> {
        Mutex::new(Default::default())
    }
}

impl<T: ?Sized + fmt::Debug> fmt::Debug for Mutex<T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut d = f.debug_struct("Mutex");
        match self.try_lock() {
            Ok(guard) => {
                d.field("data", &&*guard);
            }
            Err(TryLockError::Poisoned(err)) => {
                d.field("data", err.get_ref());
            }
            Err(TryLockError::WouldBlock) => {
                d.field("data", &format_args!("<locked>"));
            }
        }
        d.finish_non_exhaustive()
    }
}


pub fn guard_lock<'a, T: ?Sized>(guard: &MutexGuard<'a, T>) -> &'a __VERIFIER_mutex_t {
    &guard.lock.inner
}

/*pub fn guard_poison<'a, T: ?Sized>(guard: &MutexGuard<'a, T>) -> &'a poison::Flag {
    &guard.lock.poison
}*/ //We don't model failures -> poisoned=false always



//MutexGuard Implementations
//impl<T: ?Sized> !Send for MutexGuard<'_, T> {} //TODO: implement for newer versions
unsafe impl<T: ?Sized + Sync> Sync for MutexGuard<'_, T> {}

impl<T: ?Sized> Deref for MutexGuard<'_, T> {
    type Target = T;

    fn deref(&self) -> &T {
        unsafe { &*self.lock.data.get() }
    }
}


impl<T: ?Sized> DerefMut for MutexGuard<'_, T> {
    fn deref_mut(&mut self) -> &mut T {
        unsafe { &mut *self.lock.data.get() }
    }
}


impl<T: ?Sized> Drop for MutexGuard<'_, T> {
    #[inline]
    fn drop(&mut self) { //Automatic unlock when lock goes out of scope
        let ptr = &self.lock.inner as *const __VERIFIER_mutex_t; //&raw const self.lock.inner;
        let _status_unlock: i32 = __VERIFIER_mutex_unlock(ptr as *mut __VERIFIER_mutex_t);
        //TODO: Use _status_unlock to model failures in GenMC
    }
}


impl<'mutex, T: ?Sized> MutexGuard<'mutex, T> {
    unsafe fn new(lock: &'mutex Mutex<T>) -> MutexGuard<'mutex, T> {
        MutexGuard { lock: lock }
    }
}


impl<T: ?Sized + fmt::Debug> fmt::Debug for MutexGuard<'_, T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        fmt::Debug::fmt(&**self, f)
    }
}


impl<T: ?Sized + fmt::Display> fmt::Display for MutexGuard<'_, T> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        (**self).fmt(f)
    }
}
