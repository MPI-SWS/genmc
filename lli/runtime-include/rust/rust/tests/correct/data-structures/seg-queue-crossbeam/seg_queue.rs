//! This file models the SegQueue implementation from crossbeam
//! Changes that were applied for our purpose of model checking:
//! - Removed Cache Padding
//! - Removed Backoff (spin (i.e. exponential backoff), Snoozing)
//! - #[repr(C)] on the structs
//! - Loops are bounded (use: --cfg LOOP_BOUND1 / ... / -- cfg LOOP_BOUND5) to
//!   set the maximum number of loops to 1 - 5. See bottom of the file.
//! - wait_write always has a loop-bound of 1 (as it would be a nested loop otherwise)
//! - We zero-initialize the memory manually in SegQueue::new() instead of using alloc_zeroed due
//!   to a current issue that causes "reads to uninitialized memory" for atomic rmw operations
//!   on the heap-space. See SegQueue::new_original() as a reference.
//!
//! Please refer to crossbeams repo for any further information.
use std::alloc::{alloc, alloc_zeroed, handle_alloc_error, Layout};
use std::cell::UnsafeCell;
use std::marker::PhantomData;
use std::mem::MaybeUninit;
use std::panic::{RefUnwindSafe, UnwindSafe};
use std::ptr;
use std::sync::atomic::{self, AtomicPtr, AtomicUsize, Ordering};


// Bits indicating the state of a slot:
// * If a value has been written into the slot, `WRITE` is set.
// * If a value has been read from the slot, `READ` is set.
// * If the block is being destroyed, `DESTROY` is set.
const WRITE: usize = 1;
const READ: usize = 2;
const DESTROY: usize = 4;

// Each block covers one "lap" of indices.
const LAP: usize = 32;
// The maximum number of values a block can hold.
const BLOCK_CAP: usize = LAP - 1;
// How many lower bits are reserved for metadata.
const SHIFT: usize = 1;
// Indicates that the block is not the last one.
const HAS_NEXT: usize = 1;

/// A slot in a block.
struct Slot<T> {
    /// The value.
    value: UnsafeCell<MaybeUninit<T>>,

    /// The state of the slot.
    state: AtomicUsize,
}

/// OWN IMPLEMENTATION FOR CIRCUMVENTING ALLOC_ZEROED
impl<T: Default> Default for Slot<T>  {
    fn default() -> Self {
        let s = AtomicUsize::new(0);
        s.store(0, Ordering::SeqCst);
        Slot {
            value: UnsafeCell::new(MaybeUninit::new(T::default())),
            state: s,
        }
    }
}
///

impl<T> Slot<T> {
    /// Waits until a value is written into the slot.
    fn wait_write(&self) {
        let l = self.state.load(Ordering::Acquire) & WRITE == 0;
        std::genmc::__VERIFIER_assume(!l);

        /* Original as reference:

        let backoff = Backoff::new();
        while self.state_ptr.load(Ordering::Acquire) & WRITE == 0 { //HERE: Read to uninitialized memory!
            backoff.snooze();
        }
        */
    }
}

/// A block in a linked list.
///
/// Each block in the list can hold up to `BLOCK_CAP` values.
#[derive(Default)]
pub struct Block<T> {
    /// The next block in the linked list.
    next: AtomicPtr<Block<T>>,

    /// Slots for values.
    slots: [Slot<T>; BLOCK_CAP],
}

impl<T: Default> Block<T> {
    const LAYOUT: Layout = {
        let layout = Layout::new::<Self>();
        assert!(
            layout.size() != 0,
            "Block should never be zero-sized, as it has an AtomicPtr field"
        );
        layout
    };

    /// Creates an empty block.
    fn new_original() -> Box<Self> {
        // SAFETY: layout is not zero-sized
        let ptr = unsafe { alloc_zeroed(Self::LAYOUT) };
        // Handle allocation failure
        if ptr.is_null() {
            handle_alloc_error(Self::LAYOUT)
        }
        // SAFETY: This is safe because:
        //  [1] `Block::next` (AtomicPtr) may be safely zero initialized.
        //  [2] `Block::slots` (Array) may be safely zero initialized because of [3, 4].
        //  [3] `Slot::value` (UnsafeCell) may be safely zero initialized because it
        //       holds a MaybeUninit.
        //  [4] `Slot::state` (AtomicUsize) may be safely zero initialized.
        // TODO: unsafe { Box::new_zeroed().assume_init() }
        unsafe { Box::from_raw(ptr.cast()) }
    }

    ///We simulate alloc-zeroed by 0-initializing manually here
    fn new() -> Box<Self> {
        let ptr: *mut Block<T> = unsafe { alloc(Self::LAYOUT) as *mut Block<T> };
        if ptr.is_null() {
            handle_alloc_error(Self::LAYOUT)
        }

        //We 0-initialize ourselves here to simulate alloc-zeroed:
        unsafe {
            (*ptr).next = AtomicPtr::default();
            (*ptr).next.store(ptr::null_mut(), Ordering::SeqCst);
            for i in 0..BLOCK_CAP {
                (*ptr).slots[i] = Slot::default();
            }
        }

        unsafe { Box::from_raw(ptr.cast()) }
    }

    /// Waits until the next pointer is set.
    fn wait_next(&self) -> *mut Self {
        loop {
            let next = self.next.load(Ordering::Acquire);
            if !next.is_null() {
                return next;
            }
        }
    }

    /// Sets the `DESTROY` bit in slots starting from `start` and destroys the block.
    unsafe fn destroy(this: *mut Self, start: usize) {
        // It is not necessary to set the `DESTROY` bit in the last slot because that slot has
        // begun destruction of the block.
        for i in start..BLOCK_CAP - 1 {
            let slot = unsafe { (*this).slots.get_unchecked(i) };

            // Mark the `DESTROY` bit if a thread is still using the slot.
            if slot.state.load(Ordering::Acquire) & READ == 0
                && slot.state.fetch_or(DESTROY, Ordering::AcqRel) & READ == 0
            {
                // If a thread is still using the slot, it will continue destruction of the block.
                return;
            }
        }

        // No thread is using the block, now it is safe to destroy it.
        drop(unsafe { Box::from_raw(this) });
    }
}

/// A position in a queue.
#[repr(C)]
pub struct Position<T> {
    /// The index in the queue.
    pub index: AtomicUsize,

    /// The block in the linked list.
    pub block: AtomicPtr<Block<T>>,
}

#[repr(C)]
pub struct SegQueue<T> {
    /// The head of the queue.
    pub head: Position<T>,

    /// The tail of the queue.
    pub tail: Position<T>,

    /// Indicates that dropping a `SegQueue<T>` may drop values of type `T`.
    _marker: PhantomData<T>,
}

unsafe impl<T: Send> Send for SegQueue<T> {}
unsafe impl<T: Send> Sync for SegQueue<T> {}

impl<T> UnwindSafe for SegQueue<T> {}
impl<T> RefUnwindSafe for SegQueue<T> {}

impl<T: Default> SegQueue<T> {
    pub const fn new() -> Self {
        Self {
            head: Position {
                block: AtomicPtr::new(ptr::null_mut()),
                index: AtomicUsize::new(0),
            },
            tail: Position {
                block: AtomicPtr::new(ptr::null_mut()),
                index: AtomicUsize::new(0),
            },
            _marker: PhantomData,
        }
    }

    pub fn push(&self, value: T) {
        let mut tail = self.tail.index.load(Ordering::Acquire);
        let mut block = self.tail.block.load(Ordering::Acquire);
        let mut next_block = None;

        let mut i = 0;
        loop {
            std::genmc::__VERIFIER_assume(i < LOOP_BOUND);
            i += 1;

            // Calculate the offset of the index into the block.
            let offset = (tail >> SHIFT) % LAP;

            // If we reached the end of the block, wait until the next one is installed.
            if offset == BLOCK_CAP {
                tail = self.tail.index.load(Ordering::Acquire);
                block = self.tail.block.load(Ordering::Acquire);
                continue;
            }

            // If we're going to have to install the next block, allocate it in advance in order to
            // make the wait for other threads as short as possible.
            if offset + 1 == BLOCK_CAP && next_block.is_none() {
                next_block = Some(Block::<T>::new());
            }

            // If this is the first push operation, we need to allocate the first block.
            if block.is_null() {
                let new = Box::into_raw(Block::<T>::new());

                if self
                    .tail
                    .block
                    .compare_exchange(block, new, Ordering::Release, Ordering::Relaxed)
                    .is_ok()
                {
                    self.head.block.store(new, Ordering::Release);
                    block = new;
                } else {
                    next_block = unsafe { Some(Box::from_raw(new)) };
                    tail = self.tail.index.load(Ordering::Acquire);
                    block = self.tail.block.load(Ordering::Acquire);
                    continue;
                }
            }

            let new_tail = tail + (1 << SHIFT);

            // Try advancing the tail forward.
            match self.tail.index.compare_exchange_weak(
                tail,
                new_tail,
                Ordering::SeqCst,
                Ordering::Acquire,
            ) {
                Ok(_) => unsafe {
                    // If we've reached the end of the block, install the next one.
                    if offset + 1 == BLOCK_CAP {
                        let next_block = Box::into_raw(next_block.unwrap());
                        let next_index = new_tail.wrapping_add(1 << SHIFT);

                        self.tail.block.store(next_block, Ordering::Release);
                        self.tail.index.store(next_index, Ordering::Release);
                        (*block).next.store(next_block, Ordering::Release);
                    }

                    // Write the value into the slot.
                    let slot = (*block).slots.get_unchecked(offset);
                    slot.value.get().write(MaybeUninit::new(value));
                    slot.state.fetch_or(WRITE, Ordering::Release);

                    return;
                },
                Err(t) => {
                    tail = t;
                    block = self.tail.block.load(Ordering::Acquire);
                }
            }
        }
    }

    pub fn pop(&self) -> Option<T> {
        let mut head = self.head.index.load(Ordering::Acquire);
        let mut block = self.head.block.load(Ordering::Acquire);

        let mut i = 0;
        loop {
            std::genmc::__VERIFIER_assume(i < LOOP_BOUND);
            i += 1;

            // Calculate the offset of the index into the block.
            let offset = (head >> SHIFT) % LAP;

            // If we reached the end of the block, wait until the next one is installed.
            if offset == BLOCK_CAP {
                head = self.head.index.load(Ordering::Acquire);
                block = self.head.block.load(Ordering::Acquire);
                continue;
            }

            let mut new_head = head + (1 << SHIFT);

            if new_head & HAS_NEXT == 0 {
                atomic::fence(Ordering::SeqCst);
                let tail = self.tail.index.load(Ordering::Relaxed);

                // If the tail equals the head, that means the queue is empty.
                if head >> SHIFT == tail >> SHIFT {
                    return None;
                }

                // If head and tail are not in the same block, set `HAS_NEXT` in head.
                if (head >> SHIFT) / LAP != (tail >> SHIFT) / LAP {
                    new_head |= HAS_NEXT;
                }
            }

            // The block can be null here only if the first push operation is in progress. In that
            // case, just wait until it gets initialized.
            if block.is_null() {
                head = self.head.index.load(Ordering::Acquire);
                block = self.head.block.load(Ordering::Acquire);
                continue;
            }

            // Try moving the head index forward.
            match self.head.index.compare_exchange_weak(
                head,
                new_head,
                Ordering::SeqCst,
                Ordering::Acquire,
            ) {
                Ok(_) => unsafe {
                    // If we've reached the end of the block, move to the next one.
                    if offset + 1 == BLOCK_CAP {
                        let next = (*block).wait_next();
                        let mut next_index = (new_head & !HAS_NEXT).wrapping_add(1 << SHIFT);
                        if !(*next).next.load(Ordering::Relaxed).is_null() {
                            next_index |= HAS_NEXT;
                        }

                        self.head.block.store(next, Ordering::Release);
                        self.head.index.store(next_index, Ordering::Release);
                    }

                    // Read the value.
                    let slot = (*block).slots.get_unchecked(offset);
                    slot.wait_write();
                    let value = slot.value.get().read().assume_init();

                    // Destroy the block if we've reached the end, or if another thread wanted to
                    // destroy but couldn't because we were busy reading from the slot.
                    if offset + 1 == BLOCK_CAP {
                        Block::destroy(block, 0);
                    } else if slot.state.fetch_or(READ, Ordering::AcqRel) & DESTROY != 0 {
                        Block::destroy(block, offset + 1);
                    }
                    return Some(value);
                },
                Err(h) => {
                    head = h;
                    block = self.head.block.load(Ordering::Acquire);
                }
            }
        }
    }

    pub fn is_empty(&self) -> bool {
        let head = self.head.index.load(Ordering::SeqCst);
        let tail = self.tail.index.load(Ordering::SeqCst);
        head >> SHIFT == tail >> SHIFT
    }

    pub fn len(&self) -> usize {
        loop {
            // Load the tail index, then load the head index.
            let mut tail = self.tail.index.load(Ordering::SeqCst);
            let mut head = self.head.index.load(Ordering::SeqCst);

            // If the tail index didn't change, we've got consistent indices to work with.
            if self.tail.index.load(Ordering::SeqCst) == tail {
                // Erase the lower bits.
                tail &= !((1 << SHIFT) - 1);
                head &= !((1 << SHIFT) - 1);

                // Fix up indices if they fall onto block ends.
                if (tail >> SHIFT) & (LAP - 1) == LAP - 1 {
                    tail = tail.wrapping_add(1 << SHIFT);
                }
                if (head >> SHIFT) & (LAP - 1) == LAP - 1 {
                    head = head.wrapping_add(1 << SHIFT);
                }

                // Rotate indices so that head falls into the first block.
                let lap = (head >> SHIFT) / LAP;
                tail = tail.wrapping_sub((lap * LAP) << SHIFT);
                head = head.wrapping_sub((lap * LAP) << SHIFT);

                // Remove the lower bits.
                tail >>= SHIFT;
                head >>= SHIFT;

                // Return the difference minus the number of blocks between tail and head.
                return tail - head - tail / LAP;
            }
        }
    }
}

impl<T> Drop for SegQueue<T> {
    fn drop(&mut self) {
        let mut head = *self.head.index.get_mut();
        let mut tail = *self.tail.index.get_mut();
        let mut block = *self.head.block.get_mut();

        // Erase the lower bits.
        head &= !((1 << SHIFT) - 1);
        tail &= !((1 << SHIFT) - 1);

        unsafe {
            // Drop all values between `head` and `tail` and deallocate the heap-allocated blocks.
            while head != tail {
                let offset = (head >> SHIFT) % LAP;

                if offset < BLOCK_CAP {
                    // Drop the value in the slot.
                    let slot = (*block).slots.get_unchecked(offset);
                    (*slot.value.get()).assume_init_drop();
                } else {
                    // Deallocate the block and move to the next one.
                    let next = *(*block).next.get_mut();
                    drop(Box::from_raw(block));
                    block = next;
                }

                head = head.wrapping_add(1 << SHIFT);
            }

            // Deallocate the last remaining block.
            if !block.is_null() {
                drop(Box::from_raw(block));
            }
        }
    }
}

impl<T: Default> Default for SegQueue<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> IntoIterator for SegQueue<T> {
    type Item = T;

    type IntoIter = IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        IntoIter { value: self }
    }
}

//#[derive(Debug)]
pub struct IntoIter<T> {
    value: SegQueue<T>,
}

impl<T> Iterator for IntoIter<T> {
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        let value = &mut self.value;
        let head = *value.head.index.get_mut();
        let tail = *value.tail.index.get_mut();
        if head >> SHIFT == tail >> SHIFT {
            None
        } else {
            let block = *value.head.block.get_mut();
            let offset = (head >> SHIFT) % LAP;

            // SAFETY: We have mutable access to this, so we can read without
            // worrying about concurrency. Furthermore, we know this is
            // initialized because it is the value pointed at by `value.head`
            // and this is a non-empty queue.
            let item = unsafe {
                let slot = (*block).slots.get_unchecked(offset);
                slot.value.get().read().assume_init()
            };
            if offset + 1 == BLOCK_CAP {
                // Deallocate the block and move to the next one.
                // SAFETY: The block is initialized because we've been reading
                // from it this entire time. We can drop it b/c everything has
                // been read out of it, so nothing is pointing to it anymore.
                unsafe {
                    let next = *(*block).next.get_mut();
                    drop(Box::from_raw(block));
                    *value.head.block.get_mut() = next;
                }
                // The last value in a block is empty, so skip it
                *value.head.index.get_mut() = head.wrapping_add(2 << SHIFT);
                // Double-check that we're pointing to the first item in a block.
                debug_assert_eq!((*value.head.index.get_mut() >> SHIFT) % LAP, 0);
            } else {
                *value.head.index.get_mut() = head.wrapping_add(1 << SHIFT);
            }
            Some(item)
        }
    }
}



// We bound all loops
#[cfg(LOOP_BOUND1)]
pub const LOOP_BOUND: usize = 1;
#[cfg(LOOP_BOUND2)]
pub const LOOP_BOUND: usize = 2;
#[cfg(LOOP_BOUND3)]
pub const LOOP_BOUND: usize = 3;
#[cfg(LOOP_BOUND4)]
pub const LOOP_BOUND: usize = 4;
#[cfg(LOOP_BOUND5)]
pub const LOOP_BOUND: usize = 5;
//Default
#[cfg(not(any(LOOP_BOUND1, LOOP_BOUND2, LOOP_BOUND3, LOOP_BOUND4, LOOP_BOUND5)))]
pub const LOOP_BOUND: usize = 2;