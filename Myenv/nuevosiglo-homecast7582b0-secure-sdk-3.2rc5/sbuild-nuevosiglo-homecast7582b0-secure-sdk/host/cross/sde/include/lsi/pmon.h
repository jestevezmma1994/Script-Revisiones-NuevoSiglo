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
 * pmon.h: MIPS SDE / LSI Logic pmcc compatibility 
 *
 * Including this header file allows a program written for LSI Logic's 
 * pmcc compilation system and pmon libraries, to be compiled and
 * linked with MIPS SDE. 
 */


#define STDIN		STDIN_FILENO
#define STDOUT		STDOUT_FILENO

#define strequ(s0,s1)	(strcmp((s0),(s1)) == 0)
#define striequ(s0,s1)	(strcasecmp((s0),(s1)) == 0)

#define randn(n)	((rand()>>8)%((n)+1))

/*
 * Call the PMON ROM's i/o routines, rather then using MIPS SDE i/o, so
 * that we can keep the executable small.
 */
#define read		pmon_read
#define write		pmon_write
#define open		pmon_open
#define close		pmon_close
#define ioctl		pmon_ioctl
#define printf		pmon_printf
#define vsprintf	pmon_vsprintf
#define ttctl		pmon_ttctl
#define getenv		pmon_getenv
#define onintr		pmon_onintr
#define flush_cache	pmon_flush_cache
#define __exception	pmon__exception
#define adr2symoff	pmon_adr2symoff
#define sym2adr		pmon_sym2adr

/* ATMizer specific */
#define write_iram(dst,src,n) iram_write((dst),(src),(n))

#include <unistd.h>
#include <string.h>
