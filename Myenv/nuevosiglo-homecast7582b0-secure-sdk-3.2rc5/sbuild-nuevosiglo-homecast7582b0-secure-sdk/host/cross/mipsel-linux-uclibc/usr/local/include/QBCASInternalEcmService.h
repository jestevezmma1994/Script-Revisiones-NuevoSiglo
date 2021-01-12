/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CAS_INTERNAL_ECM_SERVICE_H
#define QB_CAS_INTERNAL_ECM_SERVICE_H

/**
 * @file QBCASInternalEcmService.h
 * @brief Internal ECM service object
 **/

#include <mpeg_psi_parser.h>
#include <QBCASInternalEcm.h>
#include <SvFoundation/SvData.h>
#include <QBCryptoTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBCASInternalEcmService  QBCASInternalEcmService class
 * @{
 **/

/**
 * QBCASInternalEcmService class
 *
 * This service is for serializing/deserializing ECMs from/to DVB sections
 *
 * @class QBCASInternalEcmService
 */
typedef struct QBCASInternalEcmService_s* QBCASInternalEcmService;
/*
 * Name of meta key used to authenticate ECM
 */
#define QBCAS_INTERNAL_ECM_META_KEY_NAME_HMAC "PVR_ECM_HMAC"

/**
 *  Callback fired when ECM is parsed from DVB section
 *
 *  @param[in] target callback self data
 *  @param[in] ecm parsed ECM or NULL on failure
 */
typedef void (*QBCASInternalEcmServiceSectionParsed)(void *target, QBCASInternalEcm ecm);

/**
 * Callback fired when DVB section of ECM is generated
 *
 * @param[in] target callback self data
 * @param[in] originalEcmPid original ecm pid
 * @param[in] section generated ca_message_section with ECM or NULL on failure
 */
typedef void (*QBCASInternalEcmServiceSectionGenerated)(void *target, int16_t originalEcmPid, SvData section);

/**
 * Create new empty internal ecm service instance
 *
 * @param[in] scheduler scheduler used to receive callbacks
 * @param[in] keyType key type
 * @return internal ecm handle or NULL on failure
 */
QBCASInternalEcmService QBCASInternalEcmServiceCreate(SvScheduler scheduler, const QBCryptoKeyType keyType);

/**
 * Create internal ecm from dvb section
 *
 * @param[in] self internal ecm service instance handle
 * @param[in] section input buffer with dvb section of ecm to create, if null then empty ecm is created
 * @param[in] metaSignatureKey meta data signature key
 * @param[in] callback callback called when section is parsed
 * @param[in] target first parameter passed to callback
 * @return @c 0 on success @c -1 otherwise
 */
int QBCASInternalEcmServiceParseSection(QBCASInternalEcmService self, SvData section, SvData ecmSignatureKey, QBCASInternalEcmServiceSectionParsed callback, void *target);

/**
 * Generates DVB section with ECM (more info ETSI ETSI TS 100 289 V1.1.1)
 *
 * @param[in] self internal ecm service instance handle
 * @param[in] ecm internal ecm handle
 * @param[in] originalEcmPid original ecm pid
 * @param[in] metaSignatureKey meta data signature key
 * @param[in] tableId it should be 0x80 or 0x81 (MPEG TS param)
 * @param[in] version: version of internal ecm
 * @param[in] callback callback called when section is generated
 * @param[in] target first parameter passed to callback
 * @return @c 0 on success @c -1 otherwise
 */
int QBCASInternalEcmServiceGenerateSection(QBCASInternalEcmService self, QBCASInternalEcm ecm, int16_t originalEcmPid, SvData ecmSignatureKey, uint8_t tableId, uint8_t version, QBCASInternalEcmServiceSectionGenerated callback, void* target);

/**
 * Start service
 *
 * @param[in] self internal ecm service instance handle
 * @retrun @c 0 on success @c -1 otherwise
 */
int QBCASInternalEcmServiceStart(QBCASInternalEcmService self);

/**
 * Stop service
 *
 * @param[in] self internal ecm service instance handle
 */
void QBCASInternalEcmServiceStop(QBCASInternalEcmService self);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_CAS_INTERNAL_ECM_SERVICE_H
