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

#ifndef QB_PVR_PROVIDER_REQUEST_H_
#define QB_PVR_PROVIDER_REQUEST_H_

/**
 * @file QBPVRProviderRequest.h QBPVRProviderRequest interface file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRDirectory.h"
#include "QBPVRRecording.h"
#include "QBPVRTypes.h"

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBPVRProviderRequest QBPVRProviderRequest class.
 * @ingroup QBPVRProvider
 * @{
 **/

/**
 * QBPVRProvider request.
 * @class QBPVRProviderRequest
 * @extends SvObject
 **/
typedef struct QBPVRProviderRequest_ *QBPVRProviderRequest;

/**
 * Get runtime type identification object representing QBPVRProviderRequest class.
 *
 * @memberof QBPVRProviderRequest
 *
 * @return QBPVRProviderRequest runtime type identification object
 **/
extern SvType
QBPVRProviderRequest_getType(void);

/**
 * Initialize QBPVRProviderRequest
 *
 * @memberof QBPVRProviderRequest
 *
 * @param[in] self              provider request handle
 * @param[in] providerType      provider type
 */
extern void
QBPVRProviderRequestInit(QBPVRProviderRequest self, QBPVRProviderType providerType);

/**
 * Type of the PVR request.
 * This enum should match all possible PVR actions from the PVR provider API - see QBPVRProvider.h
 */
typedef enum {
    QBPVRProviderRequestType_unknown,                           ///< unknown
    QBPVRProviderRequestType_scheduleRecording,                 ///< schedule recording
    QBPVRProviderRequestType_updateRecording,                   ///< update recording
    QBPVRProviderRequestType_moveRecording,                     ///< move recording
    QBPVRProviderRequestType_stopRecording,                     ///< stop recording
    QBPVRProviderRequestType_deleteRecording,                   ///< delete recording
    QBPVRProviderRequestType_deleteAllRecordingsInState,        ///< delete recordings
    QBPVRProviderRequestType_deleteAllRecordingsFromCategory,   ///< delete recordings
    QBPVRProviderRequestType_lockRecording,                     ///< lock recording
    QBPVRProviderRequestType_markRecordingViewed,               ///< mark recording as already viewed

    QBPVRProviderRequestType_createDirectory,                   ///< create directory
    QBPVRProviderRequestType_deleteDirectory,                   ///< delete directory
    QBPVRProviderRequestType_updateDirectory,                   ///< update directory

    QBPVRProviderRequestType_stopSchedule,                      ///< stop schedule
} QBPVRProviderRequestType;

/**
 * Type of the PVR request error.
 */
typedef enum {
    QBPVRProviderNetworkRequestError_none = 0,          ///< none network request error
    QBPVRProviderNetworkRequestError_communication,     ///< communication network request error
    QBPVRProviderNetworkRequestError_quota,             ///< quota network request error
    QBPVRProviderNetworkRequestError_rejected,          ///< rejected network request error
    QBPVRProviderNetworkRequestError_invalidAnswer,     ///< invalid answer network request error
    QBPVRProviderNetworkRequestError_other,             ///< other network request error
} QBPVRProviderNetworkRequestError;

/**
 * Type of the PVR network request exception.
 */
typedef enum {
    QBPVRProviderNetworkRequestException_none = 0,                              ///< none network request exception
    QBPVRProviderNetworkRequestException_recOnEventAlreadyScheduled,            ///< event already scheduled network request exception
    QBPVRProviderNetworkRequestException_folderNotEmpty,                        ///< folder not empty network request exception
    QBPVRProviderNetworkRequestException_invalidScheduleId,                     ///< invalid schedule id network request exception
    QBPVRProviderNetworkRequestException_quotaExceeded,                         ///< quota exceeded network request exception
    QBPVRProviderNetworkRequestException_recNotAllowedOnThatEvent,              ///< recording not allowed on that event network request exception
    QBPVRProviderNetworkRequestException_invalidRecording,                      ///< invalid recording network request exception
    QBPVRProviderNetworkRequestException_customerNotEntitledForSchedule,        ///< customer not entitled network request exception
    QBPVRProviderNetworkRequestException_folderDoesNotBelongToCustomer,         ///< folder does not belong to customer network request exception
    QBPVRProviderNetworkRequestException_recordingNotFound,                     ///< recording not found network request exception
    QBPVRProviderNetworkRequestException_wrongRecordingStateForAction,          ///< wrong recording state for action network request exception
    QBPVRProviderNetworkRequestException_otherRecordingOwner,                   ///< other recording owner network request exception
    QBPVRProviderNetworkRequestException_eventNotFound,                         ///< event not found network request exception
    QBPVRProviderNetworkRequestException_invaidPVRState,                        ///< invalid pvr state network request exception
    QBPVRProviderNetworkRequestException_cannotScheduleRecOnPastEvent,          ///< cannot schedule recording on past event network request exception
    QBPVRProviderNetworkRequestException_keywordLimitExceeded,                  ///< keyword limit exceeded network request exception
    QBPVRProviderNetworkRequestException_schedulesLimitExceeded,                ///< schedules limit exceeded network request exception
} QBPVRProviderNetworkRequestException;

/**
 * PVR network request state.
 */
typedef enum {
    QBPVRProviderNetworkRequestState_unknown,                           ///< unknown network request state
    QBPVRProviderNetworkRequestState_inProgress,                        ///< in progress network request state
    QBPVRProviderNetworkRequestState_finished,                          ///< finished network request state
    QBPVRProviderNetworkRequestState_canceled,                          ///< canceled network request state
    QBPVRProviderNetworkRequestState_failed                             ///< failed network request state
} QBPVRProviderNetworkRequestState;

/**
 * PVR disk request state.
 */
typedef enum {
    QBPVRProviderDiskRequestStatus_unknown,                             ///< unknown disk request status
    QBPVRProviderDiskRequestStatus_failed,                              ///< failed disk request status
    QBPVRProviderDiskRequestStatus_success                              ///< success disk request status
} QBPVRProviderDiskRequestStatus;

/**
 * Add QBPVRProviderRequest listener.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param[in] self          request handle
 * @param[in] listener      listener of the request
 */
extern void
QBPVRProviderRequestAddListener(QBPVRProviderRequest self, SvObject listener);

/**
 * Remove QBPVRProviderRequest listener.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param[in] self          request handle
 * @param[in] listener      listener of the request to be removed
 */
extern void
QBPVRProviderRequestRemoveListener(QBPVRProviderRequest self, SvObject listener);

/**
 * Get provider type of the given PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      provider type
 */
extern QBPVRProviderType
QBPVRProviderRequestGetProviderType(QBPVRProviderRequest self);

/**
 * Get type of the given PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      QBPVRProviderRequestType
 */
extern QBPVRProviderRequestType
QBPVRProviderRequestGetType(QBPVRProviderRequest self);

/**
 * Get state of given network PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      network request state
 */
extern QBPVRProviderNetworkRequestState
QBPVRProviderNetworkRequestGetState(QBPVRProviderRequest self);

/**
 * Get status of given disk PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      disk request status
 */
extern QBPVRProviderDiskRequestStatus
QBPVRProviderDiskRequestGetStatus(QBPVRProviderRequest self);

/**
 * Get error of the given PVR network request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      network request error
 */
extern QBPVRProviderNetworkRequestError
QBPVRProviderNetworkRequestGetError(QBPVRProviderRequest self);

/**
 * Get exception code for PVR network request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      network request exception
 */
extern QBPVRProviderNetworkRequestException
QBPVRProviderNetworkRequestGetException(QBPVRProviderRequest self);

/**
 * Get exception message for given PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      exception message string
 */
extern const char*
QBPVRProviderNetworkRequestGetExceptionMessage(QBPVRProviderRequest self);

/**
 * Get recording for given PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      recording for which PVR request was created
 */
extern QBPVRRecording
QBPVRProviderRequestGetRecording(QBPVRProviderRequest self);

/**
 * Get directory for given PVR request.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      directory for which PVR request was created
 */
extern QBPVRDirectory
QBPVRProviderRequestGetDirectory(QBPVRProviderRequest self);

/**
 * Get params for  which given PVR request was created.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param self  request handle
 * @return      params for which this request was created
 */
extern SvObject
QBPVRProviderRequestGetParams(QBPVRProviderRequest self);

/**
 * Request listener interface.
 **/
typedef const struct QBPVRProviderRequestListener_ {
    /**
     * Request state changed.
     *
     * @param self  listener handle
     * @return      request which state changed
     */
    void (*stateChanged)(SvObject self_, QBPVRProviderRequest request);
} *QBPVRProviderRequestListener;

/**
 * Get runtime type identification object representing
 * QBPVRProviderRequestListener interface.
 * @return Provider request listener interface identification object
 **/
extern SvInterface
QBPVRProviderRequestListener_getInterface(void);

/**
 * @}
 **/

#endif /* QB_PVR_PROVIDER_REQUEST_H_ */
