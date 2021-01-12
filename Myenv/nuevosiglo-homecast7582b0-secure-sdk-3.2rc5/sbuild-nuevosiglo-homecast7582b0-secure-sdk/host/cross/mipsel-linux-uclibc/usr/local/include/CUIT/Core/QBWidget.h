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

#ifndef QB_WIDGET_H_
#define QB_WIDGET_H_

/**
 * @file QBWidget.h QBWidget external API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBWidget QBWidget
 * @brief QBWidget is a element from which screen is composed.
 * Widget might have other widgets attached to it and it may also
 * be attached to other widgets. Basically what's displayed on the screen is
 * rendered representation of windows tree of widgets.
 * @ingroup CUIT
 * @{
 **/

/**
 * Attach child widget.
 *
 * @memberof QBWidget
 *
 * @param[in] self          parent widget handle
 * @param[in] child         child widget
 * @param[in] xOffset       x coordinate of top-left corner of widget (parent relative)
 * @param[in] yOffset       y coordinate of top-left corner of widget (parent relative)
 * @param[in] level         Z-order of the widget (higher number = closer to viewer)
 **/
void
QBWidgetAttachChild(QBWidget self, const struct QBWidget_ *child,
                    int xOffset, int yOffset, unsigned short int level);

/**
 * Detach widget from parent.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 **/
void
QBWidgetDetach(QBWidget self);

/**
 * Get widget width.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @return                  widget width
 */
unsigned int
QBWidgetGetWidth(QBWidget self);

/**
 * Get widget height.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @return                  widget height
 */
unsigned int
QBWidgetGetHeight(QBWidget self);

/**
 * Set widget width.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @param[in] width         widget width
 */
void
QBWidgetSetWidth(QBWidget self, unsigned int width);

/**
 * Set widget height.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @param[in] height        widget height
 */
void
QBWidgetSetHeight(QBWidget self, unsigned int height);

/**
 * Get widget x offset.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @return                  widget x offset
 */
int
QBWidgetGetXOffset(QBWidget self);

/**
 * Get widget y offset.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @return                  widget y offset
 */
int
QBWidgetGetYOffset(QBWidget self);

/**
 * Set widget x offset.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @param[in] xOffset       widget x offset
 */
void
QBWidgetSetXOffset(QBWidget self, int xOffset);

/**
 * Set widget y offset.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 * @param[in] yOffset       widget y offset
 */
void
QBWidgetSetYOffset(QBWidget self, int yOffset);

/**
 * Detaches all children widgets.
 *
 * @memberof QBWidget
 *
 * @param[in] self          widget handle
 */
void
QBWidgetDetachChildren(QBWidget self);

/**
 * @}
 **/

#endif /* QB_WIDGET_H_ */
