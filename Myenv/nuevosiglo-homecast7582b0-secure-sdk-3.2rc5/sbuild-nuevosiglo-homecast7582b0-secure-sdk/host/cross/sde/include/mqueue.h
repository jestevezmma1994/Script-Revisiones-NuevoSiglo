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
 * mqueue.h: POSIX message queue definitions
 */


#ifndef __MQUEUE_H
#define __MQUEUE_H

#include <pthread.h>
#include <sys/signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _POSIX_MESSAGE_PASSING

struct mq_attr {
    long		mq_maxmsg;
    long		mq_msgsize;
    long		mq_flags;
    long		mq_curmsgs;
};


struct _mq_opaque;
typedef struct _mq_opaque *mqd_t;

#define MQ_NONBLOCK	0x1

mqd_t	mq_open (const char *, int, ...);
int	mq_close (mqd_t);
int	mq_unlink (const char *);
int	mq_send (mqd_t, const char *, size_t, unsigned int);
int	mq_receive (mqd_t, char *, size_t, unsigned int *);
int	mq_timedreceive (mqd_t, char *, size_t, unsigned int *, 
			 struct timespec *);
int	mq_notify (mqd_t, const struct sigevent *);
int	mq_setattr (mqd_t, const struct mq_attr *, struct mq_attr *);
int	mq_getattr (mqd_t, struct mq_attr *);
#endif /* _POSIX_MESSAGE_PASSING */

#ifdef __cplusplus
}
#endif

#endif /* __MQUEUE_H */
