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
 * limits.h : MIPS machine limits
 */


#ifndef _LIMITS_H___
#define _LIMITS_H___

#define CHAR_BIT 	8
#define MB_LEN_MAX 	2

#define SCHAR_MAX	0x7f
#define SCHAR_MIN 	(-0x7f-1)
#define UCHAR_MAX	0xff

#ifdef __CHAR_UNSIGNED__
#define CHAR_MAX 	0xff
#define CHAR_MIN 	0
#else
#define CHAR_MAX 	0x7f
#define CHAR_MIN 	(-0x7f-1)
#endif

#define SHRT_MAX 	0x7fff
#define SHRT_MIN 	(-0x7fff-1)
#define USHRT_MAX 	0xffffU

#define INT_MAX 	0x7fffffff
#define INT_MIN 	(-0x7fffffff-1)
#define UINT_MAX 	0xffffffffU

#define LONG_MAX 	0x7fffffff
#define LONG_MIN 	(-0x7fffffff-1)
#define ULONG_MAX 	0xffffffffU

#ifndef __STRICT_ANSI__
/* GCC extension */
#define LONG_LONG_MAX	0x7fffffffffffffffLL
#define LONG_LONG_MIN	(-0x7fffffffffffffffLL-1)
#define ULONG_LONG_MAX	0xffffffffffffffffULL

/* ISO C 9X draft */
#define	LLONG_MAX	0x7fffffffffffffffLL
#define	LLONG_MIN	(-0x7fffffffffffffffLL-1)
#define	ULLONG_MAX	0xffffffffffffffffULL
#endif /* !__STRICT_ANSI__ */

#if !defined(_ANSI_SOURCE)
/* see <machine/ansi.h> for typedefs */
#define	SSIZE_MAX	LONG_MAX	/* max value for a ssize_t */
#if !defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
#define	SIZE_T_MAX	ULONG_MAX	/* max value for a size_t */
#endif
#endif

#endif /* _LIMITS_H___ */
