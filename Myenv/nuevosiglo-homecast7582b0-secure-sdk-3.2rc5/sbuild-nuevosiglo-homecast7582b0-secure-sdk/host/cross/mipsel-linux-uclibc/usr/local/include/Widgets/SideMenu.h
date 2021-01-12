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

#ifndef SIDE_MENU_H_
#define SIDE_MENU_H_

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/QBUIEvent.h>


/**
 * UI event sent by side menu when current level changes.
 * @class QBSideMenuLevelChangedEvent
 * @extends QBUIEvent
 **/
typedef struct QBSideMenuLevelChangedEvent_ *QBSideMenuLevelChangedEvent;

/**
 * QBSideMenuLevelChangedEvent class internals.
 **/
struct QBSideMenuLevelChangedEvent_ {
    /// super class
    struct QBUIEvent_ super_;
    /// level after change
    unsigned int currentLevel;
};

/**
 * Get runtime type identification object representing side menu level changed event class.
 *
 * @relates QBSideMenuLevelChangedEvent
 *
 * @return side menu level changed event class
 **/
extern SvType
QBSideMenuLevelChangedEvent_getType(void);


/**
 * Create new side menu widget.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name
 * @return                  handle to the new side menu widget,
 *                          @c NULL in case of error
 **/
extern SvWidget
SideMenuNew(SvApplication app, const char *widgetName);

/**
 * Returns widget representing a single level. User may attach
 * subwidgets to it, but may not resize, reposition or destroy it.
 *
 * @param[in] sideMenu      side menu object
 * @param[in] level         level of the requested widget
 * @return widget handle
 **/
extern SvWidget
SideMenuGetFrame(SvWidget sideMenu,
                 unsigned int level);

/**
 * Requests that the sidemenu shows @c level levels and notifies
 * about changes using QBSideMenuLevelChangedEvent.
 * It can show the menus with a visual effect or immediately.
 *
 * @param[in] sideMenu      side menu object
 * @param[in] level         how many levels should the sidemenu have
 * @param[in] immediately   true iff no effects should be used and the change should
 *                          be immediately visible  to the user
 **/
extern void
SideMenuShow(SvWidget sideMenu,
             unsigned int level,
             bool immediately);

/**
 * Returns the amount of levels of the menu, requested previously by SideMenuShow()
 * @param[in] sideMenu      side menu object
 * @return requested amount of levels
 **/
extern int
SideMenuGetLevel(SvWidget sideMenu);

/**
 * Allows user to override default width taken from the settings
 * for the specific frame. Should not be used when side menu
 * is moving during the show or hide stage.
 *
 * @param[in] sideMenu      side menu object
 * @param[in] level         menu level, indexes starts with 1
 * @param[in] width         menu width
 * @param[in] immediately   if true sideMenuEffect is not used
 *                          to set frame width
 **/
extern void
SideMenuSetFrameWidth(SvWidget sideMenu,
                      unsigned int level,
                      int width,
                      bool immediately);

/**
 * Allows to obtain final (not current) width of a frame.
 *
 * @param[in] sideMenu      side menu object
 * @param[in] level         menu level
 * @return width of requested frame
 **/
extern int
SideMenuGetFrameWidth(SvWidget sideMenu,
                      unsigned int level);

#endif
