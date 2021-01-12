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
 * sys/init.h: add an entry to the .sdeinit/.sdefini section.
 */


#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__

/*
 * A bit like constructors/destructors, but you have some control
 * over the order in which they are called.
 */

/* Each section contains a list of these data structures */
#ifndef __ASSEMBLER__
struct ifent {
    int		if_type;
    void	(*if_func)(void);
};

extern struct ifent __SDEINIT_LIST__[];
extern struct ifent __SDEFINI_LIST__[];
#endif

/* 
 * These type codes specify the order in which .init functions 
 * will be called.
 *
 * On termination the entries in the .fini table are called in 
 * the reverse order to this.
 */
#define IF_END		0	/* end of table */
#define IF_SYSCTRL	1	/* System controller initialisation */
#define IF_SYSCTRLDEP	2	/* System controller dependent stuff */
#define IF_DEVICE	3	/* low-level device initialisation */
#define IF_RTOS		4	/* RTOS initialisation */
#define IF_DRIVER	5	/* higher-level device driver initialisation */
#define IF_FILESYS	6	/* file system installation/initialisation */
#define IF_MOUNT	7	/* mountable file systems */
#define IF_PREMAIN	8	/* called just before main() */
#define IF_MAX		8


#ifndef __ASSEMBLER__
#define DECL_INIT_FUNC(type, func) \
  __attribute__ ((section (".sdeinit"))) \
  static const struct ifent __sdeinit__ ## func = {type, func};


#define DECL_FINI_FUNC(type, func) \
  __attribute__ ((section (".sdefini"))) \
  static const struct ifent __sdefini__ ## func = {type, func};
#else
#define DECL_INIT_FUNC(type, func) \
  .section	.sdeinit,"a",@progbits; \
__sdeinit___/**/func: ; \
  .size	__sdeinit___/**/func,8; \
  .word		type, func; \
  .previous

#define DECL_FINI_FUNC(type, func) \
  .section	.sdefini,"a",@progbits; \
__sdefini___/**/func: ; \
  .size	__sdefini___/**/func,8; \
  .previous
#endif

#endif /* __SYS_INIT_H__ */
