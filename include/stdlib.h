#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>
#include <genmc_internal.h>

void exit(int);

void abort(void);
int abs(int);
int atoi(const char *);
char *getenv(const char *);

#define free(ptr) __VERIFIER_free(ptr)

#define malloc(size) __VERIFIER_malloc(size)

#define aligned_alloc(align, size) __VERIFIER_malloc_aligned(align, size)

#endif /* __STDLIB_H__ */
