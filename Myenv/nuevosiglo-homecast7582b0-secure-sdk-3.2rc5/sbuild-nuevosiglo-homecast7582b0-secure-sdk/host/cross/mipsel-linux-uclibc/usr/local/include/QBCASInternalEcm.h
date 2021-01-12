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

#ifndef QB_CAS_INTERNAL_ECM_H
#define QB_CAS_INTERNAL_ECM_H

/**
 * @file QBCASInternalEcm.h
 * @brief Internal ECM object
 **/

#include <QBCASKeyManagerMeta.h>
#include <mpeg_psi_parser.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBCASInternalEcm  QBCASInternalEcm class
 * @{
 **/

/**
 * QBCASInternalEcm class
 *
 * In our PVR we are using internal ECM's objects, it keeps structure of DVB section
 * and could be easily serialized into TS packets. Internal ECM's are encrypted and
 * authenticated. Data included in ECM is usually: control words, fingerprints, maturity rating.
 *
 * more info: ETSI TS 100 289 V1.1.1
 *
 * @class QBCASInternalEcm
 */
typedef struct QBCASInternalEcm_s* QBCASInternalEcm;
/*
 * Value of TAG status in descriptor map, that means  this ECM contains valid descrambling data
 */
#define QBCAS_INTERNAL_ECM_STATUS_OK 0
/*
 * Value of TAG status in descriptor map, that means  this ECM contains not valid descrambling data
 */
#define QBCAS_INTERNAL_ECM_STATUS_FAILED 1
/*
 * descriptor map tag used to keep status code
 */
#define QBCAS_INTERNAL_ECM_DESC_TAG_STATUS 0

/**
 * Create new empty internal ecm object
 *
 * @return internal ecm handle or NULL on failure
 */
QBCASInternalEcm QBCASInternalEcmCreate(void);

/**
 * Get descriptor map handle which could be used to append data to ecm
 * @param[in] self internal ecm handle
 * @return mpeg descriptor map handle
 */
mpeg_descriptor_map* QBCASInternalEcmGetMap(QBCASInternalEcm self);
/**
 * Gets generic field in ecm: status
 *
 * @param[in] self internal ecm handle
 * @param[out] status: status code
 * @return 0 on success, otherwise error code
 */
int QBCASInternalEcmGetStatus(QBCASInternalEcm self, uint8_t *status);
/*
 * Sets generic field in ecm: status
 * @param[in] self internal ecm handle
 * @param[in] status: status code
 */
void QBCASInternalEcmSetStatus(QBCASInternalEcm self, uint8_t status);

/**
 * Get number of unused bytes in internal ecm which could be used to append more data
 *
 * @param[in] self internal ecm handle
 * @return number of unused bytes, could be negative when size of data is bigger that available space
 */
int QBCASInternalEcmGetFreeSpace(QBCASInternalEcm self);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_CAS_INTERNAL_ECM_H
