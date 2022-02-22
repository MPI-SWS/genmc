#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

/* src/Execution.cpp is based on the values of the definitions below */
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */
/* #define SEEK_DATA	3	/\* Seek to next data.  *\/ */
/* #define SEEK_HOLE	4	/\* Seek to next hole.  *\/ */

/* Move FD's file position to OFFSET bytes from the
   beginning of the file (if WHENCE is SEEK_SET),
   the current position (if WHENCE is SEEK_CUR),
   or the end of the file (if WHENCE is SEEK_END).
   Return the new file position.  */
#define lseek(fd, offset, whence) __VERIFIER_lseekFS(fd, offset, whence)

/* Close the file descriptor FD.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#define close(fd) __VERIFIER_closeFS(fd)

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#define read(fd, buf, nbytes) __VERIFIER_readFS(fd, buf, nbytes)

/* Write N bytes of BUF to FD.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#define write(fd, buf, n) __VERIFIER_writeFS(fd, buf, n)

/* Read NBYTES into BUF from FD at the given position OFFSET without
   changing the file pointer.  Return the number read, -1 for errors
   or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#define pread(fd, buf, nbytes, offset) __VERIFIER_preadFS(fd, buf, nbytes, offset)

/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
#define pwrite(fd, buf, n, offset) __VERIFIER_pwriteFS(fd, buf, n, offset)

/* Make a link to FROM named TO.  */
#define link(from, to) __VERIFIER_linkFS(from, to)

/* Remove the link NAME.  */
#define unlink(name) __VERIFIER_unlinkFS(name)

/* Make all changes done to FD actually appear on disk.
   This function is a cancellation point and therefore not marked with
   __THROW.  */
#define fsync(fd) __VERIFIER_fsyncFS(fd)

/* Make all changes done to all files actually appear on disk.  */
#define sync() __VERIFIER_syncFS()

/* Truncate FILE to LENGTH bytes.  */
#define truncate(file, length) __VERIFIER_truncateFS(file, length)

#ifdef __cplusplus
# error "FS functions temporarily disabled for C++ code!\n"
#endif

#endif /* __UNISTD_H__ */
