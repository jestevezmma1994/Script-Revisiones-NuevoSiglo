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
 * semaphore.h: POSIX semaphore definitions
 */


#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _POSIX_SEMAPHORES
typedef struct {
  _pthread_waitq_t	sem_waitq;
  int 	 		sem_value;
#define SEM_VALUE_MAX	INT_MAX
  const char *		sem_name;
} sem_t;

#define SEM_FAILED	((sem_t *) 0)

/* unnamed semaphores */
int sem_init (sem_t *, int, unsigned int);
int sem_destroy (sem_t *);

/* named semaphores */
sem_t * sem_open (const char *, int, ...);
int sem_close (sem_t *);
int sem_unlink (const char *);

/* generic semaphore operations */
int sem_wait (sem_t *);
int sem_trywait (sem_t *);
int sem_post (sem_t *);
int sem_getvalue (sem_t *, int *);

int sem_setname_np (sem_t *, const char *);
int sem_getname_np (sem_t *, const char **);

#define SEM_INITIALIZER(n)	{ { NULL }, n }

#endif /* _POSIX_SEMAPHORES */

#ifdef __cplusplus
}
#endif

#endif /* __SEMAPHORE_H */
