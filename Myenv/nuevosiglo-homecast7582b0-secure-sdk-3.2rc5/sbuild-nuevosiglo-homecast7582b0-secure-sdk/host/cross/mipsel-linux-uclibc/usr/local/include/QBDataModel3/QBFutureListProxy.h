/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FUTURE_LIST_PROXY_H_
#define QB_FUTURE_LIST_PROXY_H_

/**
 * @file QBFutureListProxy.h Future list proxy class.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBAppKit/QBFuture.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBFutureListProxy Future list proxy class
 * @ingroup QBDataModel3
 * @{
 *
 * A class providing @ref QBListModel, which serves
 * objects of QBFuture type. QBFuture object is observed by the proxy, and once
 * completed, is replaced by its value object.
 **/

/**
 * Future list proxy class.
 * @class QBFutureListProxy QBFutureListProxy.h <QBDataModel3/QBFutureListProxy.h>
 * @extends QBListDataSource
 * @implements QBListModelListener2
 *
 * This class is a wrapper for the @ref QBListProxy designed to work with @ref QBFuture objects.
 * Future list proxy observes instances of @ref QBFuture objects and when they are completed it notifies about the fact
 * and if the value returned by QBFuture is instance of @ref QBObservable, starts to observe it.
 *
 * Once the object from QBListProxy is requested, future list proxy checks if object is completed, and if so,
 * it returns the pair of objects as answer: first of them is object from which QBFuture originate, and second one
 * is value returned by QBFuture. In cases in which @ref QBFuture is not completed by the time its value is requested,
 * second returned object is empty.
 **/
typedef struct QBFutureListProxy_ *QBFutureListProxy;

/**
 * Callback called to obtain QBFuture object.
 *
 * @param[in]  target            callback target
 * @param[in]  object            data object for which QBFuture should be obtained
 * @param[out] errorOut          error out
 * @return                       object which is instance of @ref QBObservable
 **/
typedef QBFuture (*QBFutrureListProxyCreateFutureObjectCallback)(void *target, SvObject object, SvErrorInfo *errorOut);

/*
 * Number of items returned in answer
 */
#define QB_FUTURE_LIST_PROXY_ANSWER_CNT 2

/*
 * Original object index inside answer got from the proxy
 */
#define QB_FUTURE_LIST_PROXY_OBJ_IDX 0

/*
 * Future value index inside answer got from the proxy
 */
#define QB_FUTURE_LIST_PROXY_FUTURE_VALUE_IDX 1

/**
 * Create new future list proxy.
 *
 * @memberof QBFutureListProxy
 *
 * @param[in]  tree            handle to an object implementing @ref QBTreeModel
 * @param[in]  path            path to the parent node
 * @param[in]  callback        callback function returning instances of QBFuture object
 * @param[in]  callbackTarget  callback target
 * @param[out] errorOut        error info
 * @return                     created proxy, @c NULL in case of error
 **/
extern QBFutureListProxy
QBFutureListProxyCreate(SvObject tree,
                        SvObject path,
                        QBFutrureListProxyCreateFutureObjectCallback callback,
                        void *callbackTarget,
                        SvErrorInfo *errorOut);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_FUTURE_LIST_PROXY_H_ */
