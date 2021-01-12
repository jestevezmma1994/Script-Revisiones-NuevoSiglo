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

#ifndef CONAXPVRMANAGER_H
#define CONAXPVRMANAGER_H

#include <main_decl.h>
#include <QBICSmartcardURIData.h>

/**
 * @file QBConaxPVRManager.h
 * @brief Conax PVR Manager service for enhanced PVR
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBConaxPVRManager Conax PVR Manager service
 * @ingroup CubiTV_services
 * @{
 **/

typedef struct QBConaxPVRManager_t* QBConaxPVRManager;

/**
 * Creates Conax PVR Manager object
 * @param[in] appGlobals appGlobals
 * @return new Conax PVR Manager instance
 **/
QBConaxPVRManager QBConaxPVRManagerCreate(AppGlobals appGlobals);

/**
 * Set up current URI data
 * @param[in] self instance of QBConaxPVRManager
 * @param[in] uriData URI data
 */
void QBConaxPVRManagerSetURIData(QBConaxPVRManager self, QBICSmartcardURIData uriData);

/**
 * @}
 **/

#endif // CONAXPVRMANAGER_H
