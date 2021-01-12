/*
 * Copyright (c) 1998-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * lsi/cw400x.h : SDE LSI CoreWare 400x definitions
 */


#ifndef _CW400X_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _CW400X_H_

/*
 * The cw400x core is essentially an R3000
 * So far, none of them have a TLB
 */
#if !#mmu(r3k) && !#mmu(r4k) && !#mmu(cw10)
#assert mmu(no)
#endif
#include <mips/r3kc0.h>

/*
 * Exception Codes
 */
#undef	EXC_RESD
#define EXC_TR		0xd	  /* trap */                 

/*
 * Status Register 
 */

/* diagnostic field */
#undef	SR_ISC
#undef	SR_SWC
#undef	SR_PZ
#undef	SR_CM
#undef	SR_PE

#undef	SR_RE


/* System configuration module */
#define BSYS_BASE	0xbfff0000	/* system configuration */
#define BSYS_SCR	0xbfff0000	/* system configuration */
#ifndef __ASSEMBLER__
struct _bsys_module {
    unsigned int scr;			/* system configuration register */
};
#endif

#define SCR_TLBEN	(1<<31)		/* tlb enable */
#define	SCR_W		(1<<30)		/* write buffer enable */
#define SCR_DSNOOP	(1<<29)		/* D-snoop enable */
#define SCR_ISNOOP	(1<<28)		/* I-snoop enable */
#define SCR_FBE		(1<<24)		/* fast bus enable */
#define SCR_CPC3EN	(1<<19) 	/* connect Cpc3 to int */
#define SCR_CPC2EN	(1<<18)		/* connect Cpc2 to int */
#define SCR_CPC1EN	(1<<17) 	/* connect Cpc1 to int */
#define SCR_CPC0EN	(1<<16) 	/* connect Cpc0 to int */
#define SCR_E		(1<<13)		/* Data error */
#define SCR_PS_SHIFT	10
#define SCR_PS_MASK	(7<<SCR_PS_SHIFT)
#define SCR_PS_16	(0<<SCR_PS_SHIFT)	/* page size 16 words */
#define SCR_PS_32	(1<<SCR_PS_SHIFT)	/* page size 32 words */
#define SCR_PS_64	(2<<SCR_PS_SHIFT)	/* page size 64 words */
#define SCR_PS_128	(3<<SCR_PS_SHIFT)	/* page size 128 words */
#define SCR_PS_256	(4<<SCR_PS_SHIFT)	/* page size 256 words */
#define SCR_PS_512	(5<<SCR_PS_SHIFT)	/* page size 512 words */
#define SCR_PS_1024	(6<<SCR_PS_SHIFT)	/* page size 1024 words */
#define SCR_PS_2048	(7<<SCR_PS_SHIFT)	/* page size 2048 words */
#define SCR_CM_SHIFT	8
#define SCR_CM_MASK	(3<<SCR_CM_SHIFT)
#define SCR_CM_NORMAL	(0<<SCR_CM_SHIFT)
#define SCR_CM_IDATA	(1<<SCR_CM_SHIFT)
#define SCR_CM_ITAG	(2<<SCR_CM_SHIFT)
#define SCR_CM_DTAG	(3<<SCR_CM_SHIFT)
#define	SCR_R		(1<<7)		/* read priority */
#define SCR_DRS_SHIFT	5
#define SCR_DRS_MASK	(3<<SCR_DRS_SHIFT)
#define SCR_DRS_1	(0<<SCR_DRS_SHIFT)
#define SCR_DRS_2	(1<<SCR_DRS_SHIFT)
#define SCR_DRS_4	(2<<SCR_DRS_SHIFT)
#define SCR_DRS_8	(3<<SCR_DRS_SHIFT)
#define SCR_D		(1<<4)
#define SCR_IRS_SHIFT	2
#define SCR_IRS_MASK	(3<<SCR_IRS_SHIFT)
#define SCR_IRS_1	(0<<SCR_IRS_SHIFT)
#define SCR_IRS_2	(1<<SCR_IRS_SHIFT)
#define SCR_IRS_4	(2<<SCR_IRS_SHIFT)
#define SCR_IRS_8	(3<<SCR_IRS_SHIFT)
#define SCR_1E		(1<<1)
#define SCR_IE		(1<<0)

/* Optional timer module */
#define TMR_BASE	0xbfff0100	/* timer base */
#define TMR_TIC0	0xbfff0100	/* timer 0 initial count */
#define TMR_TCC0	0xbfff0104	/* timer 0 counter	 */
#define TMR_TIC1	0xbfff0108	/* timer 1 initial count */
#define TMR_TCC1	0xbfff010c	/* timer 1 counter	 */
#define TMR_TMODE	0xbfff0110	/* timer mode		 */
#define TMR_TSTAT	0xbfff0114	/* timer interrupt status*/

#ifndef __ASSEMBLER__
struct _timer_module {
    unsigned int tic0;		/* timer 0 initial count */
    unsigned int tcc0;		/* timer 0 counter	 */
    unsigned int tic1;		/* timer 1 initial count */
    unsigned int tcc1;		/* timer 1 counter	 */
    unsigned int tmode;		/* timer mode		 */
    unsigned int tstat;		/* timer interrupt status*/
};
#endif

/* Timer mode bits */
#define TMODE_DOG1	0x400	/* timer 1 watchpoint mode */
#define TMODE_PULSE1	0x200	/* timer 1 pulse mode */
#define TMODE_EN1	0x100	/* timer 1 enable */
#define TMODE_PULSE0	0x002	/* timer 0 pulse mode */
#define TMODE_EN0	0x001	/* timer 0 enable */

/* Timer ISR bits */
#define TSTAT_INT0	0x2	/* timer 0 interrupt pending */
#define TSTAT_IEN0	0x1	/* timer 0 interrupt enable */

#ifdef __cplusplus
}
#endif
#endif /* _CW400X_H_*/

