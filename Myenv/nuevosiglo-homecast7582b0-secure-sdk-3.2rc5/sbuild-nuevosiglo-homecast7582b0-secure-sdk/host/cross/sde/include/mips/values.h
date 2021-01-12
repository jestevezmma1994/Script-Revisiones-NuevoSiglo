/*
 * Unpublished work (c) MIPS Technologies, Inc.  All rights reserved.
 * Unpublished rights reserved under the copyright laws of the United
 * States of America and other countries.
 * 
 * This code is confidential and proprietary to MIPS Technologies,
 * Inc. ("MIPS Technologies") and may be disclosed only as permitted in
 * writing by MIPS Technologies or an authorized third party.  Any
 * copying, reproducing, modifying, use or disclosure of this code (in
 * whole or in part) that is not expressly permitted in writing by MIPS
 * Technologies or an authorized third party is strictly prohibited. At a
 * minimum, this code is protected under trade secret, unfair
 * competition, and copyright laws. Violations thereof may result in
 * criminal penalties and fines.
 * 
 * MIPS Technologies reserves the right to change this code to improve
 * function, design or otherwise. MIPS Technologies does not assume any
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
 * This code may only be disclosed to the United States government
 * ("Government"), or to Government users, with prior written consent
 * from MIPS Technologies or an authorized third party.  This code
 * constitutes one or more of the following: commercial computer
 * software, commercial computer software documentation or other
 * commercial items.  If the user of this code, or any related
 * documentation of any kind, including related technical data or
 * manuals, is an agency, department, or other entity of the Government,
 * the use, duplication, reproduction, release, modification, disclosure,
 * or transfer of this code, or any related documentation of any kind, is
 * restricted in accordance with Federal Acquisition Regulation 12.212
 * for civilian agencies and Defense Federal Acquisition Regulation
 * Supplement 227.7202 for military agencies.  The use of this code by
 * the Government is further restricted in accordance with the terms of
 * the license agreement(s) and/or applicable contract terms and
 * conditions covering this code from MIPS Technologies or an authorized
 * third party.
 */

/*
 * limits.h : MIPS floating point values 
 */


/* FOR COMPATIBILITY ONLY - DON'T USE FOR NEW CODE */

#ifndef _MIPS_VALUES_H_
#define _MIPS_VALUES_H_

#include <machine/limits.h>
#include <machine/float.h>

/* The bits per C type. */
#define	BITSPERBYTE	8
#define	BITS(type)	(BITSPERBYTE * sizeof(type))

/* The high-order bit for shorts, ints, longs. */
#define	HIBITS		((short)(1 << BITS(short) - 1))
#define	HIBITI		((int)(1 << BITS(int) - 1)
#define	HIBITL		((long)(1 << BITS(long) - 1)

/* Maximum short, int, long. */
#define	MAXSHORT	SHRT_MAX	/* Max short. */
#define	MAXINT		INT_MAX		/* Max int. */
#define	MAXLONG		LONG_MAX	/* Max long. */

#define	_IEEE		1		/* 1 if IEEE standard. */
#define	_HIDDENBIT	1		/* 1 if mantissa high-bit is implicit */

#define MAXDOUBLE	DBL_MAX		/* Max double. */
#define MINDOUBLE	DBL_MIN		/* Min double. */
#define	DMAXEXP		DBL_MAX_EXP	/* Max double exponent. */
#define	DMINEXP		DBL_MIN_EXP	/* Min double exponent. */
#define	DMAXPOWTWO	4503599627370496/* Max power of 2 in a double. */
#define	DSIGNIF		DBL_MANT_DIG	/* Significant bits in a double. */
#define	_DEXPLEN	11		/* Bits in double exponent. */
					/* natural log(MAXDOUBLE) */
#define	LN_MAXDOUBLE	709.78271289338399
					/* natural log(MINDOUBLE) */
#define	LN_MINDOUBLE	-708.3964185322641

#define MAXFLOAT	FLT_MAX		/* Max float. */
#define MINFLOAT	FLT_MIN		/* Min float. */
#define	FMAXEXP		FLT_MAX_EXP	/* Max float exponent. */
#define	FMINEXP		FLT_MIN_EXP	/* Min float exponent. */
#define	FMAXPOWTWO	8388608		/* Max power of 2 in a float. */
#define	FSIGNIF		FLT_MANT_DIG	/* Significant bits in a float. */
#define	_FEXPLEN	8		/* Bits in float exponent. */
#define	LN_MAXFLOAT	88.722839	/* natural log(MAXFLOAT) */
#define	LN_MINFLOAT	-87.336544	/* natural log(MINFLOAT) */
#endif /* ! _MIPS_VALUES_H_ */
