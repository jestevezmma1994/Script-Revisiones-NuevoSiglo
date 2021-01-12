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

#ifndef XMB_ITEM_CONTROLLER_H_
#define XMB_ITEM_CONTROLLER_H_

/**
 * @file XMBItemController.h
 * @brief Xross Media Bar item controller interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <XMB2/XMBTypes.h>


/**
 * @defgroup XMBItemController XMB item controller interface
 * @ingroup XMB
 * @{
 *
 * An interface for classes that create and manage state of widgets
 * representing XMB menu items.
 **/

/**
 * XMB item controller interface.
 **/
typedef const struct XMBItemController_t {
    /**
     * Create new XMB item widget.
     *
     * This method creates new item widget, that will be used to represent
     * single node of a list or a tree in XMB menu widget. Widget should
     * be initially in normal state.
     *
     * @param[in] self_     handle to object implementing item controller
     * @param[in] node      menu node this item will represent
     * @param[in] path      tree path describing @a node location in the tree
     *                      (@c NULL for standalone XMBMenu widgets)
     * @param[in] app       CUIT application handle
     * @param[in] initialState state of the menu this item belongs to
     * @return              handle to the newly created widget,
     *                      @c NULL in case of error
     **/
    SvWidget (*createItem)(SvObject self_,
                           SvObject node,
                           SvObject path,
                           SvApplication app,
                           XMBMenuState initialState);

    /**
     * Change state of an XMB item widget.
     *
     * @param[in] self_     handle to object implementing item controller
     * @param[in] item      item widget handle
     * @param[in] menuState state of the menu this item belongs to
     * @param[in] isFocused @c true if item is focused, @c false if not
     **/
    void (*setItemState)(SvObject self_,
                         SvWidget item,
                         XMBMenuState menuState,
                         bool isFocused);
} *XMBItemController;


/**
 * Get runtime type identification object representing
 * SvSequence interface.
 *
 * @return XMBItemController interface object
 **/
extern SvInterface
XMBItemController_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup XMBExtItemController XMB extended item controller interface
 * @ingroup XMB
 * @{
 *
 * An extended version of @ref XMBItemController.
 *
 * This is an extended version of @ref XMBItemController. It allows
 * to create XMB menu item widgets with explicitly specified initial state.
 **/

/**
 * XMB extended item controller interface.
 **/
typedef const struct XMBExtItemController_ {
    /**
     * Super interface.
     **/
    struct XMBItemController_t super_;

    /**
     * Create new XMB item widget.
     *
     * This method creates new item widget, that will be used to represent
     * single node of a vertical menu being a part of XMB menu tree.
     *
     * @param[in] self_     handle to an XMBExtItemController implementation
     * @param[in] dataSource handle to a QBListModel implementation
     * @param[in] idx       index of a menu node in @a dataSource that this item will represent
     * @param[in] app       CUIT application handle
     * @param[in] initialState state of the menu this item belongs to
     * @param[in] isFocused @c true if item should be initially focused
     * @return              handle to the newly created widget,
     *                      @c NULL in case of error
     **/
    SvWidget (*createMenuItem)(SvObject self_,
                               SvObject dataSource,
                               size_t idx,
                               SvApplication app,
                               XMBMenuState initialState,
                               bool isFocused);
} *XMBExtItemController;


/**
 * Get runtime type identification object representing
 * XMBExtItemController interface.
 *
 * @return XMBExtItemController interface object
 **/
extern SvInterface
XMBExtItemController_getInterface(void);

/**
 * @}
 **/

#endif
