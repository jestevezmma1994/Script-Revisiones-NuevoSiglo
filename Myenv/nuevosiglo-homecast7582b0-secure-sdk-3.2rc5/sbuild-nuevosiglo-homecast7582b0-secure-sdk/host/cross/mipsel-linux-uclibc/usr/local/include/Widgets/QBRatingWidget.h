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

#ifndef QBRATINGWIDGET_H_
#define QBRATINGWIDGET_H_

/**
 * @file QBRatingWidget.h Rating widget class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <stdbool.h>

/**
 * @defgroup QBRatingWidget Rating widget class
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * RatingWidget consist of icons. Icons are loaded from path described in attributes:
 *    - focusedOn - rated icon on focused widget
 *    - focusedOff - not rated icon on focused widget
 *    - nonfocusedOn - rated icon on non focused widget
 *    - nonfocusedOff - not rated icon on non focused widget
 * Attribute 'itemCount' describe how many icons will be shown.
 * Icons detailed parameters should be defined in subwidget 'Item'.
 **/

/**
 * Rating widget is created for setting.
 * Dimentions are expressed in ratio of width or height.
 * Spacing between icons is defined with 'spacingRatio'. 'spacingRatio' is a ratio from width.
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget name defined in settings
 * @param[in] width        maximum width
 * @param[in] height       maximum height
 * @param[in] focused      defined if widget is focused
 * @return                 @c rating widget, if creation ends with success, @c NULL, otherwise
 **/
SvWidget QBRatingWidgetCreateWithRatio(SvApplication app, const char* widgetName,
                                       unsigned int width, unsigned int height, bool focused);

/**
 * Rating widget is created from setting.
 * Dimentions are expressed in pixels.
 * Spacing between icons is defined with 'spacing'.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name defined in settings
 * @return                 @c rating widget, if creation ends with success, @c NULL, otherwise
 **/
SvWidget QBRatingWidgetCreate(SvApplication app, const char* widgetName);

/**
 * Get [rating value] -> [icons count] conversion factor.
 *
 * @param[in] w             rating widget handle
 * @return                  @c rating widget rating conversion resolution
 **/
unsigned int QBRatingWidetGetResolution(SvWidget w);

/**
 * Set rating widget's rating value.
 * It will be then scaled to icons count using resolution param.
 *
 * @param[in] w         rating widget handle
 * @param[in] rating    rating value
 **/
void QBRatingWidetSetRating(SvWidget w, unsigned int rating);

/**
 * @}
 **/

#endif
