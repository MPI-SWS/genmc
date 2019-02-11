/* Adapted from the sources of the Apache Portable Runtime library version 1.5.1:
 * https://apr.apache.org/
 */

/* BOUND 5 */
# 1 "main2.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 312 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "main2.c" 2





# 1 "./../test/abts.h" 1
# 21 "./../test/abts.h"
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdarg.h" 1 3
# 30 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdarg.h" 3
typedef __builtin_va_list va_list;
# 50 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdarg.h" 3
typedef __builtin_va_list __gnuc_va_list;
# 22 "./../test/abts.h" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 27 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 352 "/usr/include/features.h" 3 4
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 353 "/usr/include/features.h" 2 3 4
# 374 "/usr/include/features.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 1 3 4
# 385 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 386 "/usr/include/x86_64-linux-gnu/sys/cdefs.h" 2 3 4
# 375 "/usr/include/features.h" 2 3 4
# 398 "/usr/include/features.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 1 3 4
# 10 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/gnu/stubs-64.h" 1 3 4
# 11 "/usr/include/x86_64-linux-gnu/gnu/stubs.h" 2 3 4
# 399 "/usr/include/features.h" 2 3 4
# 28 "/usr/include/stdio.h" 2 3 4





# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 58 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 34 "/usr/include/stdio.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/types.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;







typedef long int __quad_t;
typedef unsigned long int __u_quad_t;
# 121 "/usr/include/x86_64-linux-gnu/bits/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/typesizes.h" 1 3 4
# 122 "/usr/include/x86_64-linux-gnu/bits/types.h" 2 3 4


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;
# 36 "/usr/include/stdio.h" 2 3 4








struct _IO_FILE;



typedef struct _IO_FILE FILE;
# 64 "/usr/include/stdio.h" 3 4
typedef struct _IO_FILE __FILE;
# 74 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/libio.h" 1 3 4
# 31 "/usr/include/libio.h" 3 4
# 1 "/usr/include/_G_config.h" 1 3 4
# 15 "/usr/include/_G_config.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 16 "/usr/include/_G_config.h" 2 3 4




# 1 "/usr/include/wchar.h" 1 3 4
# 82 "/usr/include/wchar.h" 3 4
typedef struct
{
  int __count;
  union
  {

    unsigned int __wch;



    char __wchb[4];
  } __value;
} __mbstate_t;
# 21 "/usr/include/_G_config.h" 2 3 4
typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;
# 32 "/usr/include/libio.h" 2 3 4
# 144 "/usr/include/libio.h" 3 4
struct _IO_jump_t; struct _IO_FILE;
# 154 "/usr/include/libio.h" 3 4
typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;
# 177 "/usr/include/libio.h" 3 4
};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};
# 245 "/usr/include/libio.h" 3 4
struct _IO_FILE {
  int _flags;




  char* _IO_read_ptr;
  char* _IO_read_end;
  char* _IO_read_base;
  char* _IO_write_base;
  char* _IO_write_ptr;
  char* _IO_write_end;
  char* _IO_buf_base;
  char* _IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;



  int _flags2;

  __off_t _old_offset;



  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];



  _IO_lock_t *_lock;
# 293 "/usr/include/libio.h" 3 4
  __off64_t _offset;
# 302 "/usr/include/libio.h" 3 4
  void *__pad1;
  void *__pad2;
  void *__pad3;
  void *__pad4;
  size_t __pad5;

  int _mode;

  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;
# 338 "/usr/include/libio.h" 3 4
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, const char *__buf,
     size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);
# 390 "/usr/include/libio.h" 3 4
extern int __underflow (_IO_FILE *);
extern int __uflow (_IO_FILE *);
extern int __overflow (_IO_FILE *, int);
# 434 "/usr/include/libio.h" 3 4
extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__ ));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__ ));

extern int _IO_peekc_locked (_IO_FILE *__fp);





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__ ));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__ ));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__ ));
# 464 "/usr/include/libio.h" 3 4
extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
   __gnuc_va_list, int *__restrict);
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
    __gnuc_va_list);
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t);
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t);

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int);
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int);

extern void _IO_free_backup_area (_IO_FILE *) __attribute__ ((__nothrow__ ));
# 75 "/usr/include/stdio.h" 2 3 4




typedef __gnuc_va_list va_list;
# 90 "/usr/include/stdio.h" 3 4
typedef __off_t off_t;
# 102 "/usr/include/stdio.h" 3 4
typedef __ssize_t ssize_t;







typedef _G_fpos_t fpos_t;
# 164 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stdio_lim.h" 1 3 4
# 165 "/usr/include/stdio.h" 2 3 4



extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;







extern int remove (const char *__filename) __attribute__ ((__nothrow__ ));

extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ ));




extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) __attribute__ ((__nothrow__ ));
# 195 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile (void) ;
# 209 "/usr/include/stdio.h" 3 4
extern char *tmpnam (char *__s) __attribute__ ((__nothrow__ )) ;





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__ )) ;
# 227 "/usr/include/stdio.h" 3 4
extern char *tempnam (const char *__dir, const char *__pfx)
     __attribute__ ((__nothrow__ )) __attribute__ ((__malloc__)) ;
# 237 "/usr/include/stdio.h" 3 4
extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);
# 252 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 272 "/usr/include/stdio.h" 3 4
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes) ;




extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) ;
# 306 "/usr/include/stdio.h" 3 4
extern FILE *fdopen (int __fd, const char *__modes) __attribute__ ((__nothrow__ )) ;
# 319 "/usr/include/stdio.h" 3 4
extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  __attribute__ ((__nothrow__ )) ;




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ )) ;






extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ ));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__ ));





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__ ));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__ ));
# 356 "/usr/include/stdio.h" 3 4
extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...);




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));





extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));
# 412 "/usr/include/stdio.h" 3 4
extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
# 425 "/usr/include/stdio.h" 3 4
extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) ;




extern int scanf (const char *__restrict __format, ...) ;

extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ ));
# 443 "/usr/include/stdio.h" 3 4
extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf") ;


extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf") ;

extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc99_sscanf") __attribute__ ((__nothrow__ ));
# 471 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) ;





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;


extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__ )) __attribute__ ((__format__ (__scanf__, 2, 0)));
# 494 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0))) ;
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")

     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vsscanf") __attribute__ ((__nothrow__ ))



     __attribute__ ((__format__ (__scanf__, 2, 0)));
# 531 "/usr/include/stdio.h" 3 4
extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);
# 550 "/usr/include/stdio.h" 3 4
extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
# 561 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream);
# 573 "/usr/include/stdio.h" 3 4
extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);
# 594 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);
# 622 "/usr/include/stdio.h" 3 4
extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
          ;
# 638 "/usr/include/stdio.h" 3 4
extern char *gets (char *__s) __attribute__ ((__deprecated__));
# 665 "/usr/include/stdio.h" 3 4
extern __ssize_t __getdelim (char **__restrict __lineptr,
          size_t *__restrict __n, int __delimiter,
          FILE *__restrict __stream) ;
extern __ssize_t getdelim (char **__restrict __lineptr,
        size_t *__restrict __n, int __delimiter,
        FILE *__restrict __stream) ;







extern __ssize_t getline (char **__restrict __lineptr,
       size_t *__restrict __n,
       FILE *__restrict __stream) ;
# 689 "/usr/include/stdio.h" 3 4
extern int fputs (const char *__restrict __s, FILE *__restrict __stream);





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s);
# 737 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream);
# 749 "/usr/include/stdio.h" 3 4
extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);
# 773 "/usr/include/stdio.h" 3 4
extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;
# 798 "/usr/include/stdio.h" 3 4
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, const fpos_t *__pos);
# 826 "/usr/include/stdio.h" 3 4
extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ ));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__ )) ;

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ )) ;




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__ ));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__ )) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__ )) ;
# 846 "/usr/include/stdio.h" 3 4
extern void perror (const char *__s);







# 1 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h" 3 4
extern int sys_nerr;
extern const char *const sys_errlist[];
# 854 "/usr/include/stdio.h" 2 3 4




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__ )) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__ )) ;
# 873 "/usr/include/stdio.h" 3 4
extern FILE *popen (const char *__command, const char *__modes) ;





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__ ));
# 913 "/usr/include/stdio.h" 3 4
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__ ));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__ )) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__ ));
# 23 "./../test/abts.h" 2
# 1 "/usr/include/stdlib.h" 1 3 4
# 32 "/usr/include/stdlib.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 86 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 3 4
typedef int wchar_t;
# 33 "/usr/include/stdlib.h" 2 3 4








# 1 "/usr/include/x86_64-linux-gnu/bits/waitflags.h" 1 3 4
# 50 "/usr/include/x86_64-linux-gnu/bits/waitflags.h" 3 4
typedef enum
{
  P_ALL,
  P_PID,
  P_PGID
} idtype_t;
# 42 "/usr/include/stdlib.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h" 1 3 4
# 64 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 36 "/usr/include/endian.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/endian.h" 1 3 4
# 37 "/usr/include/endian.h" 2 3 4
# 60 "/usr/include/endian.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 29 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 2 3 4






# 1 "/usr/include/x86_64-linux-gnu/bits/byteswap-16.h" 1 3 4
# 36 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 2 3 4
# 61 "/usr/include/endian.h" 2 3 4
# 65 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h" 2 3 4

union wait
  {
    int w_status;
    struct
      {

 unsigned int __w_termsig:7;
 unsigned int __w_coredump:1;
 unsigned int __w_retcode:8;
 unsigned int:16;







      } __wait_terminated;
    struct
      {

 unsigned int __w_stopval:8;
 unsigned int __w_stopsig:8;
 unsigned int:16;






      } __wait_stopped;
  };
# 43 "/usr/include/stdlib.h" 2 3 4
# 67 "/usr/include/stdlib.h" 3 4
typedef union
  {
    union wait *__uptr;
    int *__iptr;
  } __WAIT_STATUS __attribute__ ((__transparent_union__));
# 97 "/usr/include/stdlib.h" 3 4
typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;







__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
# 139 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__ )) ;




extern double atof (const char *__nptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern int atoi (const char *__nptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern long int atol (const char *__nptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





__extension__ extern long long int atoll (const char *__nptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





extern double strtod (const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern float strtof (const char *__restrict __nptr,
       char **__restrict __endptr) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));

extern long double strtold (const char *__restrict __nptr,
       char **__restrict __endptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern long int strtol (const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));

extern unsigned long int strtoul (const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int strtoq (const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





__extension__
extern long long int strtoll (const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 305 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) __attribute__ ((__nothrow__ )) ;


extern long int a64l (const char *__s)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





# 1 "/usr/include/x86_64-linux-gnu/sys/types.h" 1 3 4
# 33 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;




typedef __loff_t loff_t;



typedef __ino_t ino_t;
# 60 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;
# 98 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __pid_t pid_t;





typedef __id_t id_t;
# 115 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;
# 132 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 59 "/usr/include/time.h" 3 4
typedef __clock_t clock_t;
# 75 "/usr/include/time.h" 3 4
typedef __time_t time_t;
# 91 "/usr/include/time.h" 3 4
typedef __clockid_t clockid_t;
# 103 "/usr/include/time.h" 3 4
typedef __timer_t timer_t;
# 133 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4
# 146 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 147 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
# 194 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));
# 219 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/sys/select.h" 1 3 4
# 30 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/select.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/select.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/select.h" 2 3 4
# 31 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/bits/sigset.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/sigset.h" 3 4
typedef int __sig_atomic_t;




typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
# 34 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4



typedef __sigset_t sigset_t;






# 1 "/usr/include/time.h" 1 3 4
# 120 "/usr/include/time.h" 3 4
struct timespec
  {
    __time_t tv_sec;
    __syscall_slong_t tv_nsec;
  };
# 44 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/time.h" 1 3 4
# 30 "/usr/include/x86_64-linux-gnu/bits/time.h" 3 4
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
# 46 "/usr/include/x86_64-linux-gnu/sys/select.h" 2 3 4


typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 64 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 106 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 118 "/usr/include/x86_64-linux-gnu/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
# 220 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/sys/sysmacros.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/sys/sysmacros.h" 3 4
__extension__
extern unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
__extension__
extern unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
__extension__
extern unsigned long long int gnu_dev_makedev (unsigned int __major,
            unsigned int __minor)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
# 223 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4





typedef __blksize_t blksize_t;






typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 270 "/usr/include/x86_64-linux-gnu/sys/types.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 1 3 4
# 21 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 2 3 4
# 60 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 3 4
typedef unsigned long int pthread_t;


union pthread_attr_t
{
  char __size[56];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;





typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
# 90 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 3 4
typedef union
{
  struct __pthread_mutex_s
  {
    int __lock;
    unsigned int __count;
    int __owner;

    unsigned int __nusers;



    int __kind;

    short __spins;
    short __elision;
    __pthread_list_t __list;
# 124 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 3 4
  } __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  struct
  {
    int __lock;
    unsigned int __futex;
    __extension__ unsigned long long int __total_seq;
    __extension__ unsigned long long int __wakeup_seq;
    __extension__ unsigned long long int __woken_seq;
    void *__mutex;
    unsigned int __nwaiters;
    unsigned int __broadcast_seq;
  } __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;





typedef union
{

  struct
  {
    int __lock;
    unsigned int __nr_readers;
    unsigned int __readers_wakeup;
    unsigned int __writer_wakeup;
    unsigned int __nr_readers_queued;
    unsigned int __nr_writers_queued;
    int __writer;
    int __shared;
    unsigned long int __pad1;
    unsigned long int __pad2;


    unsigned int __flags;

  } __data;
# 211 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h" 3 4
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 271 "/usr/include/x86_64-linux-gnu/sys/types.h" 2 3 4
# 315 "/usr/include/stdlib.h" 2 3 4






extern long int random (void) __attribute__ ((__nothrow__ ));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__ ));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));






extern int rand (void) __attribute__ ((__nothrow__ ));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__ ));




extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__ ));







extern double drand48 (void) __attribute__ ((__nothrow__ ));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__ ));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__ ));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__ ));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    __extension__ unsigned long long int __a;

  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
# 466 "/usr/include/stdlib.h" 3 4
extern void *malloc (size_t __size) __attribute__ ((__nothrow__ )) __attribute__ ((__malloc__)) ;

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__ )) __attribute__ ((__malloc__)) ;
# 480 "/usr/include/stdlib.h" 3 4
extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__ )) __attribute__ ((__warn_unused_result__));

extern void free (void *__ptr) __attribute__ ((__nothrow__ ));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__ ));




# 1 "/usr/include/alloca.h" 1 3 4
# 24 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 25 "/usr/include/alloca.h" 2 3 4







extern void *alloca (size_t __size) __attribute__ ((__nothrow__ ));
# 493 "/usr/include/stdlib.h" 2 3 4





extern void *valloc (size_t __size) __attribute__ ((__nothrow__ )) __attribute__ ((__malloc__)) ;




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;
# 515 "/usr/include/stdlib.h" 3 4
extern void abort (void) __attribute__ ((__nothrow__ )) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 535 "/usr/include/stdlib.h" 3 4
extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__ )) __attribute__ ((__noreturn__));
# 557 "/usr/include/stdlib.h" 3 4
extern void _Exit (int __status) __attribute__ ((__nothrow__ )) __attribute__ ((__noreturn__));






extern char *getenv (const char *__name) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;
# 578 "/usr/include/stdlib.h" 3 4
extern int putenv (char *__string) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int setenv (const char *__name, const char *__value, int __replace)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (const char *__name) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) __attribute__ ((__nothrow__ ));
# 606 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 620 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
# 642 "/usr/include/stdlib.h" 3 4
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) ;
# 663 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;
# 717 "/usr/include/stdlib.h" 3 4
extern int system (const char *__command) ;
# 734 "/usr/include/stdlib.h" 3 4
extern char *realpath (const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__ )) ;






typedef int (*__compar_fn_t) (const void *, const void *);
# 755 "/usr/include/stdlib.h" 3 4
extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;







extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));
# 775 "/usr/include/stdlib.h" 3 4
extern int abs (int __x) __attribute__ ((__nothrow__ )) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__ )) __attribute__ ((__const__)) ;



__extension__ extern long long int llabs (long long int __x)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__)) ;







extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__)) ;




__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__)) ;
# 812 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3))) ;




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3))) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4, 5)));






extern int mblen (const char *__s, size_t __n) __attribute__ ((__nothrow__ ));


extern int mbtowc (wchar_t *__restrict __pwc,
     const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__ ));


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__ ));



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__ ));

extern size_t wcstombs (char *__restrict __s,
   const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__ ));
# 888 "/usr/include/stdlib.h" 3 4
extern int rpmatch (const char *__response) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;
# 899 "/usr/include/stdlib.h" 3 4
extern int getsubopt (char **__restrict __optionp,
        char *const *__restrict __tokens,
        char **__restrict __valuep)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2, 3))) ;
# 951 "/usr/include/stdlib.h" 3 4
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



# 1 "/usr/include/x86_64-linux-gnu/bits/stdlib-float.h" 1 3 4
# 956 "/usr/include/stdlib.h" 2 3 4
# 24 "./../test/abts.h" 2
# 1 "/usr/include/string.h" 1 3 4
# 32 "/usr/include/string.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 33 "/usr/include/string.h" 2 3 4
# 46 "/usr/include/string.h" 3 4
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
       size_t __n) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, const void *__src, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));






extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
        int __c, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));





extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 96 "/usr/include/string.h" 3 4
extern void *memchr (const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 129 "/usr/include/string.h" 3 4
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));







# 1 "/usr/include/xlocale.h" 1 3 4
# 27 "/usr/include/xlocale.h" 3 4
typedef struct __locale_struct
{

  struct __locale_data *__locales[13];


  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;


  const char *__names[13];
} *__locale_t;


typedef __locale_t locale_t;
# 164 "/usr/include/string.h" 2 3 4


extern int strcoll_l (const char *__s1, const char *__s2, __locale_t __l)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));

extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
    __locale_t __l) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 4)));





extern char *strdup (const char *__s)
     __attribute__ ((__nothrow__ )) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (const char *__string, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 236 "/usr/include/string.h" 3 4
extern char *strchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 263 "/usr/include/string.h" 3 4
extern char *strrchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 285 "/usr/include/string.h" 3 4
extern size_t strcspn (const char *__s, const char *__reject)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 315 "/usr/include/string.h" 3 4
extern char *strpbrk (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 342 "/usr/include/string.h" 3 4
extern char *strstr (const char *__haystack, const char *__needle)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));




extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));




extern char *__strtok_r (char *__restrict __s,
    const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
         char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3)));
# 399 "/usr/include/string.h" 3 4
extern size_t strlen (const char *__s)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern size_t strnlen (const char *__string, size_t __maxlen)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern char *strerror (int __errnum) __attribute__ ((__nothrow__ ));
# 427 "/usr/include/string.h" 3 4
extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));
# 445 "/usr/include/string.h" 3 4
extern char *strerror_l (int __errnum, __locale_t __l) __attribute__ ((__nothrow__ ));





extern void __bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern void bcopy (const void *__src, void *__dest, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 489 "/usr/include/string.h" 3 4
extern char *index (const char *__s, int __c)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 517 "/usr/include/string.h" 3 4
extern char *rindex (const char *__s, int __c)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern int ffs (int __i) __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
# 534 "/usr/include/string.h" 3 4
extern int strcasecmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 557 "/usr/include/string.h" 3 4
extern char *strsep (char **__restrict __stringp,
       const char *__restrict __delim)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) __attribute__ ((__nothrow__ ));


extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
   const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
# 25 "./../test/abts.h" 2



# 1 "/usr/include/unistd.h" 1 3 4
# 202 "/usr/include/unistd.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/posix_opt.h" 1 3 4
# 203 "/usr/include/unistd.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/environments.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/environments.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/environments.h" 2 3 4
# 207 "/usr/include/unistd.h" 2 3 4
# 226 "/usr/include/unistd.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 227 "/usr/include/unistd.h" 2 3 4
# 255 "/usr/include/unistd.h" 3 4
typedef __useconds_t useconds_t;
# 267 "/usr/include/unistd.h" 3 4
typedef __intptr_t intptr_t;






typedef __socklen_t socklen_t;
# 287 "/usr/include/unistd.h" 3 4
extern int access (const char *__name, int __type) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 304 "/usr/include/unistd.h" 3 4
extern int faccessat (int __fd, const char *__file, int __type, int __flag)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2))) ;
# 334 "/usr/include/unistd.h" 3 4
extern __off_t lseek (int __fd, __off_t __offset, int __whence) __attribute__ ((__nothrow__ ));
# 353 "/usr/include/unistd.h" 3 4
extern int close (int __fd);






extern ssize_t read (int __fd, void *__buf, size_t __nbytes) ;





extern ssize_t write (int __fd, const void *__buf, size_t __n) ;
# 376 "/usr/include/unistd.h" 3 4
extern ssize_t pread (int __fd, void *__buf, size_t __nbytes,
        __off_t __offset) ;






extern ssize_t pwrite (int __fd, const void *__buf, size_t __n,
         __off_t __offset) ;
# 417 "/usr/include/unistd.h" 3 4
extern int pipe (int __pipedes[2]) __attribute__ ((__nothrow__ )) ;
# 432 "/usr/include/unistd.h" 3 4
extern unsigned int alarm (unsigned int __seconds) __attribute__ ((__nothrow__ ));
# 444 "/usr/include/unistd.h" 3 4
extern unsigned int sleep (unsigned int __seconds);







extern __useconds_t ualarm (__useconds_t __value, __useconds_t __interval)
     __attribute__ ((__nothrow__ ));






extern int usleep (__useconds_t __useconds);
# 469 "/usr/include/unistd.h" 3 4
extern int pause (void);



extern int chown (const char *__file, __uid_t __owner, __gid_t __group)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;



extern int fchown (int __fd, __uid_t __owner, __gid_t __group) __attribute__ ((__nothrow__ )) ;




extern int lchown (const char *__file, __uid_t __owner, __gid_t __group)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;






extern int fchownat (int __fd, const char *__file, __uid_t __owner,
       __gid_t __group, int __flag)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2))) ;



extern int chdir (const char *__path) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;



extern int fchdir (int __fd) __attribute__ ((__nothrow__ )) ;
# 511 "/usr/include/unistd.h" 3 4
extern char *getcwd (char *__buf, size_t __size) __attribute__ ((__nothrow__ )) ;
# 525 "/usr/include/unistd.h" 3 4
extern char *getwd (char *__buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__)) ;




extern int dup (int __fd) __attribute__ ((__nothrow__ )) ;


extern int dup2 (int __fd, int __fd2) __attribute__ ((__nothrow__ ));
# 543 "/usr/include/unistd.h" 3 4
extern char **__environ;







extern int execve (const char *__path, char *const __argv[],
     char *const __envp[]) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));




extern int fexecve (int __fd, char *const __argv[], char *const __envp[])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));




extern int execv (const char *__path, char *const __argv[])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern int execle (const char *__path, const char *__arg, ...)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern int execl (const char *__path, const char *__arg, ...)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern int execvp (const char *__file, char *const __argv[])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));




extern int execlp (const char *__file, const char *__arg, ...)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
# 598 "/usr/include/unistd.h" 3 4
extern int nice (int __inc) __attribute__ ((__nothrow__ )) ;




extern void _exit (int __status) __attribute__ ((__noreturn__));






# 1 "/usr/include/x86_64-linux-gnu/bits/confname.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/confname.h" 3 4
enum
  {
    _PC_LINK_MAX,

    _PC_MAX_CANON,

    _PC_MAX_INPUT,

    _PC_NAME_MAX,

    _PC_PATH_MAX,

    _PC_PIPE_BUF,

    _PC_CHOWN_RESTRICTED,

    _PC_NO_TRUNC,

    _PC_VDISABLE,

    _PC_SYNC_IO,

    _PC_ASYNC_IO,

    _PC_PRIO_IO,

    _PC_SOCK_MAXBUF,

    _PC_FILESIZEBITS,

    _PC_REC_INCR_XFER_SIZE,

    _PC_REC_MAX_XFER_SIZE,

    _PC_REC_MIN_XFER_SIZE,

    _PC_REC_XFER_ALIGN,

    _PC_ALLOC_SIZE_MIN,

    _PC_SYMLINK_MAX,

    _PC_2_SYMLINKS

  };


enum
  {
    _SC_ARG_MAX,

    _SC_CHILD_MAX,

    _SC_CLK_TCK,

    _SC_NGROUPS_MAX,

    _SC_OPEN_MAX,

    _SC_STREAM_MAX,

    _SC_TZNAME_MAX,

    _SC_JOB_CONTROL,

    _SC_SAVED_IDS,

    _SC_REALTIME_SIGNALS,

    _SC_PRIORITY_SCHEDULING,

    _SC_TIMERS,

    _SC_ASYNCHRONOUS_IO,

    _SC_PRIORITIZED_IO,

    _SC_SYNCHRONIZED_IO,

    _SC_FSYNC,

    _SC_MAPPED_FILES,

    _SC_MEMLOCK,

    _SC_MEMLOCK_RANGE,

    _SC_MEMORY_PROTECTION,

    _SC_MESSAGE_PASSING,

    _SC_SEMAPHORES,

    _SC_SHARED_MEMORY_OBJECTS,

    _SC_AIO_LISTIO_MAX,

    _SC_AIO_MAX,

    _SC_AIO_PRIO_DELTA_MAX,

    _SC_DELAYTIMER_MAX,

    _SC_MQ_OPEN_MAX,

    _SC_MQ_PRIO_MAX,

    _SC_VERSION,

    _SC_PAGESIZE,


    _SC_RTSIG_MAX,

    _SC_SEM_NSEMS_MAX,

    _SC_SEM_VALUE_MAX,

    _SC_SIGQUEUE_MAX,

    _SC_TIMER_MAX,




    _SC_BC_BASE_MAX,

    _SC_BC_DIM_MAX,

    _SC_BC_SCALE_MAX,

    _SC_BC_STRING_MAX,

    _SC_COLL_WEIGHTS_MAX,

    _SC_EQUIV_CLASS_MAX,

    _SC_EXPR_NEST_MAX,

    _SC_LINE_MAX,

    _SC_RE_DUP_MAX,

    _SC_CHARCLASS_NAME_MAX,


    _SC_2_VERSION,

    _SC_2_C_BIND,

    _SC_2_C_DEV,

    _SC_2_FORT_DEV,

    _SC_2_FORT_RUN,

    _SC_2_SW_DEV,

    _SC_2_LOCALEDEF,


    _SC_PII,

    _SC_PII_XTI,

    _SC_PII_SOCKET,

    _SC_PII_INTERNET,

    _SC_PII_OSI,

    _SC_POLL,

    _SC_SELECT,

    _SC_UIO_MAXIOV,

    _SC_IOV_MAX = _SC_UIO_MAXIOV,

    _SC_PII_INTERNET_STREAM,

    _SC_PII_INTERNET_DGRAM,

    _SC_PII_OSI_COTS,

    _SC_PII_OSI_CLTS,

    _SC_PII_OSI_M,

    _SC_T_IOV_MAX,



    _SC_THREADS,

    _SC_THREAD_SAFE_FUNCTIONS,

    _SC_GETGR_R_SIZE_MAX,

    _SC_GETPW_R_SIZE_MAX,

    _SC_LOGIN_NAME_MAX,

    _SC_TTY_NAME_MAX,

    _SC_THREAD_DESTRUCTOR_ITERATIONS,

    _SC_THREAD_KEYS_MAX,

    _SC_THREAD_STACK_MIN,

    _SC_THREAD_THREADS_MAX,

    _SC_THREAD_ATTR_STACKADDR,

    _SC_THREAD_ATTR_STACKSIZE,

    _SC_THREAD_PRIORITY_SCHEDULING,

    _SC_THREAD_PRIO_INHERIT,

    _SC_THREAD_PRIO_PROTECT,

    _SC_THREAD_PROCESS_SHARED,


    _SC_NPROCESSORS_CONF,

    _SC_NPROCESSORS_ONLN,

    _SC_PHYS_PAGES,

    _SC_AVPHYS_PAGES,

    _SC_ATEXIT_MAX,

    _SC_PASS_MAX,


    _SC_XOPEN_VERSION,

    _SC_XOPEN_XCU_VERSION,

    _SC_XOPEN_UNIX,

    _SC_XOPEN_CRYPT,

    _SC_XOPEN_ENH_I18N,

    _SC_XOPEN_SHM,


    _SC_2_CHAR_TERM,

    _SC_2_C_VERSION,

    _SC_2_UPE,


    _SC_XOPEN_XPG2,

    _SC_XOPEN_XPG3,

    _SC_XOPEN_XPG4,


    _SC_CHAR_BIT,

    _SC_CHAR_MAX,

    _SC_CHAR_MIN,

    _SC_INT_MAX,

    _SC_INT_MIN,

    _SC_LONG_BIT,

    _SC_WORD_BIT,

    _SC_MB_LEN_MAX,

    _SC_NZERO,

    _SC_SSIZE_MAX,

    _SC_SCHAR_MAX,

    _SC_SCHAR_MIN,

    _SC_SHRT_MAX,

    _SC_SHRT_MIN,

    _SC_UCHAR_MAX,

    _SC_UINT_MAX,

    _SC_ULONG_MAX,

    _SC_USHRT_MAX,


    _SC_NL_ARGMAX,

    _SC_NL_LANGMAX,

    _SC_NL_MSGMAX,

    _SC_NL_NMAX,

    _SC_NL_SETMAX,

    _SC_NL_TEXTMAX,


    _SC_XBS5_ILP32_OFF32,

    _SC_XBS5_ILP32_OFFBIG,

    _SC_XBS5_LP64_OFF64,

    _SC_XBS5_LPBIG_OFFBIG,


    _SC_XOPEN_LEGACY,

    _SC_XOPEN_REALTIME,

    _SC_XOPEN_REALTIME_THREADS,


    _SC_ADVISORY_INFO,

    _SC_BARRIERS,

    _SC_BASE,

    _SC_C_LANG_SUPPORT,

    _SC_C_LANG_SUPPORT_R,

    _SC_CLOCK_SELECTION,

    _SC_CPUTIME,

    _SC_THREAD_CPUTIME,

    _SC_DEVICE_IO,

    _SC_DEVICE_SPECIFIC,

    _SC_DEVICE_SPECIFIC_R,

    _SC_FD_MGMT,

    _SC_FIFO,

    _SC_PIPE,

    _SC_FILE_ATTRIBUTES,

    _SC_FILE_LOCKING,

    _SC_FILE_SYSTEM,

    _SC_MONOTONIC_CLOCK,

    _SC_MULTI_PROCESS,

    _SC_SINGLE_PROCESS,

    _SC_NETWORKING,

    _SC_READER_WRITER_LOCKS,

    _SC_SPIN_LOCKS,

    _SC_REGEXP,

    _SC_REGEX_VERSION,

    _SC_SHELL,

    _SC_SIGNALS,

    _SC_SPAWN,

    _SC_SPORADIC_SERVER,

    _SC_THREAD_SPORADIC_SERVER,

    _SC_SYSTEM_DATABASE,

    _SC_SYSTEM_DATABASE_R,

    _SC_TIMEOUTS,

    _SC_TYPED_MEMORY_OBJECTS,

    _SC_USER_GROUPS,

    _SC_USER_GROUPS_R,

    _SC_2_PBS,

    _SC_2_PBS_ACCOUNTING,

    _SC_2_PBS_LOCATE,

    _SC_2_PBS_MESSAGE,

    _SC_2_PBS_TRACK,

    _SC_SYMLOOP_MAX,

    _SC_STREAMS,

    _SC_2_PBS_CHECKPOINT,


    _SC_V6_ILP32_OFF32,

    _SC_V6_ILP32_OFFBIG,

    _SC_V6_LP64_OFF64,

    _SC_V6_LPBIG_OFFBIG,


    _SC_HOST_NAME_MAX,

    _SC_TRACE,

    _SC_TRACE_EVENT_FILTER,

    _SC_TRACE_INHERIT,

    _SC_TRACE_LOG,


    _SC_LEVEL1_ICACHE_SIZE,

    _SC_LEVEL1_ICACHE_ASSOC,

    _SC_LEVEL1_ICACHE_LINESIZE,

    _SC_LEVEL1_DCACHE_SIZE,

    _SC_LEVEL1_DCACHE_ASSOC,

    _SC_LEVEL1_DCACHE_LINESIZE,

    _SC_LEVEL2_CACHE_SIZE,

    _SC_LEVEL2_CACHE_ASSOC,

    _SC_LEVEL2_CACHE_LINESIZE,

    _SC_LEVEL3_CACHE_SIZE,

    _SC_LEVEL3_CACHE_ASSOC,

    _SC_LEVEL3_CACHE_LINESIZE,

    _SC_LEVEL4_CACHE_SIZE,

    _SC_LEVEL4_CACHE_ASSOC,

    _SC_LEVEL4_CACHE_LINESIZE,



    _SC_IPV6 = _SC_LEVEL1_ICACHE_SIZE + 50,

    _SC_RAW_SOCKETS,


    _SC_V7_ILP32_OFF32,

    _SC_V7_ILP32_OFFBIG,

    _SC_V7_LP64_OFF64,

    _SC_V7_LPBIG_OFFBIG,


    _SC_SS_REPL_MAX,


    _SC_TRACE_EVENT_NAME_MAX,

    _SC_TRACE_NAME_MAX,

    _SC_TRACE_SYS_MAX,

    _SC_TRACE_USER_EVENT_MAX,


    _SC_XOPEN_STREAMS,


    _SC_THREAD_ROBUST_PRIO_INHERIT,

    _SC_THREAD_ROBUST_PRIO_PROTECT

  };


enum
  {
    _CS_PATH,


    _CS_V6_WIDTH_RESTRICTED_ENVS,



    _CS_GNU_LIBC_VERSION,

    _CS_GNU_LIBPTHREAD_VERSION,


    _CS_V5_WIDTH_RESTRICTED_ENVS,



    _CS_V7_WIDTH_RESTRICTED_ENVS,



    _CS_LFS_CFLAGS = 1000,

    _CS_LFS_LDFLAGS,

    _CS_LFS_LIBS,

    _CS_LFS_LINTFLAGS,

    _CS_LFS64_CFLAGS,

    _CS_LFS64_LDFLAGS,

    _CS_LFS64_LIBS,

    _CS_LFS64_LINTFLAGS,


    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,

    _CS_XBS5_ILP32_OFF32_LDFLAGS,

    _CS_XBS5_ILP32_OFF32_LIBS,

    _CS_XBS5_ILP32_OFF32_LINTFLAGS,

    _CS_XBS5_ILP32_OFFBIG_CFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LIBS,

    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,

    _CS_XBS5_LP64_OFF64_CFLAGS,

    _CS_XBS5_LP64_OFF64_LDFLAGS,

    _CS_XBS5_LP64_OFF64_LIBS,

    _CS_XBS5_LP64_OFF64_LINTFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LIBS,

    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LIBS,

    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V6_LP64_OFF64_CFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LIBS,

    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V7_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LIBS,

    _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V7_LP64_OFF64_CFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LIBS,

    _CS_POSIX_V7_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS,


    _CS_V6_ENV,

    _CS_V7_ENV

  };
# 610 "/usr/include/unistd.h" 2 3 4


extern long int pathconf (const char *__path, int __name)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern long int fpathconf (int __fd, int __name) __attribute__ ((__nothrow__ ));


extern long int sysconf (int __name) __attribute__ ((__nothrow__ ));



extern size_t confstr (int __name, char *__buf, size_t __len) __attribute__ ((__nothrow__ ));




extern __pid_t getpid (void) __attribute__ ((__nothrow__ ));


extern __pid_t getppid (void) __attribute__ ((__nothrow__ ));


extern __pid_t getpgrp (void) __attribute__ ((__nothrow__ ));


extern __pid_t __getpgid (__pid_t __pid) __attribute__ ((__nothrow__ ));

extern __pid_t getpgid (__pid_t __pid) __attribute__ ((__nothrow__ ));






extern int setpgid (__pid_t __pid, __pid_t __pgid) __attribute__ ((__nothrow__ ));
# 660 "/usr/include/unistd.h" 3 4
extern int setpgrp (void) __attribute__ ((__nothrow__ ));






extern __pid_t setsid (void) __attribute__ ((__nothrow__ ));



extern __pid_t getsid (__pid_t __pid) __attribute__ ((__nothrow__ ));



extern __uid_t getuid (void) __attribute__ ((__nothrow__ ));


extern __uid_t geteuid (void) __attribute__ ((__nothrow__ ));


extern __gid_t getgid (void) __attribute__ ((__nothrow__ ));


extern __gid_t getegid (void) __attribute__ ((__nothrow__ ));




extern int getgroups (int __size, __gid_t __list[]) __attribute__ ((__nothrow__ )) ;
# 700 "/usr/include/unistd.h" 3 4
extern int setuid (__uid_t __uid) __attribute__ ((__nothrow__ )) ;




extern int setreuid (__uid_t __ruid, __uid_t __euid) __attribute__ ((__nothrow__ )) ;




extern int seteuid (__uid_t __uid) __attribute__ ((__nothrow__ )) ;






extern int setgid (__gid_t __gid) __attribute__ ((__nothrow__ )) ;




extern int setregid (__gid_t __rgid, __gid_t __egid) __attribute__ ((__nothrow__ )) ;




extern int setegid (__gid_t __gid) __attribute__ ((__nothrow__ )) ;
# 756 "/usr/include/unistd.h" 3 4
extern __pid_t fork (void) __attribute__ ((__nothrow__));







extern __pid_t vfork (void) __attribute__ ((__nothrow__ ));





extern char *ttyname (int __fd) __attribute__ ((__nothrow__ ));



extern int ttyname_r (int __fd, char *__buf, size_t __buflen)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2))) ;



extern int isatty (int __fd) __attribute__ ((__nothrow__ ));





extern int ttyslot (void) __attribute__ ((__nothrow__ ));




extern int link (const char *__from, const char *__to)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2))) ;




extern int linkat (int __fromfd, const char *__from, int __tofd,
     const char *__to, int __flags)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 4))) ;




extern int symlink (const char *__from, const char *__to)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2))) ;




extern ssize_t readlink (const char *__restrict __path,
    char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2))) ;




extern int symlinkat (const char *__from, int __tofd,
        const char *__to) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 3))) ;


extern ssize_t readlinkat (int __fd, const char *__restrict __path,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3))) ;



extern int unlink (const char *__name) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int unlinkat (int __fd, const char *__name, int __flag)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));



extern int rmdir (const char *__path) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern __pid_t tcgetpgrp (int __fd) __attribute__ ((__nothrow__ ));


extern int tcsetpgrp (int __fd, __pid_t __pgrp_id) __attribute__ ((__nothrow__ ));






extern char *getlogin (void);







extern int getlogin_r (char *__name, size_t __name_len) __attribute__ ((__nonnull__ (1)));




extern int setlogin (const char *__name) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 871 "/usr/include/unistd.h" 3 4
# 1 "/usr/include/getopt.h" 1 3 4
# 57 "/usr/include/getopt.h" 3 4
extern char *optarg;
# 71 "/usr/include/getopt.h" 3 4
extern int optind;




extern int opterr;



extern int optopt;
# 150 "/usr/include/getopt.h" 3 4
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts)
       __attribute__ ((__nothrow__ ));
# 872 "/usr/include/unistd.h" 2 3 4







extern int gethostname (char *__name, size_t __len) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));






extern int sethostname (const char *__name, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;



extern int sethostid (long int __id) __attribute__ ((__nothrow__ )) ;





extern int getdomainname (char *__name, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;
extern int setdomainname (const char *__name, size_t __len)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;





extern int vhangup (void) __attribute__ ((__nothrow__ ));


extern int revoke (const char *__file) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;







extern int profil (unsigned short int *__sample_buffer, size_t __size,
     size_t __offset, unsigned int __scale)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int acct (const char *__name) __attribute__ ((__nothrow__ ));



extern char *getusershell (void) __attribute__ ((__nothrow__ ));
extern void endusershell (void) __attribute__ ((__nothrow__ ));
extern void setusershell (void) __attribute__ ((__nothrow__ ));





extern int daemon (int __nochdir, int __noclose) __attribute__ ((__nothrow__ )) ;






extern int chroot (const char *__path) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;



extern char *getpass (const char *__prompt) __attribute__ ((__nonnull__ (1)));







extern int fsync (int __fd);
# 969 "/usr/include/unistd.h" 3 4
extern long int gethostid (void);


extern void sync (void) __attribute__ ((__nothrow__ ));





extern int getpagesize (void) __attribute__ ((__nothrow__ )) __attribute__ ((__const__));




extern int getdtablesize (void) __attribute__ ((__nothrow__ ));
# 993 "/usr/include/unistd.h" 3 4
extern int truncate (const char *__file, __off_t __length)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) ;
# 1016 "/usr/include/unistd.h" 3 4
extern int ftruncate (int __fd, __off_t __length) __attribute__ ((__nothrow__ )) ;
# 1037 "/usr/include/unistd.h" 3 4
extern int brk (void *__addr) __attribute__ ((__nothrow__ )) ;





extern void *sbrk (intptr_t __delta) __attribute__ ((__nothrow__ ));
# 1058 "/usr/include/unistd.h" 3 4
extern long int syscall (long int __sysno, ...) __attribute__ ((__nothrow__ ));
# 1081 "/usr/include/unistd.h" 3 4
extern int lockf (int __fd, int __cmd, __off_t __len) ;
# 1112 "/usr/include/unistd.h" 3 4
extern int fdatasync (int __fildes);
# 29 "./../test/abts.h" 2
# 41 "./../test/abts.h"
struct sub_suite {
    const char *name;
    int num_test;
    int failed;
    int not_run;
    int not_impl;
    struct sub_suite *next;
};
typedef struct sub_suite sub_suite;

struct abts_suite {
    sub_suite *head;
    sub_suite *tail;
};
typedef struct abts_suite abts_suite;

struct abts_case {
    int failed;
    sub_suite *suite;
};
typedef struct abts_case abts_case;

typedef void (*test_func)(abts_case *tc, void *data);



abts_suite *abts_add_suite(abts_suite *suite, const char *suite_name);
void abts_run_test(abts_suite *ts, test_func f, void *value);
void abts_log_message(const char *fmt, ...);

void abts_int_equal(abts_case *tc, const int expected, const int actual, int lineno);
void abts_int_nequal(abts_case *tc, const int expected, const int actual, int lineno);
void abts_str_equal(abts_case *tc, const char *expected, const char *actual, int lineno);
void abts_str_nequal(abts_case *tc, const char *expected, const char *actual,
                       size_t n, int lineno);
void abts_ptr_notnull(abts_case *tc, const void *ptr, int lineno);
void abts_ptr_equal(abts_case *tc, const void *expected, const void *actual, int lineno);
void abts_true(abts_case *tc, int condition, int lineno);
void abts_fail(abts_case *tc, const char *message, int lineno);
void abts_not_impl(abts_case *tc, const char *message, int lineno);
void abts_assert(abts_case *tc, const char *message, int condition, int lineno);
void abts_size_equal(abts_case *tc, size_t expected, size_t actual, int lineno);
# 99 "./../test/abts.h"
abts_suite *run_tests(abts_suite *suite);
abts_suite *run_tests1(abts_suite *suite);
# 7 "main2.c" 2
# 1 "./../test/testutil.h" 1
# 17 "./../test/testutil.h"
# 1 "../include/apr_pools.h" 1
# 43 "../include/apr_pools.h"
# 1 "../include/apr.h" 1
# 168 "../include/apr.h"
# 1 "/usr/include/x86_64-linux-gnu/sys/socket.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/sys/uio.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/sys/uio.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/uio.h" 1 3 4
# 43 "/usr/include/x86_64-linux-gnu/bits/uio.h" 3 4
struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };
# 29 "/usr/include/x86_64-linux-gnu/sys/uio.h" 2 3 4
# 39 "/usr/include/x86_64-linux-gnu/sys/uio.h" 3 4
extern ssize_t readv (int __fd, const struct iovec *__iovec, int __count)
       ;
# 50 "/usr/include/x86_64-linux-gnu/sys/uio.h" 3 4
extern ssize_t writev (int __fd, const struct iovec *__iovec, int __count)
       ;
# 65 "/usr/include/x86_64-linux-gnu/sys/uio.h" 3 4
extern ssize_t preadv (int __fd, const struct iovec *__iovec, int __count,
         __off_t __offset) ;
# 77 "/usr/include/x86_64-linux-gnu/sys/uio.h" 3 4
extern ssize_t pwritev (int __fd, const struct iovec *__iovec, int __count,
   __off_t __offset) ;
# 27 "/usr/include/x86_64-linux-gnu/sys/socket.h" 2 3 4

# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 29 "/usr/include/x86_64-linux-gnu/sys/socket.h" 2 3 4
# 38 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/socket.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/socket.h" 2 3 4
# 38 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/socket_type.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/socket_type.h" 3 4
enum __socket_type
{
  SOCK_STREAM = 1,


  SOCK_DGRAM = 2,


  SOCK_RAW = 3,

  SOCK_RDM = 4,

  SOCK_SEQPACKET = 5,


  SOCK_DCCP = 6,

  SOCK_PACKET = 10,







  SOCK_CLOEXEC = 02000000,


  SOCK_NONBLOCK = 00004000


};
# 39 "/usr/include/x86_64-linux-gnu/bits/socket.h" 2 3 4
# 146 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/sockaddr.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/sockaddr.h" 3 4
typedef unsigned short int sa_family_t;
# 147 "/usr/include/x86_64-linux-gnu/bits/socket.h" 2 3 4


struct sockaddr
  {
    sa_family_t sa_family;
    char sa_data[14];
  };
# 162 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
struct sockaddr_storage
  {
    sa_family_t ss_family;
    unsigned long int __ss_align;
    char __ss_padding[(128 - (2 * sizeof (unsigned long int)))];
  };



enum
  {
    MSG_OOB = 0x01,

    MSG_PEEK = 0x02,

    MSG_DONTROUTE = 0x04,






    MSG_CTRUNC = 0x08,

    MSG_PROXY = 0x10,

    MSG_TRUNC = 0x20,

    MSG_DONTWAIT = 0x40,

    MSG_EOR = 0x80,

    MSG_WAITALL = 0x100,

    MSG_FIN = 0x200,

    MSG_SYN = 0x400,

    MSG_CONFIRM = 0x800,

    MSG_RST = 0x1000,

    MSG_ERRQUEUE = 0x2000,

    MSG_NOSIGNAL = 0x4000,

    MSG_MORE = 0x8000,

    MSG_WAITFORONE = 0x10000,

    MSG_FASTOPEN = 0x20000000,


    MSG_CMSG_CLOEXEC = 0x40000000



  };




struct msghdr
  {
    void *msg_name;
    socklen_t msg_namelen;

    struct iovec *msg_iov;
    size_t msg_iovlen;

    void *msg_control;
    size_t msg_controllen;




    int msg_flags;
  };


struct cmsghdr
  {
    size_t cmsg_len;




    int cmsg_level;
    int cmsg_type;

    __extension__ unsigned char __cmsg_data [];

  };
# 272 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
          struct cmsghdr *__cmsg) __attribute__ ((__nothrow__ ));
# 299 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
enum
  {
    SCM_RIGHTS = 0x01





  };
# 345 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/asm/socket.h" 1 3 4
# 1 "/usr/include/asm-generic/socket.h" 1 3 4



# 1 "/usr/include/x86_64-linux-gnu/asm/sockios.h" 1 3 4
# 1 "/usr/include/asm-generic/sockios.h" 1 3 4
# 2 "/usr/include/x86_64-linux-gnu/asm/sockios.h" 2 3 4
# 5 "/usr/include/asm-generic/socket.h" 2 3 4
# 2 "/usr/include/x86_64-linux-gnu/asm/socket.h" 2 3 4
# 346 "/usr/include/x86_64-linux-gnu/bits/socket.h" 2 3 4
# 379 "/usr/include/x86_64-linux-gnu/bits/socket.h" 3 4
struct linger
  {
    int l_onoff;
    int l_linger;
  };
# 39 "/usr/include/x86_64-linux-gnu/sys/socket.h" 2 3 4




struct osockaddr
  {
    unsigned short int sa_family;
    unsigned char sa_data[14];
  };




enum
{
  SHUT_RD = 0,

  SHUT_WR,

  SHUT_RDWR

};
# 113 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern int socket (int __domain, int __type, int __protocol) __attribute__ ((__nothrow__ ));





extern int socketpair (int __domain, int __type, int __protocol,
         int __fds[2]) __attribute__ ((__nothrow__ ));


extern int bind (int __fd, const struct sockaddr * __addr, socklen_t __len)
     __attribute__ ((__nothrow__ ));


extern int getsockname (int __fd, struct sockaddr *__restrict __addr,
   socklen_t *__restrict __len) __attribute__ ((__nothrow__ ));
# 137 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern int connect (int __fd, const struct sockaddr * __addr, socklen_t __len);



extern int getpeername (int __fd, struct sockaddr *__restrict __addr,
   socklen_t *__restrict __len) __attribute__ ((__nothrow__ ));






extern ssize_t send (int __fd, const void *__buf, size_t __n, int __flags);






extern ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);






extern ssize_t sendto (int __fd, const void *__buf, size_t __n,
         int __flags, const struct sockaddr * __addr,
         socklen_t __addr_len);
# 174 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
    int __flags, struct sockaddr *__restrict __addr,
    socklen_t *__restrict __addr_len);







extern ssize_t sendmsg (int __fd, const struct msghdr *__message,
   int __flags);
# 202 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags);
# 219 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern int getsockopt (int __fd, int __level, int __optname,
         void *__restrict __optval,
         socklen_t *__restrict __optlen) __attribute__ ((__nothrow__ ));




extern int setsockopt (int __fd, int __level, int __optname,
         const void *__optval, socklen_t __optlen) __attribute__ ((__nothrow__ ));





extern int listen (int __fd, int __n) __attribute__ ((__nothrow__ ));
# 243 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern int accept (int __fd, struct sockaddr *__restrict __addr,
     socklen_t *__restrict __addr_len);
# 261 "/usr/include/x86_64-linux-gnu/sys/socket.h" 3 4
extern int shutdown (int __fd, int __how) __attribute__ ((__nothrow__ ));




extern int sockatmark (int __fd) __attribute__ ((__nothrow__ ));







extern int isfdtype (int __fd, int __fdtype) __attribute__ ((__nothrow__ ));
# 169 "../include/apr.h" 2
# 178 "../include/apr.h"
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdint.h" 1 3
# 61 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdint.h" 3
# 1 "/usr/bin/../lib/gcc/x86_64-linux-gnu/4.9/include/stdint.h" 1 3 4








# 1 "/usr/include/stdint.h" 1 3 4
# 26 "/usr/include/stdint.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wchar.h" 1 3 4
# 27 "/usr/include/stdint.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 28 "/usr/include/stdint.h" 2 3 4
# 48 "/usr/include/stdint.h" 3 4
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef unsigned int uint32_t;



typedef unsigned long int uint64_t;
# 65 "/usr/include/stdint.h" 3 4
typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;

typedef long int int_least64_t;






typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;

typedef unsigned long int uint_least64_t;
# 90 "/usr/include/stdint.h" 3 4
typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
# 103 "/usr/include/stdint.h" 3 4
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
# 122 "/usr/include/stdint.h" 3 4
typedef unsigned long int uintptr_t;
# 134 "/usr/include/stdint.h" 3 4
typedef long int intmax_t;
typedef unsigned long int uintmax_t;
# 10 "/usr/bin/../lib/gcc/x86_64-linux-gnu/4.9/include/stdint.h" 2 3 4
# 62 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdint.h" 2 3
# 179 "../include/apr.h" 2



# 1 "/usr/include/x86_64-linux-gnu/sys/wait.h" 1 3 4
# 29 "/usr/include/x86_64-linux-gnu/sys/wait.h" 3 4
# 1 "/usr/include/signal.h" 1 3 4
# 32 "/usr/include/signal.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/sigset.h" 1 3 4
# 102 "/usr/include/x86_64-linux-gnu/bits/sigset.h" 3 4
extern int __sigismember (const __sigset_t *, int);
extern int __sigaddset (__sigset_t *, int);
extern int __sigdelset (__sigset_t *, int);
# 33 "/usr/include/signal.h" 2 3 4







typedef __sig_atomic_t sig_atomic_t;
# 57 "/usr/include/signal.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/signum.h" 1 3 4
# 58 "/usr/include/signal.h" 2 3 4
# 75 "/usr/include/signal.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 76 "/usr/include/signal.h" 2 3 4




# 1 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 2 3 4







typedef union sigval
  {
    int sival_int;
    void *sival_ptr;
  } sigval_t;
# 58 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 3 4
typedef __clock_t __sigchld_clock_t;



typedef struct
  {
    int si_signo;
    int si_errno;

    int si_code;

    union
      {
 int _pad[((128 / sizeof (int)) - 4)];


 struct
   {
     __pid_t si_pid;
     __uid_t si_uid;
   } _kill;


 struct
   {
     int si_tid;
     int si_overrun;
     sigval_t si_sigval;
   } _timer;


 struct
   {
     __pid_t si_pid;
     __uid_t si_uid;
     sigval_t si_sigval;
   } _rt;


 struct
   {
     __pid_t si_pid;
     __uid_t si_uid;
     int si_status;
     __sigchld_clock_t si_utime;
     __sigchld_clock_t si_stime;
   } _sigchld;


 struct
   {
     void *si_addr;
     short int si_addr_lsb;
   } _sigfault;


 struct
   {
     long int si_band;
     int si_fd;
   } _sigpoll;


 struct
   {
     void *_call_addr;
     int _syscall;
     unsigned int _arch;
   } _sigsys;
      } _sifields;
  } siginfo_t ;
# 153 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 3 4
enum
{
  SI_ASYNCNL = -60,

  SI_TKILL = -6,

  SI_SIGIO,

  SI_ASYNCIO,

  SI_MESGQ,

  SI_TIMER,

  SI_QUEUE,

  SI_USER,

  SI_KERNEL = 0x80

};



enum
{
  ILL_ILLOPC = 1,

  ILL_ILLOPN,

  ILL_ILLADR,

  ILL_ILLTRP,

  ILL_PRVOPC,

  ILL_PRVREG,

  ILL_COPROC,

  ILL_BADSTK

};


enum
{
  FPE_INTDIV = 1,

  FPE_INTOVF,

  FPE_FLTDIV,

  FPE_FLTOVF,

  FPE_FLTUND,

  FPE_FLTRES,

  FPE_FLTINV,

  FPE_FLTSUB

};


enum
{
  SEGV_MAPERR = 1,

  SEGV_ACCERR

};


enum
{
  BUS_ADRALN = 1,

  BUS_ADRERR,

  BUS_OBJERR,

  BUS_MCEERR_AR,

  BUS_MCEERR_AO

};


enum
{
  TRAP_BRKPT = 1,

  TRAP_TRACE

};


enum
{
  CLD_EXITED = 1,

  CLD_KILLED,

  CLD_DUMPED,

  CLD_TRAPPED,

  CLD_STOPPED,

  CLD_CONTINUED

};


enum
{
  POLL_IN = 1,

  POLL_OUT,

  POLL_MSG,

  POLL_ERR,

  POLL_PRI,

  POLL_HUP

};
# 307 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 3 4
typedef struct sigevent
  {
    sigval_t sigev_value;
    int sigev_signo;
    int sigev_notify;

    union
      {
 int _pad[((64 / sizeof (int)) - 4)];



 __pid_t _tid;

 struct
   {
     void (*_function) (sigval_t);
     pthread_attr_t *_attribute;
   } _sigev_thread;
      } _sigev_un;
  } sigevent_t;






enum
{
  SIGEV_SIGNAL = 0,

  SIGEV_NONE,

  SIGEV_THREAD,


  SIGEV_THREAD_ID = 4

};
# 81 "/usr/include/signal.h" 2 3 4




typedef void (*__sighandler_t) (int);




extern __sighandler_t __sysv_signal (int __sig, __sighandler_t __handler)
     __attribute__ ((__nothrow__ ));
# 102 "/usr/include/signal.h" 3 4
extern __sighandler_t signal (int __sig, __sighandler_t __handler)
     __attribute__ ((__nothrow__ ));
# 127 "/usr/include/signal.h" 3 4
extern int kill (__pid_t __pid, int __sig) __attribute__ ((__nothrow__ ));






extern int killpg (__pid_t __pgrp, int __sig) __attribute__ ((__nothrow__ ));




extern int raise (int __sig) __attribute__ ((__nothrow__ ));




extern __sighandler_t ssignal (int __sig, __sighandler_t __handler)
     __attribute__ ((__nothrow__ ));
extern int gsignal (int __sig) __attribute__ ((__nothrow__ ));




extern void psignal (int __sig, const char *__s);




extern void psiginfo (const siginfo_t *__pinfo, const char *__s);
# 167 "/usr/include/signal.h" 3 4
extern int __sigpause (int __sig_or_mask, int __is_sig);
# 189 "/usr/include/signal.h" 3 4
extern int sigblock (int __mask) __attribute__ ((__nothrow__ )) __attribute__ ((__deprecated__));


extern int sigsetmask (int __mask) __attribute__ ((__nothrow__ )) __attribute__ ((__deprecated__));


extern int siggetmask (void) __attribute__ ((__nothrow__ )) __attribute__ ((__deprecated__));
# 209 "/usr/include/signal.h" 3 4
typedef __sighandler_t sig_t;





extern int sigemptyset (sigset_t *__set) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int sigfillset (sigset_t *__set) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int sigaddset (sigset_t *__set, int __signo) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int sigdelset (sigset_t *__set, int __signo) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int sigismember (const sigset_t *__set, int __signo)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 245 "/usr/include/signal.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/sigaction.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/sigaction.h" 3 4
struct sigaction
  {


    union
      {

 __sighandler_t sa_handler;

 void (*sa_sigaction) (int, siginfo_t *, void *);
      }
    __sigaction_handler;







    __sigset_t sa_mask;


    int sa_flags;


    void (*sa_restorer) (void);
  };
# 246 "/usr/include/signal.h" 2 3 4


extern int sigprocmask (int __how, const sigset_t *__restrict __set,
   sigset_t *__restrict __oset) __attribute__ ((__nothrow__ ));






extern int sigsuspend (const sigset_t *__set) __attribute__ ((__nonnull__ (1)));


extern int sigaction (int __sig, const struct sigaction *__restrict __act,
        struct sigaction *__restrict __oact) __attribute__ ((__nothrow__ ));


extern int sigpending (sigset_t *__set) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));






extern int sigwait (const sigset_t *__restrict __set, int *__restrict __sig)
     __attribute__ ((__nonnull__ (1, 2)));






extern int sigwaitinfo (const sigset_t *__restrict __set,
   siginfo_t *__restrict __info) __attribute__ ((__nonnull__ (1)));






extern int sigtimedwait (const sigset_t *__restrict __set,
    siginfo_t *__restrict __info,
    const struct timespec *__restrict __timeout)
     __attribute__ ((__nonnull__ (1)));



extern int sigqueue (__pid_t __pid, int __sig, const union sigval __val)
     __attribute__ ((__nothrow__ ));
# 303 "/usr/include/signal.h" 3 4
extern const char *const _sys_siglist[65];
extern const char *const sys_siglist[65];


struct sigvec
  {
    __sighandler_t sv_handler;
    int sv_mask;

    int sv_flags;

  };
# 327 "/usr/include/signal.h" 3 4
extern int sigvec (int __sig, const struct sigvec *__vec,
     struct sigvec *__ovec) __attribute__ ((__nothrow__ ));




# 1 "/usr/include/x86_64-linux-gnu/bits/sigcontext.h" 1 3 4
# 29 "/usr/include/x86_64-linux-gnu/bits/sigcontext.h" 3 4
struct _fpx_sw_bytes
{
  __uint32_t magic1;
  __uint32_t extended_size;
  __uint64_t xstate_bv;
  __uint32_t xstate_size;
  __uint32_t padding[7];
};

struct _fpreg
{
  unsigned short significand[4];
  unsigned short exponent;
};

struct _fpxreg
{
  unsigned short significand[4];
  unsigned short exponent;
  unsigned short padding[3];
};

struct _xmmreg
{
  __uint32_t element[4];
};
# 121 "/usr/include/x86_64-linux-gnu/bits/sigcontext.h" 3 4
struct _fpstate
{

  __uint16_t cwd;
  __uint16_t swd;
  __uint16_t ftw;
  __uint16_t fop;
  __uint64_t rip;
  __uint64_t rdp;
  __uint32_t mxcsr;
  __uint32_t mxcr_mask;
  struct _fpxreg _st[8];
  struct _xmmreg _xmm[16];
  __uint32_t padding[24];
};

struct sigcontext
{
  __uint64_t r8;
  __uint64_t r9;
  __uint64_t r10;
  __uint64_t r11;
  __uint64_t r12;
  __uint64_t r13;
  __uint64_t r14;
  __uint64_t r15;
  __uint64_t rdi;
  __uint64_t rsi;
  __uint64_t rbp;
  __uint64_t rbx;
  __uint64_t rdx;
  __uint64_t rax;
  __uint64_t rcx;
  __uint64_t rsp;
  __uint64_t rip;
  __uint64_t eflags;
  unsigned short cs;
  unsigned short gs;
  unsigned short fs;
  unsigned short __pad0;
  __uint64_t err;
  __uint64_t trapno;
  __uint64_t oldmask;
  __uint64_t cr2;
  __extension__ union
    {
      struct _fpstate * fpstate;
      __uint64_t __fpstate_word;
    };
  __uint64_t __reserved1 [8];
};



struct _xsave_hdr
{
  __uint64_t xstate_bv;
  __uint64_t reserved1[2];
  __uint64_t reserved2[5];
};

struct _ymmh_state
{
  __uint32_t ymmh_space[64];
};

struct _xstate
{
  struct _fpstate fpstate;
  struct _xsave_hdr xstate_hdr;
  struct _ymmh_state ymmh;
};
# 333 "/usr/include/signal.h" 2 3 4


extern int sigreturn (struct sigcontext *__scp) __attribute__ ((__nothrow__ ));







# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 343 "/usr/include/signal.h" 2 3 4




extern int siginterrupt (int __sig, int __interrupt) __attribute__ ((__nothrow__ ));


# 1 "/usr/include/x86_64-linux-gnu/bits/sigstack.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/sigstack.h" 3 4
struct sigstack
  {
    void *ss_sp;
    int ss_onstack;
  };



enum
{
  SS_ONSTACK = 1,

  SS_DISABLE

};
# 49 "/usr/include/x86_64-linux-gnu/bits/sigstack.h" 3 4
typedef struct sigaltstack
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;
# 350 "/usr/include/signal.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/sys/ucontext.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/sys/ucontext.h" 3 4
# 1 "/usr/include/signal.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/sys/ucontext.h" 2 3 4








__extension__ typedef long long int greg_t;





typedef greg_t gregset_t[23];
# 92 "/usr/include/x86_64-linux-gnu/sys/ucontext.h" 3 4
struct _libc_fpxreg
{
  unsigned short int significand[4];
  unsigned short int exponent;
  unsigned short int padding[3];
};

struct _libc_xmmreg
{
  __uint32_t element[4];
};

struct _libc_fpstate
{

  __uint16_t cwd;
  __uint16_t swd;
  __uint16_t ftw;
  __uint16_t fop;
  __uint64_t rip;
  __uint64_t rdp;
  __uint32_t mxcsr;
  __uint32_t mxcr_mask;
  struct _libc_fpxreg _st[8];
  struct _libc_xmmreg _xmm[16];
  __uint32_t padding[24];
};


typedef struct _libc_fpstate *fpregset_t;


typedef struct
  {
    gregset_t gregs;

    fpregset_t fpregs;
    __extension__ unsigned long long __reserved1 [8];
} mcontext_t;


typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;
# 353 "/usr/include/signal.h" 2 3 4





extern int sigstack (struct sigstack *__ss, struct sigstack *__oss)
     __attribute__ ((__nothrow__ )) __attribute__ ((__deprecated__));



extern int sigaltstack (const struct sigaltstack *__restrict __ss,
   struct sigaltstack *__restrict __oss) __attribute__ ((__nothrow__ ));
# 388 "/usr/include/signal.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/sigthread.h" 1 3 4
# 30 "/usr/include/x86_64-linux-gnu/bits/sigthread.h" 3 4
extern int pthread_sigmask (int __how,
       const __sigset_t *__restrict __newmask,
       __sigset_t *__restrict __oldmask)__attribute__ ((__nothrow__ ));


extern int pthread_kill (pthread_t __threadid, int __signo) __attribute__ ((__nothrow__ ));
# 389 "/usr/include/signal.h" 2 3 4






extern int __libc_current_sigrtmin (void) __attribute__ ((__nothrow__ ));

extern int __libc_current_sigrtmax (void) __attribute__ ((__nothrow__ ));
# 30 "/usr/include/x86_64-linux-gnu/sys/wait.h" 2 3 4
# 102 "/usr/include/x86_64-linux-gnu/sys/wait.h" 3 4
extern __pid_t wait (__WAIT_STATUS __stat_loc);
# 125 "/usr/include/x86_64-linux-gnu/sys/wait.h" 3 4
extern __pid_t waitpid (__pid_t __pid, int *__stat_loc, int __options);
# 135 "/usr/include/x86_64-linux-gnu/sys/wait.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/siginfo.h" 2 3 4
# 136 "/usr/include/x86_64-linux-gnu/sys/wait.h" 2 3 4
# 148 "/usr/include/x86_64-linux-gnu/sys/wait.h" 3 4
extern int waitid (idtype_t __idtype, __id_t __id, siginfo_t *__infop,
     int __options);





struct rusage;






extern __pid_t wait3 (__WAIT_STATUS __stat_loc, int __options,
        struct rusage * __usage) __attribute__ ((__nothrow__));




extern __pid_t wait4 (__pid_t __pid, __WAIT_STATUS __stat_loc, int __options,
        struct rusage *__usage) __attribute__ ((__nothrow__));
# 183 "../include/apr.h" 2
# 193 "../include/apr.h"
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/limits.h" 1 3
# 37 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/limits.h" 3
# 1 "/usr/include/limits.h" 1 3 4
# 143 "/usr/include/limits.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 1 3 4
# 160 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 1 3 4
# 38 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 3 4
# 1 "/usr/include/linux/limits.h" 1 3 4
# 39 "/usr/include/x86_64-linux-gnu/bits/local_lim.h" 2 3 4
# 161 "/usr/include/x86_64-linux-gnu/bits/posix1_lim.h" 2 3 4
# 144 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/posix2_lim.h" 1 3 4
# 148 "/usr/include/limits.h" 2 3 4
# 38 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/limits.h" 2 3
# 194 "../include/apr.h" 2
# 309 "../include/apr.h"
typedef unsigned char apr_byte_t;

typedef short apr_int16_t;
typedef unsigned short apr_uint16_t;

typedef int apr_int32_t;
typedef unsigned int apr_uint32_t;
# 352 "../include/apr.h"
 typedef long apr_int64_t;
 typedef unsigned long apr_uint64_t;


typedef size_t apr_size_t;
typedef ssize_t apr_ssize_t;
typedef off_t apr_off_t;
typedef socklen_t apr_socklen_t;
typedef ino_t apr_ino_t;


typedef apr_uint64_t apr_uintptr_t;
# 44 "../include/apr_pools.h" 2
# 1 "../include/apr_errno.h" 1
# 28 "../include/apr_errno.h"
# 1 "/usr/include/errno.h" 1 3 4
# 35 "/usr/include/errno.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/errno.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/errno.h" 3 4
# 1 "/usr/include/linux/errno.h" 1 3 4
# 1 "/usr/include/x86_64-linux-gnu/asm/errno.h" 1 3 4
# 1 "/usr/include/asm-generic/errno.h" 1 3 4



# 1 "/usr/include/asm-generic/errno-base.h" 1 3 4
# 5 "/usr/include/asm-generic/errno.h" 2 3 4
# 2 "/usr/include/x86_64-linux-gnu/asm/errno.h" 2 3 4
# 2 "/usr/include/linux/errno.h" 2 3 4
# 25 "/usr/include/x86_64-linux-gnu/bits/errno.h" 2 3 4
# 50 "/usr/include/x86_64-linux-gnu/bits/errno.h" 3 4
extern int *__errno_location (void) __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
# 36 "/usr/include/errno.h" 2 3 4
# 29 "../include/apr_errno.h" 2
# 44 "../include/apr_errno.h"
typedef int apr_status_t;







char * apr_strerror(apr_status_t statcode, char *buf,
                                 apr_size_t bufsize);
# 45 "../include/apr_pools.h" 2
# 1 "../include/apr_general.h" 1
# 29 "../include/apr_general.h"
# 1 "../include/apr_pools.h" 1
# 30 "../include/apr_general.h" 2
# 68 "../include/apr_general.h"
typedef int apr_signum_t;
# 177 "../include/apr_general.h"
apr_status_t apr_initialize(void);
# 191 "../include/apr_general.h"
apr_status_t apr_app_initialize(int *argc,
                                             char const * const * *argv,
                                             char const * const * *env);
# 205 "../include/apr_general.h"
void apr_terminate(void);
# 216 "../include/apr_general.h"
void apr_terminate2(void);
# 233 "../include/apr_general.h"
apr_status_t apr_generate_random_bytes(unsigned char * buf,
                                                    apr_size_t length);
# 46 "../include/apr_pools.h" 2

# 1 "../include/apr_want.h" 1
# 48 "../include/apr_pools.h" 2
# 60 "../include/apr_pools.h"
typedef struct apr_pool_t apr_pool_t;
# 148 "../include/apr_pools.h"
typedef int (*apr_abortfunc_t)(int retcode);
# 164 "../include/apr_pools.h"
apr_status_t apr_pool_initialize(void);







void apr_pool_terminate(void);







# 1 "../include/apr_allocator.h" 1
# 28 "../include/apr_allocator.h"
# 1 "../include/apr_want.h" 1
# 29 "../include/apr_allocator.h" 2
# 41 "../include/apr_allocator.h"
typedef struct apr_allocator_t apr_allocator_t;

typedef struct apr_memnode_t apr_memnode_t;
# 54 "../include/apr_allocator.h"
struct apr_memnode_t {
    apr_memnode_t *next;
    apr_memnode_t **ref;
    apr_uint32_t index;
    apr_uint32_t free_index;
    char *first_avail;
    char *endp;
};
# 74 "../include/apr_allocator.h"
apr_status_t apr_allocator_create(apr_allocator_t **allocator)
                          __attribute__((nonnull(1)));







void apr_allocator_destroy(apr_allocator_t *allocator)
                  __attribute__((nonnull(1)));







apr_memnode_t * apr_allocator_alloc(apr_allocator_t *allocator,
                                                 apr_size_t size)
                             __attribute__((nonnull(1)));
# 103 "../include/apr_allocator.h"
void apr_allocator_free(apr_allocator_t *allocator,
                                     apr_memnode_t *memnode)
                  __attribute__((nonnull(1,2)));


# 1 "../include/apr_pools.h" 1
# 108 "../include/apr_allocator.h" 2
# 120 "../include/apr_allocator.h"
void apr_allocator_owner_set(apr_allocator_t *allocator,
                                          apr_pool_t *pool)
                  __attribute__((nonnull(1)));





apr_pool_t * apr_allocator_owner_get(apr_allocator_t *allocator)
                          __attribute__((nonnull(1)));







void apr_allocator_max_free_set(apr_allocator_t *allocator,
                                             apr_size_t size)
                  __attribute__((nonnull(1)));


# 1 "../include/apr_thread_mutex.h" 1
# 41 "../include/apr_thread_mutex.h"
typedef struct apr_thread_mutex_t apr_thread_mutex_t;







# 1 "../include/apr_pools.h" 1
# 49 "../include/apr_thread_mutex.h" 2
# 65 "../include/apr_thread_mutex.h"
apr_status_t apr_thread_mutex_create(apr_thread_mutex_t **mutex,
                                                  unsigned int flags,
                                                  apr_pool_t *pool);





apr_status_t apr_thread_mutex_lock(apr_thread_mutex_t *mutex);
# 82 "../include/apr_thread_mutex.h"
apr_status_t apr_thread_mutex_trylock(apr_thread_mutex_t *mutex);





apr_status_t apr_thread_mutex_unlock(apr_thread_mutex_t *mutex);





apr_status_t apr_thread_mutex_destroy(apr_thread_mutex_t *mutex);





apr_pool_t * apr_thread_mutex_pool_get (const apr_thread_mutex_t *thethread_mutex);
# 142 "../include/apr_allocator.h" 2







void apr_allocator_mutex_set(apr_allocator_t *allocator,
                                          apr_thread_mutex_t *mutex)
                  __attribute__((nonnull(1)));





apr_thread_mutex_t * apr_allocator_mutex_get(
                                          apr_allocator_t *allocator)
                                  __attribute__((nonnull(1)));
# 180 "../include/apr_pools.h" 2
# 196 "../include/apr_pools.h"
apr_status_t apr_pool_create_ex(apr_pool_t **newpool,
                                             apr_pool_t *parent,
                                             apr_abortfunc_t abort_fn,
                                             apr_allocator_t *allocator)
                          __attribute__((nonnull(1)));





apr_status_t apr_pool_create_core_ex(apr_pool_t **newpool,
                                                  apr_abortfunc_t abort_fn,
                                                  apr_allocator_t *allocator);
# 226 "../include/apr_pools.h"
apr_status_t apr_pool_create_unmanaged_ex(apr_pool_t **newpool,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator)
                          __attribute__((nonnull(1)));
# 247 "../include/apr_pools.h"
apr_status_t apr_pool_create_ex_debug(apr_pool_t **newpool,
                                                   apr_pool_t *parent,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator,
                                                   const char *file_line)
                          __attribute__((nonnull(1)));
# 264 "../include/apr_pools.h"
apr_status_t apr_pool_create_core_ex_debug(apr_pool_t **newpool,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator,
                                                   const char *file_line);
# 284 "../include/apr_pools.h"
apr_status_t apr_pool_create_unmanaged_ex_debug(apr_pool_t **newpool,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator,
                                                   const char *file_line)
                          __attribute__((nonnull(1)));
# 354 "../include/apr_pools.h"
apr_allocator_t * apr_pool_allocator_get(apr_pool_t *pool)
                               __attribute__((nonnull(1)));
# 365 "../include/apr_pools.h"
void apr_pool_clear(apr_pool_t *p) __attribute__((nonnull(1)));
# 380 "../include/apr_pools.h"
void apr_pool_clear_debug(apr_pool_t *p,
                                       const char *file_line)
                  __attribute__((nonnull(1)));
# 395 "../include/apr_pools.h"
void apr_pool_destroy(apr_pool_t *p) __attribute__((nonnull(1)));
# 410 "../include/apr_pools.h"
void apr_pool_destroy_debug(apr_pool_t *p,
                                         const char *file_line)
                  __attribute__((nonnull(1)));
# 430 "../include/apr_pools.h"
void * apr_palloc(apr_pool_t *p, apr_size_t size)



                    __attribute__((nonnull(1)));
# 444 "../include/apr_pools.h"
void * apr_palloc_debug(apr_pool_t *p, apr_size_t size,
                                     const char *file_line)



                    __attribute__((nonnull(1)));
# 476 "../include/apr_pools.h"
void * apr_pcalloc_debug(apr_pool_t *p, apr_size_t size,
                                      const char *file_line)
                    __attribute__((nonnull(1)));
# 498 "../include/apr_pools.h"
void apr_pool_abort_set(apr_abortfunc_t abortfunc,
                                     apr_pool_t *pool)
                  __attribute__((nonnull(2)));






apr_abortfunc_t apr_pool_abort_get(apr_pool_t *pool)
                             __attribute__((nonnull(1)));






apr_pool_t * apr_pool_parent_get(apr_pool_t *pool)
                          __attribute__((nonnull(1)));
# 529 "../include/apr_pools.h"
int apr_pool_is_ancestor(apr_pool_t *a, apr_pool_t *b);






void apr_pool_tag(apr_pool_t *pool, const char *tag)
                  __attribute__((nonnull(1)));
# 563 "../include/apr_pools.h"
apr_status_t apr_pool_userdata_set(const void *data,
                                                const char *key,
                                                apr_status_t (*cleanup)(void *),
                                                apr_pool_t *pool)
                          __attribute__((nonnull(2,4)));
# 588 "../include/apr_pools.h"
apr_status_t apr_pool_userdata_setn(
                                const void *data, const char *key,
                                apr_status_t (*cleanup)(void *),
                                apr_pool_t *pool)
                          __attribute__((nonnull(2,4)));







apr_status_t apr_pool_userdata_get(void **data, const char *key,
                                                apr_pool_t *pool)
                          __attribute__((nonnull(1,2,3)));
# 628 "../include/apr_pools.h"
void apr_pool_cleanup_register(
                            apr_pool_t *p, const void *data,
                            apr_status_t (*plain_cleanup)(void *),
                            apr_status_t (*child_cleanup)(void *))
                  __attribute__((nonnull(3,4)));
# 646 "../include/apr_pools.h"
void apr_pool_pre_cleanup_register(
                            apr_pool_t *p, const void *data,
                            apr_status_t (*plain_cleanup)(void *))
                  __attribute__((nonnull(3)));
# 663 "../include/apr_pools.h"
void apr_pool_cleanup_kill(apr_pool_t *p, const void *data,
                                        apr_status_t (*cleanup)(void *))
                  __attribute__((nonnull(3)));
# 679 "../include/apr_pools.h"
void apr_pool_child_cleanup_set(
                        apr_pool_t *p, const void *data,
                        apr_status_t (*plain_cleanup)(void *),
                        apr_status_t (*child_cleanup)(void *))
                  __attribute__((nonnull(3,4)));
# 696 "../include/apr_pools.h"
apr_status_t apr_pool_cleanup_run(apr_pool_t *p, void *data,
                                               apr_status_t (*cleanup)(void *))
                          __attribute__((nonnull(3)));
# 707 "../include/apr_pools.h"
apr_status_t apr_pool_cleanup_null(void *data);







void apr_pool_cleanup_for_exec(void);
# 18 "./../test/testutil.h" 2

# 1 "./../test/abts.h" 1
# 20 "./../test/testutil.h" 2
# 52 "./../test/testutil.h"
extern apr_pool_t *p;



void apr_assert_success(abts_case* tc, const char *context,
                        apr_status_t rv, int lineno);



void initialize(void);

abts_suite *testatomic(abts_suite *suite);
abts_suite *testdir(abts_suite *suite);
abts_suite *testdso(abts_suite *suite);
abts_suite *testdup(abts_suite *suite);
abts_suite *testescape(abts_suite *suite);
abts_suite *testenv(abts_suite *suite);
abts_suite *testfile(abts_suite *suite);
abts_suite *testfilecopy(abts_suite *suite);
abts_suite *testfileinfo(abts_suite *suite);
abts_suite *testflock(abts_suite *suite);
abts_suite *testfmt(abts_suite *suite);
abts_suite *testfnmatch(abts_suite *suite);
abts_suite *testgetopt(abts_suite *suite);
abts_suite *testglobalmutex(abts_suite *suite);
abts_suite *testhash(abts_suite *suite);
abts_suite *testipsub(abts_suite *suite);
abts_suite *testlock(abts_suite *suite);
abts_suite *testcond(abts_suite *suite);
abts_suite *testlfs(abts_suite *suite);
abts_suite *testmmap(abts_suite *suite);
abts_suite *testnames(abts_suite *suite);
abts_suite *testoc(abts_suite *suite);
abts_suite *testpath(abts_suite *suite);
abts_suite *testpipe(abts_suite *suite);
abts_suite *testpoll(abts_suite *suite);
abts_suite *testpool(abts_suite *suite);
abts_suite *testproc(abts_suite *suite);
abts_suite *testprocmutex(abts_suite *suite);
abts_suite *testrand(abts_suite *suite);
abts_suite *testsleep(abts_suite *suite);
abts_suite *testshm(abts_suite *suite);
abts_suite *testsock(abts_suite *suite);
abts_suite *testsockets(abts_suite *suite);
abts_suite *testsockopt(abts_suite *suite);
abts_suite *teststr(abts_suite *suite);
abts_suite *teststrnatcmp(abts_suite *suite);
abts_suite *testtable(abts_suite *suite);
abts_suite *testtemp(abts_suite *suite);
abts_suite *testthread(abts_suite *suite);
abts_suite *testtime(abts_suite *suite);
abts_suite *testud(abts_suite *suite);
abts_suite *testuser(abts_suite *suite);
abts_suite *testvsn(abts_suite *suite);
# 8 "main2.c" 2

# 1 "./../test/testatomic.c" 1
# 17 "./../test/testatomic.c"
# 1 "./../test/testutil.h" 1
# 19 "./../test/testutil.h"
# 1 "./../test/abts.h" 1
# 20 "./../test/testutil.h" 2
# 18 "./../test/testatomic.c" 2
# 1 "../include/apr_strings.h" 1
# 52 "../include/apr_strings.h"
# 1 "../include/apr_want.h" 1
# 53 "../include/apr_strings.h" 2
# 76 "../include/apr_strings.h"
int apr_strnatcmp(char const *a, char const *b);
# 87 "../include/apr_strings.h"
int apr_strnatcasecmp(char const *a, char const *b);







char * apr_pstrdup(apr_pool_t *p, const char *s);
# 109 "../include/apr_strings.h"
char * apr_pstrmemdup(apr_pool_t *p, const char *s, apr_size_t n)



    ;
# 125 "../include/apr_strings.h"
char * apr_pstrndup(apr_pool_t *p, const char *s, apr_size_t n);
# 135 "../include/apr_strings.h"
void * apr_pmemdup(apr_pool_t *p, const void *m, apr_size_t n)



    ;







char * apr_pstrcat(apr_pool_t *p, ...)

    __attribute__((sentinel))

    ;
# 161 "../include/apr_strings.h"
char * apr_pstrcatv(apr_pool_t *p, const struct iovec *vec,
                                 apr_size_t nvec, apr_size_t *nbytes);
# 172 "../include/apr_strings.h"
char * apr_pvsprintf(apr_pool_t *p, const char *fmt, va_list ap);
# 182 "../include/apr_strings.h"
char * apr_psprintf(apr_pool_t *p, const char *fmt, ...)
        __attribute__((format(printf,2,3)));
# 207 "../include/apr_strings.h"
char * apr_cpystrn(char *dst, const char *src,
                                apr_size_t dst_size);
# 217 "../include/apr_strings.h"
char * apr_collapse_spaces(char *dest, const char *src);
# 226 "../include/apr_strings.h"
apr_status_t apr_tokenize_to_argv(const char *arg_str,
                                               char ***argv_out,
                                               apr_pool_t *token_context);
# 247 "../include/apr_strings.h"
char * apr_strtok(char *str, const char *sep, char **last);
# 279 "../include/apr_strings.h"
int apr_snprintf(char *buf, apr_size_t len,
                                     const char *format, ...)
        __attribute__((format(printf,3,4)));
# 291 "../include/apr_strings.h"
int apr_vsnprintf(char *buf, apr_size_t len, const char *format,
                               va_list ap);
# 301 "../include/apr_strings.h"
char * apr_itoa(apr_pool_t *p, int n);







char * apr_ltoa(apr_pool_t *p, long n);







char * apr_off_t_toa(apr_pool_t *p, apr_off_t n);
# 335 "../include/apr_strings.h"
apr_status_t apr_strtoff(apr_off_t *offset, const char *buf,
                                      char **end, int base);
# 353 "../include/apr_strings.h"
apr_int64_t apr_strtoi64(const char *buf, char **end, int base);
# 362 "../include/apr_strings.h"
apr_int64_t apr_atoi64(const char *buf);
# 372 "../include/apr_strings.h"
char * apr_strfsize(apr_off_t size, char *buf);
# 19 "./../test/testatomic.c" 2
# 1 "../include/apr_thread_proc.h" 1
# 26 "../include/apr_thread_proc.h"
# 1 "../include/apr_file_io.h" 1
# 27 "../include/apr_file_io.h"
# 1 "../include/apr_time.h" 1
# 40 "../include/apr_time.h"
                 extern const char apr_month_snames[12][4];

                 extern const char apr_day_snames[7][4];



typedef apr_int64_t apr_time_t;
# 56 "../include/apr_time.h"
typedef apr_int64_t apr_interval_time_t;

typedef apr_int32_t apr_short_interval_time_t;
# 88 "../include/apr_time.h"
apr_time_t apr_time_now(void);


typedef struct apr_time_exp_t apr_time_exp_t;






struct apr_time_exp_t {

    apr_int32_t tm_usec;

    apr_int32_t tm_sec;

    apr_int32_t tm_min;

    apr_int32_t tm_hour;

    apr_int32_t tm_mday;

    apr_int32_t tm_mon;

    apr_int32_t tm_year;

    apr_int32_t tm_wday;

    apr_int32_t tm_yday;

    apr_int32_t tm_isdst;

    apr_int32_t tm_gmtoff;
};






apr_status_t apr_time_ansi_put(apr_time_t *result,
                                                    time_t input);
# 138 "../include/apr_time.h"
apr_status_t apr_time_exp_tz(apr_time_exp_t *result,
                                          apr_time_t input,
                                          apr_int32_t offs);






apr_status_t apr_time_exp_gmt(apr_time_exp_t *result,
                                           apr_time_t input);






apr_status_t apr_time_exp_lt(apr_time_exp_t *result,
                                          apr_time_t input);







apr_status_t apr_time_exp_get(apr_time_t *result,
                                           apr_time_exp_t *input);







apr_status_t apr_time_exp_gmt_get(apr_time_t *result,
                                               apr_time_exp_t *input);






void apr_sleep(apr_interval_time_t t);
# 193 "../include/apr_time.h"
apr_status_t apr_rfc822_date(char *date_str, apr_time_t t);
# 207 "../include/apr_time.h"
apr_status_t apr_ctime(char *date_str, apr_time_t t);
# 217 "../include/apr_time.h"
apr_status_t apr_strftime(char *s, apr_size_t *retsize,
                                       apr_size_t max, const char *format,
                                       apr_time_exp_t *tm);







void apr_time_clock_hires(apr_pool_t *p);
# 28 "../include/apr_file_io.h" 2

# 1 "../include/apr_file_info.h" 1
# 26 "../include/apr_file_info.h"
# 1 "../include/apr_user.h" 1
# 45 "../include/apr_user.h"
typedef uid_t apr_uid_t;
# 54 "../include/apr_user.h"
typedef gid_t apr_gid_t;
# 66 "../include/apr_user.h"
apr_status_t apr_uid_current(apr_uid_t *userid,
                                          apr_gid_t *groupid,
                                          apr_pool_t *p);
# 77 "../include/apr_user.h"
apr_status_t apr_uid_name_get(char **username, apr_uid_t userid,
                                           apr_pool_t *p);
# 88 "../include/apr_user.h"
apr_status_t apr_uid_get(apr_uid_t *userid, apr_gid_t *groupid,
                                      const char *username, apr_pool_t *p);
# 98 "../include/apr_user.h"
apr_status_t apr_uid_homepath_get(char **dirname,
                                               const char *username,
                                               apr_pool_t *p);
# 123 "../include/apr_user.h"
apr_status_t apr_gid_name_get(char **groupname,
                                             apr_gid_t groupid, apr_pool_t *p);
# 133 "../include/apr_user.h"
apr_status_t apr_gid_get(apr_gid_t *groupid,
                                      const char *groupname, apr_pool_t *p);
# 27 "../include/apr_file_info.h" 2

# 1 "../include/apr_tables.h" 1
# 56 "../include/apr_tables.h"
typedef struct apr_table_t apr_table_t;


typedef struct apr_array_header_t apr_array_header_t;


struct apr_array_header_t {

    apr_pool_t *pool;

    int elt_size;

    int nelts;

    int nalloc;

    char *elts;
};




typedef struct apr_table_entry_t apr_table_entry_t;


struct apr_table_entry_t {

    char *key;



    char *val;


    apr_uint32_t key_checksum;
};






const apr_array_header_t * apr_table_elts(const apr_table_t *t);






int apr_is_empty_table(const apr_table_t *t);






int apr_is_empty_array(const apr_array_header_t *a);
# 121 "../include/apr_tables.h"
apr_array_header_t * apr_array_make(apr_pool_t *p,
                                                 int nelts, int elt_size);
# 131 "../include/apr_tables.h"
void * apr_array_push(apr_array_header_t *arr);
# 158 "../include/apr_tables.h"
void * apr_array_pop(apr_array_header_t *arr);







void apr_array_clear(apr_array_header_t *arr);







void apr_array_cat(apr_array_header_t *dst,
           const apr_array_header_t *src);
# 186 "../include/apr_tables.h"
apr_array_header_t * apr_array_copy(apr_pool_t *p,
                                      const apr_array_header_t *arr);
# 196 "../include/apr_tables.h"
apr_array_header_t * apr_array_copy_hdr(apr_pool_t *p,
                                      const apr_array_header_t *arr);
# 206 "../include/apr_tables.h"
apr_array_header_t * apr_array_append(apr_pool_t *p,
                                      const apr_array_header_t *first,
                                      const apr_array_header_t *second);
# 221 "../include/apr_tables.h"
char * apr_array_pstrcat(apr_pool_t *p,
          const apr_array_header_t *arr,
          const char sep);
# 232 "../include/apr_tables.h"
apr_table_t * apr_table_make(apr_pool_t *p, int nelts);
# 241 "../include/apr_tables.h"
apr_table_t * apr_table_copy(apr_pool_t *p,
                                          const apr_table_t *t);
# 252 "../include/apr_tables.h"
apr_table_t * apr_table_clone(apr_pool_t *p,
                                           const apr_table_t *t);





void apr_table_clear(apr_table_t *t);
# 268 "../include/apr_tables.h"
const char * apr_table_get(const apr_table_t *t, const char *key);
# 279 "../include/apr_tables.h"
const char * apr_table_getm(apr_pool_t *p, const apr_table_t *t,
                                         const char *key);
# 291 "../include/apr_tables.h"
void apr_table_set(apr_table_t *t, const char *key,
                                const char *val);
# 304 "../include/apr_tables.h"
void apr_table_setn(apr_table_t *t, const char *key,
                                 const char *val);






void apr_table_unset(apr_table_t *t, const char *key);
# 323 "../include/apr_tables.h"
void apr_table_merge(apr_table_t *t, const char *key,
                                  const char *val);
# 335 "../include/apr_tables.h"
void apr_table_mergen(apr_table_t *t, const char *key,
                                   const char *val);
# 347 "../include/apr_tables.h"
void apr_table_add(apr_table_t *t, const char *key,
                                const char *val);
# 360 "../include/apr_tables.h"
void apr_table_addn(apr_table_t *t, const char *key,
                                 const char *val);
# 370 "../include/apr_tables.h"
apr_table_t * apr_table_overlay(apr_pool_t *p,
                                             const apr_table_t *overlay,
                                             const apr_table_t *base);
# 384 "../include/apr_tables.h"
typedef int (apr_table_do_callback_fn_t)(void *rec, const char *key,
                                                    const char *value);
# 406 "../include/apr_tables.h"
int apr_table_do(apr_table_do_callback_fn_t *comp,
                                     void *rec, const apr_table_t *t, ...)

    __attribute__((sentinel))

    ;
# 432 "../include/apr_tables.h"
int apr_table_vdo(apr_table_do_callback_fn_t *comp,
                               void *rec, const apr_table_t *t, va_list vp);
# 478 "../include/apr_tables.h"
void apr_table_overlap(apr_table_t *a, const apr_table_t *b,
                                     unsigned flags);
# 491 "../include/apr_tables.h"
void apr_table_compress(apr_table_t *t, unsigned flags);
# 29 "../include/apr_file_info.h" 2
# 62 "../include/apr_file_info.h"
typedef enum {
    APR_NOFILE = 0,
    APR_REG,
    APR_DIR,
    APR_CHR,
    APR_BLK,
    APR_PIPE,
    APR_LNK,
    APR_SOCK,
    APR_UNKFILE = 127
} apr_filetype_e;
# 121 "../include/apr_file_info.h"
typedef struct apr_dir_t apr_dir_t;



typedef apr_int32_t apr_fileperms_t;
# 135 "../include/apr_file_info.h"
typedef dev_t apr_dev_t;







typedef struct apr_finfo_t apr_finfo_t;
# 174 "../include/apr_file_info.h"
struct apr_finfo_t {

    apr_pool_t *pool;


    apr_int32_t valid;

    apr_fileperms_t protection;




    apr_filetype_e filetype;

    apr_uid_t user;

    apr_gid_t group;

    apr_ino_t inode;

    apr_dev_t device;

    apr_int32_t nlink;

    apr_off_t size;

    apr_off_t csize;

    apr_time_t atime;

    apr_time_t mtime;

    apr_time_t ctime;

    const char *fname;

    const char *name;

    struct apr_file_t *filehand;
};
# 229 "../include/apr_file_info.h"
apr_status_t apr_stat(apr_finfo_t *finfo, const char *fname,
                                   apr_int32_t wanted, apr_pool_t *pool);
# 244 "../include/apr_file_info.h"
apr_status_t apr_dir_open(apr_dir_t **new_dir,
                                       const char *dirname,
                                       apr_pool_t *pool);





apr_status_t apr_dir_close(apr_dir_t *thedir);
# 267 "../include/apr_file_info.h"
apr_status_t apr_dir_read(apr_finfo_t *finfo, apr_int32_t wanted,
                                       apr_dir_t *thedir);





apr_status_t apr_dir_rewind(apr_dir_t *thedir);
# 336 "../include/apr_file_info.h"
apr_status_t apr_filepath_root(const char **rootpath,
                                            const char **filepath,
                                            apr_int32_t flags,
                                            apr_pool_t *p);
# 354 "../include/apr_file_info.h"
apr_status_t apr_filepath_merge(char **newpath,
                                             const char *rootpath,
                                             const char *addpath,
                                             apr_int32_t flags,
                                             apr_pool_t *p);
# 369 "../include/apr_file_info.h"
apr_status_t apr_filepath_list_split(apr_array_header_t **pathelts,
                                                  const char *liststr,
                                                  apr_pool_t *p);
# 382 "../include/apr_file_info.h"
apr_status_t apr_filepath_list_merge(char **liststr,
                                                  apr_array_header_t *pathelts,
                                                  apr_pool_t *p);
# 393 "../include/apr_file_info.h"
apr_status_t apr_filepath_get(char **path, apr_int32_t flags,
                                           apr_pool_t *p);






apr_status_t apr_filepath_set(const char *path, apr_pool_t *p);
# 419 "../include/apr_file_info.h"
apr_status_t apr_filepath_encoding(int *style, apr_pool_t *p);
# 30 "../include/apr_file_io.h" 2
# 1 "../include/apr_inherit.h" 1
# 31 "../include/apr_file_io.h" 2



# 1 "../include/apr_want.h" 1
# 35 "../include/apr_file_io.h" 2
# 180 "../include/apr_file_io.h"
typedef apr_uint32_t apr_fileattrs_t;


typedef int apr_seek_where_t;




typedef struct apr_file_t apr_file_t;
# 250 "../include/apr_file_io.h"
apr_status_t apr_file_open(apr_file_t **newf, const char *fname,
                                        apr_int32_t flag, apr_fileperms_t perm,
                                        apr_pool_t *pool);





apr_status_t apr_file_close(apr_file_t *file);
# 267 "../include/apr_file_io.h"
apr_status_t apr_file_remove(const char *path, apr_pool_t *pool);
# 278 "../include/apr_file_io.h"
apr_status_t apr_file_rename(const char *from_path,
                                          const char *to_path,
                                          apr_pool_t *pool);







apr_status_t apr_file_link(const char *from_path,
                                          const char *to_path);
# 303 "../include/apr_file_io.h"
apr_status_t apr_file_copy(const char *from_path,
                                        const char *to_path,
                                        apr_fileperms_t perms,
                                        apr_pool_t *pool);
# 319 "../include/apr_file_io.h"
apr_status_t apr_file_append(const char *from_path,
                                          const char *to_path,
                                          apr_fileperms_t perms,
                                          apr_pool_t *pool);






apr_status_t apr_file_eof(apr_file_t *fptr);
# 346 "../include/apr_file_io.h"
apr_status_t apr_file_open_stderr(apr_file_t **thefile,
                                               apr_pool_t *pool);
# 356 "../include/apr_file_io.h"
apr_status_t apr_file_open_stdout(apr_file_t **thefile,
                                               apr_pool_t *pool);
# 366 "../include/apr_file_io.h"
apr_status_t apr_file_open_stdin(apr_file_t **thefile,
                                              apr_pool_t *pool);
# 386 "../include/apr_file_io.h"
apr_status_t apr_file_open_flags_stderr(apr_file_t **thefile,
                                                     apr_int32_t flags,
                                                     apr_pool_t *pool);
# 407 "../include/apr_file_io.h"
apr_status_t apr_file_open_flags_stdout(apr_file_t **thefile,
                                                     apr_int32_t flags,
                                                     apr_pool_t *pool);
# 428 "../include/apr_file_io.h"
apr_status_t apr_file_open_flags_stdin(apr_file_t **thefile,
                                                     apr_int32_t flags,
                                                     apr_pool_t *pool);
# 449 "../include/apr_file_io.h"
apr_status_t apr_file_read(apr_file_t *thefile, void *buf,
                                        apr_size_t *nbytes);
# 467 "../include/apr_file_io.h"
apr_status_t apr_file_write(apr_file_t *thefile, const void *buf,
                                         apr_size_t *nbytes);
# 485 "../include/apr_file_io.h"
apr_status_t apr_file_writev(apr_file_t *thefile,
                                          const struct iovec *vec,
                                          apr_size_t nvec, apr_size_t *nbytes);
# 509 "../include/apr_file_io.h"
apr_status_t apr_file_read_full(apr_file_t *thefile, void *buf,
                                             apr_size_t nbytes,
                                             apr_size_t *bytes_read);
# 533 "../include/apr_file_io.h"
apr_status_t apr_file_write_full(apr_file_t *thefile,
                                              const void *buf,
                                              apr_size_t nbytes,
                                              apr_size_t *bytes_written);
# 552 "../include/apr_file_io.h"
apr_status_t apr_file_writev_full(apr_file_t *thefile,
                                               const struct iovec *vec,
                                               apr_size_t nvec,
                                               apr_size_t *nbytes);





apr_status_t apr_file_putc(char ch, apr_file_t *thefile);






apr_status_t apr_file_getc(char *ch, apr_file_t *thefile);






apr_status_t apr_file_ungetc(char ch, apr_file_t *thefile);
# 585 "../include/apr_file_io.h"
apr_status_t apr_file_gets(char *str, int len,
                                        apr_file_t *thefile);






apr_status_t apr_file_puts(const char *str, apr_file_t *thefile);





apr_status_t apr_file_flush(apr_file_t *thefile);





apr_status_t apr_file_sync(apr_file_t *thefile);





apr_status_t apr_file_datasync(apr_file_t *thefile);
# 620 "../include/apr_file_io.h"
apr_status_t apr_file_dup(apr_file_t **new_file,
                                       apr_file_t *old_file,
                                       apr_pool_t *p);
# 632 "../include/apr_file_io.h"
apr_status_t apr_file_dup2(apr_file_t *new_file,
                                        apr_file_t *old_file,
                                        apr_pool_t *p);
# 647 "../include/apr_file_io.h"
apr_status_t apr_file_setaside(apr_file_t **new_file,
                                            apr_file_t *old_file,
                                            apr_pool_t *p);
# 662 "../include/apr_file_io.h"
apr_status_t apr_file_buffer_set(apr_file_t *thefile,
                                              char * buffer,
                                              apr_size_t bufsize);





apr_size_t apr_file_buffer_size_get(apr_file_t *thefile);
# 683 "../include/apr_file_io.h"
apr_status_t apr_file_seek(apr_file_t *thefile,
                                   apr_seek_where_t where,
                                   apr_off_t *offset);
# 700 "../include/apr_file_io.h"
apr_status_t apr_file_pipe_create(apr_file_t **in,
                                               apr_file_t **out,
                                               apr_pool_t *pool);
# 723 "../include/apr_file_io.h"
apr_status_t apr_file_pipe_create_ex(apr_file_t **in,
                                                  apr_file_t **out,
                                                  apr_int32_t blocking,
                                                  apr_pool_t *pool);







apr_status_t apr_file_namedpipe_create(const char *filename,
                                                    apr_fileperms_t perm,
                                                    apr_pool_t *pool);






apr_status_t apr_file_pipe_timeout_get(apr_file_t *thepipe,
                                               apr_interval_time_t *timeout);







apr_status_t apr_file_pipe_timeout_set(apr_file_t *thepipe,
                                                  apr_interval_time_t timeout);
# 766 "../include/apr_file_io.h"
apr_status_t apr_file_lock(apr_file_t *thefile, int type);





apr_status_t apr_file_unlock(apr_file_t *thefile);
# 781 "../include/apr_file_io.h"
apr_status_t apr_file_name_get(const char **new_path,
                                            apr_file_t *thefile);







apr_status_t apr_file_data_get(void **data, const char *key,
                                            apr_file_t *file);
# 800 "../include/apr_file_io.h"
apr_status_t apr_file_data_set(apr_file_t *file, void *data,
                                            const char *key,
                                            apr_status_t (*cleanup)(void *));
# 811 "../include/apr_file_io.h"
int apr_file_printf(apr_file_t *fptr,
                                        const char *format, ...)
        __attribute__((format(printf,2,3)));
# 827 "../include/apr_file_io.h"
apr_status_t apr_file_perms_set(const char *fname,
                                             apr_fileperms_t perms);
# 846 "../include/apr_file_io.h"
apr_status_t apr_file_attrs_set(const char *fname,
                                             apr_fileattrs_t attributes,
                                             apr_fileattrs_t attr_mask,
                                             apr_pool_t *pool);
# 859 "../include/apr_file_io.h"
apr_status_t apr_file_mtime_set(const char *fname,
                                             apr_time_t mtime,
                                             apr_pool_t *pool);







apr_status_t apr_dir_make(const char *path, apr_fileperms_t perm,
                                       apr_pool_t *pool);
# 879 "../include/apr_file_io.h"
apr_status_t apr_dir_make_recursive(const char *path,
                                                 apr_fileperms_t perm,
                                                 apr_pool_t *pool);
# 890 "../include/apr_file_io.h"
apr_status_t apr_dir_remove(const char *path, apr_pool_t *pool);







apr_status_t apr_file_info_get(apr_finfo_t *finfo,
                                            apr_int32_t wanted,
                                            apr_file_t *thefile);
# 909 "../include/apr_file_io.h"
apr_status_t apr_file_trunc(apr_file_t *fp, apr_off_t offset);






apr_int32_t apr_file_flags_get(apr_file_t *f);




apr_pool_t * apr_file_pool_get (const apr_file_t *thefile);





apr_status_t apr_file_inherit_set( apr_file_t *thefile);




apr_status_t apr_file_inherit_unset( apr_file_t *thefile);
# 951 "../include/apr_file_io.h"
apr_status_t apr_file_mktemp(apr_file_t **fp, char *templ,
                                          apr_int32_t flags, apr_pool_t *p);
# 964 "../include/apr_file_io.h"
apr_status_t apr_temp_dir_get(const char **temp_dir,
                                           apr_pool_t *p);
# 27 "../include/apr_thread_proc.h" 2




# 1 "/usr/include/x86_64-linux-gnu/sys/time.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/sys/time.h" 2 3 4

# 1 "/usr/include/x86_64-linux-gnu/bits/time.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/sys/time.h" 2 3 4
# 55 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
struct timezone
  {
    int tz_minuteswest;
    int tz_dsttime;
  };

typedef struct timezone *__restrict __timezone_ptr_t;
# 71 "/usr/include/x86_64-linux-gnu/sys/time.h" 3 4
extern int gettimeofday (struct timeval *__restrict __tv,
    __timezone_ptr_t __tz) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));




extern int settimeofday (const struct timeval *__tv,
    const struct timezone *__tz)
     __attribute__ ((__nothrow__ ));





extern int adjtime (const struct timeval *__delta,
      struct timeval *__olddelta) __attribute__ ((__nothrow__ ));




enum __itimer_which
  {

    ITIMER_REAL = 0,


    ITIMER_VIRTUAL = 1,



    ITIMER_PROF = 2

  };



struct itimerval
  {

    struct timeval it_interval;

    struct timeval it_value;
  };






typedef int __itimer_which_t;




extern int getitimer (__itimer_which_t __which,
        struct itimerval *__value) __attribute__ ((__nothrow__ ));




extern int setitimer (__itimer_which_t __which,
        const struct itimerval *__restrict __new,
        struct itimerval *__restrict __old) __attribute__ ((__nothrow__ ));




extern int utimes (const char *__file, const struct timeval __tvp[2])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int lutimes (const char *__file, const struct timeval __tvp[2])
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int futimes (int __fd, const struct timeval __tvp[2]) __attribute__ ((__nothrow__ ));
# 32 "../include/apr_thread_proc.h" 2
# 1 "/usr/include/x86_64-linux-gnu/sys/resource.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/sys/resource.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/resource.h" 1 3 4
# 31 "/usr/include/x86_64-linux-gnu/bits/resource.h" 3 4
enum __rlimit_resource
{

  RLIMIT_CPU = 0,



  RLIMIT_FSIZE = 1,



  RLIMIT_DATA = 2,



  RLIMIT_STACK = 3,



  RLIMIT_CORE = 4,






  __RLIMIT_RSS = 5,



  RLIMIT_NOFILE = 7,
  __RLIMIT_OFILE = RLIMIT_NOFILE,




  RLIMIT_AS = 9,



  __RLIMIT_NPROC = 6,



  __RLIMIT_MEMLOCK = 8,



  __RLIMIT_LOCKS = 10,



  __RLIMIT_SIGPENDING = 11,



  __RLIMIT_MSGQUEUE = 12,





  __RLIMIT_NICE = 13,




  __RLIMIT_RTPRIO = 14,





  __RLIMIT_RTTIME = 15,


  __RLIMIT_NLIMITS = 16,
  __RLIM_NLIMITS = __RLIMIT_NLIMITS


};
# 131 "/usr/include/x86_64-linux-gnu/bits/resource.h" 3 4
typedef __rlim_t rlim_t;







struct rlimit
  {

    rlim_t rlim_cur;

    rlim_t rlim_max;
  };
# 158 "/usr/include/x86_64-linux-gnu/bits/resource.h" 3 4
enum __rusage_who
{

  RUSAGE_SELF = 0,



  RUSAGE_CHILDREN = -1
# 176 "/usr/include/x86_64-linux-gnu/bits/resource.h" 3 4
};



# 1 "/usr/include/x86_64-linux-gnu/bits/time.h" 1 3 4
# 180 "/usr/include/x86_64-linux-gnu/bits/resource.h" 2 3 4







struct rusage
  {

    struct timeval ru_utime;

    struct timeval ru_stime;

    __extension__ union
      {
 long int ru_maxrss;
 __syscall_slong_t __ru_maxrss_word;
      };



    __extension__ union
      {
 long int ru_ixrss;
 __syscall_slong_t __ru_ixrss_word;
      };

    __extension__ union
      {
 long int ru_idrss;
 __syscall_slong_t __ru_idrss_word;
      };

    __extension__ union
      {
 long int ru_isrss;
  __syscall_slong_t __ru_isrss_word;
      };


    __extension__ union
      {
 long int ru_minflt;
 __syscall_slong_t __ru_minflt_word;
      };

    __extension__ union
      {
 long int ru_majflt;
 __syscall_slong_t __ru_majflt_word;
      };

    __extension__ union
      {
 long int ru_nswap;
 __syscall_slong_t __ru_nswap_word;
      };


    __extension__ union
      {
 long int ru_inblock;
 __syscall_slong_t __ru_inblock_word;
      };

    __extension__ union
      {
 long int ru_oublock;
 __syscall_slong_t __ru_oublock_word;
      };

    __extension__ union
      {
 long int ru_msgsnd;
 __syscall_slong_t __ru_msgsnd_word;
      };

    __extension__ union
      {
 long int ru_msgrcv;
 __syscall_slong_t __ru_msgrcv_word;
      };

    __extension__ union
      {
 long int ru_nsignals;
 __syscall_slong_t __ru_nsignals_word;
      };



    __extension__ union
      {
 long int ru_nvcsw;
 __syscall_slong_t __ru_nvcsw_word;
      };


    __extension__ union
      {
 long int ru_nivcsw;
 __syscall_slong_t __ru_nivcsw_word;
      };
  };







enum __priority_which
{
  PRIO_PROCESS = 0,

  PRIO_PGRP = 1,

  PRIO_USER = 2

};
# 25 "/usr/include/x86_64-linux-gnu/sys/resource.h" 2 3 4
# 42 "/usr/include/x86_64-linux-gnu/sys/resource.h" 3 4
typedef int __rlimit_resource_t;
typedef int __rusage_who_t;
typedef int __priority_which_t;





extern int getrlimit (__rlimit_resource_t __resource,
        struct rlimit *__rlimits) __attribute__ ((__nothrow__ ));
# 69 "/usr/include/x86_64-linux-gnu/sys/resource.h" 3 4
extern int setrlimit (__rlimit_resource_t __resource,
        const struct rlimit *__rlimits) __attribute__ ((__nothrow__ ));
# 87 "/usr/include/x86_64-linux-gnu/sys/resource.h" 3 4
extern int getrusage (__rusage_who_t __who, struct rusage *__usage) __attribute__ ((__nothrow__ ));





extern int getpriority (__priority_which_t __which, id_t __who) __attribute__ ((__nothrow__ ));



extern int setpriority (__priority_which_t __which, id_t __who, int __prio)
     __attribute__ ((__nothrow__ ));
# 33 "../include/apr_thread_proc.h" 2
# 45 "../include/apr_thread_proc.h"
typedef enum {
    APR_SHELLCMD,
    APR_PROGRAM,
    APR_PROGRAM_ENV,
    APR_PROGRAM_PATH,
    APR_SHELLCMD_ENV


} apr_cmdtype_e;

typedef enum {
    APR_WAIT,
    APR_NOWAIT
} apr_wait_how_e;






typedef enum {
    APR_PROC_EXIT = 1,
    APR_PROC_SIGNAL = 2,
    APR_PROC_SIGNAL_CORE = 4
} apr_exit_why_e;
# 133 "../include/apr_thread_proc.h"
typedef struct apr_proc_t {

    pid_t pid;

    apr_file_t *in;

    apr_file_t *out;

    apr_file_t *err;
# 161 "../include/apr_thread_proc.h"
} apr_proc_t;
# 173 "../include/apr_thread_proc.h"
typedef void (apr_child_errfn_t)(apr_pool_t *proc, apr_status_t err,
                                 const char *description);


typedef struct apr_thread_t apr_thread_t;


typedef struct apr_threadattr_t apr_threadattr_t;


typedef struct apr_procattr_t apr_procattr_t;


typedef struct apr_thread_once_t apr_thread_once_t;


typedef struct apr_threadkey_t apr_threadkey_t;


typedef struct apr_other_child_rec_t apr_other_child_rec_t;




typedef void *( *apr_thread_start_t)(apr_thread_t*, void*);

typedef enum {
    APR_KILL_NEVER,


    APR_KILL_ALWAYS,
    APR_KILL_AFTER_TIMEOUT,
    APR_JUST_WAIT,
    APR_KILL_ONLY_ONCE
} apr_kill_conditions_e;
# 218 "../include/apr_thread_proc.h"
apr_status_t apr_threadattr_create(apr_threadattr_t **new_attr,
                                                apr_pool_t *cont);






apr_status_t apr_threadattr_detach_set(apr_threadattr_t *attr,
                                                    apr_int32_t on);







apr_status_t apr_threadattr_detach_get(apr_threadattr_t *attr);






apr_status_t apr_threadattr_stacksize_set(apr_threadattr_t *attr,
                                                       apr_size_t stacksize);
# 255 "../include/apr_thread_proc.h"
apr_status_t apr_threadattr_guardsize_set(apr_threadattr_t *attr,
                                                       apr_size_t guardsize);
# 266 "../include/apr_thread_proc.h"
apr_status_t apr_thread_create(apr_thread_t **new_thread,
                                            apr_threadattr_t *attr,
                                            apr_thread_start_t func,
                                            void *data, apr_pool_t *cont);






apr_status_t apr_thread_exit(apr_thread_t *thd,
                                          apr_status_t retval);






apr_status_t apr_thread_join(apr_status_t *retval,
                                          apr_thread_t *thd);




void apr_thread_yield(void);







apr_status_t apr_thread_once_init(apr_thread_once_t **control,
                                               apr_pool_t *p);
# 310 "../include/apr_thread_proc.h"
apr_status_t apr_thread_once(apr_thread_once_t *control,
                                          void (*func)(void));





apr_status_t apr_thread_detach(apr_thread_t *thd);







apr_status_t apr_thread_data_get(void **data, const char *key,
                                             apr_thread_t *thread);
# 335 "../include/apr_thread_proc.h"
apr_status_t apr_thread_data_set(void *data, const char *key,
                                             apr_status_t (*cleanup) (void *),
                                             apr_thread_t *thread);







apr_status_t apr_threadkey_private_create(apr_threadkey_t **key,
                                                    void (*dest)(void *),
                                                    apr_pool_t *cont);






apr_status_t apr_threadkey_private_get(void **new_mem,
                                                 apr_threadkey_t *key);






apr_status_t apr_threadkey_private_set(void *priv,
                                                 apr_threadkey_t *key);





apr_status_t apr_threadkey_private_delete(apr_threadkey_t *key);







apr_status_t apr_threadkey_data_get(void **data, const char *key,
                                                apr_threadkey_t *threadkey);
# 387 "../include/apr_thread_proc.h"
apr_status_t apr_threadkey_data_set(void *data, const char *key,
                                                apr_status_t (*cleanup) (void *),
                                                apr_threadkey_t *threadkey);
# 398 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_create(apr_procattr_t **new_attr,
                                                  apr_pool_t *cont);
# 415 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_io_set(apr_procattr_t *attr,
                                             apr_int32_t in, apr_int32_t out,
                                             apr_int32_t err);
# 434 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_child_in_set(struct apr_procattr_t *attr,
                                                  apr_file_t *child_in,
                                                  apr_file_t *parent_in);
# 451 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_child_out_set(struct apr_procattr_t *attr,
                                                   apr_file_t *child_out,
                                                   apr_file_t *parent_out);
# 468 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_child_err_set(struct apr_procattr_t *attr,
                                                   apr_file_t *child_err,
                                                   apr_file_t *parent_err);
# 479 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_dir_set(apr_procattr_t *attr,
                                              const char *dir);
# 493 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_cmdtype_set(apr_procattr_t *attr,
                                                  apr_cmdtype_e cmd);






apr_status_t apr_procattr_detach_set(apr_procattr_t *attr,
                                                 apr_int32_t detach);
# 517 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_limit_set(apr_procattr_t *attr,
                                                apr_int32_t what,
                                                struct rlimit *limit);
# 533 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_child_errfn_set(apr_procattr_t *attr,
                                                       apr_child_errfn_t *errfn);
# 548 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_error_check_set(apr_procattr_t *attr,
                                                       apr_int32_t chk);
# 558 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_addrspace_set(apr_procattr_t *attr,
                                                       apr_int32_t addrspace);
# 569 "../include/apr_thread_proc.h"
apr_status_t apr_procattr_user_set(apr_procattr_t *attr,
                                                const char *username,
                                                const char *password);






apr_status_t apr_procattr_group_set(apr_procattr_t *attr,
                                                 const char *groupname);
# 591 "../include/apr_thread_proc.h"
apr_status_t apr_proc_fork(apr_proc_t *proc, apr_pool_t *cont);
# 610 "../include/apr_thread_proc.h"
apr_status_t apr_proc_create(apr_proc_t *new_proc,
                                          const char *progname,
                                          const char * const *args,
                                          const char * const *env,
                                          apr_procattr_t *attr,
                                          apr_pool_t *pool);
# 643 "../include/apr_thread_proc.h"
apr_status_t apr_proc_wait(apr_proc_t *proc,
                                        int *exitcode, apr_exit_why_e *exitwhy,
                                        apr_wait_how_e waithow);
# 673 "../include/apr_thread_proc.h"
apr_status_t apr_proc_wait_all_procs(apr_proc_t *proc,
                                                  int *exitcode,
                                                  apr_exit_why_e *exitwhy,
                                                  apr_wait_how_e waithow,
                                                  apr_pool_t *p);
# 688 "../include/apr_thread_proc.h"
apr_status_t apr_proc_detach(int daemonize);
# 707 "../include/apr_thread_proc.h"
void apr_proc_other_child_register(apr_proc_t *proc,
                                           void (*maintenance) (int reason,
                                                                void *,
                                                                int status),
                                           void *data, apr_file_t *write_fd,
                                           apr_pool_t *p);
# 723 "../include/apr_thread_proc.h"
void apr_proc_other_child_unregister(void *data);
# 745 "../include/apr_thread_proc.h"
apr_status_t apr_proc_other_child_alert(apr_proc_t *proc,
                                                     int reason,
                                                     int status);
# 756 "../include/apr_thread_proc.h"
void apr_proc_other_child_refresh(apr_other_child_rec_t *ocr,
                                               int reason);







void apr_proc_other_child_refresh_all(int reason);






apr_status_t apr_proc_kill(apr_proc_t *proc, int sig);
# 787 "../include/apr_thread_proc.h"
void apr_pool_note_subprocess(apr_pool_t *a, apr_proc_t *proc,
                                           apr_kill_conditions_e how);
# 798 "../include/apr_thread_proc.h"
apr_status_t apr_setup_signal_thread(void);
# 807 "../include/apr_thread_proc.h"
apr_status_t apr_signal_thread(int(*signal_handler)(int signum));







apr_pool_t * apr_thread_pool_get (const apr_thread_t *thethread);
# 20 "./../test/testatomic.c" 2


# 1 "../include/apr_atomic.h" 1
# 47 "../include/apr_atomic.h"
apr_status_t apr_atomic_init(apr_pool_t *p);
# 59 "../include/apr_atomic.h"
apr_uint32_t apr_atomic_read32(volatile apr_uint32_t *mem);






void apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val);







apr_uint32_t apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val);






void apr_atomic_sub32(volatile apr_uint32_t *mem, apr_uint32_t val);






apr_uint32_t apr_atomic_inc32(volatile apr_uint32_t *mem);






int apr_atomic_dec32(volatile apr_uint32_t *mem);
# 105 "../include/apr_atomic.h"
apr_uint32_t apr_atomic_cas32(volatile apr_uint32_t *mem, apr_uint32_t with,
                              apr_uint32_t cmp);







apr_uint32_t apr_atomic_xchg32(volatile apr_uint32_t *mem, apr_uint32_t val);
# 124 "../include/apr_atomic.h"
void* apr_atomic_casptr(volatile void **mem, void *with, const void *cmp);







void* apr_atomic_xchgptr(volatile void **mem, void *with);
# 23 "./../test/testatomic.c" 2
# 37 "./../test/testatomic.c"
static void test_init(abts_case *tc, void *data)
{
    apr_assert_success(tc, "Could not initliaze atomics", apr_atomic_init(p), 39);
}

static void test_set32(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    apr_atomic_set32(&y32, 2);
    abts_int_equal(tc, 2, y32, 46);
}

static void test_read32(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    apr_atomic_set32(&y32, 2);
    abts_int_equal(tc, 2, apr_atomic_read32(&y32), 53);
}

static void test_dec32(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    int rv;

    apr_atomic_set32(&y32, 2);

    rv = apr_atomic_dec32(&y32);
    abts_int_equal(tc, 1, y32, 64);
    abts_assert(tc, "atomic_dec returned zero when it shouldn't", rv != 0, 65);;

    rv = apr_atomic_dec32(&y32);
    abts_int_equal(tc, 0, y32, 68);
    abts_assert(tc, "atomic_dec didn't returned zero when it should", rv == 0, 69);;
}

static void test_xchg32(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 100);
    oldval = apr_atomic_xchg32(&y32, 50);

    abts_int_equal(tc, 100, oldval, 80);
    abts_int_equal(tc, 50, y32, 81);
}

static void test_xchgptr(abts_case *tc, void *data)
{
    int a;
    void *ref = "little piggy";
    volatile void *target_ptr = ref;
    void *old_ptr;

    old_ptr = apr_atomic_xchgptr(&target_ptr, &a);
    abts_ptr_equal(tc, ref, old_ptr, 92);
    abts_ptr_equal(tc, &a, (void *) target_ptr, 93);
}

static void test_cas_equal(abts_case *tc, void *data)
{
    apr_uint32_t casval = 0;
    apr_uint32_t oldval;

    oldval = apr_atomic_cas32(&casval, 12, 0);
    abts_int_equal(tc, 0, oldval, 102);
    abts_int_equal(tc, 12, casval, 103);
}

static void test_cas_equal_nonnull(abts_case *tc, void *data)
{
    apr_uint32_t casval = 12;
    apr_uint32_t oldval;

    oldval = apr_atomic_cas32(&casval, 23, 12);
    abts_int_equal(tc, 12, oldval, 112);
    abts_int_equal(tc, 23, casval, 113);
}

static void test_cas_notequal(abts_case *tc, void *data)
{
    apr_uint32_t casval = 12;
    apr_uint32_t oldval;

    oldval = apr_atomic_cas32(&casval, 23, 2);
    abts_int_equal(tc, 12, oldval, 122);
    abts_int_equal(tc, 12, casval, 123);
}

static void test_casptr_equal(abts_case *tc, void *data)
{
    int a;
    volatile void *target_ptr = ((void*)0);
    void *old_ptr;

    old_ptr = apr_atomic_casptr(&target_ptr, &a, ((void*)0));
    abts_ptr_equal(tc, ((void*)0), old_ptr, 133);
    abts_ptr_equal(tc, &a, (void *) target_ptr, 134);
}

static void test_casptr_equal_nonnull(abts_case *tc, void *data)
{
    int a, b;
    volatile void *target_ptr = &a;
    void *old_ptr;

    old_ptr = apr_atomic_casptr(&target_ptr, &b, &a);
    abts_ptr_equal(tc, &a, old_ptr, 144);
    abts_ptr_equal(tc, &b, (void *) target_ptr, 145);
}

static void test_casptr_notequal(abts_case *tc, void *data)
{
    int a, b;
    volatile void *target_ptr = &a;
    void *old_ptr;

    old_ptr = apr_atomic_casptr(&target_ptr, &a, &b);
    abts_ptr_equal(tc, &a, old_ptr, 155);
    abts_ptr_equal(tc, &a, (void *) target_ptr, 156);
}

static void test_add32(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 23);
    oldval = apr_atomic_add32(&y32, 4);
    abts_int_equal(tc, 23, oldval, 166);
    abts_int_equal(tc, 27, y32, 167);
}

static void test_add32_neg(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 23);
    oldval = apr_atomic_add32(&y32, -10);
    abts_int_equal(tc, 23, oldval, 177);
    abts_int_equal(tc, 13, y32, 178);
}

static void test_inc32(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 23);
    oldval = apr_atomic_inc32(&y32);
    abts_int_equal(tc, 23, oldval, 188);
    abts_int_equal(tc, 24, y32, 189);
}

static void test_set_add_inc_sub(abts_case *tc, void *data)
{
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 0);
    apr_atomic_add32(&y32, 20);
    apr_atomic_inc32(&y32);
    apr_atomic_sub32(&y32, 10);

    abts_int_equal(tc, 11, y32, 201);
}

static void test_wrap_zero(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    apr_uint32_t rv;
    apr_uint32_t minus1 = -1;
    char *str;

    apr_atomic_set32(&y32, 0);
    rv = apr_atomic_dec32(&y32);

    abts_assert(tc, "apr_atomic_dec32 on zero returned zero.", rv != 0, 214);;
    str = apr_psprintf(p, "zero wrap failed: 0 - 1 = %d", y32);
    abts_assert(tc, str, y32 == minus1, 216);;
}

static void test_inc_neg1(abts_case *tc, void *data)
{
    apr_uint32_t y32 = -1;
    apr_uint32_t minus1 = -1;
    apr_uint32_t rv;
    char *str;

    rv = apr_atomic_inc32(&y32);

    abts_assert(tc, "apr_atomic_inc32 didn't return the old value.", rv == minus1, 228);;
    str = apr_psprintf(p, "zero wrap failed: -1 + 1 = %d", y32);
    abts_assert(tc, str, y32 == 0, 230);;
}




void * thread_func_mutex(apr_thread_t *thd, void *data);
void * thread_func_atomic(apr_thread_t *thd, void *data);

apr_thread_mutex_t *thread_lock;
volatile apr_uint32_t mutex_locks = 0;
volatile apr_uint32_t atomic_ops = 0;
apr_status_t exit_ret_val = 123;

void * thread_func_mutex(apr_thread_t *thd, void *data)
{
    int i;

    for (i = 0; i < 1; i++) {
        apr_thread_mutex_lock(thread_lock);
        mutex_locks++;
        apr_thread_mutex_unlock(thread_lock);
    }
    apr_thread_exit(thd, exit_ret_val);
    return ((void*)0);
}

void * thread_func_atomic(apr_thread_t *thd, void *data)
{
    int i;

    for (i = 0; i < 1 ; i++) {
        apr_atomic_inc32(&atomic_ops);
        apr_atomic_add32(&atomic_ops, 2);
        apr_atomic_dec32(&atomic_ops);
        apr_atomic_dec32(&atomic_ops);
    }
    apr_thread_exit(thd, exit_ret_val);
    return ((void*)0);
}

static void test_atomics_threaded(abts_case *tc, void *data)
{
    apr_thread_t *t1[2];
    apr_thread_t *t2[2];
    apr_status_t rv;
    int i;





    rv = apr_thread_mutex_create(&thread_lock, 0x0, p);
    apr_assert_success(tc, "Could not create lock", rv, 283);

    for (i = 0; i < 2; i++) {
        apr_status_t r1, r2;
        r1 = apr_thread_create(&t1[i], ((void*)0), thread_func_mutex, ((void*)0), p);
        r2 = apr_thread_create(&t2[i], ((void*)0), thread_func_atomic, ((void*)0), p);
        abts_assert(tc, "Failed creating threads", !r1 && !r2, 289);;
    }

    for (i = 0; i < 2; i++) {
        apr_status_t s1, s2;
        apr_thread_join(&s1, t1[i]);
        apr_thread_join(&s2, t2[i]);

        abts_assert(tc, "Invalid return value from thread_join", s1 == exit_ret_val && s2 == exit_ret_val, 298);;

    }

    abts_int_equal(tc, 2 * 1, mutex_locks, 301);
    abts_int_equal(tc, 2 * 1, apr_atomic_read32(&atomic_ops), 303);


    rv = apr_thread_mutex_destroy(thread_lock);
    abts_assert(tc, "Failed creating threads", rv == 0, 306);;
}

typedef struct tbox_t tbox_t;

struct tbox_t {
    abts_case *tc;
    apr_uint32_t *mem;
    apr_uint32_t preval;
    apr_uint32_t postval;
    apr_uint32_t loop;
    void (*func)(tbox_t *box);
};

static __inline__ void busyloop_read32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        val = apr_atomic_read32(tbox->mem);

        if (val != tbox->preval)
            apr_thread_yield();
        else
            break;
    } while (1);
}

static void busyloop_set32(tbox_t *tbox)
{
    do {
        busyloop_read32(tbox);
        apr_atomic_set32(tbox->mem, tbox->postval);
    } while (--tbox->loop);
}

static void busyloop_add32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_add32(tbox->mem, tbox->postval);
        apr_thread_mutex_lock(thread_lock);
        abts_int_equal(tbox->tc, val, tbox->preval, 350);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void busyloop_sub32(tbox_t *tbox)
{
    do {
        busyloop_read32(tbox);
        apr_atomic_sub32(tbox->mem, tbox->postval);
    } while (--tbox->loop);
}

static void busyloop_inc32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_inc32(tbox->mem);
        apr_thread_mutex_lock(thread_lock);
        abts_int_equal(tbox->tc, val, tbox->preval, 371);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void busyloop_dec32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_dec32(tbox->mem);
        apr_thread_mutex_lock(thread_lock);
        abts_int_nequal(tbox->tc, 0, val, 384);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void busyloop_cas32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        do {
            val = apr_atomic_cas32(tbox->mem, tbox->postval, tbox->preval);

            if (val != tbox->preval)
                apr_thread_yield();
            else
                break;
        } while (1);
    } while (--tbox->loop);
}

static void busyloop_xchg32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_xchg32(tbox->mem, tbox->postval);
        apr_thread_mutex_lock(thread_lock);
        abts_int_equal(tbox->tc, val, tbox->preval, 413);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void * thread_func_busyloop(apr_thread_t *thd, void *data)
{
    tbox_t *tbox = data;

    tbox->func(tbox);

    apr_thread_exit(thd, 0);

    return ((void*)0);
}

static void test_atomics_busyloop_threaded(abts_case *tc, void *data)
{
    unsigned int i;
    apr_status_t rv;
    apr_uint32_t count = 0;
    tbox_t tbox[2];
    apr_thread_t *thread[2];

    rv = apr_thread_mutex_create(&thread_lock, 0x0, p);
    apr_assert_success(tc, "Could not create lock", rv, 438);


    for (i = 0; i < 2; i++) {
        tbox[i].tc = tc;
        tbox[i].mem = &count;
        tbox[i].loop = 50;
    }

    tbox[0].preval = 98;
    tbox[0].postval = 3891;
    tbox[0].func = busyloop_add32;

    tbox[1].preval = 3989;
    tbox[1].postval = 1010;
    tbox[1].func = busyloop_sub32;

    tbox[2].preval = 2979;
    tbox[2].postval = 0;
    tbox[2].func = busyloop_inc32;

    tbox[3].preval = 2980;
    tbox[3].postval = 16384;
    tbox[3].func = busyloop_set32;

    tbox[4].preval = 16384;
    tbox[4].postval = 0;
    tbox[4].func = busyloop_dec32;

    tbox[5].preval = 16383;
    tbox[5].postval = 1048576;
    tbox[5].func = busyloop_cas32;

    tbox[6].preval = 1048576;
    tbox[6].postval = 98;
    tbox[6].func = busyloop_xchg32;


    for (i = 0; i < 2; i++) {
        rv = apr_thread_create(&thread[i], ((void*)0), thread_func_busyloop,
                               &tbox[i], p);
        abts_assert(tc, "Failed creating thread", rv == 0, 479);;
    }


    apr_atomic_set32(tbox->mem, 98);

    for (i = 0; i < 2; i++) {
        apr_status_t retval;
        rv = apr_thread_join(&retval, thread[i]);
        abts_assert(tc, "Thread join failed", rv == 0, 488);;
        abts_assert(tc, "Invalid return value from thread_join", retval == 0, 489);;
    }

    abts_int_equal(tbox->tc, 98, count, 492);

    rv = apr_thread_mutex_destroy(thread_lock);
    abts_assert(tc, "Failed creating threads", rv == 0, 495);;
}



abts_suite *testatomic(abts_suite *suite)
{
    suite = abts_add_suite(suite, "./../test/testatomic.c");

    abts_run_test(suite, test_init, ((void*)0));
    abts_run_test(suite, test_set32, ((void*)0));
    abts_run_test(suite, test_read32, ((void*)0));
    abts_run_test(suite, test_dec32, ((void*)0));
    abts_run_test(suite, test_xchg32, ((void*)0));
    abts_run_test(suite, test_xchgptr, ((void*)0));
    abts_run_test(suite, test_cas_equal, ((void*)0));
    abts_run_test(suite, test_cas_equal_nonnull, ((void*)0));
    abts_run_test(suite, test_cas_notequal, ((void*)0));
    abts_run_test(suite, test_casptr_equal, ((void*)0));
    abts_run_test(suite, test_casptr_equal_nonnull, ((void*)0));
    abts_run_test(suite, test_casptr_notequal, ((void*)0));
    abts_run_test(suite, test_add32, ((void*)0));
    abts_run_test(suite, test_add32_neg, ((void*)0));
    abts_run_test(suite, test_inc32, ((void*)0));
    abts_run_test(suite, test_set_add_inc_sub, ((void*)0));
    abts_run_test(suite, test_wrap_zero, ((void*)0));
    abts_run_test(suite, test_inc_neg1, ((void*)0));


    abts_run_test(suite, test_atomics_threaded, ((void*)0));
    abts_run_test(suite, test_atomics_busyloop_threaded, ((void*)0));


    return suite;
}
# 10 "main2.c" 2
# 1 "./../test/testutil.c" 1
# 20 "./../test/testutil.c"
# 1 "./../test/abts.h" 1
# 21 "./../test/testutil.c" 2
# 1 "./../test/testutil.h" 1
# 19 "./../test/testutil.h"
# 1 "./../test/abts.h" 1
# 20 "./../test/testutil.h" 2
# 22 "./../test/testutil.c" 2


apr_pool_t *p;

void apr_assert_success(abts_case* tc, const char* context, apr_status_t rv,
                        int lineno)
{
    if (rv == ((20000 + 50000) + 23)) {
        abts_not_impl(tc, context, lineno);
    } else if (rv != 0) {
        char buf[8096], ebuf[128];
        sprintf(buf, "%s (%d): %s\n", context, rv,
                apr_strerror(rv, ebuf, sizeof ebuf));
        abts_fail(tc, buf, lineno);
    }
}

void initialize(void) {
    apr_initialize();
    atexit(apr_terminate);

    apr_pool_create_ex(&p, ((void*)0), ((void*)0), ((void*)0));
}
# 11 "main2.c" 2
# 1 "./../locks/unix/thread_mutex.c" 1
# 17 "./../locks/unix/thread_mutex.c"
# 1 "../include/arch/unix/apr_arch_thread_mutex.h" 1
# 21 "../include/arch/unix/apr_arch_thread_mutex.h"
# 1 "../include/arch/unix/apr_private.h" 1
# 999 "../include/arch/unix/apr_private.h"
# 1 "../include/arch/unix/../apr_private_common.h" 1
# 27 "../include/arch/unix/../apr_private_common.h"
apr_status_t apr_filepath_list_split_impl(apr_array_header_t **pathelts,
                                          const char *liststr,
                                          char separator,
                                          apr_pool_t *p);

apr_status_t apr_filepath_list_merge_impl(char **liststr,
                                          apr_array_header_t *pathelts,
                                          char separator,
                                          apr_pool_t *p);
# 1000 "../include/arch/unix/apr_private.h" 2
# 22 "../include/arch/unix/apr_arch_thread_mutex.h" 2


# 1 "../include/apr_portable.h" 1
# 32 "../include/apr_portable.h"
# 1 "../include/apr_network_io.h" 1
# 31 "../include/apr_network_io.h"
# 1 "/usr/include/netinet/in.h" 1 3 4
# 22 "/usr/include/netinet/in.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stdint.h" 1 3 4
# 23 "/usr/include/netinet/in.h" 2 3 4







typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };



# 1 "/usr/include/x86_64-linux-gnu/bits/in.h" 1 3 4
# 112 "/usr/include/x86_64-linux-gnu/bits/in.h" 3 4
struct ip_opts
  {
    struct in_addr ip_dst;
    char ip_opts[40];
  };


struct ip_mreqn
  {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
  };


struct in_pktinfo
  {
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
  };
# 38 "/usr/include/netinet/in.h" 2 3 4


enum
  {
    IPPROTO_IP = 0,

    IPPROTO_ICMP = 1,

    IPPROTO_IGMP = 2,

    IPPROTO_IPIP = 4,

    IPPROTO_TCP = 6,

    IPPROTO_EGP = 8,

    IPPROTO_PUP = 12,

    IPPROTO_UDP = 17,

    IPPROTO_IDP = 22,

    IPPROTO_TP = 29,

    IPPROTO_DCCP = 33,

    IPPROTO_IPV6 = 41,

    IPPROTO_RSVP = 46,

    IPPROTO_GRE = 47,

    IPPROTO_ESP = 50,

    IPPROTO_AH = 51,

    IPPROTO_MTP = 92,

    IPPROTO_BEETPH = 94,

    IPPROTO_ENCAP = 98,

    IPPROTO_PIM = 103,

    IPPROTO_COMP = 108,

    IPPROTO_SCTP = 132,

    IPPROTO_UDPLITE = 136,

    IPPROTO_RAW = 255,

    IPPROTO_MAX
  };





enum
  {
    IPPROTO_HOPOPTS = 0,

    IPPROTO_ROUTING = 43,

    IPPROTO_FRAGMENT = 44,

    IPPROTO_ICMPV6 = 58,

    IPPROTO_NONE = 59,

    IPPROTO_DSTOPTS = 60,

    IPPROTO_MH = 135

  };



typedef uint16_t in_port_t;


enum
  {
    IPPORT_ECHO = 7,
    IPPORT_DISCARD = 9,
    IPPORT_SYSTAT = 11,
    IPPORT_DAYTIME = 13,
    IPPORT_NETSTAT = 15,
    IPPORT_FTP = 21,
    IPPORT_TELNET = 23,
    IPPORT_SMTP = 25,
    IPPORT_TIMESERVER = 37,
    IPPORT_NAMESERVER = 42,
    IPPORT_WHOIS = 43,
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,


    IPPORT_EXECSERVER = 512,
    IPPORT_LOGINSERVER = 513,
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,


    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,


    IPPORT_RESERVED = 1024,


    IPPORT_USERRESERVED = 5000
  };
# 209 "/usr/include/netinet/in.h" 3 4
struct in6_addr
  {
    union
      {
 uint8_t __u6_addr8[16];

 uint16_t __u6_addr16[8];
 uint32_t __u6_addr32[4];

      } __in6_u;





  };


extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;
# 237 "/usr/include/netinet/in.h" 3 4
struct sockaddr_in
  {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;


    unsigned char sin_zero[sizeof (struct sockaddr) -
      (sizeof (unsigned short int)) -
      sizeof (in_port_t) -
      sizeof (struct in_addr)];
  };



struct sockaddr_in6
  {
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
  };




struct ip_mreq
  {

    struct in_addr imr_multiaddr;


    struct in_addr imr_interface;
  };

struct ip_mreq_source
  {

    struct in_addr imr_multiaddr;


    struct in_addr imr_interface;


    struct in_addr imr_sourceaddr;
  };




struct ipv6_mreq
  {

    struct in6_addr ipv6mr_multiaddr;


    unsigned int ipv6mr_interface;
  };




struct group_req
  {

    uint32_t gr_interface;


    struct sockaddr_storage gr_group;
  };

struct group_source_req
  {

    uint32_t gsr_interface;


    struct sockaddr_storage gsr_group;


    struct sockaddr_storage gsr_source;
  };



struct ip_msfilter
  {

    struct in_addr imsf_multiaddr;


    struct in_addr imsf_interface;


    uint32_t imsf_fmode;


    uint32_t imsf_numsrc;

    struct in_addr imsf_slist[1];
  };





struct group_filter
  {

    uint32_t gf_interface;


    struct sockaddr_storage gf_group;


    uint32_t gf_fmode;


    uint32_t gf_numsrc;

    struct sockaddr_storage gf_slist[1];
};
# 374 "/usr/include/netinet/in.h" 3 4
extern uint32_t ntohl (uint32_t __netlong) __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
extern uint16_t ntohs (uint16_t __netshort)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
extern uint32_t htonl (uint32_t __hostlong)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
extern uint16_t htons (uint16_t __hostshort)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));





# 1 "/usr/include/x86_64-linux-gnu/bits/byteswap.h" 1 3 4
# 386 "/usr/include/netinet/in.h" 2 3 4
# 501 "/usr/include/netinet/in.h" 3 4
extern int bindresvport (int __sockfd, struct sockaddr_in *__sock_in) __attribute__ ((__nothrow__ ));


extern int bindresvport6 (int __sockfd, struct sockaddr_in6 *__sock_in)
     __attribute__ ((__nothrow__ ));
# 32 "../include/apr_network_io.h" 2
# 108 "../include/apr_network_io.h"
typedef enum {
    APR_SHUTDOWN_READ,
    APR_SHUTDOWN_WRITE,
    APR_SHUTDOWN_READWRITE
} apr_shutdown_how_e;
# 172 "../include/apr_network_io.h"
typedef enum {
    APR_LOCAL,
    APR_REMOTE
} apr_interface_e;
# 193 "../include/apr_network_io.h"
typedef struct apr_socket_t apr_socket_t;



typedef struct apr_hdtr_t apr_hdtr_t;

typedef struct in_addr apr_in_addr_t;

typedef struct apr_ipsubnet_t apr_ipsubnet_t;


typedef apr_uint16_t apr_port_t;




typedef struct apr_sockaddr_t apr_sockaddr_t;



struct apr_sockaddr_t {

    apr_pool_t *pool;

    char *hostname;

    char *servname;

    apr_port_t port;

    apr_int32_t family;

    apr_socklen_t salen;

    int ipaddr_len;


    int addr_str_len;


    void *ipaddr_ptr;


    apr_sockaddr_t *next;

    union {

        struct sockaddr_in sin;


        struct sockaddr_in6 sin6;




        struct sockaddr_storage sas;

    } sa;
};
# 263 "../include/apr_network_io.h"
struct apr_hdtr_t {

    struct iovec* headers;

    int numheaders;

    struct iovec* trailers;

    int numtrailers;
};
# 287 "../include/apr_network_io.h"
apr_status_t apr_socket_create(apr_socket_t **new_sock,
                                            int family, int type,
                                            int protocol,
                                            apr_pool_t *cont);
# 305 "../include/apr_network_io.h"
apr_status_t apr_socket_shutdown(apr_socket_t *thesocket,
                                              apr_shutdown_how_e how);





apr_status_t apr_socket_close(apr_socket_t *thesocket);
# 321 "../include/apr_network_io.h"
apr_status_t apr_socket_bind(apr_socket_t *sock,
                                          apr_sockaddr_t *sa);
# 331 "../include/apr_network_io.h"
apr_status_t apr_socket_listen(apr_socket_t *sock,
                                            apr_int32_t backlog);
# 345 "../include/apr_network_io.h"
apr_status_t apr_socket_accept(apr_socket_t **new_sock,
                                            apr_socket_t *sock,
                                            apr_pool_t *connection_pool);







apr_status_t apr_socket_connect(apr_socket_t *sock,
                                             apr_sockaddr_t *sa);
# 370 "../include/apr_network_io.h"
apr_status_t apr_socket_atreadeof(apr_socket_t *sock,
                                               int *atreadeof);
# 396 "../include/apr_network_io.h"
apr_status_t apr_sockaddr_info_get(apr_sockaddr_t **sa,
                                          const char *hostname,
                                          apr_int32_t family,
                                          apr_port_t port,
                                          apr_int32_t flags,
                                          apr_pool_t *p);
# 411 "../include/apr_network_io.h"
apr_status_t apr_getnameinfo(char **hostname,
                                          apr_sockaddr_t *sa,
                                          apr_int32_t flags);
# 445 "../include/apr_network_io.h"
apr_status_t apr_parse_addr_port(char **addr,
                                              char **scope_id,
                                              apr_port_t *port,
                                              const char *str,
                                              apr_pool_t *p);
# 459 "../include/apr_network_io.h"
apr_status_t apr_gethostname(char *buf, int len, apr_pool_t *cont);







apr_status_t apr_socket_data_get(void **data, const char *key,
                                              apr_socket_t *sock);
# 477 "../include/apr_network_io.h"
apr_status_t apr_socket_data_set(apr_socket_t *sock, void *data,
                                              const char *key,
                                              apr_status_t (*cleanup)(void*));
# 498 "../include/apr_network_io.h"
apr_status_t apr_socket_send(apr_socket_t *sock, const char *buf,
                                          apr_size_t *len);
# 519 "../include/apr_network_io.h"
apr_status_t apr_socket_sendv(apr_socket_t *sock,
                                           const struct iovec *vec,
                                           apr_int32_t nvec, apr_size_t *len);
# 530 "../include/apr_network_io.h"
apr_status_t apr_socket_sendto(apr_socket_t *sock,
                                            apr_sockaddr_t *where,
                                            apr_int32_t flags, const char *buf,
                                            apr_size_t *len);
# 548 "../include/apr_network_io.h"
apr_status_t apr_socket_recvfrom(apr_sockaddr_t *from,
                                              apr_socket_t *sock,
                                              apr_int32_t flags, char *buf,
                                              apr_size_t *len);
# 573 "../include/apr_network_io.h"
apr_status_t apr_socket_sendfile(apr_socket_t *sock,
                                              apr_file_t *file,
                                              apr_hdtr_t *hdtr,
                                              apr_off_t *offset,
                                              apr_size_t *len,
                                              apr_int32_t flags);
# 601 "../include/apr_network_io.h"
apr_status_t apr_socket_recv(apr_socket_t *sock,
                                   char *buf, apr_size_t *len);
# 626 "../include/apr_network_io.h"
apr_status_t apr_socket_opt_set(apr_socket_t *sock,
                                             apr_int32_t opt, apr_int32_t on);
# 640 "../include/apr_network_io.h"
apr_status_t apr_socket_timeout_set(apr_socket_t *sock,
                                                 apr_interval_time_t t);
# 662 "../include/apr_network_io.h"
apr_status_t apr_socket_opt_get(apr_socket_t *sock,
                                             apr_int32_t opt, apr_int32_t *on);






apr_status_t apr_socket_timeout_get(apr_socket_t *sock,
                                                 apr_interval_time_t *t);







apr_status_t apr_socket_atmark(apr_socket_t *sock,
                                            int *atmark);
# 690 "../include/apr_network_io.h"
apr_status_t apr_socket_addr_get(apr_sockaddr_t **sa,
                                              apr_interface_e which,
                                              apr_socket_t *sock);
# 701 "../include/apr_network_io.h"
apr_status_t apr_sockaddr_ip_get(char **addr,
                                              apr_sockaddr_t *sockaddr);






apr_status_t apr_sockaddr_ip_getbuf(char *buf, apr_size_t buflen,
                                                 apr_sockaddr_t *sockaddr);
# 722 "../include/apr_network_io.h"
int apr_sockaddr_equal(const apr_sockaddr_t *addr1,
                                    const apr_sockaddr_t *addr2);
# 733 "../include/apr_network_io.h"
int apr_sockaddr_is_wildcard(const apr_sockaddr_t *addr);






apr_status_t apr_socket_type_get(apr_socket_t *sock,
                                              int *type);






apr_status_t apr_getservbyname(apr_sockaddr_t *sockaddr,
                                            const char *servname);
# 758 "../include/apr_network_io.h"
apr_status_t apr_ipsubnet_create(apr_ipsubnet_t **ipsub,
                                              const char *ipstr,
                                              const char *mask_or_numbits,
                                              apr_pool_t *p);
# 770 "../include/apr_network_io.h"
int apr_ipsubnet_test(apr_ipsubnet_t *ipsub, apr_sockaddr_t *sa);
# 791 "../include/apr_network_io.h"
apr_status_t apr_socket_protocol_get(apr_socket_t *sock,
                                                  int *protocol);




apr_pool_t * apr_socket_pool_get (const apr_socket_t *thesocket);




apr_status_t apr_socket_inherit_set( apr_socket_t *thesocket);




apr_status_t apr_socket_inherit_unset( apr_socket_t *thesocket);
# 823 "../include/apr_network_io.h"
apr_status_t apr_mcast_join(apr_socket_t *sock,
                                         apr_sockaddr_t *join,
                                         apr_sockaddr_t *iface,
                                         apr_sockaddr_t *source);
# 838 "../include/apr_network_io.h"
apr_status_t apr_mcast_leave(apr_socket_t *sock,
                                          apr_sockaddr_t *addr,
                                          apr_sockaddr_t *iface,
                                          apr_sockaddr_t *source);
# 850 "../include/apr_network_io.h"
apr_status_t apr_mcast_hops(apr_socket_t *sock,
                                         apr_byte_t ttl);






apr_status_t apr_mcast_loopback(apr_socket_t *sock,
                                             apr_byte_t opt);







apr_status_t apr_mcast_interface(apr_socket_t *sock,
                                              apr_sockaddr_t *iface);
# 33 "../include/apr_portable.h" 2

# 1 "../include/apr_global_mutex.h" 1
# 26 "../include/apr_global_mutex.h"
# 1 "../include/apr_proc_mutex.h" 1
# 44 "../include/apr_proc_mutex.h"
typedef enum {
    APR_LOCK_FCNTL,
    APR_LOCK_FLOCK,
    APR_LOCK_SYSVSEM,
    APR_LOCK_PROC_PTHREAD,
    APR_LOCK_POSIXSEM,
    APR_LOCK_DEFAULT
} apr_lockmech_e;


typedef struct apr_proc_mutex_t apr_proc_mutex_t;
# 79 "../include/apr_proc_mutex.h"
apr_status_t apr_proc_mutex_create(apr_proc_mutex_t **mutex,
                                                const char *fname,
                                                apr_lockmech_e mech,
                                                apr_pool_t *pool);
# 95 "../include/apr_proc_mutex.h"
apr_status_t apr_proc_mutex_child_init(apr_proc_mutex_t **mutex,
                                                    const char *fname,
                                                    apr_pool_t *pool);






apr_status_t apr_proc_mutex_lock(apr_proc_mutex_t *mutex);
# 113 "../include/apr_proc_mutex.h"
apr_status_t apr_proc_mutex_trylock(apr_proc_mutex_t *mutex);





apr_status_t apr_proc_mutex_unlock(apr_proc_mutex_t *mutex);





apr_status_t apr_proc_mutex_destroy(apr_proc_mutex_t *mutex);







apr_status_t apr_proc_mutex_cleanup(void *mutex);






const char * apr_proc_mutex_lockfile(apr_proc_mutex_t *mutex);






const char * apr_proc_mutex_name(apr_proc_mutex_t *mutex);




const char * apr_proc_mutex_defname(void);





apr_pool_t * apr_proc_mutex_pool_get (const apr_proc_mutex_t *theproc_mutex);
# 27 "../include/apr_global_mutex.h" 2
# 46 "../include/apr_global_mutex.h"
typedef struct apr_global_mutex_t apr_global_mutex_t;
# 74 "../include/apr_global_mutex.h"
apr_status_t apr_global_mutex_create(apr_global_mutex_t **mutex,
                                                  const char *fname,
                                                  apr_lockmech_e mech,
                                                  apr_pool_t *pool);
# 90 "../include/apr_global_mutex.h"
apr_status_t apr_global_mutex_child_init(
                              apr_global_mutex_t **mutex,
                              const char *fname,
                              apr_pool_t *pool);






apr_status_t apr_global_mutex_lock(apr_global_mutex_t *mutex);
# 109 "../include/apr_global_mutex.h"
apr_status_t apr_global_mutex_trylock(apr_global_mutex_t *mutex);





apr_status_t apr_global_mutex_unlock(apr_global_mutex_t *mutex);





apr_status_t apr_global_mutex_destroy(apr_global_mutex_t *mutex);





const char * apr_global_mutex_lockfile(apr_global_mutex_t *mutex);







const char * apr_global_mutex_name(apr_global_mutex_t *mutex);





apr_pool_t * apr_global_mutex_pool_get (const apr_global_mutex_t *theglobal_mutex);
# 35 "../include/apr_portable.h" 2


# 1 "../include/apr_dso.h" 1
# 44 "../include/apr_dso.h"
typedef struct apr_dso_handle_t apr_dso_handle_t;




typedef void * apr_dso_handle_sym_t;
# 59 "../include/apr_dso.h"
apr_status_t apr_dso_load(apr_dso_handle_t **res_handle,
                                       const char *path, apr_pool_t *ctx);





apr_status_t apr_dso_unload(apr_dso_handle_t *handle);







apr_status_t apr_dso_sym(apr_dso_handle_sym_t *ressym,
                                      apr_dso_handle_t *handle,
                                      const char *symname);







const char * apr_dso_error(apr_dso_handle_t *dso, char *buf, apr_size_t bufsize);
# 38 "../include/apr_portable.h" 2
# 1 "../include/apr_shm.h" 1
# 43 "../include/apr_shm.h"
typedef struct apr_shm_t apr_shm_t;
# 69 "../include/apr_shm.h"
apr_status_t apr_shm_create(apr_shm_t **m,
                                         apr_size_t reqsize,
                                         const char *filename,
                                         apr_pool_t *pool);
# 114 "../include/apr_shm.h"
apr_status_t apr_shm_create_ex(apr_shm_t **m,
                                            apr_size_t reqsize,
                                            const char *filename,
                                            apr_pool_t *pool,
                                            apr_int32_t flags);
# 135 "../include/apr_shm.h"
apr_status_t apr_shm_remove(const char *filename,
                                         apr_pool_t *pool);





apr_status_t apr_shm_destroy(apr_shm_t *m);
# 153 "../include/apr_shm.h"
apr_status_t apr_shm_attach(apr_shm_t **m,
                                         const char *filename,
                                         apr_pool_t *pool);
# 167 "../include/apr_shm.h"
apr_status_t apr_shm_attach_ex(apr_shm_t **m,
                                            const char *filename,
                                            apr_pool_t *pool,
                                            apr_int32_t flags);






apr_status_t apr_shm_detach(apr_shm_t *m);
# 188 "../include/apr_shm.h"
void * apr_shm_baseaddr_get(const apr_shm_t *m);






apr_size_t apr_shm_size_get(const apr_shm_t *m);




apr_pool_t * apr_shm_pool_get (const apr_shm_t *theshm);
# 39 "../include/apr_portable.h" 2


# 1 "/usr/include/dirent.h" 1 3 4
# 61 "/usr/include/dirent.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/dirent.h" 1 3 4
# 22 "/usr/include/x86_64-linux-gnu/bits/dirent.h" 3 4
struct dirent
  {

    __ino_t d_ino;
    __off_t d_off;




    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];
  };
# 62 "/usr/include/dirent.h" 2 3 4
# 97 "/usr/include/dirent.h" 3 4
enum
  {
    DT_UNKNOWN = 0,

    DT_FIFO = 1,

    DT_CHR = 2,

    DT_DIR = 4,

    DT_BLK = 6,

    DT_REG = 8,

    DT_LNK = 10,

    DT_SOCK = 12,

    DT_WHT = 14

  };
# 127 "/usr/include/dirent.h" 3 4
typedef struct __dirstream DIR;






extern DIR *opendir (const char *__name) __attribute__ ((__nonnull__ (1)));






extern DIR *fdopendir (int __fd);







extern int closedir (DIR *__dirp) __attribute__ ((__nonnull__ (1)));
# 162 "/usr/include/dirent.h" 3 4
extern struct dirent *readdir (DIR *__dirp) __attribute__ ((__nonnull__ (1)));
# 183 "/usr/include/dirent.h" 3 4
extern int readdir_r (DIR *__restrict __dirp,
        struct dirent *__restrict __entry,
        struct dirent **__restrict __result)
     __attribute__ ((__nonnull__ (1, 2, 3)));
# 208 "/usr/include/dirent.h" 3 4
extern void rewinddir (DIR *__dirp) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern void seekdir (DIR *__dirp, long int __pos) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern long int telldir (DIR *__dirp) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int dirfd (DIR *__dirp) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 244 "/usr/include/dirent.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 245 "/usr/include/dirent.h" 2 3 4
# 254 "/usr/include/dirent.h" 3 4
extern int scandir (const char *__restrict __dir,
      struct dirent ***__restrict __namelist,
      int (*__selector) (const struct dirent *),
      int (*__cmp) (const struct dirent **,
      const struct dirent **))
     __attribute__ ((__nonnull__ (1, 2)));
# 324 "/usr/include/dirent.h" 3 4
extern int alphasort (const struct dirent **__e1,
        const struct dirent **__e2)
     __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 352 "/usr/include/dirent.h" 3 4
extern __ssize_t getdirentries (int __fd, char *__restrict __buf,
    size_t __nbytes,
    __off_t *__restrict __basep)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 4)));
# 42 "../include/apr_portable.h" 2


# 1 "/usr/include/fcntl.h" 1 3 4
# 35 "/usr/include/fcntl.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 1 3 4
# 35 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 3 4
struct flock
  {
    short int l_type;
    short int l_whence;

    __off_t l_start;
    __off_t l_len;




    __pid_t l_pid;
  };
# 61 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/fcntl-linux.h" 1 3 4
# 62 "/usr/include/x86_64-linux-gnu/bits/fcntl.h" 2 3 4
# 36 "/usr/include/fcntl.h" 2 3 4
# 67 "/usr/include/fcntl.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 68 "/usr/include/fcntl.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stat.h" 1 3 4
# 46 "/usr/include/x86_64-linux-gnu/bits/stat.h" 3 4
struct stat
  {
    __dev_t st_dev;




    __ino_t st_ino;







    __nlink_t st_nlink;
    __mode_t st_mode;

    __uid_t st_uid;
    __gid_t st_gid;

    int __pad0;

    __dev_t st_rdev;




    __off_t st_size;



    __blksize_t st_blksize;

    __blkcnt_t st_blocks;
# 91 "/usr/include/x86_64-linux-gnu/bits/stat.h" 3 4
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
# 106 "/usr/include/x86_64-linux-gnu/bits/stat.h" 3 4
    __syscall_slong_t __glibc_reserved[3];
# 115 "/usr/include/x86_64-linux-gnu/bits/stat.h" 3 4
  };
# 69 "/usr/include/fcntl.h" 2 3 4
# 137 "/usr/include/fcntl.h" 3 4
extern int fcntl (int __fd, int __cmd, ...);
# 146 "/usr/include/fcntl.h" 3 4
extern int open (const char *__file, int __oflag, ...) __attribute__ ((__nonnull__ (1)));
# 170 "/usr/include/fcntl.h" 3 4
extern int openat (int __fd, const char *__file, int __oflag, ...)
     __attribute__ ((__nonnull__ (2)));
# 192 "/usr/include/fcntl.h" 3 4
extern int creat (const char *__file, mode_t __mode) __attribute__ ((__nonnull__ (1)));
# 238 "/usr/include/fcntl.h" 3 4
extern int posix_fadvise (int __fd, off_t __offset, off_t __len,
     int __advise) __attribute__ ((__nothrow__ ));
# 260 "/usr/include/fcntl.h" 3 4
extern int posix_fallocate (int __fd, off_t __offset, off_t __len);
# 45 "../include/apr_portable.h" 2


# 1 "/usr/include/pthread.h" 1 3 4
# 23 "/usr/include/pthread.h" 3 4
# 1 "/usr/include/sched.h" 1 3 4
# 28 "/usr/include/sched.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 29 "/usr/include/sched.h" 2 3 4



# 1 "/usr/include/time.h" 1 3 4
# 33 "/usr/include/sched.h" 2 3 4








# 1 "/usr/include/x86_64-linux-gnu/bits/sched.h" 1 3 4
# 72 "/usr/include/x86_64-linux-gnu/bits/sched.h" 3 4
struct sched_param
  {
    int __sched_priority;
  };
# 103 "/usr/include/x86_64-linux-gnu/bits/sched.h" 3 4
struct __sched_param
  {
    int __sched_priority;
  };
# 118 "/usr/include/x86_64-linux-gnu/bits/sched.h" 3 4
typedef unsigned long int __cpu_mask;






typedef struct
{
  __cpu_mask __bits[1024 / (8 * sizeof (__cpu_mask))];
} cpu_set_t;
# 203 "/usr/include/x86_64-linux-gnu/bits/sched.h" 3 4
extern int __sched_cpucount (size_t __setsize, const cpu_set_t *__setp)
  __attribute__ ((__nothrow__ ));
extern cpu_set_t *__sched_cpualloc (size_t __count) __attribute__ ((__nothrow__ )) ;
extern void __sched_cpufree (cpu_set_t *__set) __attribute__ ((__nothrow__ ));
# 42 "/usr/include/sched.h" 2 3 4







extern int sched_setparam (__pid_t __pid, const struct sched_param *__param)
     __attribute__ ((__nothrow__ ));


extern int sched_getparam (__pid_t __pid, struct sched_param *__param) __attribute__ ((__nothrow__ ));


extern int sched_setscheduler (__pid_t __pid, int __policy,
          const struct sched_param *__param) __attribute__ ((__nothrow__ ));


extern int sched_getscheduler (__pid_t __pid) __attribute__ ((__nothrow__ ));


extern int sched_yield (void) __attribute__ ((__nothrow__ ));


extern int sched_get_priority_max (int __algorithm) __attribute__ ((__nothrow__ ));


extern int sched_get_priority_min (int __algorithm) __attribute__ ((__nothrow__ ));


extern int sched_rr_get_interval (__pid_t __pid, struct timespec *__t) __attribute__ ((__nothrow__ ));
# 24 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/time.h" 1 3 4
# 37 "/usr/include/time.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 38 "/usr/include/time.h" 2 3 4



# 1 "/usr/include/x86_64-linux-gnu/bits/time.h" 1 3 4
# 42 "/usr/include/time.h" 2 3 4
# 133 "/usr/include/time.h" 3 4
struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;


  long int tm_gmtoff;
  const char *tm_zone;




};
# 161 "/usr/include/time.h" 3 4
struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };


struct sigevent;
# 189 "/usr/include/time.h" 3 4
extern clock_t clock (void) __attribute__ ((__nothrow__ ));


extern time_t time (time_t *__timer) __attribute__ ((__nothrow__ ));


extern double difftime (time_t __time1, time_t __time0)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));


extern time_t mktime (struct tm *__tp) __attribute__ ((__nothrow__ ));





extern size_t strftime (char *__restrict __s, size_t __maxsize,
   const char *__restrict __format,
   const struct tm *__restrict __tp) __attribute__ ((__nothrow__ ));
# 223 "/usr/include/time.h" 3 4
extern size_t strftime_l (char *__restrict __s, size_t __maxsize,
     const char *__restrict __format,
     const struct tm *__restrict __tp,
     __locale_t __loc) __attribute__ ((__nothrow__ ));
# 239 "/usr/include/time.h" 3 4
extern struct tm *gmtime (const time_t *__timer) __attribute__ ((__nothrow__ ));



extern struct tm *localtime (const time_t *__timer) __attribute__ ((__nothrow__ ));





extern struct tm *gmtime_r (const time_t *__restrict __timer,
       struct tm *__restrict __tp) __attribute__ ((__nothrow__ ));



extern struct tm *localtime_r (const time_t *__restrict __timer,
          struct tm *__restrict __tp) __attribute__ ((__nothrow__ ));





extern char *asctime (const struct tm *__tp) __attribute__ ((__nothrow__ ));


extern char *ctime (const time_t *__timer) __attribute__ ((__nothrow__ ));







extern char *asctime_r (const struct tm *__restrict __tp,
   char *__restrict __buf) __attribute__ ((__nothrow__ ));


extern char *ctime_r (const time_t *__restrict __timer,
        char *__restrict __buf) __attribute__ ((__nothrow__ ));




extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;




extern char *tzname[2];



extern void tzset (void) __attribute__ ((__nothrow__ ));



extern int daylight;
extern long int timezone;





extern int stime (const time_t *__when) __attribute__ ((__nothrow__ ));
# 319 "/usr/include/time.h" 3 4
extern time_t timegm (struct tm *__tp) __attribute__ ((__nothrow__ ));


extern time_t timelocal (struct tm *__tp) __attribute__ ((__nothrow__ ));


extern int dysize (int __year) __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
# 334 "/usr/include/time.h" 3 4
extern int nanosleep (const struct timespec *__requested_time,
        struct timespec *__remaining);



extern int clock_getres (clockid_t __clock_id, struct timespec *__res) __attribute__ ((__nothrow__ ));


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __attribute__ ((__nothrow__ ));


extern int clock_settime (clockid_t __clock_id, const struct timespec *__tp)
     __attribute__ ((__nothrow__ ));






extern int clock_nanosleep (clockid_t __clock_id, int __flags,
       const struct timespec *__req,
       struct timespec *__rem);


extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id) __attribute__ ((__nothrow__ ));




extern int timer_create (clockid_t __clock_id,
    struct sigevent *__restrict __evp,
    timer_t *__restrict __timerid) __attribute__ ((__nothrow__ ));


extern int timer_delete (timer_t __timerid) __attribute__ ((__nothrow__ ));


extern int timer_settime (timer_t __timerid, int __flags,
     const struct itimerspec *__restrict __value,
     struct itimerspec *__restrict __ovalue) __attribute__ ((__nothrow__ ));


extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     __attribute__ ((__nothrow__ ));


extern int timer_getoverrun (timer_t __timerid) __attribute__ ((__nothrow__ ));
# 25 "/usr/include/pthread.h" 2 3 4


# 1 "/usr/include/x86_64-linux-gnu/bits/setjmp.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/bits/setjmp.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 27 "/usr/include/x86_64-linux-gnu/bits/setjmp.h" 2 3 4




typedef long int __jmp_buf[8];
# 28 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 29 "/usr/include/pthread.h" 2 3 4



enum
{
  PTHREAD_CREATE_JOINABLE,

  PTHREAD_CREATE_DETACHED

};



enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP

  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL





};




enum
{
  PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_STALLED_NP = PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_ROBUST,
  PTHREAD_MUTEX_ROBUST_NP = PTHREAD_MUTEX_ROBUST
};





enum
{
  PTHREAD_PRIO_NONE,
  PTHREAD_PRIO_INHERIT,
  PTHREAD_PRIO_PROTECT
};
# 125 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP
};
# 166 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_INHERIT_SCHED,

  PTHREAD_EXPLICIT_SCHED

};



enum
{
  PTHREAD_SCOPE_SYSTEM,

  PTHREAD_SCOPE_PROCESS

};



enum
{
  PTHREAD_PROCESS_PRIVATE,

  PTHREAD_PROCESS_SHARED

};
# 201 "/usr/include/pthread.h" 3 4
struct _pthread_cleanup_buffer
{
  void (*__routine) (void *);
  void *__arg;
  int __canceltype;
  struct _pthread_cleanup_buffer *__prev;
};


enum
{
  PTHREAD_CANCEL_ENABLE,

  PTHREAD_CANCEL_DISABLE

};
enum
{
  PTHREAD_CANCEL_DEFERRED,

  PTHREAD_CANCEL_ASYNCHRONOUS

};
# 244 "/usr/include/pthread.h" 3 4
extern int pthread_create (pthread_t *__restrict __newthread,
      const pthread_attr_t *__restrict __attr,
      void *(*__start_routine) (void *),
      void *__restrict __arg) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3)));





extern void pthread_exit (void *__retval) __attribute__ ((__noreturn__));







extern int pthread_join (pthread_t __th, void **__thread_return);
# 282 "/usr/include/pthread.h" 3 4
extern int pthread_detach (pthread_t __th) __attribute__ ((__nothrow__ ));



extern pthread_t pthread_self (void) __attribute__ ((__nothrow__ )) __attribute__ ((__const__));


extern int pthread_equal (pthread_t __thread1, pthread_t __thread2)
  __attribute__ ((__nothrow__ )) __attribute__ ((__const__));







extern int pthread_attr_init (pthread_attr_t *__attr) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_destroy (pthread_attr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getdetachstate (const pthread_attr_t *__attr,
     int *__detachstate)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setdetachstate (pthread_attr_t *__attr,
     int __detachstate)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getguardsize (const pthread_attr_t *__attr,
          size_t *__guardsize)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setguardsize (pthread_attr_t *__attr,
          size_t __guardsize)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getschedparam (const pthread_attr_t *__restrict __attr,
           struct sched_param *__restrict __param)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr,
           const struct sched_param *__restrict
           __param) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_getschedpolicy (const pthread_attr_t *__restrict
     __attr, int *__restrict __policy)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getinheritsched (const pthread_attr_t *__restrict
      __attr, int *__restrict __inherit)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setinheritsched (pthread_attr_t *__attr,
      int __inherit)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getscope (const pthread_attr_t *__restrict __attr,
      int *__restrict __scope)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getstackaddr (const pthread_attr_t *__restrict
          __attr, void **__restrict __stackaddr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__deprecated__));





extern int pthread_attr_setstackaddr (pthread_attr_t *__attr,
          void *__stackaddr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__));


extern int pthread_attr_getstacksize (const pthread_attr_t *__restrict
          __attr, size_t *__restrict __stacksize)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_attr_setstacksize (pthread_attr_t *__attr,
          size_t __stacksize)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getstack (const pthread_attr_t *__restrict __attr,
      void **__restrict __stackaddr,
      size_t *__restrict __stacksize)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2, 3)));




extern int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
      size_t __stacksize) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 440 "/usr/include/pthread.h" 3 4
extern int pthread_setschedparam (pthread_t __target_thread, int __policy,
      const struct sched_param *__param)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3)));


extern int pthread_getschedparam (pthread_t __target_thread,
      int *__restrict __policy,
      struct sched_param *__restrict __param)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3)));


extern int pthread_setschedprio (pthread_t __target_thread, int __prio)
     __attribute__ ((__nothrow__ ));
# 505 "/usr/include/pthread.h" 3 4
extern int pthread_once (pthread_once_t *__once_control,
    void (*__init_routine) (void)) __attribute__ ((__nonnull__ (1, 2)));
# 517 "/usr/include/pthread.h" 3 4
extern int pthread_setcancelstate (int __state, int *__oldstate);



extern int pthread_setcanceltype (int __type, int *__oldtype);


extern int pthread_cancel (pthread_t __th);




extern void pthread_testcancel (void);




typedef struct
{
  struct
  {
    __jmp_buf __cancel_jmp_buf;
    int __mask_was_saved;
  } __cancel_jmp_buf[1];
  void *__pad[4];
} __pthread_unwind_buf_t __attribute__ ((__aligned__));
# 551 "/usr/include/pthread.h" 3 4
struct __pthread_cleanup_frame
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;
};
# 691 "/usr/include/pthread.h" 3 4
extern void __pthread_register_cancel (__pthread_unwind_buf_t *__buf)
                            ;
# 703 "/usr/include/pthread.h" 3 4
extern void __pthread_unregister_cancel (__pthread_unwind_buf_t *__buf)
                         ;
# 744 "/usr/include/pthread.h" 3 4
extern void __pthread_unwind_next (__pthread_unwind_buf_t *__buf)
                             __attribute__ ((__noreturn__))

     __attribute__ ((__weak__))

     ;



struct __jmp_buf_tag;
extern int __sigsetjmp (struct __jmp_buf_tag *__env, int __savemask) __attribute__ ((__nothrow__));





extern int pthread_mutex_init (pthread_mutex_t *__mutex,
          const pthread_mutexattr_t *__mutexattr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_destroy (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_trylock (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_lock (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,
        const struct timespec *__restrict
        __abstime) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutex_unlock (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutex_getprioceiling (const pthread_mutex_t *
      __restrict __mutex,
      int *__restrict __prioceiling)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutex_setprioceiling (pthread_mutex_t *__restrict __mutex,
      int __prioceiling,
      int *__restrict __old_ceiling)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 3)));




extern int pthread_mutex_consistent (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 817 "/usr/include/pthread.h" 3 4
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getpshared (const pthread_mutexattr_t *
      __restrict __attr,
      int *__restrict __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
      int __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_gettype (const pthread_mutexattr_t *__restrict
          __attr, int *__restrict __kind)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_getprotocol (const pthread_mutexattr_t *
       __restrict __attr,
       int *__restrict __protocol)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutexattr_setprotocol (pthread_mutexattr_t *__attr,
       int __protocol)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *
          __restrict __attr,
          int *__restrict __prioceiling)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *__attr,
          int __prioceiling)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_getrobust (const pthread_mutexattr_t *__attr,
     int *__robustness)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));







extern int pthread_mutexattr_setrobust (pthread_mutexattr_t *__attr,
     int __robustness)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 899 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
    const pthread_rwlockattr_t *__restrict
    __attr) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock)
  __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
           const struct timespec *__restrict
           __abstime) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
           const struct timespec *__restrict
           __abstime) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr,
       int __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pref)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr,
       int __pref) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));







extern int pthread_cond_init (pthread_cond_t *__restrict __cond,
         const pthread_condattr_t *__restrict __cond_attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_destroy (pthread_cond_t *__cond)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_signal (pthread_cond_t *__cond)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_broadcast (pthread_cond_t *__cond)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int pthread_cond_wait (pthread_cond_t *__restrict __cond,
         pthread_mutex_t *__restrict __mutex)
     __attribute__ ((__nonnull__ (1, 2)));
# 1011 "/usr/include/pthread.h" 3 4
extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       const struct timespec *__restrict __abstime)
     __attribute__ ((__nonnull__ (1, 2, 3)));




extern int pthread_condattr_init (pthread_condattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_destroy (pthread_condattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_getpshared (const pthread_condattr_t *
     __restrict __attr,
     int *__restrict __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setpshared (pthread_condattr_t *__attr,
     int __pshared) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));



extern int pthread_condattr_getclock (const pthread_condattr_t *
          __restrict __attr,
          __clockid_t *__restrict __clock_id)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setclock (pthread_condattr_t *__attr,
          __clockid_t __clock_id)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 1055 "/usr/include/pthread.h" 3 4
extern int pthread_spin_init (pthread_spinlock_t *__lock, int __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_destroy (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_lock (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_trylock (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_unlock (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int pthread_barrier_init (pthread_barrier_t *__restrict __barrier,
     const pthread_barrierattr_t *__restrict
     __attr, unsigned int __count)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_destroy (pthread_barrier_t *__barrier)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_wait (pthread_barrier_t *__barrier)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_barrierattr_init (pthread_barrierattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_destroy (pthread_barrierattr_t *__attr)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_getpshared (const pthread_barrierattr_t *
        __restrict __attr,
        int *__restrict __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_barrierattr_setpshared (pthread_barrierattr_t *__attr,
        int __pshared)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
# 1122 "/usr/include/pthread.h" 3 4
extern int pthread_key_create (pthread_key_t *__key,
          void (*__destr_function) (void *))
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));


extern int pthread_key_delete (pthread_key_t __key) __attribute__ ((__nothrow__ ));


extern void *pthread_getspecific (pthread_key_t __key) __attribute__ ((__nothrow__ ));


extern int pthread_setspecific (pthread_key_t __key,
    const void *__pointer) __attribute__ ((__nothrow__ )) ;




extern int pthread_getcpuclockid (pthread_t __thread_id,
      __clockid_t *__clock_id)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));
# 1156 "/usr/include/pthread.h" 3 4
extern int pthread_atfork (void (*__prepare) (void),
      void (*__parent) (void),
      void (*__child) (void)) __attribute__ ((__nothrow__ ));
# 48 "../include/apr_portable.h" 2
# 127 "../include/apr_portable.h"
struct apr_os_proc_mutex_t {


    int crossproc;



    pthread_mutex_t *pthread_interproc;





    pthread_mutex_t *intraproc;


};

typedef int apr_os_file_t;
typedef DIR apr_os_dir_t;
typedef int apr_os_sock_t;
typedef struct apr_os_proc_mutex_t apr_os_proc_mutex_t;



typedef pthread_t apr_os_thread_t;
typedef pthread_key_t apr_os_threadkey_t;


typedef pid_t apr_os_proc_t;
typedef struct timeval apr_os_imp_time_t;
typedef struct tm apr_os_exp_time_t;
# 169 "../include/apr_portable.h"
typedef void * apr_os_dso_handle_t;

typedef void* apr_os_shm_t;
# 183 "../include/apr_portable.h"
struct apr_os_sock_info_t {
    apr_os_sock_t *os_sock;
    struct sockaddr *local;
    struct sockaddr *remote;
    int family;
    int type;
    int protocol;
};

typedef struct apr_os_sock_info_t apr_os_sock_info_t;
# 203 "../include/apr_portable.h"
    struct apr_os_global_mutex_t {
        apr_pool_t *pool;
        apr_proc_mutex_t *proc_mutex;

        apr_thread_mutex_t *thread_mutex;

    };
    typedef struct apr_os_global_mutex_t apr_os_global_mutex_t;

apr_status_t apr_os_global_mutex_get(apr_os_global_mutex_t *ospmutex,
                                                apr_global_mutex_t *pmutex);
# 224 "../include/apr_portable.h"
apr_status_t apr_os_file_get(apr_os_file_t *thefile,
                                          apr_file_t *file);






apr_status_t apr_os_dir_get(apr_os_dir_t **thedir,
                                         apr_dir_t *dir);






apr_status_t apr_os_sock_get(apr_os_sock_t *thesock,
                                          apr_socket_t *sock);






apr_status_t apr_os_proc_mutex_get(apr_os_proc_mutex_t *ospmutex,
                                                apr_proc_mutex_t *pmutex);






apr_status_t apr_os_exp_time_get(apr_os_exp_time_t **ostime,
                                 apr_time_exp_t *aprtime);






apr_status_t apr_os_imp_time_get(apr_os_imp_time_t **ostime,
                                              apr_time_t *aprtime);






apr_status_t apr_os_shm_get(apr_os_shm_t *osshm,
                                         apr_shm_t *shm);
# 285 "../include/apr_portable.h"
apr_status_t apr_os_thread_get(apr_os_thread_t **thethd,
                                            apr_thread_t *thd);






apr_status_t apr_os_threadkey_get(apr_os_threadkey_t *thekey,
                                               apr_threadkey_t *key);







apr_status_t apr_os_thread_put(apr_thread_t **thd,
                                            apr_os_thread_t *thethd,
                                            apr_pool_t *cont);







apr_status_t apr_os_threadkey_put(apr_threadkey_t **key,
                                               apr_os_threadkey_t *thekey,
                                               apr_pool_t *cont);



apr_os_thread_t apr_os_thread_current(void);







int apr_os_thread_equal(apr_os_thread_t tid1,
                                     apr_os_thread_t tid2);
# 341 "../include/apr_portable.h"
apr_status_t apr_os_file_put(apr_file_t **file,
                                          apr_os_file_t *thefile,
                                          apr_int32_t flags, apr_pool_t *cont);
# 353 "../include/apr_portable.h"
apr_status_t apr_os_pipe_put(apr_file_t **file,
                                          apr_os_file_t *thefile,
                                          apr_pool_t *cont);
# 367 "../include/apr_portable.h"
apr_status_t apr_os_pipe_put_ex(apr_file_t **file,
                                             apr_os_file_t *thefile,
                                             int register_cleanup,
                                             apr_pool_t *cont);







apr_status_t apr_os_dir_put(apr_dir_t **dir,
                                         apr_os_dir_t *thedir,
                                         apr_pool_t *cont);
# 390 "../include/apr_portable.h"
apr_status_t apr_os_sock_put(apr_socket_t **sock,
                                          apr_os_sock_t *thesock,
                                          apr_pool_t *cont);
# 404 "../include/apr_portable.h"
apr_status_t apr_os_sock_make(apr_socket_t **apr_sock,
                                           apr_os_sock_info_t *os_sock_info,
                                           apr_pool_t *cont);







apr_status_t apr_os_proc_mutex_put(apr_proc_mutex_t **pmutex,
                                                apr_os_proc_mutex_t *ospmutex,
                                                apr_pool_t *cont);







apr_status_t apr_os_imp_time_put(apr_time_t *aprtime,
                                              apr_os_imp_time_t **ostime,
                                              apr_pool_t *cont);







apr_status_t apr_os_exp_time_put(apr_time_exp_t *aprtime,
                                              apr_os_exp_time_t **ostime,
                                              apr_pool_t *cont);
# 447 "../include/apr_portable.h"
apr_status_t apr_os_shm_put(apr_shm_t **shm,
                                         apr_os_shm_t *osshm,
                                         apr_pool_t *cont);
# 463 "../include/apr_portable.h"
apr_status_t apr_os_dso_handle_put(apr_dso_handle_t **dso,
                                                apr_os_dso_handle_t thedso,
                                                apr_pool_t *pool);






apr_status_t apr_os_dso_handle_get(apr_os_dso_handle_t *dso,
                                                apr_dso_handle_t *aprdso);
# 491 "../include/apr_portable.h"
const char* apr_os_default_encoding(apr_pool_t *pool);
# 500 "../include/apr_portable.h"
const char* apr_os_locale_encoding(apr_pool_t *pool);
# 25 "../include/arch/unix/apr_arch_thread_mutex.h" 2







struct apr_thread_mutex_t {
    apr_pool_t *pool;
    pthread_mutex_t mutex;
};
# 18 "./../locks/unix/thread_mutex.c" 2

# 1 "../include/apr_want.h" 1
# 20 "./../locks/unix/thread_mutex.c" 2



static apr_status_t thread_mutex_cleanup(void *data)
{
    apr_thread_mutex_t *mutex = data;
    apr_status_t rv;

    rv = pthread_mutex_destroy(&mutex->mutex);





    return rv;
}

apr_status_t apr_thread_mutex_create(apr_thread_mutex_t **mutex,
                                                  unsigned int flags,
                                                  apr_pool_t *pool)
{
    apr_thread_mutex_t *new_mutex;
    apr_status_t rv;







    new_mutex = memset(apr_palloc(pool, sizeof(apr_thread_mutex_t)), 0, sizeof(apr_thread_mutex_t));
    new_mutex->pool = pool;


    if (flags & 0x1) {
        pthread_mutexattr_t mattr;

        rv = pthread_mutexattr_init(&mattr);
        if (rv) return rv;

        rv = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
        if (rv) {
            pthread_mutexattr_destroy(&mattr);
            return rv;
        }

        rv = pthread_mutex_init(&new_mutex->mutex, &mattr);

        pthread_mutexattr_destroy(&mattr);
    } else

        rv = pthread_mutex_init(&new_mutex->mutex, ((void*)0));

    if (rv) {



        return rv;
    }

    apr_pool_cleanup_register(new_mutex->pool,
                              new_mutex, thread_mutex_cleanup,
                              apr_pool_cleanup_null);

    *mutex = new_mutex;
    return 0;
}

apr_status_t apr_thread_mutex_lock(apr_thread_mutex_t *mutex)
{
    apr_status_t rv;

    rv = pthread_mutex_lock(&mutex->mutex);






    return rv;
}

apr_status_t apr_thread_mutex_trylock(apr_thread_mutex_t *mutex)
{
    apr_status_t rv;

    rv = pthread_mutex_trylock(&mutex->mutex);
    if (rv) {



        return (rv == 16) ? ((20000 + 50000) + 25) : rv;
    }

    return 0;
}

apr_status_t apr_thread_mutex_unlock(apr_thread_mutex_t *mutex)
{
    apr_status_t status;

    status = pthread_mutex_unlock(&mutex->mutex);






    return status;
}

apr_status_t apr_thread_mutex_destroy(apr_thread_mutex_t *mutex)
{
    return apr_pool_cleanup_run(mutex->pool, mutex, thread_mutex_cleanup);
}

apr_pool_t * apr_thread_mutex_pool_get (const apr_thread_mutex_t *thethread_mutex) { return thethread_mutex->pool; }
# 12 "main2.c" 2
# 1 "./../locks/unix/proc_mutex.c" 1
# 19 "./../locks/unix/proc_mutex.c"
# 1 "../include/arch/unix/apr_arch_proc_mutex.h" 1
# 23 "../include/arch/unix/apr_arch_proc_mutex.h"
# 1 "../include/apr_lib.h" 1
# 32 "../include/apr_lib.h"
# 1 "/usr/include/ctype.h" 1 3 4
# 46 "/usr/include/ctype.h" 3 4
enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
# 79 "/usr/include/ctype.h" 3 4
extern const unsigned short int **__ctype_b_loc (void)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
extern const __int32_t **__ctype_tolower_loc (void)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
extern const __int32_t **__ctype_toupper_loc (void)
     __attribute__ ((__nothrow__ )) __attribute__ ((__const__));
# 110 "/usr/include/ctype.h" 3 4
extern int isalnum (int) __attribute__ ((__nothrow__ ));
extern int isalpha (int) __attribute__ ((__nothrow__ ));
extern int iscntrl (int) __attribute__ ((__nothrow__ ));
extern int isdigit (int) __attribute__ ((__nothrow__ ));
extern int islower (int) __attribute__ ((__nothrow__ ));
extern int isgraph (int) __attribute__ ((__nothrow__ ));
extern int isprint (int) __attribute__ ((__nothrow__ ));
extern int ispunct (int) __attribute__ ((__nothrow__ ));
extern int isspace (int) __attribute__ ((__nothrow__ ));
extern int isupper (int) __attribute__ ((__nothrow__ ));
extern int isxdigit (int) __attribute__ ((__nothrow__ ));



extern int tolower (int __c) __attribute__ ((__nothrow__ ));


extern int toupper (int __c) __attribute__ ((__nothrow__ ));
# 136 "/usr/include/ctype.h" 3 4
extern int isblank (int) __attribute__ ((__nothrow__ ));
# 150 "/usr/include/ctype.h" 3 4
extern int isascii (int __c) __attribute__ ((__nothrow__ ));



extern int toascii (int __c) __attribute__ ((__nothrow__ ));



extern int _toupper (int) __attribute__ ((__nothrow__ ));
extern int _tolower (int) __attribute__ ((__nothrow__ ));
# 271 "/usr/include/ctype.h" 3 4
extern int isalnum_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isalpha_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int iscntrl_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isdigit_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int islower_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isgraph_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isprint_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int ispunct_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isspace_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isupper_l (int, __locale_t) __attribute__ ((__nothrow__ ));
extern int isxdigit_l (int, __locale_t) __attribute__ ((__nothrow__ ));

extern int isblank_l (int, __locale_t) __attribute__ ((__nothrow__ ));



extern int __tolower_l (int __c, __locale_t __l) __attribute__ ((__nothrow__ ));
extern int tolower_l (int __c, __locale_t __l) __attribute__ ((__nothrow__ ));


extern int __toupper_l (int __c, __locale_t __l) __attribute__ ((__nothrow__ ));
extern int toupper_l (int __c, __locale_t __l) __attribute__ ((__nothrow__ ));
# 33 "../include/apr_lib.h" 2
# 59 "../include/apr_lib.h"
typedef struct apr_vformatter_buff_t apr_vformatter_buff_t;




struct apr_vformatter_buff_t {

    char *curpos;

    char *endpos;
};
# 84 "../include/apr_lib.h"
const char * apr_filepath_name_get(const char *pathname);
# 174 "../include/apr_lib.h"
int apr_vformatter(int (*flush_func)(apr_vformatter_buff_t *b),
           apr_vformatter_buff_t *c, const char *fmt,
           va_list ap);
# 189 "../include/apr_lib.h"
apr_status_t apr_password_get(const char *prompt, char *pwbuf,
                                           apr_size_t *bufsize);
# 24 "../include/arch/unix/apr_arch_proc_mutex.h" 2




# 1 "../include/arch/unix/apr_arch_file_io.h" 1
# 41 "../include/arch/unix/apr_arch_file_io.h"
# 1 "/usr/include/errno.h" 1 3 4
# 42 "../include/arch/unix/apr_arch_file_io.h" 2





# 1 "/usr/include/strings.h" 1 3 4
# 48 "../include/arch/unix/apr_arch_file_io.h" 2





# 1 "/usr/include/x86_64-linux-gnu/sys/stat.h" 1 3 4
# 105 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/stat.h" 1 3 4
# 106 "/usr/include/x86_64-linux-gnu/sys/stat.h" 2 3 4
# 209 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int stat (const char *__restrict __file,
   struct stat *__restrict __buf) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));



extern int fstat (int __fd, struct stat *__buf) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));
# 238 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int fstatat (int __fd, const char *__restrict __file,
      struct stat *__restrict __buf, int __flag)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3)));
# 263 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int lstat (const char *__restrict __file,
    struct stat *__restrict __buf) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1, 2)));
# 284 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int chmod (const char *__file, __mode_t __mode)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int lchmod (const char *__file, __mode_t __mode)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));




extern int fchmod (int __fd, __mode_t __mode) __attribute__ ((__nothrow__ ));





extern int fchmodat (int __fd, const char *__file, __mode_t __mode,
       int __flag)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2))) ;






extern __mode_t umask (__mode_t __mask) __attribute__ ((__nothrow__ ));
# 321 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int mkdir (const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int mkdirat (int __fd, const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));






extern int mknod (const char *__path, __mode_t __mode, __dev_t __dev)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int mknodat (int __fd, const char *__path, __mode_t __mode,
      __dev_t __dev) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));





extern int mkfifo (const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));





extern int mkfifoat (int __fd, const char *__path, __mode_t __mode)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));





extern int utimensat (int __fd, const char *__path,
        const struct timespec __times[2],
        int __flags)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2)));




extern int futimens (int __fd, const struct timespec __times[2]) __attribute__ ((__nothrow__ ));
# 399 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int __fxstat (int __ver, int __fildes, struct stat *__stat_buf)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3)));
extern int __xstat (int __ver, const char *__filename,
      struct stat *__stat_buf) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3)));
extern int __lxstat (int __ver, const char *__filename,
       struct stat *__stat_buf) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 3)));
extern int __fxstatat (int __ver, int __fildes, const char *__filename,
         struct stat *__stat_buf, int __flag)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 4)));
# 442 "/usr/include/x86_64-linux-gnu/sys/stat.h" 3 4
extern int __xmknod (int __ver, const char *__path, __mode_t __mode,
       __dev_t *__dev) __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (2, 4)));

extern int __xmknodat (int __ver, int __fd, const char *__path,
         __mode_t __mode, __dev_t *__dev)
     __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (3, 5)));
# 54 "../include/arch/unix/apr_arch_file_io.h" 2
# 75 "../include/arch/unix/apr_arch_file_io.h"
# 1 "/usr/include/x86_64-linux-gnu/sys/param.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/sys/param.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/sys/param.h" 2 3 4







# 1 "/usr/include/x86_64-linux-gnu/bits/param.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/param.h" 3 4
# 1 "/usr/include/linux/param.h" 1 3 4



# 1 "/usr/include/x86_64-linux-gnu/asm/param.h" 1 3 4
# 1 "/usr/include/asm-generic/param.h" 1 3 4
# 2 "/usr/include/x86_64-linux-gnu/asm/param.h" 2 3 4
# 5 "/usr/include/linux/param.h" 2 3 4
# 29 "/usr/include/x86_64-linux-gnu/bits/param.h" 2 3 4
# 32 "/usr/include/x86_64-linux-gnu/sys/param.h" 2 3 4
# 76 "../include/arch/unix/apr_arch_file_io.h" 2
# 93 "../include/arch/unix/apr_arch_file_io.h"
struct apr_file_t {
    apr_pool_t *pool;
    int filedes;
    char *fname;
    apr_int32_t flags;
    int eof_hit;
    int is_pipe;
    apr_interval_time_t timeout;
    int buffered;
    enum {BLK_UNKNOWN, BLK_OFF, BLK_ON } blocking;
    int ungetchar;





    char *buffer;
    apr_size_t bufpos;
    apr_size_t bufsize;
    unsigned long dataRead;
    int direction;
    apr_off_t filePtr;

    struct apr_thread_mutex_t *thlock;

};
# 142 "../include/arch/unix/apr_arch_file_io.h"
typedef struct stat struct_stat;
# 151 "../include/arch/unix/apr_arch_file_io.h"
struct apr_dir_t {
    apr_pool_t *pool;
    char *dirname;
    DIR *dirstruct;



    struct dirent *entry;

};

apr_status_t apr_unix_file_cleanup(void *);
apr_status_t apr_unix_child_file_cleanup(void *);

mode_t apr_unix_perms2mode(apr_fileperms_t perms);
apr_fileperms_t apr_unix_mode2perms(mode_t mode);

apr_status_t apr_file_flush_locked(apr_file_t *thefile);
apr_status_t apr_file_info_get_locked(apr_finfo_t *finfo, apr_int32_t wanted,
                                      apr_file_t *thefile);
# 29 "../include/arch/unix/apr_arch_proc_mutex.h" 2
# 42 "../include/arch/unix/apr_arch_proc_mutex.h"
# 1 "/usr/include/x86_64-linux-gnu/sys/ipc.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/sys/ipc.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/ipctypes.h" 1 3 4
# 28 "/usr/include/x86_64-linux-gnu/bits/ipctypes.h" 3 4
typedef int __ipc_pid_t;
# 29 "/usr/include/x86_64-linux-gnu/sys/ipc.h" 2 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/ipc.h" 1 3 4
# 42 "/usr/include/x86_64-linux-gnu/bits/ipc.h" 3 4
struct ipc_perm
  {
    __key_t __key;
    __uid_t uid;
    __gid_t gid;
    __uid_t cuid;
    __gid_t cgid;
    unsigned short int mode;
    unsigned short int __pad1;
    unsigned short int __seq;
    unsigned short int __pad2;
    __syscall_ulong_t __glibc_reserved1;
    __syscall_ulong_t __glibc_reserved2;
  };
# 30 "/usr/include/x86_64-linux-gnu/sys/ipc.h" 2 3 4
# 54 "/usr/include/x86_64-linux-gnu/sys/ipc.h" 3 4
extern key_t ftok (const char *__pathname, int __proj_id) __attribute__ ((__nothrow__ ));
# 43 "../include/arch/unix/apr_arch_proc_mutex.h" 2


# 1 "/usr/include/x86_64-linux-gnu/sys/sem.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/sys/sem.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/sys/sem.h" 2 3 4





# 1 "/usr/include/x86_64-linux-gnu/bits/sem.h" 1 3 4
# 38 "/usr/include/x86_64-linux-gnu/bits/sem.h" 3 4
struct semid_ds
{
  struct ipc_perm sem_perm;
  __time_t sem_otime;
  __syscall_ulong_t __glibc_reserved1;
  __time_t sem_ctime;
  __syscall_ulong_t __glibc_reserved2;
  __syscall_ulong_t sem_nsems;
  __syscall_ulong_t __glibc_reserved3;
  __syscall_ulong_t __glibc_reserved4;
};
# 72 "/usr/include/x86_64-linux-gnu/bits/sem.h" 3 4
struct seminfo
{
  int semmap;
  int semmni;
  int semmns;
  int semmnu;
  int semmsl;
  int semopm;
  int semume;
  int semusz;
  int semvmx;
  int semaem;
};
# 31 "/usr/include/x86_64-linux-gnu/sys/sem.h" 2 3 4
# 41 "/usr/include/x86_64-linux-gnu/sys/sem.h" 3 4
struct sembuf
{
  unsigned short int sem_num;
  short int sem_op;
  short int sem_flg;
};





extern int semctl (int __semid, int __semnum, int __cmd, ...) __attribute__ ((__nothrow__ ));


extern int semget (key_t __key, int __nsems, int __semflg) __attribute__ ((__nothrow__ ));


extern int semop (int __semid, struct sembuf *__sops, size_t __nsops) __attribute__ ((__nothrow__ ));
# 46 "../include/arch/unix/apr_arch_proc_mutex.h" 2


# 1 "/usr/include/x86_64-linux-gnu/sys/file.h" 1 3 4
# 50 "/usr/include/x86_64-linux-gnu/sys/file.h" 3 4
extern int flock (int __fd, int __operation) __attribute__ ((__nothrow__ ));
# 49 "../include/arch/unix/apr_arch_proc_mutex.h" 2
# 60 "../include/arch/unix/apr_arch_proc_mutex.h"
# 1 "/usr/include/x86_64-linux-gnu/sys/mman.h" 1 3 4
# 25 "/usr/include/x86_64-linux-gnu/sys/mman.h" 3 4
# 1 "/usr/lib/llvm-3.5/bin/../lib/clang/3.5.0/include/stddef.h" 1 3 4
# 26 "/usr/include/x86_64-linux-gnu/sys/mman.h" 2 3 4
# 41 "/usr/include/x86_64-linux-gnu/sys/mman.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/mman.h" 1 3 4
# 45 "/usr/include/x86_64-linux-gnu/bits/mman.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/mman-linux.h" 1 3 4
# 46 "/usr/include/x86_64-linux-gnu/bits/mman.h" 2 3 4
# 42 "/usr/include/x86_64-linux-gnu/sys/mman.h" 2 3 4
# 57 "/usr/include/x86_64-linux-gnu/sys/mman.h" 3 4
extern void *mmap (void *__addr, size_t __len, int __prot,
     int __flags, int __fd, __off_t __offset) __attribute__ ((__nothrow__ ));
# 76 "/usr/include/x86_64-linux-gnu/sys/mman.h" 3 4
extern int munmap (void *__addr, size_t __len) __attribute__ ((__nothrow__ ));




extern int mprotect (void *__addr, size_t __len, int __prot) __attribute__ ((__nothrow__ ));







extern int msync (void *__addr, size_t __len, int __flags);




extern int madvise (void *__addr, size_t __len, int __advice) __attribute__ ((__nothrow__ ));



extern int posix_madvise (void *__addr, size_t __len, int __advice) __attribute__ ((__nothrow__ ));




extern int mlock (const void *__addr, size_t __len) __attribute__ ((__nothrow__ ));


extern int munlock (const void *__addr, size_t __len) __attribute__ ((__nothrow__ ));




extern int mlockall (int __flags) __attribute__ ((__nothrow__ ));



extern int munlockall (void) __attribute__ ((__nothrow__ ));







extern int mincore (void *__start, size_t __len, unsigned char *__vec)
     __attribute__ ((__nothrow__ ));
# 144 "/usr/include/x86_64-linux-gnu/sys/mman.h" 3 4
extern int shm_open (const char *__name, int __oflag, mode_t __mode);


extern int shm_unlink (const char *__name);
# 61 "../include/arch/unix/apr_arch_proc_mutex.h" 2





# 1 "/usr/include/semaphore.h" 1 3 4
# 29 "/usr/include/semaphore.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 1 3 4
# 23 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 3 4
# 1 "/usr/include/x86_64-linux-gnu/bits/wordsize.h" 1 3 4
# 24 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 2 3 4
# 36 "/usr/include/x86_64-linux-gnu/bits/semaphore.h" 3 4
typedef union
{
  char __size[32];
  long int __align;
} sem_t;
# 30 "/usr/include/semaphore.h" 2 3 4






extern int sem_init (sem_t *__sem, int __pshared, unsigned int __value)
     __attribute__ ((__nothrow__ ));

extern int sem_destroy (sem_t *__sem) __attribute__ ((__nothrow__ ));


extern sem_t *sem_open (const char *__name, int __oflag, ...) __attribute__ ((__nothrow__ ));


extern int sem_close (sem_t *__sem) __attribute__ ((__nothrow__ ));


extern int sem_unlink (const char *__name) __attribute__ ((__nothrow__ ));





extern int sem_wait (sem_t *__sem);






extern int sem_timedwait (sem_t *__restrict __sem,
     const struct timespec *__restrict __abstime);



extern int sem_trywait (sem_t *__sem) __attribute__ ((__nothrow__));


extern int sem_post (sem_t *__sem) __attribute__ ((__nothrow__));


extern int sem_getvalue (sem_t *__restrict __sem, int *__restrict __sval)
     __attribute__ ((__nothrow__ ));
# 67 "../include/arch/unix/apr_arch_proc_mutex.h" 2



struct apr_proc_mutex_unix_lock_methods_t {
    unsigned int flags;
    apr_status_t (*create)(apr_proc_mutex_t *, const char *);
    apr_status_t (*acquire)(apr_proc_mutex_t *);
    apr_status_t (*tryacquire)(apr_proc_mutex_t *);
    apr_status_t (*release)(apr_proc_mutex_t *);
    apr_status_t (*cleanup)(void *);
    apr_status_t (*child_init)(apr_proc_mutex_t **, apr_pool_t *, const char *);
    const char *name;
};
typedef struct apr_proc_mutex_unix_lock_methods_t apr_proc_mutex_unix_lock_methods_t;





union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


struct apr_proc_mutex_t {
    apr_pool_t *pool;
    const apr_proc_mutex_unix_lock_methods_t *meth;
    const apr_proc_mutex_unix_lock_methods_t *inter_meth;
    int curr_locked;
    char *fname;

    apr_file_t *interproc;


    sem_t *psem_interproc;


    pthread_mutex_t *pthread_interproc;

};

void apr_proc_mutex_unix_setup_lock(void);
# 20 "./../locks/unix/proc_mutex.c" 2

# 1 "../include/apr_hash.h" 1
# 52 "../include/apr_hash.h"
typedef struct apr_hash_t apr_hash_t;




typedef struct apr_hash_index_t apr_hash_index_t;







typedef unsigned int (*apr_hashfunc_t)(const char *key, apr_ssize_t *klen);




unsigned int apr_hashfunc_default(const char *key,
                                                      apr_ssize_t *klen);






apr_hash_t * apr_hash_make(apr_pool_t *pool);







apr_hash_t * apr_hash_make_custom(apr_pool_t *pool,
                                               apr_hashfunc_t hash_func);
# 96 "../include/apr_hash.h"
apr_hash_t * apr_hash_copy(apr_pool_t *pool,
                                        const apr_hash_t *h);
# 107 "../include/apr_hash.h"
void apr_hash_set(apr_hash_t *ht, const void *key,
                               apr_ssize_t klen, const void *val);
# 117 "../include/apr_hash.h"
void * apr_hash_get(apr_hash_t *ht, const void *key,
                                 apr_ssize_t klen);
# 147 "../include/apr_hash.h"
apr_hash_index_t * apr_hash_first(apr_pool_t *p, apr_hash_t *ht);







apr_hash_index_t * apr_hash_next(apr_hash_index_t *hi);
# 166 "../include/apr_hash.h"
void apr_hash_this(apr_hash_index_t *hi, const void **key,
                                apr_ssize_t *klen, void **val);






const void* apr_hash_this_key(apr_hash_index_t *hi);






apr_ssize_t apr_hash_this_key_len(apr_hash_index_t *hi);






void* apr_hash_this_val(apr_hash_index_t *hi);






unsigned int apr_hash_count(apr_hash_t *ht);





void apr_hash_clear(apr_hash_t *ht);
# 212 "../include/apr_hash.h"
apr_hash_t * apr_hash_overlay(apr_pool_t *p,
                                           const apr_hash_t *overlay,
                                           const apr_hash_t *base);
# 230 "../include/apr_hash.h"
apr_hash_t * apr_hash_merge(apr_pool_t *p,
                                         const apr_hash_t *h1,
                                         const apr_hash_t *h2,
                                         void * (*merger)(apr_pool_t *p,
                                                     const void *key,
                                                     apr_ssize_t klen,
                                                     const void *h1_val,
                                                     const void *h2_val,
                                                     const void *data),
                                         const void *data);
# 252 "../include/apr_hash.h"
typedef int (apr_hash_do_callback_fn_t)(void *rec, const void *key,
                                                   apr_ssize_t klen,
                                                   const void *value);
# 268 "../include/apr_hash.h"
int apr_hash_do(apr_hash_do_callback_fn_t *comp,
                             void *rec, const apr_hash_t *ht);




apr_pool_t * apr_hash_pool_get (const apr_hash_t *thehash);
# 22 "./../locks/unix/proc_mutex.c" 2

apr_status_t apr_proc_mutex_destroy(apr_proc_mutex_t *mutex)
{
    return apr_pool_cleanup_run(mutex->pool, mutex, apr_proc_mutex_cleanup);
}



static apr_status_t proc_mutex_no_child_init(apr_proc_mutex_t **mutex,
                                             apr_pool_t *cont,
                                             const char *fname)
{
    return 0;
}
# 44 "./../locks/unix/proc_mutex.c"
static apr_status_t proc_mutex_posix_cleanup(void *mutex_)
{
    apr_proc_mutex_t *mutex = mutex_;

    if (sem_close(mutex->psem_interproc) < 0) {
        return (*__errno_location ());
    }

    return 0;
}

static unsigned int rshash (char *p) {

   unsigned int b = 378551;
   unsigned int a = 63689;
   unsigned int retval = 0;

   for( ; *p; p++)
   {
      retval = retval * a + (*p);
      a *= b;
   }

   return retval;
}

static apr_status_t proc_mutex_posix_create(apr_proc_mutex_t *new_mutex,
                                            const char *fname)
{

    sem_t *psem;
    char semname[32];

    new_mutex->interproc = apr_palloc(new_mutex->pool,
                                      sizeof(*new_mutex->interproc));
# 101 "./../locks/unix/proc_mutex.c"
    if (fname) {
        apr_ssize_t flen = strlen(fname);
        char *p = apr_pstrndup(new_mutex->pool, fname, strlen(fname));
        unsigned int h1, h2;
        h1 = (apr_hashfunc_default((const char *)p, &flen) & 0xffffffff);
        h2 = (rshash(p) & 0xffffffff);
        apr_snprintf(semname, sizeof(semname), "/ApR.%xH%x", h1, h2);
    } else {
        apr_time_t now;
        unsigned long sec;
        unsigned long usec;
        now = apr_time_now();
        sec = ((now) / 1000000L);
        usec = ((now) % 1000000L);
        apr_snprintf(semname, sizeof(semname), "/ApR.%lxZ%lx", sec, usec);
    }
    psem = sem_open(semname, 0100 | 0200, 0644, 1);
    if (psem == (sem_t *)((sem_t *) 0)) {
        if ((*__errno_location ()) == 36) {

            semname[13] = '\0';
        } else {
            return (*__errno_location ());
        }
        psem = sem_open(semname, 0100 | 0200, 0644, 1);
    }

    if (psem == (sem_t *)((sem_t *) 0)) {
        return (*__errno_location ());
    }

    sem_unlink(semname);
    new_mutex->psem_interproc = psem;
    new_mutex->fname = apr_pstrdup(new_mutex->pool, semname);
    apr_pool_cleanup_register(new_mutex->pool, (void *)new_mutex,
                              apr_proc_mutex_cleanup,
                              apr_pool_cleanup_null);
    return 0;
}

static apr_status_t proc_mutex_posix_acquire(apr_proc_mutex_t *mutex)
{
    if (sem_wait(mutex->psem_interproc) < 0) {
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_posix_tryacquire(apr_proc_mutex_t *mutex)
{
    if (sem_trywait(mutex->psem_interproc) < 0) {
        if ((*__errno_location ()) == 11) {
            return ((20000 + 50000) + 25);
        }
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_posix_release(apr_proc_mutex_t *mutex)
{
    mutex->curr_locked = 0;
    if (sem_post(mutex->psem_interproc) < 0) {


        return (*__errno_location ());
    }
    return 0;
}

static const apr_proc_mutex_unix_lock_methods_t mutex_posixsem_methods =
{



    0,

    proc_mutex_posix_create,
    proc_mutex_posix_acquire,
    proc_mutex_posix_tryacquire,
    proc_mutex_posix_release,
    proc_mutex_posix_cleanup,
    proc_mutex_no_child_init,
    "posixsem"
};





static struct sembuf proc_mutex_op_on;
static struct sembuf proc_mutex_op_try;
static struct sembuf proc_mutex_op_off;

static void proc_mutex_sysv_setup(void)
{
    proc_mutex_op_on.sem_num = 0;
    proc_mutex_op_on.sem_op = -1;
    proc_mutex_op_on.sem_flg = 0x1000;
    proc_mutex_op_try.sem_num = 0;
    proc_mutex_op_try.sem_op = -1;
    proc_mutex_op_try.sem_flg = 0x1000 | 04000;
    proc_mutex_op_off.sem_num = 0;
    proc_mutex_op_off.sem_op = 1;
    proc_mutex_op_off.sem_flg = 0x1000;
}

static apr_status_t proc_mutex_sysv_cleanup(void *mutex_)
{
    apr_proc_mutex_t *mutex=mutex_;
    union semun ick;

    if (mutex->interproc->filedes != -1) {
        ick.val = 0;
        semctl(mutex->interproc->filedes, 0, 0, ick);
    }
    return 0;
}

static apr_status_t proc_mutex_sysv_create(apr_proc_mutex_t *new_mutex,
                                           const char *fname)
{
    union semun ick;
    apr_status_t rv;

    new_mutex->interproc = apr_palloc(new_mutex->pool, sizeof(*new_mutex->interproc));
    new_mutex->interproc->filedes = semget(((__key_t) 0), 1, 01000 | 0600);

    if (new_mutex->interproc->filedes < 0) {
        rv = (*__errno_location ());
        proc_mutex_sysv_cleanup(new_mutex);
        return rv;
    }
    ick.val = 1;
    if (semctl(new_mutex->interproc->filedes, 0, 16, ick) < 0) {
        rv = (*__errno_location ());
        proc_mutex_sysv_cleanup(new_mutex);
        return rv;
    }
    new_mutex->curr_locked = 0;
    apr_pool_cleanup_register(new_mutex->pool,
                              (void *)new_mutex, apr_proc_mutex_cleanup,
                              apr_pool_cleanup_null);
    return 0;
}

static apr_status_t proc_mutex_sysv_acquire(apr_proc_mutex_t *mutex)
{
    int rc;

    do {
        rc = semop(mutex->interproc->filedes, &proc_mutex_op_on, 1);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_sysv_tryacquire(apr_proc_mutex_t *mutex)
{
    int rc;

    do {
        rc = semop(mutex->interproc->filedes, &proc_mutex_op_try, 1);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        if ((*__errno_location ()) == 11) {
            return ((20000 + 50000) + 25);
        }
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_sysv_release(apr_proc_mutex_t *mutex)
{
    int rc;

    mutex->curr_locked = 0;
    do {
        rc = semop(mutex->interproc->filedes, &proc_mutex_op_off, 1);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        return (*__errno_location ());
    }
    return 0;
}

static const apr_proc_mutex_unix_lock_methods_t mutex_sysv_methods =
{



    0,

    proc_mutex_sysv_create,
    proc_mutex_sysv_acquire,
    proc_mutex_sysv_tryacquire,
    proc_mutex_sysv_release,
    proc_mutex_sysv_cleanup,
    proc_mutex_no_child_init,
    "sysvsem"
};





static apr_status_t proc_mutex_proc_pthread_cleanup(void *mutex_)
{
    apr_proc_mutex_t *mutex=mutex_;
    apr_status_t rv;

    if (mutex->curr_locked == 1) {
        if ((rv = pthread_mutex_unlock(mutex->pthread_interproc))) {



            return rv;
        }
    }

    if (mutex->curr_locked != -1) {
        if ((rv = pthread_mutex_destroy(mutex->pthread_interproc))) {



            return rv;
        }
    }
    if (munmap((caddr_t)mutex->pthread_interproc, sizeof(pthread_mutex_t))) {
        return (*__errno_location ());
    }
    return 0;
}

static apr_status_t proc_mutex_proc_pthread_create(apr_proc_mutex_t *new_mutex,
                                                   const char *fname)
{
    apr_status_t rv;
    int fd;
    pthread_mutexattr_t mattr;

    fd = open("/dev/zero", 02);
    if (fd < 0) {
        return (*__errno_location ());
    }

    new_mutex->pthread_interproc = (pthread_mutex_t *)mmap(
                                       (caddr_t) 0,
                                       sizeof(pthread_mutex_t),
                                       0x1 | 0x2, 0x01,
                                       fd, 0);
    if (new_mutex->pthread_interproc == (pthread_mutex_t *) (caddr_t) -1) {
        close(fd);
        return (*__errno_location ());
    }
    close(fd);

    new_mutex->curr_locked = -1;

    if ((rv = pthread_mutexattr_init(&mattr))) {



        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }
    if ((rv = pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED))) {



        proc_mutex_proc_pthread_cleanup(new_mutex);
        pthread_mutexattr_destroy(&mattr);
        return rv;
    }


    if ((rv = pthread_mutexattr_setrobust_np(&mattr,
                                               PTHREAD_MUTEX_ROBUST_NP))) {



        proc_mutex_proc_pthread_cleanup(new_mutex);
        pthread_mutexattr_destroy(&mattr);
        return rv;
    }
    if ((rv = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT))) {



        proc_mutex_proc_pthread_cleanup(new_mutex);
        pthread_mutexattr_destroy(&mattr);
        return rv;
    }


    if ((rv = pthread_mutex_init(new_mutex->pthread_interproc, &mattr))) {



        proc_mutex_proc_pthread_cleanup(new_mutex);
        pthread_mutexattr_destroy(&mattr);
        return rv;
    }

    new_mutex->curr_locked = 0;

    if ((rv = pthread_mutexattr_destroy(&mattr))) {



        proc_mutex_proc_pthread_cleanup(new_mutex);
        return rv;
    }

    apr_pool_cleanup_register(new_mutex->pool,
                              (void *)new_mutex,
                              apr_proc_mutex_cleanup,
                              apr_pool_cleanup_null);
    return 0;
}

static apr_status_t proc_mutex_proc_pthread_acquire(apr_proc_mutex_t *mutex)
{
    apr_status_t rv;

    if ((rv = pthread_mutex_lock(mutex->pthread_interproc))) {





        if (rv == 130) {
            pthread_mutex_consistent_np(mutex->pthread_interproc);
        }
        else
            return rv;



    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_proc_pthread_tryacquire(apr_proc_mutex_t *mutex)
{
    apr_status_t rv;

    if ((rv = pthread_mutex_trylock(mutex->pthread_interproc))) {



        if (rv == 16) {
            return ((20000 + 50000) + 25);
        }


        if (rv == 130) {
            pthread_mutex_consistent_np(mutex->pthread_interproc);
            rv = 0;
        }
        else
            return rv;



    }
    mutex->curr_locked = 1;
    return rv;
}

static apr_status_t proc_mutex_proc_pthread_release(apr_proc_mutex_t *mutex)
{
    apr_status_t rv;

    mutex->curr_locked = 0;
    if ((rv = pthread_mutex_unlock(mutex->pthread_interproc))) {



        return rv;
    }
    return 0;
}

static const apr_proc_mutex_unix_lock_methods_t mutex_proc_pthread_methods =
{
    1,
    proc_mutex_proc_pthread_create,
    proc_mutex_proc_pthread_acquire,
    proc_mutex_proc_pthread_tryacquire,
    proc_mutex_proc_pthread_release,
    proc_mutex_proc_pthread_cleanup,
    proc_mutex_no_child_init,
    "pthread"
};





static struct flock proc_mutex_lock_it;
static struct flock proc_mutex_unlock_it;

static apr_status_t proc_mutex_fcntl_release(apr_proc_mutex_t *);

static void proc_mutex_fcntl_setup(void)
{
    proc_mutex_lock_it.l_whence = 0;
    proc_mutex_lock_it.l_start = 0;
    proc_mutex_lock_it.l_len = 0;
    proc_mutex_lock_it.l_type = 1;
    proc_mutex_lock_it.l_pid = 0;
    proc_mutex_unlock_it.l_whence = 0;
    proc_mutex_unlock_it.l_start = 0;
    proc_mutex_unlock_it.l_len = 0;
    proc_mutex_unlock_it.l_type = 2;
    proc_mutex_unlock_it.l_pid = 0;
}

static apr_status_t proc_mutex_fcntl_cleanup(void *mutex_)
{
    apr_status_t status;
    apr_proc_mutex_t *mutex=mutex_;

    if (mutex->curr_locked == 1) {
        status = proc_mutex_fcntl_release(mutex);
        if (status != 0)
            return status;
    }

    return apr_file_close(mutex->interproc);
}

static apr_status_t proc_mutex_fcntl_create(apr_proc_mutex_t *new_mutex,
                                            const char *fname)
{
    int rv;

    if (fname) {
        new_mutex->fname = apr_pstrdup(new_mutex->pool, fname);
        rv = apr_file_open(&new_mutex->interproc, new_mutex->fname,
                           0x00004 | 0x00002 | 0x00040,
                           0x0400 | 0x0200 | 0x0040 | 0x0004,
                           new_mutex->pool);
    }
    else {
        new_mutex->fname = apr_pstrdup(new_mutex->pool, "/tmp/aprXXXXXX");
        rv = apr_file_mktemp(&new_mutex->interproc, new_mutex->fname,
                             0x00004 | 0x00002 | 0x00040,
                             new_mutex->pool);
    }

    if (rv != 0) {
        return rv;
    }

    new_mutex->curr_locked = 0;
    unlink(new_mutex->fname);
    apr_pool_cleanup_register(new_mutex->pool,
                              (void*)new_mutex,
                              apr_proc_mutex_cleanup,
                              apr_pool_cleanup_null);
    return 0;
}

static apr_status_t proc_mutex_fcntl_acquire(apr_proc_mutex_t *mutex)
{
    int rc;

    do {
        rc = fcntl(mutex->interproc->filedes, 7, &proc_mutex_lock_it);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        return (*__errno_location ());
    }
    mutex->curr_locked=1;
    return 0;
}

static apr_status_t proc_mutex_fcntl_tryacquire(apr_proc_mutex_t *mutex)
{
    int rc;

    do {
        rc = fcntl(mutex->interproc->filedes, 6, &proc_mutex_lock_it);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {



        if ((*__errno_location ()) == 11) {

            return ((20000 + 50000) + 25);
        }
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_fcntl_release(apr_proc_mutex_t *mutex)
{
    int rc;

    mutex->curr_locked=0;
    do {
        rc = fcntl(mutex->interproc->filedes, 7, &proc_mutex_unlock_it);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        return (*__errno_location ());
    }
    return 0;
}

static const apr_proc_mutex_unix_lock_methods_t mutex_fcntl_methods =
{



    0,

    proc_mutex_fcntl_create,
    proc_mutex_fcntl_acquire,
    proc_mutex_fcntl_tryacquire,
    proc_mutex_fcntl_release,
    proc_mutex_fcntl_cleanup,
    proc_mutex_no_child_init,
    "fcntl"
};





static apr_status_t proc_mutex_flock_release(apr_proc_mutex_t *);

static apr_status_t proc_mutex_flock_cleanup(void *mutex_)
{
    apr_status_t status;
    apr_proc_mutex_t *mutex=mutex_;

    if (mutex->curr_locked == 1) {
        status = proc_mutex_flock_release(mutex);
        if (status != 0)
            return status;
    }
    if (mutex->interproc) {
        apr_file_close(mutex->interproc);
    }
    unlink(mutex->fname);
    return 0;
}

static apr_status_t proc_mutex_flock_create(apr_proc_mutex_t *new_mutex,
                                            const char *fname)
{
    int rv;

    if (fname) {
        new_mutex->fname = apr_pstrdup(new_mutex->pool, fname);
        rv = apr_file_open(&new_mutex->interproc, new_mutex->fname,
                           0x00004 | 0x00002 | 0x00040,
                           0x0400 | 0x0200,
                           new_mutex->pool);
    }
    else {
        new_mutex->fname = apr_pstrdup(new_mutex->pool, "/tmp/aprXXXXXX");
        rv = apr_file_mktemp(&new_mutex->interproc, new_mutex->fname,
                             0x00004 | 0x00002 | 0x00040,
                             new_mutex->pool);
    }

    if (rv != 0) {
        proc_mutex_flock_cleanup(new_mutex);
        return (*__errno_location ());
    }
    new_mutex->curr_locked = 0;
    apr_pool_cleanup_register(new_mutex->pool, (void *)new_mutex,
                              apr_proc_mutex_cleanup,
                              apr_pool_cleanup_null);
    return 0;
}

static apr_status_t proc_mutex_flock_acquire(apr_proc_mutex_t *mutex)
{
    int rc;

    do {
        rc = flock(mutex->interproc->filedes, 2);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_flock_tryacquire(apr_proc_mutex_t *mutex)
{
    int rc;

    do {
        rc = flock(mutex->interproc->filedes, 2 | 4);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        if ((*__errno_location ()) == 11 || (*__errno_location ()) == 11) {
            return ((20000 + 50000) + 25);
        }
        return (*__errno_location ());
    }
    mutex->curr_locked = 1;
    return 0;
}

static apr_status_t proc_mutex_flock_release(apr_proc_mutex_t *mutex)
{
    int rc;

    mutex->curr_locked = 0;
    do {
        rc = flock(mutex->interproc->filedes, 8);
    } while (rc < 0 && (*__errno_location ()) == 4);
    if (rc < 0) {
        return (*__errno_location ());
    }
    return 0;
}

static apr_status_t proc_mutex_flock_child_init(apr_proc_mutex_t **mutex,
                                                apr_pool_t *pool,
                                                const char *fname)
{
    apr_proc_mutex_t *new_mutex;
    int rv;

    new_mutex = (apr_proc_mutex_t *)apr_palloc(pool, sizeof(apr_proc_mutex_t));

    memcpy(new_mutex, *mutex, sizeof *new_mutex);
    new_mutex->pool = pool;
    if (!fname) {
        fname = (*mutex)->fname;
    }
    new_mutex->fname = apr_pstrdup(pool, fname);
    rv = apr_file_open(&new_mutex->interproc, new_mutex->fname,
                       0x00002, 0, new_mutex->pool);
    if (rv != 0) {
        return rv;
    }
    *mutex = new_mutex;
    return 0;
}

static const apr_proc_mutex_unix_lock_methods_t mutex_flock_methods =
{



    0,

    proc_mutex_flock_create,
    proc_mutex_flock_acquire,
    proc_mutex_flock_tryacquire,
    proc_mutex_flock_release,
    proc_mutex_flock_cleanup,
    proc_mutex_flock_child_init,
    "flock"
};



void apr_proc_mutex_unix_setup_lock(void)
{


    proc_mutex_sysv_setup();


    proc_mutex_fcntl_setup();

}

static apr_status_t proc_mutex_choose_method(apr_proc_mutex_t *new_mutex, apr_lockmech_e mech)
{
    switch (mech) {
    case APR_LOCK_FCNTL:

        new_mutex->inter_meth = &mutex_fcntl_methods;



        break;
    case APR_LOCK_FLOCK:

        new_mutex->inter_meth = &mutex_flock_methods;



        break;
    case APR_LOCK_SYSVSEM:

        new_mutex->inter_meth = &mutex_sysv_methods;



        break;
    case APR_LOCK_POSIXSEM:

        new_mutex->inter_meth = &mutex_posixsem_methods;



        break;
    case APR_LOCK_PROC_PTHREAD:

        new_mutex->inter_meth = &mutex_proc_pthread_methods;



        break;
    case APR_LOCK_DEFAULT:



        new_mutex->inter_meth = &mutex_sysv_methods;
# 842 "./../locks/unix/proc_mutex.c"
        break;
    default:
        return ((20000 + 50000) + 23);
    }
    return 0;
}

const char * apr_proc_mutex_defname(void)
{
    apr_status_t rv;
    apr_proc_mutex_t mutex;

    if ((rv = proc_mutex_choose_method(&mutex, APR_LOCK_DEFAULT)) != 0) {
        return "unknown";
    }
    mutex.meth = mutex.inter_meth;

    return apr_proc_mutex_name(&mutex);
}

static apr_status_t proc_mutex_create(apr_proc_mutex_t *new_mutex, apr_lockmech_e mech, const char *fname)
{
    apr_status_t rv;

    if ((rv = proc_mutex_choose_method(new_mutex, mech)) != 0) {
        return rv;
    }

    new_mutex->meth = new_mutex->inter_meth;

    if ((rv = new_mutex->meth->create(new_mutex, fname)) != 0) {
        return rv;
    }

    return 0;
}

apr_status_t apr_proc_mutex_create(apr_proc_mutex_t **mutex,
                                                const char *fname,
                                                apr_lockmech_e mech,
                                                apr_pool_t *pool)
{
    apr_proc_mutex_t *new_mutex;
    apr_status_t rv;

    new_mutex = memset(apr_palloc(pool, sizeof(apr_proc_mutex_t)), 0, sizeof(apr_proc_mutex_t));
    new_mutex->pool = pool;

    if ((rv = proc_mutex_create(new_mutex, mech, fname)) != 0)
        return rv;

    *mutex = new_mutex;
    return 0;
}

apr_status_t apr_proc_mutex_child_init(apr_proc_mutex_t **mutex,
                                                    const char *fname,
                                                    apr_pool_t *pool)
{
    return (*mutex)->meth->child_init(mutex, pool, fname);
}

apr_status_t apr_proc_mutex_lock(apr_proc_mutex_t *mutex)
{
    return mutex->meth->acquire(mutex);
}

apr_status_t apr_proc_mutex_trylock(apr_proc_mutex_t *mutex)
{
    return mutex->meth->tryacquire(mutex);
}

apr_status_t apr_proc_mutex_unlock(apr_proc_mutex_t *mutex)
{
    return mutex->meth->release(mutex);
}

apr_status_t apr_proc_mutex_cleanup(void *mutex)
{
    return ((apr_proc_mutex_t *)mutex)->meth->cleanup(mutex);
}

const char * apr_proc_mutex_name(apr_proc_mutex_t *mutex)
{
    return mutex->meth->name;
}

const char * apr_proc_mutex_lockfile(apr_proc_mutex_t *mutex)
{



    if (mutex->meth == &mutex_flock_methods) {
        return mutex->fname;
    }


    if (mutex->meth == &mutex_fcntl_methods) {
        return mutex->fname;
    }

    return ((void*)0);
}

apr_pool_t * apr_proc_mutex_pool_get (const apr_proc_mutex_t *theproc_mutex) { return theproc_mutex->pool; }



apr_status_t apr_os_proc_mutex_get(apr_os_proc_mutex_t *ospmutex,
                                                apr_proc_mutex_t *pmutex)
{

    if (pmutex->interproc) {
        ospmutex->crossproc = pmutex->interproc->filedes;
    }
    else {
        ospmutex->crossproc = -1;
    }


    ospmutex->pthread_interproc = pmutex->pthread_interproc;

    return 0;
}

apr_status_t apr_os_proc_mutex_put(apr_proc_mutex_t **pmutex,
                                                apr_os_proc_mutex_t *ospmutex,
                                                apr_pool_t *pool)
{
    if (pool == ((void*)0)) {
        return (20000 + 2);
    }
    if ((*pmutex) == ((void*)0)) {
        (*pmutex) = (apr_proc_mutex_t *)memset(apr_palloc(pool, sizeof(apr_proc_mutex_t)), 0, sizeof(apr_proc_mutex_t));

        (*pmutex)->pool = pool;
    }

    apr_os_file_put(&(*pmutex)->interproc, &ospmutex->crossproc, 0, pool);


    (*pmutex)->pthread_interproc = ospmutex->pthread_interproc;

    return 0;
}
# 13 "main2.c" 2
# 1 "./../memory/unix/apr_pools.c" 1
# 31 "./../memory/unix/apr_pools.c"
# 1 "../include/apr_want.h" 1
# 32 "./../memory/unix/apr_pools.c" 2
# 1 "../include/apr_env.h" 1
# 42 "../include/apr_env.h"
apr_status_t apr_env_get(char **value, const char *envvar,
                                      apr_pool_t *pool);







apr_status_t apr_env_set(const char *envvar, const char *value,
                                      apr_pool_t *pool);






apr_status_t apr_env_delete(const char *envvar, apr_pool_t *pool);
# 33 "./../memory/unix/apr_pools.c" 2
# 85 "./../memory/unix/apr_pools.c"
struct apr_allocator_t {

    apr_uint32_t max_index;





    apr_uint32_t max_free_index;




    apr_uint32_t current_free_index;

    apr_thread_mutex_t *mutex;

    apr_pool_t *owner;
# 113 "./../memory/unix/apr_pools.c"
    apr_memnode_t *free[20];
};
# 123 "./../memory/unix/apr_pools.c"
apr_status_t apr_allocator_create(apr_allocator_t **allocator)
{
    apr_allocator_t *new_allocator;

    *allocator = ((void*)0);

    if ((new_allocator = malloc((((sizeof(apr_allocator_t)) + ((8) - 1)) & ~((8) - 1)))) == ((void*)0))
        return 12;

    memset(new_allocator, 0, (((sizeof(apr_allocator_t)) + ((8) - 1)) & ~((8) - 1)));
    new_allocator->max_free_index = 0;

    *allocator = new_allocator;

    return 0;
}

void apr_allocator_destroy(apr_allocator_t *allocator)
{
    apr_uint32_t index;
    apr_memnode_t *node, **ref;

    for (index = 0; index < 20; index++) {
        ref = &allocator->free[index];
        while ((node = *ref) != ((void*)0)) {
            *ref = node->next;



            free(node);

        }
    }

    free(allocator);
}


void apr_allocator_mutex_set(apr_allocator_t *allocator,
                                          apr_thread_mutex_t *mutex)
{
    allocator->mutex = mutex;
}

apr_thread_mutex_t * apr_allocator_mutex_get(
                                      apr_allocator_t *allocator)
{
    return allocator->mutex;
}


void apr_allocator_owner_set(apr_allocator_t *allocator,
                                          apr_pool_t *pool)
{
    allocator->owner = pool;
}

apr_pool_t * apr_allocator_owner_get(apr_allocator_t *allocator)
{
    return allocator->owner;
}

void apr_allocator_max_free_set(apr_allocator_t *allocator,
                                             apr_size_t in_size)
{
    apr_uint32_t max_free_index;
    apr_uint32_t size = (apr_uint32_t)in_size;


    apr_thread_mutex_t *mutex;

    mutex = apr_allocator_mutex_get(allocator);
    if (mutex != ((void*)0))
        apr_thread_mutex_lock(mutex);


    max_free_index = (((size) + (((1 << 12)) - 1)) & ~(((1 << 12)) - 1)) >> 12;
    allocator->current_free_index += max_free_index;
    allocator->current_free_index -= allocator->max_free_index;
    allocator->max_free_index = max_free_index;
    if (allocator->current_free_index > max_free_index)
        allocator->current_free_index = max_free_index;


    if (mutex != ((void*)0))
        apr_thread_mutex_unlock(mutex);

}

static __inline__
apr_memnode_t *allocator_alloc(apr_allocator_t *allocator, apr_size_t in_size)
{
    apr_memnode_t *node, **ref;
    apr_uint32_t max_index;
    apr_size_t size, i, index;




    size = (((in_size + (((sizeof(apr_memnode_t)) + ((8) - 1)) & ~((8) - 1))) + (((1 << 12)) - 1)) & ~(((1 << 12)) - 1));
    if (size < in_size) {
        return ((void*)0);
    }
    if (size < (2 * (1 << 12)))
        size = (2 * (1 << 12));




    index = (size >> 12) - 1;

    if (index > (4294967295U)) {
        return ((void*)0);
    }




    if (index <= allocator->max_index) {

        if (allocator->mutex)
            apr_thread_mutex_lock(allocator->mutex);
# 257 "./../memory/unix/apr_pools.c"
        max_index = allocator->max_index;
        ref = &allocator->free[index];
        i = index;
        while (*ref == ((void*)0) && i < max_index) {
           ref++;
           i++;
        }

        if ((node = *ref) != ((void*)0)) {





            if ((*ref = node->next) == ((void*)0) && i >= max_index) {
                do {
                    ref--;
                    max_index--;
                }
                while (*ref == ((void*)0) && max_index > 0);

                allocator->max_index = max_index;
            }

            allocator->current_free_index += node->index + 1;
            if (allocator->current_free_index > allocator->max_free_index)
                allocator->current_free_index = allocator->max_free_index;


            if (allocator->mutex)
                apr_thread_mutex_unlock(allocator->mutex);


            node->next = ((void*)0);
            node->first_avail = (char *)node + (((sizeof(apr_memnode_t)) + ((8) - 1)) & ~((8) - 1));

            return node;
        }


        if (allocator->mutex)
            apr_thread_mutex_unlock(allocator->mutex);

    }




    else if (allocator->free[0]) {

        if (allocator->mutex)
            apr_thread_mutex_lock(allocator->mutex);





        ref = &allocator->free[0];
        while ((node = *ref) != ((void*)0) && index > node->index)
            ref = &node->next;

        if (node) {
            *ref = node->next;

            allocator->current_free_index += node->index + 1;
            if (allocator->current_free_index > allocator->max_free_index)
                allocator->current_free_index = allocator->max_free_index;


            if (allocator->mutex)
                apr_thread_mutex_unlock(allocator->mutex);


            node->next = ((void*)0);
            node->first_avail = (char *)node + (((sizeof(apr_memnode_t)) + ((8) - 1)) & ~((8) - 1));

            return node;
        }


        if (allocator->mutex)
            apr_thread_mutex_unlock(allocator->mutex);

    }
# 349 "./../memory/unix/apr_pools.c"
    if ((node = malloc(size)) == ((void*)0))

        return ((void*)0);

    node->next = ((void*)0);
    node->index = (apr_uint32_t)index;
    node->first_avail = (char *)node + (((sizeof(apr_memnode_t)) + ((8) - 1)) & ~((8) - 1));
    node->endp = (char *)node + size;

    return node;
}

static __inline__
void allocator_free(apr_allocator_t *allocator, apr_memnode_t *node)
{
    apr_memnode_t *next, *freelist = ((void*)0);
    apr_uint32_t index, max_index;
    apr_uint32_t max_free_index, current_free_index;


    if (allocator->mutex)
        apr_thread_mutex_lock(allocator->mutex);


    max_index = allocator->max_index;
    max_free_index = allocator->max_free_index;
    current_free_index = allocator->current_free_index;




    do {
        next = node->next;
        index = node->index;

        if (max_free_index != 0
            && index + 1 > current_free_index) {
            node->next = freelist;
            freelist = node;
        }
        else if (index < 20) {



            if ((node->next = allocator->free[index]) == ((void*)0)
                && index > max_index) {
                max_index = index;
            }
            allocator->free[index] = node;
            if (current_free_index >= index + 1)
                current_free_index -= index + 1;
            else
                current_free_index = 0;
        }
        else {



            node->next = allocator->free[0];
            allocator->free[0] = node;
            if (current_free_index >= index + 1)
                current_free_index -= index + 1;
            else
                current_free_index = 0;
        }
    } while ((node = next) != ((void*)0));

    allocator->max_index = max_index;
    allocator->current_free_index = current_free_index;


    if (allocator->mutex)
        apr_thread_mutex_unlock(allocator->mutex);


    while (freelist != ((void*)0)) {
        node = freelist;
        freelist = node->next;



        free(node);

    }
}

apr_memnode_t * apr_allocator_alloc(apr_allocator_t *allocator,
                                                 apr_size_t size)
{
    return allocator_alloc(allocator, size);
}

void apr_allocator_free(apr_allocator_t *allocator,
                                     apr_memnode_t *node)
{
    allocator_free(allocator, node);
}
# 467 "./../memory/unix/apr_pools.c"
typedef struct cleanup_t cleanup_t;


struct process_chain {

    apr_proc_t *proc;
    apr_kill_conditions_e kill_how;

    struct process_chain *next;
};
# 500 "./../memory/unix/apr_pools.c"
struct apr_pool_t {
    apr_pool_t *parent;
    apr_pool_t *child;
    apr_pool_t *sibling;
    apr_pool_t **ref;
    cleanup_t *cleanups;
    cleanup_t *free_cleanups;
    apr_allocator_t *allocator;
    struct process_chain *subprocesses;
    apr_abortfunc_t abort_fn;
    apr_hash_t *user_data;
    const char *tag;


    apr_memnode_t *active;
    apr_memnode_t *self;
    char *self_first_avail;
# 535 "./../memory/unix/apr_pools.c"
    cleanup_t *pre_cleanups;
};
# 545 "./../memory/unix/apr_pools.c"
static apr_byte_t apr_pools_initialized = 0;
static apr_pool_t *global_pool = ((void*)0);


static apr_allocator_t *global_allocator = ((void*)0);
# 560 "./../memory/unix/apr_pools.c"
static void run_cleanups(cleanup_t **c);
static void free_proc_chain(struct process_chain *procs);
# 572 "./../memory/unix/apr_pools.c"
apr_status_t apr_pool_initialize(void)
{
    apr_status_t rv;

    if (apr_pools_initialized++)
        return 0;
# 587 "./../memory/unix/apr_pools.c"
    if ((rv = apr_allocator_create(&global_allocator)) != 0) {
        apr_pools_initialized = 0;
        return rv;
    }

    if ((rv = apr_pool_create_ex(&global_pool, ((void*)0), ((void*)0),
                                 global_allocator)) != 0) {
        apr_allocator_destroy(global_allocator);
        global_allocator = ((void*)0);
        apr_pools_initialized = 0;
        return rv;
    }

    apr_pool_tag(global_pool, "apr_global_pool");







    if ((rv = apr_atomic_init(global_pool)) != 0) {
        return rv;
    }


    {
        apr_thread_mutex_t *mutex;

        if ((rv = apr_thread_mutex_create(&mutex,
                                          0x0,
                                          global_pool)) != 0) {
            return rv;
        }

        apr_allocator_mutex_set(global_allocator, mutex);
    }


    apr_allocator_owner_set(global_allocator, global_pool);

    return 0;
}

void apr_pool_terminate(void)
{
    if (!apr_pools_initialized)
        return;

    if (--apr_pools_initialized)
        return;

    apr_pool_destroy(global_pool);
    global_pool = ((void*)0);

    global_allocator = ((void*)0);
}
# 668 "./../memory/unix/apr_pools.c"
void * apr_palloc(apr_pool_t *pool, apr_size_t in_size)
{
    apr_memnode_t *active, *node;
    void *mem;
    apr_size_t size, free_index;

    size = (((in_size) + ((8) - 1)) & ~((8) - 1));
    if (size < in_size) {
        if (pool->abort_fn)
            pool->abort_fn(12);

        return ((void*)0);
    }
    active = pool->active;


    if (size <= ((apr_size_t)(active->endp - active->first_avail))) {
        mem = active->first_avail;
        active->first_avail += size;

        return mem;
    }

    node = active->next;
    if (size <= ((apr_size_t)(node->endp - node->first_avail))) {
        do { *node->ref = node->next; node->next->ref = node->ref; } while (0);
    }
    else {
        if ((node = allocator_alloc(pool->allocator, size)) == ((void*)0)) {
            if (pool->abort_fn)
                pool->abort_fn(12);

            return ((void*)0);
        }
    }

    node->free_index = 0;

    mem = node->first_avail;
    node->first_avail += size;

    do { node->ref = active->ref; *node->ref = node; node->next = active; active->ref = &node->next; } while (0);

    pool->active = node;

    free_index = ((((active->endp - active->first_avail + 1) + (((1 << 12)) - 1)) & ~(((1 << 12)) - 1)) - (1 << 12)) >> 12;


    active->free_index = (apr_uint32_t)free_index;
    node = active->next;
    if (free_index >= node->free_index)
        return mem;

    do {
        node = node->next;
    }
    while (free_index < node->free_index);

    do { *active->ref = active->next; active->next->ref = active->ref; } while (0);
    do { active->ref = node->ref; *active->ref = active; active->next = node; node->ref = &active->next; } while (0);

    return mem;
}
# 740 "./../memory/unix/apr_pools.c"
void * apr_pcalloc(apr_pool_t *pool, apr_size_t size);
void * apr_pcalloc(apr_pool_t *pool, apr_size_t size)
{
    void *mem;

    if ((mem = apr_palloc(pool, size)) != ((void*)0)) {
        memset(mem, 0, size);
    }

    return mem;
}






void apr_pool_clear(apr_pool_t *pool)
{
    apr_memnode_t *active;


    run_cleanups(&pool->pre_cleanups);
    pool->pre_cleanups = ((void*)0);




    while (pool->child)
        apr_pool_destroy(pool->child);


    run_cleanups(&pool->cleanups);
    pool->cleanups = ((void*)0);
    pool->free_cleanups = ((void*)0);


    free_proc_chain(pool->subprocesses);
    pool->subprocesses = ((void*)0);


    pool->user_data = ((void*)0);




    active = pool->active = pool->self;
    active->first_avail = pool->self_first_avail;

    if (active->next == active)
        return;

    *active->ref = ((void*)0);
    allocator_free(pool->allocator, active->next);
    active->next = active;
    active->ref = &active->next;
}

void apr_pool_destroy(apr_pool_t *pool)
{
    apr_memnode_t *active;
    apr_allocator_t *allocator;


    run_cleanups(&pool->pre_cleanups);
    pool->pre_cleanups = ((void*)0);




    while (pool->child)
        apr_pool_destroy(pool->child);


    run_cleanups(&pool->cleanups);


    free_proc_chain(pool->subprocesses);


    if (pool->parent) {

        apr_thread_mutex_t *mutex;

        if ((mutex = apr_allocator_mutex_get(pool->parent->allocator)) != ((void*)0))
            apr_thread_mutex_lock(mutex);


        if ((*pool->ref = pool->sibling) != ((void*)0))
            pool->sibling->ref = pool->ref;


        if (mutex)
            apr_thread_mutex_unlock(mutex);

    }




    allocator = pool->allocator;
    active = pool->self;
    *active->ref = ((void*)0);


    if (apr_allocator_owner_get(allocator) == pool) {



        apr_allocator_mutex_set(allocator, ((void*)0));
    }





    allocator_free(allocator, active);






    if (apr_allocator_owner_get(allocator) == pool) {
        apr_allocator_destroy(allocator);
    }
}

apr_status_t apr_pool_create_ex(apr_pool_t **newpool,
                                             apr_pool_t *parent,
                                             apr_abortfunc_t abort_fn,
                                             apr_allocator_t *allocator)
{
    apr_pool_t *pool;
    apr_memnode_t *node;

    *newpool = ((void*)0);

    if (!parent)
        parent = global_pool;





    if (!abort_fn && parent)
        abort_fn = parent->abort_fn;

    if (allocator == ((void*)0))
        allocator = parent->allocator;

    if ((node = allocator_alloc(allocator,
                                (2 * (1 << 12)) - (((sizeof(apr_memnode_t)) + ((8) - 1)) & ~((8) - 1)))) == ((void*)0)) {
        if (abort_fn)
            abort_fn(12);

        return 12;
    }

    node->next = node;
    node->ref = &node->next;

    pool = (apr_pool_t *)node->first_avail;
    node->first_avail = pool->self_first_avail = (char *)pool + (((sizeof(apr_pool_t)) + ((8) - 1)) & ~((8) - 1));

    pool->allocator = allocator;
    pool->active = pool->self = node;
    pool->abort_fn = abort_fn;
    pool->child = ((void*)0);
    pool->cleanups = ((void*)0);
    pool->free_cleanups = ((void*)0);
    pool->pre_cleanups = ((void*)0);
    pool->subprocesses = ((void*)0);
    pool->user_data = ((void*)0);
    pool->tag = ((void*)0);





    if ((pool->parent = parent) != ((void*)0)) {

        apr_thread_mutex_t *mutex;

        if ((mutex = apr_allocator_mutex_get(parent->allocator)) != ((void*)0))
            apr_thread_mutex_lock(mutex);


        if ((pool->sibling = parent->child) != ((void*)0))
            pool->sibling->ref = &pool->sibling;

        parent->child = pool;
        pool->ref = &parent->child;


        if (mutex)
            apr_thread_mutex_unlock(mutex);

    }
    else {
        pool->sibling = ((void*)0);
        pool->ref = ((void*)0);
    }

    *newpool = pool;

    return 0;
}



apr_status_t apr_pool_create_core_ex(apr_pool_t **newpool,
                                                  apr_abortfunc_t abort_fn,
                                                  apr_allocator_t *allocator)
{
    return apr_pool_create_unmanaged_ex(newpool, abort_fn, allocator);
}

apr_status_t apr_pool_create_unmanaged_ex(apr_pool_t **newpool,
                                                  apr_abortfunc_t abort_fn,
                                                  apr_allocator_t *allocator)
{
    apr_pool_t *pool;
    apr_memnode_t *node;
    apr_allocator_t *pool_allocator;

    *newpool = ((void*)0);

    if (!apr_pools_initialized)
        return (20000 + 2);
    if ((pool_allocator = allocator) == ((void*)0)) {
        if ((pool_allocator = malloc((((sizeof(apr_allocator_t)) + ((8) - 1)) & ~((8) - 1)))) == ((void*)0)) {
            if (abort_fn)
                abort_fn(12);

            return 12;
        }
        memset(pool_allocator, 0, (((sizeof(apr_allocator_t)) + ((8) - 1)) & ~((8) - 1)));
        pool_allocator->max_free_index = 0;
    }
    if ((node = allocator_alloc(pool_allocator,
                                (2 * (1 << 12)) - (((sizeof(apr_memnode_t)) + ((8) - 1)) & ~((8) - 1)))) == ((void*)0)) {
        if (abort_fn)
            abort_fn(12);

        return 12;
    }

    node->next = node;
    node->ref = &node->next;

    pool = (apr_pool_t *)node->first_avail;
    node->first_avail = pool->self_first_avail = (char *)pool + (((sizeof(apr_pool_t)) + ((8) - 1)) & ~((8) - 1));

    pool->allocator = pool_allocator;
    pool->active = pool->self = node;
    pool->abort_fn = abort_fn;
    pool->child = ((void*)0);
    pool->cleanups = ((void*)0);
    pool->free_cleanups = ((void*)0);
    pool->pre_cleanups = ((void*)0);
    pool->subprocesses = ((void*)0);
    pool->user_data = ((void*)0);
    pool->tag = ((void*)0);
    pool->parent = ((void*)0);
    pool->sibling = ((void*)0);
    pool->ref = ((void*)0);




    if (!allocator)
        pool_allocator->owner = pool;
    *newpool = pool;

    return 0;
}
# 1036 "./../memory/unix/apr_pools.c"
struct psprintf_data {
    apr_vformatter_buff_t vbuff;
    apr_memnode_t *node;
    apr_pool_t *pool;
    apr_byte_t got_a_new_node;
    apr_memnode_t *free;
};



static int psprintf_flush(apr_vformatter_buff_t *vbuff)
{
    struct psprintf_data *ps = (struct psprintf_data *)vbuff;
    apr_memnode_t *node, *active;
    apr_size_t cur_len, size;
    char *strp;
    apr_pool_t *pool;
    apr_size_t free_index;

    pool = ps->pool;
    active = ps->node;
    strp = ps->vbuff.curpos;
    cur_len = strp - active->first_avail;
    size = cur_len << 1;






    if (size < 32)
        size = 32;

    node = active->next;
    if (!ps->got_a_new_node && size <= ((apr_size_t)(node->endp - node->first_avail))) {

        do { *node->ref = node->next; node->next->ref = node->ref; } while (0);
        do { node->ref = active->ref; *node->ref = node; node->next = active; active->ref = &node->next; } while (0);

        node->free_index = 0;

        pool->active = node;

        free_index = ((((active->endp - active->first_avail + 1) + (((1 << 12)) - 1)) & ~(((1 << 12)) - 1)) - (1 << 12)) >> 12;


        active->free_index = (apr_uint32_t)free_index;
        node = active->next;
        if (free_index < node->free_index) {
            do {
                node = node->next;
            }
            while (free_index < node->free_index);

            do { *active->ref = active->next; active->next->ref = active->ref; } while (0);
            do { active->ref = node->ref; *active->ref = active; active->next = node; node->ref = &active->next; } while (0);
        }

        node = pool->active;
    }
    else {
        if ((node = allocator_alloc(pool->allocator, size)) == ((void*)0))
            return -1;

        if (ps->got_a_new_node) {
            active->next = ps->free;
            ps->free = active;
        }

        ps->got_a_new_node = 1;
    }

    memcpy(node->first_avail, active->first_avail, cur_len);

    ps->node = node;
    ps->vbuff.curpos = node->first_avail + cur_len;
    ps->vbuff.endpos = node->endp - 1;

    return 0;
}

char * apr_pvsprintf(apr_pool_t *pool, const char *fmt, va_list ap)
{
    struct psprintf_data ps;
    char *strp;
    apr_size_t size;
    apr_memnode_t *active, *node;
    apr_size_t free_index;

    ps.node = active = pool->active;
    ps.pool = pool;
    ps.vbuff.curpos = ps.node->first_avail;


    ps.vbuff.endpos = ps.node->endp - 1;
    ps.got_a_new_node = 0;
    ps.free = ((void*)0);




    if (ps.node->first_avail == ps.node->endp) {
        if (psprintf_flush(&ps.vbuff) == -1) {
            if (pool->abort_fn) {
                pool->abort_fn(12);
            }

            return ((void*)0);
        }
    }

    if (apr_vformatter(psprintf_flush, &ps.vbuff, fmt, ap) == -1) {
        if (pool->abort_fn)
            pool->abort_fn(12);

        return ((void*)0);
    }

    strp = ps.vbuff.curpos;
    *strp++ = '\0';

    size = strp - ps.node->first_avail;
    size = (((size) + ((8) - 1)) & ~((8) - 1));
    strp = ps.node->first_avail;
    ps.node->first_avail += size;

    if (ps.free)
        allocator_free(pool->allocator, ps.free);




    if (!ps.got_a_new_node)
        return strp;

    active = pool->active;
    node = ps.node;

    node->free_index = 0;

    do { node->ref = active->ref; *node->ref = node; node->next = active; active->ref = &node->next; } while (0);

    pool->active = node;

    free_index = ((((active->endp - active->first_avail + 1) + (((1 << 12)) - 1)) & ~(((1 << 12)) - 1)) - (1 << 12)) >> 12;


    active->free_index = (apr_uint32_t)free_index;
    node = active->next;

    if (free_index >= node->free_index)
        return strp;

    do {
        node = node->next;
    }
    while (free_index < node->free_index);

    do { *active->ref = active->next; active->next->ref = active->ref; } while (0);
    do { active->ref = node->ref; *active->ref = active; active->next = node; node->ref = &active->next; } while (0);

    return strp;
}
# 2049 "./../memory/unix/apr_pools.c"
char * apr_psprintf(apr_pool_t *p, const char *fmt, ...)
{
    va_list ap;
    char *res;

    __builtin_va_start(ap, fmt);
    res = apr_pvsprintf(p, fmt, ap);
    __builtin_va_end(ap);
    return res;
}





void apr_pool_abort_set(apr_abortfunc_t abort_fn,
                                     apr_pool_t *pool)
{
    pool->abort_fn = abort_fn;
}

apr_abortfunc_t apr_pool_abort_get(apr_pool_t *pool)
{
    return pool->abort_fn;
}

apr_pool_t * apr_pool_parent_get(apr_pool_t *pool)
{







    return pool->parent;
}

apr_allocator_t * apr_pool_allocator_get(apr_pool_t *pool)
{
    return pool->allocator;
}




int apr_pool_is_ancestor(apr_pool_t *a, apr_pool_t *b)
{
    if (a == ((void*)0))
        return 1;
# 2108 "./../memory/unix/apr_pools.c"
    while (b) {
        if (a == b)
            return 1;

        b = b->parent;
    }

    return 0;
}

void apr_pool_tag(apr_pool_t *pool, const char *tag)
{
    pool->tag = tag;
}






apr_status_t apr_pool_userdata_set(const void *data, const char *key,
                                                apr_status_t (*cleanup) (void *),
                                                apr_pool_t *pool)
{




    if (pool->user_data == ((void*)0))
        pool->user_data = apr_hash_make(pool);

    if (apr_hash_get(pool->user_data, key, (-1)) == ((void*)0)) {
        char *new_key = apr_pstrdup(pool, key);
        apr_hash_set(pool->user_data, new_key, (-1), data);
    }
    else {
        apr_hash_set(pool->user_data, key, (-1), data);
    }

    if (cleanup)
        apr_pool_cleanup_register(pool, data, cleanup, cleanup);

    return 0;
}

apr_status_t apr_pool_userdata_setn(const void *data,
                              const char *key,
                              apr_status_t (*cleanup)(void *),
                              apr_pool_t *pool)
{




    if (pool->user_data == ((void*)0))
        pool->user_data = apr_hash_make(pool);

    apr_hash_set(pool->user_data, key, (-1), data);

    if (cleanup)
        apr_pool_cleanup_register(pool, data, cleanup, cleanup);

    return 0;
}

apr_status_t apr_pool_userdata_get(void **data, const char *key,
                                                apr_pool_t *pool)
{




    if (pool->user_data == ((void*)0)) {
        *data = ((void*)0);
    }
    else {
        *data = apr_hash_get(pool->user_data, key, (-1));
    }

    return 0;
}






struct cleanup_t {
    struct cleanup_t *next;
    const void *data;
    apr_status_t (*plain_cleanup_fn)(void *data);
    apr_status_t (*child_cleanup_fn)(void *data);
};

void apr_pool_cleanup_register(apr_pool_t *p, const void *data,
                      apr_status_t (*plain_cleanup_fn)(void *data),
                      apr_status_t (*child_cleanup_fn)(void *data))
{
    cleanup_t *c;





    if (p != ((void*)0)) {
        if (p->free_cleanups) {

            c = p->free_cleanups;
            p->free_cleanups = c->next;
        } else {
            c = apr_palloc(p, sizeof(cleanup_t));
        }
        c->data = data;
        c->plain_cleanup_fn = plain_cleanup_fn;
        c->child_cleanup_fn = child_cleanup_fn;
        c->next = p->cleanups;
        p->cleanups = c;
    }
}

void apr_pool_pre_cleanup_register(apr_pool_t *p, const void *data,
                      apr_status_t (*plain_cleanup_fn)(void *data))
{
    cleanup_t *c;





    if (p != ((void*)0)) {
        if (p->free_cleanups) {

            c = p->free_cleanups;
            p->free_cleanups = c->next;
        } else {
            c = apr_palloc(p, sizeof(cleanup_t));
        }
        c->data = data;
        c->plain_cleanup_fn = plain_cleanup_fn;
        c->next = p->pre_cleanups;
        p->pre_cleanups = c;
    }
}

void apr_pool_cleanup_kill(apr_pool_t *p, const void *data,
                      apr_status_t (*cleanup_fn)(void *))
{
    cleanup_t *c, **lastp;





    if (p == ((void*)0))
        return;

    c = p->cleanups;
    lastp = &p->cleanups;
    while (c) {
# 2276 "./../memory/unix/apr_pools.c"
        if (c->data == data && c->plain_cleanup_fn == cleanup_fn) {
            *lastp = c->next;

            c->next = p->free_cleanups;
            p->free_cleanups = c;
            break;
        }

        lastp = &c->next;
        c = c->next;
    }


    c = p->pre_cleanups;
    lastp = &p->pre_cleanups;
    while (c) {
# 2301 "./../memory/unix/apr_pools.c"
        if (c->data == data && c->plain_cleanup_fn == cleanup_fn) {
            *lastp = c->next;

            c->next = p->free_cleanups;
            p->free_cleanups = c;
            break;
        }

        lastp = &c->next;
        c = c->next;
    }

}

void apr_pool_child_cleanup_set(apr_pool_t *p, const void *data,
                      apr_status_t (*plain_cleanup_fn)(void *),
                      apr_status_t (*child_cleanup_fn)(void *))
{
    cleanup_t *c;





    if (p == ((void*)0))
        return;

    c = p->cleanups;
    while (c) {
        if (c->data == data && c->plain_cleanup_fn == plain_cleanup_fn) {
            c->child_cleanup_fn = child_cleanup_fn;
            break;
        }

        c = c->next;
    }
}

apr_status_t apr_pool_cleanup_run(apr_pool_t *p, void *data,
                              apr_status_t (*cleanup_fn)(void *))
{
    apr_pool_cleanup_kill(p, data, cleanup_fn);
    return (*cleanup_fn)(data);
}

static void run_cleanups(cleanup_t **cref)
{
    cleanup_t *c = *cref;

    while (c) {
        *cref = c->next;
        (*c->plain_cleanup_fn)((void *)c->data);
        c = *cref;
    }
}



static void run_child_cleanups(cleanup_t **cref)
{
    cleanup_t *c = *cref;

    while (c) {
        *cref = c->next;
        (*c->child_cleanup_fn)((void *)c->data);
        c = *cref;
    }
}

static void cleanup_pool_for_exec(apr_pool_t *p)
{
    run_child_cleanups(&p->cleanups);

    for (p = p->child; p; p = p->sibling)
        cleanup_pool_for_exec(p);
}

void apr_pool_cleanup_for_exec(void)
{
    cleanup_pool_for_exec(global_pool);
}
# 2402 "./../memory/unix/apr_pools.c"
apr_status_t apr_pool_cleanup_null(void *data)
{

    return 0;
}
# 2415 "./../memory/unix/apr_pools.c"
void apr_pool_note_subprocess(apr_pool_t *pool, apr_proc_t *proc,
                                           apr_kill_conditions_e how)
{
    struct process_chain *pc = apr_palloc(pool, sizeof(struct process_chain));

    pc->proc = proc;
    pc->kill_how = how;
    pc->next = pool->subprocesses;
    pool->subprocesses = pc;
}

static void free_proc_chain(struct process_chain *procs)
{




    struct process_chain *pc;
    int need_timeout = 0;
    apr_time_t timeout_interval;

    if (!procs)
        return;
# 2448 "./../memory/unix/apr_pools.c"
    for (pc = procs; pc; pc = pc->next) {
        if (apr_proc_wait(pc->proc, ((void*)0), ((void*)0), APR_NOWAIT) != ((20000 + 50000) + 6))
            pc->kill_how = APR_KILL_NEVER;
    }


    for (pc = procs; pc; pc = pc->next) {

        if ((pc->kill_how == APR_KILL_AFTER_TIMEOUT)
            || (pc->kill_how == APR_KILL_ONLY_ONCE)) {






            if (apr_proc_kill(pc->proc, 15) == 0)
                need_timeout = 1;
        }
        else if (pc->kill_how == APR_KILL_ALWAYS) {





            apr_proc_kill(pc->proc, 9);
        }
    }





    if (need_timeout) {
        timeout_interval = 46875;
        apr_sleep(timeout_interval);

        do {

            need_timeout = 0;
            for (pc = procs; pc; pc = pc->next) {
                if (pc->kill_how == APR_KILL_AFTER_TIMEOUT) {
                    if (apr_proc_wait(pc->proc, ((void*)0), ((void*)0), APR_NOWAIT)
                            == ((20000 + 50000) + 6))
                        need_timeout = 1;
                    else
                        pc->kill_how = APR_KILL_NEVER;
                }
            }
            if (need_timeout) {
                if (timeout_interval >= 3000000) {
                    break;
                }
                apr_sleep(timeout_interval);
                timeout_interval *= 2;
            }
        } while (need_timeout);
    }





    for (pc = procs; pc; pc = pc->next) {
        if (pc->kill_how == APR_KILL_AFTER_TIMEOUT)
            apr_proc_kill(pc->proc, 9);
    }


    for (pc = procs; pc; pc = pc->next) {
        if (pc->kill_how != APR_KILL_NEVER)
            (void)apr_proc_wait(pc->proc, ((void*)0), ((void*)0), APR_WAIT);
    }
}
# 2530 "./../memory/unix/apr_pools.c"
void * apr_palloc_debug(apr_pool_t *pool, apr_size_t size,
                                     const char *file_line)
{
    return apr_palloc(pool, size);
}

void * apr_pcalloc_debug(apr_pool_t *pool, apr_size_t size,
                                      const char *file_line)
{
    return apr_pcalloc(pool, size);
}

void apr_pool_clear_debug(apr_pool_t *pool,
                                       const char *file_line)
{
    apr_pool_clear(pool);
}

void apr_pool_destroy_debug(apr_pool_t *pool,
                                         const char *file_line)
{
    apr_pool_destroy(pool);
}

apr_status_t apr_pool_create_ex_debug(apr_pool_t **newpool,
                                                   apr_pool_t *parent,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator,
                                                   const char *file_line)
{
    return apr_pool_create_ex(newpool, parent, abort_fn, allocator);
}

apr_status_t apr_pool_create_core_ex_debug(apr_pool_t **newpool,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator,
                                                   const char *file_line)
{
    return apr_pool_create_unmanaged_ex(newpool, abort_fn, allocator);
}

apr_status_t apr_pool_create_unmanaged_ex_debug(apr_pool_t **newpool,
                                                   apr_abortfunc_t abort_fn,
                                                   apr_allocator_t *allocator,
                                                   const char *file_line)
{
    return apr_pool_create_unmanaged_ex(newpool, abort_fn, allocator);
}
# 14 "main2.c" 2
# 1 "./../misc/unix/start.c" 1
# 20 "./../misc/unix/start.c"
# 1 "../include/apr_signal.h" 1
# 57 "../include/apr_signal.h"
typedef void apr_sigfunc_t(int);






apr_sigfunc_t * apr_signal(int signo, apr_sigfunc_t * func);
# 80 "../include/apr_signal.h"
const char * apr_signal_description_get(int signum);






void apr_signal_init(apr_pool_t *pglobal);






apr_status_t apr_signal_block(int signum);






apr_status_t apr_signal_unblock(int signum);
# 21 "./../misc/unix/start.c" 2



# 1 "../include/arch/unix/apr_arch_internal_time.h" 1
# 22 "../include/arch/unix/apr_arch_internal_time.h"
void apr_unix_setup_time(void);
# 25 "./../misc/unix/start.c" 2


apr_status_t apr_app_initialize(int *argc,
                                             const char * const * *argv,
                                             const char * const * *env)
{





    return apr_initialize();
}

static int initialized = 0;

apr_status_t apr_initialize(void)
{
    apr_pool_t *pool;
    apr_status_t status;

    if (initialized++) {
        return 0;
    }


    apr_proc_mutex_unix_setup_lock();
    apr_unix_setup_time();


    if ((status = apr_pool_initialize()) != 0)
        return status;

    if (apr_pool_create_ex(&pool, ((void*)0), ((void*)0), ((void*)0)) != 0) {
        return (20000 + 2);
    }

    apr_pool_tag(pool, "apr_initialize");
# 71 "./../misc/unix/start.c"
    apr_signal_init(pool);

    return 0;
}

void apr_terminate(void)
{
    initialized--;
    if (initialized) {
        return;
    }
    apr_pool_terminate();

}

void apr_terminate2(void)
{
    apr_terminate();
}
# 15 "main2.c" 2
# 1 "./../time/unix/time.c" 1
# 43 "./../time/unix/time.c"
static apr_int32_t get_offset(struct tm *tm)
{

    return tm->tm_gmtoff;
# 64 "./../time/unix/time.c"
}

apr_status_t apr_time_ansi_put(apr_time_t *result,
                                            time_t input)
{
    *result = (apr_time_t)input * 1000000L;
    return 0;
}


apr_time_t apr_time_now(void)
{
    struct timeval tv;
    gettimeofday(&tv, ((void*)0));
    return tv.tv_sec * 1000000L + tv.tv_usec;
}

static void explode_time(apr_time_exp_t *xt, apr_time_t t,
                         apr_int32_t offset, int use_localtime)
{
    struct tm tm;
    time_t tt = (t / 1000000L) + offset;
    xt->tm_usec = t % 1000000L;


    if (use_localtime)
        localtime_r(&tt, &tm);
    else
        gmtime_r(&tt, &tm);







    xt->tm_sec = tm.tm_sec;
    xt->tm_min = tm.tm_min;
    xt->tm_hour = tm.tm_hour;
    xt->tm_mday = tm.tm_mday;
    xt->tm_mon = tm.tm_mon;
    xt->tm_year = tm.tm_year;
    xt->tm_wday = tm.tm_wday;
    xt->tm_yday = tm.tm_yday;
    xt->tm_isdst = tm.tm_isdst;
    xt->tm_gmtoff = get_offset(&tm);
}

apr_status_t apr_time_exp_tz(apr_time_exp_t *result,
                                          apr_time_t input, apr_int32_t offs)
{
    explode_time(result, input, offs, 0);
    result->tm_gmtoff = offs;
    return 0;
}

apr_status_t apr_time_exp_gmt(apr_time_exp_t *result,
                                           apr_time_t input)
{
    return apr_time_exp_tz(result, input, 0);
}

apr_status_t apr_time_exp_lt(apr_time_exp_t *result,
                                                apr_time_t input)
{




    explode_time(result, input, 0, 1);
    return 0;

}

apr_status_t apr_time_exp_get(apr_time_t *t, apr_time_exp_t *xt)
{
    apr_time_t year = xt->tm_year;
    apr_time_t days;
    static const int dayoffset[12] =
    {306, 337, 0, 31, 61, 92, 122, 153, 184, 214, 245, 275};



    if (xt->tm_mon < 2)
        year--;



    days = year * 365 + year / 4 - year / 100 + (year / 100 + 3) / 4;
    days += dayoffset[xt->tm_mon] + xt->tm_mday - 1;
    days -= 25508;
    days = ((days * 24 + xt->tm_hour) * 60 + xt->tm_min) * 60 + xt->tm_sec;

    if (days < 0) {
        return (20000 + 4);
    }
    *t = days * 1000000L + xt->tm_usec;
    return 0;
}

apr_status_t apr_time_exp_gmt_get(apr_time_t *t,
                                               apr_time_exp_t *xt)
{
    apr_status_t status = apr_time_exp_get(t, xt);
    if (status == 0)
        *t -= (apr_time_t) xt->tm_gmtoff * 1000000L;
    return status;
}

apr_status_t apr_os_imp_time_get(apr_os_imp_time_t **ostime,
                                              apr_time_t *aprtime)
{
    (*ostime)->tv_usec = *aprtime % 1000000L;
    (*ostime)->tv_sec = *aprtime / 1000000L;
    return 0;
}

apr_status_t apr_os_exp_time_get(apr_os_exp_time_t **ostime,
                                              apr_time_exp_t *aprtime)
{
    (*ostime)->tm_sec = aprtime->tm_sec;
    (*ostime)->tm_min = aprtime->tm_min;
    (*ostime)->tm_hour = aprtime->tm_hour;
    (*ostime)->tm_mday = aprtime->tm_mday;
    (*ostime)->tm_mon = aprtime->tm_mon;
    (*ostime)->tm_year = aprtime->tm_year;
    (*ostime)->tm_wday = aprtime->tm_wday;
    (*ostime)->tm_yday = aprtime->tm_yday;
    (*ostime)->tm_isdst = aprtime->tm_isdst;


    (*ostime)->tm_gmtoff = aprtime->tm_gmtoff;




    return 0;
}

apr_status_t apr_os_imp_time_put(apr_time_t *aprtime,
                                              apr_os_imp_time_t **ostime,
                                              apr_pool_t *cont)
{
    *aprtime = (*ostime)->tv_sec * 1000000L + (*ostime)->tv_usec;
    return 0;
}

apr_status_t apr_os_exp_time_put(apr_time_exp_t *aprtime,
                                              apr_os_exp_time_t **ostime,
                                              apr_pool_t *cont)
{
    aprtime->tm_sec = (*ostime)->tm_sec;
    aprtime->tm_min = (*ostime)->tm_min;
    aprtime->tm_hour = (*ostime)->tm_hour;
    aprtime->tm_mday = (*ostime)->tm_mday;
    aprtime->tm_mon = (*ostime)->tm_mon;
    aprtime->tm_year = (*ostime)->tm_year;
    aprtime->tm_wday = (*ostime)->tm_wday;
    aprtime->tm_yday = (*ostime)->tm_yday;
    aprtime->tm_isdst = (*ostime)->tm_isdst;


    aprtime->tm_gmtoff = (*ostime)->tm_gmtoff;




    return 0;
}

void apr_sleep(apr_interval_time_t t)
{







    struct timeval tv;
    tv.tv_usec = t % 1000000L;
    tv.tv_sec = t / 1000000L;
    select(0, ((void*)0), ((void*)0), ((void*)0), &tv);

}
# 296 "./../time/unix/time.c"
void apr_unix_setup_time(void)
{
# 339 "./../time/unix/time.c"
}




void apr_time_clock_hires(apr_pool_t *p)
{
    return;
}
# 16 "main2.c" 2
# 1 "./../atomic/unix/builtins.c" 1
# 17 "./../atomic/unix/builtins.c"
# 1 "../include/arch/unix/apr_arch_atomic.h" 1
# 18 "./../atomic/unix/builtins.c" 2



apr_status_t apr_atomic_init(apr_pool_t *p)
{
    return 0;
}

apr_uint32_t apr_atomic_read32(volatile apr_uint32_t *mem)
{
    return *mem;
}

void apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    *mem = val;
}

apr_uint32_t apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    return __sync_fetch_and_add(mem, val);
}

void apr_atomic_sub32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    __sync_fetch_and_sub(mem, val);
}

apr_uint32_t apr_atomic_inc32(volatile apr_uint32_t *mem)
{
    return __sync_fetch_and_add(mem, 1);
}

int apr_atomic_dec32(volatile apr_uint32_t *mem)
{
    return __sync_sub_and_fetch(mem, 1);
}

apr_uint32_t apr_atomic_cas32(volatile apr_uint32_t *mem, apr_uint32_t with,
                                           apr_uint32_t cmp)
{
    return __sync_val_compare_and_swap(mem, cmp, with);
}

apr_uint32_t apr_atomic_xchg32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    __sync_synchronize();

    return __sync_lock_test_and_set(mem, val);
}

void* apr_atomic_casptr(volatile void **mem, void *with, const void *cmp)
{
    return (void*) __sync_val_compare_and_swap(mem, cmp, with);
}

void* apr_atomic_xchgptr(volatile void **mem, void *with)
{
    __sync_synchronize();

    return (void*) __sync_lock_test_and_set(mem, with);
}
# 17 "main2.c" 2
# 1 "./../threadproc/unix/signals.c" 1
# 18 "./../threadproc/unix/signals.c"
# 1 "../include/arch/unix/apr_arch_threadproc.h" 1
# 55 "../include/arch/unix/apr_arch_threadproc.h"
struct apr_thread_t {
    apr_pool_t *pool;
    pthread_t *td;
    void *data;
    apr_thread_start_t func;
    apr_status_t exitval;
};

struct apr_threadattr_t {
    apr_pool_t *pool;
    pthread_attr_t attr;
};

struct apr_threadkey_t {
    apr_pool_t *pool;
    pthread_key_t key;
};

struct apr_thread_once_t {
    pthread_once_t once;
};



struct apr_procattr_t {
    apr_pool_t *pool;
    apr_file_t *parent_in;
    apr_file_t *child_in;
    apr_file_t *parent_out;
    apr_file_t *child_out;
    apr_file_t *parent_err;
    apr_file_t *child_err;
    char *currdir;
    apr_int32_t cmdtype;
    apr_int32_t detached;

    struct rlimit *limit_cpu;


    struct rlimit *limit_mem;


    struct rlimit *limit_nproc;


    struct rlimit *limit_nofile;

    apr_child_errfn_t *errfn;
    apr_int32_t errchk;
    apr_uid_t uid;
    apr_gid_t gid;
};
# 19 "./../threadproc/unix/signals.c" 2





# 1 "/usr/include/assert.h" 1 3 4
# 69 "/usr/include/assert.h" 3 4
extern void __assert_fail (const char *__assertion, const char *__file,
      unsigned int __line, const char *__function)
     __attribute__ ((__nothrow__ )) __attribute__ ((__noreturn__));


extern void __assert_perror_fail (int __errnum, const char *__file,
      unsigned int __line, const char *__function)
     __attribute__ ((__nothrow__ )) __attribute__ ((__noreturn__));




extern void __assert (const char *__assertion, const char *__file, int __line)
     __attribute__ ((__nothrow__ )) __attribute__ ((__noreturn__));
# 25 "./../threadproc/unix/signals.c" 2
# 35 "./../threadproc/unix/signals.c"
apr_status_t apr_proc_kill(apr_proc_t *proc, int signum)
{
# 48 "./../threadproc/unix/signals.c"
    if (kill(proc->pid, signum) == -1) {
        return (*__errno_location ());
    }

    return 0;
}
# 74 "./../threadproc/unix/signals.c"
apr_sigfunc_t * apr_signal(int signo, apr_sigfunc_t * func)
{
    struct sigaction act, oact;

    act.__sigaction_handler.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    act.sa_flags |= 0x20000000;
# 103 "./../threadproc/unix/signals.c"
    if (sigaction(signo, &act, &oact) < 0)
        return ((__sighandler_t) -1);
    return oact.__sigaction_handler.sa_handler;
}







void apr_signal_init(apr_pool_t *pglobal)
{
}
const char *apr_signal_description_get(int signum)
{
    return (signum >= 0) ? sys_siglist[signum] : "unknown signal (number)";
}
# 274 "./../threadproc/unix/signals.c"
static void remove_sync_sigs(sigset_t *sig_mask)
{

    sigdelset(sig_mask, 6);


    sigdelset(sig_mask, 7);





    sigdelset(sig_mask, 8);


    sigdelset(sig_mask, 4);


    sigdelset(sig_mask, 6);


    sigdelset(sig_mask, 13);


    sigdelset(sig_mask, 11);


    sigdelset(sig_mask, 31);


    sigdelset(sig_mask, 5);
# 315 "./../threadproc/unix/signals.c"
    sigdelset(sig_mask, 12);

}

apr_status_t apr_signal_thread(int(*signal_handler)(int signum))
{
    sigset_t sig_mask;

    int (*sig_func)(int signum) = (int (*)(int))signal_handler;



    sigfillset(&sig_mask);






    sigdelset(&sig_mask, 9);


    sigdelset(&sig_mask, 19);


    sigdelset(&sig_mask, 18);






    remove_sync_sigs(&sig_mask);
# 379 "./../threadproc/unix/signals.c"
    while (1) {

        int signal_received;

        if (sigwait((&sig_mask),(&signal_received)) != 0)
        {

        }

        if (sig_func(signal_received) == 1) {
            return 0;
        }





    }
}

apr_status_t apr_setup_signal_thread(void)
{
    sigset_t sig_mask;
    int rv;
# 416 "./../threadproc/unix/signals.c"
    sigfillset(&sig_mask);
    remove_sync_sigs(&sig_mask);






    if ((rv = pthread_sigmask(2, &sig_mask, ((void*)0))) != 0) {



    }

    return rv;
}



apr_status_t apr_signal_block(int signum)
{

    sigset_t sig_mask;
    int rv;

    sigemptyset(&sig_mask);

    sigaddset(&sig_mask, signum);






    if ((rv = pthread_sigmask(0, &sig_mask, ((void*)0))) != 0) {



    }

    return rv;



}

apr_status_t apr_signal_unblock(int signum)
{

    sigset_t sig_mask;
    int rv;

    sigemptyset(&sig_mask);

    sigaddset(&sig_mask, signum);






    if ((rv = pthread_sigmask(1, &sig_mask, ((void*)0))) != 0) {



    }

    return rv;



}
# 18 "main2.c" 2
# 1 "./../threadproc/unix/thread.c" 1
# 19 "./../threadproc/unix/thread.c"
# 1 "../include/arch/unix/apr_arch_threadproc.h" 1
# 20 "./../threadproc/unix/thread.c" 2






static apr_status_t threadattr_cleanup(void *data)
{
    apr_threadattr_t *attr = data;
    apr_status_t rv;

    rv = pthread_attr_destroy(&attr->attr);





    return rv;
}

apr_status_t apr_threadattr_create(apr_threadattr_t **new,
                                                apr_pool_t *pool)
{
    apr_status_t stat;

    (*new) = apr_palloc(pool, sizeof(apr_threadattr_t));
    (*new)->pool = pool;
    stat = pthread_attr_init(&(*new)->attr);

    if (stat == 0) {
        apr_pool_cleanup_register(pool, *new, threadattr_cleanup,
                                  apr_pool_cleanup_null);
        return 0;
    }




    return stat;
}







apr_status_t apr_threadattr_detach_set(apr_threadattr_t *attr,
                                                    apr_int32_t on)
{
    apr_status_t stat;





    if ((stat = pthread_attr_setdetachstate(&attr->attr,
                                            ((on) ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE))) == 0) {

        return 0;
    }
    else {




        return stat;
    }
}

apr_status_t apr_threadattr_detach_get(apr_threadattr_t *attr)
{
    int state;




    pthread_attr_getdetachstate(&attr->attr, &state);

    if (state == ((1) ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE))
        return ((20000 + 50000) + 3);
    return ((20000 + 50000) + 4);
}

apr_status_t apr_threadattr_stacksize_set(apr_threadattr_t *attr,
                                                       apr_size_t stacksize)
{
    int stat;

    stat = pthread_attr_setstacksize(&attr->attr, stacksize);
    if (stat == 0) {
        return 0;
    }




    return stat;
}

apr_status_t apr_threadattr_guardsize_set(apr_threadattr_t *attr,
                                                       apr_size_t size)
{

    apr_status_t rv;

    rv = pthread_attr_setguardsize(&attr->attr, size);
    if (rv == 0) {
        return 0;
    }



    return rv;



}

static void *dummy_worker(void *opaque)
{
    apr_thread_t *thread = (apr_thread_t*)opaque;
    return thread->func(thread, thread->data);
}

apr_status_t apr_thread_create(apr_thread_t **new,
                                            apr_threadattr_t *attr,
                                            apr_thread_start_t func,
                                            void *data,
                                            apr_pool_t *pool)
{
    apr_status_t stat;
    pthread_attr_t *temp;

    (*new) = (apr_thread_t *)apr_pcalloc(pool, sizeof(apr_thread_t));

    if ((*new) == ((void*)0)) {
        return 12;
    }

    (*new)->td = (pthread_t *)apr_pcalloc(pool, sizeof(pthread_t));

    if ((*new)->td == ((void*)0)) {
        return 12;
    }

    (*new)->data = data;
    (*new)->func = func;

    if (attr)
        temp = &attr->attr;
    else
        temp = ((void*)0);

    stat = apr_pool_create_ex(&(*new)->pool, pool, ((void*)0), ((void*)0));
    if (stat != 0) {
        return stat;
    }

    if ((stat = pthread_create((*new)->td, temp, dummy_worker, (*new))) == 0) {
        return 0;
    }
    else {




        return stat;
    }
}

apr_os_thread_t apr_os_thread_current(void)
{
    return pthread_self();
}

int apr_os_thread_equal(apr_os_thread_t tid1,
                                     apr_os_thread_t tid2)
{
    return pthread_equal(tid1, tid2);
}

apr_status_t apr_thread_exit(apr_thread_t *thd,
                                          apr_status_t retval)
{
    thd->exitval = retval;
    apr_pool_destroy(thd->pool);
    pthread_exit(((void*)0));
    return 0;
}

apr_status_t apr_thread_join(apr_status_t *retval,
                                          apr_thread_t *thd)
{
    apr_status_t stat;
    apr_status_t *thread_stat;

    if ((stat = pthread_join(*thd->td,(void *)&thread_stat)) == 0) {
        *retval = thd->exitval;
        return 0;
    }
    else {




        return stat;
    }
}

apr_status_t apr_thread_detach(apr_thread_t *thd)
{
    apr_status_t stat;




    if ((stat = pthread_detach(*thd->td)) == 0) {


        return 0;
    }
    else {




        return stat;
    }
}

void apr_thread_yield(void)
{




    pthread_yield();






}

apr_status_t apr_thread_data_get(void **data, const char *key,
                                              apr_thread_t *thread)
{
    return apr_pool_userdata_get(data, key, thread->pool);
}

apr_status_t apr_thread_data_set(void *data, const char *key,
                              apr_status_t (*cleanup)(void *),
                              apr_thread_t *thread)
{
    return apr_pool_userdata_set(data, key, cleanup, thread->pool);
}

apr_status_t apr_os_thread_get(apr_os_thread_t **thethd,
                                            apr_thread_t *thd)
{
    *thethd = thd->td;
    return 0;
}

apr_status_t apr_os_thread_put(apr_thread_t **thd,
                                            apr_os_thread_t *thethd,
                                            apr_pool_t *pool)
{
    if (pool == ((void*)0)) {
        return (20000 + 2);
    }

    if ((*thd) == ((void*)0)) {
        (*thd) = (apr_thread_t *)apr_pcalloc(pool, sizeof(apr_thread_t));
        (*thd)->pool = pool;
    }

    (*thd)->td = thethd;
    return 0;
}

apr_status_t apr_thread_once_init(apr_thread_once_t **control,
                                               apr_pool_t *p)
{
    static const pthread_once_t once_init = 0;

    *control = apr_palloc(p, sizeof(**control));
    (*control)->once = once_init;
    return 0;
}

apr_status_t apr_thread_once(apr_thread_once_t *control,
                                          void (*func)(void))
{
    return pthread_once(&control->once, func);
}

apr_pool_t * apr_thread_pool_get (const apr_thread_t *thethread) { return thethread->pool; }
# 19 "main2.c" 2
# 1 "./../test/abts.c" 1
# 17 "./../test/abts.c"
# 1 "./../test/abts.h" 1
# 18 "./../test/abts.c" 2
# 1 "./../test/abts_tests.h" 1
# 20 "./../test/abts_tests.h"
# 1 "./../test/abts.h" 1
# 21 "./../test/abts_tests.h" 2
# 1 "./../test/testutil.h" 1
# 19 "./../test/testutil.h"
# 1 "./../test/abts.h" 1
# 20 "./../test/testutil.h" 2
# 22 "./../test/abts_tests.h" 2

const struct testlist {
    abts_suite *(*func)(abts_suite *suite);
} alltests[] = {
    {testatomic},
    {testdir},
    {testdso},
    {testdup},
    {testenv},
    {testescape},
    {testfile},
    {testfilecopy},
    {testfileinfo},
    {testflock},
    {testfmt},
    {testfnmatch},
    {testgetopt},



    {testhash},
    {testipsub},
    {testlock},
    {testcond},
    {testlfs},
    {testmmap},
    {testnames},
    {testoc},
    {testpath},
    {testpipe},
    {testpoll},
    {testpool},
    {testproc},
    {testprocmutex},
    {testrand},
    {testsleep},
    {testshm},
    {testsock},
    {testsockets},
    {testsockopt},
    {teststr},
    {teststrnatcmp},
    {testtable},
    {testtemp},
    {testthread},
    {testtime},
    {testud},
    {testuser},
    {testvsn}
};
# 19 "./../test/abts.c" 2
# 1 "./../test/testutil.h" 1
# 19 "./../test/testutil.h"
# 1 "./../test/abts.h" 1
# 20 "./../test/testutil.h" 2
# 20 "./../test/abts.c" 2


static char status[6] = {'|', '/', '-', '|', '\\', '-'};
static int curr_char;
static int verbose = 0;
static int exclude = 0;
static int quiet = 0;
static int list_tests = 0;

const char **testlist = ((void*)0);

static int find_test_name(const char *testname) {
    int i;
    for (i = 0; testlist[i] != ((void*)0); i++) {
        if (!strcmp(testlist[i], testname)) {
            return 1;
        }
    }
    return 0;
}


static int should_test_run(const char *testname) {
    int found = 0;
    if (list_tests == 1) {
        return 0;
    }
    if (testlist == ((void*)0)) {
        return 1;
    }
    found = find_test_name(testname);
    if ((found && !exclude) || (!found && exclude)) {
        return 1;
    }
    return 0;
}

static void reset_status(void)
{
    curr_char = 0;
}

static void update_status(void)
{
    if (!quiet) {
        curr_char = (curr_char + 1) % 6;
        fprintf(stdout, "\b%c", status[curr_char]);
        fflush(stdout);
    }
}

static void end_suite(abts_suite *suite)
{
    if (suite != ((void*)0)) {
        sub_suite *last = suite->tail;
        if (!quiet) {
            fprintf(stdout, "\b");
            fflush(stdout);
        }
        if (last->failed == 0) {
            fprintf(stdout, "SUCCESS\n");
            fflush(stdout);
        }
        else {
            fprintf(stdout, "FAILED %d of %d\n", last->failed, last->num_test);
            fflush(stdout);
        }
    }
}

abts_suite *abts_add_suite(abts_suite *suite, const char *suite_name_full)
{
    sub_suite *subsuite;
    char *p;
    const char *suite_name;
    curr_char = 0;


    if (suite && suite->tail &&!suite->tail->not_run) {
        end_suite(suite);
    }

    subsuite = malloc(sizeof(*subsuite));
    subsuite->num_test = 0;
    subsuite->failed = 0;
    subsuite->next = ((void*)0);


    suite_name = strrchr(suite_name_full, '/');
    if (!suite_name) {
        suite_name = strrchr(suite_name_full, '\\');
    }
    if (suite_name) {
        suite_name++;
    } else {
        suite_name = suite_name_full;
    }
    p = strrchr(suite_name, '.');
    if (p) {
        subsuite->name = memcpy(calloc(p - suite_name + 1, 1),
                                suite_name, p - suite_name);
    }
    else {
        subsuite->name = suite_name;
    }

    if (list_tests) {
        fprintf(stdout, "%s\n", subsuite->name);
    }

    subsuite->not_run = 0;

    if (suite == ((void*)0)) {
        suite = malloc(sizeof(*suite));
        suite->head = subsuite;
        suite->tail = subsuite;
    }
    else {
        suite->tail->next = subsuite;
        suite->tail = subsuite;
    }

    if (!should_test_run(subsuite->name)) {
        subsuite->not_run = 1;
        return suite;
    }

    reset_status();
    fprintf(stdout, "%-20s:  ", subsuite->name);
    update_status();
    fflush(stdout);

    return suite;
}

void abts_run_test(abts_suite *ts, test_func f, void *value)
{
    abts_case tc;
    sub_suite *ss;

    if (!should_test_run(ts->tail->name)) {
        return;
    }
    ss = ts->tail;

    tc.failed = 0;
    tc.suite = ss;

    ss->num_test++;
    update_status();

    f(&tc, value);

    if (tc.failed) {
        ss->failed++;
    }
}

static int report(abts_suite *suite)
{
    int count = 0;
    sub_suite *dptr;

    if (suite && suite->tail &&!suite->tail->not_run) {
        end_suite(suite);
    }

    for (dptr = suite->head; dptr; dptr = dptr->next) {
        count += dptr->failed;
    }

    if (list_tests) {
        return 0;
    }

    if (count == 0) {
        printf("All tests passed.\n");
        return 0;
    }

    dptr = suite->head;
    fprintf(stdout, "%-15s\t\tTotal\tFail\tFailed %%\n", "Failed Tests");
    fprintf(stdout, "===================================================\n");
    while (dptr != ((void*)0)) {
        if (dptr->failed != 0) {
            float percent = ((float)dptr->failed / (float)dptr->num_test);
            fprintf(stdout, "%-15s\t\t%5d\t%4d\t%6.2f%%\n", dptr->name,
                    dptr->num_test, dptr->failed, percent * 100);
        }
        dptr = dptr->next;
    }
    return 1;
}

void abts_log_message(const char *fmt, ...)
{
    va_list args;
    update_status();

    if (verbose) {
        __builtin_va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        __builtin_va_end(args);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
}

void abts_int_equal(abts_case *tc, const int expected, const int actual, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (expected == actual) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: expected <%d>, but saw <%d>\n", lineno, expected, actual);
        fflush(stderr);
    }
}

void abts_int_nequal(abts_case *tc, const int expected, const int actual, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (expected != actual) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: expected something other than <%d>, but saw <%d>\n",
                lineno, expected, actual);
        fflush(stderr);
    }
}

void abts_size_equal(abts_case *tc, size_t expected, size_t actual, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (expected == actual) return;

    tc->failed = 1;
    if (verbose) {

        fprintf(stderr, "Line %d: expected %lu, but saw %lu\n", lineno,
                (unsigned long)expected, (unsigned long)actual);
        fflush(stderr);
    }
}

void abts_str_equal(abts_case *tc, const char *expected, const char *actual, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (!expected && !actual) return;
    if (expected && actual)
        if (!strcmp(expected, actual)) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: expected <%s>, but saw <%s>\n", lineno, expected, actual);
        fflush(stderr);
    }
}

void abts_str_nequal(abts_case *tc, const char *expected, const char *actual,
                       size_t n, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (!strncmp(expected, actual, n)) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: expected something other than <%s>, but saw <%s>\n",
                lineno, expected, actual);
        fflush(stderr);
    }
}

void abts_ptr_notnull(abts_case *tc, const void *ptr, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (ptr != ((void*)0)) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: expected non-NULL, but saw NULL\n", lineno);
        fflush(stderr);
    }
}

void abts_ptr_equal(abts_case *tc, const void *expected, const void *actual, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (expected == actual) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: expected <%p>, but saw <%p>\n", lineno, expected, actual);
        fflush(stderr);
    }
}

void abts_fail(abts_case *tc, const char *message, int lineno)
{
    update_status();
    if (tc->failed) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: %s\n", lineno, message);
        fflush(stderr);
    }
}

void abts_assert(abts_case *tc, const char *message, int condition, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (condition) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: %s\n", lineno, message);
        fflush(stderr);
    }
}

void abts_true(abts_case *tc, int condition, int lineno)
{
    update_status();
    if (tc->failed) return;

    if (condition) return;

    tc->failed = 1;
    if (verbose) {
        fprintf(stderr, "Line %d: Condition is false, but expected true\n", lineno);
        fflush(stderr);
    }
}

void abts_not_impl(abts_case *tc, const char *message, int lineno)
{
    update_status();

    tc->suite->not_impl++;
    if (verbose) {
        fprintf(stderr, "Line %d: %s\n", lineno, message);
        fflush(stderr);
    }
}
# 20 "main2.c" 2






int fflush(FILE *stream){
  return 0;
}

int fprintf(FILE *stream, const char *format, ...){
  return 0;
}

void test_atomics_threaded_2(abts_case *tc, void *data)
{
    apr_thread_t *t2[2];
    apr_status_t rv;
    int i;





    rv = apr_thread_mutex_create(&thread_lock, 0x0, p);
    (((rv == 0) && "Could not create lock") ? (void) (0) : __assert_fail ("(rv == 0) && \"Could not create lock\"", "main2.c", 45, __PRETTY_FUNCTION__));

    for (i = 0; i < 2; i++) {
        apr_status_t r2;
        r2 = apr_thread_create(&t2[i], ((void*)0), thread_func_atomic, ((void*)0), p);
        ((!r2 && "Failed creating threads") ? (void) (0) : __assert_fail ("!r2 && \"Failed creating threads\"", "main2.c", 50, __PRETTY_FUNCTION__));
    }

    for (i = 0; i < 2; i++) {
        apr_status_t s2;
        apr_thread_join(&s2, t2[i]);

        ((s2 == exit_ret_val && "Invalid return value from thread_join") ? (void) (0) : __assert_fail ("s2 == exit_ret_val && \"Invalid return value from thread_join\"", "main2.c", 58, __PRETTY_FUNCTION__));

    }

    abts_int_equal(tc, 2 * 1, apr_atomic_read32(&atomic_ops), 62);


    rv = apr_thread_mutex_destroy(thread_lock);
    ((rv == 0 && "Failed creating threads") ? (void) (0) : __assert_fail ("rv == 0 && \"Failed creating threads\"", "main2.c", 65, __PRETTY_FUNCTION__));
}

int main(int argc, char *argv[]){

  initialize();

  abts_case tc;
  tc.failed = 0;
  tc.suite = ((void*)0);
  test_atomics_threaded_2(&tc,((void*)0));

  return 0;
}
