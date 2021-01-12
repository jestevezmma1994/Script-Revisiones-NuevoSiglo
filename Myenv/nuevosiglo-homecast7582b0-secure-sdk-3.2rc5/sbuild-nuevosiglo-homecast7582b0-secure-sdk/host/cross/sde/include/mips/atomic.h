/*
 * Copyright (c) 2003 MIPS Technologies, Inc.  All rights reserved.
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
 * mips/atomic.h: atomic r-m-w memory operations
 */

#ifndef _MIPS_ATOMIC_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _MIPS_ATOMIC_H_

/* 
 * The atomic_update() primitive provides a range of atomic operations
 * on the 32-bit word pointed to by its first argument.  In all cases 
 * the previous value of the word is returned.
 * 
 * The primitive is used to implement the following functions: 
 * bis = bit set; bic = bit clear; flip = bit clear and set;
 * swap = swap; add = increment/decrement.
 */

/* FIXME it's not true that all MIPS II+ ISAs support ll/sc */

#if __mips >= 2 && ! __mips16
#define _mips_atomic_update(wp, clear, addend)		\
    __extension__ ({unsigned int __old, __tmp;		\
      __asm__ __volatile__ (				\
"1:	ll	%0,%1\n"				\
"	and	%2,%0,%3\n"				\
"	addu	%2,%4\n"				\
"	sc	%2,%1\n"				\
"	beqz	%2,1b"					\
	: "=&d" (__old), "+m" (*(wp)), "=&d" (__tmp)	\
	: "dJK" (~(clear)), "dJI" (addend)); 		\
      __old; })

/*
 * The atomic_cas() primitive implements compare-and-swap.
 * The old value of the word is returned.
 */
#define _mips_atomic_cas(wp, new, cmp)			\
    __extension__ ({unsigned int __old, __tmp;		\
      __asm__ __volatile__ (				\
"1:	ll	%0,%1\n"				\
"	%(bne	%0,%z3,2f\n"				\
"	move	%2,%z4%)\n"				\
"	sc	%2,%1\n"				\
"	beqz	%2,1b\n"				\
"2:"							\
	: "=&d" (__old), "+m" (*(wp)), "=&d" (__tmp)	\
	: "dJ" (cmp), "dJ" (new)); 			\
      __old; })
#else
extern unsigned int	_mips_atomic_update (volatile unsigned int *,
					     unsigned int, unsigned int);
extern unsigned int	_mips_atomic_cas (volatile unsigned int *,
					  unsigned int, unsigned int);
#endif

#define mips_atomic_bis(wp, val)	_mips_atomic_update(wp,val,val)
#define mips_atomic_bic(wp, val)	_mips_atomic_update(wp,val,0)
#define mips_atomic_bcs(wp, clr, set)	_mips_atomic_update(wp,(clr)|(set),set)
#define mips_atomic_flip(wp, clr, set)	_mips_atomic_update(wp,(clr)|(set),set)
#define mips_atomic_swap(wp, val)	_mips_atomic_update(wp,~0,val)
#define mips_atomic_add(wp, val)	_mips_atomic_update(wp,0,val)
#define mips_atomic_inc(wp)		_mips_atomic_update(wp,0,1)
#define mips_atomic_dec(wp)		_mips_atomic_update(wp,0,-1)
#define mips_atomic_cas(wp, new, cmp)	_mips_atomic_cas(wp,new,cmp)

#ifdef __cplusplus
}
#endif
#endif /*_MIPS_ATOMIC_H_*/
