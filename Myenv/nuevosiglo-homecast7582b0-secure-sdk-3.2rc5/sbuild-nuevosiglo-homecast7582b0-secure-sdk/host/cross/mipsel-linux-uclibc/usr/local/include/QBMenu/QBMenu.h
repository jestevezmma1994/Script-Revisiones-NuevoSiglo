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

#ifndef QBMENU_H_
#define QBMENU_H_

/**
 * @file QBMenu.h Abstract main menu interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBMenu/QBMenuPushableWidget.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMenu QBMenu: application's main menu abstraction library
 * @ingroup GUI
 **/


/**
 * @defgroup QBMenuInterface Menu interface
 * @ingroup QBMenu
 * @{
 *
 * An interface for controlling application's menu.
 **/

/// Menu selected user event type.
#define SV_EVENT_QB_MENU_SELECTED 425

/**
 * Get runtime type identification object representing QBMenu interface.
 *
 * @return QBMenu interface object
 **/
SvInterface QBMenu_getInterface(void);

/**
 * Callback function called when user tries to enter into the menu.
 *
 * @param target         opaque pointer to callback function's private data
 * @param dataSource     data source of the current menu
 * @param event          input event received by the menu
 * @return               @c true if event was consumed, otherwise @c false
 **/
typedef bool (*QBMenuOnEnterCallback)(void *target,
                                      SvObject dataSource,
                                      SvInputEvent event);

/**
 * Private data of notification events.
 **/
typedef struct QBMenuEvent_ {
    /// selected tree node or @c NULL
    SvObject node;
    /// path of the selected tree node or @c NULL
    SvObject nodePath;
    /// @c true if menu item was clicked, @c false if it was only focused
    bool clicked;
    /**
     * position of the node in menu; this is only informative, you should
     * not trust this information, because it can change before event
     * is delivered
     **/
    int position;
} *QBMenuEvent;

typedef const struct QBMenu_ {
    /**
     * Connect menu widget to data source.
     *
     * @param[in]  self_            menu widget handle
     * @param[in]  dataSource       handle to a @ref QBTreeModel implementation
     * @param[in]  menuController   handle to an item controller for main menu
     * @param[in]  menuEventHandler handle to default event handler
     * @param[in]  controller       handle to an item controller for submenus
     * @param[out] errorOut         error info
     **/
    void (*connectToDataSource)(SvObject self_, SvObject dataSource, SvObject menuController,
                                SvObject menuEventHandler, SvObject controller, SvErrorInfo *errorOut);

    /**
     * Set target for menu notification events.
     *
     * @param[in] self_         menu widget handle
     * @param[in] target        ID of the widget that will receive
     *                          notification events from menu widget,
     *                          @c 0 to disable notifications
     **/
    void (*setNotificationTarget)(SvObject self_, SvWidgetId target);

    /**
     * Get cursor position.
     *
     * @param[in] self_         menu widget handle
     * @return                  current cursor position,
     *                          @c NULL in case of error
     **/
    SvObject (*getPosition)(SvObject self_);

    /**
     * Set cursor position in a menu.
     *
     * @param[in]  self_        menu widget handle
     * @param[in]  path_        new cursor position
     * @param[out] errorOut     error info
     **/
    void (*setPosition)(SvObject self_, SvObject path_, SvErrorInfo *errorOut);

    /**
     * Set specialized item controller for given tree path.
     *
     * @param[in] self_         menu widget handle
     * @param[in] path          tree path for which controller will be used
     * @param[in] controller    handle to an item controller for menus,
     *                          @c NULL to remove controller for @a path
     * @param[out] errorOut     error info
     **/
    void (*setItemControllerForPath)(SvObject self_, SvObject path_, SvObject controller, SvErrorInfo *errorOut);

    /**
     * Get specialized item controller for given tree path.
     *
     * @param[in] self_         menu widget handle
     * @param[in] path          tree path for which to get the controller
     * @param[out] errorOut     error info
     * @return                  handle to an item controller for menus
     **/
    SvObject (*getItemControllerForPath)(SvObject self_, SvObject path_, SvErrorInfo *errorOut);

    /**
     * Select cursor position on current lowest level menu.
     *
     * @param[in] self_         menu widget handle
     * @param[in] parentPath    parent path
     * @param[in] position      new position (counting from 0)
     * @param[in] immediate     should it be immediate
     **/
    void (*setSubmenuPosition)(SvObject self_, SvObject parentPath, int position, bool immediate);

    /**
     * Set event handler for given tree path.
     *
     * @param[in] self_         menu widget handle
     * @param[in] path_         tree path for which event handler will be used
     * @param[in] handler       handle to an menu bar event handler,
     *                          @c NULL to remove handler for @a path
     * @param[out] errorOut     error info
     **/
    void (*setEventHandlerForPath)(SvObject self_, SvObject path_, SvObject handler, SvErrorInfo *errorOut);

    /**
     * Get event handler for given tree path.
     *
     * @param[in] self_         menu widget handle
     * @param[in] path          tree path for which to get the handler
     * @param[out] errorOut     error info
     * @return                  handle to an item event handler for menus
     **/
    SvObject (*getEventHandlerForPath)(SvObject self_, SvObject path_, SvErrorInfo *errorOut);

    /**
     * Set current menu active state.
     *
     * @param[in] self_         menu widget handle
     * @param[in] isActive      active state
     **/
    void (*setOnEnterCallback)(SvObject self_, void *target, QBMenuOnEnterCallback callback);

    /**
     * Set current menu active state.
     *
     * @param[in] self_         menu widget handle
     * @param[in] isActive      active state
     **/
    void (*setCurrentMenuActiveState)(SvObject self_, bool isActive);

    /**
     * Push widget to QBTileGrid as another level
     *
     * @param[in] self_         menu widget handle
     * @param[in] immediate     should it happen without stacking effect
     * @param[in] widget        widget to be pushed
     **/
    void (*pushWidget)(SvObject self_, QBMenuPushableWidget widget, bool immediate, SvErrorInfo *errorOut);

    /**
     * Go to previous menu element
     *
     * @param[in] self_         menu widget handle
     * @param[in] immediate     should it happen without stacking effect
     **/
    void (*goToPreviousElement)(SvObject self_, bool immediate, SvErrorInfo *errorOut);
} *QBMenu;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif /* QBMENU_H_ */
