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
 * mips/cop2.h: COP2 intrinsics
 */


#ifndef _MIPS_COP2_H_
#define _MIPS_COP2_H_

/* Load 32-bit COP2 data register C2REG from memory operand MEM */
#define mips_lwc2(C2REG, MEM)					\
do {								\
    __asm__ __volatile ("lwc2 $%0,%1"				\
			  : /* no outputs */			\
			  : "n" (C2REG), "m" (MEM));		\
} while (0)

/* Store 32-bit COP2 data register C2REG to memory operand MEM */
#define mips_swc2(C2REG, MEM)					\
do {								\
    __asm__ __volatile ("swc2 $%1,%0"				\
			  : "=m" (MEM)				\
			  : "n" (C2REG));			\
} while (0)

/* Load 64-bit COP2 data register C2REG from memory operand MEM */
#define mips_ldc2(C2REG, MEM)					\
do {								\
    __asm__ __volatile ("ldc2 $%0,%1"				\
			  : /* no outputs */			\
			  : "n" (C2REG), "m" (MEM));		\
} while (0)

/* Store 64-bit COP2 data register C2REG to memory operand MEM */
#define mips_sdc2(C2REG, MEM)					\
do {								\
    __asm__ __volatile ("sdc1 $%1,%0"				\
			  : "=m" (MEM)				\
			  : "n" (C2REG));			\
} while (0)

/* Set 32-bit COP2 data register C2REG,SEL to VAL */
#define mips_mtc2(VAL, C2REG, SEL)				\
do {								\
    __asm__ __volatile ("mtc2 %0,$%1,%2"			\
			  : /* no outputs */			\
			  :  "d" (VAL), "n" (C2REG), "n" (SEL));\
} while (0)

/* Get 32-bit COP2 data register C2REG,SEL */
#define mips_mfc2(C2REG, SEL)					\
__extension__ ({						\
    int __v;							\
    __asm__ __volatile ("mfc2 %0,$%1,%2"			\
			  : "=d" (__v)				\
			  : "n" (C2REG), "n" (SEL));		\
    __v;							\
})

/* Set 64-bit COP2 data register C2REG,SEL to VAL */
#define mips_dmtc2(VAL, C2REG, SEL)				\
do {								\
    __asm__ __volatile ("dmtc2 %0,$%1,%2"			\
			  : /* no outputs */			\
			  :  "d" (VAL), "n" (C2REG), "n" (SEL));\
} while (0)

/* Get 64-bit COP2 data register C2REG,SEL */
#define mips_dmfc2(C2REG, SEL)					\
__extension__ ({						\
    long long __v;						\
    __asm__ __volatile ("dmfc2 %0,$%1,%2"			\
			  : "=d" (__v)				\
			  : "n" (C2REG), "n" (SEL));		\
    __v;							\
})

/* Perform COP2 operation OP */
#define mips_cop2(OP)						\
do {								\
    __asm__ __volatile ("c2 %0"					\
			  : /* no outputs */			\
			  :  "n" (OP));				\
} while (0)

/* Set 32-bit COP2 control register C2REG to VAL */
#define mips_ctc2(VAL, C2REG)					\
do {								\
    __asm__ __volatile ("ctc2 %1,$%0"				\
			  : /* no outputs */			\
			  :  "n" (C2REG), "d" (VAL));		\
} while (0)

/* Get 32-bit COP2 control register C2REG */
#define mips_cfc2(C2REG)					\
__extension__ ({						\
    int __v;							\
    __asm__ __volatile ("cfc2 %0,$%1"				\
			  : "=d" (__v)				\
			  : "n" (C2REG));			\
    __v;							\
})

/* Return value of COP2 cond code number CC */
#define mips_c2t(CC)						\
__extension__ ({						\
   int __c;							\
   __asm__ __volatile (						\
       "li	%0,1\n"						\
"	%(bc2tl	$cc%1,0f\n"					\
"	move	%0,%.%)\n"					\
"0:"								\
  	: "=&d" (__c)						\
 	:  "n" (CC));						\
   __c;								\
})

/* Return inverted value of COP2 cond code number CC */
#define mips_c2f(CC)						\
__extension__ ({						\
   int __c;							\
   __asm__ __volatile (						\
       "li	%0,1\n"						\
"	%(bc2fl	$cc%1,0f\n"					\
"	move	%0,%.%)\n"					\
"0:"								\
  	: "=&d" (__c)						\
 	:  "n" (CC));						\
   __c;								\
})

#endif
