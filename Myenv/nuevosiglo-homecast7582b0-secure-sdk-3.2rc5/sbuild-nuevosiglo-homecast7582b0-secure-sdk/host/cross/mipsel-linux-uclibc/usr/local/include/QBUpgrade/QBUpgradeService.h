/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_UPGRADE_SERVICE_H_
#define QB_UPGRADE_SERVICE_H_

/**
 * @file QBUpgradeService.h
 * @brief Upgrade service class
 **/

/**
 * @defgroup QBUpgradeService Upgrade service class
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBUpgrade/QBUpgradeObject.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>


/**
 * Upgrade service class.
 * @class QBUpgradeService
 * @extends SvObject
 **/
typedef struct QBUpgradeService_ *QBUpgradeService;


/**
 * Get runtime type identification object
 * representing QBUpgradeService class.
 *
 * @return QBUpgradeService type identification object
 **/
extern SvType
QBUpgradeService_getType(void);

/**
 * Create upgrade service
 *
 * @memberof QBUpgradeService
 *
 * @param[out] errorOut error info
 * @return              created upgrade service, @c NULL in case of error
 **/
extern QBUpgradeService
QBUpgradeServiceCreate(SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeServiceGetImageFileName(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeServiceGetBuildName(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeServiceGetBoardName(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeServiceGetLocalVersion(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeServiceGetLocalInformativeVersion(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern SvString
QBUpgradeServiceGetLocalHash(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @return              FIXME
 **/
extern const QBDVBUpgradeSelector *
QBUpgradeServiceGetSelector(QBUpgradeService self);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @param[in] scheduler FIXME
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeServiceStart(QBUpgradeService self,
                      SvScheduler scheduler,
                      SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeServiceStop(QBUpgradeService self,
                     SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @param[in] monitor   FIXME
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeServiceRegisterMonitor(QBUpgradeService self,
                                SvObject monitor,
                                SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeService
 *
 * @param[in] self      upgrade service handle
 * @param[in] downloader downloader
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeServiceRegisterDownloader(QBUpgradeService self,
                                   SvObject downloader,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
