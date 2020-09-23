#ifndef __GENMC_H__
#define __GENMC_H__

/*
 * If the argument is not true, blocks the execution
 */
void __VERIFIER_assume(int);

/*
 * Models a limited amount of non-determinism by returning
 * a pseudo-random sequence of integers. This sequence
 * is always the same per execution for each thread
 */
int __VERIFIER_nondet_int(void);

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

#endif /* __GENMC_H__ */
