/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_MENU_PUSHABLE_WIDGET_H_
#define QB_MENU_PUSHABLE_WIDGET_H_

/**
 * @file QBMenuPushableWidget.h QBMenuPushableWidget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <CUIT/Core/QBCUITWidget.h>
#include <CUIT/Core/QBCUITWidgetInternal.h>
#include <CUIT/Core/QBWidget.h>
#include <CUIT/Core/QBWidgetInternal.h>
#include <SvFoundation/SvCoreTypes.h>
/**
 * @defgroup QBMenuPushableWidget QBMenuPushableWidget is a base class for for widgets that can be pushed to menu
 * @ingroup QBTileGrid
 * @{
 **/

/**
 * QBMenuPushableWidget is a widget which can be pushed in main menu.
 *
 * @class QBMenuPushableWidget
 * @extends QBWidget
 **/
typedef struct QBMenuPushableWidget_ {
    struct QBCUITWidget_ super_;    ///< super class
} *QBMenuPushableWidget;

/**
 * Virtual methods of the pushable widget class.
 **/
typedef const struct QBMenuPushableWidgetVTable_ {
    /// virtual methods table of the base class
    struct QBWidgetVTable_ super_;

    /**
     * Handle input event.
     *
     * @param[in] self    pushable widget handle
     * @param[in] ev      input event to be handled
     *
     * @return            flag indicating if the event was handled
     *
     **/
    bool (*handleInputEvent)(QBMenuPushableWidget self, SvInputEvent ev);

    /**
     * Get focused widget.
     *
     * @param[in] self    pushable widget handle
     * @return            focused widget
     *
     **/
    SvWidget (*getFocusedWidget)(QBMenuPushableWidget self);
} *QBMenuPushableWidgetVTable;

/**
 * Get runtime type identification object representing QBMenuPushableWidget class.
 *
 * @return QBMenuPushableWidget runtime type identification object
 **/
extern SvType
QBMenuPushableWidget_getType(void);

/**
 * Initialize menu pushable widget with given width and height.
 *
 * @memberof QBMenuPushableWidget
 *
 * @param[in] self          tile content handle
 * @param[in] app           application handle
 * @param[in] width         widget width
 * @param[in] height        widget height
 * @param[out] *errorOut    error info
 * @return                  self on success, @c NULL on error
 */
QBMenuPushableWidget
QBMenuPushableWidgetInit(QBMenuPushableWidget self,
                         SvApplication app,
                         unsigned int width,
                         unsigned int height,
                         SvErrorInfo *errorOut);

/**
 * Handle input event.
 *
 * @memberof QBMenuPushableWidget
 *
 * @param[in] self    pushable widget handle
 * @param[in] ev      input event to be handled
 *
 * @return            flag indicating if the event was handled
 **/
extern bool
QBMenuPushableWidgetHandleInputEvent(QBMenuPushableWidget self, SvInputEvent ev);

/**
 * Get focused widget
 *
 * @memberof QBMenuPushableWidget
 *
 * @param[in] self    pushable widget handle
 * @return            focused widget
 *
 **/
extern SvWidget
QBMenuPushableWidgetGetFocusedWidget(QBMenuPushableWidget self);

/**
 * @}
 **/


#endif /* QB_MENU_PUSHABLE_WIDGET_H_ */
