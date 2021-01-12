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

#ifndef QB_CONAX_PLAYREADY_H
#define QB_CONAX_PLAYREADY_H

/**
 * @file QBConaxPlayReady.h Conax implementation of QBPlayReadyInterface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBConaxPlayReady Conax PlayReady implementation
 * @ingroup DRM
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <QBAppKit/QBFuture.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief QBConaxPlayReady object handle
 */
typedef struct QBConaxPlayReady_* QBConaxPlayReady;

/**
 * @brief QBConaxPlayReadyGenerateConaxId generates unique ConaxId for the device.
 *
 * @note Function is non blocking so returned future may be completed with NULL in case of error.
 *
 * @param[out] errorOut error pointer to be filled
 * @return QBFuture with SvString ConaxId or NULL in case of error
 */
QBFuture QBConaxPlayReadyGenerateConaxId(SvErrorInfo* errorOut);

/**
 * @brief QBConaxPlayReadyCreate creates new QBConaxPlayReady object.
 *
 * @param[out] errorOut error pointer to be filled
 * @return created object or NULL in case of error
 */
SvObject QBConaxPlayReadyCreate(SvErrorInfo* errorOut);

/**
 * @brief QBConaxPlayReadySetHelperService set helper service object.
 *
 * @param[in] self self object
 * @param[in] helper object that implements QBConaxPlayReadyHelperServiceInterface
 * @param[out] errorOut error pointer to be filled
 */
void QBConaxPlayReadySetHelperService(SvObject self, SvObject helper, SvErrorInfo* errorOut);

/**
 * @brief QBCOnaxPlayReadyStart starts individualisation process if necessary.
 *
 * @note QBConaxPlayReadyHelperService object must be set to successfully perform
 * this operation.
 *
 * @param[in] self self object
 * @param[out] errorOut error pointer to be filled in case of error
 */
void QBConaxPlayReadyStart(SvObject self, SvErrorInfo* errorOut);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // QB_CONAX_PLAYREADY_H
