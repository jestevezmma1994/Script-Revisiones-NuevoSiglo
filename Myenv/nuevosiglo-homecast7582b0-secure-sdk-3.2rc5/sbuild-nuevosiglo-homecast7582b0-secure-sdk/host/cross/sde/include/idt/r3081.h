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
 * r3081.h: IDT R3081 configuration register, and functions to access it.
 */


#ifndef _R3081_H_
#define _R3081_H_

/* generic R3000 definitions */
#include <mips/r3kc0.h>

#define CFG_LOCK	0x80000000	/* lock register */
#define CFG_SLOWBUS	0x40000000	/* slow external bus */
#define CFG_DBREFILL	0x20000000	/* data bus refill: 1/4 words */
#define CFG_FPINT_MASK	0x1c000000 	/* select fpa interrupt (0-5) */
#define CFG_FPINT_SHIFT	26
#define CFG_HALT	0x02000000 	/* halt until interrupt */
#define CFG_RF		0x01000000	/* reduce frequency */
#define CFG_AC		0x00800000	/* alt cache: 8kb+8kb */

/* extra CP0 registers */

#ifdef __ASSEMBLER__

#define C0_CONFIG	$3

$config		= 	$3

#else

#define C0_CONFIG	3

#endif /* __ASSEMBLER__ */

#endif /* _R3081_H_ */
