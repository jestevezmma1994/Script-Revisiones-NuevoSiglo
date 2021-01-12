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
 * instr16def.h: instruction classifications
 */


#include <machine/endian.h>

/* MIPS instruction generators */
#define	ITYPE16_addiusp	0
#define	ITYPE16_addiupc	1
#define	ITYPE16_b	2
#define	ITYPE16_jal	3
#define	ITYPE16_beqz	4
#define	ITYPE16_bnez	5
#define	ITYPE16_shift	6
#define	ITYPE16_ld	7
#define	ITYPE16_rria	8
#define	ITYPE16_addiu8	9
#define	ITYPE16_slti	10
#define	ITYPE16_sltiu	11
#define	ITYPE16_i8	12
#define	ITYPE16_li	13
#define	ITYPE16_cmpi	14
#define	ITYPE16_sd	15
#define	ITYPE16_lb	16
#define	ITYPE16_lh	17
#define	ITYPE16_lwsp	18
#define	ITYPE16_lw	19
#define	ITYPE16_lbu	20
#define	ITYPE16_lhu	21
#define	ITYPE16_lwpc	22
#define	ITYPE16_lwu	23
#define	ITYPE16_sb	24
#define	ITYPE16_sh	25
#define	ITYPE16_swsp	26
#define	ITYPE16_sw	27
#define	ITYPE16_rrr	28
#define	ITYPE16_rr	29
#define	ITYPE16_extend	30
#define	ITYPE16_i64	31

#define RRTYPE16_jr	0
#define RRTYPE16_1	1
#define RRTYPE16_slt	2
#define RRTYPE16_sltu	3
#define RRTYPE16_sllv	4
#define RRTYPE16_break	5
#define RRTYPE16_srlv	6
#define RRTYPE16_srav	7
#define RRTYPE16_dsrl	8
#define RRTYPE16_entry	9	/* currently reserved */
#define RRTYPE16_cmp	10
#define RRTYPE16_neg	11
#define RRTYPE16_and	12
#define RRTYPE16_or	13
#define RRTYPE16_xor	14
#define RRTYPE16_not	15
#define RRTYPE16_mfhi	16
#define RRTYPE16_17	17
#define RRTYPE16_mflo	18
#define RRTYPE16_dsra	19
#define RRTYPE16_dsllv	20
#define RRTYPE16_21	21
#define RRTYPE16_dsrlv	22
#define RRTYPE16_dsrav	23
#define RRTYPE16_mult	24
#define RRTYPE16_multu	25
#define RRTYPE16_div	26
#define RRTYPE16_divu	27
#define RRTYPE16_dmult	28
#define RRTYPE16_dmultu	29
#define RRTYPE16_ddiv	30
#define RRTYPE16_ddivu	31

#define RRRTYPE16_daddu	0
#define RRRTYPE16_addu	1
#define RRRTYPE16_dsubu	2
#define RRRTYPE16_subu	3

#define RRIATYPE16_addiu  0
#define RRIATYPE16_daddiu 1

#define SHIFTTYPE16_sll  0
#define SHIFTTYPE16_dsll 1
#define SHIFTTYPE16_srl	 2
#define SHIFTTYPE16_sra	 3

#define I8TYPE16_bteqz	0
#define I8TYPE16_btnez	1
#define I8TYPE16_swrasp	2
#define I8TYPE16_adjsp	3
#define I8TYPE16_svrs	4
#define I8TYPE16_mov32r	5
#define I8TYPE16_6	6
#define I8TYPE16_movr32	7

#define I64TYPE16_ldsp	   0
#define I64TYPE16_sdsp	   1
#define I64TYPE16_sdrasp   2
#define I64TYPE16_dadjsp   3
#define I64TYPE16_ldpc	   4
#define I64TYPE16_daddiu   5
#define I64TYPE16_daddiupc 6
#define I64TYPE16_daddiusp 7

#if BYTE_ORDER == LITTLE_ENDIAN
union  mips16_instr {
    struct i16itype {
	int		imm:11;
	unsigned	op:5;
    } itype;
    struct i16ritype {
	int		imm:8;
	unsigned	rx:3;
	unsigned	op:5;
    } ritype;
    struct i16rrtype {
	unsigned	funct:5;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;	/* op == INST16_RR */
    } rrtype;
    struct i16rritype {
	int		imm:5;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;
    } rritype;
    struct i16rrrtype {
	unsigned	funct:2;
	unsigned	rz:3;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;	/* == INST16_RRR */
    } rrrtype;
    struct i16rriatype {
	int		imm:4;
	unsigned	funct:1;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;	/* == INST16_RRIA */
    } rriatype;
    struct i16stype {
	unsigned	funct:2;
	unsigned	sa:3;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;	/* == INST16_SHIFT */
    } stype;
    struct i16i8type {
	int		imm:8;
	unsigned	funct:3;
	unsigned	op:5;	/* == INST16_I8 */
    } i8type;
    struct i16i8r32type {
	unsigned	r32:5;
	unsigned	ry:3;
	unsigned	funct:3;
	unsigned	op:5;	/* == INST16_I8 */
    } i8r32type;
    struct i16i8r32rtype {
	unsigned	rz:3;
	unsigned	r32:5;
	unsigned	funct:3;
	unsigned	i8:5;	/* == INST16_I8 */
    } i8r32rtype;
    struct i16i64type {
	int		imm:8;
	unsigned	funct:3;
	unsigned	op:5;	/* == INST16_64 */
    } i64type;
    struct i16ri64type {
	int		imm:5;
	unsigned	ry:3;
	unsigned	funct:3;
	unsigned	op:5;	/* == INST16_64 */
    } ri64type;
    struct i16exttype {
	unsigned	imm:11;
	unsigned	op:5;	/* == INST16_EXTEND */
    } exttype;
    struct i16extitype {
	unsigned 	imm:5;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;
    } extitype;
    struct i16extrriatype {
	unsigned 	imm:4;
	unsigned 	funct:1;
	unsigned	ry:3;
	unsigned	rx:3;
	unsigned	op:5;
    } extrriatype;
    unsigned short	value;
};
#elif BYTE_ORDER == BIG_ENDIAN
union  mips16_instr {
    struct i16itype {
	unsigned	op:5;
	int		imm:11;
    } itype;
    struct i16ritype {
	unsigned	op:5;
	unsigned	rx:3;
	int		imm:8;
    } ritype;
    struct i16rrtype {
	unsigned	op:5;	/* op == INST16_RR */
	unsigned	rx:3;
	unsigned	ry:3;
	unsigned	funct:5;
    } rrtype;
    struct i16rritype {
	unsigned	op:5;
	unsigned	rx:3;
	unsigned	ry:3;
	int		imm:5;
    } rritype;
    struct i16rrrtype {
	unsigned	op:5;	/* == INST16_RRR */
	unsigned	rx:3;
	unsigned	ry:3;
	unsigned	rz:3;
	unsigned	funct:2;
    } rrrtype;
    struct i16rriatype {
	unsigned	op:5;	/* == INST16_RRIA */
	unsigned	rx:3;
	unsigned	ry:3;
	unsigned	funct:1;
	int		imm:4;
    } rriatype;
    struct i16stype {
	unsigned	op:5;	/* == INST16_SHIFT */
	unsigned	rx:3;
	unsigned	ry:3;
	unsigned	sa:3;
	unsigned	funct:2;
    } stype;
    struct i16i8type {
	unsigned	op:5;	/* == INST16_I8 */
	unsigned	funct:3;
	int		imm:8;
    } i8type;
    struct i16i8r32type {
	unsigned	op:5;	/* == INST16_I8 */
	unsigned	funct:3;
	unsigned	ry:3;
	unsigned	r32:5;
    } i8r32type;
    struct i16i832rtype {
	unsigned	i8:5;	/* == INST16_I8 */
	unsigned	funct:3;
	unsigned	r32:5;
	unsigned	rz:3;
    } i832rtype;
    struct i16i64type {
	unsigned	op:5;	/* == INST16_64 */
	unsigned	funct:3;
	int		imm:8;
    } i64type;
    struct i16ri64type {
	unsigned	op:5;	/* == INST16_64 */
	unsigned	funct:3;
	unsigned	ry:3;
	int		imm:5;
    } ri64type;
    struct i16jaltype {
	unsigned	op:5;	/* == INST16_JAL */
	unsigned	x:1;
	unsigned	imm20_16;		
	int		imm25_21;
    } jaltype;
    struct i16exttype {
	unsigned	op:5;	/* == INST16_EXTEND */
	unsigned	imm:11;		
    } exttype;
    struct i16extitype {
	unsigned	op:5;
	unsigned	rx:3;
	unsigned	ry:3;
	unsigned 	imm:5;
    } extitype;
    struct i16extrriatype {
	unsigned	op:5;
	unsigned	rx:3;
	unsigned	ry:3;
	unsigned 	funct:1;
	unsigned 	imm:4;
    } extrriatype;
    unsigned short	value;
};
#endif

