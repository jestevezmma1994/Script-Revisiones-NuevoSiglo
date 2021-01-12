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
 * impl.h: POSIX threads implementation-specific definitions
 */


#ifndef __PTHREAD_IMPL_H
#define __PTHREAD_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * FIRST SECTION DEFINES pthread_t and _pthread_waitq_t
 * which are opaque types required by <pthread.h>
 */

/* basic pthread_t type */
struct _rtpx_thread;
typedef struct _rtpx_thread * pthread_t;

/* waitq type for mutexes, cond variables, etc */
typedef struct _rtpx_waitq {
    struct _rtpx_thread * wq_first;
} _pthread_waitq_t;

/* 
 * REMAINDER ARE RTPX SPECIFIC EXTENSIONS, WHICH ARE NOT
 * REQUIRED BY <pthread.h>, BUT MAY BE CALLED BY 
 * KNOWLEDGABLE USERS.
 */

/* debug hooks */
extern unsigned int _rtpx_evlog_mask;
extern unsigned int _rtpx_evlog_size;
void _rtpx_elog_dump (void);

/* print thread table */
void _rtpx_threadstat (void);

/* print thread table via user-supplied printf function */
void _rtpx_threadstat_user (int (*)(const char *, ...));

/* print mutex state */
struct pthread_mutex;
void _rtpx_mutexstat (const struct pthread_mutex *);

struct _rtpx_eloginfo {
    const char	* 	fmt;
    int 		nargs;
    int 		intern;
};

void _rtpx_elog (struct _rtpx_eloginfo *, ...);
/*  __attribute__((format(printf,2,3)));*/

#if ! __STRICT_ANSI__
/* user-level logging */
#define _RTPX_ELOG(fmt, args...) \
   do { \
    static struct _rtpx_eloginfo _einfo = {fmt, -1, 0}; \
    _rtpx_elog (&_einfo , ## args); \
   } while (0)

/* kernel-level logging */
#define _RTPX_ELOG_INTERN(fmt, args...) \
   do { \
    static struct _rtpx_eloginfo _einfo = {fmt, -1, 1}; \
    _rtpx_elog (&_einfo , ## args); \
   } while (0)

#endif

#ifdef __cplusplus
}
#endif

#endif /* __PTHREAD_IMPL_H */
