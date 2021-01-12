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

#ifndef XMB_MENU_H_
#define XMB_MENU_H_

/**
 * @file XMBMenu.h
 * @brief Xross Media Bar menu interface
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/event.h>
#include <XMB2/XMBTypes.h>

/**
 * @defgroup XMBMenu XMB menu interface
 * @ingroup XMB
 * @{
 *
 * An interface for classes that implement XMB vertical menu widgets.
 **/

typedef enum {
  XMBMenuEventType_unknown = 0,
  XMBMenuEventType_selected,
  XMBMenuEventType_reordered,

  XMBMenuEventType_cnt,
} XMBMenuEventType;

/**
 * Private data of XMB notification events.
 **/
typedef struct XMBMenuEvent_ {
    /// previous position in the XMB menu
    unsigned int prevPosition;
    /// new position in the XMB menu
    unsigned int position;
    /// @c true if menu item was clicked, @c false if it was only focused
    bool clicked;
    /// type of the event
    XMBMenuEventType type;
} *XMBMenuEvent;


/**
* @brief Callback used to notify widget about changes.
*
* @param[in] target     object that will be notifed about changes
* @param[in] data       XMBMenuEvent - information about changes
*/
typedef void (*XMBMenuNotificationCallback)(void *target, const XMBMenuEvent data);

/**
 * XMB menu interface.
 **/
typedef const struct XMBMenu_ {
    /**
     * Attach XMB menu to data source.
     *
     * @param[in] self_         XMB menu handle
     * @param[in] dataSource    handle to a QBTreeModel implementation
     * @param[in] path          path to the parent node whose child nodes
     *                          are to be presented in the menu
     * @param[in] controller    handle to an item controller
     * @param[out] errorOut     error info
     **/
    void (*connectToDataSource)(SvObject self_,
                                SvObject dataSource,
                                SvObject path,
                                SvObject controller,
                                SvErrorInfo *errorOut);

    /**
     * Set target for menu notification events.
     *
     * @param[in] self_         XMB menu handle
     * @param[in] target        object that will receive
     *                          notification events from menu,
     *                          @c 0 to disable notifications
     * @param[in] callback      pointer to implementation of
     *                          notification callback - this
     *                          function is used on target to
     *                          notify
     **/
    void (*setNotificationTarget)(SvObject self_,
                                  void *target,
                                  XMBMenuNotificationCallback callback);

    /**
     * Get item controller used by XMB menu.
     *
     * @param[in] self_         XMB menu handle
     * @return                  handle to an item controller,
     *                          @c NULL if not available
     **/
    SvObject (*getItemController)(SvObject self_);

    /**
     * Set item controller to be used by XMB menu.
     *
     * @param[in] menu          XMB menu handle
     * @param[in] controller    handle to an item controller
     * @param[out] errorOut     error info
     **/
    void (*setItemController)(SvObject self_,
                              SvObject controller,
                              SvErrorInfo *errorOut);

    /**
     * Get a handle to CUIT widget implementing XMB menu.
     *
     * @param[in] self_         XMB menu handle
     * @return                  menu widget handle
     **/
    SvWidget (*getWidget)(SvObject self_);

    /**
     * Get extra move value.
     *
     * @param[in] self_         XMB menu handle
     * @return                  amount of extra horizontal space
     *                          needed by this menu, in pixels
     **/
    int (*getExtraSpace)(SvObject self_);

    /**
     * Get cursor position.
     *
     * @param[in] self_         XMB menu handle
     * @return                  current cursor position,
     *                          @c -1 in case of error
     **/
    int (*getPosition)(SvObject self_);

    /**
     * Set cursor position in a menu.
     *
     * @param[in] self_         XMB menu handle
     * @param[in] pos           new cursor position
     * @param[in] immediately   @c true to skip animations
     * @param[out] errorOut     error info
     **/
    void (*setPosition)(SvObject self_,
                        unsigned int pos,
                        bool immediately,
                        SvErrorInfo *errorOut);

    /**
     * Handle input event.
     *
     * @param[in] self_         XMB menu handle
     * @param[in] iev           CUIT input event
     * @return                  @c true if event is consumed
     **/
    bool (*handleInputEvent)(SvObject self_,
                             SvInputEvent iev);

    /**
     * Change menu state.
     *
     * @param[in] self_         XMB menu handle
     * @param[in] state         new state
     **/
    void (*setState)(SvObject self_,
                     XMBMenuState state);

    /**
     * Change menu focus.
     *
     * @param[in] self_         XMB menu handle
     * @param[in] focus         new focus state
     **/
    void (*setFocus)(SvObject self_,
                     bool focus);
} *XMBMenu;


/**
 * Get runtime type identification object representing
 * XMBMenu interface.
 *
 * @return XMBMenu interface object
 **/
extern SvInterface
XMBMenu_getInterface(void);


/**
 * @}
 **/

#endif
