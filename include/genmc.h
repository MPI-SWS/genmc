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
void __VERIFIER_assume(int);

/*
 * Models a limited amount of non-determinism by returning
 * a pseudo-random sequence of integers. This sequence
 * is always the same per execution for each thread
 */
int __VERIFIER_nondet_int(void);

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
void __VERIFIER_loop_begin(void);
void __VERIFIER_spin_start(void);
void __VERIFIER_spin_end(int);

/*
 * Marker function that denotes that a store is local.
 * Since non-local stores may preclude the spin-assume transformation,
 * this marker serves as a hint to GenMC.
 */
#define __VERIFIER_local_write(s)			\
do {							\
	__VERIFIER_annotate_write(GENMC_ATTR_LOCAL);	\
	s;						\
} while (0)

/*
 * Marker function that denotes that a store is final, that is,
 * that the location of the store will not be assigned again.
 */
#define __VERIFIER_final_write(s)			\
do {							\
	__VERIFIER_annotate_write(GENMC_ATTR_FINAL);	\
	s;						\
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
	__VERIFIER_annotate_CAS(GENMC_KIND_HELPED);	\
	c;						\
} while (0)
#define __VERIFIER_helping_CAS(c)			\
do {							\
	__VERIFIER_annotate_CAS(GENMC_KIND_HELPING);	\
	c;						\
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
	__VERIFIER_annotate_read(GENMC_KIND_SPECUL);	\
	c;						\
})
#define __VERIFIER_confirming_read(c)			\
({							\
	__VERIFIER_annotate_read(GENMC_KIND_CONFIRM);	\
	c;						\
})
#define __VERIFIER_confirming_CAS(c)			\
({							\
	__VERIFIER_annotate_CAS(GENMC_KIND_CONFIRM);	\
	c;						\
})

#define __VERIFIER_final_CAS(c)				\
({							\
	__VERIFIER_annotate_CAS(GENMC_ATTR_FINAL);	\
	c;						\
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
 * The signature of a recovery routine to be specified
 * by the user. This routine will run after each execution,
 * if the checker is run with the respective flags enabled
 */
void __VERIFIER_recovery_routine(void);

/*
 * All the file opeartions before this barrier will
 * have persisted to memory when the recovery routine runs.
 * Should be used only once.
 */
void __VERIFIER_pbarrier(void);

#ifdef __cplusplus
}
#endif

#endif /* __GENMC_H__ */
