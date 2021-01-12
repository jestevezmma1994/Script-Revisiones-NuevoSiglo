/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBMAINMENULOGIC_H_
#define QBMAINMENULOGIC_H_

#include <stdbool.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <QBContentManager/QBContentTree.h>
#include <main_decl.h>

typedef struct QBMainMenuLogic_t* QBMainMenuLogic;

QBMainMenuLogic QBMainMenuLogicNew(AppGlobals appGlobals) __attribute__((weak));
bool QBMainMenuLogicChosen(QBMainMenuLogic self, SvObject node, SvObject path) __attribute__((weak));
bool QBMainMenuLogicHandleInputEvent(QBMainMenuLogic self, SvInputEvent e) __attribute__((weak));
void QBMainMenuLogicRegisterMenus(QBMainMenuLogic self, SvWidget menuBar, QBTreePathMap pathMap) __attribute__((weak));
void QBMainMenuLogicStart(QBMainMenuLogic self) __attribute__((weak));
void QBMainMenuLogicStop(QBMainMenuLogic self) __attribute__((weak));
void QBMainMenuLogicPostWidgetsCreate(QBMainMenuLogic self) __attribute__((weak));
void QBMainMenuLogicPreWidgetsDestroy(QBMainMenuLogic self) __attribute__((weak));

/**
 * Create provider for QBInnov8onCarouselMenu.
 *
 * @param[in] self                  QBMainMenuLogic handle
 * @param[in] serviceType           service type
 * @param[in] serviceID             service id
 * @param[in] serviceName           service name
 * @param[in] serviceLayout         service layout
 * @param[in] service               object representing service
 * @param[in] call2Action           is call2Action enabled
 * @param[in] externalUrl           call2Action external url
 * @param[out] tree                 tree representing all provided items
 * @param[out] serverInfo           server info
 * @return                          newly created provider object or @c NULL in case of error
 **/
SvObject QBMainMenuLogicCreateProvider(QBMainMenuLogic self,
                                       SvString serviceType,
                                       SvString serviceID,
                                       SvString serviceName,
                                       SvString serviceLayout,
                                       SvDBRawObject service,
                                       bool call2Action,
                                       SvString externalUrl,
                                       SvObject *tree,
                                       SvObject *serverInfo) __attribute__((weak));

/**
 * Function performs special action (if necessary) when some item has been chosen.
 *
 * @param[in] self      QBMainMenuLogic handle
 * @param[in] product   product related to chosen menu item
 * @return              true if choice has been handled
 **/
bool QBMainMenuLogicHandleMenuChoice(QBMainMenuLogic self, SvDBRawObject product);
#endif
