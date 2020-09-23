#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <errno.h>
#include <pthread.h>
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
extern __off_t __VERIFIER_lseekFS (int __fd, __off_t __offset, int __whence);
#define lseek __VERIFIER_lseekFS

/* Close the file descriptor FD.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int __VERIFIER_closeFS (int __fd);
#define close __VERIFIER_closeFS

/* Read NBYTES into BUF from FD.  Return the
   number read, -1 for errors or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t __VERIFIER_readFS (int __fd, void *__buf, size_t __nbytes);
#define read __VERIFIER_readFS

/* Write N bytes of BUF to FD.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t __VERIFIER_writeFS (int __fd, const void *__buf, size_t __n);
#define write __VERIFIER_writeFS

/* Read NBYTES into BUF from FD at the given position OFFSET without
   changing the file pointer.  Return the number read, -1 for errors
   or 0 for EOF.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t __VERIFIER_preadFS (int __fd, void *__buf, size_t __nbytes,
				   __off_t __offset);
#define pread __VERIFIER_preadFS

/* Write N bytes of BUF to FD at the given position OFFSET without
   changing the file pointer.  Return the number written, or -1.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern ssize_t __VERIFIER_pwriteFS (int __fd, const void *__buf, size_t __n,
				    __off_t __offset);
#define pwrite __VERIFIER_pwriteFS

/* Make a link to FROM named TO.  */
extern int __VERIFIER_linkFS (const char *__from, const char *__to);
#define link __VERIFIER_linkFS

/* Remove the link NAME.  */
extern int __VERIFIER_unlinkFS (const char *__name);
#define unlink __VERIFIER_unlinkFS

/* Make all changes done to FD actually appear on disk.
   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int __VERIFIER_fsyncFS (int __fd);
#define fsync __VERIFIER_fsyncFS

/* Make all changes done to all files actually appear on disk.  */
extern void __VERIFIER_syncFS (void);
#define sync __VERIFIER_syncFS

/* Truncate FILE to LENGTH bytes.  */
extern int __VERIFIER_truncateFS (const char *__file, __off_t __length);
#define truncate __VERIFIER_truncateFS


/*
 * ******** GENMC RESERVED NAMESPACE ********
 */

#ifndef __CONFIG_GENMC_INODE_DATA_SIZE
# error "Internal error: inode size not defined!"
#endif

struct __genmc_inode {
	/* VFS */
	pthread_mutex_t lock; // setupFsInfo() + interp rely on the layout
	int i_size; // <-- need to treat this atomically

	/* journaling helpers (embedded avoid indirection) */
	int i_transaction; // <-- writes protected by the lock above

	/* ext4 disk data (embedded to avoid indirection)
	 * The implementation can now accommodate only one metadata piece
	 * due to the current metadata block mapping */
	int i_disksize; // <-- used as metadata block mapping
	char data[__CONFIG_GENMC_INODE_DATA_SIZE]; // <-- used as data block mapping
};

struct __genmc_file {
	struct inode *inode;
	unsigned int count; // need to manipulate atomically
	unsigned int flags; // encompasses f_mode
	pthread_mutex_t pos_lock;
	int pos;
};

struct __genmc_inode __attribute((address_space(42))) __genmc_dir_inode;
struct __genmc_file __attribute((address_space(42))) __genmc_dummy_file;

#endif /* __UNISTD_H__ */
