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
 * lsi/cw10tlb.h: definitions for LSI Logic Coreware CW401x MMU/TLB
 */


#ifndef _CW10TLB_H_
#define _CW10TLB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* CW401X EntryHi bits */
#define TLBHI_VPNMASK	0xfffff000
#define TLBHI_VPNSHIFT	12
#define TLBHI_PIDMASK	0x000000ff
#define TLBHI_PIDSHIFT	0x00000000

/* CW401X EntryLo bits */
#define TLB_PFNMASK	0x03ffffc0
#define TLB_PFNSHIFT	6
#define TLB_FLAGS	0x0000003f
#define TLB_CMASK	0x00000038
#define TLB_CSHIFT	3
#define TLB_D		0x00000004
#define TLB_V		0x00000002
#define TLB_G		0x00000001

#define TLB_UNCACHED		(2 << TLB_CSHIFT)
#define TLB_WTHRU		(3 << TLB_CSHIFT)
#define TLB_WBACK		(7 << TLB_CSHIFT)

/* CW401X Index bits */
#define TLBIDX_MASK	0x3f
#define TLBIDX_SHIFT	0

/* CW401X Random bits */
#define TLBRAND_MASK	0x3f
#define TLBRAND_SHIFT	0

/* total number of tlb entry pairs (variable per cpu) */
#define NTLBID		64	/* max known */

/* macros to constuct tlbhi and tlblo */
#define mktlbhi(vpn,id)   (((tlbhi_t)(vpn) << TLBHI_VPNSHIFT) | \
			   ((id) << TLBHI_PIDSHIFT))
#define mktlblo(pn,flags) (((tlblo_t)(pn) << TLB_PFNSHIFT) | (flags))

/* and destruct them */
#define tlbhiVpn(hi)	((hi) >> TLBHI_VPNSHIFT)
#define tlbhiId(hi)	((hi) & TLBHI_PIDMASK)
#define tlbloPn(lo)	((lo) >> TLB_PFNSHIFT)
#define tlbloFlags(lo)	((lo) & TLB_FLAGS)

#ifdef __ASSEMBLER__
/* 
 * CW401X virtual memory regions (MIPS SDE only supports 32-bit addressing)
 */
#define	KSEG0_BASE	0x80000000
#define	KSEG1_BASE	0xa0000000
#define	KSEG2_BASE	0xc0000000
#define RVEC_BASE	0xbfc00000	/* reset vector base */

#define KUSEG_SIZE	0x80000000
#define KSEG0_SIZE	0x20000000
#define KSEG1_SIZE	0x20000000
#define KSEG2_SIZE	0x40000000

/* 
 * Translate a kernel virtual address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((pa) | 0x80000000)
#define PA_TO_KVA1(pa)	((pa) | 0xa0000000)

/* translate betwwen KSEG0 and KSEG1 virtual addresses */
#define KVA0_TO_KVA1(v)	((v) | 0x20000000)
#define KVA1_TO_KVA0(v)	((v) & ~0x20000000)

#else /* __ASSEMBLER__ */
/*
 * Standard address types
 */
#ifndef _PADDR_T_DEFINED_
typedef unsigned long		paddr_t;	/* a physical address */
#define _PADDR_T_DEFINED_
#endif
#ifndef _VADDR_T_DEFINED_
typedef unsigned long		vaddr_t;	/* a virtual address */
#define _VADDR_T_DEFINED_
#endif

#if __mips64
typedef unsigned long long	tlbhi_t;	/* the tlbhi field */
#else
typedef unsigned long		tlbhi_t;	/* the tlbhi field */
#endif
typedef unsigned long		tlblo_t;	/* the tlblo field */

/* 
 * CW401X virtual memory regions (MIPS SDE only supports 32-bit addressing)
 */
#define KUSEG_BASE 	((void *)0x00000000)
#define KSEG0_BASE	((void *)0x80000000)
#define KSEG1_BASE	((void *)0xa0000000)
#define KSEG2_BASE	((void *)0xc0000000)
#define RVEC_BASE	((void *)0xbfc00000)	/* reset vector base */

#define KUSEG_SIZE	0x80000000u
#define KSEG0_SIZE	0x20000000u
#define KSEG1_SIZE	0x20000000u
#define KSEG2_SIZE	0x40000000u

/* 
 * Translate a kernel virtual address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((paddr_t)(v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((void *) ((pa) | 0x80000000))
#define PA_TO_KVA1(pa)	((void *) ((pa) | 0xa0000000))

/* translate betwwen KSEG0 and KSEG1 virtual addresses */
#define KVA0_TO_KVA1(v)	((void *) ((unsigned)(v) | 0x20000000))
#define KVA1_TO_KVA0(v)	((void *) ((unsigned)(v) & ~0x20000000))

/* Test for KSEGS */
#define IS_KVA(v)	((int)(v) < 0)
#define IS_KVA0(v)	(((unsigned)(v) >> 29) == 0x4)
#define IS_KVA1(v)	(((unsigned)(v) >> 29) == 0x5)
#define IS_KVA01(v)	(((unsigned)(v) >> 30) == 0x2)
#define IS_KVAS(v)	(((unsigned)(v) >> 29) == 0x6)
#define IS_KVA2(v)	(((unsigned)(v) >> 29) == 0x7)
#define IS_UVA(v)	((int)(v) >= 0)

/* convert register type to address and back */
#define VA_TO_REG(v)	((long)(v))		/* sign-extend 32->64 */
#define REG_TO_VA(v)	((void *)(long)(v))	/* truncate 64->32 */

/*
 * CW401X can set the page size on each TLB entry,
 * we present the default cases here (i.e. with
 * PageMask == 0).
 */
#define VMPGSIZE 	4096			/* min page size = 4K */
#define VMPGSIZEMAX	(16*1024*1024)		/* max page size = 16M */
#define VMPGSHIFT 	12			/* log2(vmpgsize) */
#define VMPGMASK 	(VMPGSIZE-1)

/* virtual address to virtual page number and back */
#define vaToVpn(va)	((reg_t)(va) >> VMPGSHIFT)
#define vpnToVa(vpn)	(void *)((vpn) << VMPGSHIFT)

/* physical address to physical page number and back */
#define paToPn(pa)	((pa) >> VMPGSHIFT)
#define pnToPa(pn)	((paddr_t)((pn) << VMPGSHIFT))

/* 
 * CW401X TLB acccess 
 */
void	cw401x_tlbri (tlbhi_t *, tlblo_t *, unsigned *, unsigned);
void	cw401x_tlbwi (tlbhi_t, tlblo_t, unsigned, unsigned);
void	cw401x_tlbwr (tlbhi_t, tlblo_t, unsigned);
int	cw401x_tlbrwr (tlbhi_t, tlblo_t, unsigned);
int	cw401x_tlbprobe (tlbhi_t, tlblo_t *, unsigned *);
void	mips_tlbinval (tlbhi_t);
void	mips_tlbinvalall (void);

/* CP0 Context register */
#define mips_getcontext()	_mips_mfc0(C0_CONTEXT)
#define mips_setcontext(v)	_mips_mtc0(C0_CONTEXT,v)
#define mips_xchcontext(v)	_mips_mxc0(C0_CONTEXT,v)

/* CP0 EntryHi register */
#define mips_getentryhi()	_mips_mfc0(C0_ENTRYHI)
#define mips_setentryhi(v)	_mips_mtc0(C0_ENTRYHI,v)
#define mips_xchentryhi(v)	_mips_mxc0(C0_ENTRYHI,v)

/* CP0 EntryLo register */
#define mips_getentrylo()	_mips_mfc0(C0_ENTRYLO)
#define mips_setentrylo(v)	_mips_mtc0(C0_ENTRYLO,v)
#define mips_xchentrylo(v)	_mips_mxc0(C0_ENTRYLO,v)

/* CP0 PageMask register */
#define mips_getpagemask()	_mips_mfc0(C0_PAGEMASK)
#define mips_setpagemask(v)	_mips_mtc0(C0_PAGEMASK,v)
#define mips_xchpagemask(v)	_mips_mxc0(C0_PAGEMASK,v)

/* CP0 Wired register */
#define mips_getwired()		_mips_mfc0(C0_WIRED)
#define mips_setwired(v)	_mips_mtc0(C0_WIRED,v)
#define mips_xchwired(v)	_mips_mxc0(C0_WIRED,v)

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif
#endif /* _CW10TLB_H_ */
