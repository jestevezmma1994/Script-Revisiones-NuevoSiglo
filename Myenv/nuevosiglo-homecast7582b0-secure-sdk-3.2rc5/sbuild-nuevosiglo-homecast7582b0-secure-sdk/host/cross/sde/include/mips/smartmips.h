/*
 * Copyright (c) 2002-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * mips/smartmips.h: SmartMIPS intrinsics
 */


#ifndef _MIPS_SMARTMIPS_H_
#define _MIPS_SMARTMIPS_H_

#include <mips/m32c0.h>

/* Extra CP0 register bits for SmartMIPS ASE */

/* EntryLo0/1 extensions */
#define TLB_RI		0x80000000	/* Read Inhibit */
#define TLB_XI		0x40000000	/* Execute Inhibit */

/* PageGrain bits */
#define PAGEGRAIN_RIE	0x80000000	/* Read Inhibit Enable */
#define PAGEGRAIN_XIE	0x40000000	/* Execute Inhibit Enable */
#define PAGEGRAIN_MASK	0x00001800	/* Mask bits */
#define PAGEGRAIN_MBO	0x00000700	/* must be one */

#ifdef __ASSEMBLER__

/* 
 * Coprocessor 0 register numbers 
 */
#define C0_CONTEXTCFG	$4,1
#define C0_PAGEGRAIN	$5,1

#else

/* SmartMIPS ContextConfig register */
#define mips32_getcontextcfg()	_m32c0_mfc0(4,1)
#define mips32_setcontextcfg(x)	_m32c0_mtc0(4,1,x)
#define mips32_xchcontextcfg(x)	_m32c0_mxc0(4,1,x)

#if 0
/* SmartMIPS PageGrain register */
/* now defined in m32c0.h */
#define mips32_getpagegrain()	_m32c0_mfc0(5,1)
#define mips32_setpagegrain(x)	_m32c0_mtc0(5,1,x)
#define mips32_xchpagegrain(x)	_m32c0_mxc0(5,1,x)
#endif

#endif

#ifndef __ASSEMBLER__

/* SmartMIPS C Intrinsics */

/* Polynomial LO = A * B, and return LO register as result. */
#define mips_multp(a, b) 					\
__extension__ ({						\
    unsigned int __lo; 						\
    __asm__ ("multp %1,%2" 					\
	     : "=l" (__lo) 					\
	     : "d" (a), "d" (b) 				\
	     : "hi"); 						\
    __lo; 							\
})

/* Polynomial LO += A * B, and return LO register as result.  Assumes
   you've already preloaded LO somehow, since it's not an input. */
#define mips_maddp2(a, b) 					\
__extension__ ({						\
   unsigned int __lo;						\
    __asm__ ("maddp %1,%2" 					\
	     : "=l" (__lo) 					\
	     : "d" (a), "d" (b) 				\
	     : "hi"); 						\
    __lo; 							\
})

/* Polynomial LO += A * B, and return LO register as result.  LO is
   normally the result of a previous mips_multp() or mips_maddp(). */
#define mips_maddp(lo, a, b) 					\
__extension__ ({						\
    __asm__ ("maddp %1,%2" 					\
	     : "+l" (lo) 					\
	     : "d" (a), "d" (b) 				\
	     : "hi"); 						\
    lo; 							\
})

/* Polynomial HILO = A * B, and return HI/LO registers as a 64-bit
   long long result. */
#define mips_multpx(a, b) 					\
__extension__ ({						\
    unsigned long long __hilo; 					\
    __asm__ ("multp %1,%2" 					\
	     : "=x" (__hilo) 					\
	     : "d" (a), "d" (b)); 				\
    __hilo; 							\
})

/* Polynomial HILO += A * B, and return HI/LO registers as a 64-bit
   long long result. Assumes you've already preloaded HILO somehow,
   since it's not an input. */
#define mips_maddp2x(a, b) 					\
__extension__ ({						\
   unsigned long long __hilo;					\
    __asm__ ("maddp %1,%2" 					\
	     : "=x" (__hilo) 					\
	     : "d" (a), "d" (b)); 				\
    __hilo; 							\
})

/* Polynomial HILO += A * B, and return HI/LO registers as a 64-bit
   long long result.  HILO is normally the result of a previous
   mips_multpx() or mips_maddpx(). */
#define mips_maddpx(hilo, a, b)					\
__extension__ ({						\
    __asm__ ("maddp %1,%2" 					\
	     : "+x" (hilo) 					\
	     : "d" (a), "d" (b)); 				\
    hilo; 							\
})

/* Return the LO register and then shift HI->LO, ACX->HI, 0->ACX */
/* Note: even though HI and LO will be modified we don't "clobber"
   them in the asm statement, so as far as gcc is concerned they still
   contain valid data. This allows you to fetch and scale the partial
   accumulator during a multiply-add loop, without requiring LO & HI
   to be saved and restored. But you'd better know what you're
   doing! */
#define mips_mflhxu(acc, l)					\
__extension__ ({						\
    __asm__ ("mflhxu %0"					\
	     : "=d" (l), "+l" (acc));				\
    acc;							\
})

#define mips_mflhxux(acc, l)					\
__extension__ ({						\
    __asm__ ("mflhxu %0"					\
	     : "=d" (l), "+x" (acc));				\
    acc;							\
})
    
/* Return just the multiply accumulator extension. */
#define mips_mfxu()	 					\
__extension__ ({						\
    unsigned int __ex;						\
    __asm__ __volatile (					\
       "mflhxu %.\n"						\
"	mfhi %0"						\
	     : "=d" (__ex)					\
	     : /* no inputs */					\
	     : "hi", "lo");					\
    __ex;							\
})

/* Write 96-bit accumulator from L, H, and X. */
#define mips_mtlhx(l, h, x) 					\
do {								\
    __asm__ __volatile (					\
       "mtlhx %z0\n"						\
"	mtlhx %z1\n"						\
"	mtlhx %z2" 						\
	     : /* no outputs */					\
	     : "dJ" (x), "dJ" (h), "dJ" (l)			\
	     : "hi", "lo");					\
} while (0)

/* Shift the 96-bit accumulator 6 bits left, and mix in 6 bits of SRC,
   permuted by SEL. */
#define mips_pperm(src, sel) 					\
do {								\
    __asm__ __volatile (					\
       "pperm %z0,%z1\n"					\
	     : /* no outputs */					\
	     : "dJ" (src), "dJ" (sel)				\
	     : "hi", "lo");					\
} while (0)

#endif /* __ASSEMBLER__ */

#endif /* _MIPS_SMARTMIPS_H_ */
