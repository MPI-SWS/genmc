#ifndef __GENMC_H__
#define __GENMC_H__

#include <genmc_internal.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Blocks the current execution if the argument is false
 */
void __VERIFIER_assume(int) __attribute__ ((__nothrow__));

/*
 * Models a limited amount of non-determinism by returning
 * a pseudo-random sequence of integers. This sequence
 * is always the same per execution for each thread
 */
int __VERIFIER_nondet_int(void) __attribute__ ((__nothrow__));

/*
 * An (opaque) datatype used by GenMC's thread functions,
 * __VERIFIER_spawn* and __VERIFIER_join (see below).
 */
typedef __VERIFIER_thread_t __VERIFIER_thread_t;

/*
 * Spawns a thread with argument ARG running FUNC.
 * Returns the spawned thread's TID.
 */
__attribute__ ((__nothrow__, always_inline)) static inline
__VERIFIER_thread_t __VERIFIER_spawn (void *(*__start_routine) (void *),
				      void *__restrict __arg)
{
	return __VERIFIER_thread_create(NULL, __start_routine, __arg);
}

/*
 * Spawns a thread with argument ARG running FUNC as symmetric to TH.
 * Returns the spawned thread's TID.
 */
__attribute__ ((__nothrow__, always_inline)) static inline
__VERIFIER_thread_t __VERIFIER_spawn_symmetric (void *(*__start_routine) (void *),
						void *__restrict __arg,
						__VERIFIER_thread_t __th)
{
	return __VERIFIER_thread_create_symmetric(NULL, __start_routine, __arg, __th);
}

/*
 * Joins thread TH and returns its result.
 */
__attribute__ ((always_inline, always_inline)) static inline
void *__VERIFIER_join (__VERIFIER_thread_t __th)
{
	return __VERIFIER_thread_join(__th);
}

/*
 * Helper function that joins thread TH, which was spawned as symmetric.
 * This function merely ensures that the result of the joined thread is
 * not going to be used.
 */
__attribute__ ((always_inline, always_inline)) static inline
void __VERIFIER_join_symmetric (__VERIFIER_thread_t __th)
{
	__VERIFIER_join(__th);
}

/*
 * Marker functions that can be used to mark the
 * beginning and end of spinloops that are not automatically
 * transformed to assume() statements by GenMC.
 *
 * 1) __VERIFIER_loop_begin() marks the beginning of a loop
 * 2) __VERIFIER_spin_start() marks the beginning of an iteration
 * 3) __VERIFIER_spin_end(cond) ends a given loop iteration if
 *   COND does not hold
 *
 * Example usage:
 *
 *     for (__VERIFIER_loop_begin();
 *          __VERIFIER_spin_start(), ..., __VERIFIER_spin_end(...);
 *          ...) {
 *     // no side-effects, preferrably empty
 * }
 *
 * NOTE: These should _not_ be used on top of the
 * automatic spin-assume transformation.
 */
void __VERIFIER_loop_begin(void) __attribute__ ((__nothrow__));
void __VERIFIER_spin_start(void) __attribute__ ((__nothrow__));
void __VERIFIER_spin_end(int) __attribute__ ((__nothrow__));

/*
 * Marker function that denotes that a store is local.
 * Since non-local stores may preclude the spin-assume transformation,
 * this marker serves as a hint to GenMC.
 */
#define __VERIFIER_local_write(s)			\
do {							\
	__VERIFIER_annotate_begin(GENMC_ATTR_LOCAL);	\
	s;						\
	__VERIFIER_annotate_end(GENMC_ATTR_LOCAL);	\
} while (0)

/*
 * Marker function that denotes that a store is final, that is,
 * that the location of the store will not be assigned again.
 */
#define __VERIFIER_final_write(s)			\
do {							\
	__VERIFIER_annotate_begin(GENMC_ATTR_FINAL);	\
	s;						\
	__VERIFIER_annotate_end(GENMC_ATTR_FINAL);	\
} while (0)

/*
 * Helping instructions. They take as argument a CAS instruction that
 * has a "helping" role (e.g., tail advancement in Michael-Scott
 * queue), and can aid in state-space reduction. They do not return
 * any value.
 * See the manual for a lengthier explanation.
 */
#define __VERIFIER_helped_CAS(c)			\
do {							\
	__VERIFIER_annotate_begin(GENMC_KIND_HELPED);	\
	c;						\
	__VERIFIER_annotate_end(GENMC_KIND_HELPED);	\
} while (0)
#define __VERIFIER_helping_CAS(c)			\
do {							\
	__VERIFIER_annotate_begin(GENMC_KIND_HELPING);	\
	c;						\
	__VERIFIER_annotate_end(GENMC_KIND_HELPING);	\
} while (0)


/*
 * Marker instructions. They take as an argument a read and
 * a CAS instruction (respectively), that mark the beginning/end
 * of a large atomic operation (loop). For example:
 *
 * int old, new;
 * do {
 *     old = __VERIFIER_speculative_read(
 *         atomic_load_explicit(&x, ...);
 *     );
 *     new = old * 42;
 * while (!__VERIFIER_confirming_CAS(
 *            atomic_compare_exchange_strong_explicit(&x, &old, new, ...);
 *       ))
 *
 * NOTE: These should _not_ be used on top of the automatic
 * automatic loop marking transformation.
 */
#define __VERIFIER_speculative_read(c)			\
({							\
	__VERIFIER_annotate_begin(GENMC_KIND_SPECUL);	\
	__auto_type __ret = c;				\
	__VERIFIER_annotate_end(GENMC_KIND_SPECUL);	\
	__ret;						\
})
#define __VERIFIER_confirming_read(c)			\
({							\
	__VERIFIER_annotate_begin(GENMC_KIND_CONFIRM);	\
	__auto_type __ret = c;				\
	__VERIFIER_annotate_end(GENMC_KIND_CONFIRM);	\
	__ret;						\
})
#define __VERIFIER_confirming_CAS(c)			\
({							\
	__VERIFIER_annotate_begin(GENMC_KIND_CONFIRM);	\
	__auto_type __ret = c;				\
	__VERIFIER_annotate_end(GENMC_KIND_CONFIRM);	\
	__ret;						\
})

#define __VERIFIER_final_CAS(c)				\
({							\
	__VERIFIER_annotate_begin(GENMC_ATTR_FINAL);	\
	__auto_type __ret = c;				\
	__VERIFIER_annotate_end(GENMC_ATTR_FINAL);	\
	__ret;						\
})


/*
 * A block of code enclosed in __VERIFIER_optional() is a hint to
 * GenMC that the contents of the block are not crucial for verifying
 * safety properties of the program (e.g., sleep-waiting, no-op
 * functions used when spinning, etc). Using __VERIFIER_optional()
 * should lead to a reduced state space.
 *
 * NOTE: The contents of the optional block should _not_ write to
 * memory or modify variables that are live after the block.
 */
#define __VERIFIER_optional(x)				\
do {							\
	if (__VERIFIER_opt_begin()) {			\
		x;					\
		__VERIFIER_assume(0);			\
	}						\
} while (0)


/*
 * An opaque type for hazard pointers.
 * The interface is similar to the one for atomic variables
 * (i.e., all operations on hazard pointers take the address
 * of a hazard pointer object as a parameter, etc)
 */
typedef __VERIFIER_hazptr_t __VERIFIER_hp_t;

/*
 * Allocates a hazard pointer (and links it to the global list of hazard pointers).
 * Returns an (opaque) __VERIFIER_hp_t.
 */
#define __VERIFIER_hp_alloc()						\
	__VERIFIER_hazptr_alloc()					\

/*
 * Protects P using HP; returns the value protected.
 * (HP needs to be the address of a hazard pointer, while
 * p needs to be the address of an (atomic) variable.)
 */
#define __VERIFIER_hp_protect(hp, p)					\
({									\
	void *_p_ = __atomic_load_n((void **) p, __ATOMIC_ACQUIRE);	\
	__VERIFIER_hazptr_protect(hp, _p_);				\
	_p_;								\
})

/*
 * Clears HP's protection
 */
#define __VERIFIER_hp_clear(hp)			\
	__VERIFIER_hazptr_clear(hp);

/*
 * Deallocates HP (and removes it from the global list).
 * (HP needs to be the address of a hazard pointer.)
 */
#define __VERIFIER_hp_free(hp)			\
	__VERIFIER_hazptr_free(hp)

/*
 * Deallocates P after it ensures that it is no longer protected.
 * This is equivalent to adding P to a list of items that will
 * eventually be freed.
 */
#define __VERIFIER_hp_retire(p)			\
	__VERIFIER_hazptr_retire(p)

/*
 * Specifies that D is going to be allocated in persistent storage
 * (i.e., the respective variable is going to be durable).
 */
#define __VERIFIER_persistent_storage(d)		\
	__attribute__ ((section ("__genmc_persist"))) d

/*
 * Dynamically allocates a new variable with durable storage.
 */
void *__VERIFIER_palloc(size_t);

/*
 * The signature of a recovery routine to be specified
 * by the user. This routine will run after each execution,
 * if the checker is run with the respective flags enabled.
 */
void __VERIFIER_recovery_routine(void) __attribute__ ((__nothrow__));

/*
 * All the file opeartions before this barrier will
 * have persisted to memory when the recovery routine runs.
 * Should be used only once.
 */
void __VERIFIER_pbarrier(void) __attribute__ ((__nothrow__));

#ifdef __cplusplus
}
#endif

#endif /* __GENMC_H__ */
