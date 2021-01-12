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
 * mips/m16c1.h: MIPS16 coprocessor 1 (fpu) support.
 *  
 * Define functions for accessing the coprocessor 1 control
 * registers * from mips16 mode.
 *
 * Most apart from "set" return the original register value.
 */


#ifndef _M16C1_H_
#define _M16C1_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASSEMBLER__

unsigned int	_mips16_frid (void);
unsigned int	_mips16_fsr (unsigned int clear, unsigned int set);
void		_mips16_lwc1 (const void *);
void		_mips16_ldc1 (const void *);

#undef fpa_getsr
#undef fpa_setsr
#undef fpa_xchsr
#undef fpa_bicsr
#undef fpa_bissr
#undef fpa_getrid

#define fpa_getsr()	_mips16_fsr (0, 0)
#define fpa_setsr(v)	_mips16_fsr (~0, v)
#define fpa_xchsr(v)	_mips16_fsr (~0, v)
#define fpa_bicsr(v)	_mips16_fsr (v, 0)
#define fpa_bissr(v)	_mips16_fsr (0, v)
#define fpa_getrid() 	_mips16_frid ();

#endif /* __ASSEMBLER */

#ifdef __cplusplus
}
#endif

#endif /* _M16C1_H_ */
