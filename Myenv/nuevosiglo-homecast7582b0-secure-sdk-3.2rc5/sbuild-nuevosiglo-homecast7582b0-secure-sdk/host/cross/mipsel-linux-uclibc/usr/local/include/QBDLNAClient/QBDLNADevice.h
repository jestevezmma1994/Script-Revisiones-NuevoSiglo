/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* SMP/QBDLNAClient/QBDLNADevice.h */

#ifndef QBDLNACLIENT_DEVICE_H_
#define QBDLNACLIENT_DEVICE_H_

/**
 * @file QBDLNADevice.h
 * @brief DLNA device (media server) interface and structures
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDLNADevice DLNA device (media server) interface and structures
 * @ingroup QBDLNAClient
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>

typedef struct QBDLNAService_t *QBDLNAService;
typedef struct QBDLNADevice_t *QBDLNADevice;

/**
 * Get runtime type identification object representing
 * type of DLNA service class.
 *
 * @return DLNA service class
 **/
SvType QBDLNAService_getType(void);
/**
 * Create DLNA service
 *
 * @param[in] serviceID     service identifier
 * @param[in] serviceType   type of service
 * @param[in] eventURL      URL of service's event
 * @param[in] controlURL    URL pointing to service
 * @param[in] SID           SID of service
 * @param[out] errorOut     error info
 * @return                  @a self or @c NULL in case of error
 **/
QBDLNAService QBDLNAServiceCreate(SvString serviceID, SvString serviceType, SvString eventURL, SvString controlURL, SvString SID, SvErrorInfo *errorOut);

/**
 * Get unique identifier of the device
 *
 * @param[in] self     device handle
 * @return             identifier
 **/
SvString QBDLNADeviceGetID(QBDLNADevice self);

/**
 * Get friendly name of the device
 *
 * @param[in] self     device handle
 * @return             friendly name
 **/
SvString QBDLNADeviceGetFriendlyName(QBDLNADevice self);

/**
 * Get device's CDS
 *
 * @param[in] self     device handle
 * @return             @a CDS
 **/
QBDLNAService QBDLNADeviceGetCDS(QBDLNADevice self);

/**
 * Get device's CMS
 *
 * @param[in] self     device handle
 * @return             @a CMS
 **/
QBDLNAService QBDLNADeviceGetCMS(QBDLNADevice self);

/**
 * Get device's advertisement timeout
 *
 * @param[in] self      device handle
 * @return              expiration time
 */
int QBDLNADeviceGetAdvertisementTimeout(QBDLNADevice self);

/**
 * Set device's advertisement timeout
 *
 * @param[in] self      device handle
 * @param[in] expires   expiration time
 */
void QBDLNADeviceSetAdvertisementTimeout(QBDLNADevice self, int expires);
/**
 * Get controlURL from device's CDS.
 *
 * @param[in] self     device handle
 * @return             @a controlURL
 **/
SvString QBDLNAServiceGetControlURL(QBDLNAService self);

/**
 * Get eventURL from device's CDS.
 *
 * @param[in] self     device handle
 * @return             @a eventURL
 **/
SvString QBDLNAServiceGetEventURL(QBDLNAService self);

/**
 * Get service ID.
 *
 * @param[in] self      device handle
 * @return device service ID
 **/
SvString QBDLNAServiceGetSID(QBDLNAService self);

/**
 * Sets service ID.
 * @param[in] self      device handle
 * @param[in] SID       new service ID
 **/
void QBDLNAServiceSetSID(QBDLNAService self, SvString SID);

/**
 * @}
 **/

#endif
