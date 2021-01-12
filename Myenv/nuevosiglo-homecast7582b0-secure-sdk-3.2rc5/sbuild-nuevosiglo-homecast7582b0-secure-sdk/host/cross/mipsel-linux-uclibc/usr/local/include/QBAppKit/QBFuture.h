/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FUTURE_H_
#define QB_FUTURE_H_

/**
 * @file QBFuture.h Future class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBFuture Future class
 * @ingroup QBAppKit
 * @{
 **/


/**
 * Future class.
 * @class QBFuture
 * @extends QBObservable
 **/
typedef struct QBFuture_ *QBFuture;

/**
 * Type of callback function notifying about change in future state.
 *
 * @param[in] owner     opaque pointer to callback's owner
 * @param[in] future    handle to a future whose state has just changed
 **/
typedef void (*QBFutureCallback)(void *owner,
                                 QBFuture future);


/**
 * Get runtime type identification object representing QBFuture class.
 *
 * @return future class
 **/
extern SvType
QBFuture_getType(void);

/**
 * Create new future instance.
 *
 * @memberof QBFuture
 *
 * @param[out] errorOut error info
 * @return              created future, @c NULL in case of error
 **/
extern QBFuture
QBFutureCreate(SvErrorInfo *errorOut);

/**
 * Check if future is completed.
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @return              @c true if future is completed,
 *                      @c false if it pending or cancelled
 **/
extern bool
QBFutureIsCompleted(QBFuture self);

/**
 * Check if future has been cancelled.
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @return              @c true if future has been cancelled,
 *                      @c false if it pending or completed
 **/
extern bool
QBFutureIsCancelled(QBFuture self);

/**
 * Get value of a completed future.
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @param[out] errorOut error info
 * @return              value of the future, @c NULL if future is not complete
 **/
extern SvObject
QBFutureGetValue(QBFuture self,
                 SvErrorInfo *errorOut);

/**
 * Register callback for notifications about changes in future state.
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @param[in] callback  callback function
 * @param[in] owner     opaque pointer to callback's owner
 **/
extern void
QBFutureAddCallback(QBFuture self,
                    QBFutureCallback callback,
                    void *owner);

/**
 * Unregister callback previously registered using QBFutureAddCallback().
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @param[in] callback  callback function
 * @param[in] owner     opaque pointer to callback's owner
 **/
extern void
QBFutureRemoveCallback(QBFuture self,
                       QBFutureCallback callback,
                       void *owner);

/**
 * Complete the future assigning given value.
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @param[in] value     value to be assigned to the future
 * @param[out] errorOut error info
 **/
extern void
QBFutureComplete(QBFuture self,
                 SvObject value,
                 SvErrorInfo *errorOut);
/**
 * Cancel the future.
 *
 * @memberof QBFuture
 *
 * @param[in] self      future handle
 * @param[out] errorOut error info
 **/
extern void
QBFutureCancel(QBFuture self,
               SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
