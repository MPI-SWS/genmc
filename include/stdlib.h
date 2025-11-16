#ifndef __STDLIB_H__
#define __STDLIB_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <assert.h>
#include <genmc_internal.h>

void exit(int);

static inline __attribute__((always_inline))
int atexit(void (*func)(void))
{
	return __VERIFIER_atexit(func);
}

void abort(void);
int abs(int);
int atoi(const char *);
char *getenv(const char *);

static inline __attribute__((always_inline))
void free(void *ptr)
{
	return __VERIFIER_free(ptr);
}

static inline __attribute__((always_inline))
void *malloc(size_t size)
{
	assert(size > 0 && "Zero size in malloc()");
	return __VERIFIER_malloc(size);
}

static inline __attribute__((always_inline))
void *aligned_alloc(size_t align, size_t size)
{
	assert(align > 0 && (align & (align - 1)) == 0 && "Invalid alignment in aligned_alloc()");
	assert(size > 0 && (size % align) == 0 && "Invalid size in aligned_alloc()");
	return __VERIFIER_malloc_aligned(align, size);
}

#ifdef __cplusplus
}
#endif

#endif /* __STDLIB_H__ */
