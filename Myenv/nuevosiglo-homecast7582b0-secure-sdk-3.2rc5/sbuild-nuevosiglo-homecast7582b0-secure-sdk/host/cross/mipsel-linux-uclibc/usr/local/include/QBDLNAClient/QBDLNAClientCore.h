/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef _QB_DLNA_CLIENT_H
#define _QB_DLNA_CLIENT_H

/**
 * @file QBDLNAClientCore.h
 * @brief DLNA client interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDLNAClientCore DLNA client interface
 * @ingroup QBDLNA
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvArray.h>
#include <QBDLNAClient/QBDLNARequest.h>

/**
 * max SOAP length
 */
#define QB_UPNP_MAX_SOAP_LENGTH           (1024*1024*4)

/**
 * DLNA device type for UPnP - media server
 */
#define QB_UPNP_DEVICE_TYPE               "urn:schemas-upnp-org:device:MediaServer:1"
/**
 * Service type CDS
 */
#define QB_UPNP_SERVICE_TYPE_CDS          "urn:schemas-upnp-org:service:ContentDirectory:1"
/**
 * Service type CMS
 */
#define QB_UPNP_SERVICE_TYPE_CMS          "urn:schemas-upnp-org:service:ConnectionManager:1"

/**
 * Get DLNA client instance. Returned object is retained.
 *
 * @param[in]           scheduler scheduler for internal fibers
 * @return              @a dlna client instance or @c NULL in case of error
 **/
QBDLNAClient QBDLNAClientGetInstance(SvScheduler scheduler);

/**
 * Perform UPnP discovery immediately and periodically check each device's subscription timeout.
 *
 * @param[in] self      DLNA client handle
 */
void QBDLNAClientPerformDiscovery(QBDLNAClient self);

/**
 * Unregister DLNA client and cancel periodic checking each device's subscription timeout.
 *
 * @param[in] self      DLNA client handle
 */
void QBDLNAClientPause(QBDLNAClient self);
/**
 * Send request to DLNA server
 *
 * @param[in] self      DLNA client handle
 * @param[in] request   structure containing what to sent to server
 **/
void QBDLNAClientSendRequest(QBDLNAClient self, QBDLNARequest request);

/**
 * Get current list of devices.
 *
 * @param[in] self      DLNA client handle
 * @return              array of devices
 **/
SvArray QBDLNAClientGetDeviceList(QBDLNAClient self);

/**
 * Set listener of DLNA client
 *
 * @param[in] self      DLNA client handle
 * @param[in] listener  object to be set as listener
 * @param[out] errorOut error info
 **/
void QBDLNAClientSetListener(QBDLNAClient self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Add request to ingored requests array to ignore it when action callback is called
 *
 * @param[in] self      DLNA client handle
 * @param[in] request   request to be ignored
 **/
void QBDLNAClientIgnoreRequest(QBDLNAClient self, QBDLNARequest request);
/**
 * Set request state as cancelled
 *
 * @param[in] self      DLNA client handle
 * @param[in] request   request to be cancelled
 **/
void QBDLNAClientCancelRequest(QBDLNAClient self, QBDLNARequest request);
/**
 * @}
 **/

#endif
