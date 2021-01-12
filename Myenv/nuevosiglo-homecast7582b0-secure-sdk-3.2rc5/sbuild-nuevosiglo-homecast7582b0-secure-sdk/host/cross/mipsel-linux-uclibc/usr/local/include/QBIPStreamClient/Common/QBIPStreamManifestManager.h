/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_IP_STREAM_MANIFEST_MANAGER_H
#define QB_IP_STREAM_MANIFEST_MANAGER_H

/**
 * @file  QBIPStreamManifestManager.h Streaming manifest manager.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/SvBufMan.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBIPStreamManifestManager Stream manifest manager
 * @{
 **/

/**
 * QBIPStreamManifestManager.
 * @class QBIPStreamManifestManager
 * @extends SvObject
 */
typedef struct QBIPStreamManifestManager_ *QBIPStreamManifestManager;

/**
 * Possible error types in QBIPStreamManifestManager
 */
typedef enum QBIPStreamManifestManagerError {
    QBIPStreamManifestManagerError_fatal,    //!< Fatal error
} QBIPStreamManifestManagerError;

/**
 * Create stream manifest manager.
 *
 * @param[in] bufMan            buffer manager
 * @param[in] scheduler         scheduler
 * @param[in] httpClientEngine  HTTP client engine
 * @param[in] url               URL for which manager is created
 * @param[in] logicIface        QBIPStreamManifestLogicIface object
 * @param[in] parserIface       parser interface object
 * @param[out] errorOut         error info
 * @return                      created manager, @c NULL in case of error
 */
QBIPStreamManifestManager
QBIPStreamManifestManagerCreate(SvBufMan bufMan,
                                SvScheduler scheduler,
                                SvHTTPClientEngine httpClientEngine,
                                SvURL url,
                                SvObject logicIface,
                                SvObject parserIface,
                                SvErrorInfo *errorOut);

/**
 * Add listener for manifest manager.
 *
 * @param[in] self          manifest manager
 * @param[in] listener      QBIPStreamManifestManagerListener object to add
 */
void
QBIPStreamManifestManagerAddListener(QBIPStreamManifestManager self,
                                     SvObject listener);

/**
 * Remove listener from manifest manager listeners list.
 *
 * @param[in] self          manifest manager
 * @param[in] listener      QBIPStreamManifestManagerListener object to remove
 */
void
QBIPStreamManifestManagerRemoveListener(QBIPStreamManifestManager self,
                                        SvObject listener);

/**
 * Start manifest downloading procedure.
 *
 * @param[in] self          manifest manager
 */
void
QBIPStreamManifestManagerStart(QBIPStreamManifestManager self);

/**
 * Stop manifest downloading procedure.
 *
 * @param[in] self          manifest manager
 */
void
QBIPStreamManifestManagerStop(QBIPStreamManifestManager self);

/**
 * @}
 **/

#endif /* QB_IP_STREAM_MANIFEST_MANAGER_H */
