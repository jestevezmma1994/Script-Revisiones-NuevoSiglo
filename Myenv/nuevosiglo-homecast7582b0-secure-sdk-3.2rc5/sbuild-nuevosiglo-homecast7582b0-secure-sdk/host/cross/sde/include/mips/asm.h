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
 * asm.h: various macros to help assembly languages writers
 */


#ifndef _TEXT_SECTION
#define _TEXT_SECTION .text
#endif

#define LEAF(name) \
  	_TEXT_SECTION; \
  	.globl	name; \
  	.ent	name; \
name:

#define XLEAF(name) \
  	_TEXT_SECTION; \
  	.globl	name; \
  	.aent	name; \
name:

#define WLEAF(name) \
  	_TEXT_SECTION; \
  	.weakext name; \
  	.ent	name; \
name:

#define ALEAF(name,alias) \
  	_TEXT_SECTION; \
  	.weakext alias,name; \
  	.ent	name; \
name:

#define SLEAF(name) \
  	_TEXT_SECTION; \
  	.ent	name; \
name:

#define SXLEAF(name) \
  	_TEXT_SECTION; \
  	.aent	name; \
name:

#define END(name) \
  	.size name,.-name; \
  	.end	name

#define SEND(name)	END(name)
#define WEND(name)	END(name)
#define AEND(name,alias) END(name)

#define EXPORT(name) \
  	.globl name; \
  	.type name,@object; \
name:

#define EXPORTS(name,sz) \
  	.globl name; \
  	.type name,@object; \
  	.size name,sz; \
name:

#define WEXPORT(name,sz) \
  	.weakext name; \
  	.type name,@object; \
  	.size name,sz; \
name:

#define	IMPORT(name, size) \
	.extern	name,size

#define BSS(name,size) \
  	.type name,@object; \
	.comm	name,size

#define LBSS(name,size) \
  	.lcomm	name,size

#ifdef __PROFILING__
#define _MCOUNT \
	.set noat; \
	move	$1,$31; \
	jal	_mcount; \
  	.set at
#else
#define _MCOUNT
#endif
