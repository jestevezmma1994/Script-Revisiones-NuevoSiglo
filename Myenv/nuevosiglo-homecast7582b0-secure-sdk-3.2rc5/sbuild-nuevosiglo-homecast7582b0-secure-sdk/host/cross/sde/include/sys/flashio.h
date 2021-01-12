/*
 * Copyright (c) 2000-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * sys/flashio.h: raw flash memory ioctl interface
 */


#ifndef _sys_flashio_H_
#define _sys_flashio_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Information structure returned by the flashinfo() function
 */
struct flashinfo {
    char 		name[32];	/* dev name */
    unsigned long	base;		/* dev base (phys address) */
    unsigned int	size;		/* dev size */
    unsigned long	mapbase; 	/* memory mapped base (phys addr) */
    unsigned char	unit;		/* unit byte size (1,2,4,8 or 16) */
    unsigned int	maxssize; 	/* maximum sector size */
    unsigned int	soffs;		/* base offset of specified sector */
    unsigned int	ssize;		/* size of specified sector */
    int			sprot;		/* specified sector is protected */
};


/* flash partition information */
struct flashpart {
    int			type;		/* partition type */
    unsigned int	offs;		/* base of partition */
    unsigned int	size;		/* size of partition */
};


#define FLASHNPART	8		/* max number of partitions */

/* partition table */
struct flashparts {
    struct flashpart	part[FLASHNPART];
};


/* flash partition types */
#define FLASHPART_RAW		0	/* raw (whole) device */
#define FLASHPART_BOOT		1	/* boot partition */
#define FLASHPART_POST		2	/* post partition (self test) */
#define FLASHPART_ENV		3	/* non-volatile environment */
#define FLASHPART_FFS		4	/* flash file system partition */
#define FLASHPART_UNDEF		255	/* undefined partition */

/* programming/access mode flags */
#define FLASHFLGS_REBOOT	0x01	/* reboot after next write */
#define FLASHFLGS_NOCOPY	0x02	/* don't copy prog code to ram */
#define FLASHFLGS_MERGE		0x04	/* merge partially written sectors */
#define FLASHFLGS_CODE_EB	0x08	/* access as if big-endian code */
#define FLASHFLGS_CODE_EL	0x00	/* access as if little-endian code */
#if #endian(big)
#define FLASHFLGS_CODE		FLASHFLGS_CODE_EB
#else
#define FLASHFLGS_CODE		FLASHFLGS_CODE_EL
#endif
#define FLASHFLGS_STREAM	0x10	/* access as byte stream */

/* ioctl interface */
#define	FLASHIOINFO	 _IOWR('h', 0, struct flashinfo)/* get flash info */
#define	FLASHIOGPART	 _IOR('h', 1, struct flashpart) /* get partition */
#define	FLASHIOGFLGS	 _IOR('h', 2, unsigned int)	/* get flags */
#define	FLASHIOSFLGS	 _IOW('h', 3, unsigned int)	/* set flags */
#define	FLASHIOERASEDEV	 _IO('h', 4)			/* erase device */
#define	FLASHIOERASESECT _IOW('h', 5, unsigned int)	/* erase sector */
#define	FLASHIOGPARTS	 _IOR('h', 7, struct flashparts)/* get all partns */
#define	FLASHIOSPARTS	 _IOR('h', 8, struct flashparts)/* set all partns */
#define	FLASHIOFLUSH	 _IO('h', 9) 			/* flush writes */

#ifdef __cplusplus
}
#endif

#endif /* _sys_flashio_H_ */
