#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>

void exit(int);

void abort(void);
int abs(int);
int atoi(const char *);
void free(void *);
char *getenv(const char *);
void *malloc(size_t);

#endif /* __STDLIB_H__ */
