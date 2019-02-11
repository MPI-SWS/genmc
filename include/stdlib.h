#ifndef __STDLIB_H__
#define __STDLIB_H__

#undef NULL
#define NULL ((void *)0)

typedef __SIZE_TYPE__ size_t;

void exit(int);

void abort(void) ;
int atoi(const char *);
void free(void *);
char *getenv(const char *);
void *malloc(size_t);

#endif /* __STDLIB_H__ */
