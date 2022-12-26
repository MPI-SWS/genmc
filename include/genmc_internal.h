#ifndef __GENMC_INTERNAL_H__
#define __GENMC_INTERNAL_H__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Internal declarations for GenMC -- should not be used by user programs */

#ifdef __cplusplus
extern "C"
{
#endif

/* Data types and variables */

typedef long __VERIFIER_thread_t;
typedef struct { int __private; } __VERIFIER_attr_t;

typedef struct { int __private; } __VERIFIER_barrier_t;
typedef int __VERIFIER_barrierattr_t;

typedef struct { int __private; } __VERIFIER_mutex_t;
typedef long __VERIFIER_mutexattr_t;

#define __VERIFIER_MUTEX_INITIALIZER { 0 }

typedef struct { void *__dummy; } __VERIFIER_hazptr_t;

#ifndef __cplusplus

#ifndef __CONFIG_GENMC_INODE_DATA_SIZE
# error "Internal error: inode size not defined!"
#endif

#define __genmc __attribute__((address_space(42)))

struct __VERIFIER_inode {
	/* VFS */
	__VERIFIER_mutex_t lock; // setupFsInfo() + interp rely on the layout
	int i_size; // <-- need to treat this atomically

	/* journaling helpers (embedded avoid indirection) */
	int i_transaction; // <-- writes protected by the lock above

	/* ext4 disk data (embedded to avoid indirection)
	 * The implementation can now accommodate only one metadata piece
	 * due to the current metadata block mapping */
	int i_disksize; // <-- used as metadata block mapping
	char data[__CONFIG_GENMC_INODE_DATA_SIZE]; // <-- used as data block mapping
};

struct __VERIFIER_file {
	struct inode *inode;
	unsigned int count; // need to manipulate atomically
	unsigned int flags; // encompasses f_mode
	__VERIFIER_mutex_t pos_lock;
	int pos;
};

struct __VERIFIER_inode __genmc __genmc_dir_inode;
struct __VERIFIER_file __genmc __genmc_dummy_file;

#endif /* __cplusplus */

/* assert */

extern void __VERIFIER_assert_fail(const char *, const char *, int) __attribute__ ((__nothrow__));


/* stdlib */

extern void __VERIFIER_free(void *) __attribute__ ((__nothrow__));

extern void *__VERIFIER_malloc(size_t) __attribute__ ((__nothrow__));

extern void *__VERIFIER_malloc_aligned(size_t, size_t) __attribute__ ((__nothrow__));

extern int __VERIFIER_atexit(void (*func)(void)) __attribute__ ((__nothrow__));


/* fcntl */

extern int __VERIFIER_openFS (const char *__file, int __oflag, mode_t __mode) __attribute__ ((__nothrow__));

extern int __VERIFIER_creatFS (const char *__file, mode_t __mode) __attribute__ ((__nothrow__));

extern int __VERIFIER_renameFS (const char *__old, const char *__new) __attribute__ ((__nothrow__));


/* unistd */

extern __off_t __VERIFIER_lseekFS (int __fd, __off_t __offset, int __whence) __attribute__ ((__nothrow__));

extern int __VERIFIER_closeFS (int __fd) __attribute__ ((__nothrow__));

extern ssize_t __VERIFIER_readFS (int __fd, void *__buf, size_t __nbytes) __attribute__ ((__nothrow__));

extern ssize_t __VERIFIER_writeFS (int __fd, const void *__buf, size_t __n) __attribute__ ((__nothrow__));

extern ssize_t __VERIFIER_preadFS (int __fd, void *__buf, size_t __nbytes,
				   __off_t __offset) __attribute__ ((__nothrow__));

extern ssize_t __VERIFIER_pwriteFS (int __fd, const void *__buf, size_t __n,
				    __off_t __offset) __attribute__ ((__nothrow__));

extern int __VERIFIER_linkFS (const char *__from, const char *__to) __attribute__ ((__nothrow__));

extern int __VERIFIER_unlinkFS (const char *__name) __attribute__ ((__nothrow__));

extern int __VERIFIER_fsyncFS (int __fd) __attribute__ ((__nothrow__));

extern void __VERIFIER_syncFS (void) __attribute__ ((__nothrow__));

extern int __VERIFIER_truncateFS (const char *__file, __off_t __length) __attribute__ ((__nothrow__));


/* Thread functions */

extern int __VERIFIER_thread_create (const __VERIFIER_attr_t *__restrict __attr,
				     void *(*__start_routine) (void *),
				     void *__restrict __arg) __attribute__ ((__nothrow__));

extern void __VERIFIER_thread_exit (void *__retval) __attribute__ ((__noreturn__)) __attribute__ ((__nothrow__));

extern int __VERIFIER_thread_join (__VERIFIER_thread_t __th, void **__thread_return) __attribute__ ((__nothrow__));

extern __VERIFIER_thread_t __VERIFIER_thread_self (void) __attribute__ ((__nothrow__));


/* Mutex functions */

extern int __VERIFIER_mutex_init (__VERIFIER_mutex_t *__mutex,
				  const __VERIFIER_mutexattr_t *__mutexattr) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_destroy (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_trylock (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_lock (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));

extern int __VERIFIER_mutex_unlock (__VERIFIER_mutex_t *__mutex) __attribute__ ((__nothrow__));


/* barrier functions */

extern int __VERIFIER_barrier_init (__VERIFIER_barrier_t *__restrict __barrier,
				    const __VERIFIER_barrierattr_t *__restrict __attr,
				    unsigned int __count) __attribute__ ((__nothrow__));

extern int __VERIFIER_barrier_destroy (__VERIFIER_barrier_t *__barrier) __attribute__ ((__nothrow__));

extern int __VERIFIER_barrier_wait (__VERIFIER_barrier_t *__barrier) __attribute__ ((__nothrow__));

extern int __VERIFIER_barrier_destroy (__VERIFIER_barrier_t *__barrier) __attribute__ ((__nothrow__));


/* Custom opcode implementations */

#define GENMC_ATTR_LOCAL   0x00000001
#define GENMC_ATTR_FINAL   0x00000002

#define GENMC_KIND_NONVR   0x00010000
#define GENMC_KIND_HELPED  0x00020000
#define GENMC_KIND_HELPING 0x00040000
#define GENMC_KIND_SPECUL  0x00080000
#define GENMC_KIND_CONFIRM 0x00100000

/*
 * Annotate a subsequent instruction with the given mask.
 */
extern void __VERIFIER_annotate_begin(int mask) __attribute__ ((__nothrow__));
extern void __VERIFIER_annotate_end(int mask) __attribute__ ((__nothrow__));

/* Marks the beginning of an optional block. */
extern int __VERIFIER_opt_begin(void) __attribute__ ((__nothrow__));

/* Hazard pointer functions */
extern __VERIFIER_hazptr_t *__VERIFIER_hazptr_alloc(void)  __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_protect(__VERIFIER_hazptr_t *hp, void *p) __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_clear(__VERIFIER_hazptr_t *hp) __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_free(__VERIFIER_hazptr_t *hp) __attribute__ ((__nothrow__));
extern void __VERIFIER_hazptr_retire(void *p) __attribute__ ((__nothrow__));

/* RCU functions */
extern void __VERIFIER_rcu_read_lock(void) __attribute__ ((__nothrow__));
extern void __VERIFIER_rcu_read_unlock(void) __attribute__ ((__nothrow__));
extern void __VERIFIER_synchronize_rcu(void) __attribute__ ((__nothrow__));

#ifdef __cplusplus
}
#endif

#endif /* __GENMC_INTERNAL_H__ */
