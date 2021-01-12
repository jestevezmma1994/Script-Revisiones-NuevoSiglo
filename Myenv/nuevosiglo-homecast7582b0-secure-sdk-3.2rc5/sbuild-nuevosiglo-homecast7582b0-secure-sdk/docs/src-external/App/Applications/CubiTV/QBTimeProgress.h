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

#ifndef QBTIMEPROGRESS_H_
#define QBTIMEPROGRESS_H_

#include <CUIT/Core/types.h>
#include <Services/core/QBTextRenderer.h>

/**
 * @file QBTimeProgress.h CubiTV Time Progres Widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBTimeProgress CubiTV Time Progress widget
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Create Time Progress Widget.
 *
 * This is a widget that displays a guage representing time (ie. event duration),
 * a marker on top of the guage showing current time, two labels on the beggining
 * and end of the guage, and a label next to the marker. This widget is also capable
 * of displaying another guage showing part of the content that is curently available
 * (ie. for ongoing nPVR recording the part from beginning of the event to live point).
 *
 * Mind that the start time of this widget is always 00:00:00 so all times passed to
 * this widget should be normalized (ie. The end time of the event should be the time
 * measured from the beggining of the event rather than the time from 01.01.1970)
 *
 * @param[in] app       global application data handle
 * @param[in] name      name of the widget in settings
 * @param[in] renderer  text renderer for labels
 * @return              Newly created time progress widget
 **/
SvWidget
QBTimeProgressCreate(SvApplication app, const char *name, QBTextRenderer renderer);

/**
 * Set marker of the widget to show desired time.
 *
 * @param[in] w     QBTimeProgress widget handle
 * @param[in] time  time (normalized) to set the marker to
 **/
void
QBTimeProgressSetCurrentTime(SvWidget w, double time);

/**
 * Set the widget time range end
 *
 * @param[in] w     QBTimeProgress widget handle
 * @param[in] time  time range end (normalized)
 **/
void
QBTimeProgressSetMaxTime(SvWidget w, double time);

/**
 * Shows or hides the widget
 *
 * @param[in] w         QBTimeProgress widget handle
 * @param[in] enabled   true to show the widget and false to hide it
 **/
void
QBTimeProgressSetEnabled(SvWidget w, bool enabled);

/**
 * Set the range of the available content (different color guage over the time guage).
 *
 * The available content guage will span from start to end. In order to
 * show the content guage use the QBTimeProgressShowAvailableContent function
 *
 * @param[in] w     QBTimeProgress widget handle
 * @param[in] start start time of available content (normalized)
 * @param[in] end   end time of available content (normalized)
 **/
void
QBTimeProgressSetAvailableContent(SvWidget w, time_t start, time_t end);

/**
 * Show the avialable content guage (will be shown only if the whole widget is shown)
 *
 * @param[in] w     QBTimeProgress widget handle
 **/
void
QBTimeProgressShowAvailableContent(SvWidget w);

/**
 * Hide the avialable content guage (can be hidden even if the widget is visible)
 *
 * @param[in] w     QBTimeProgress widget handle
 **/
void
QBTimeProgressHideAvailableContent(SvWidget w);

/**
 * @}
 **/

#endif /* QBTIMEPROGRESS_H_ */
