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

#ifndef QB_PVR_PROVIDER_REQUEST_INTERNAL_H_
#define QB_PVR_PROVIDER_REQUEST_INTERNAL_H_

/**
 * @file QBPVRProviderRequestInternal.h QBPVRProviderRequest type details file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRProviderRequest.h"

#include <SvFoundation/SvWeakList.h>
#include <stdbool.h>

/**
 * @defgroup QBPVRProviderRequestInternal QBPVRProviderRequest class internals.
 * @ingroup QBPVRProviderRequest
 * @{
 **/

/**
 * QBPVRProvider request.
 * @class QBPVRProviderRequest
 * @extends SvObject
 **/
struct QBPVRProviderRequest_ {
    struct SvObject_ super_;            ///< super type

    bool initialized;                   ///< is request initialized

    QBPVRProviderRequestType type;      ///< request type

    QBPVRProviderType providerType;     ///< provider type for which request was created

    /// request type specific state union
    union {
        struct {
            QBPVRProviderNetworkRequestState state;             ///< network PVR request state
            QBPVRProviderNetworkRequestError error;             ///< network PVR request error
            QBPVRProviderNetworkRequestException exception;     ///< network PVR request exception

            SvWeakList listeners;                               ///< network request listeners
        } network;                                              ///< network PVR request connected variables
        struct {
            QBPVRProviderDiskRequestStatus status;              ///< disk request status
        } disk;                                                 ///< disk PVR request connected variables
    };

    QBPVRDirectory directory;           ///< directory which was created by this request
    QBPVRRecording recording;           ///< recording which was created by this request

    SvObject params;                    ///< params with which this request was created
};

/**
 * Notify listeners about request state change.
 * Notification method exported for the provider implementations.
 *
 * @memberof QBPVRProviderRequest
 *
 * @param[in] self   QBPVRProviderRequest handle
 */
extern void QBPVRProviderNetworkRequestNotifyListeners(QBPVRProviderRequest self);

/**
 * Translate XMLRPC exception code to the QBPVRProviderRequestException
 *
 * @memberof QBPVRProviderRequest
 *
 * @param xmlRPCExceptionCode   XMLRPC exception code
 * @return                      QBPVRProviderRequestException
 */
extern QBPVRProviderNetworkRequestException
QBPVRProviderNetworkRequestTranslateXMLRPCExceptionCode(int xmlRPCExceptionCode);

/**
 * @}
 **/

#endif /* QB_PVR_PROVIDER_REQUEST_INTERNAL_H_ */
