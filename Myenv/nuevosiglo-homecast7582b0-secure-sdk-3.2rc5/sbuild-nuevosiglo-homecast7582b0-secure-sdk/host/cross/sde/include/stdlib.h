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
 * stdlib.h : ANSI stdlib definitions
 */


#ifndef __STDLIB_H
#ifdef __cplusplus
extern "C" {
#endif
#define __STDLIB_H

/* 
 * This is a ANSI-C header file only
*/

/* --- Inclusions --- */
#include <stddef.h>		/* wrong -- offsetof again */

/* --- Constants --- */
#define RAND_MAX	0x7fffffff

#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

typedef struct {int quot; int rem;} div_t;
typedef struct {long int quot; long int rem;} ldiv_t;

double	atof	(const char *);
int	atoi	(const char *);
long	atol	(const char *);
double	strtod	(const char *, char **);
long	strtol	(const char *, char **, int);
unsigned long strtoul (const char *, char **, int);
#ifndef __STRICT_ANSI__
long long strtoll (const char *, char **, int);
unsigned long long strtoull (const char *, char **, int);
#endif

/* --- Pseudo-random sequence generation --- */
int	rand	(void);
int	rand_r	(unsigned int *);
void	srand	(unsigned int);
#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
unsigned int arc4random (void);
void	arc4random_stir (void);
void	arc4random_addrandom (unsigned char *, int);
#endif

/* --- Memory management --- */
void	free	(void *);
void	*malloc	(size_t);
void	*realloc (void *, size_t);

void 	*calloc	(size_t, size_t);
void	cfree(void *);

/* system hooks */
void	abort	(void) __attribute__ ((noreturn));
void	exit	(int) __attribute__ ((noreturn));
int	atexit	(void (*)(void));
int	system	(const char *);

/* enviroment hooks */
char	*getenv	(const char *);

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
/* Not ANSI / POSIX defined */
int	putenv	(const char *);
int	setenv (const char *, const char *, int);
void	unsetenv (const char *);
void	deleteenv (const char *);
#endif

  /* --- Searching and sorting --- */
void	*bsearch (const void *, const void *, 
		  size_t, size_t,
		  int (*)(const void *, const void *));

void	qsort	(void *base,
		 size_t, size_t , 
		 int (*)(const void *, const void *));

/* --- Integer arithmetic --- */
int	abs	(int);
div_t	div	(int, int);

long int labs	(long int);
ldiv_t	ldiv	(long int, long int);

/* --- Multibyte characters --- */

/* current UTF has MAX 5 (only 3 required for 16bit wchar_t) 
 * ISO8859 style has 3 (have seen 4) byte code set select sequences
 * this is big enogh for now.
*/
#define MB_CHR_MAX	8 

int mbstowc(wchar_t *pwc,const char *s,size_t n);
int wctomb(char *s,wchar_t wc);
size_t wcstombs(char *s,const wchar_t *pwcs,size_t n);
size_t mbstowcs(wchar_t *pwc,const char *s,size_t n);

/* --- Macros ---
 */
#define atoi(nptr)	((int) strtol((nptr), NULL, 10))
#define atol(nptr)	strtol((nptr), NULL, 10)
#define atof(nptr)	strtod((nptr), NULL)

#ifdef __cplusplus
}
#endif
#endif /* !defined __STDLIB_H */
