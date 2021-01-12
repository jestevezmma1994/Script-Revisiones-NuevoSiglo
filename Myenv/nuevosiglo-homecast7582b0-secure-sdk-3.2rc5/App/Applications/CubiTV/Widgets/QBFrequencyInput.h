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

#ifndef QBFREQUENCYINPUT_H_
#define QBFREQUENCYINPUT_H_

/**
 * @file QBFrequencyInput.h widget used for frequency input
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <Services/core/QBStandardRastersManager.h>
#include <QBChannelScanningConf.h>
#include <QBStandardRastersParams.h>
#include <main_decl.h>

/**
 * Create from settings and prepare for use widget for inputting frequency.
 *
 * Widget works on integer values.
 *
 * @param[in] app        application handle
 * @param[in] widgetName name of widget
 * @return created frequency input widget, or @c NULL in case of error
 **/
SvWidget QBFrequencyInputNewFromSM(SvApplication app, const char *widgetName);

/**
 * Create from ratio settings and prepare for use widget for inputting frequency.
 *
 * Widget works on integer values.
 *
 * @param[in] app          application handle
 * @param[in] widgetName   name of widget
 * @param[in] parentWidth  width of parent widget
 * @param[in] parentHeight height of parent widget
 * @return created frequency input widget, or @c NULL in case of error
 **/
SvWidget QBFrequencyInputNewFromRatio(SvApplication app, const char *widgetName, unsigned int parentWidth, unsigned int parentHeight);

/**
 * Update bandwidth in widget.
 *
 * @param[in] w            widget handle
 * @param[in] bandwidth    new distance between bands
 **/
void QBFrequencyInputUpdateRange(SvWidget w, int bandwidth);

/**
 * Set parameters used by widget.
 *
 * @param[in] w            widget handle
 * @param[in] standardRastersParams    additional restrictions on possible values
 * @param[in] conf         additional value/range settings
 * @param[in] value        starting value of widget
 * @param[in] bandwidth    distance between bands
 **/
void QBFrequencyInputSetParameters(SvWidget w,
                                   QBStandardRastersParams standardRastersParams,
                                   QBChannelScanningConf conf,
                                   int value,
                                   int bandwidth);

/**
 * Get current value of input field.
 *
 * @param[in] w            widget handle
 * @return value of input
 **/
int QBFrequencyInputGetValue(SvWidget w);

/**
 * Get widget to be focused on.
 *
 * @param[in] w            widget handle
 * @return widget to be handled
 **/
SvWidget QBFrequencyInputGetWidget(SvWidget w);

/**
 * Set new value of input field.
 *
 * @param[in] w            widget handle
 * @param[in] value        new value
 **/
void QBFrequencyInputSetValue(SvWidget w, int value);

#endif
