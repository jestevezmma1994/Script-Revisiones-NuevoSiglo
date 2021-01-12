/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBCEC_H
#define QBCEC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBCEC.h QBCEC interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 * @defgroup QBCECCallingParameters CEC call parameters
 * @ingroup CubiTV_services
 * @{
 *
 * This service maintains functionality to encapsulate any supported CEC message.
 */

/** @see QBPlatformCECUICommand */
#define QBCECCallParametersKey_BUTTON            "button"
/** @see QBPlatformCECCECVersion */
#define QBCECCallParametersKey_CECVERSION        "cecversion"
/** @see QBPlatformCECDeviceAddress */
#define QBCECCallParametersKey_DESTINATION       "destination"
/** @see QBPlatformCECDeviceType */
#define QBCECCallParametersKey_DEVICETYPE        "devicetype"
/** @see QBPlatformCECOpcode (used to identify command) */
#define QBCECCallParametersKey_OPCODE            "opcode"
/** @see QBPlatformCECOpcode (used as command parameter) */
#define QBCECCallParametersKey_OPCODEARG         "opcodearg"
/** string */
#define QBCECCallParametersKey_OSDNAME           "osdname"
/** string */
#define QBCECCallParametersKey_OSDSTRING         "osdstring"
/** 16 bit value */
#define QBCECCallParametersKey_PHYSICALADDR      "physicaladdr"
/** @see QBPlatformCECPowerStatus */
#define QBCECCallParametersKey_POWERSTATUS       "powerstatus"
/** @see QBPlatformCECAbortReason */
#define QBCECCallParametersKey_ABORTREASON       "abortreason"

/**
 * Container of QBCEC Calling Parameters
 **/
typedef struct QBCECCallParameters_ *QBCECCallParameters;

/**
 * Create CallParameters to store arguments describing a message.
 *
 * @return newly created CallParameters collection
 */
QBCECCallParameters QBCECCallParametersCreate(void);

/**
 * Add an integer value to hash, in a format hash[key]=value.
 *
 * @param params an existing @c QBCECCallParameters, it shall be created with QBCECCallParametersCreate()
 * @param key an alphanumeric key
 * @param value an integer value
 */
void QBCECCallParametersAddInteger(QBCECCallParameters params, const char *key, const long long int value);

/**
 * Add an integer value to hash, in a format hash[key]=value.
 *
 * @param params an existing @c QBCECCallParameters, it shall be created with QBCECCallParametersCreate()
 * @param key an alphanumeric key
 * @param value a NULL-terminated ASCII string value
 */
void QBCECCallParametersAddString(QBCECCallParameters params, const char *key, const char *value);

/**
 * @}
 **/

/**
 * @defgroup QBCECService handler class
 * @ingroup CubiTV_services
 * @{
 *
 * This service maintains functionality to send any supported CEC Message through HAL.
 */

/**
 * QBCEC class.
 * @class QBCEC
 * @extends SvObject
 **/
typedef struct QBCEC_* QBCEC;

/**
 * Create QBCEC
 *
 * @memberof QBCEC
 *
 * @return created QBCEC, @c NULL in case of error
 **/
QBCEC QBCECCreate(void);

/**
 * Start service.
 *
 * @param self  QBCEC object handle
 */
void QBCECStart(QBCEC self);

/**
 * Stop service.
 *
 * @param self  QBCEC object handle
 */
void QBCECStop(QBCEC self);

/**
 * Send CEC Message.
 *
 * @param self QBCEC object handle
 * @param args arguments specifying message
 * @return @c 0 on success, negative value in case of error
 */
int QBCECSendMessage(QBCEC self, QBCECCallParameters args);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBCEC_H */
