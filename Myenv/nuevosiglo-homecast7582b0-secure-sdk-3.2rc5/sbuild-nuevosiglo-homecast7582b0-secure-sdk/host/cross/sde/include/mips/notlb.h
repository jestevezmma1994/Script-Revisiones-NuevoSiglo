/*
 * Copyright (c) 1999-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * notlb.h : MIPS SDE stub MMU/TLB definitions
 */


#ifndef _NOTLB_H_
#define _NOTLB_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ROM_BASE
#define ROM_BASE	0xbfc00000	/* standard ROM base address */
#endif

#ifdef __ASSEMBLER__

/* 
 * Stub 32-bit memory regions 
 */
#define	KSEG0_BASE	0x80000000
#define	KSEG1_BASE	0xa0000000
#define KSEG0_SIZE	0x20000000
#define KSEG1_SIZE	0x20000000
#define RVEC_BASE	ROM_BASE

/* 
 * Translate a kernel address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((pa) | 0x80000000)
#define PA_TO_KVA1(pa)	((pa) | 0xa0000000)

/* translate between KSEG0 and KSEG1 addresses */
#define KVA0_TO_KVA1(v)	((v) | 0x20000000)
#define KVA1_TO_KVA0(v)	((v) & ~0x20000000)

#else /* __ASSEMBLER__ */
/*
 * Standard address types
 */
#ifndef _PADDR_T_DEFINED_
typedef unsigned long		paddr_t;	/* a physical address */
#define _PADDR_T_DEFINED_
#endif
#ifndef _VADDR_T_DEFINED_
typedef unsigned long		vaddr_t;	/* a virtual address */
#define _VADDR_T_DEFINED_
#endif

/* 
 * Stub 32-bit memory regions 
 */
#define KSEG0_BASE	((void  *)0x80000000)
#define KSEG1_BASE	((void  *)0xa0000000)
#define KSEG0_SIZE	0x20000000u
#define KSEG1_SIZE	0x20000000u

#define RVEC_BASE	((void *)ROM_BASE)	/* reset vector base */

/* 
 * Translate a kernel virtual address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((paddr_t)(v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((void *) ((pa) | 0x80000000))
#define PA_TO_KVA1(pa)	((void *) ((pa) | 0xa0000000))

/* translate between KSEG0 and KSEG1 virtual addresses */
#define KVA0_TO_KVA1(v)	((void *) ((unsigned)(v) | 0x20000000))
#define KVA1_TO_KVA0(v)	((void *) ((unsigned)(v) & ~0x20000000))

/* Test for KSEGS */
#define IS_KVA(v)	((int)(v) < 0)
#define IS_KVA0(v)	(((unsigned)(v) >> 29) == 0x4)
#define IS_KVA1(v)	(((unsigned)(v) >> 29) == 0x5)
#define IS_KVA01(v)	(((unsigned)(v) >> 30) == 0x2)

/* convert register type to address and back */
#define VA_TO_REG(v)	((long)(v))		/* sign-extend 32->64 */
#define REG_TO_VA(v)	((void *)(long)(v))	/* truncate 64->32 */

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif
#endif /* _NOTLB_H_*/
