/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSATELLITESETUPLOGIC_H_
#define QBSATELLITESETUPLOGIC_H_

#include <QBInput/QBInputEvent.h>
#include <SvFoundation/SvCoreTypes.h>
#include <main_decl.h>

typedef struct QBSatelliteSetupLogic_t* QBSatelliteSetupLogic;


/**
 * Get runtime type identification object representing QBSatelliteSetupLogic type.
 *
 * @return QBSatelliteSetupLogic runtime type identification object
**/
SvType QBSatelliteSetupLogic_getType(void);

/**
  * Create QBSatelliteSetupLogic instance
  *
  * @param[in] appGlobals   appGlobals handle
  * @return                 satellite setup logic object
  **/
QBSatelliteSetupLogic QBSatelliteSetupLogicNew(AppGlobals appGlobals) __attribute__((weak));

/**
  * Handle input event
  *
  * @param[in] self     satellite setup logic handle
  * @param[in] e        inpute event handle
  * @return             0
  **/
int QBSatelliteSetupLogicInputEventHandler(QBSatelliteSetupLogic self, const QBInputEvent* e) __attribute__((weak));

/**
 * Check if all satellites have the same DiSEqC port
 *
 * @param self          satellite setup logic handle
 * @return              @c true if all satelites have the same DiSEqC port, @c false otherwise
 */
bool QBSatelliteSetupLogicIsSetDiSEqCPortForAllSatellites(QBSatelliteSetupLogic self) __attribute__((weak));

/**
 * Check if default satellites can be modified or removed
 *
 * @return              @c true if default satellites can be modified or removed, @c false otherwise
 */
bool QBSatelliteSetupLogicCanModifyDefaultSatellites(void) __attribute__((weak));

#endif
