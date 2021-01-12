/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_MT_QUEUE_H_
#define QB_MT_QUEUE_H_

#include <pthread.h>
#include <stdint.h>

#include <sv-list.h>

struct QBMTQueue_t {
    list_t elements;
    unsigned int count;
    pthread_cond_t *cond;
    pthread_mutex_t *mutex;

    bool cancelled;

    uint64_t sync_point;
};

#define MTQ_ELEM_TYPE_ANY 0xFFFFFFFFU
#define MTQ_DEF_ELEM_ID -1

typedef void (* QBMTQueueElemClean)(void *prv);

typedef struct QBMTQueue_t *QBMTQueue;

typedef struct QBMTQueueElem_t *QBMTQueueElem;

QBMTQueueElem
QBMTQueueElemCreate(uint32_t type, void *privateData, QBMTQueueElemClean cleanup);

void
QBMTQueueElemSetID(QBMTQueueElem elem, int id);

int
QBMTQueueElemGetID(QBMTQueueElem elem);

void
QBMTQueueElemDestroy(QBMTQueueElem elem);

void *
QBMTQueueElemGetPrivateData(QBMTQueueElem elem);

int
QBMTQueueInit(QBMTQueue queue,
                pthread_mutex_t *mutex,
                pthread_cond_t *cond);

void
QBMTQueueDeinit(QBMTQueue queue);

void
QBMTQueueCancel(QBMTQueue queue);

bool
QBMTQueueEmpty(QBMTQueue queue);


/* return NULL if cancelled */
QBMTQueueElem
QBMTQueueGet(QBMTQueue queue, uint32_t typeMask, int id);

QBMTQueueElem
QBMTQueueGetNoWait(QBMTQueue queue, uint32_t typeMask, int id);

int
QBMTQueuePut(QBMTQueue queue, QBMTQueueElem elem);

void
QBMTQueueFlush(QBMTQueue queue, uint32_t typeMask);

void
QBMTQueueFlushEx(QBMTQueue queue, uint32_t typeMask, int id);

#endif

