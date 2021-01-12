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
 * dirent.h : directory definitions
 */


#ifndef __DIRENT_H
#ifdef __cplusplus
extern "C" {
#endif
#define __DIRENT_H

/* POSIX, SVR3, BSD  include file */


/* --- Required -- */
#ifndef _POSIX_SOURCE
#include <sys/types.h>
#endif

/*
 * The kernel defines the format of directory entries returned by 
 * the getdirentries(2) system call.
 */
#include <sys/dirent.h>

/* definitions for library routines operating on directories. */
#define	DIRBLKSIZ	1024

/* --- Types --- */
#ifdef _POSIX_SOURCE
typedef void *	DIR;
#else

#define	d_ino		d_fileno	/* backward compatibility */

typedef struct _dirdesc {
    int		dd_fd;		/* file descriptor */
    long	dd_loc;		/* offset in block */
    long	dd_size;	/* amount of valid data */
    char	*dd_buf;	/* -> directory block */
    int		dd_len;		/* size of data buffer */
    long	dd_seek;	/* magic cookie from getdirentries */
    long	dd_rewind;	/* magic cookie for rewinding */
    int		dd_flags;	/* flags for readdir */
} DIR;			/* stream data from opendir() */

#define	dirfd(dirp)	((dirp)->dd_fd)

/* flags for opendir2 */
#define DTF_HIDEW	0x0001	/* hide whiteout entries */
#define DTF_NODUP	0x0002	/* don't return duplicate names */
#define DTF_REWIND	0x0004	/* rewind after reading union stack */
#define __DTF_READALL	0x0008	/* everything has been read */

#endif

/* --- Prototypes --- */
DIR	*opendir	(const char *);
struct dirent *readdir	(DIR *);
int	readdir_r	(DIR *, struct dirent *, struct dirent **);
void	rewinddir	(DIR *);
int	closedir	(DIR *);

#ifndef _POSIX_SOURCE
/* --- non POSIX --- */
int	getdirentries	(int, char *, int, long *);
long	telldir		(const DIR *);
void	seekdir		(DIR *, long);
int	scandir		(const char *, struct dirent ***,
			 int (*)(struct dirent *), 
			 int (*)(const void *, const void *));
int	alphasort 	(const void *, const void *);
#endif

#ifdef __cplusplus
}
#endif
#endif /* !defined __DIRENT_H */
