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
#ifndef QBCALL2ACTION_STATE_LISTENER_H_
#define QBCALL2ACTION_STATE_LISTENER_H_

/**
 * @file QBCall2ActionStateListener.h Call2Action provider state and state listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

/**
 * @defgroup QBCall2ActionStateListener Call2ActionStateListener class.
 * @ingroup QBCall2ActionProvider
 **/

/**
 * Enum QBCall2ActionStateType represents current request state.
 **/
typedef enum {
    /*
     * Default QBCall2ActionState type.
     */
    QBCall2ActionStateType_executed, /**< QBCall2ActionStateType is created only if request had started successfully */
    QBCall2ActionStateType_finished, /**< Request finished with no errors */
    QBCall2ActionStateType_cancelled, /**< Request were cancelled */
    QBCall2ActionStateType_timeouted, /**< Request were cancelled because of time out expiration */
}QBCall2ActionStateType;

/**
 * @brief QBCall2ActionProvider notify state listeners
 **/
typedef struct QBCall2ActionState_ * QBCall2ActionState;

typedef const struct QBCall2ActionStateListener_ {
    /**
     * Notify listener that provider got request response.
     *
     * @param[in] self_  object implementing interface
     * @param[in] state  action new state
     */
    void (*stateChanged)(SvObject self_, QBCall2ActionState state);
} *QBCall2ActionStateListener;


/**
 * Get runtime type identification object representing
 * QBCall2ActionStateListener interface.
 *
 * @return QBCall2ActionStateListener interface object
 **/
SvInterface
QBCall2ActionStateListener_getInterface(void);

/**
 * Get refresh table. Hash table with ids to refresh.
 * Available keys: {actionID, productIDs, categoryIDs, service}
 *
 * @param[in] self          object that represents state
 * @param[out] errorOut     error info handler
 * @return                  @c refresh table if table is available, @c NULL otherwise
 **/
SvHashTable
QBCall2ActionStateGetRefreshTable(QBCall2ActionState self, SvErrorInfo * errorOut);

/**
 * Get failure cause. If request failed, we should get description here.
 *
 * @param[in] self          object that represent state
 * @param[out] errorOut     error info handler
 * @return                  @c failure description if request failed and we received
 *                          failure description, @c NULL otherwise
 **/
SvString
QBCall2ActionStateGetFailureCause(QBCall2ActionState self, SvErrorInfo * errorOut);

/**
 * Check if request ended up with success.
 *
 * @param[in] self          object that represent state
 * @param[out] errorOut     error handler
 * @return                  @c true if request ends up with success, @c false otherwise
 **/
bool
QBCall2ActionStateIsSuccess(QBCall2ActionState self, SvErrorInfo * errorOut);

/**
 * Method set listener to state.
 *
 * Listener should implement interface : QBCall2ActionStateListener.
 * When request finished, listener will be notified by method 'stateChanged'.
 *
 * @param[in] self          object that represent state
 * @param[in] listener      request listener
 * @param[out] errorOut     error handler
 * @return                  @c true if function ends up with success, @c false otherwise
 **/
bool
QBCall2ActionStateSetListener(QBCall2ActionState self, SvObject listener, SvErrorInfo * errorOut);

/**
 * @}
 **/

#endif
