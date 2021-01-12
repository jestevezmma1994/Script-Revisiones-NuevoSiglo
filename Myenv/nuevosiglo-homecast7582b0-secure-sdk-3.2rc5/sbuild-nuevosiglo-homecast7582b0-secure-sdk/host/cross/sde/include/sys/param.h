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
 * sys/param.h: BSD-style system parameters
 */


#ifdef _KERNEL

/* include the real network kernel version */
#include_next <sys/param.h>

#else
/* usually this is what an application expects */
#define BSD 	199306
#define BSD4_3	1
#define BSD4_4	1

#define OpenBSD	200012		/* OpenBSD version (year & month). */
#define OpenBSD2_8 1		/* OpenBSD 2.8 */
#define __AlgRTX__ 1

#include <sys/types.h>
#include <machine/endian.h>
#include <limits.h>
#include <machine/limits.h>
#include <sys/sdeversion.h>	/* history */

#if 0
/* exit() should kill current process only */
void pthread_proc_exit_np (int) __attribute__ ((noreturn));
#define exit	pthread_proc_exit_np
#endif

#define MAXHOSTNAMELEN	256		/* max hostname size */

/*
 * Round p (pointer or byte index) up to a correctly-aligned value for all
 * data types (int, long, ...).   The result is u_int and must be cast to
 * any desired pointer type.
 */
#define	ALIGNBYTES	7
#define	ALIGN(p)	(((u_int)(p) + ALIGNBYTES) &~ ALIGNBYTES)
#define ALIGNED_POINTER(p, t)  ((((u_int32_t)(p)) & (sizeof (t) - 1)) == 0)

#ifndef NULL
#ifdef 	__GNUG__
#define	NULL	__null
#else
#define	NULL	0
#endif
#endif
#endif
