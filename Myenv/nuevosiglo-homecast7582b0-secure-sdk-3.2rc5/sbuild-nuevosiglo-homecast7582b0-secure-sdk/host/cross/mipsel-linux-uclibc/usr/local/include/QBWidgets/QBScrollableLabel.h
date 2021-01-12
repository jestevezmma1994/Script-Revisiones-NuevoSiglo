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

#ifndef QB_SCROLLABLE_LABEL_H
#define QB_SCROLLABLE_LABEL_H

#include <Services/core/QBTextRenderer.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @file QBScrollableLabel.h QBScrollableLabel class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBScrollableLabel QBScrollableLabel: Asynchronous label widget with scrolling text
 * @ingroup QBWidgets
 * @{
 *
 * Asynchronous label widget created from the Settings Manager with text scrolling
 **/

/**
 * Create scrolling label widget using ratio settings from the Settings Manager.
 *
 * This method creates asynchronous label widget. It use QBAsyncLabel as a label widget.
 * Speed is defined as a number of screen scrolled per second.
 *
 * It uses parameters from QBAsyncLabel and:
 *
 *  - scrollSpeed : double, optional.
 *  - fullScroll : bool, optional.
 *  - scrollableTextAlign : string, optional
 *
 * @param[in] app             CUIT application handle
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @param[in] widgetName      widget name
 * @param[in] renderer        QBTextRenderer for async text rendering
 * @return                    newly created widget or NULL in case of error
 **/
SvWidget QBScrollableLabelNewFromRatio(SvApplication app,
                                       unsigned int parentWidth,
                                       unsigned int parentHeight,
                                       const char *widgetName,
                                       QBTextRenderer renderer);

/**
 * Set label contents.
 *
 * @param[in] label          label widget handle
 * @param[in] text           text to be displayed
 **/
void QBScrollableLabelSetText(SvWidget label, SvString text);

/**
 * Clear label from text.
 *
 * @param[in] label          label widget handle
 **/
void QBScrollableLabelClear(SvWidget label);

/**
 * Start scrolling.
 *
 * @param[in] label          label widget handle
 **/
void QBScrollableLabelStartScrolling(SvWidget label);

/**
 * Stop scrolling.
 *
 * @param[in] label          label widget handle
 * @return                   @c 1 on success, @c 0 otherwise
 **/
int QBScrollableLabelStopScrolling(SvWidget label);

/**
 * @}
 **/

#endif // QB_SCROLLABLE_LABEL_H
