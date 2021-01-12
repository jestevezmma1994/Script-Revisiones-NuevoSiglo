/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SWL_SCROLLBAR_H_
#define SWL_SCROLLBAR_H_

/**
 * @file SMP/SWL/swl/scrollbar.h Scrollbar widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>


/**
 * @defgroup SvScrollBar Scroll bar widget
 * @ingroup SWL_widgets
 * @{
 *
 * Scroll bar widget displays the position of window.
 **/


/**
 * Create scroll bar widget using settings from the Settings Manager.
 *
 * This function creates scroll bar widget. Its appearance is controlled by
 * the Settings Manager. It uses following parameters:
 *
 *  - orientation: string, required, {horizontal, vertical},
 *  - hasArrows, hasBar: boolean, optional (default values: true),
 *  - width, height : integer, required,
 *  - duration: double, required,
 *  - startArrow, endArrow: bitmap, required if hasArrows==true,
 *  - inactiveStartArrow, inactiveEndArrow: bitmap, optional,
 *  - bar, slider: bitmap, required if hasBar==true.
 *  - minSliderSize: integer, optional (default 0)
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    widget name
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
svScrollBarNew(SvApplication app, const char *widgetName);

/**
 * Get the range listener used for communicating with the scroll bar.
 *
 * @param[in] w             scroll bar widget handle
 * @return                  range listener handler
 **/
extern SvObject
svScrollBarGetRangeListener(SvWidget w);

/**
 * @}
 **/

#endif
