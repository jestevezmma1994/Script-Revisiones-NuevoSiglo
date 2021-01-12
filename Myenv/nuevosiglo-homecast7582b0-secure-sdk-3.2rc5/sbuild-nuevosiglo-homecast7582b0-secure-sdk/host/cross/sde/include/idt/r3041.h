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
 * r3041.h: IDT R3041 cp0 registers, and functions to access them.
 */


#ifndef _R3041_H_
#define _R3041_H_

/* generic R3000 definitions */
#include <mips/r3kc0.h>

/* bus control (cp0 $2) */
#define BUSC_LOCK	0x80000000	/* write lock */
#define BUSC_SET	0x60170300	/* must be set (reserved high) */
#define BUSC_MEM_SH	26		/* MemStrobe Control field shift */
#define   BUSC_CTL_HIGH	  0x0		  /* remain high */
#define   BUSC_CTL_WR	  0x1		  /* on write cycles only */
#define   BUSC_CTL_RD	  0x2		  /* on read cycles only */
#define   BUSC_CTL_RDWR	  0x3		  /* on read and write cycles */
#define BUSC_ED_SH	24		/* ExtDataEn Control field shift */
#define BUSC_IO_SH	22		/* IOStrobe  Control field shift */
#define BUSC_BE16	0x00200000	/* enable BE16(1:0) */
#define BUSC_BE		0x00080000	/* enable BE(3:0) */
#define BUSC_BTA_0	0x00000000	/* bus turnaround (>= 0.5 cycles) */
#define BUSC_BTA_1	0x00004000	/* bus turnaround (>= 1.5 cycles) */
#define BUSC_BTA_2	0x00008000	/* bus turnaround (>= 2.5 cycles) */
#define BUSC_BTA_3	0x0000c000	/* bus turnaround (>= 3.5 cycles) */
#define BUSC_DMA	0x00002000	/* DMA protocol control */
#define BUSC_TC		0x00001000	/* TC control */
#define BUSC_BR		0x00000800	/* BR control */

/* cache configuration (cp0 $3) */
#define CCFG_LOCK	0x80000000	/* write lock */
#define CCFG_SET	0x40000000	/* must be set (reserved high) */
#define CCFG_DBR	0x20000000	/* data block refill enable */
#define CCFG_FDM	0x00080000	/* force dcache miss */

/* portsize (cp0 $10) */
#define PRTSZ_LOCK	0x80000000	/* write lock */

/* port width encodings */
#define PRTSZ_32	0x0		/* 32 bit */
#define PRTSZ_8		0x1		/* 8 bit */
#define PRTSZ_16	0x2		/* 16 bit */

/* memory region shift values */
#define PRTSZ_KSEG2B_SH 28
#define PRTSZ_KSEG2A_SH 26
#define PRTSZ_KUSEGD_SH	24
#define PRTSZ_KUSEGC_SH	22
#define PRTSZ_KUSEGB_SH	20
#define PRTSZ_KUSEGA_SH	18
#define PRTSZ_KSEG1H_SH	14
#define PRTSZ_KSEG1G_SH	12
#define PRTSZ_KSEG1F_SH	10
#define PRTSZ_KSEG1E_SH	8
#define PRTSZ_KSEG1D_SH	6
#define PRTSZ_KSEG1C_SH	4
#define PRTSZ_KSEG1B_SH	2
#define PRTSZ_KSEG1A_SH	0


/* extra CP0 registers */

#ifdef __ASSEMBLER__

#define C0_BUSCTRL	$2
#define C0_CACHECFG	$3
#define C0_COUNT	$9
#define C0_PORTSIZE	$10
#define C0_COMPARE	$11

$busctrl	=	$2
$cachecfg	=	$3
$count		=	$9
$portsize	=	$10
$compare	=	$11

#else

#define C0_BUSCTRL	2
#define C0_CACHECFG	3
#define C0_COUNT	9
#define C0_PORTSIZE	10
#define C0_COMPARE	11

#define r3041_getbusc()		_mips_mfc0(2)
#define r3041_setbusc(v)	_mips_mtc0(2,v)
#define r3041_xchbusc(v)	_mips_mxc0(2,v)
#define r3041_bicbusc(v)	_mips_bcc0(2,v)
#define r3041_bisbusc(v)	_mips_bsc0(2,v)
#define r3041_bcsbusc(c,s)	_mips_bcsc0(2,c,s)

#define r3041_getccfg()		_mips_mfc0(3)
#define r3041_setccfg(v)	_mips_mtc0(3,v)
#define r3041_xchccfg(v)	_mips_mxc0(3,v)
#define r3041_bicccfg(v)	_mips_bcc0(3,v)
#define r3041_bisccfg(v)	_mips_bsc0(3,v)
#define r3041_bcsccfg(c,s)	_mips_bcsc0(3,c,s)

#define r3041_getprtsz()	_mips_mfc0(10)
#define r3041_setprtsz(v)	_mips_mtc0(10,v)
#define r3041_xchprtsz(v)	_mips_mxc0(10,v)

#endif /* __ASSEMBLER__ */

#endif /* _R3041_H_ */
