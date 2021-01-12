/*
 * Copyright (c) 1997-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * r5kc1.h : SDE R5000 coprocessor 1 (fpa) definitions
 */


#ifndef _R5KC1_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _R5KC1_H_

#ifndef __ASSEMBLER__

typedef unsigned long long _fpreg_t;

struct fpactx {
    unsigned	sr;		/* floating-point status register */
    unsigned	dummy;
    _fpreg_t	regs[32];	/* floating-point registers $f0-$f31 */
#if #cpu(r54xx)
    _fpreg_t	acl;		/* MDMX accumulator (low 64 bits) */
    _fpreg_t	acm;		/* MDMX accumulator (middle 64 bits) */
    _fpreg_t	ach;		/* MDMX accumulator (high 64 bits) */
#endif
};

#ifdef __STDC__
int	 fpa_enable(int);	/* enable fpa, return 1 if present (always) */
unsigned fpa_getrid(void);	/* get fpa revision id */
unsigned fpa_getsr(void);	/* get fpa status register */
void	 fpa_setsr(unsigned);
unsigned fpa_xchsr(unsigned);
unsigned fpa_bicsr(unsigned);
unsigned fpa_bissr(unsigned);
void	fpa_save(struct fpactx *);
void	fpa_restore(const struct fpactx *);
#else
int	 fpa_enable();
unsigned fpa_getrid();
unsigned fpa_getsr();
void	 fpa_setsr();
unsigned fpa_xchsr();
unsigned fpa_bicsr();
unsigned fpa_bissr();
void	fpa_save();
void	fpa_restore();
#endif /* __STDC__ */

/* 
 * Define macros to accessing the R3000 coprocessor 1 control registers.
 * Most apart from "set" return the original register value.
 */

#define fpa_getrid() \
({ \
  register unsigned __r; \
  __asm__ __volatile ("cfc1 %0,$0" : "=d" (__r)); \
  __r; \
})

#define fpa_getsr() \
({ \
  register unsigned __r; \
  __asm__ __volatile ("cfc1 %0,$31" : "=d" (__r)); \
  __r; \
})

#define fpa_setsr(val) \
({ \
    register unsigned __r = (val); \
    __asm__ __volatile ("ctc1 %0,$31" : : "d" (__r)); \
    __r; \
})

#define fpa_xchsr(val) \
({ \
    register unsigned __o, __n = (val); \
    __asm__ __volatile ("cfc1 %0,$31" : "=d" (__o)); \
    __asm__ __volatile ("ctc1 %0,$31" : : "d" (__n)); \
    __o; \
})


#define fpa_bissr(val) \
({ \
    register unsigned __o, __n; \
    __asm__ __volatile ("cfc1 %0,$31" : "=d" (__o)); \
    __n = __o | (val); \
    __asm__ __volatile ("ctc1 %0,$31" : : "d" (__n)); \
    __o; \
})


#define fpa_bicsr(val) \
({ \
    register unsigned __o, __n; \
    __asm__ __volatile ("cfc1 %0,$31" : "=d" (__o)); \
    __n = __o &~ (val); \
    __asm__ __volatile ("ctc1 %0,$31" : : "d" (__n)); \
    __o; \
})
#endif /* !ASSEMBLER */

/* 
 * Status Register Values
 */

#define FPA_CSR_COND	0x00800000	/* $fcc0 */
#define FPA_CSR_COND0	0x00800000	/* $fcc0 */
#define FPA_CSR_FLUSH	0x01000000	/* flush to 0 */
#define FPA_CSR_FS	0x01000000	/* flush to 0 */
#define FPA_CSR_COND1	0x02000000	/* $fcc1 */
#define FPA_CSR_COND2	0x04000000	/* $fcc2 */
#define FPA_CSR_COND3	0x08000000	/* $fcc3 */
#define FPA_CSR_COND4	0x10000000	/* $fcc4 */
#define FPA_CSR_COND5	0x20000000	/* $fcc5 */
#define FPA_CSR_COND6	0x40000000	/* $fcc6 */
#define FPA_CSR_COND7	0x80000000	/* $fcc7 */

/*
 * X the exception cause indicator
 * E the exception enable
 * S the sticky/flag bit
*/
#define FPA_CSR_ALL_X 0x0003f000
#define FPA_CSR_UNI_X	0x00020000
#define FPA_CSR_INV_X	0x00010000
#define FPA_CSR_DIV_X	0x00008000
#define FPA_CSR_OVF_X	0x00004000
#define FPA_CSR_UDF_X	0x00002000
#define FPA_CSR_INE_X	0x00001000

#define FPA_CSR_ALL_E	0x00000f80
#define FPA_CSR_INV_E	0x00000800
#define FPA_CSR_DIV_E	0x00000400
#define FPA_CSR_OVF_E	0x00000200
#define FPA_CSR_UDF_E	0x00000100
#define FPA_CSR_INE_E	0x00000080

#define FPA_CSR_ALL_S	0x0000007c
#define FPA_CSR_INV_S	0x00000040
#define FPA_CSR_DIV_S	0x00000020
#define FPA_CSR_OVF_S	0x00000010
#define FPA_CSR_UDF_S	0x00000008
#define FPA_CSR_INE_S	0x00000004

/* rounding mode */
#define FPA_CSR_RN	0x0	/* nearest */
#define FPA_CSR_RZ	0x1	/* towards zero */
#define FPA_CSR_RU	0x2	/* towards +Infinity */
#define FPA_CSR_RD	0x3	/* towards -Infinity */

#ifdef __ASSEMBLER__

	.struct 0
FPACTX_SR:	.word	0
FPACTX_DUMMY:	.word	0
#define FPACTX_RSIZE	8
FPACTX_REGS:	.dword	0:32
#if #cpu(r54xx)
FPACTX_ACL:	.dword	0
FPACTX_ACM:	.dword	0
FPACTX_ACH:	.dword	0
#endif
	.previous

/* control regs */
	$fpa_rid =	$0
	$fpa_sr  =	$31
#endif

#ifdef __cplusplus
}
#endif

#endif /*_R5KC1_H_*/
