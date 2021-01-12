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

#ifndef QB_CUIT_WIDGET_H_
#define QB_CUIT_WIDGET_H_

/**
 * @file QBCUITWidget.h QBCUITWidget external API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Core/SvColor.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBCUITWidget QBCUITWidget
 * @brief QBCUITWidget is a graphical element of the screen,
 * it's main purpouse is to provide mechanism for
 * widgets methods type correctness and to prevent code duplication.
 * @ingroup CUIT
 * @{
 **/

/**
 * Create widget with given width and height
 *
 * @memberof QBCUITWidget
 *
 * @param[in] app           application handle
 * @param[in] width         widget width
 * @param[in] height        widget height
 * @param[out] *errorOut    error info
 * @return                  created widget, @c NULL in case of error
 */
QBCUITWidget
QBCUITWidgetCreate(SvApplication app,
                   unsigned int width,
                   unsigned int height,
                   SvErrorInfo *errorOut);

/**
 * Attach SvWidget to parent widget.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] parent        parent widget
 * @param[in] child         child widget
 * @param[in] xOffset       x coordinate of top-left corner of widget (parent relative)
 * @param[in] yOffset       y coordinate of top-left corner of widget (parent relative)
 * @param[in] level         Z-order of the widget (higher number = closer to viewer)
 **/
void
QBCUITWidgetAttachSvWidget(QBCUITWidget parent, SvWidget child,
                           int xOffset, int yOffset, unsigned short int level);

/**
 * Set widgets alpha value.
 *
 * @memberof QBCUITWidget
 *
 * Alpha value affects only widget on which it was set.
 * Final alpha of the widget is calculated by multiplication of
 * widgets alpha and product of multiplication of all
 * tree alpha values up to the windows widgets tree root.
 *
 * @param[in] self          widget handle
 * @param[in] alpha         alpha value
 */
void
QBCUITWidgetSetAlpha(QBCUITWidget self, SvPixel alpha);

/**
 * Set widgets tree alpha value.
 *
 * @memberof QBCUITWidget
 *
 * Tree alpha value affects whole widgets sub tree
 * that originates from this widget.
 * Final alpha of the widget is calculated by multiplication of
 * widgets alpha and product of multiplication of all
 * tree alpha values up to the widgets tree root.
 *
 * @param[in] self          widget handle
 * @param[in] treeAlpha     tree alpha value
 */
void
QBCUITWidgetSetTreeAlpha(QBCUITWidget self, SvPixel treeAlpha);

/**
 * Check if widget has focus.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self          widget handle
 * @return                  @c true if widget has focus, @c false otherwise
 */
bool
QBCUITWidgetIsFocused(QBCUITWidget self);

/**
 * Set focused widget.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self           widget handle
 */
void
QBCUITWidgetSetFocus(QBCUITWidget self);

/**
 * Search for closest child of the widget.
 *
 * @param[in] self           widget handle
 */
void
QBCUITWidgetSetFocusToClosestChild(QBCUITWidget self);

/**
 * Set widget focus policy.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self           widget handle
 * @param[in] isFocusable    @c true if widget should accept focus otherwise @c false
 **/
void
QBCUITWidgetSetFocusable(QBCUITWidget self, bool isFocusable);

/**
 * Is widget hidden.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self           widget handle
 * @return                   @c true if widget is hidden otherwise @c false
 **/
bool
QBCUITWidgetIsHidden(QBCUITWidget self);

/**
 * Hide or show widget.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self           widget handle
 * @param[in] hidden         @c true to hide widget, @c false to show hidden widget
 */
void
QBCUITWidgetSetHidden(QBCUITWidget self, bool hidden);

/** Set widget's background bitmap.
 *
 * @memberof QBCUITWidget
 *
 * @param[in] self           widget handle
 * @param[in] bmp            new bitmap
*/
void
QBCUITWidgetSetBitmap(QBCUITWidget self, SvBitmap bmp);


/**
 * @}
 **/

#endif /* QB_CUIT_WIDGET_H_ */
