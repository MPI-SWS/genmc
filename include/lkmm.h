/*
 * GenMC -- Generic Model Checking.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-3.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef __LKMM_H__
#define __LKMM_H__

#ifdef __CLANG_STDATOMIC_H
#error "Only one of <stdatomic.h> and <lkmm.h> may be used!"
#endif

/* Rely on pthread types for the time being */
#include <pthread.h>
#include <stdint.h>

/*
 * Helper macros for the rest of definitions
 *
 * We define these in C11 style so that user programs that use C11 atomics can
 * be tested under the LKMM too (provided that variables are redefined).
 * The parts that are LKMM-specific are translated to special internal functions.
 */

typedef enum memory_order {
  memory_order_relaxed = __ATOMIC_RELAXED,
  memory_order_consume = __ATOMIC_CONSUME,
  memory_order_acquire = __ATOMIC_ACQUIRE,
  memory_order_release = __ATOMIC_RELEASE,
  memory_order_acq_rel = __ATOMIC_ACQ_REL,
  memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

#define atomic_load_explicit(p, m)     __atomic_load_n(p, m)
#define atomic_store_explicit(p, v, m) __atomic_store_n(p, v, m)

#define atomic_exchange_explicit(p, v, m) __atomic_exchange_n(p, v, m)

#define atomic_compare_exchange_strong_explicit(p, e, d, ms, mf)	\
	__atomic_compare_exchange_n(p, e, d, 0, ms, mf)
#define atomic_compare_exchange_weak_explicit(p, e, d, ms, mf)		\
	__atomic_compare_exchange_n(p, e, d, 1, ms, mf)

#define atomic_fetch_add_explicit(p, v, m) __atomic_fetch_add(p, v, m)
#define atomic_fetch_sub_explicit(p, v, m) __atomic_fetch_sub(p, v, m)
#define atomic_fetch_or_explicit(p, v, m)  __atomic_fetch_or(p, v, m)
#define atomic_fetch_xor_explicit(p, v, m) __atomic_fetch_xor(p, v, m)
#define atomic_fetch_and_explicit(p, v, m) __atomic_fetch_and(p, v, m)

/* These do not exist in C11 */
#define atomic_add_fetch_explicit(p, v, m) __atomic_add_fetch(p, v, m)
#define atomic_sub_fetch_explicit(p, v, m) __atomic_sub_fetch(p, v, m)
#define atomic_or_fetch_explicit(p, v, m)  __atomic_or_fetch(p, v, m)
#define atomic_xor_fetch_explicit(p, v, m) __atomic_xor_fetch(p, v, m)
#define atomic_and_fetch_explicit(p, v, m) __atomic_and_fetch(p, v, m)


/*******************************************************************************
 **                         ONCE, FENCES AND FRIENDS
 ******************************************************************************/

/* ONCE */
#define READ_ONCE(x)     atomic_load_explicit(&x, memory_order_relaxed)
#define WRITE_ONCE(x, v) atomic_store_explicit(&x, v, memory_order_relaxed)

/* Fences */

/* atomic_signal_fence(memory_order_relaxed) is useless for barrier() */
#define barrier() __asm__ __volatile__ (""   : : : "memory")

void __VERIFIER_lkmm_fence(const char *);

#define __LKMM_FENCE(type) __VERIFIER_lkmm_fence(#type)
#define smp_mb()  __LKMM_FENCE(mb)
#define smp_rmb() __LKMM_FENCE(rmb)
#define smp_wmb() __LKMM_FENCE(wmb)
#define smp_mb__before_atomic()     __LKMM_FENCE(ba)
#define smp_mb__after_atomic()      __LKMM_FENCE(aa)
#define smp_mb__after_spinlock()    __LKMM_FENCE(as)
#define smp_mb__after_unlock_lock() __LKMM_FENCE(aul)

/* Acquire/Release and friends */
#define smp_load_acquire(p)      atomic_load_explicit(p, memory_order_acquire)
#define smp_store_release(p, v)  atomic_store_explicit(p, v, memory_order_release)
#define rcu_dereference(p)       READ_ONCE(p)
#define rcu_assign_pointer(p, v) smp_store_release(&(p), v)
#define smp_store_mb(x, v)					\
do {								\
	atomic_store_explicit(&x, v, memory_order_relaxed);	\
	smp_mb();						\
} while (0)

/* Exchange */
#define __xchg(p, v, m)				\
	atomic_exchange_explicit(p, v, m)
#define xchg(p, v)							\
({									\
	__typeof__((v)) _v_ = (v);					\
	smp_mb__before_atomic();					\
	_v_ = __xchg(p, v, memory_order_relaxed);			\
	smp_mb__after_atomic();						\
	_v_;								\
})
#define xchg_relaxed(p, v) __xchg(p, v, memory_order_relaxed)
#define xchg_release(p, v) __xchg(p, v, memory_order_release)
#define xchg_acquire(p, v) __xchg(p, v, memory_order_acquire)

#define __cmpxchg(p, o, n, s, f)					\
({									\
	__typeof__((o)) _o_ = (o);					\
	(atomic_compare_exchange_strong_explicit(p, &_o_, n, s, f));	\
	_o_;								\
})
#define cmpxchg(p, o, n)						\
({									\
	__typeof__((o)) _o_ = (o);					\
	smp_mb__before_atomic();					\
	_o_ = __cmpxchg(p, o, n, memory_order_relaxed, memory_order_relaxed); \
	smp_mb__after_atomic();						\
	_o_;								\
})
#define cmpxchg_relaxed(p, o, n)				\
	__cmpxchg(p, o, n, memory_order_relaxed, memory_order_relaxed)
#define cmpxchg_acquire(p, o, n)				\
	__cmpxchg(p, o, n, memory_order_acquire, memory_order_acquire)
#define cmpxchg_release(p, o, n)				\
	__cmpxchg(p, o, n, memory_order_release, memory_order_release)


/*******************************************************************************
 **                               SPINLOCKS
 ******************************************************************************/

/* Spinlocks */
typedef pthread_mutex_t spinlock_t;

#define spin_lock(l)      pthread_mutex_lock(l)
#define spin_unlock(l)    pthread_mutex_unlock(l)
#define spin_trylock(l)   !pthread_mutex_trylock(l)
#define spin_is_locked(l) (atomic_load_explicit(&((l)->__private), memory_order_relaxed) == 1)


/*******************************************************************************
 **                               RCU
 ******************************************************************************/

void __VERIFIER_rcu_read_lock();
void __VERIFIER_rcu_read_unlock();
void __VERIFIER_synchronize_rcu();

#define rcu_read_lock()   __VERIFIER_rcu_read_lock()
#define rcu_read_unlock() __VERIFIER_rcu_read_unlock()
#define synchronize_rcu() __VERIFIER_synchronize_rcu()
#define synchronize_rcu_expedited() __VERIFIER_synchronize_rcu()


/*******************************************************************************
 **                            ATOMIC OPERATIONS
 ******************************************************************************/

/* Atomic data types */
typedef struct {
	int counter;
} atomic_t;

typedef struct {
	int64_t counter;
} atomic64_t;
typedef atomic64_t  atomic_long_t;

/* Initialization */
#define ATOMIC_INIT(i) { (i) }

/* Basic operations */
#define atomic_read(v)   READ_ONCE((v)->counter)
#define atomic_set(v, i) WRITE_ONCE(((v)->counter), (i))
#define atomic_read_acquire(v)   smp_load_acquire(&(v)->counter)
#define atomic_set_release(v, i) smp_store_release(&(v)->counter, (i))

/* Helpers for *non-value-returning* atomics.
 * The last argument of __VERIFIER_atomic_noret stems from llvm::AtomicRMWInst::BinOp,
 * and encodes the type of operation performed. */
void __VERIFIER_atomicrmw_noret(int *, int, memory_order, int);
#define __VERIFIER_fetch_add_noret(v, i, m) __VERIFIER_atomicrmw_noret(v, i, m, 1)
#define __VERIFIER_fetch_sub_noret(v, i, m) __VERIFIER_atomicrmw_noret(v, i, m, 2)

/* Non-value-returning atomics */
#define __atomic_add(i, v, m) __VERIFIER_fetch_add_noret(&(v)->counter, i, m)
#define __atomic_sub(i, v, m) __VERIFIER_fetch_sub_noret(&(v)->counter, i, m)

#define atomic_add(i, v) __atomic_add(i, v, memory_order_relaxed)
#define atomic_sub(i, v) __atomic_sub(i, v, memory_order_relaxed)
#define atomic_inc(v) atomic_add(1, v)
#define atomic_dec(v) atomic_sub(1, v)

/* Value-returning atomics */
#define __atomic_fetch_add(i, v, m) atomic_fetch_add_explicit(&(v)->counter, i, m)
#define __atomic_fetch_sub(i, v, m) atomic_fetch_sub_explicit(&(v)->counter, i, m)
#define __atomic_add_return(i, v, m) atomic_add_fetch_explicit(&(v)->counter, i, m)
#define __atomic_sub_return(i, v, m) atomic_sub_fetch_explicit(&(v)->counter, i, m)

#define atomic_add_return(i, v)						\
({									\
	__typeof__((i)) _i_ = (i);					\
	smp_mb__before_atomic();					\
	_i_ = __atomic_add_return(i, v, memory_order_relaxed);		\
	smp_mb__after_atomic();						\
	_i_;								\
})
#define atomic_add_return_relaxed(i, v) __atomic_add_return(i, v, memory_order_relaxed)
#define atomic_add_return_acquire(i, v) __atomic_add_return(i, v, memory_order_acquire)
#define atomic_add_return_release(i, v) __atomic_add_return(i, v, memory_order_release)

#define atomic_fetch_add(i, v)						\
({									\
	__typeof__((i)) _i_ = (i);					\
	smp_mb__before_atomic();					\
	_i_ = __atomic_fetch_add(i, v, memory_order_relaxed);		\
	smp_mb__after_atomic();						\
	_i_;								\
})
#define atomic_fetch_add_relaxed(i, v) __atomic_fetch_add(i, v, memory_order_relaxed)
#define atomic_fetch_add_acquire(i, v) __atomic_fetch_add(i, v, memory_order_acquire)
#define atomic_fetch_add_release(i, v) __atomic_fetch_add(i, v, memory_order_release)

#define atomic_inc_return(v)         atomic_add_return(1, v)
#define atomic_inc_return_relaxed(v) atomic_add_return_relaxed(1, v)
#define atomic_inc_return_acquire(v) atomic_add_return_acquire(1, v)
#define atomic_inc_return_release(v) atomic_add_return_release(1, v)
#define atomic_fetch_inc(v)         atomic_fetch_add(1, v)
#define atomic_fetch_inc_relaxed(v) atomic_fetch_add_relaxed(1, v)
#define atomic_fetch_inc_acquire(v) atomic_fetch_add_acquire(1, v)
#define atomic_fetch_inc_release(v) atomic_fetch_add_release(1, v)

#define atomic_sub_return(i, v)						\
({									\
	__typeof__((i)) _i_ = (i);					\
	smp_mb__before_atomic();					\
	_i_ = __atomic_sub_return(i, v, memory_order_relaxed);		\
	smp_mb__after_atomic();						\
	_i_;								\
})
#define atomic_sub_return_relaxed(i, v) __atomic_sub_return(i, v, memory_order_relaxed)
#define atomic_sub_return_acquire(i, v) __atomic_sub_return(i, v, memory_order_acquire)
#define atomic_sub_return_release(i, v) __atomic_sub_return(i, v, memory_order_release)

#define atomic_fetch_sub(i, v)						\
({									\
	__typeof__((i)) _i_ = (i);					\
	smp_mb__before_atomic();					\
	_i_ = __atomic_fetch_sub(i, v, memory_order_relaxed);		\
	smp_mb__after_atomic();						\
	_i_;								\
})
#define atomic_fetch_sub_relaxed(i, v) __atomic_fetch_sub(i, v, memory_order_relaxed)
#define atomic_fetch_sub_acquire(i, v) __atomic_fetch_sub(i, v, memory_order_acquire)
#define atomic_fetch_sub_release(i, v) __atomic_fetch_sub(i, v, memory_order_release)

#define atomic_dec_return(v)         atomic_sub_return(1, v)
#define atomic_dec_return_relaxed(v) atomic_sub_return_relaxed(1, v)
#define atomic_dec_return_acquire(v) atomic_sub_return_acquire(1, v)
#define atomic_dec_return_release(v) atomic_sub_return_release(1, v)
#define atomic_fetch_dec(v)         atomic_fetch_sub(1, v)
#define atomic_fetch_dec_relaxed(v) atomic_fetch_sub_relaxed(1, v)
#define atomic_fetch_dec_acquire(v) atomic_fetch_sub_acquire(1, v)
#define atomic_fetch_dec_release(v) atomic_fetch_sub_release(1, v)

#define atomic_xchg(x, i)         xchg(&(x)->counter, i)
#define atomic_xchg_relaxed(x, i) xchg_relaxed(&(x)->counter, i)
#define atomic_xchg_release(x, i) xchg_release(&(x)->counter, i)
#define atomic_xchg_acquire(x, i) xchg_acquire(&(x)->counter, i)
#define atomic_cmpxchg(x, o, n)         cmpxchg(&(x)->counter, o, n)
#define atomic_cmpxchg_relaxed(x, o, n) cmpxchg_relaxed(&(x)->counter, o, n)
#define atomic_cmpxchg_acquire(x, o, n) cmpxchg_acquire(&(x)->counter, o, n)
#define atomic_cmpxchg_release(x, o, n) cmpxchg_release(&(x)->counter, o, n)

#define atomic_sub_and_test(i, v) (atomic_sub_return(i, v) == 0)
#define atomic_dec_and_test(v)    (atomic_dec_return(v) == 0)
#define atomic_inc_and_test(v)    (atomic_inc_return(v) == 0)
#define atomic_add_negative(i, v) (atomic_add_return(i, v) < 0)

#endif /* __LKMM_H__ */
