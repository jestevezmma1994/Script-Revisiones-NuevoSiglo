/*
 * math.h : mathematical function defintions
 * Copyright (c) 1985, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)math.h	8.1 (Berkeley) 6/2/93
 */

#ifndef __MATH_H
#define __MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HUGE_VAL
#if defined(vax) || defined(tahoe)		/* DBL_MAX from float.h */
#define	HUGE_VAL	1.701411834604692294E+38
#else
/* Generate an overflow to create +Inf; the multiply shuts up gcc 1 */
#define	HUGE_VAL	(1e250*1e250)		/* IEEE: positive infinity */
#endif
#endif

#ifndef HUGE
#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
#if defined(vax) || defined(tahoe)
/*
 * HUGE for the VAX and Tahoe converts to the largest possible F-float value.
 * This implies an understanding of the conversion behavior of atof(3).  It
 * was defined to be the largest float so that overflow didn't occur when it
 * was assigned to a single precision number.  HUGE_VAL is strongly preferred.
 */
#define	HUGE	1.701411733192644270E+38		
#else
#define	HUGE	HUGE_VAL
#endif
#endif
#endif

#define	M_E		2.7182818284590452354	/* e */
#define	M_LOG2E		1.4426950408889634074	/* log 2e */
#define	M_LOG10E	0.43429448190325182765	/* log 10e */
#define	M_LN2		0.69314718055994530942	/* log e2 */
#define	M_LN10		2.30258509299404568402	/* log e10 */
#define	M_PI		3.14159265358979323846	/* pi */
#define	M_PI_2		1.57079632679489661923	/* pi/2 */
#define	M_PI_4		0.78539816339744830962	/* pi/4 */
#define	M_1_PI		0.31830988618379067154	/* 1/pi */
#define	M_2_PI		0.63661977236758134308	/* 2/pi */
#define	M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define	M_SQRT2		1.41421356237309504880	/* sqrt(2) */
#define	M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */

#if defined(__GNUC__)
#define __attrconst __attribute__((__const))
#define __pure
#else
#define __attrconst
#define __pure
#endif

__pure double	acos (double) __attrconst;
__pure double	asin (double) __attrconst;
__pure double	atan (double) __attrconst;
__pure double	atan2 (double, double) __attrconst;
__pure double	ceil (double) __attrconst;
__pure double	cos (double) __attrconst;
__pure double	cosh (double) __attrconst;
__pure double	exp (double) __attrconst;
__pure double	fabs (double) __attrconst;
__pure float	fabsf (float) __attrconst;
__pure double	floor (double) __attrconst;
__pure double	fmod (double, double) __attrconst;
	double	frexp (double, int *);
__pure double	ldexp (double, int) __attrconst;
__pure double	log (double) __attrconst;
__pure double	log10 (double) __attrconst;
	double	modf (double, double *);
__pure double	pow (double, double) __attrconst;
__pure double	sin (double) __attrconst;
__pure double	sinh (double) __attrconst;
__pure double	sqrt (double) __attrconst;
__pure float	sqrtf (float) __attrconst;
__pure double	tan (double) __attrconst;
__pure double	tanh (double) __attrconst;

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
__pure double	acosh (double) __attrconst;
__pure double	asinh (double) __attrconst;
__pure double	atanh (double) __attrconst;
	double	cabs();	/* we can't describe cabs()'s argument properly */
__pure double	cbrt (double) __attrconst;
__pure double	copysign (double, double) __attrconst;
__pure double	drem (double, double) __attrconst;
__pure double	erf (double) __attrconst;
__pure double	erfc (double) __attrconst;
__pure double	expm1 (double) __attrconst;
__pure int	finite (double) __attrconst;
__pure double	hypot (double, double) __attrconst;
#if defined(vax) || defined(tahoe)
	double	infnan (int);
#endif
__pure int	isinf (double) __attrconst;
__pure int	isnan (double) __attrconst;
__pure double	j0 (double) __attrconst;
__pure double	j1 (double) __attrconst;
__pure double	jn (int, double) __attrconst;
__pure double	lgamma (double) __attrconst;
__pure double	log1p (double) __attrconst;
__pure double	logb (double) __attrconst;
__pure double	rint (double) __attrconst;
__pure double	scalb (double, int) __attrconst;
__pure double	y0 (double) __attrconst;
__pure double	y1 (double) __attrconst;
__pure double	yn (int, double) __attrconst;
#endif

#undef __pure
#undef __attrconst

#ifdef __cplusplus
}
#endif

#endif 
