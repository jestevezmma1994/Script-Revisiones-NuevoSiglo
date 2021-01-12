/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBIWAREMWNPVRREQUEST_H
#define CUBIWAREMWNPVRREQUEST_H

/**
 * @file CubiwareMWNPvrRequest.h
 * @brief Cubiware MW NPvr request class
 **/

#include <CubiwareMWClient/NPvr/CubiwareMWNPvrProvider.h>
#include <NPvr/QBnPVRProviderRequestImpl.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvFoundation/SvImmutableArray.h>

/**
 * @defgroup CubiwareMWNPvrRequest NPvr request handling functions
 * @ingroup CubiwareMWClient
 **/

/**
 * NPvr request types.
 * If you change it, look at CubiwareMWNPvrRequestCreateMethodName
 **/
typedef enum {
    CubiwareMWNPvrRequestType_scheduleRecording,
    CubiwareMWNPvrRequestType_updateRecording,
    CubiwareMWNPvrRequestType_deleteRecordings,
    CubiwareMWNPvrRequestType_deleteAllInStatesFromDirectory,
    CubiwareMWNPvrRequestType_refreshRecording,
    CubiwareMWNPvrRequestType_scheduleSeries,
    CubiwareMWNPvrRequestType_scheduleKeyword,
    CubiwareMWNPvrRequestType_deleteSchedule,
    CubiwareMWNPvrRequestType_updateSchedule,
    CubiwareMWNPvrRequestType_createDirectory,
    CubiwareMWNPvrRequestType_deleteDirectory,
    CubiwareMWNPvrRequestType_updateDirectory,
    CubiwareMWNPvrRequestType_getDirectories,
    CubiwareMWNPvrRequestType_getProducts
} CubiwareMWNPvrRequestType;

struct CubiwareMWNPvrRequest_s {
    struct QBnPVRProviderRequest_s  super_;

    CubiwareMWNPvrProvider provider;
    CubiwareMWNPvrRequestType type;

    SvXMLRPCRequest xmlRPCRequest;
};

/**
 * CubiwareMWNPvrRequest type.
 */
typedef struct CubiwareMWNPvrRequest_s* CubiwareMWNPvrRequest;

/**
 * Get runtime type identification object representing CubiwareMWNPvrRequest class.
 * @param[in] provider          CubiwareMWNPvrProvider handle
 * @param[in] type              type of the request - see @ref CubiwareMWNPvrRequestType
 * @param[in] arguments         request parameters
 * @return                      CubiwareMWNPvrRequest handle
 **/
CubiwareMWNPvrRequest
CubiwareMWNPvrRequestCreate(CubiwareMWNPvrProvider provider, CubiwareMWNPvrRequestType type, SvImmutableArray arguments);

/**
 * Start Cubiware MW NPvr request.
 * @param[in] self      CubiwareMWNPvrRequest handle
 */
void
CubiwareMWNPvrRequestStart(CubiwareMWNPvrRequest self);

/**
 * Stop Cubiware MW NPvr request.
 * @param[in] self      CubiwareMWNPvrRequest handle
 */
void
CubiwareMWNPvrRequestStop(CubiwareMWNPvrRequest self);

/** @} */

#endif // CUBIWAREMWNPVRREQUEST_H
