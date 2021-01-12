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
 * lsi/l64360.h: defines for LSI Logic L64360/ATMizer 
 */


#ifndef _LR64360_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _LR64360_H_

/* ATMizer based on LR333x0 */
#include <lsi/lr33300.h>

/* redefine the kseg 0/1 testing functions: everything is kseg1 (uncached) */
#undef	IS_KVA
#undef	IS_KVA0
#undef	IS_KVA1
#undef	IS_KVA01
#undef	IS_KVA2
#undef	IS_UVA

#define	IS_KVA(v)	1
#define	IS_KVA0(v)	0
#define	IS_KVA1(v)	1
#define	IS_KVA01(v)	1
#define	IS_KVA2(v)	0
#define	IS_UVA(v)	0

/* special IRAM region */
#define IRAM_BASE	0x00000000	/* IRAM */
#define IRAM_SIZE	4096	
#define IS_IRAM(v)	((unsigned) (v) < IRAM_SIZE)

/* registers */
#define VCR_BASE    0xfff00000	/* VCR ram 				*/
#define M_CREDIT    0xfff04000	/* channel group credit register 	*/
#define M_CONFIG    0xfff04100	/* configuration register 		*/
#define M_STALL	    0xfff04200	/* stall register 			*/
#define M_PRPC      0xfff04300	/* peak rate pacing counter 		*/
#define M_RXBUFPTR  0xfff04400	/* current rx cell address 		*/
#define M_TXFIFO    0xfff04500	/* transmit address fifo 		*/
#define M_RXACK	    0xfff0460c	/* receive cell indicator 		*/
#define M_GPRR      0xfff04700	/* global pacing rate register 		*/
#define M_SCR	    0xfff04a00	/* system control register 		*/
#define M_AHI	    0xfff04b00	/* assert host interrupt 		*/
#define M_CRC32	    0xfff04c00	/* CRC32 partial result 		*/
#define M_ASUBR     0xfff04d00	/* address substitution register	*/
#define M_DMACR	    0x00400000	/* DMA control register 		*/

#ifndef __ASSEMBLER__
#define CREDIT		(*((volatile unsigned long *)M_CREDIT))
#define CONFIG		(*((volatile unsigned long *)M_CONFIG))
#define STALL		(*((volatile unsigned long *)M_STALL))
#define PRPC		(*((volatile unsigned long *)M_PRPC))
#define RXBUFPTR	(*((volatile unsigned long *)M_RXBUFPTR))
#define TXFIFO		(*((volatile unsigned long *)M_TXFIFO))
#define RXACK		(*((volatile unsigned long *)M_RXACK))
#define GPRR		(*((volatile unsigned long *)M_GPRR))
#define SCR		(*((volatile unsigned long *)M_SCR))
#define AHI		(*((volatile unsigned long *)M_AHI))
#define CRC32		(*((volatile unsigned long *)M_CRC32))
#define ASUBR		(*((volatile unsigned long *)M_ASUBR))
#define DMACR		(*((volatile unsigned long *)M_DMACR))

/* 
 * IRAM access functions
 */

/* Block copy */
void		iram_write (vaddr_t, unsigned int *, size_t);
void		iram_read (unsigned int *, vaddr_t, size_t);

/*
 * Special versions of mips_get/put which handle the on-chip IRAM.
 */
unsigned int	_iram_get (void *, size_t, int *);
int		_iram_put (void *, size_t, unsigned int);

#define mips_get_byte(addr, errp)	_iram_get (addr, 1, errp)
#define mips_get_half(addr, errp)	_iram_get (addr, 2, errp)
#define mips_get_word(addr, errp)	_iram_get (addr, 4, errp)
#define mips_get_dword(addr, errp)	_iram_get (addr, 8, errp)

#define mips_put_byte(addr, v)		_iram_put (addr, 1, v)
#define mips_put_half(addr, v)		_iram_put (addr, 2, v)
#define mips_put_word(addr, v)		_iram_put (addr, 4, v)
#define mips_put_dword(addr, v)		_iram_put (addr, 8, v)
#endif

/* PRPC */
#define PRPC_RRSHFT 2
#define PRPC_RRMASK (7<<2)
#define PRPC_I  (1<<6)

/* CGCR */
#define CGCR_BBMASK (7<<2)
#define CGCR_BBSHFT 2

/* M_SCR system control register */
#define SCR_TAFMASK 7		/* tx addr fifo */
#define SCR_TAFSHFT 0
#define SCR_AWD	    (1<<4)	/* automatic watch-dog time-out */
#define SCR_RWD	    (1<<5)	/* regular watch-dog time-out */
#define SCR_BOCMASK (3<<6)
#define SCR_BOCSHFT 6
#define SCR_CHMASK  (0x3f<<8)
#define SCR_CHSHFT  8
#define SCR_SAFE    (1<<14)
#define SCR_CRCERR  (1<<15)
#define SCR_BSSHFT  16
#define SCR_BSMASK  (7<<16)	/* buffer size */
#define SCR_BS4     (0<<16)
#define SCR_BS8     (1<<16)
#define SCR_BS16    (2<<16)
#define SCR_BS32   (3<<16)	/* rx buffer size 32 cells */
#define SCR_CBSMASK (3<<19)	/* cache block size */
#define SCR_CBS2    (0<<19)
#define SCR_CBS4    (1<<19)
#define SCR_HHMASK  (7<<21)
#define SCR_HHSHFT  21
#define SCR_HH0	    (1<<21)
#define SCR_HH1	    (1<<22)
#define SCR_HH2	    (1<<23)
#define SCR_BM	    (1<<24)
#define SCR_TI	    (1<<26)	/* tx init */
#define SCR_RI      (1<<27)	/* receive init */
#define SCR_TOMASK  (3<<28)	/* tx offset */
#define SCR_TOSHFT  28
#define SCR_ROMASK  (3<<30)	/* rx offset */
#define SCR_ROSHFT  30
#define SCR_TO52   (3<<28)	/* tx cell size 52 bytes */
#define SCR_RO52   (3<<30)	/* rx cell size 52 bytes */
#define SCR_TXHEC  (1<<21)	/* generate HEC on tx */
#define SCR_RXHEC  (1<<22)	/* expect HEC on receive */
#define SCR_CHKHEC (1<<23)	/* check HEC on receive */
#define SCR_RXINIT (1<<27)	/* enable receiver */


/* address modifiers when writing to M_TAF */
#define A_CRC10	    (1<<2)	/* generate CRC10 value 		p39 */

/* address modifiers when writing to M_DMA 			  p28 & p52 */
#define DMA_LOMASK  (3<<30)	/* local offset register (VCR) */
#define DMA_LOSHFT  30	
#define DMA_BCMASK  (0x3f<<24)	/* transfer length counter */
#define DMA_BCSHFT  24
#define DMA_RD	    (1<<14)	/* read (versus write) */
#define DMA_WR	    0		/* rite (default) */
#define DMA_G	    (1<<13)	/* ghost write */
#define DMA_BCSHFT  24
#define DMA_LAMASK  (0x3ff<<2)	/* local address counter (VCR) */
#define DMA_LASHFT  2

/* RT stuff								p28 */
#define RT_MARMASK  (0xff<<24)	/* memory address register */
#define RT_MARSHFT  24
#define RT_MACMASK  (0x3fffff<<2) /* memory address counter */
#define RT_MACSHFT  2
#define RT_MORMASK  (3<<0)	/* memory (byte) offset register */
#define RT_MORSHFT  0

/* SP_AD 								p45 */
#define SPAD_BEMASK (0xf<<28)	/* byte enables */
#define SPAD_AT	    (1<<26)	/* access type */
#define SPAD_BF     (1<<25)	/* block fetch */
#define SPAD_ATOM   (1<<24)	/* atomic */


#ifdef __cplusplus
}
#endif
#endif /* _LR33300_H_ */
