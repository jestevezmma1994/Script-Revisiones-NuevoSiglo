/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBICSMARTCARDPVRMETADATA_H
#define QBICSMARTCARDPVRMETADATA_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>

#include <QBICSmartcardURIData.h>
#include <QBICSmartcardFingerprint.h>

#include <SvCore/SvLocalTime.h>

#define REC_META_KEY__CONAX_METADATA "rec:conax_metadata"

typedef struct QBICSmartcardPVRMetadata_ *QBICSmartcardPVRMetadata;

QBICSmartcardPVRMetadata QBICSmartcardPVRMetadataCreate(void);
void QBICSmartcardPVRMetadataSetTimeshiftDuration(QBICSmartcardPVRMetadata self, SvTime timeshiftDuration);
SvType QBICSmartcardPVRMetadata_getType(void);

SvString QBICSmartcardPVRMetadataSerialize(QBICSmartcardPVRMetadata self);
QBICSmartcardPVRMetadata QBICSmartcardPVRMetadataDeserialize(SvString self);

void QBICSmartcardPVRMetadataSetSignatureKey(QBICSmartcardPVRMetadata self, SvData signatureKey);
SvData QBICSmartcardPVRMetadataGetSignatureKey(QBICSmartcardPVRMetadata self);

int QBICSmartcardPVRMetadataAddURIData(QBICSmartcardPVRMetadata self, QBICSmartcardURIData uriData);
QBICSmartcardURIData QBICSmartcardPVRMetadataGetURIData(QBICSmartcardPVRMetadata self, unsigned int index);

int QBICSmartcardPVRMetadataAddFingerprint(QBICSmartcardPVRMetadata self, QBICSmartcardFingerprint *fingerprint);
QBICSmartcardFingerprint *QBICSmartcardPVRMetadataGetFingerprint(QBICSmartcardPVRMetadata self, unsigned int index);

int QBICSmartcardPVRMetadataPopNextIndex(QBICSmartcardPVRMetadata self);
void QBICSmartcardPVRMetadataIncrementIndex(QBICSmartcardPVRMetadata self);
bool QBICSmartcardPVRMetadataIsValidIndex(QBICSmartcardPVRMetadata self, unsigned int index);

SvLocalTime QBICSmartcardPVRMetadataGetExpirationTime(QBICSmartcardPVRMetadata self);

/**
 * Gets first valid URI position base on current position
 *
 * @param[in] self  QBICSmartcardPVRMetadata class instance
 * @param[in] currentPosition position in stream from which we will be looking for first valid URI
 * @param[in] retentionTime show how many time left from recording time
 * @param[in] isProperTime @c true if valid time was set
 * @return first valid content period position (could be lower that currentPosition if we are in valid content period)
 */
double QBICSmartcardPVRMetadataGetFirstValidContentPeriod(QBICSmartcardPVRMetadata self, double currentPosition, SvTime retentionTime, bool isProperTime);

#endif // QBICSMARTCARDPVRMETADATA_H
