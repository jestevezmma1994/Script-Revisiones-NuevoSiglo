/*
 * Copyright (c) 2003 MIPS Technologies, Inc.  All rights reserved.
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
 * mips/mips24k.h : MIPS 24K family coprocessor 0 definitions
 */


#ifndef _MIPS24K_H_
#define _MIPS24K_H_

#ifdef __cplusplus
extern "C" {
#endif

/* MIPS32 compliant */
#include <mips/m32c0.h>

/*
 * Processor-specific fields in Status Register  (CP0 Register 12, Select 0)
 * (Pro cores only)
 */
#define SR_CEE		0x00020000 	/* CorExtend Enable */

/*
 * Processor-specific exceptions in Cause Register (CP0 Register 13, Select 0)
 * (Pro cores only)
 */
#define EXC_CEU		17		/* CorExtend Unusable */

/*
 * Processor-specific fields in Config0 Register  (CP0 Register 16, Select 0)
 */
#define CFG0_K23MASK	0x70000000	/* KSEG2/3 cacheability */
#define CFG0_K23SHIFT	28
#define CFG0_KUMASK	0x0e000000	/* KUSEG cacheability */
#define CFG0_KUSHIFT	25
#define CFG0_UDI	0x00400000	/* UDI present */
#define CFG0_SB		0x00200000	/* SimpleBE bus mode */
#define CFG0_MM		0x00040000	/* write buffer merging enabled */
#define CFG0_BM		0x00010000	/* burst order (0 = incremental) */

/*
 * Processor-specific fields in Config7 Register  (CP0 Register 16, Select 7)
 */
#define CFG7_RPS	0x00000004	/* Return Predicion Stack */
#define CFG7_BP		0x00000002	/* Branch Prediction */
#define CFG7_SL		0x00000001	/* Scheduled Loads */

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
#define DBG_DDBSimpr	0x00080000	/* debug data break imprecise */
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
#define PERFCNT_EV_ICOMPLETE	1
#define PERFCNT_EV_BRANCH_0	2
#define PERFCNT_EV_BMISS_1	2
#define PERFCNT_EV_JR31_0	3
#define PERFCNT_EV_JR31MISS_1	3
#define PERFCNT_EV_JRN31_0	4
#define PERFCNT_EV_ITLB_0	5
#define PERFCNT_EV_ITLBMISS_1	5
#define PERFCNT_EV_DTLB_0	6
#define PERFCNT_EV_DTLBMISS_1	6
#define PERFCNT_EV_JTLBI_0	7
#define PERFCNT_EV_JTLBIMISS_1	7
#define PERFCNT_EV_JTLBD_0	8
#define PERFCNT_EV_JTLBDMISS_1	8
#define PERFCNT_EV_ICACHE_0	9
#define PERFCNT_EV_ICACHEMISS_1	9
#define PERFCNT_EV_DCACHE_0	10
#define PERFCNT_EV_DCACHEWB_1	10
#define PERFCNT_EV_DCACHEMISS	11
#define PERFCNT_EV_EIV		12
#define PERFCNT_EV_EIV_DIRTY_0	13
#define PERFCNT_EV_EIV_CLEAN_1	13
#define PERFCNT_EV_INT_COMPL_0	14
#define PERFCNT_EV_FP_COMPL_1	14
#define PERFCNT_EV_LD_COMPL_0	15
#define PERFCNT_EV_ST_COMPL_1	15
#define PERFCNT_EV_J_COMPL_0	16
#define PERFCNT_EV_M16_COMPL_1	16
#define PERFCNT_EV_NOP_COMPL_0	17
#define PERFCNT_EV_MD_COMPL_1	17
#define PERFCNT_EV_STALLS_0	18
#define PERFCNT_EV_REPLAYS_1	18
#define PERFCNT_EV_SC_COMPL_0	19
#define PERFCNT_EV_SC_FAIL_1	19
#define PERFCNT_EV_PF_COMPL_0	20
#define PERFCNT_EV_PF_HIT_1	20
#define PERFCNT_EV_L2_WB_0	21
#define PERFCNT_EV_L2_ACC_1	21
#define PERFCNT_EV_L2_MISS	22
#define PERFCNT_EV_EXC_0	23
#define PERFCNT_EV_FIXUP_0	24

/* ErrCtl Register (CP0 Register 26, Select 0) */
#define ERRCTL_PE	0x80000000	/* parity enable */
#define ERRCTL_PO	0x40000000	/* parity overwrite */
#define ERRCTL_WST	0x20000000	/* way select / tag array */
#define ERRCTL_PCO	0x08000000	/* precode override */
#define ERRCTL_PCI	0x000007e0	/* instruction precoded bits */
#define ERRCTL_PI	0x00000010	/* parity bits (insn) */
#define ERRCTL_PD	0x0000000f	/* parity bits (data) */

/*
 * CacheErr register (CP Register 27, Select 0)
 */
#define CACHEERR_ER	        0x80000000	/* 0 = Instr, 1 = Data */
#define CACHEERR_EC		0x40000000	/* non-primary cache */
#define CACHEERR_ED		0x20000000	/* data ram error */
#define CACHEERR_ET		0x10000000	/* tag ram error */
#define CACHEERR_ES		0x08000000	/* error source: 1=external */
#define CACHEERR_EE		0x04000000	/* bus error */
#define CACHEERR_EB		0x02000000	/* both i & d cache error */
#define CACHEERR_EF		0x01000000	/* fatal error */
#define CACHEERR_EW		0x00400000	/* way selection error */
#define CACHEERR_IDX_MASK	0x0000ffff	/* cache index */
#define CACHEERR_IDX_SHIFT	0

/* MIPS SDE historical names */
#define CACHEERR_TYPE		CACHEERR_ER
#define CACHEERR_LEVEL		CACHEERR_EC	/* cache level */
#define CACHEERR_DATA		CACHEERR_ED
#define CACHEERR_TAG		CACHEERR_ET
#define CACHEERR_MISS		0		/* error on cache miss */
#define CACHEERR_BUS		CACHEERR_EE	/* bus error */
#define CACHEERR_BOTH		CACHEERR_EB

#ifdef __cplusplus
}
#endif
#endif /* _MIPS24K_H_ */
