/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_VIEWRIGHT_DVB_PVR_MANAGER_H_
#define QB_VIEWRIGHT_DVB_PVR_MANAGER_H_

#include <main_decl.h>
#include <QBViewRightOSM.h>

/**
 * @file QBViewRightDVBPVRManager.h
 * @brief ViewRight DVB PVR Manager service for DVR
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBViewRightDVBPVRManager ViewRight DVB PVR Manager service for DVR
 * @ingroup CubiTV_services
 * @{
 **/

typedef struct QBViewRightDVBPVRManager_ *QBViewRightDVBPVRManager;

/**
 * @brief QBViewRightDVBPVRManager constructor
 * @param appGlobals app globals
 * @return instance of QBViewRightDVBPVRManager
 */
QBViewRightDVBPVRManager QBViewRightDVBPVRManagerCreate(AppGlobals appGlobals);

/**
 * @brief Mark recording as parental controlled
 * @param self instance of QBViewRightDVBPVRManager
 * @param sessionId pvr recording session id
 */
void QBViewRightDVBPVRManagerMarkParentalControlledEvent(QBViewRightDVBPVRManager self, int sessionId);

/**
 * @brief Try to enter cached PIN
 * @param self instance of QBViewRightDVBPVRManager
 * @param pinType pin type
 * @param textSelector text select (pin reason)
 */
void QBViewRightDVBPVRManagerEnterCachedPin(QBViewRightDVBPVRManager self, QBViewRightPinType pinType, QBViewRightPinTextSelector textSelector);

/**
 * @}
 **/

#endif // QB_VIEWRIGHT_DVB_PVR_MANAGER_H_
