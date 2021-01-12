/*
 * Copyright (c) 2001-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * m64c0.h : MIPS64 coprocessor 0 definitions
 */


#ifndef _M64C0_H_
#define _M64C0_H_

/* superset of MIPS32 */
#include <mips/m32c0.h>

#ifndef __ASSEMBLER__

#if ! __mips16

/* C interface to dclz/dclo instructions */
#define mips_dclz(x) __extension__({ \
    unsigned long long __x = (x); \
    unsigned int __v; \
    __asm__ ("dclz %0,%1" : "=d" (__v) : "d" (__x)); \
    __v; \
})

#define mips_dclo(x) __extension__({ \
    unsigned long long __x = (x); \
    unsigned int __v; \
    __asm__ ("dclo %0,%1" : "=d" (__v) : "d" (__x)); \
    __v; \
})

#endif /* ! __mips16 */

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif

#endif /* _M64C0_H_ */
