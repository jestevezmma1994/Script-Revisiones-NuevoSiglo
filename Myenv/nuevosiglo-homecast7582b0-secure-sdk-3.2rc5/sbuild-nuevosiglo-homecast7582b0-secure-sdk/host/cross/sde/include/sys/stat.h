/*
 * Copyright (c) 1996-2003 MIPS Technologies, Inc.  All rights reserved.
 * 
 * Unpublished rights (if any) reserved under the copyright laws of the
 * United States of America and other countries.
 * 
 * This code is proprietary to MIPS Technologies, Inc. ("MIPS
 * Technologies").  Any copying, reproducing, modifying or use of this
 * code (in whole or in part) that is not expressly permitted in writing
 * by MIPS Technologies or an authorized third party is strictly
 * prohibited.  At a minimum, this code is protected under unfair
 * competition and copyright laws.  Violations thereof may result in
 * criminal penalties and fines.
 * 
 * MIPS Technologies reserves the right to change this code to improve
 * function, design or otherwise.  MIPS Technologies does not assume any
 * liability arising out of the application or use of this code, or of
 * any error or omission in such code.  Any warranties, whether express,
 * statutory, implied or otherwise, including but not limited to the
 * implied warranties of merchantability or fitness for a particular
 * purpose, are excluded.  Except as expressly provided in any written
 * license agreement from MIPS Technologies or an authorized third party,
 * the furnishing of this code does not give recipient any license to any
 * intellectual property rights, including any patent rights, that cover
 * this code.
 * 
 * This code shall not be exported or transferred for the purpose of
 * reexporting in violation of any U.S. or non-U.S. regulation, treaty,
 * Executive Order, law, statute, amendment or supplement thereto.
 * 
 * This code constitutes one or more of the following: commercial
 * computer software, commercial computer software documentation or other
 * commercial items.  If the user of this code, or any related
 * documentation of any kind, including related technical data or
 * manuals, is an agency, department, or other entity of the United
 * States government ("Government"), the use, duplication, reproduction,
 * release, modification, disclosure, or transfer of this code, or any
 * related documentation of any kind, is restricted in accordance with
 * Federal Acquisition Regulation 12.212 for civilian agencies and
 * Defense Federal Acquisition Regulation Supplement 227.7202 for
 * military agencies.  The use of this code by the Government is further
 * restricted in accordance with the terms of the license agreement(s)
 * and/or applicable contract terms and conditions covering this code
 * from MIPS Technologies or an authorized third party.
 */

/*
 * stat.h : SDE file system stat definitions
 */


#ifndef __SYS_STAT_H
#ifdef __cplusplus
extern "C" {
#endif
#define __SYS_STAT_H

/* --- Required  --- */
#include <sys/types.h>

/* --- Constants --- */
#ifndef _POSIX_SOURCE
#define S_IFMT  	0170000		/* type of file */
#define S_IFIFO 	0010000		/* named pipe (FIFO) */
#define S_IFCHR 	0020000		/* character special */
#define S_IFDIR 	0040000  	/* directory */
#define S_IFBLK 	0060000		/* block special */
#define S_IFREG 	0100000		/* regular */
#define S_IFLNK 	0120000		/* a BSD symbolic link */
#define S_IFSOCK 	0140000		/* a BSD socket */
#endif

#define S_ISUID	04000		/* set user id on execution */
#define S_ISGID	02000		/* set group id on execution */
#ifndef _POSIX_SOURCE
#define S_ISVTX 01000		/* save swapped text even after use */
#define S_ISTXT 01000		/* save swapped text even after use */
#endif

#define S_IRWXU	00700		/* owner rwx	*/
#define S_IRUSR	00400		/* owner r	*/
#define S_IWUSR	00200		/* owner  w	*/
#define S_IXUSR	00100		/* owner   x	*/

#define S_IRWXG	00070		/* group rwx	*/
#define S_IRGRP	00040		/* group r	*/
#define S_IWGRP	00020		/* group  w	*/
#define S_IXGRP	00010		/* group   x	*/

#define S_IRWXO	00007		/* other rwx	*/
#define S_IROTH	00004		/* other r	*/
#define S_IWOTH	00002		/* other  w	*/
#define S_IXOTH	00001		/* other   x	*/

#ifndef _POSIX_SOURCE
/* --- Compatibility --- */
#define S_IREAD   S_IRUSR	/* read permission, owner */
#define S_IWRITE  S_IWUSR	/* write permission, owner */
#define S_IEXEC   S_IXUSR	/* execute/search permission, owner */
#endif

/* --- Structures --- */
struct stat {
    dev_t 	st_dev;
    ino_t	st_ino;
    mode_t	st_mode;
    nlink_t 	st_nlink;
    uid_t 	st_uid;
    gid_t 	st_gid;
    dev_t 	st_rdev;
    off_t 	st_size;
    size_t	st_blksize;	/* for I/O optim */
    time_t	st_atime;
    time_t 	st_mtime;
    time_t 	st_ctime;
};

/* --- Macros --- */
#define S_ISDIR(m)	(((m) & 0170000) == 0040000)
#define S_ISCHR(m)	(((m) & 0170000) == 0020000)
#define S_ISBLK(m)	(((m) & 0170000) == 0060000)
#define S_ISREG(m)	(((m) & 0170000) == 0100000)
#define S_ISFIFO(m)	(((m) & 0170000) == 0010000 || ((m) & 0170000) == 0140000)
#ifndef _POSIX_SOURCE
#define S_ISLNK(m)	(((m) & 0170000) == 0120000)
#define S_ISSOCK(m)	(((m) & 0170000) == 0140000)
#endif


#ifndef _KERNEL
/* --- Prototypes --- */
int	chmod (const char *, mode_t);
int	fchmod (int, mode_t);
int	fstat (int, struct stat *);
int	mkdir (const char *, mode_t);
int	mkfifo (const char *, mode_t);
int	stat (const char *, struct stat *);
int	lstat (const char *, struct stat *);
mode_t	umask (mode_t);
#endif

#ifdef __cplusplus
}
#endif
#endif /* !__SYS_STAT_H */
