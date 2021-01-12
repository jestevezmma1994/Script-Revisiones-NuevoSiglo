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
 * idt/idtsim.h: definitions for programs wanting to call IDT/sim PROM
 */


#define	PROM_LINK(name,entry)		\
LEAF(name);				\
	la	$v0,+entry;		\
	j	$v0;			\
END(name)

#define	PROM_ENTRY(x)	(0xbfc00000+((x)*8))

#define PROM_RESET	PROM_ENTRY(0)
#define PROM_NOT_IMP	PROM_ENTRY(1)
#define PROM_RESTART	PROM_ENTRY(2)
#define PROM_REINIT	PROM_ENTRY(3)
#define PROM_REENTER	PROM_ENTRY(4)
#define	PROM_OPEN	PROM_ENTRY(6)
#define	PROM_READ	PROM_ENTRY(7)
#define	PROM_WRITE	PROM_ENTRY(8)
#define	PROM_IOCTL	PROM_ENTRY(9)
#define	PROM_CLOSE	PROM_ENTRY(10)
#define	PROM_GETCHAR	PROM_ENTRY(11)
#define	PROM_PUTCHAR	PROM_ENTRY(12)
#define	PROM_SHOWCHAR	PROM_ENTRY(13)
#define	PROM_GETS	PROM_ENTRY(14)
#define	PROM_PUTS	PROM_ENTRY(15)
#define	PROM_PRINTF	PROM_ENTRY(16)
#define	PROM_RETURN	PROM_ENTRY(17)

#define	PROM_RFILEINIT	PROM_ENTRY(18)
#define	PROM_ROPEN	PROM_ENTRY(19)
#define	PROM_RCLOSE	PROM_ENTRY(20)
#define	PROM_RREAD	PROM_ENTRY(21)
#define	PROM_RWRITE	PROM_ENTRY(22)
#define	PROM_RLSEEK	PROM_ENTRY(23)
#define	PROM_RPRINTF	PROM_ENTRY(24)
#define	PROM_RGETS	PROM_ENTRY(25)

#define PROM_FLUSHCACHE	PROM_ENTRY(28)
#define PROM_CLEARCACHE	PROM_ENTRY(29)
#define PROM_SETJMP	PROM_ENTRY(30)
#define PROM_LONGJMP	PROM_ENTRY(31)

#if #cpu(r3000)
#define PROM_UTLBEXC	PROM_ENTRY(32)
#endif

#define PROM_SPRINTF	PROM_ENTRY(34)
#define PROM_ATOB	PROM_ENTRY(35)
#define PROM_STRCMP	PROM_ENTRY(36)
#define PROM_STRLEN	PROM_ENTRY(37)
#define PROM_STRCPY	PROM_ENTRY(38)
#define PROM_STRCAT	PROM_ENTRY(39)
#define PROM_CMDPARSE	PROM_ENTRY(40)
#define PROM_GETRANGE	PROM_ENTRY(41)
#define PROM_TOKEN	PROM_ENTRY(42)
#define PROM_HELP	PROM_ENTRY(43)

#define PROM_TIMERSTART	PROM_ENTRY(44)
#define PROM_TIMERSTOP	PROM_ENTRY(45)

#if #cpu(r3000)
#define PROM_NORMEXC	PROM_ENTRY(48)
#endif

#if #cpu(r4000) || #cpu(r5000)
#define PROM_PRINTF32	PROM_ENTRY(49)
#endif

#define PROM_GETMCONF	PROM_ENTRY(55)
#define PROM_SETMCONF	PROM_ENTRY(56)

#if #cpu(r4000) || #cpu(r5000)
#define PROM_TLBEXC	PROM_ENTRY(64)
#define PROM_UTLBEXC	PROM_ENTRY(64)	/* r3000 compat */
#endif

#define PROM_NEWDEV	PROM_ENTRY(65)
#define PROM_FASTINT	PROM_ENTRY(66)
#define PROM_NORMINT	PROM_ENTRY(67)
#define PROM_NEWCMD	PROM_ENTRY(68)

#define PROM_TFTPOPEN	PROM_ENTRY(69)
#define PROM_TFTPCLOSE	PROM_ENTRY(70)
#define PROM_TFTPREAD	PROM_ENTRY(71)
#define PROM_TFTPWRITE	PROM_ENTRY(72)
#define PROM_TFTPLSEEK	PROM_ENTRY(73)
#define PROM_SOCSYSCALL	PROM_ENTRY(74)

#if #cpu(r4000) || #cpu(r5000)
#define PROM_SETJMP64	PROM_ENTRY(75)
#define PROM_LONGJMP64	PROM_ENTRY(76)
#endif

#if #cpu(r4000) || #cpu(r5000) || #cpu(rc32364)
/* R4000 specific exception vectors */
#define PROM_XTLBEXC	PROM_ENTRY(80)
#define PROM_CACHEEXC	PROM_ENTRY(96)
#define PROM_NORMEXC	PROM_ENTRY(112)
#endif

