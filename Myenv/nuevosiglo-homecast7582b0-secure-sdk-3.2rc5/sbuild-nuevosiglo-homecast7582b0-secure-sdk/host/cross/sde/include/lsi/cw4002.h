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
 * lsi/cw4002.h: definitions for LSI Logic CW4002
 */


#ifndef _CW4002_H_
#define _CW4002_H_

#ifdef __cplusplus
extern "C" {
#endif

/* cw400x + dbx module */
#include <lsi/cw400x.h>

#ifdef __ASSEMBLER__
/* 
 * DBX debug unit registers
 * N.B. Use mfd/mtd instructions to access these registers
 */
#define C0_DCS		$7
#define C0_BPC		$18
#define C0_BDA		$19
#define C0_BPCM		$20
#define	C0_BDAM		$21

$dcs		=	$7
$bpc		=	$18
$bda		=	$19
$bpcm		=	$20
$bdam		=	$21

#else

#define C0_DCS		7
#define C0_BPC		18
#define C0_BDA		19
#define C0_BPCM		20
#define	C0_BDAM		21

/* CW400X-specific dbx module register access */

#if __mips16
extern reg32_t	_cw400x_mfd (int);
extern void	_cw400x_mtd (int, reg32_t);
#else
#define _cw400x_mfd(reg) \
({ \
  register reg32_t __r; \
  __asm__ __volatile ("mfd %0,$%1" \
		      : "=d" (__r) \
      		      : "JK" (reg)); \
  __r; \
})

#define _cw400x_mtd(reg, val) \
do { \
    __asm__ __volatile ("%(mtd %z0,$%1; nop; nop; nop%)" \
			: \
			: "dJ" ((reg32_t)(val)), "JK" (reg) \
			: "memory");\
} while (0)
#endif

#define cw400x_getdcs()		_cw400x_mfd(7)
#define cw400x_setdcs(v)	_cw400x_mtd(7,v)

#define cw400x_getbpc()		_cw400x_mfd(18)
#define cw400x_setbpc(v)	_cw400x_mtd(18,v)

#define cw400x_getbda()		_cw400x_mfd(19)
#define cw400x_setbda(v)	_cw400x_mtd(19,v)

#define cw400x_getbpcm()	_cw400x_mfd(20)
#define cw400x_setbpcm(v)	_cw400x_mtd(20,v)

#define cw400x_getbdam()	_cw400x_mfd(21)
#define cw400x_setbdam(v)	_cw400x_mtd(21,v)

#endif

/* Debug Control and Status register */
#define DCS_TR		0x80000000	/* Trap enable */
#define DCS_UD		0x40000000	/* User debug enable */
#define DCS_KD		0x20000000	/* Kernel debug enable */
#define DCS_TE		0x10000000	/* Trace enable */
#define DCS_DW		0x08000000	/* Enable data breakpoints on write */
#define DCS_DR		0x04000000	/* Enable data breakpoints on read */
#define DCS_DAE 	0x02000000	/* Enable data addresss breakpoints */
#define DCS_PCE 	0x01000000	/* Enable instruction breakpoints */
#define DCS_DE		0x00800000	/* Debug enable */
#if #cpu(cw4002) || #cpu(tr4101)
#define DCS_IBD		0x00400000	/* Internal break disable */
#endif
#if #cpu(cw4002)
#define DCS_EBE		0x00200000	/* External break enable */
#endif
#define DCS_T		0x00000020	/* Trace, set by CPU */
#define DCS_W		0x00000010	/* Write reference, set by CPU */
#define DCS_R		0x00000008	/* Read reference, set by CPU */
#define DCS_DA		0x00000004	/* Data address, set by CPU */
#define DCS_PC		0x00000002	/* Program counter, set by CPU */
#define DCS_DB		0x00000001	/* Debug, set by CPU */

#ifdef __cplusplus
}
#endif
#endif /* _CW4002_H_ */
