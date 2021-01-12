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

#ifndef QBNUMERICINPUT_H_
#define QBNUMERICINPUT_H_

/**
 * @file QBNumericInput.h widget used for any numeric input
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>

/**
 * Create from settings and prepare for use widget for inputting numerical values.
 *
 * Widget works on integer values, but can display them as fixed-point numbers.
 *
 * @param[in] app        application handle
 * @param[in] widgetName name of widget
 * @return created numeric input widget, or @c NULL in case of error
 **/
SvWidget QBNumericInputNewFromSM(SvApplication app, const char *widgetName);

/**
 * Create from ratio settings and prepare for use widget for inputting numerical values.
 *
 * Widget works on integer values, but can display them as fixed-point numbers.
 *
 * @param[in] app          application handle
 * @param[in] widgetName   name of widget
 * @param[in] parentWidth  width of parent widget
 * @param[in] parentHeight height of parent widget
 * @return created numeric input widget, or @c NULL in case of error
 **/
SvWidget QBNumericInputNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight);

/**
 * Set initial value, range of values and default.
 *
 * @param[in] w            widget handle
 * @param[in] initialValue initial value of input field
 * @param[in] minValue     minimal allowed value
 * @param[in] maxValue     maximal allowed value
 **/
void QBNumericInputSetValueAndRange(SvWidget w, int initialValue, int minValue, int maxValue);

/**
 * Set width of decimal part of number.
 *
 * If width is above 0, number is displayed as fixed-point, otherwise as integer.
 *
 * @param[in] w            widget handle
 * @param[in] width        fixed width of decimal part, in decimal system
 **/
void QBNumericInputSetDecimalPartWidth(SvWidget w, unsigned int width);

/**
 * Get current value of input field.
 *
 * @param[in] w            widget handle
 * @return value of input
 **/
int QBNumericInputGetValue(SvWidget w);

#endif
