/*
 * Copyright (c) 2000-2003 MIPS Technologies, Inc.  All rights reserved.
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
 * watchpoint.h: MIPS SDE h/w watchpoint and debug support
 */



#define WATCHPOINT_MAX	8	/* max # of watchpoints for all cpus */

struct mips_watchpoint {
    int			num;
    int			capabilities;
    void		*private;
    unsigned short	type;
    short		asid;
    vaddr_t		va;
    paddr_t		pa;
    size_t		len;
    vaddr_t		mask;
    vaddr_t		maxmask;
};

/* Per-cpu watchpoint support functions */
struct mips_watchpoint_funcs {
    int		(*wp_init) (unsigned int);
    void	(*wp_setup) (struct mips_watchpoint *);
    int		(*wp_hit) (const struct xcptcontext *, vaddr_t *, size_t *);
    int		(*wp_set) (struct mips_watchpoint *);
    int		(*wp_clear) (struct mips_watchpoint *);
    void	(*wp_remove) (void);
    void	(*wp_insert) (void);
    void	(*wp_reset) (void);
};


/* watchpoint and other debug capabilities (returned) */
#define MIPS_WATCHPOINT_INEXACT	0x8000	/* inexact (unmatched) watchpoint */
#define MIPS_WATCHPOINT_SSTEP	0x1000	/* single-step supported */
#define MIPS_WATCHPOINT_VALUE	0x0400	/* data value match supported */
#define MIPS_WATCHPOINT_ASID	0x0200	/* ASID match supported */
#define MIPS_WATCHPOINT_VADDR	0x0100	/* virtual address (not physical) */
#define MIPS_WATCHPOINT_RANGE	0x0080	/* supports an address range */
#define MIPS_WATCHPOINT_MASK	0x0040	/* supports an address mask */
#define MIPS_WATCHPOINT_DWORD	0x0020	/* dword alignment (8 bytes) */
#define MIPS_WATCHPOINT_WORD	0x0010	/* word alignment (4 bytes) */

/* watchpoint capabilities and type (when setting) */
#define MIPS_WATCHPOINT_X	0x0004	/* instruction fetch wp */
#define MIPS_WATCHPOINT_R	0x0002	/* data read wp */
#define MIPS_WATCHPOINT_W	0x0001	/* data write wp */

/* watchpoint support functions */
struct xcptcontext;
int	_mips_watchpoint_init (void);
int	_mips_watchpoint_howmany (void);
int	_mips_watchpoint_capabilities (int);
vaddr_t	_mips_watchpoint_mask (int);
int	_mips_watchpoint_hit (const struct xcptcontext *, vaddr_t *, size_t *);
int	_mips_watchpoint_set (int, int, vaddr_t, paddr_t, size_t);
int	_mips_watchpoint_clear (int, int, vaddr_t, size_t);
void	_mips_watchpoint_remove (void);
void	_mips_watchpoint_insert (void);
void	_mips_watchpoint_reset (void);

/* internal utility functions for watchpoint code */
vaddr_t		_mips_watchpoint_calc_mask (vaddr_t, size_t);
int		_mips_watchpoint_address (const struct xcptcontext *, int,
					  vaddr_t *, size_t *);
int		_mips_watchpoint_set_callback (int, vaddr_t, size_t);

/* return codes from set/clear */
#define MIPS_WP_OK		0
#define MIPS_WP_NONE		1
#define MIPS_WP_NOTSUP		2
#define MIPS_WP_INUSE		3
#define MIPS_WP_NOMATCH		4
#define MIPS_WP_OVERLAP		5
#define MIPS_WP_BADADDR		6

/* in exception handler */
extern reg_t _mips_watchlo, _mips_watchhi;

