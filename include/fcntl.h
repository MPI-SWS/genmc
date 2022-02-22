#ifndef __FCNTL_H__
#define __FCNTL_H__

#include <bits/stat.h>
#include <unistd.h>
#include <genmc_internal.h>

/* Flags */

#define O_ACCMODE	00000003
#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002
#define O_CREAT		00000100
#define O_EXCL		00000200
#define O_NOCTTY	00000400
#define O_TRUNC		00001000
#define O_APPEND	00002000
#define O_NONBLOCK	00004000
#define O_SYNC	        04010000
#define O_DIRECT	00040000
#define O_LARGEFILE	00100000
#define O_DIRECTORY	00200000
#define O_NOFOLLOW	00400000
#define O_NOATIME	01000000
#define O_DSYNC		00010000
#define O_CLOEXEC	02000000

/* Protection bits */

#define S_IRUSR		__S_IREAD       /* Read by owner.  */
#define S_IWUSR		__S_IWRITE      /* Write by owner.  */
#define S_IXUSR		__S_IEXEC       /* Execute by owner.  */
/* Read, write, and execute by owner.  */
#define S_IRWXU		(__S_IREAD|__S_IWRITE|__S_IEXEC)
#define S_IRGRP		(S_IRUSR >> 3)  /* Read by group.  */
#define S_IWGRP		(S_IWUSR >> 3)  /* Write by group.  */
#define S_IXGRP		(S_IXUSR >> 3)  /* Execute by group.  */
/* Read, write, and execute by group.  */
#define S_IRWXG		(S_IRWXU >> 3)
#define S_IROTH		(S_IRGRP >> 3)  /* Read by others.  */
#define S_IWOTH		(S_IWGRP >> 3)  /* Write by others.  */
#define S_IXOTH		(S_IXGRP >> 3)  /* Execute by others.  */
/* Read, write, and execute by others.  */
#define S_IRWXO		(S_IRWXG >> 3)

#define open(file, oflag, mode) __VERIFIER_openFS(file, oflag, mode)

/* extern int creat (const char *__file, mode_t __mode); */
#define creat(file, mode) __VERIFIER_creatFS(file, mode)

#define rename(old, new) __VERIFIER_renameFS(old, new)

#endif /* __FCNTL_H__ */
