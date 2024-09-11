/*
 * "Fake" declarations to scaffold a Linux-kernel SMP environment.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <mixaskok@gmail.com>
 */

#ifndef __FAKE_H
#define __FAKE_H

#include <assert.h>
#include <stdatomic.h>
#include "ordering.h"

/* Stub some compiler directives */
#ifndef __always_inline
#define __always_inline
#endif

#define __percpu
#define __pure

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define prefetchw(x) do {} while (0)

#define EXPORT_SYMBOL(sym)

/* Various data types */
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;

typedef int8_t __s8;
typedef uint8_t __u8;
typedef int16_t __s16;
typedef uint16_t __u16;
typedef int32_t __s32;
typedef uint32_t __u32;
typedef int64_t __s64;
typedef uint64_t __u64;


/* "Cheater" options based on specific Kconfig build */
#ifndef CONFIG_NR_CPUS
#define CONFIG_NR_CPUS (1U << 14) /* We want 1 pending bit only! */
#endif
#ifndef NR_CPUS
#define NR_CPUS 2
#endif

#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN
#endif

/* BUG() statements and relatives */
#define BUG() assert(0)
#define BUG_ON(x) assert(!(x))
#define BUILD_BUG_ON(x) BUG_ON(x)


/* Some basic cpu stuff */
int __thread __running_cpu;
#define get_cpu()    ({ __running_cpu; })
#define set_cpu(cpu) ({ __running_cpu = cpu; })

#define smp_processor_id() get_cpu()

#define DEFINE_PER_CPU(type, name) type name[NR_CPUS];
#define DEFINE_PER_CPU_ALIGNED(type, name) DEFINE_PER_CPU(type, name)

#define per_cpu_ptr(ptr, cpu) (&(ptr)[cpu])
#define this_cpu_ptr(ptr)     per_cpu_ptr(ptr, get_cpu())

#define preempt_enable()  do {} while (0)
#define preempt_disable() do {} while (0)


/* RC11 semantics for memory barriers */
#define barrier() atomic_signal_fence(mo_acq_rel)
#define smp_mb()  atomic_thread_fence(mo_seq_cst)

#define smp_rmb() atomic_thread_fence(mo_acq_rel)
#define smp_wmb() atomic_thread_fence(mo_acq_rel)

#define smp_read_barrier_depends()    do {} while (0)
#define smp_acquire__after_ctrl_dep() barrier() /* no load speculation */

#define smp_cond_load_acquire(ptr, cond_expr) ({		\
	typeof(ptr) __PTR = (ptr);				\
	typeof(*ptr) VAL;					\
	for (;;) {						\
		VAL = READ_ONCE(*__PTR);			\
		if (cond_expr)					\
			break;					\
		cpu_relax();					\
	}							\
	smp_acquire__after_ctrl_dep();				\
	VAL;							\
})


/* Accessing shared memory */
#define __READ_ONCE_SIZE						\
({									\
	switch (size) {							\
	case 1: *(__u8 *)res = *(volatile __u8 *)p; break;		\
	case 2: *(__u16 *)res = *(volatile __u16 *)p; break;		\
	case 4: *(__u32 *)res = *(volatile __u32 *)p; break;		\
	case 8: *(__u64 *)res = *(volatile __u64 *)p; break;		\
	default:							\
		barrier();						\
		__builtin_memcpy((void *)res, (const void *)p, size);	\
		barrier();						\
	}								\
})

static __always_inline
void __read_once_size(const volatile void *p, void *res, int size)
{
	__READ_ONCE_SIZE;
}

#ifdef CONFIG_KASAN
/*
 * We can't declare function 'inline' because __no_sanitize_address confilcts
 * with inlining. Attempt to inline it may cause a build failure.
 * 	https://gcc.gnu.org/bugzilla/show_bug.cgi?id=67368
 * '__maybe_unused' allows us to avoid defined-but-not-used warnings.
 */
# define __no_kasan_or_inline __no_sanitize_address __maybe_unused
#else
# define __no_kasan_or_inline __always_inline
#endif

static __no_kasan_or_inline
void __read_once_size_nocheck(const volatile void *p, void *res, int size)
{
	__READ_ONCE_SIZE;
}

static __always_inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile __u8 *)p = *(__u8 *)res; break;
	case 2: *(volatile __u16 *)p = *(__u16 *)res; break;
	case 4: *(volatile __u32 *)p = *(__u32 *)res; break;
	case 8: *(volatile __u64 *)p = *(__u64 *)res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

/*
 * Prevent the compiler from merging or refetching reads or writes. The
 * compiler is also forbidden from reordering successive instances of
 * READ_ONCE and WRITE_ONCE, but only when the compiler is aware of some
 * particular ordering. One way to make the compiler aware of ordering is to
 * put the two invocations of READ_ONCE or WRITE_ONCE in different C
 * statements.
 *
 * These two macros will also work on aggregate data types like structs or
 * unions. If the size of the accessed data type exceeds the word size of
 * the machine (e.g., 32 bits or 64 bits) READ_ONCE() and WRITE_ONCE() will
 * fall back to memcpy(). There's at least two memcpy()s: one for the
 * __builtin_memcpy() and then one for the macro doing the copy of variable
 * - '__u' allocated on the stack.
 *
 * Their two major use cases are: (1) Mediating communication between
 * process-level code and irq/NMI handlers, all running on the same CPU,
 * and (2) Ensuring that the compiler does not fold, spindle, or otherwise
 * mutilate accesses that either do not require ordering or that interact
 * with an explicit memory barrier or atomic instruction that provides the
 * required ordering.
 */
#include <asm/barrier.h>
#include <linux/kasan-checks.h>

#define __READ_ONCE(x, check)						\
({									\
	union { typeof(x) __val; char __c[1]; } __u;			\
	if (check)							\
		__read_once_size(&(x), __u.__c, sizeof(x));		\
	else								\
		__read_once_size_nocheck(&(x), __u.__c, sizeof(x));	\
	smp_read_barrier_depends(); /* Enforce dependency ordering from x */ \
	__u.__val;							\
})
#define READ_ONCE(x) __READ_ONCE(x, 1)

/*
 * Use READ_ONCE_NOCHECK() instead of READ_ONCE() if you need
 * to hide memory access from KASAN.
 */
#define READ_ONCE_NOCHECK(x) __READ_ONCE(x, 0)

/* static __no_kasan_or_inline */
/* unsigned long read_word_at_a_time(const void *addr) */
/* { */
/* 	kasan_check_read(addr, 1); */
/* 	return *(unsigned long *)addr; */
/* } */

#define WRITE_ONCE(x, val) \
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (__force typeof(x)) (val) }; \
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})


/* Locks & lockdep */
#define CONFIG_DEBUG_SPINLOCK
#undef CONFIG_DEBUG_LOCK_ALLOC
#undef CONFIG_LOCKDEP

#define lock_acquire(l, s, t, r, c, n, i)	do { } while (0)
#define lock_release(l, n, i)			do { } while (0)

#define lock_acquire_exclusive(l, s, t, n, i)		lock_acquire(l, s, t, 0, 1, n, i)
#define lock_acquire_shared(l, s, t, n, i)		lock_acquire(l, s, t, 1, 1, n, i)
#define lock_acquire_shared_recursive(l, s, t, n, i)	lock_acquire(l, s, t, 2, 1, n, i)

#define seqcount_acquire(l, s, t, i)		lock_acquire_exclusive(l, s, t, NULL, i)
#define seqcount_acquire_read(l, s, t, i)	lock_acquire_shared_recursive(l, s, t, NULL, i)
#define seqcount_release(l, n, i)		lock_release(l, n, i)

#define lockdep_init_map(...) do {} while (0)
struct lock_class_key {};

typedef pthread_mutex_t spinlock_t;
#define SPINLOCK_INITIALIZER PTHREAD_MUTEX_INITIALIZER
#define __SPIN_LOCK_UNLOCKED(lockname) PTHREAD_MUTEX_INITIALIZER

void spin_lock(spinlock_t *l)
{
	preempt_disable();
	if (pthread_mutex_lock(l))
		exit(-1);
}

void spin_unlock(spinlock_t *l)
{
	if (pthread_mutex_unlock(l))
		exit(-1);
	preempt_enable();
}
/* IRQs can be modeled, but do not bother for this example */
#define spin_lock_irq(l)          spin_lock(l)
#define spin_unlock_irq(l)        spin_unlock(l)
#define spin_lock_irqsave(l, flags)      spin_lock(l)
#define spin_unlock_irqrestore(l, flags) spin_unlock(l)
#define spin_lock_bh(l)   spin_lock(l)
#define spin_unlock_bh(l) spin_unlock(l)


/* RC11 semantics for various atomic types and ops */
typedef struct {
	atomic_int counter;
} atomic_t;

#define ATOMIC_INIT(i) { ATOMIC_VAR_INIT(i) }

#define atomic_read(v)   atomic_load_explicit(&(v)->counter, mo_relaxed)
#define atomic_add(i, v) atomic_fetch_add_explicit(&(v)->counter, i, mo_relaxed)

#define smp_store_release(p, v)			        \
	atomic_store_explicit(p, v, mo_release)
#define smp_load_acquire(p)			        \
	atomic_load_explicit(p, mo_acquire)

#define xchg(p, v)					\
	atomic_exchange_explicit(p, v, mo_acq_rel)

#define __cmpxchg(ptr, old, new, ord)		        \
({					                \
	__typeof__(old) __old = (old);			\
	atomic_compare_exchange_strong_explicit(ptr,	\
				&__old, new, ord, ord);	\
	__old;						\
})
#define cmpxchg_relaxed(...) __cmpxchg(__VA_ARGS__, mo_relaxed)
#define cmpxchg_acquire(...) __cmpxchg(__VA_ARGS__, mo_acquire)
#define cmpxchg_release(...) __cmpxchg(__VA_ARGS__, mo_release)
#define cmpxchg_acq_rel(...) __cmpxchg(__VA_ARGS__, mo_acq_rel)
#define cmpxchg(...) cmpxchg_acq_rel(__VA_ARGS__) /* should suffice... */

#define atomic_cmpxchg(v, o, n) (cmpxchg(&((v)->counter), (o), (n)))
#define atomic_cmpxchg_relaxed(v, o, n) \
	cmpxchg_relaxed(&((v)->counter), (o), (n))
#define atomic_cmpxchg_acquire(v, o, n) \
	cmpxchg_acquire(&((v)->counter), (o), (n))
#define atomic_cmpxchg_release(v, o, n) \
	cmpxchg_release(&((v)->counter), (o), (n))

#define __atomic_sub_return(val, ptr, ord)		\
({						        \
	__typeof__(val) __ret;				\
	__ret = atomic_fetch_sub_explicit(&(ptr)->counter, val, ord);	\
	__ret = __ret - val;				\
	__ret;						\
})
#define atomic_sub_return_relaxed(...)			\
	__atomic_sub_return(__VA_ARGS__, mo_relaxed)
#define atomic_sub_return_acquire(...)			\
	__atomic_sub_return(__VA_ARGS__, mo_acquire)
#define atomic_sub_return_release(...)			\
	__atomic_sub_return(__VA_ARGS__, mo_release)
#define atomic_sub_return_acq_rel(...)			\
	__atomic_sub_return(__VA_ARGS__, mo_acq_rel)
#define atomic_sub_return(...)				\
	atomic_sub_return_acq_rel(__VA_ARGS__) /* should suffice... */


/* Normally a REP NOP, but do not bother with cpu stuff */
#define cpu_relax() do {} while (0)

#endif /* __FAKE_H */
