/*
 * Copyright (c) 1999-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * lsi/tr4102.h: definitions for LSI Logic TinyRISC TR4102
 */


#ifndef _TR4102_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _TR4102_H_

/* tr4102 = tr4101 + r3000 mmu + extensions */
#if !#mmu(no)
/* XXX is this true, or is it like the cw401x */
#assert mmu(r3k)
#endif
#include <lsi/tr4101.h>

/* BSYS_SCR register gains some extra bits */
#define SCR_MMU  	(1<<31)		/* mmu enable? */
#define SCR_DSNPEN 	(1<<29)		/* data cache snoop enable */
#define SCR_ISNPEN 	(1<<28)		/* insn cache snoop enable */
#define SCR_INTCONDMASK (15<<16)
#define SCR_CPC3EN	(1<<19) 	/* connect Cpc3 to int */
#define SCR_CPC2EN	(1<<18) 	/* connect Cpc2 to int */
#define SCR_CPC1EN	(1<<17) 	/* connect Cpc1 to int */
#define SCR_CPC0EN	(1<<16) 	/* connect Cpc0 to int */

/* 
 * New system configuration #2 
 */
#define BSYS_SCR2	0xbfff0004	

#define SCR2_TMRIE1 	(1<<21) /* TINT1E */
#define SCR2_TMRIE0 	(1<<20) /* TINT0E */
#define SCR2_MRR 	(1<<19)
#define SCR2_4101C 	(1<<17)
#define SCR2_MDUSATE  	(1<<16)
#define SCR2_MDUSATI  	(1<<15)
#define SCR2_FDBSTOP  	(1<<14)
#define SCR2_CLKDBMASK  (15<<7)
#define SCR2_CLKDDMASK  (3<<5)
#define SCR2_CLKSEL  	(1<<4)
#define SCR2_CLKPLLE  	(1<<3)
#define SCR2_CLKST  	(1<<2)
#define SCR2_CLKDCMASK  (3<<0)

/* Timer gains some extra control bits */
#define TMODE_E0	(1<<0)	/* timer enable */
#define TMODE_P0	(1<<1)	/* pulse (vs toggle) mode */
#define TMODE_M0	(1<<2)	/* watchdog mode */
#define TMODE_I0	(1<<3)	/* invert output */
#define TMODE_S0	(1<<4)	/* stop on zero */
#define TMODE_D0	(1<<5)	/* stop when debug mode */
#define TMODE_MASK0      0x3f

#define TMODE_E1	(1<<8)	/* timer enable */
#define TMODE_P1	(1<<9)	/* pulse (vs toggle) mode */
#define TMODE_M1	(1<<10)	/* watchdog mode */
#define TMODE_I1	(1<<11)	/* invert output */
#define TMODE_S1	(1<<12)	/* stop on zero */
#define TMODE_D1	(1<<13)	/* stop when debug mode */
#define TMODE_MASK1     (0x3f<<8)

/* FBUS */
#define M_FACFG0 0xbfff0400	/* address range 0 */
#define M_FACFG1 0xbfff0404	/* address range 1 */
#define M_FACFG2 0xbfff0408	/* address range 2 */
#define M_FACFG3 0xbfff040c	/* address range 3 */
#define M_FACFG4 0xbfff0410	/* address range 4 */
#define M_FACFG5 0xbfff0414	/* address range 5 */
#define M_FBUSTA 0xbfff0418	/* Bus Turnaround */
#define M_FACMP0 0xbfff0440	/* address range 0 comparitor */
#define M_FACMP1 0xbfff0444	/* address range 1 comparitor */
#define M_FACMP2 0xbfff0448	/* address range 2 comparitor */
#define M_FACMP3 0xbfff044c	/* address range 3 comparitor */
#define M_FACMP4 0xbfff0450	/* address range 4 comparitor */
#define M_FACMP5 0xbfff0454	/* address range 5 comparitor */
#define M_FBUSCMP 0xbfff0470	/* address compare register for Fbus */
#define M_FBUSAC 0xbfff0474	/* address compare register for memory etc */
#define M_FBUSCFG 0xbfff0480	/* config for non 0-5 range  */
#define M_FBURST 0xbfff0484	/* timing for burst mode  */
#define M_FSFLASH 0xbfff0488	/* timing for flash */
#define M_FSDRAM 0xbfff0490	/* cfg for edo sdram */
#define M_FDRAMT 0xbfff0494	/* timing for edo dram */
#define M_FSDRAMT 0xbfff0498	/* timing for sdram */
#define M_FSDRAM2 0xbfff049c	/* cfg for sdram */
#define M_FGPCFG 0xbfff04c0	/* cfg for gpio */
#define M_FGPOUTPUT 0xbfff04c4	/* output for gpio */
#define M_FGPINPUT 0xbfff04c8	/* input for gpio */

#define FACFG_WIDTH_8	(0<<30)
#define FACFG_WIDTH_16	(1<<30)
#define FACFG_WIDTH_32	(2<<30)
#define FACFG_GPSEL	(1<<29)
#define FACFG_RAMFLSH	(1<<28)
#define FACFG_RDADMASK	(7<<25)
#define FACFG_RDDDMASK	(3<<23)
#define FACFG_WRADMASK	(7<<20)
#define FACFG_WRDDMASK	(3<<18)
#define FACFG_CSRADMASK	(7<<15)
#define FACFG_CSWADMASK	(7<<12)
#define FACFG_CSRDDMASK	(3<<10)
#define FACFG_CSWDDMASK	(3<<8)
#define FACFG_BURST	(1<<7)
#define FACFG_XACK	(1<<6)
#define FACFG_WAIT_15	(15<<1)
#define FACFG_WAITMASK	(0x1f<<1)
#define FACFG_EN	(1<<0)

#define FACMP_MASK  	(0x1fff<<16)
#define FACMP_ADDRMASK   0xffff

#if 0 /* FACMP examples */
00001fbc        64KB starting at 1fbc0000 (default boot prom)
001f1fc0	2MB starting at 1fc00000   (boot prom)
00ff0000	16MB starting at 00000000  (SDRAM for Primary RAM)
07ff0000	128MB starting at 00000000  (SDRAM for Primary RAM)
00010000        128KB starting at 00000000 (SRAM for Primary RAM)
00ff0100	16MB starting at 01000000  (SDRAM at a1000000)
00010100	128KB starting at 01000000  (SRAM at a1000000)
00010e00	128KB starting at 0e000000  (SRAM at ae000000)
00001fbf        64KB starting at 1fbf0000
00001e00        64KB starting at 1e000000 (UART at be000000)
00001c00        64KB starting at 1c000000 (Ether at bc000000)
#endif

#ifdef __cplusplus
}
#endif
#endif /* _TR4102_H_ */
