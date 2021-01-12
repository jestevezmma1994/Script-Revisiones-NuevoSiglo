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

#ifndef QB_UPGRADE_VERIFIER_H_
#define QB_UPGRADE_VERIFIER_H_

/**
 * @file QBUpgradeVerifier.h
 * @brief Upgrade verifier class
 **/

/**
 * @defgroup QBUpgradeVerifier Upgrade verifier class
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBUpgrade/QBUpgradeObject.h>


/**
 * Upgrade service class.
 * @class QBUpgradeVerifier
 * @extends SvObject
 **/
typedef struct QBUpgradeVerifier_ *QBUpgradeVerifier;


/**
 * Get runtime type identification object
 * representing QBUpgradeVerifier class.
 *
 * @return QBUpgradeVerifier type identification object
 **/
extern SvType
QBUpgradeVerifier_getType(void);

/**
 * Create upgrade verifier.
 *
 * @memberof QBUpgradeVerifier
 *
 * @param[out] errorOut error info
 * @return              created upgrade verifier, @c NULL in case of error
 **/
extern QBUpgradeVerifier
QBUpgradeVerifierCreate(SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeVerifier
 *
 * @param[in] self      upgrade verifier handle
 * @param[in] scheduler FIXME
 * @param[in] listener  FIXME
 * @param[in] object    FIXME
 * @param[in] directoryPath FIXME
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeVerifierStart(QBUpgradeVerifier self,
                       SvScheduler scheduler,
                       SvObject listener,
                       QBUpgradeObject object,
                       SvString directoryPath,
                       SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeVerifier
 *
 * @param[in] self      upgrade verifier handle
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeVerifierStop(QBUpgradeVerifier self,
                      SvErrorInfo *errorOut);

/**
 * FIXME
 *
 * @memberof QBUpgradeVerifier
 *
 * @param[in] self      upgrade verifier handle
 * @param[out] totalFilesCount FIXME
 * @param[out] verifiedFilesCount FIXME
 * @param[out] invalidFilesCount FIXME
 * @param[out] errorOut error info
 **/
extern void
QBUpgradeVerifierGetResult(QBUpgradeVerifier self,
                           unsigned int *totalFilesCount,
                           unsigned int *verifiedFilesCount,
                           unsigned int *invalidFilesCount,
                           SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
