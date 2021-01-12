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

#ifndef QBSCROLLBAR_H_
#define QBSCROLLBAR_H_

#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

/**
 * Create scroll bar widget using settings from the Settings Manager.
 *
 * This method creates scroll bar widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - width, height : integer, required,
 *  - arrowLow, arrowHeight : widget, required,
 *  - mode : string ("vertical" or "horizontal"), optional (default value "vertical"),
 *  - arrowFadeDuration : double, optional.
 *
 * @param[in] app             CUIT application handle
 * @param[in] name            widget name
 * @return                    newly created widget or NULL in case of error
 **/
extern SvWidget
QBScrollBarNew(SvApplication app, const char *name);

/**
 * Create scroll bar widget using ratio settings from the Settings Manager.
 *
 * This method creates scroll bar widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use following parameters:
 *
 *  - widthRatio, heightRatio : double, required,
 *  - arrowLow, arrowHeight : widget with ratio settings, required,
 *  - mode : string ("vertical" or "horizontal"), optional (default value "vertical"),
 *  - arrowFadeDuration : double, optional.
 *
 * @param[in] app             CUIT application handle
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @param[in] name            widget name
 * @return                    newly created widget or NULL in case of error
 **/
extern SvWidget
QBScrollBarNewWithRatio(SvApplication app,
                        unsigned int parentWidth,
                        unsigned int parentHeight,
                        const char *name);

/**
 * Get object implementing SvRangeListener interface that is connected with the scroll bar.
 *
 * @param[in] w               scroll bar widget
 * @return                    range listener
 **/
extern SvGenericObject
QBScrollBarGetRangeListener(SvWidget w);

#endif /* QBSCROLLBAR_H_ */
