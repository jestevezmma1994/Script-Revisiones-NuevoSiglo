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

#ifndef QBNPVRPROVIDERREQUEST_H_
#define QBNPVRPROVIDERREQUEST_H_

/**
 * @file QBnPVRProviderRequest.h QBnPVRProviderRequest interface file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBnPVRProviderRequest The QBnPVRProviderRequest class.
 * @ingroup NPvr
 * @{
 **/

/**
 * QBnPVRProviderRequest type
 */
typedef struct QBnPVRProviderRequest_s *QBnPVRProviderRequest;

typedef const struct QBnPVRProviderRequestListener_ {
    void (*stateChanged)(SvObject self_, QBnPVRProviderRequest request);
} *QBnPVRProviderRequestListener;

/**
 * Get runtime type identification object representing
 * QBnPVRProviderRequestListener interface.
 **/
extern SvInterface
QBnPVRProviderRequestListener_getInterface(void);

/**
 * Get runtime type identification object representing QBnPVRProviderRequest class.
**/
extern SvType
QBnPVRProviderRequest_getType(void);

/**
 * Initialize QBnPVRProviderRequest
 * @param self  QBnPVRProviderRequest handle
 */
extern void
QBnPVRProviderRequestInit(QBnPVRProviderRequest self);

/**
 * Type of the NPvr request.
 * This enum should be consistent with the Provider API - see QBnPVRProvider.h
 */
typedef enum {
    QBnPVRProviderRequestType_unknown,
    QBnPVRProviderRequestType_scheduleRecording,
    QBnPVRProviderRequestType_updateRecording,
    QBnPVRProviderRequestType_deleteRecordings,
    QBnPVRProviderRequestType_refreshRecording,
    QBnPVRProviderRequestType_refreshAllRecordings,
    QBnPVRProviderRequestType_refreshRecordingsByEvent,
    QBnPVRProviderRequestType_lockRecording,

    QBnPVRProviderRequestType_createDirectory,
    QBnPVRProviderRequestType_deleteDirectory,
    QBnPVRProviderRequestType_refreshDirectory,
    QBnPVRProviderRequestType_updateDirectory,
    QBnPVRProviderRequestType_getDirectories,

    QBnPVRProviderRequestType_scheduleSeries,
    QBnPVRProviderRequestType_scheduleKeyword,
    QBnPVRProviderRequestType_deleteSchedule,
    QBnPVRProviderRequestType_updateSchedule,

    QBnPVRProviderRequestType_setQuota
} QBnPVRProviderRequestType;

/**
 * Type of the NPvr request error.
 */
typedef enum {
    QBnPVRProviderRequestError_none = 0,
    QBnPVRProviderRequestError_communication,
    QBnPVRProviderRequestError_quota,
    QBnPVRProviderRequestError_rejected,
    QBnPVRProviderRequestError_invalid_answer,
    QBnPVRProviderRequestError_other
} QBnPVRProviderRequestError;

/**
 * Type of the NPvr request exception.
 */
typedef enum {
    QBnPVRProviderRequestException_none = 0,
    QBnPVRProviderRequestException_recOnEventAlreadyScheduled,
    QBnPVRProviderRequestException_folderNotEmpty,
    QBnPVRProviderRequestException_invalidScheduleId,
    QBnPVRProviderRequestException_quotaExceeded,
    QBnPVRProviderRequestException_recNotAllowedOnThatEvent,
    QBnPVRProviderRequestException_invalidRecording,
    QBnPVRProviderRequestException_customerNotEntitledForSchedule,
    QBnPVRProviderRequestException_folderDoesNotBelongToCustomer,
    QBnPVRProviderRequestException_recordingNotFound,
    QBnPVRProviderRequestException_wrongRecordingStateForAction,
    QBnPVRProviderRequestException_otherRecordingOwner,
    QBnPVRProviderRequestException_eventNotFound,
    QBnPVRProviderRequestException_invaidNPvrState,
    QBnPVRProviderRequestException_cannotScheduleRecOnPastRevent,
    QBnPVRProviderRequestException_keywordLimitExceeded,
    QBnPVRProviderRequestException_schedulesLimitExceeded
} QBnPVRProviderRequestException;

/**
 * NPvr request state.
 */
typedef enum {
    QBnPVRProviderRequestState_unknown,
    QBnPVRProviderRequestState_inProgress,
    QBnPVRProviderRequestState_finished,
    QBnPVRProviderRequestState_canceled,
    QBnPVRProviderRequestState_failed
} QBnPVRProviderRequestState;

/**
 * Add QBnPVRProviderRequest listener.
 * @param[in] self          request handle
 * @param[in] listener      listener of the request
 */
extern void
QBnPVRProviderRequestAddListener(QBnPVRProviderRequest self, SvObject listener);

/**
 * Remove QBnPVRProviderRequest listener.
 * @param[in] self          request handle
 * @param[in] listener      listener of the request to be removed
 */
extern void
QBnPVRProviderRequestRemoveListener(QBnPVRProviderRequest self, SvObject listener);

/**
 * Return type of the given NPvr request.
 * @param self  request handle
 * @return      QBnPVRProviderRequestType
 */
extern QBnPVRProviderRequestType
QBnPVRProviderRequestGetType(QBnPVRProviderRequest self);

/**
 * Return state of the given NPvr request.
 * @param self  request handle
 * @return      QBnPVRProviderRequestState
 */
extern QBnPVRProviderRequestState
QBnPVRProviderRequestGetState(QBnPVRProviderRequest self);

/**
 * Return error of the given NPvr request.
 * @param self  request handle
 * @return      QBnPVRProviderRequestError
 */
extern QBnPVRProviderRequestError
QBnPVRProviderRequestGetError(QBnPVRProviderRequest self);

/**
 * Translate XMLRPC exception code to the QBnPVRProviderRequestException
 * @param xmlRPCExceptionCode   XMLRPC exception code
 * @return                      QBnPVRProviderRequestException
 */
extern QBnPVRProviderRequestException
QBnPVRProviderRequestTranslateXMLRPCExceptionCode(int xmlRPCExceptionCode);

/**
 * Return exception code for the NPvr request.
 * @param self  request handle
 * @return      QBnPVRProviderRequestException
 */
extern QBnPVRProviderRequestException
QBnPVRProviderRequestGetException(QBnPVRProviderRequest self);

/**
 * Return exception message for the given NPvr request.
 * @param self  request handle
 * @return      exception message string
 */
extern const char*
QBnPVRProviderRequestGetExceptionMessage(QBnPVRProviderRequest self);

/** @} */

#endif /* QBNPVRPROVIDERREQUEST_H_ */
