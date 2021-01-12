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

#ifndef QB_CHANNEL_INSTALLATION_MENU_ITEM_CHOICE_H_
#define QB_CHANNEL_INSTALLATION_MENU_ITEM_CHOICE_H_

/**
 * @file QBChannelInstallationMenuItemChoice.h Channel Installation menu item choice
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>
#include <QBDataModel3/QBActiveTree.h>
#include <main_decl.h>

/**
 * @defgroup QBChannelInstallationMenuItemChoice Channel Installation menu item choice
 * @ingroup QBNewConfigurationMenu
 * @{
 */

/**
 * Channel Installation menu item choice class.
 *
 * @class QBChannelInstallationMenuItemChoice QBChannelInstallationMenuItemChoice.h  <Menus/ConfigurationMenu/QBChannelInstallationMenuItemChoice.h>
 * @extends SvObject
 * @implements QBMenuEventHandler
 **/
typedef struct QBChannelInstallationMenuItemChoice_ *QBChannelInstallationMenuItemChoice;

/**
 * Create new Channel Installation menu item choice.
 *
 * @public @memberof QBChannelInstallationMenuItemChoice
 *
 * @param[in] appGlobals    application globals
 * @param[in] menu          menu for which this item choice is created
 * @param[in] tree          tree for which this item choice is created
 * @param[out] *errorOut    error info
 * @return    New Channel Installation menu item choice or @c NULL in case of error
 **/
QBChannelInstallationMenuItemChoice
QBChannelInstallationMenuItemChoiceCreate(AppGlobals appGlobals, SvWidget menu, QBActiveTree tree, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_CHANNEL_INSTALLATION_MENU_ITEM_CHOICE_H_ */
