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

#ifndef QB_DLNA_REQUEST_H_
#define QB_DLNA_REQUEST_H_

/**
 * @file QBDLNARequest.h
 * @brief DLNA request class
 **/

/**
 * @defgroup QBDLNARequest DLNA request structures and interface
 * @ingroup QBDLNAClient
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvArray.h>


/**
 * DLNA request class.
 **/
typedef struct QBDLNARequest_ *QBDLNARequest;
typedef struct QBDLNAClient_t *QBDLNAClient;

/**
 * DLNA request ID.
 **/
typedef unsigned int QBDLNARequestID;

/**
 * DLNA request state.
 **/
typedef enum {
   QBDLNARequestState_uninitialized = 0,
   QBDLNARequestState_ready,
   QBDLNARequestState_sent,
   QBDLNARequestState_gotItemList,
   QBDLNARequestState_gotException,
   QBDLNARequestState_accessDenied,
   QBDLNARequestState_cancelled,
   QBDLNARequestState_finished,
   QBDLNARequestState_timeout,
   QBDLNARequestState_brokenConnection,
   QBDLNARequestState_subscriptionExpired,
} QBDLNARequestState;

typedef enum {
    QBDLNARequestActionType_browseDevice ,
    QBDLNARequestActionType_browseItem,
    QBDLNARequestActionType_searchDevice,
    QBDLNARequestActionType_searchItem,
} QBDLNARequestActionType;

/**
 * Get runtime type identification object representing
 * type of DLNA request class.
 *
 * @return DLNA request class
 **/
extern SvType
QBDLNARequest_getType(void);

/**
 * Initialize DLNA request object.
 *
 * @param[in] self          DLNA request handle
 * @param[in] dlnaClient    DLNA client handle
 * @param[in] serverID      DLNA server identifier
 * @param[in] itemID        DLNA item identifier in server
 * @param[in] actionType    action type
 * @param[in] rangeStart    start of request range
 * @param[in] rangeEnd      end of request range
 * @param[in] timeout       response timeout
 * @param[in] scheduler     scheduler
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
QBDLNARequest
QBDLNARequestInit(QBDLNARequest self,
                    QBDLNAClient dlnaClient,
                    SvValue serverID,
                    SvValue itemID,
                    QBDLNARequestActionType actionType,
                    int rangeStart,
                    int rangeEnd,
                    unsigned int timeout,
                    SvScheduler scheduler,
                    SvErrorInfo *errorOut);


/**
 * Set DLNA client listener.
 *
 * This method installs a listener object for the DLNA response.
 * Listener must implement QBDLNAClientListener interface.
 *
 * @param[in] self         DLNA request handle
 * @param[in] listener     DLNA listener handle, pass @c NULL
 *                         to remove existing listener
 * @param[out] errorOut    error info
 **/
extern void
QBDLNARequestSetListener(QBDLNARequest self,
                         SvObject listener,
                         SvErrorInfo *errorOut);

/**
 * Start asynchronous DLNA call.
 *
 * This method starts the state machine of the DLNA call.
 *
 * @param[in] self         DLNA request handle
 * @param[in] dlnaClient   DLNA client handle
 * @param[out] errorOut    error info
 **/
extern void
QBDLNARequestStart(QBDLNARequest self,
                     QBDLNAClient dlnaClient,
                     SvErrorInfo *errorOut);

/**
 * Cancel previously started DLNA call.
 *
 * @param[in] self         DLNA request handle
 **/
extern void
QBDLNARequestCancel(QBDLNARequest self);

/**
 * Confirm that request was handled
 *
 * @param[in] self         DLNA request handle
 **/
extern void
QBDLNARequestFinish(QBDLNARequest self);

/**
 * Get current request's state.
 *
 * @param[in] self         DLNA request handle
 * @return                 @a request's state
 **/
extern QBDLNARequestState
QBDLNARequestGetState(QBDLNARequest self);

/**
 * Get request's range start.
 *
 * @param[in] self         DLNA request handle
 * @return                 @a request's range start
 **/
extern int
QBDLNARequestGetRangeStart(QBDLNARequest self);

/**
 * Get request's range end.
 *
 * @param[in] self         DLNA request handle
 * @return                 @a request's range end
 **/
extern int
QBDLNARequestGetRangeEnd(QBDLNARequest self);

/**
 * Get request's result data.
 *
 * @param[in] self         DLNA request handle
 * @return                 @a request's result data
 **/
extern SvArray
QBDLNARequestGetBrowseResult(QBDLNARequest self);

/**
 * @}
 **/

#endif
