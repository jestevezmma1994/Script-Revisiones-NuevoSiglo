/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_TIME_AXIS_H_
#define SV_EPG_TIME_AXIS_H_

/**
 * @file SvEPGTimeAxis.h EPG time axis widget API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <time.h>
#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>
#include <SvPlayerKit/SvTimeRange.h>


/**
 * @defgroup SvEPGTimeAxis EPG time axis widget
 * @ingroup SvEPG
 * @{
 **/

/**
 * Create new EPG time axis widget.
 *
 * This method creates time axis widget.
 * Appearance of this widget is controlled by the Settings Manager.
 * It will use following parameters:
 *
 *  - height : integer, required,
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required,
 *  - tickMark : bitmap, optional,
 *  - timeMark : bitmap, optional,
 *  - twelveHourFormat : boolean, optional (default: false).
 *
 * @param[in] app          CUIT application context
 * @param[in] widgetName   widget name
 * @param[in] timeScale    number of pixels per one hour
 * @param[out] errorOut    error info
 * @return                 SvEPGTimeAxis widget, NULL in case of error
 **/
extern SvWidget
SvEPGTimeAxisNew(SvApplication app,
                 const char *widgetName,
                 unsigned int timeScale,
                 SvErrorInfo *errorOut);

/**
 * Set time range to be shown.
 *
 * @param[in] axis         EPG time axis widget
 * @param[in] range        time range to be shown
 * @param[out] errorOut    error info
 **/
extern void
SvEPGTimeAxisSetTimeRange(SvWidget axis,
                          SvTimeRange range);

/**
 * Update current time marker.
 *
 * @param[in] axis         EPG time axis widget
 * @param[in] now          current time
 * @return                 @c true if widgets has changed and needs redraw,
 *                         @c false otherwise
 **/
extern bool
SvEPGTimeAxisSetCurrentTime(SvWidget axis,
                            time_t now);

/**
 * Hide time labels from outside the given view port.
 *
 * @param[in] axis           EPG time axis widget
 * @param[in] viewPortWidth  Width of view port
 **/
extern void
SvEPGTimeAxisHideOutsideTimeLabels(SvWidget axis,
                                   int viewPortWidth);

/**
 * @}
 **/


#endif
