/*
 * Copyright (c) 1998-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * lsi/cw401x.h: definitions for LSI Logic CW401x & ATMizer-2
 */


#ifndef _CW401X_H_
#define _CW401X_H_

#ifdef __cplusplus
extern "C" {
#endif

/* CW401x has its own mmu variant, which is a cross between R4000
   and R3000. */
#assert mmu(cw10)
#include <lsi/cw10tlb.h>

/*
 * CW401X Exception Codes
 */
#define EXC_INTR	0	/* interrupt */
#define EXC_MOD		1	/* tlb modification */
#define EXC_TLBL	2	/* tlb miss (load/i-fetch) */
#define EXC_TLBS	3	/* tlb miss (store) */
#define EXC_ADEL	4	/* address error (load/i-fetch) */
#define EXC_ADES	5	/* address error (store) */
#define EXC_BE		6	/* bus error (i-fetch) */
#define EXC_RES7	7
#define EXC_SYS		8	/* system call */
#define EXC_BP		9	/* breakpoint */
#define EXC_RI		10	/* reserved instruction */
#define EXC_CPU		11	/* coprocessor unusable */
#define EXC_OVF		12	/* integer overflow */
#define EXC_TRAP	13	/* trap exception */
#define EXC_RES14	14
#define EXC_FPE		15	/* floating point exception */
#define EXC_RES16	16
#define EXC_RES17	17
#define EXC_RES18	18
#define EXC_RES19	19
#define EXC_RES20	20
#define EXC_RES21	21
#define EXC_RES22	22
#define EXC_RES23	23
#define EXC_RES24	24
#define EXC_RES25	25
#define EXC_RES26	26
#define EXC_RES27	27
#define EXC_RES28	28
#define EXC_RES29	29
#define EXC_RES30	30
#define EXC_RES31	31

/*
 * CW401X Cause Register 
 */
#define CR_BD		0x80000000	/* branch delay */
#define	CR_BT		0x40000000	/* branch taken */
#define CR_CEMASK	0x30000000      /* coprocessor used */
#define CR_CESHIFT	28

/* interrupt pending bits */
#define CR_SINT0	0x00000100 	/* s/w interrupt 0 */
#define CR_SINT1	0x00000200	/* s/w interrupt 1 */
#define CR_HINT0	0x00000400	/* h/w interrupt 0 */
#define CR_HINT1	0x00000800	/* h/w interrupt 1 */
#define CR_HINT2	0x00001000	/* h/w interrupt 2 */
#define CR_HINT3	0x00002000	/* h/w interrupt 3 */
#define CR_HINT4	0x00004000	/* h/w interrupt 4 */
#define CR_HINT5	0x00008000	/* h/w interrupt 5 */

/* alternative interrupt pending bit naming */
#define CR_IP0		0x00000100
#define CR_IP1		0x00000200
#define CR_IP2		0x00000400
#define CR_IP3		0x00000800
#define CR_IP4		0x00001000
#define CR_IP5		0x00002000
#define CR_IP6		0x00004000
#define CR_IP7		0x00008000

#define CR_IMASK	0x0000ff00 	/* interrupt pending mask */
#define CR_XMASK	0x0000007c 	/* exception code mask */
#define CR_XCPT(x)	((x)<<2)


/*
 * CW401X Status Register 
 */
#if #option(r3kcompat)
#define SR_IEC		0x00000001	/* interrupt enable current */
#define SR_KUC		0x00000002	/* kernel mode current */

#define SR_IEP		0x00000004	/* interrupt enable prev */
#define SR_KUP		0x00000008	/* kernel mode prev */

#define SR_IEO		0x00000010	/* interrupt enable old */
#define SR_KUO		0x00000020	/* kernel mode old */

#define SR_IE		SR_IEC		/* generic alias for int enable */
#else
#define SR_IE		0x00000001 	/* interrupt enable */
#define SR_EXL		0x00000002	/* exception level */
#define SR_ERL		0x00000004	/* error level */

#define SR_KSU_MASK	0x00000018	/* ksu mode mask */
#define SR_KSU_USER	0x00000010	/* user mode */
#define SR_KSU_KERN	0x00000000	/* kernel mode */
#endif

/* interrupt mask bits */
#define SR_SINT0	0x00000100	/* enable s/w interrupt 0 */
#define SR_SINT1	0x00000200	/* enable s/w interrupt 1 */
#define SR_HINT0	0x00000400	/* enable h/w interrupt 1 */
#define SR_HINT1	0x00000800	/* enable h/w interrupt 2 */
#define SR_HINT2	0x00001000	/* enable h/w interrupt 3 */
#define SR_HINT3	0x00002000	/* enable h/w interrupt 4 */
#define SR_HINT4	0x00004000	/* enable h/w interrupt 5 */
#define SR_HINT5	0x00008000	/* enable h/w interrupt 6 */

/* alternative interrupt mask naming */
#define SR_IM0		0x00000100
#define SR_IM1		0x00000200
#define SR_IM2		0x00000400
#define SR_IM3		0x00000800
#define SR_IM4		0x00001000
#define SR_IM5		0x00002000
#define SR_IM6		0x00004000
#define SR_IM7		0x00008000

#define SR_IMASK	0x0000ff00

/* diagnostic field */
#define SR_SR		0x00100000	/* soft reset occurred */
#define SR_BEV		0x00400000	/* boot exception vectors */

#define SR_CU0		0x10000000	/* coprocessor 0 enable */
#define SR_CU1		0x20000000	/* coprocessor 1 enable */
#define SR_CU2		0x40000000	/* coprocessor 2 enable */
#define SR_CU3		0x80000000	/* coprocessor 3 enable */


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
#define DCS_T		0x00000020	/* Trace, set by CPU */
#define DCS_W		0x00000010	/* Write reference, set by CPU */
#define DCS_R		0x00000008	/* Read reference, set by CPU */
#define DCS_DA		0x00000004	/* Data address, set by CPU */
#define DCS_PC		0x00000002	/* Program counter, set by CPU */
#define DCS_DB		0x00000001	/* Debug, set by CPU */

/* Configuration and Cache Control Register */
#define CCC_SDB		0x08000000	/* Scan Debug Mode */
#define CCC_ISR1	0x04000000	/* Icache Scratchpad Ram */
#define	CCC_EVI		0x02000000	/* External Vectored Interrupt */
#define	CCC_CMP		0x01000000	/* R3000 compatibility */
#define	CCC_IIE		0x00800000	/* Icache Invalidate Enable */	
#define	CCC_DIE		0x00400000	/* Dcache Invalidate Enable */	
#define	CCC_MUL		0x00200000	/* Multiplier Enable */	
#define	CCC_MAD		0x00100000	/* Multiply-Accumulate Enable */
#define	CCC_TMR		0x00080000	/* Timer i/u enable */	
#define	CCC_BEG		0x00040000	/* Bus Enable Grant */	
#define	CCC_IE0		0x00020000	/* ICache Set 0 Enable */	
#define	CCC_IE1		0x00010000	/* ICache Set 1 Enable */	
#define	CCC_IS		0x0000c000	/* ICache Size */	
#define	 CCC_IS_8K	 0x0000c000
#define	 CCC_IS_4K	 0x00008000
#define	 CCC_IS_2K	 0x00004000
#define	 CCC_IS_1K	 0x00000000
#define	CCC_DE0		0x00002000	/* DCache Set 0 Enable */	
#define	CCC_DE1		0x00001000	/* DCache Set 1 Enable */	
#define	CCC_DS		0x00000c00	/* DCache Size */	
#define	 CCC_DS_8K	 0x00000c00
#define	 CCC_DS_4K	 0x00000800
#define	 CCC_DS_2K	 0x00000400
#define	 CCC_DS_1K	 0x00000000
#define	CCC_IPWE	0x00000200	/* In Page Write Enable */
#define	CCC_IPWS	0x00000180	/* In Page Write Size */
#define	 CCC_IPWS_8K	 0x00000180
#define	 CCC_IPWS_4K	 0x00000100
#define	 CCC_IPWS_2K	 0x00000080
#define	 CCC_IPWS_1K	 0x00000000
#define	CCC_TE		0x00000040	/* TLB Enable */	
#define	CCC_WB		0x00000020	/* Write Back */	
#define	CCC_SR0		0x00000010	/* Scratchpad RAM Mode Set 0 */	
#define	CCC_SR1		0x00000008	/* Scratchpad RAM Mode Set 1 */	
#define	CCC_ISC		0x00000004	/* Isolate Cache Mode */	
#define	CCC_TAG		0x00000002	/* Tag Test Mode */	
#define CCC_INV		0x00000001	/* Invalidate Cache Mode */

/*
 * Memory-mapped configuration registers 
 */
#define M_DRAM_CFG	0xb0000000

#define DRAMCFG_PC	0x40000000	/* precharge command */
#define DRAMCFG_MRS	0x20000000	/* mode register set */
#define DRAMCFG_REF	0x10000000	/* refresh cycle */
#define DRAMCFG_CS	0x01000000	/* clock suspension */
#define DRAMCFG_CL_SHFT		20	/* cache latency shift */
#define DRAMCFG_CL	0x00300000	/* cache latency mask */
#define DRAMCFG_RCD_SHFT	16	/* RAS to CAS period shift */
#define DRAMCFG_RCD	0x00030000	/* RAS to CAS period mask */
#define DRAMCFG_RC_SHFT		12	/* refresh to refresh shift */
#define DRAMCFG_RC	0x0000f000	/* refresh to refresh mask */
#define DRAMCFG_RAS_SHFT	 8	/* RAS shift */
#define DRAMCFG_RAS	0x00000700	/* RAS mask */
#define DRAMCFG_DAL_SHFT	 4	/* data-in to active/refresh shift */
#define DRAMCFG_DAL	0x00000070	/* data-in to active/refresh mask */
#define DRAMCFG_RP3	0x00000002	/* precharge to active */
#define DRAMCFG_DPL2	0x00000001	/* data-in to precharge */

#define M_DRAM_REFRESH	0xb0000004	/* Dram cntrlr refresh reg */
#define M_SCBUS_EA	0xb0100000	/* SCbus bus-error adr reg r/o */

#define M_SCBUS_ST	0xb0100004	/* SCbus bus-error status reg */
#define SCBUSST_BEDE	(1<<1)		/* bus error detect enable */
#define SCBUSST_BERR	(1<<0)		/* bus error detected */

#define M_EXVI		0xb0100008	/* Extn Vectored int reg */
#define EXVI_HEVI	(1<<1)		/* hardware EVInt */
#define EXVI_SEVI	(1<<0)		/* software EVInt */

#define M_CL1		0xa0108000	/* CAS Latency=1 */
#define M_CL2		0xa0110000	/* CAS Latency=2 */
#define M_CL3		0xa0118000	/* CAS Latency=3 */


#ifdef __ASSEMBLER__
/* 
 * CW401X Coprocessor 0 register numbers 
 */
#if !#mmu(no)
#define C0_INDEX	$0
#define C0_INX		$0
#define C0_RANDOM	$1
#define C0_RAND		$1
#define C0_ENTRYLO	$2
#define C0_TLBLO	$2
#define C0_CONTEXT	$4
#define C0_CTXT		$4
#define C0_PAGEMASK	$5
#define C0_WIRED	$6
#define C0_ENTRYHI	$10
#define C0_TLBHI	$10
#endif /* !#mmu(no) */
#define C0_DCS		$7
#define C0_BADVADDR 	$8
#define C0_VADDR 	$8
#define C0_COUNT 	$9
#define C0_COMPARE	$11
#define C0_STATUS	$12
#define C0_SR		$12
#define C0_CAUSE	$13
#define C0_CR		$13
#define C0_EPC 		$14
#define C0_PRID		$15
#define C0_CCC		$16
#define C0_LLADDR	$17
#define C0_BPC		$18
#define C0_BDA		$19
#define	C0_BPCM		$20
#define C0_BDAM		$21
#define C0_ROTATE	$23
#define C0_CMASK	$24
#define C0_ERRPC	$30

#if !#mmu(no)
$index		=	$0
$random		=	$1
$entrylo	=	$2
$context	=	$4
$pagemask	=	$5
$wired		=	$6
$entryhi	=	$10
#endif /* !#mmu(no) */
$dcs		=	$7
$vaddr 		=	$8
$count 		=	$9
$compare	=	$11
$sr		=	$12
$cr		=	$13
$epc 		=	$14
$prid		=	$15
$ccc		=	$16
$lladdr		=	$17
$bpc		=	$18
$bda		=	$19
$bpcm		=	$20
$bdam		=	$21
$rotate		=	$23
$cmask		=	$24
$errpc		=	$30

#else

/*
 * Standard types
 */
typedef unsigned long		reg32_t;	/* a 32-bit register */
typedef unsigned long long	reg64_t;	/* a 64-bit register */
#if (__mips >= 3 && __mips != 32) || __mips64
typedef unsigned long long	reg_t;
typedef signed long long	sreg_t;
#else
typedef unsigned long		reg_t;
typedef signed long		sreg_t;
#endif

/* 
 * CW401X Coprocessor 0 register numbers 
 */
#if !#mmu(no)
#define C0_INDEX	0
#define C0_INX		0
#define C0_RANDOM	1
#define C0_RAND		1
#define C0_ENTRYLO	2
#define C0_TLBLO	2
#define C0_CONTEXT	4
#define C0_CTXT		4
#define C0_PAGEMASK	5
#define C0_WIRED	6
#define C0_ENTRYHI	10
#define C0_TLBHI	10
#endif /* !#mmu(no) */
#define C0_DCS		7
#define C0_BADVADDR 	8
#define C0_VADDR 	8
#define C0_COUNT 	9
#define C0_COMPARE	11
#define C0_STATUS	12
#define C0_SR		12
#define C0_CAUSE	13
#define C0_CR		13
#define C0_EPC 		14
#define C0_PRID		15
#define C0_CCC		16
#define C0_LLADDR	17
#define C0_BPC		18
#define C0_BDA		19
#define	C0_BPCM		20
#define C0_BDAM		21
#define C0_ROTATE	23
#define C0_CMASK	24
#define C0_ERRPC	30

#define _mips_nop() \
  __asm__ __volatile ("%(nop%)" : :) 

#define _mips_sync() \
  __asm__ __volatile ("sync")

/* 
 * Define generic macros for accessing the coprocessor 0 registers.
 * Most apart from "set" return the original register value.
 */

#define _mips_mfc0(reg) \
({ \
  register reg32_t __r; \
  __asm__ __volatile ("mfc0 %0,$%1" \
		      : "=d" (__r) \
      		      : "JK" (reg)); \
  __r; \
})

#define _mips_mtc0(reg, val) \
do { \
    __asm__ __volatile ("%(mtc0 %z0,$%1; nop; nop; nop%)" \
			: \
			: "dJ" ((reg32_t)(val)), "JK" (reg) \
			: "memory");\
} while (0)

/* CW401X-specific CP0 register access */
#define cw401x_getdcs()		_mips_mfc0(7)
#define cw401x_setdcs(v)	_mips_mtc0(7,v)
#define cw401x_xchdcs(v)	_mips_mxc0(7,v)
#define cw401x_bicdcs(clr)	_mips_bcc0(7,clr)
#define cw401x_bisdcs(set)	_mips_bsc0(7,set)
#define cw401x_bcsdcs(c,s)	_mips_bcsc0(7,c,s)

#define cw401x_getccc()		_mips_mfc0(16)
#define cw401x_setccc(v)	_mips_mtc0(16,v)
#define cw401x_xchccc(v)	_mips_mxc0(16,v)
#define cw401x_bicccc(clr)	_mips_bcc0(16,clr)
#define cw401x_bisccc(set)	_mips_bsc0(16,set)
#define cw401x_bcsccc(c,s)	_mips_bsc0(16,c,s)

#define cw401x_getbpc()		_mips_mfc0(18)
#define cw401x_setbpc(v)	_mips_mtc0(18,v)
#define cw401x_xchbpc(v)	_mips_mxc0(18,v)

#define cw401x_getbda()		_mips_mfc0(19)
#define cw401x_setbda(v)	_mips_mtc0(19,v)
#define cw401x_xchbda(v)	_mips_mxc0(19,v)

#define cw401x_getbpcm()	_mips_mfc0(20)
#define cw401x_setbpcm(v)	_mips_mtc0(20,v)
#define cw401x_xchbpcm(v)	_mips_mxc0(20,v)

#define cw401x_getbdam()	_mips_mfc0(21)
#define cw401x_setbdam(v)	_mips_mtc0(21,v)
#define cw401x_xchbdam(v)	_mips_mxc0(21,v)

#define cw401x_getrotate()	_mips_mfc0(23)
#define cw401x_setrotate(v)	_mips_mtc0(23,v)
#define cw401x_xchrotate(v)	_mips_mxc0(23,v)

#define cw401x_getcmask()	_mips_mfc0(24)
#define cw401x_setcmask(v)	_mips_mtc0(24,v)
#define cw401x_xchcmask(v)	_mips_mxc0(24,v)

/*
 * C interface to CW401x instruction set extensions 
 */
#define cw401x_max(a,b) ({ \
    int __a = (a); \
    int __b = (b); \
    int __v; \
    __asm__ ("max %0,%1,%2" : "=d" (__v) : "d" (__a), "d" (__b)); \
    __v; \
})

#define cw401x_min(a,b) ({ \
    int __a = (a); \
    int __b = (b); \
    int __v; \
    __asm__ ("min %0,%1,%2" : "=d" (__v) : "d" (__a), "d" (__b)); \
    __v; \
})

/* addciu: assume that cp0 $cmask register already set (see cw401x_setcmask), 
   otherwise we might just as well do an addiu/and pair */
#define cw401x_addciu(a,i) ({ \
    unsigned int __a = (a); \
    unsigned int __v; \
    __asm__ ("addciu %0,%1,%2" : "=d" (__v) : "d" (__a), "JI" ((i))); \
    __v; \
})

#define cw401x_selsl(a,s) ({ \
    unsigned long long __a = (a); \
    unsigned int __s = (s); \
    unsigned int __v; \
    __asm__ ("mtc0 %2,$23; selsl %0,%M1,%L1" : "=d" (__v) \
	     : "d" (__a), "d" (__s)); \
    __v; \
})

#define cw401x_selsr(a,s) ({ \
    unsigned long long __a = (a); \
    unsigned int __s = (s); \
    unsigned int __v; \
    __asm__ ("mtc0 %2,$23; selsr %0,%M1,%L1" : "=d" (__v) \
	     : "d" (__a), "d" (__s)); \
    __v; \
})

#define cw401x_ffs(x) ({ \
    unsigned int __x = (x); \
    unsigned int __n; \
    __asm__ ("ffs %0,%1" : "=d" (__n) : "d" (__x)); \
    __n; \
})

#define cw401x_ffc(x) ({ \
    unsigned int __x = (x); \
    unsigned int __n; \
    __asm__ ("ffc %0,%1" : "=d" (__n) : "d" (__x)); \
    __n; \
})

/* generic count leading zeros */
#define mips_clz(x) (31 - cw401x_ffs (x))

/* generic count leading ones */
#define mips_clo(x) (31 - cw401x_ffc (x))


/* 
 * ATMIZER-2 instruction set extension intrinsics
 */

#if #cpu(atm2)

#define atm2_r2u(r) ({ \
    unsigned int __r = (r); \
    unsigned int __u; \
    __asm__ ("r2u %1" : "=l" (__u) : "d" (__r) : "hi"); \
    __u; \
})

#define atm2_u2r(u) ({ \
    unsigned int __u = (u); \
    unsigned int __r; \
    __asm__ ("u2r %1" : "=l" (__r) : "d" (__u) : "hi"); \
    __r; \
})

#define atm2_radd(a,b) ({ \
    unsigned int __a = (a); \
    unsigned int __b = (b); \
    unsigned int __v; \
    __asm__ ("radd %1,%2" : "=l" (__v) : "d" (__a), "d" (__b) : "hi"); \
    __v; \
})

#define atm2_rsub(a,b) ({ \
    unsigned int __a = (a); \
    unsigned int __b = (b); \
    unsigned int __v; \
    __asm__ ("rsub %1,%2" : "=l" (__v) : "d" (__a), "d" (__b) : "hi"); \
    __v; \
})

#define atm2_rmul(a,b) ({ \
    unsigned int __a = (a); \
    unsigned int __b = (b); \
    unsigned int __v; \
    __asm__ ("rmul %1,%2" : "=l" (__v) : "d" (__a), "d" (__b) : "hi"); \
    __v; \
})

#if 0
/* XXX no doc for this insn yet */
#define atm2_selrr(a,s) ({ \
    unsigned long long __a = (a); \
    unsigned int __s = (s); \
    unsigned int __v; \
    __asm__ ("mtc0 %2,$23; selrr %0,%M1,%L1" : "=d" (__v) \
	     : "d" (__a), "d" (__s)); \
    __v; \
})
#endif

#endif /* #cpu(atm2) */

#endif /* !ASSEMBLER */

#ifdef __cplusplus
}
#endif
#endif /* _CW401X_H_ */

