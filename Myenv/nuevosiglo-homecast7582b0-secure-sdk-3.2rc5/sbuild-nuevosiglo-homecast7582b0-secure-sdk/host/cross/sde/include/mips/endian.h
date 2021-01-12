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
 * mips/endian.h : MIPS-specific endian definitions
 */


#ifndef BYTE_ORDER
/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax) */
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */

#if #endian(big) || defined(__MIPSEB__) || defined(MIPSEB)
#define BYTE_ORDER      BIG_ENDIAN
#elif #endian(little) || defined(__MIPSEL__) || defined(MIPSEL)
#define BYTE_ORDER      LITTLE_ENDIAN
#else
#error BYTE_ORDER 
#endif

#if __mips_isa_rev >= 2 && ! __mips16

/* MIPS32r2 & MIPS64r2 can use the wsbh and rotate instructions, define
   MD_SWAP so that <sys/endian.h> will use them. */

#define MD_SWAP

#define __swap16md(x) __extension__({					\
    u_int16_t __swap16md_x = (x);					\
    u_int16_t __swap16md_v;						\
    __asm__ ("wsbh %0,%1" 						\
	     : "=d" (__swap16md_v) 					\
	     : "d" (__swap16md_x)); 					\
    __swap16md_v; 							\
})

#define __swap32md(x) __extension__({					\
    u_int32_t __swap32md_x = (x);					\
    u_int32_t __swap32md_v;						\
    __asm__ ("wsbh %0,%1; rotr %0,16" 					\
	     : "=d" (__swap32md_v) 					\
	     : "d" (__swap32md_x)); 					\
    __swap32md_v; 							\
})

#endif /* __mips_isa_rev >= 2 && ! __mips16 */

#endif	/* BYTE_ORDER */

#include <sys/endian.h>
