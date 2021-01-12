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

#ifndef QB_MAIN_MENU_INTERFACE_H_
#define QB_MAIN_MENU_INTERFACE_H_

/**
 * @file QBMainMenuInterface.h
 * @brief Main Menu interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <CUIT/Core/types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMainMenu Main Menu interface
 * @ingroup QBMenu
 * @{
 *
 * An interface for handling Main Menu.
 **/

/**
 * Main Menu interface.
 **/
typedef const struct QBMainMenu_ {
    /**
     * Initialize Main Menu.
     *
     * This method must be called before doing any action with Main Menu.
     *
     * @param[in] self_ Main Menu Context handle
     **/
    void (*init)(SvObject self_);

    /**
     * Deinitialize Main Menu.
     *
     * @param[in] self_ Main Menu Context handle
     **/
    void (*deinit)(SvObject self_);

    /**
     * Whenever user selects an item in menu, handlers registered using this
     * method will be notified about that.
     *
     * @note The handlers must implement QBMenuChoice interface.
     *
     * @param[in] self_     Main Menu Context handle
     * @param[in] handler   object that will be notified
     **/
    void (*addGlobalHandler)(SvObject self_, SvObject handler);

    /**
     * Returns instance of Main Menu widget.
     *
     * Returned widget will be used as main widget for menu.
     *
     * @param[in] self_ Main Menu Context handle
     * @return          Main Menu widget
     **/
    SvWidget (*getMenu)(SvObject self_);

    /**
     * Switch to specific node.
     *
     * Switches to an existing node in the top-level menu tree, not in
     * mounted subtrees. If actual context is Menu then just jump into new
     * node, otherwise switch context then jump into.
     *
     * @param[in] self_     Main Menu Context handle
     * @param[in] nodeID    node id
     * @returns             @c true if @a nodeID was found in the main menu tree,
     *                      otherwise no switch is done
     */
    bool (*switchToNode)(SvObject self_, SvString nodeID);

    /**
     * Return menu tree path map.
     *
     * Return path map that can be used to register local handlers, see @ref MainMenuHandlers.
     *
     * @param[in] self_     Main Menu Context handle
     * @return              internal tree path map
     **/
    QBTreePathMap (*getPathMap)(SvObject self_);

    /**
     * Return menu tree skin path map.
     *
     * Return skin path map that can be used to register skin local handlers
     *
     * @param[in] self_     Main Menu Context handle
     * @return              internal tree skin path map
     **/
    QBTreePathMap (*getSkinPathMap)(SvObject self_);
} *QBMainMenu;


/**
 * Get runtime type identification object representing QBMainMenu interface.
 *
 * @return QBMainMenu interface object
 **/
extern SvInterface
QBMainMenu_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
