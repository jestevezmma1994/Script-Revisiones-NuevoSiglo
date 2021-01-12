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
 * conc.h: Transputer C interface
 */


#ifndef __CONC_H
#define __CONC_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199506L
#endif

#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
  pthread_attr_t	tha;
  pthread_t		thid;
  int			running;
  void			(*pfun)();
  int			nParam;
  int			*params;
} Process;

/* Note for Transputer 1 is low priority, 0 is high priority 
 */

Process *ProcAlloc (void (*)(), int, int, ...);
void ProcInit (Process *, void (*)(), void *, int, int, ...);
void ProcParam (Process *p, ...);
void ProcFree (Process *p);

int  ProcToHigh(void);		/* return 0,1 */
int  ProcToLow(void);		/* return 0,1 */
int  ProcGetPriority(void);	/* return 0,1 */

void ProcReschedule(void);	/* == yield */
void ProcStop(void);		/* == exit */

void ProcRun(Process *);
void ProcRunHigh(Process *);
void ProcRunLow(Process *);
void ProcPriPar(Process *, Process *);
void ProcPar(Process *, ...);
void ProcParList(Process *p[]);

void ProcWait(int);	/* NOTE 1us/64us ticks */
void ProcWaitNs(int);
void ProcWaitUs(int);
void ProcWaitMs(int);

#ifdef _TPUTER_PRIORITY_EXTENSIONS
/* extended priority stuff */
int  ProcSetPriority(int);
int  ProcGetPolicy(void);
int  ProcSetPolicy(int);
#endif

typedef sem_t	Semaphore;
#define SEMAPHOREINIT	SEM_INITIALIZER(0)

Semaphore * SemAlloc(void);
void SemFree (Semaphore *);

#define _HSemP(s)	sem_wait(s);
#define _HSemV(s)	sem_post(s);
#define _SemP(s)	_HSemP(s)
#define _SemV(s)	_HSemV(s)

#define SemP(s)		_SemP(&(s))
#define SemV(s)		_SemV(&(s))
#define HSemP(s)	_HSemP(&(s))
#define HSemV(s)	_HSemV(&(s))

#ifdef _POSIX_THREAD_DOORBELL_NP

/* Channel poll/wait 
*/
typedef struct {
  int			chan_state;
  int			chan_value;
#define CHAN_DEAD	0
#define CHAN_EMPTY	1
#define CHAN_FULL	2
  pthread_doorbell_t	chan_db;
} Channel;

int ProcAlt(Channel *cl,...);
int ProcSkipAlt(Channel *cl,...);
int ProcTimerAlt(int time,Channel *cl,...);

/* channel input
 */
int ChanInInt(Channel *cl);

/* 32 hardware channels
 */
extern Channel	_tpuHwChannelTable[32];

#define LINKIN(n)	(&_tpuHwChannelTable[n])
#define LINK0IN		LINKIN(0)
#define LINK1IN		LINKIN(1)
#define LINK2IN		LINKIN(2)
#define LINK3IN		LINKIN(3)
#define LINK4IN		LINKIN(4)
#define LINK5IN		LINKIN(5)
#define LINK6IN		LINKIN(6)
#define LINK7IN		LINKIN(7)

#endif

#endif /* __CONC_H */
