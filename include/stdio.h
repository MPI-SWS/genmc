#ifndef __STDIO_H__
#define __STDIO_H__

#include <errno.h>
#include <stddef.h>

struct _IO_FILE;
typedef struct _IO_FILE FILE;

extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;

extern int fclose(FILE *);
extern int fflush(FILE *);
extern FILE *fopen(const char *, const char *);
extern int printf(const char *, ...);
extern int fprintf(FILE *, const char *, ...);
extern size_t fwrite(const void *, size_t, size_t, FILE *);

#endif /* __STDIO_H__ */
