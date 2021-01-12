/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONFIGURATION_MENU_DEFAULT_ITEM_CONTROLLER_H_
#define QB_CONFIGURATION_MENU_DEFAULT_ITEM_CONTROLLER_H_

/**
 * @file QBConfigurationMenuDefaultItemController.h Configuration menu default item controller
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBDataModel3/QBActiveTree.h>
#include <main_decl.h>

/**
 * @defgroup QBConfigurationMenuDefaultItemController Configuration menu default item controller
 * @ingroup QBNewConfigurationMenu
 * @{
 */

/**
 * Network Information menu item controller class.
 *
 * @class QBConfigurationMenuDefaultItemController QBConfigurationMenuDefaultItemController.h  <Menus/ConfigurationMenu/QBConfigurationMenuDefaultItemController.h>
 * @extends SvObject
 * @implements XMBItemController
 **/
typedef struct QBConfigurationMenuDefaultItemController_ *QBConfigurationMenuDefaultItemController;

/**
 * Create new configuration menu default item controller.
 *
 * @public @memberof QBConfigurationMenuDefaultItemController
 *
 * @param[in] appGlobals    application globals
 * @param[in] tree          tree for which this item choice is created
 * @param[out] *errorOut    error info
 * @return    New configuration menu default item controller or @c NULL in case of error
 **/
QBConfigurationMenuDefaultItemController
QBConfigurationMenuDefaultItemControllerCreate(AppGlobals appGlobals, QBActiveTree tree, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_CONFIGURATION_MENU_DEFAULT_ITEM_CONTROLLER_H_ */
