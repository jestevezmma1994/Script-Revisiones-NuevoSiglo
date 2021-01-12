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
 * m32xcpt.h : MIPS32 exception handling
 */


#ifndef _M32XCPT_H_
#define _M32XCPT_H_

#define XCPTINTR	0
#define XCPTMOD		1
#define XCPTTLBL	2
#define XCPTTLBS	3
#define XCPTADEL	4
#define XCPTADES	5
#define XCPTIBE		6
#define XCPTDBE		7
#define XCPTSYS		8
#define XCPTBP		9
#define XCPTRI		10
#define XCPTCPU		11
#define XCPTOVF		12
#define XCPTTRAP	13
#define XCPTFPE		15
#define XCPTIS1		16
#define XCPTIS2		17
#define XCPTCEU		17
#define XCPTC2E		18
#define XCPTRES19	19
#define XCPTRES20	20
#define XCPTRES21	21
#define XCPTMDMX	22
#define XCPTWATCH	23
#define XCPTMCHECK	24
#define XCPTRES25	25
#define XCPTRES26	26
#define XCPTRES27	27
#define XCPTRES28	28
#define XCPTRES29	29
#define XCPTCACHEERR	30	/* not a real exception number */
#define XCPTRES31	31
#define NXCPT		32

/* exception classes */
#define XCPC_GENERAL	0
#define XCPC_TLBMISS	1
/*#define XCPC_XTLBMISS	2*/
#define XCPC_CACHEERR	3
#define XCPC_INTR	4
#define XCPC_DEBUG	5
#define XCPC_CLASS	0x0ff
#define XCPC_USRSTACK	0x100

#if __mips == 64 || __mips64
#define XCP_RSIZE	8
#else
#define XCP_RSIZE	4
#endif

#define XJB_S0		0
#define XJB_S1		1
#define XJB_S2		2
#define XJB_S3		3
#define XJB_S4		4
#define XJB_S5		5
#define XJB_S6		6
#define XJB_S7		7
#define XJB_SP		8
#define XJB_S8		9
#define XJB_RA		10
#define XJB_SR		11
#define XJB_SIZE	(12*XCP_RSIZE)

#ifndef __ASSEMBLER__
struct xcptcontext {
    reg_t	sr;
    reg_t	cr;
    reg_t	epc;
    reg_t	vaddr;

    reg_t	regs[32];
    reg_t	mdlo;
    reg_t	mdhi;
    reg_t	mdex;

    struct xcptcontext	*prev;
    unsigned	xclass;
    unsigned	signo;
    unsigned	srsctl;
};

/* Cache Exception Context */
struct cxcptcontext {
    reg_t	cacheerr;
    reg_t	errpc;
    reg_t	taglo;
    reg_t	taghi;
    reg_t	ecc;
    reg_t	config;
};


#define XCP_STATUS(xcp)		((unsigned)(xcp)->sr)
#define XCP_CAUSE(xcp)		((unsigned)(xcp)->cr)
#define XCP_CODE(xcp)		((XCP_CAUSE(xcp) & CR_XMASK) >> 2)
#define XCP_CLASS(xcp)		((xcp)->xclass & XCPC_CLASS)
#define XCP_USRSTACK(xcp)	((xcp)->xclass & XCPC_USRSTACK)

/* indices into xcptcontext.regs[] */
#define XCP_ZERO	0
#define XCP_AT		1
#define XCP_V0		2
#define XCP_V1		3
#define XCP_A0		4
#define XCP_A1		5
#define XCP_A2		6
#define XCP_A3		7
#define XCP_T0		8
#define XCP_T1		9
#define XCP_T2		10
#define XCP_T3		11
#define XCP_T4		12
#define XCP_T5		13
#define XCP_T6		14
#define XCP_T7		15
#define XCP_S0		16
#define XCP_S1		17
#define XCP_S2		18
#define XCP_S3		19
#define XCP_S4		20
#define XCP_S5		21
#define XCP_S6		22
#define XCP_S7		23
#define XCP_T8		24
#define XCP_T9		25
#define XCP_K0		26
#define XCP_K1		27
#define XCP_GP		28
#define XCP_SP		29
#define XCP_S8		30
#define XCP_RA		31

#else

#if __mips == 64 || __mips64
#define xcpreg	.dword	0
#else
#define xcpreg	.word	0
#endif

/* xcptcontext for assembler */
	.struct 0
XCP_SR:		xcpreg
XCP_CR:		xcpreg
XCP_EPC:	xcpreg
XCP_VADDR:	xcpreg

XCP_ZERO:	xcpreg
XCP_AT:		xcpreg
XCP_V0:		xcpreg
XCP_V1:		xcpreg
XCP_A0:		xcpreg
XCP_A1:		xcpreg
XCP_A2:		xcpreg
XCP_A3:		xcpreg
XCP_T0:		xcpreg
XCP_T1:		xcpreg
XCP_T2:		xcpreg
XCP_T3:		xcpreg
XCP_T4:		xcpreg
XCP_T5:		xcpreg
XCP_T6:		xcpreg
XCP_T7:		xcpreg
XCP_S0:		xcpreg
XCP_S1:		xcpreg
XCP_S2:		xcpreg
XCP_S3:		xcpreg
XCP_S4:		xcpreg
XCP_S5:		xcpreg
XCP_S6:		xcpreg
XCP_S7:		xcpreg
XCP_T8:		xcpreg
XCP_T9:		xcpreg
XCP_K0:		xcpreg
XCP_K1:		xcpreg
XCP_GP:		xcpreg
XCP_SP:		xcpreg
XCP_S8:		xcpreg
XCP_RA:		xcpreg

XCP_MDLO:	xcpreg
XCP_MDHI:	xcpreg
XCP_MDEX:	xcpreg

XCP_PREV:	.word 0
XCP_XCLASS:	.word 0
XCP_SIGNO:	.word 0
XCP_SRSCTL:	.word 0
XCP_SIZE:	
	.previous


/* Cache Exception Context */
	.struct 0
CXCP_CACHEERR:	xcpreg
CXCP_ERRPC:	xcpreg
CXCP_TAGLO:	xcpreg
CXCP_TAGHI:	xcpreg
CXCP_ECC:	xcpreg
CXCP_CONFIG:	xcpreg
CXCP_SIZE:
	.previous

#undef xcpreg

#endif /* __ASSEMBLER__ */

#endif /*_M32XCPT_H_*/
