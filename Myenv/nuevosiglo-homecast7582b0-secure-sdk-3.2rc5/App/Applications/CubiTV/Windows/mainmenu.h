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

#ifndef MAINMENU_H
#define MAINMENU_H

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <CUIT/Core/types.h>
#include <QBWindowContext.h>
#include <main_decl.h>

/**
 * @defgroup MainMenu    Main menu context
 * @ingroup CubiTV_windows
 *
 * Main menu context is used to handle CubiTV menu. It allows to register
 * new sub menus (data models, item controllers, etc.) and provides notifications
 * about user actions on the menus.
 *
 * @section MainMenuInit Main Menu initialization
 * The main menu on start will load its skeleton structure from a '.appmenu' file.
 * The path to the file is provided by MainMenuLogic. The file is loaded into a
 * QBActiveTree residing in appGlobals->menuTree and implementing QBTreeModel.
 *
 * @section MainMenuSubMenu Main Menu sub menu creation
 * In order to add a fully fledged submenu a few things need to be done:
 * - the menu must chose a path in menu tree where it will reside. In case the
 *   path is not available it shall not register (this mechanism is used to control
 *   which menus are available, by supplying appropriate .appmenu file).
 * - data source must be provided, it may either be static (from appmenus) or
 *   dynamic, implementing QBTreeModel and mounted under appGlobals->menuTree.
 *   The data source will provide the item list for a submenu.
 * - item controller must be provided, it will be used to construct widgets
 *   representing menu items. The item controller is registered by obtaining
 *   XMBMenuBar handle and using its API.
 * - Main Menu handler must be provided, that will react on user actions in
 *   the menu like chosing a menu or requesting sidemenu showing. In order to
 *   register the handler QBMainMenuContextGetPathMap() must be called and
 *   the handler must be registered for appropriate path. It will receive
 *   all notifications, no matter how deep they happen in the tree.
 *
 *
 * @section MainMenuHandlers Main Menu handlers
 * Main Menu notifies about user actions to two kinds of handlers - global and local.
 * - Global handlers receive all events no matter where in the Main Menu they happen.
 * - Local handlers receive only events that happen in a part of the menu that they
 * should manage. The events are routed based on a simple algorithm, only the handler
 * that has longest common subpath with path of the event will be notified (For
 * details see QBTreePathMap).
 * Usually a local handler is just an object implementing a single submenu, so
 * that it can ignore what is happening in other menus and will get notifications
 * only form its submenu.
 *
 * There are two kinds of events on elements that may be notified:
 * - user selects a leaf item, in this case QBMenuChoice interface is called.
 * - user requests a sidemenu, in this case QBMenuContextChoice interface is called.

 * @{
 **/

typedef enum QBMainMenuState_ {
    QBMainMenuState_Created,
    QBMainMenuState_Destroyed
} QBMainMenuState;

typedef void (*QBMainMenuChanged)(SvGenericObject self_, QBMainMenuState state, SvGenericObject nodePath_);

/**
 * Interface used to obtain changes about global state of the context.
 **/
struct QBMainMenuChanges_t {
    QBMainMenuChanged changed; //< Called when Main Menu is shown/hidden from screen.
};
typedef struct QBMainMenuChanges_t* QBMainMenuChanges;

/**
 * Get runtime type indentification object representing
 * type of Main Menu changes interface.
 *
 * @return Main Menu Changes interface handle
 **/
SvInterface QBMainMenuChanges_getInterface(void);

/**
 * Get runtime type identification object representing
 * type of Main Menu context.
 * @return Main Menu context type
 **/
SvType QBMainMenuContext_getType(void);

/**
 * Creates a new instance of Main Menu Context. Appearance of main menu can be
 * controlled by the Settings Manager. It will use following parameters:
 *
 * | Name               | Type     | Description
 * | ------------------ | -------- | ----------------
 * | width              | integer  | widget width
 * | height             | integer  | widget height
 * | font               | font     | label font
 * | frame              | bool     | if @c true, use QBFrame as background widget, otherwise use SvIcon, default @c false
 * | Bg                 | widget   | background widget, QBFrame or SvIcon, depending on value of 'frame' parameter
 *
 * @param[in] appGlobals        CubiTV application
 * @return instance of Main Menu Context
 **/
QBWindowContext QBMainMenuContextCreate(AppGlobals appGlobals);

/**
 * Requests creation of a new status indicator.
 * @param[in] ctx_          Main Menu Context
 * @param[in] widgetName    name in settings of global indicator status container
 * @param[in] indicatorName name in settings of indicator widget
 * @return    indicator widget
 **/
SvWidget QBMainMenuContextInitAdditionalIndicator(SvObject ctx_, const char *widgetName, const char *indicatorName);

void QBMainMenuAddManualChannelSelectionListener(QBWindowContext ctx_, SvGenericObject listener, SvGenericObject node);

/**
 * @}
 **/

#endif

