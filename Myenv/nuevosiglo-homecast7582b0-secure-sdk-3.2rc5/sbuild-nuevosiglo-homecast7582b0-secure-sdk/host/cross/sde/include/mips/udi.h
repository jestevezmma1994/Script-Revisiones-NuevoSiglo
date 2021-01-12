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
 * mips/udi.h: UDI intrinsics
 */


#ifndef _MIPS_UDI_H_
#define _MIPS_UDI_H_

#define __mips_udi_ri_x(VOL, OP, S, IMM)  			\
__extension__ ({						\
  __typeof__(S) __s = (S);					\
  __asm__ VOL ("udi%1 %0,%2" 					\
	       : "+d" (__s) 					\
	       : "K" (OP), "K" (IMM)); 				\
  __s; 								\
})

#define __mips_udi_rri_x(VOL, OP, S, T, IMM)  			\
__extension__ ({						\
  __typeof__(S) __t = (T);					\
  __asm__ VOL ("udi%1 %z0,%0,%3" 				\
	   : "+d" (__t) 					\
	   : "K" (OP), "dJ" (S), "K" (IMM)); 			\
  __t; 								\
})

#define __mips_udi_rrri_x(VOL, OP, S, T, IMM)  			\
__extension__ ({						\
  __typeof__(S) __d;						\
  __asm__ VOL ("udi%1 %z2,%z3,%0,%4" 				\
	   : "=d" (__d) 					\
	   : "K" (OP), "dJ" (S), "dJ" (T), "K" (IMM)); 		\
  __d; 								\
})


/* Basic UDI instructiions are assumed to write a result to their
   final CPU register operand, but may may have other side effects
   such as using or modifying internal UDI registers, so they won't be
   optimised by the compiler. */

/* The mips_udi_ri() intrinsic passes S in RS register, 
   and returns the r/w RS register */
#define mips_udi_ri(OP, S, IMM)					\
	__mips_udi_ri_x (__volatile, OP, S, IMM)

/* The mips_udi_rri() intrinsic passes S in RS register, T in RT,
   and returns the r/w RT register */
#define mips_udi_rri(OP, S, T, IMM)				\
	__mips_udi_rri_x (__volatile, OP, S, T, IMM) 

/* The mips_udi_rrri() intrinsic passes S in RS, T in RT, 
   and returns the w/o RD register */
#define mips_udi_rrri(OP, S, T, IMM)				\
	__mips_udi_rrri_x (__volatile, OP, S, T, IMM)


/* Optimisable intrinsics for UDI instructions which read only the CPU
   source registers and write to the destination CPU register only,
   and have no other side effects, i.e. they only use and modify the
   supplied CPU registers. */

#define mips_udi_ri_safe(OP, S, IMM)				\
	__mips_udi_ri_x (, OP, S, IMM)
#define mips_udi_rri_safe(OP, S, T, IMM)				\
	__mips_udi_rri_x (, OP, S, T, IMM) 
#define mips_udi_rrri_safe(OP, S, T, IMM)				\
	__mips_udi_rrri_x (, OP, S, T, IMM)


/* "Novalue" intrinsics for UDI instructions which don't write a
   result to a CPU register, so presumably must have some other side
   effect, such as modifying an internal UDI register. */

#define mips_udi_nv(IMM)					\
  do { 								\
    __asm__ __volatile ("udi %0"				\
		 : /* no outputs */				\
 	         : "n" (IMM));					\
  } while (0)

#define mips_udi_i_nv(OP, IMM)					\
  do { 								\
    __asm__ __volatile ("udi%0 %1"				\
		 : /* no outputs */				\
		 : "K" (OP), "n" (IMM)); 			\
  } while (0)

#define mips_udi_ri_nv(OP, S, IMM)				\
  do { 								\
    __asm__ __volatile ("udi%0 %z1,%2"				\
	       : /* no outputs */				\
	       : "K" (OP), "dJ" (S), "K" (IMM)); 		\
  } while (0)

#define mips_udi_rri_nv(OP, S, T, IMM)				\
  do { 								\
    __asm__ __volatile ("udi%0 %z1,%z2,%3"			\
	       : /* no outputs */				\
	       : "K" (OP), "dJ" (S), "dJ" (T), "K" (IMM)); 	\
  } while (0)

/* These two cannot write a result, i.e. they are implicitly "novalue" */
#define mips_udi(IMM)						\
	mips_udi_nv (IMM)
#define mips_udi_i(OP, IMM)					\
	mips_udi_i_nv (OP, IMM)


/* These 4 variants of the three register operand format allow
   constant values to be placed in the RS, RT fields, presumably
   because they name internal UDI registers. The RD register is still
   allocated by the compiler. They are implicitly "unsafe" or
   volatile. */

#define mips_udi_riri(OP, S, IT, IMM) 				\
__extension__ ({ 						\
  __typeof__(S) __d;						\
  __asm__ __volatile ("udi%1 %z2,$%3,%0,%4" 			\
	   : "=d" (__d) 					\
	   : "K" (OP), "dJ" (S), "K" (IT), "K" (IMM)); 		\
  __d; 								\
})

#define mips_udi_irri(OP, IS, T, IMM)  				\
__extension__ ({ 						\
  __typeof__(T) __d;						\
  __asm__ __volatile ("udi%1 $%2,%z3,%0,%4" 			\
	   : "=d" (__d) 					\
	   : "K" (OP), "K" (IS), "dJ" (T), "K" (IMM)); 		\
  __d; 								\
})

#define mips_udi_iiri_32(OP, IS, IT, IMM) 			\
__extension__ ({						\
  int __d;							\
  __asm__ __volatile ("udi%1 $%2,$%3,%0,%4" 			\
	   : "=d" (__d) 					\
	   : "K" (OP), "K" (IS), "K" (IT), "K" (IMM)); 		\
  __d; 								\
})

#define mips_udi_iiri_64(OP, IS, IT, IMM) 			\
__extension__ ({ 						\
  long long __d;						\
  __asm__ __volatile ("udi%1 $%2,$%3,%0,%4" 			\
	   : "=d" (__d) 					\
	   : "K" (OP), "K" (IS), "K" (IT), "K" (IMM)); 		\
  __d; 								\
})


/* These 5 variants of the three register format allow constant values
   to be placed in the RS, RT and RD fields, presumably because they
   name internal UDI registers. In case the instruction writes to an
   implicit gp register, pass the register number as GPDEST and the
   compiler will be told that it's been clobbered, and its value will
   be returned - if no gp register is written, pass 0. They are all
   implicitly unsafe, or volatile. */

#define mips_udi_rrii(OP, S, T, ID, IMM, GPDEST) 		\
__extension__ ({ 						\
  __typeof__(S) __d;						\
  __asm__ ("udi%0 %z1,%z2,$%3,%4" 				\
	   : /* no outputs */ 					\
	   : "K" (OP), "dJ" (S), "dJ" (T), "K" (ID), "K" (IMM)  \
           : "$" # GPDEST); 					\
  if (GPDEST != 0)						\
    __asm__ __volatile ("move %0,$"  # GPDEST			\
			: "=d" (__d));				\
  __d;								\
})

#define mips_udi_riii(OP, S, IT, ID, IMM, GPDEST)  		\
__extension__ ({ 						\
  __typeof__(S) __d;						\
  __asm__ ("udi%0 %z1,$%2,$%3,%4" 				\
	   : /* no outputs */ 					\
	   : "K" (OP), "dJ" (S), "K" (IT), "K" (ID), "K" (IMM)	\
           : "$" # GPDEST); 					\
  if (GPDEST != 0)						\
    __asm__ __volatile ("move %0,$"  # GPDEST			\
			: "=d" (__d));				\
})

#define mips_udi_irii(OP, IS, T, ID, IMM, GPDEST)  		\
__extension__ ({ 						\
  __typeof__(T) __d;						\
  __asm__ ("udi%0 $%1,%z2,$%3,%4" 				\
	   : /* no outputs */ 					\
	   : "K" (OP), "K" (IS), "dJ" (T), "K" (ID), "K" (IMM)	\
           : "$" # GPDEST); 					\
  if (GPDEST != 0)						\
    __asm__ __volatile ("move %0,$"  # GPDEST			\
			: "=d" (__d));				\
})

#define mips_udi_iiii_32(OP, IS, IT, ID, IMM, GPDEST)  		\
__extension__ ({						\
  int __d;							\
  __asm__ ("udi%0 $%1,$%2,$%3,%4" 				\
	   : /* no outputs */ 					\
	   : "K" (OP), "K" (IS), "K" (IT), "K" (ID), "K" (IMM)  \
           : "$" # GPDEST); 					\
  if (GPDEST != 0)						\
    __asm__ __volatile ("move %0,$"  # GPDEST			\
			: "=d" (__d));				\
})

#define mips_udi_iiii_64(OP, IS, IT, ID, IMM, GPDEST)  		\
__extension__ ({ 						\
  long long __d;						\
  __asm__ ("udi%0 $%1,$%2,$%3,%4" 				\
	   : /* no outputs */ 					\
	   : "K" (OP), "K" (IS), "K" (IT), "K" (ID), "K" (IMM)  \
           : "$" # GPDEST); 					\
  if (GPDEST != 0)						\
    __asm__ __volatile ("move %0,$"  # GPDEST			\
			: "=d" (__d));				\
})

#endif
