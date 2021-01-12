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
 * lsi/lr33300.h: defines for LSI Logic LR33300 and LR33310
 */


#ifndef _LR33300_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _LR33300_H_

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
#define DCIC_T	0x00000020	/* Trace, set by CPU */
#define DCIC_W	0x00000010	/* Write reference, set by CPU */
#define DCIC_R	0x00000008	/* Read reference, set by CPU */
#define DCIC_DA	0x00000004	/* Data address, set by CPU */
#define DCIC_PC	0x00000002	/* Program counter, set by CPU */
#define DCIC_DB	0x00000001	/* Debug, set by CPU */

/* wait-state config registers */
#define	SPC_INHIBITMASK	(0xf<<24)
#define	SPC_INHIBITSHFT	24
#define	SPC_EXTGNT	(1<<23)
#define	SPC_16WIDE	(1<<22)
#define	SPC_8WIDE	(1<<21)
#define	SPC_PENA	(1<<20)
#define	SPC_CACHED	(1<<19)
#define	SPC_CSDLYMASK	(3<<17)
#define	SPC_CSDLYSHFT	17
#define	SPC_BLKENA	(1<<16)
#define	SPC_BLKWAITMASK	(7<<13)
#define	SPC_BLKWAITSHFT	13
#define	SPC_RECMASK	(63<<7)
#define	SPC_RECSHFT	7
#define	SPC_WAITENA	(1<<6)
#define	SPC_WAITMASK	(63<<0)
#define	SPC_WAITSHFT	0

/* BIU */
#define	BIU_NOSTR	(1<<17)
#define	BIU_LDSCH	(1<<16)
#define	BIU_BGNT	(1<<15)
#define	BIU_NOPAD	(1<<14)
#define	BIU_RDPRI	(1<<13)
#define	BIU_INTP	(1<<12)
#define	BIU_IS1		(1<<11)
#define	BIU_IS0		(1<<10)
#define	BIU_IBLKSZMASK	(3<<8)
#define	BIU_IBLKSZSHFT	8
#define	BIU_IBLKSZ2	(0<<BIU_IBLKSZSHFT)
#define	BIU_IBLKSZ4	(1<<BIU_IBLKSZSHFT)
#define	BIU_IBLKSZ8	(2<<BIU_IBLKSZSHFT)
#define	BIU_IBLKSZ16	(3<<BIU_IBLKSZSHFT)
#define	BIU_DS		(1<<7)
#define	BIU_DS1		(1<<7)
#define	BIU_DS0		(1<<6)
#define	BIU_DBLKSZMASK	(3<<4)
#define	BIU_DBLKSZSHFT	4
#define	BIU_DBLKSZ2	(0<<BIU_DBLKSZSHFT)
#define	BIU_DBLKSZ4	(1<<BIU_DBLKSZSHFT)
#define	BIU_DBLKSZ8	(2<<BIU_DBLKSZSHFT)
#define	BIU_DBLKSZ16	(3<<BIU_DBLKSZSHFT)
#define	BIU_RAM		(1<<3)
#define	BIU_TAG		(1<<2)
#define	BIU_INV		(1<<1)
#define	BIU_LOCK 	(1<<0)

/* DCR */ 
#define	DRAM_DLP1	(1<<28)
#define	DRAM_SYNC	(1<<27)
#define	DRAM_SCFG	(1<<26)
#define	DRAM_DMARDY	(1<<25)
#define	DRAM_DMABLKMASK	(7<<22)
#define	DRAM_DMABLKSHFT	22
#define	DRAM_DPTHMASK	(3<<20)
#define	DRAM_DPTHSHFT	20
#define	DRAM_RDYW	(1<<19)
#define	DRAM_PGSZMASK	(7<<16)
#define	DRAM_PGSZSHFT	16
#define	DRAM_PGMW	(1<<15)
#define	DRAM_RFWEMASK	(3<<13)
#define	DRAM_RFWESHFT	13
#define	DRAM_RFEN	(1<<12)
#define	DRAM_RDYEN	(1<<11)
#define	DRAM_BFD	(1<<10)
#define	DRAM_PE		(1<<9)
#define	DRAM_RPCMASK	(3<<7)
#define	DRAM_RPCSHFT	7
#define	DRAM_RCDMASK	(3<<5)
#define	DRAM_RCDSHFT	5
#define	DRAM_CS		(1<<4)
#define	DRAM_CLMASK	(7<<1)
#define	DRAM_CLSHFT	1
#define	DRAM_DCE	(1<<0)

/* extra Cause Register definitions */ 
#define CR_BT		0x40000000      /* branch taken */   

/* Define counter/timer register addresses */
#define M_SRAM		0xfffe0100	/* SRAM config reg */
#define M_SPEC0		0xfffe0104
#define M_SPEC1		0xfffe0108
#define	M_SPEC2		0xfffe010c
#define	M_SPEC3		0xfffe0110
#define	M_DRAM		0xfffe0120	/* DRAM config reg */
#define	M_BIU		0xfffe0130

/* extra CP0 registers */

#ifdef __ASSEMBLER__

#define C0_BPC	$3		/* breakpoint on instr */
#define C0_BDA	$5		/* breakpoint on data */
#define C0_TAR	$6		/* target address register */
#define C0_DCIC	$7		/* cache control */
#define C0_BDAM	$9		/* breakpoint data address mask */
#define C0_BPCM	$11		/* breakpoint instruction address mask */

$bpc	=	$3
$bda	=	$5
$tar	=	$6
$dcic	=	$7
$bdam	=	$9
$bpcm	=	$11

#else

#define C0_BPC	3		/* breakpoint on instr */
#define C0_BDA	5		/* breakpoint on data */
#define C0_TAR	6		/* target address register */
#define C0_DCIC	7		/* cache control */
#define C0_BDAM	9		/* breakpoint data address mask */
#define C0_BPCM	11		/* breakpoint instruction address mask */

#define lr33k_getbpc()		r3kc0_get(C0_BPC)
#define lr33k_setbpc(v)		r3kc0_set(C0_BPC,v)
#define lr33k_xchbpc(v)		r3kc0_xch(C0_BPC,v)

#define lr33k_getbda()		r3kc0_get(C0_BDA)
#define lr33k_setbda(v)		r3kc0_set(C0_BDA,v)
#define lr33k_xchbda(v)		r3kc0_xch(C0_BDA,v)

#define lr33k_getbdam()		r3kc0_get(C0_BDAM)
#define lr33k_setbdam(v)	r3kc0_set(C0_BDAM,v)
#define lr33k_xchbdam(v)	r3kc0_xch(C0_BDAM,v)

#define lr33k_getbpcm()		r3kc0_get(C0_BPCM)
#define lr33k_setbpcm(v)	r3kc0_set(C0_BPCM,v)
#define lr33k_xchbpcm(v)	r3kc0_xch(C0_BPCM,v)

#define lr33k_getdcic()		r3kc0_get(C0_DCIC)
#define lr33k_setdcic(v)	r3kc0_set(C0_DCIC,v)
#define lr33k_xchdcic(v)	r3kc0_xch(C0_DCIC,v)
#define lr33k_bicdcic(v)	r3kc0_bic(C0_DCIC,v)
#define lr33k_bisdcic(v)	r3kc0_bis(C0_DCIC,v)

#define SRAM		(*((volatile unsigned long *)M_SRAM))
#define SPEC0		(*((volatile unsigned long *)M_SPEC0))
#define SPEC1		(*((volatile unsigned long *)M_SPEC1))
#define SPEC2		(*((volatile unsigned long *)M_SPEC2))
#define SPEC3		(*((volatile unsigned long *)M_SPEC3))
#define DRAM		(*((volatile unsigned long *)M_DRAM))
#define BIU		(*((volatile unsigned long *)M_BIU))

#endif /* __ASSEMBLER */


#ifdef __cplusplus
}
#endif
#endif /* _LR33300_H_ */
