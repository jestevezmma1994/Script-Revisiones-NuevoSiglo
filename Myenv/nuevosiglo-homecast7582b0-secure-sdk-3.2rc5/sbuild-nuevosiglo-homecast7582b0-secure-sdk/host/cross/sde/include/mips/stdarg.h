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
 * stdarg.h : MIPS stdarg handling
 */


#ifndef __MIPS_STDARG_H
#define __MIPS_STDARG_H

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)

/* use machine-independent header */
#include <stdarg.h>

#else 

/*
 * ANSI style varargs for MIPS
 */

#include <machine/ansi.h>

typedef _VA_LIST_ va_list;

#if __mips64 && ! (#abi(32) || defined(__mips_algabi))
/* Cygnus GCC passes args as 64-bits regs */
#define __VA_REG	long long
#else
/* Algor GCC passes args as 32-bits regs for compatibility */
#define __VA_REG	int
#endif

#ifndef _VA_MIPS_H_ENUM
#define _VA_MIPS_H_ENUM
/* values returned by __builtin_classify_type */
enum {
  __no_type_class = -1,
  __void_type_class,
  __integer_type_class,
  __char_type_class,
  __enumeral_type_class,
  __boolean_type_class,
  __pointer_type_class,
  __reference_type_class,
  __offset_type_class,
  __real_type_class,
  __complex_type_class,
  __function_type_class,
  __method_type_class,
  __record_type_class,
  __union_type_class,
  __array_type_class,
  __string_type_class,
  __set_type_class,
  __file_type_class,
  __lang_type_class
};
#endif

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (__VA_REG) - 1) / sizeof (__VA_REG)) 	\
   * sizeof (__VA_REG))

/* Alignment of an arg of given type (char,short promote to reg), - 1. */

#if __mips64 && ! (#abi(32) || defined(__mips_algabi))
#define __va_alignoff(TYPE)  7
#else
#define __va_alignoff(TYPE)  						\
  (((__alignof(TYPE) <= __alignof(__VA_REG)) 				\
    ?__alignof(__VA_REG) 						\
    : __alignof(TYPE)) - 1)
#endif

/* When big-endian, small structures are shifted left in register so
   as to be in low memory when saved, but scalar data smaller than a
   register is not. */

#define __va_real_size(TYPE) \
    ((sizeof(TYPE) <= sizeof(__VA_REG)	 				\
      && __builtin_classify_type(*(TYPE *)0) != __record_type_class	\
      && __builtin_classify_type(*(TYPE *)0) != __union_type_class)	\
     ? sizeof(TYPE) : __va_rounded_size(TYPE))

#if defined(__VARARGS_H)
/* old-style vararg */
#define va_alist	__builtin_va_alist
#define va_dcl		__VA_REG __builtin_va_alist; ...
#define va_start(AP)	AP = (va_list)&__builtin_va_alist;
#elif __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
/* new-style stdarg, modern gcc */
#define va_start(AP, LASTARG) 						\
 (AP = ((va_list) __builtin_next_arg(LASTARG)))
#else
/* new-style stdarg, older gcc */
#define va_start(AP, LASTARG) 						\
 (AP = ((va_list) __builtin_next_arg()))
#endif

#define va_end(AP)	((void)0)

/* Copy va_list into another variable of this type.  */
#define __va_copy(dest, src) (dest) = (src)

#ifdef __MIPSEB__
/* big-endian: args smaller than register in higher memory address */
#define va_arg(AP, TYPE) 						      \
 (AP = 									      \
   (va_list) ((((long)AP + __va_alignoff(TYPE)) & ~__va_alignoff(TYPE))       \
      + __va_rounded_size(TYPE)), 					      \
   *((TYPE *) (void *) ((char *)AP - __va_real_size(TYPE))))
#else
/* little-endian: args smaller than register in lower memory address */
#define va_arg(AP, TYPE) 						      \
 (AP = 									      \
   (va_list) ((((long)AP + __va_alignoff(TYPE)) & ~__va_alignoff(TYPE))       \
      + __va_rounded_size(TYPE)), 					      \
   *((TYPE *) (void *) ((char *)AP - __va_rounded_size(TYPE))))
#endif

#endif /* __GNUC__ >= 2.96 */

#endif /* __MIPS_STDARG_H */
