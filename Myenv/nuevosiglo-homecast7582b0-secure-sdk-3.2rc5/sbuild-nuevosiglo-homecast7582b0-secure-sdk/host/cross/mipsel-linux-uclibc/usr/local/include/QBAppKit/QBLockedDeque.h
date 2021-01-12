/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBLOCKEDDEQUE_H_
#define QBLOCKEDDEQUE_H_

/**
 * @file QBLockedDeque.h
 * @brief Thread safe wrapper over @ref SvDeque.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvDeque.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBLockedDeque Locked deque class
 * @ingroup QBAppKit
 * @{
 *
 * QBLockedDeque is a wrapper over @ref SvDeque.
 * All methods of QBLockedDeque API are thread safe.
 **/

/**
 * Universal callback. Used to perform some operations directly at SvDeque in thread safe manner.
 * @param[in] deque instance of @link SvDeque @endlink
 * @return user defined value
 */
typedef int (*QBLockedDequeCallback)(SvDeque deque);

/**
 * Thread safe wrapper over @link SvDeque @endlink
 * @class QBLockedDeque
 * @extends SvObject
 **/
typedef struct QBLockedDeque_ *QBLockedDeque;

/**
 * @brief Create instance
 * @param[out] errorOut error info
 * @return instance of @link QBLockedDeque @endlink
 */
QBLockedDeque QBLockedDequeCreate(SvErrorInfo *errorOut);

/**
 * Add object at the front of the queue.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 * @param[in] obj object to be added at the front of the queue
 * @param[out] errorOut error info
 */
void QBLockedDequePushFront(QBLockedDeque self, SvObject obj, SvErrorInfo *errorOut);

/**
 * Add object at the end of the queue.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 * @param[in] obj object to be added at the end of the queue
 * @param[out] errorOut
 */
void QBLockedDequePushBack(QBLockedDeque self, SvObject obj, SvErrorInfo *errorOut);

/**
 * Take object from the front of the queue. Remove it from the queue.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 * @return instance of object taken from the queue
 */
SvObject QBLockedDequeTakeFront(QBLockedDeque self);

/**
 * Take object from the end of the queue. Remove it from the queue.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 * @return instance of object taken from the queue
 */
SvObject QBLockedDequeTakeBack(QBLockedDeque self);

/**
 * Get the number of objects in the queue.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 * @return number of elements in queue
 */
size_t QBLockedDequeCount(QBLockedDeque self);

/**
 * Remove all objects from queue.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 */
void QBLockedDequeRemoveAllObjects(QBLockedDeque self);

/**
 * Invoke functor inside critical section.
 *
 * @memberof QBLockedDeque
 * @param[in] self instance of @link QBLockedDeque @endlink
 * @param[in] functor instance of the functor
 * @return value returned by functor
 */
int QBLockedDequeInvokeCallback(QBLockedDeque self, const QBLockedDequeCallback functor);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBLOCKEDDEQUE_H_ */
