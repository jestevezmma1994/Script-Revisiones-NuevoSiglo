/*
 * Copyright (c) 2002-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * mips/amethyst.h : MIPS "Amethyst" 25Kf family coprocessor 0 definitions
 */


#ifndef _AMETHYST_H_
#define _AMETHYST_H_

#ifdef __cplusplus
extern "C" {
#endif

/* MIPS64 compliant */
#include <mips/m64c0.h>

/*
 * Processor-specific fields in Config0 Register  (CP0 Register 16, Select 0)
 */
#define CFG0_EC		0x78000000	/* External clock ratio */
#define CFG0_SM		0x02000000	/* Sync Mode (Fast/Safe) */
#define CFG0_TI		0x01000000	/* Timer Disable */
#define CFG0_TD		0x00800000	/* Test Done */
#define CFG0_TF		0x00300000	/* Test Fail */

#define CFG_ECMASK	CFG0_EC
#define CFG_ECSHIFT	27

/*
 * Processor-specific fields in Debug Register  (CP0 Register 23, Select 0)
 */
#define DBG_DBD		0x80000000	/* branch delay slot */
#define DBG_DM		0x40000000	/* in debug mode */
#define DBG_NoDCR	0x20000000	/* dseg is not present */
#define DBG_LSNM	0x10000000	/* load/store dseg/remain */
#define DBG_DOZE	0x08000000	/* was in low power mode */
#define DBG_HALT	0x04000000	/* system bus clock was running */
#define DBG_COUNTDM	0x02000000	/* count reg counts in debug mode */
#define DBG_IBusEP	0x01000000	/* instruction bus error pending */
#define DBG_MCheckP	0x00800000	/* machine check pending */
#define DBG_CacheEP	0x00400000	/* cache error pending */
#define DBG_DBusEP	0x00200000	/* data bus error pending */
#define DBG_IEXI	0x00100000	/* imprecise exception inhibit */
#define DBG_DDBLimpr	0x00040000	/* debug data break imprecise */
#define DBG_VERMASK	0x00038000	/* EJTAG version */
#define DBG_VERSHIFT	15
#define DBG_DEXCMASK	0x00007c00	/* latest exception code */
#define DBG_DEXCSHIFT	10
#define DBG_NoSST	0x00000200	/* no single-step */
#define DBG_SST		0x00000100	/* single-step enable */
#define DBG_DINT	0x00000020	/* debug interrupt exception */
#define DBG_DIB		0x00000010	/* debug instruction break */
#define DBG_DDBS	0x00000008	/* debug data break on store */
#define DBG_DDBL	0x00000004	/* debug data break on load */
#define DBG_DBP		0x00000002	/* debug breakpoint */
#define DBG_DSS		0x00000001	/* debug single-step */

/* Performance counter event numbers for 
   PerfCnt Registers (CP0 Register 25, Select 0 and 1) */
#define PERFCNT_EV_CYCLES	0
#define PERFCNT_EV_IDISPATCH	1
#define PERFCNT_EV_FPU		2
#define PERFCNT_EV_ALU		3
#define PERFCNT_EV_LOAD		4
#define PERFCNT_EV_STORE	5
#define PERFCNT_EV_BRANCH	6
#define PERFCNT_EV_DUALISSUE	7
#define PERFCNT_EV_COMPLETE	8
#define PERFCNT_EV_FETCHGROUP	9
#define PERFCNT_EV_FPUCOMPLETE	10
#define PERFCNT_EV_FPUEXCEPT	11
#define PERFCNT_EV_FPUXPREDICT	12
#define PERFCNT_EV_BRMISPREDICT	13
#define PERFCNT_EV_BRCOMPLETE	14
#define PERFCNT_EV_JRMISPREDICT	15
#define PERFCNT_EV_JRCOMPLETE	16
#define PERFCNT_EV_UTLBMISS	17
#define PERFCNT_EV_ITLBMISS	18
#define PERFCNT_EV_DTLBMISS	19
#define PERFCNT_EV_JTLBEXCEPT	20
#define PERFCNT_EV_ITLBEXCEPT	21
#define PERFCNT_EV_DTLBEXCEPT	22
#define PERFCNT_EV_ITLBREFILL	23
#define PERFCNT_EV_ICFETCH	24
#define PERFCNT_EV_ICFILL	25
#define PERFCNT_EV_ICMISS	26
#define PERFCNT_EV_DCMISS	27
#define PERFCNT_EV_DCWRITEBACK	28
#define PERFCNT_EV_DCFETCH	29
#define PERFCNT_EV_SCMISS	30
#define PERFCNT_EV_SCWRITEBACK	31
#define PERFCNT_EV_SCFETCH	32
#define PERFCNT_EV_REPLAY	33
#define PERFCNT_EV_LSUREPLAY	34
#define PERFCNT_EV_SPECREPLAY	35
#define PERFCNT_EV_WBBREPLAY	36
#define PERFCNT_EV_FSBREPLAY	37
#define PERFCNT_EV_ICPSEUDO	38
#define PERFCNT_EV_LDSTISSUE	39

/* ErrCtl Register (CP0 Register 26, Select 0) */
#define ERRCTL_PE	0x80000000	/* parity enable */
#define ERRCTL_PO	0x40000000	/* parity overwrite */
#define ERRCTL_WST	0x20000000	/* way select / tag array */
#define ERRCTL_P	0x000000ff	/* parity bits */

/*
 * CacheErr register (CP Register 27, Select 0)
 */
#define CACHEERR_ER	        0x80000000	/* 0 = Instr, 1 = Data */
#define CACHEERR_ED		0x20000000	/* data ram error */
#define CACHEERR_ET		0x10000000	/* tag ram error */
#define CACHEERR_EB		0x02000000	/* both i & d cache error */
#define CACHEERR_EF		0x01000000	/* fatal error */
#define CACHEERR_EW		0x00400000	/* way selection error */
#define CACHEERR_IDX_MASK	0x00007fe0	/* cache index */
#define CACHEERR_IDX_SHIFT	0

/* MIPS SDE historical names */
#define CACHEERR_TYPE		CACHEERR_ER
#define CACHEERR_LEVEL		0		/* cache level */
#define CACHEERR_DATA		CACHEERR_ED
#define CACHEERR_TAG		CACHEERR_ET
#define CACHEERR_MISS		0		/* error on cache miss */
#define CACHEERR_BUS		0		/* bus error */
#define CACHEERR_BOTH		CACHEERR_EB

#ifdef __cplusplus
}
#endif
#endif /* _AMETHYST_H_ */
