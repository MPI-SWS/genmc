#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>

void exit(int);

void abort(void);
int abs(int);
int atoi(const char *);
char *getenv(const char *);

void __VERIFIER_free(void *);
#define free __VERIFIER_free
void *__VERIFIER_malloc(size_t);
#define malloc __VERIFIER_malloc

#endif /* __STDLIB_H__ */
