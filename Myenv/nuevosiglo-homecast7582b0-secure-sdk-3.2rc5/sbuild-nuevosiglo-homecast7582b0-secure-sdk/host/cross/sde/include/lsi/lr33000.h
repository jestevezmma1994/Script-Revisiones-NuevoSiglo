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
 * lsi/lr33000.h: defines for LSI Logic LR33000
 */


#ifndef _LR33000_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _LR33000_H_

/* generic R3000 definitions */
#include <mips/r3kc0.h>

#define DEBUG_VECT	0x040
#define K0_DEBUG_VECT	(KSEG0_BASE + DEBUG_VECT)
#define K1_DEBUG_VECT	(KSEG1_BASE + DEBUG_VECT)

/* definitions for DCIC register bits */
#define DCIC_TR	0x80000000	/* Trap enable */
#define DCIC_UD	0x40000000	/* User debug enable */
#define DCIC_KD	0x20000000	/* Kernel debug enable */
#define DCIC_TE	0x10000000	/* Trace enable */
#define DCIC_DW	0x08000000	/* Enable data breakpoints on write */
#define DCIC_DR	0x04000000	/* Enable data breakpoints on read */
#define DCIC_DAE 0x02000000	/* Enable data addresss breakpoints */
#define DCIC_PCE 0x01000000	/* Enable instruction breakpoints */
#define DCIC_DE	0x00800000	/* Debug enable */
#define DCIC_D	0x00002000	/* Data cache invalidate enable */
#define DCIC_I	0x00001000	/* Instruction cache invalidate enable */
#define DCIC_T	0x00000020	/* Trace, set by CPU */
#define DCIC_W	0x00000010	/* Write reference, set by CPU */
#define DCIC_R	0x00000008	/* Read reference, set by CPU */
#define DCIC_DA	0x00000004	/* Data address, set by CPU */
#define DCIC_PC	0x00000002	/* Program counter, set by CPU */
#define DCIC_DB	0x00000001	/* Debug, set by CPU */


/* Define counter/timer register addresses */
#define M_TIC1	0xfffe0000	/* timer 1 initial count */
#define M_TC1	0xfffe0004	/* timer 1 control	 */
#define M_TIC2	0xfffe0008	/* timer 2 initial count */
#define M_TC2	0xfffe000c	/* timer 2 control	 */
#define M_RTIC	0xfffe0010	/* refresh timer	 */
#define M_CFGREG	0xfffe0020	/* configuration reg	 */

/* Definitions for counter/timer control register bits */
#define TC_CE	 0x00000004	/* count enable */
#define TC_IE	 0x00000002	/* interrupt enable */
#define TC_INT 	 0x00000001	/* interrupt request */

/* Definitions for Configuration register bits */
#define CR_WBE		0x80000000	/* write buffer enable */
#define CR_BEN 		0x40000000	/* block write enable */
#define CR_PGSZMSK 	0x38000000	/* page size mask */
#define CR_PGSZSHFT 	27		/* page size shift amount */
#define CR_IW8 		0x02000000	/* add 8 cycles to IOWAIT */
#define CR_PW8		0x01000000	/* add 8 cycles to PWAIT */
#define CR_ICDISABLE	0x00800000	/* Instruction cache disable */
#define CR_DCDISABLE	0x00400000	/* Data cache disable */
#define CR_IBLK_2	0x00000000	/* Instruction cache block size */
#define CR_IBLK_4	0x00100000	/* Instruction cache block size */
#define CR_IBLK_8	0x00200000	/* Instruction cache block size */
#define CR_IBLK_16	0x00300000	/* Instruction cache block size */
#define CR_IBLKMSK	0x00300000	/* Instruction cache block size */
#define CR_DBLK_2	0x00000000	/* Data cache block size */
#define CR_DBLK_4	0x00040000	/* Data cache block size */
#define CR_DBLK_8	0x00080000	/* Data cache block size */
#define CR_DBLK_16	0x000c0000	/* Data cache block size */
#define CR_DBLKMSK	0x000c0000	/* Data cache block size */
#define CR_IODIS	0x00020000	/* Disable DRDY for I/O addresses */
#define CR_IOWAITSHFT	13		/* I/O wait states */
#define CR_PDIS		0x00001000	/* Disable DRDY for PROM addresses */
#define CR_PWAITSHFT	8		/* PROM wait states */
#define CR_DPEN		0x00000040	/* Enable parity check for DRAM */
#define CR_RDYGEN	0x00000020	/* Disable DRDY for DRAM addresses */
#define CR_BLKFDIS	0x00000010	/* Disable DRAM block refill */
#define CR_RFSHEN	0x00000008	/* Enable refresh generator */
#define CR_RASPCHG	0x00000004	/* Define RAS precharge */
#define CR_CASLNTH	0x00000002	/* Define CAS active time */
#define CR_DRAMEN	0x00000001	/* Enable DRAM controller */

/* extra CP0 registers */

#ifdef __ASSEMBLER__

#define C0_BPC	$3		/* breakpoint on instr */
#define C0_BDA	$5		/* breakpoint on data */
#define C0_DCIC	$7		/* cache control */

$bpc	=	$3
$bda	=	$5
$dcic	=	$7

#else

#define C0_BPC	3		/* breakpoint on instr */
#define C0_BDA	5		/* breakpoint on data */
#define C0_DCIC	7		/* cache control */

#define lr33k_getbpc()		_mips_mfc0(3)
#define lr33k_setbpc(v)		_mips_mtc0(3,v)
#define lr33k_xchbpc(v)		_mips_mxc0(3,v)

#define lr33k_getbda()		_mips_mfc0(5)
#define lr33k_setbda(v)		_mips_mtc0(5,v)
#define lr33k_xchbda(v)		_mips_mxc0(5,v)

#define lr33k_getdcic()		_mips_mfc0(7)
#define lr33k_setdcic(v)	_mips_mtc0(7,v)
#define lr33k_xchdcic(v)	_mips_mxc0(7,v)
#define lr33k_bicdcic(v)	_mips_bcc0(7,v)
#define lr33k_bisdcic(v)	_mips_bsc0(7,v)
#define lr33k_bcsdcic(c,s)	_mips_bcsc0(7,c,s)

#define TIC1   (*((volatile unsigned *)M_TIC1)) /* timer 1 initial count */
#define TC1    (*((volatile unsigned *)M_TC1))  /* timer 1 control	 */
#define TIC2   (*((volatile unsigned *)M_TIC2)) /* timer 2 initial count */
#define TC2    (*((volatile unsigned *)M_TC2))  /* timer 2 control */
#define RTIC   (*((volatile unsigned *)M_RTIC)) /* refresh timer */
#define CFGREG (*((volatile unsigned *)M_CFGREG)) /* configuration reg */

#endif /* __ASSEMBLER */


#ifdef __cplusplus
}
#endif
#endif /* _LR33000_H_ */
