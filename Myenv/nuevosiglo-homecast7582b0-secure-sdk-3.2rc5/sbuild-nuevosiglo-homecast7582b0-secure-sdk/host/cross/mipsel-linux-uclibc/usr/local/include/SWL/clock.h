/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2007 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SWL_CLOCK_H_
#define SWL_CLOCK_H_

/**
 * @file SMP/SWL/swl/clock.h Clock widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <CUIT/Core/types.h>

/**
 * @defgroup SvClock Clock widget
 * @ingroup SWL_widgets
 * @{
 *
 * Clock widget displays current date and/or time.
 **/

/**
 * Create clock widget.
 *
 * Appereance of the clock widget is controlled by the Settings Manager.
 * It uses the following parameters:
 *
 * - all parameters used by the @ref SvLabel (except the 'text' parameter),
 * - timeFormat - string (used by strftime()), optional (default: "%T"),
 * - icon - bitmap, optional (default: no icon),
 * - spacing - integer, optional (default: 0).
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    name of the widget used with Settings Manager
 * @returns                 newly created widget or @c NULL in case of error
 */
extern SvWidget
svClockNew(SvApplication app, const char* widgetName);

/**
 * Create clock widget using ratio settings.
 *
 * @param[in] app           CUIT application handle
 * @param[in] widgetName    name of the widget used with Settings Manager
 * @param[in] parentWidth   parent width
 * @param[in] parentHeight  parent height
 * @returns                 newly created widget or @c NULL in case of error
 */
extern SvWidget
svClockNewFromRatio(SvApplication app, const char* widgetName, int parentWidth, int parentHeight);

/**
 * @}
 **/

#endif
