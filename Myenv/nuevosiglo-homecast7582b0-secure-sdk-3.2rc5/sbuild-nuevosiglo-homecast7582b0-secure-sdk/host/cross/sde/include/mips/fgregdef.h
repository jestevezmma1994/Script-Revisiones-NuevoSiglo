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
 * fgregdef.h : MIPS Rx000 symbolic floating-point register names
 */


/* result registers */
#define fv0	$f0
#define fv1	$f2

/* argument registers */
#define fa0	$f12
#define fa1	$f14

#if __mipsfp64

/* 64-bit f.p. registers (-mips3 and above) */

/* temporary registers */
#define ft0	$f4
#define ft1	$f5
#define ft2	$f6
#define ft3	$f7
#define ft4	$f8
#define ft5	$f9
#define ft6	$f10
#define ft7	$f11
#define ft8	$f16
#define ft9	$f17
#define ft10	$f18
#define ft11	$f19

/* saved registers */
#define	fs0	$f20
#define	fs1	$f21
#define	fs2	$f22
#define	fs3	$f23
#define	fs4	$f24
#define	fs5	$f35
#define	fs6	$f26
#define	fs7	$f27
#define	fs8	$f28
#define	fs9	$f29
#define	fs10	$f30
#define	fs11	$f31

#else

/* 32-bit f.p. registers */

/* temporary registers */
#define ft0	$f4
#define ft1	$f6
#define ft2	$f8
#define ft3	$f10
#define ft4	$f16
#define ft5	$f18

/* saved registers */
#define	fs0	$f20
#define	fs1	$f22
#define	fs2	$f24
#define	fs3	$f26
#define	fs4	$f28
#define	fs5	$f30

#endif
