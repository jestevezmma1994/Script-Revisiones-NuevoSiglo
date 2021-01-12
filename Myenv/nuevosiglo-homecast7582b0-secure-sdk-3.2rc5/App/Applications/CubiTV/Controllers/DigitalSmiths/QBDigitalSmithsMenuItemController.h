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

#ifndef QB_DIGITAL_SMITHS_MENU_ITEM_CONTROLLER_H_
#define QB_DIGITAL_SMITHS_MENU_ITEM_CONTROLLER_H_

/**
 * @file QBDigitalSmithsMenuItemController.h DigitalSmiths  menu item controller
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <QBContentManager/QBContentProvider.h>
#include <main_decl.h>

/**
 * @defgroup QBDigitalSmithsMenuItemController Digital smiths meun item controller
 * @ingroup CubiConnect_controllers
 * @{
 **/

/**
 * DigitalSmiths menu item controller class.
 *
 * @class QBDigitalSmithsMenuItemController QBDigitalSmithsMenuItemController.h <Controllers/DigitalSmiths/QBDigitalSmithsItemController.h>
 * @extends SvObject
 * @implements QBMenuEventHandler
 * @implements QBTileItemController
 * @implements QBTileOSKController
 **/
typedef struct QBDigitalSmithsMenuItemController_ *QBDigitalSmithsMenuItemController;

/**
 * Create new DigitalSmiths menu item controller.
 *
 * @param[in] appGlobals    application globals
 * @param[in] provider      digitalsmith provider handle
 * @param[in] serviceId     service identifier
 * @param[in] serviceName   service name
 * @param[in] serviceType   service type
 * @param[in] serviceLayout service layout
 * @return    New DigitalSmiths menu item controller or @c NULL in case of error
 **/
QBDigitalSmithsMenuItemController
QBDigitalSmithsMenuItemControllerCreate(AppGlobals appGlobals,
                                        QBContentProvider provider,
                                        SvString serviceId,
                                        SvString serviceName,
                                        SvString serviceType,
                                        SvString serviceLayout);

/**
 * @}
 **/

#endif /* QB_DIGITAL_SMITHS_MENU_ITEM_CONTROLLER_H_ */
