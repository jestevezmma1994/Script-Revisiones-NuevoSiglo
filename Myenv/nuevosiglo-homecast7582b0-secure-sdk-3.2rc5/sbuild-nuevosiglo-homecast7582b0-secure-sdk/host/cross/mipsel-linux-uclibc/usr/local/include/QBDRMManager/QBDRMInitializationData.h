/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DRM_INITIALIZATION_DATA_H_
#define QB_DRM_INITIALIZATION_DATA_H_

/**
 * @file QBDRMSessionManager.h DRM Session Manager interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBAppKit/QBFuture.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * DRM system type
 **/
typedef enum {
    QBDRMSystemType_unknown,    /**< unknown DRM system type */
    QBDRMSystemType_latens,     /**< Latens DRM system type */
} QBDRMSystemType;

/**
 * DRM Initialization Data class.
 * @class QBDRMInitializationData
 * @extends SvObject
 **/
typedef struct QBDRMInitializationData_ *QBDRMInitializationData;

/**
 * Create an instance of QBDRMInitializationData class.
 *
 * At least one of uuid or systemType parameters should be passed. If systemType
 * will not be passed DRM Manager will detect type from UUID.
 *
 * @param[in] uuid          uuid string
 * @param[in] systemType    type of DRM system (e.g., Latens, PlayReady)
 * @param[in] customData    custom data required by a specific DRM system
 * @param[out] errorOut     error info
 **/
QBDRMInitializationData
QBDRMInitializationDataCreate(SvString uuid, QBDRMSystemType systemType, SvObject customData, SvErrorInfo *errorOut);

/**
 * Get UUID string from an instance of QBDRMInitializationData class.
 *
 * @param[in] self          QBDRMInitializationData instance handle
 * @return  UUID string
 **/
SvString
QBDRMInitializationDataGetUUID(QBDRMInitializationData self);

/**
 * Get DRM system type from an instance of QBDRMInitializationData class.
 *
 * @param[in] self          QBDRMInitializationData instance handle
 * @return  DRM system type
 **/
QBDRMSystemType
QBDRMInitializationDataGetDRMSystemType(QBDRMInitializationData self);

/**
 * Get DRM custom data from an instance of QBDRMInitializationData class.
 *
 * @param[in] self          QBDRMInitializationData instance handle
 * @return  DRM custom data
 **/
SvObject
QBDRMInitializationDataGetCustomData(QBDRMInitializationData self);


#endif /* QB_DRM_INITIALIZATION_DATA_H_ */
