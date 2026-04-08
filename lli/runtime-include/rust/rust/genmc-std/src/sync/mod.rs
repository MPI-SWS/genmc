pub use std::sync::*; //Re-Export everything

pub(crate) mod genmc;
mod mutex;

pub use mutex::Mutex;
pub use mutex::MutexGuard;