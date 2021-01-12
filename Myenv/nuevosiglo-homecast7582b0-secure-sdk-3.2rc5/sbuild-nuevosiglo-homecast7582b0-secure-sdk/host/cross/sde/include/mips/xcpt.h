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
 * xcpt.h: include appropriate SDE xception handling definitions
 */


#ifndef _XCPT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _XCPT_H_

#include <mips/cpu.h>

#ifndef XCPC_GENERAL
#if #cpu(cw401x)
#include <lsi/cwxcpt.h>
#elif #cpu(rm7000)
#include <mips/rm7kxcpt.h>
#elif #cpu(rc3236x)
#include <mips/r4kxcpt.h>
#elif __mips == 32 || __mips == 64
#include <mips/m32xcpt.h>
#elif #cpu(r5000) || __mips >= 4
#include <mips/r5kxcpt.h>
#elif #cpu(r4000) || __mips >= 3 || __mips64
#include <mips/r4kxcpt.h>
#elif __mips
#include <mips/r3kxcpt.h>
#else
#error unknown cpu
#endif
#endif /* XCPC_GENERAL */

#ifndef __ASSEMBLER__
extern const char *const _sys_xcptlist[];

#ifdef __STDC__
typedef int (*xcpt_t)(int, struct xcptcontext *);
#else
typedef int (*xcpt_t)();
#endif

typedef reg_t xcptjmp_buf[XJB_SIZE/sizeof(reg_t)];

/*
 * Structure used to define low-level xception handler.
 */
struct xcptaction {
    xcpt_t	xa_handler;
    unsigned 	xa_flags;	/* must be zero */
};

/* Bitmap of interrupts (max 64) */
typedef unsigned long long intrset_t;

/*
 * Structure used to define mid-level interrupt handler.
 */
struct intraction {
    xcpt_t	ia_handler;	/* handler function */
    int		ia_arg;		/* passed to handler */
#ifdef NEWINTRUPT
    intrset_t	ia_mask;	/* new mask to apply */
#else
    unsigned	ia_ipl;
#endif
    unsigned 	ia_flags;	/* see below */
};

/* interrupt configuration for intrsettype() */
typedef struct {
    enum {
	INTRTYPE_LEVEL_LO,
	INTRTYPE_LEVEL_HI,
	INTRTYPE_PULSE_LO,
	INTRTYPE_PULSE_HI,
	INTRTYPE_EDGE_FALLING,
	INTRTYPE_EDGE_RISING
    } mode;
    /* other add useful could be added here */
} intrtype_t;


#define XCPT_DFL	(xcpt_t)0
#define XCPT_ERR	(xcpt_t)-1

/* states returned from xcptstackinfo */
enum  XcptStackTraceStates {
    XcptStackTracePC,		/* Valid PC */
    XcptStackTraceLast,		/* Stacktrace completed */
    XcptStackTraceLoop,		/* Stacktrace completed - loop detected */
    XcptStackTraceBadPC,	/* Stacktrace completed - bad PC */
    XcptStackTraceNA,		/* Not available */
};


/* install exception handlers */
extern int xcptaction (int, struct xcptaction *, struct xcptaction *);
extern xcpt_t xcption (int, xcpt_t);

/* setjmp/longjmp free of floating-point, but with interrupt mask */
extern int xcptsetjmp (xcptjmp_buf);
extern volatile void xcptlongjmp (xcptjmp_buf, int);

/* return to a different exception context */
extern void xcptrestore (struct xcptcontext *xcp) __attribute__ ((noreturn));

/* display an exception context */
extern void xcpt_show (struct xcptcontext *);

/* display a stack backtrace */
extern enum XcptStackTraceStates xcptstackinfo (struct xcptcontext *xcp,
						int atdepth, void **vap);
extern void xcptstacktrace (struct xcptcontext *);
extern void _xcptstackinfo_load (void);	/* install full code */

/* similar routines for interrupt handling */
extern int intraction (unsigned int, const struct intraction *, 
		       struct intraction *);
extern xcpt_t intrupt (unsigned int, xcpt_t, int);

/* test for pending interrupts */
#ifdef NEWINTRUPT
extern int intrpending (intrset_t *);
#else
extern int intrpending (unsigned int);
#endif

/* manipulate interrupt mask */
extern int intrprocmask (int, const intrset_t *, intrset_t *);

/* configure interrupt pin */
extern int intrtype (int, const intrtype_t *, intrtype_t *);

/* set priority level (*nix style) */
#ifndef _KERNEL
extern unsigned int spln (unsigned int);
extern unsigned int splx (unsigned int);
#endif

/* software interrupt (0/1) on/off */
extern void siron (unsigned int);
extern void siroff (unsigned int);

#endif /*!__ASSEMBLER__*/

/* macros for interrupt mask manipulation */
#define	intraddset(set, intrno)	(*(set) |= 1ULL << ((intrno)-1), 0)
#define	intrdelset(set, intrno)	(*(set) &= ~(1ULL << ((intrno)-1)), 0)
#define	intremptyset(set)	(*(set) = 0, 0)
#define	intrfillset(set)	(*(set) = ~(intrset_t)0, 0)
#define	intrismember(set, intrno) ((*(set) & (1ULL << ((intrno)-1))) != 0)

/* how argument for intrprocmask() */
#define	INTR_DISABLE	1	/* disable specified interrupt set */
#define	INTR_ENABLE	2	/* enable specified interrupt set */
#define	INTR_SETMASK	3	/* set specified interrupt enables */

/* xcptaction/intraction flags */
#define XA_SHORT	0x01	/* no 's' regs in xcptcontext */
#define XA_NO_SREG	0x01	/* no 's' regs in xcptcontext */
#define XA_ASM		0x02	/* assembler-level handler */

#ifdef __cplusplus
}
#endif
#endif /*_XCPT_H_*/
