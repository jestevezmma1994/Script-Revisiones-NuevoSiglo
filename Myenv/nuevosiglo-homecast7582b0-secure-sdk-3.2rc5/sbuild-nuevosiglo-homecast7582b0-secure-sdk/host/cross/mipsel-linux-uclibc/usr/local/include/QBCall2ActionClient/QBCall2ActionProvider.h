/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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
#ifndef QBCALL2ACTION_PROVIDER_H_
#define QBCALL2ACTION_PROVIDER_H_

/**
 * @file QBCall2ActionProvider.h Call2ActionProvider class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBCall2ActionClient/QBCall2ActionStateListener.h>
#include <QBCall2ActionClient/QBCall2ActionTree.h>
#include <Services/core/QBMiddlewareManager.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>
#include <stdbool.h>

/**
 * @defgroup QBCall2ActionProvider Call2ActionProvider class.
 * @{
 **/

/**
 * QBCall2ActionProviderParams - provider parameters.
 * Without these parameters provider can't be setup.
 **/
typedef struct QBCall2ActionProviderParams_ {
    /**
     * serviceId - identify service e.g. VOD, WEBONTV
     **/
    SvString serviceId;
    /**
     * productId - provider requests actions for product with that id
     **/
    SvString productId;
    /**
     * serverInfo - connection parameters
     **/
    SvObject serverInfo;
} * QBCall2ActionProviderParams;

/**
 * Call2ActionProvider class.
 * @class QBCall2ActionProvider
 * @implements QBTreeModel
 * @implements QBDataModel
 * @implements SvXMLRPCClientListener
 * @extends SvObject
 **/
typedef struct QBCall2ActionProvider_ * QBCall2ActionProvider;

/**
 * Create QBCall2ActionProvider
 *
 * @param[in] middlewareManager  middleware manager handle
 * @param[in] params            params needed to setup provider
 * @param[out] errorOut         error info handle
 * @return                      @c provider handle if creation ends with success, @c NULL otherwise
 **/
extern QBCall2ActionProvider
QBCall2ActionProviderCreate(QBMiddlewareManager middlewareManager, QBCall2ActionProviderParams params, SvErrorInfo * errorOut);

/**
 * Execute action
 *
 * @param[in] self      provider handle
 * @param[in] action    action handle
 * @param[in] metadata  hash table of extra data to send
 * @param[out] errorOut error info handle
 * @return              @c request state handle if function ends with success, @c NULL otherwise
 **/
extern QBCall2ActionState
QBCall2ActionProviderExecuteAction(QBCall2ActionProvider self, SvObject action, SvHashTable metadata, SvErrorInfo * errorOut);

/**
 * Refresh action.
 *
 * @note Remove subaction array and send request to get new one.
 *
 * @param[in] self      provider handle
 * @param[in] actionId  id action to refresh
 * @param[out] errorOut error info handle
 * @return              @c refresh state handle if function ends with success, @c NULL otherwise
 **/
extern QBCall2ActionState
QBCall2ActionProviderRefreshAction(QBCall2ActionProvider self, SvValue actionId, SvErrorInfo * errorOut);

/**
 * Get tree.
 *
 * @note QBCall2ActionProvider can't exist without tree.
 * @param[in] self  provider handle
 * @return          @c tree handle if self parameter isn't NULL, @c NULL otherwise
 **/
extern QBCall2ActionTree
QBCall2ActionProviderGetTree(QBCall2ActionProvider self);

/**
 * Provider send first request to middleware with productId set at creation.
 *
 * @param[in] self      provider handle
 * @param[out] errorOut error info handle
 * @return              @c true if function end with success, @c false otherwise
 **/
extern bool
QBCall2ActionProviderStart(QBCall2ActionProvider self, SvErrorInfo * errorOut);

/**
 * Stop provider. Provider won't send any new request.
 *
 * @param[in] self      provider handle
 * @param[out] errorOut error info handle
 * @return              @c true if function end with success, @c false otherwise
 **/
extern bool
QBCall2ActionProviderStop(QBCall2ActionProvider self, SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif
