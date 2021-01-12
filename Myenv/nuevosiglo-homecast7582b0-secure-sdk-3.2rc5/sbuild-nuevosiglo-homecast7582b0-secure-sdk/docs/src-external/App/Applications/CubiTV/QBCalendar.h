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

/*
 * QBCalendar.h
 *
 *  Created on: Feb 7, 2011
 *      Author: Rafał Duszyński
 */

#ifndef QBCALENDAR_H_
#define QBCALENDAR_H_

/**
 * @file QBCalendar.h Calendar widget interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <time.h>

#include <SvCore/SvErrorInfo.h>
#include <SvCore/QBDate.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/QBUIEvent.h>
#include <QBOSK/QBOSKRenderer.h>
#include <Services/core/QBTextRenderer.h>

/**
 * @defgroup QBCalendar Calendar widget
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Calendar date selected event class.
 * @class QBCalendarDateSelectedEvent
 * @extends QBUIEvent
 **/
typedef struct QBCalendarDateSelectedEvent_ *QBCalendarDateSelectedEvent;

/**
 * QBCalendarDateSelectedEvent class internals.
 **/
struct QBCalendarDateSelectedEvent_ {
    /// super class
    struct QBUIEvent_ super;
    /// selected date
    QBDate selectedDate;
};

/**
 * Get runtime type identification object representing QBCalendarDateSelectedEvent class.
 *
 * @return calendar date selected event class
 **/
extern SvType
QBCalendarDateSelectedEvent_getType(void);


/**
 * Create new Calendar widget.
 *
 * @param[in] app           CUIT application handle
 * @param[in] name          widget name
 * @param[in] txtRenderer   renderer for labels
 * @param[out] errorOut     error info
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBCalendarNew(SvApplication app, const char *name, QBTextRenderer txtRenderer, SvErrorInfo *errorOut);

/**
 * Set current date for Calendar (starts rendering of widgets).
 *
 * @param[in] Calendar      Calendar widget handle
 * @param[in] CurrentDate   Date to be displayed on Calendar (YY/MM/dd only),
 *                          if tm_mday is specified focus is set to this day,
 *                          else it is set to "today" or first day of the month
 * @param[in] renderer      OSK renderer instance
 * @param[out] errorOut     error info
 **/
extern void
QBCalendarSetCurrentDate(SvWidget Calendar, struct tm *CurrentDate, QBOSKRenderer renderer, SvErrorInfo *errorOut);

/**
 * Set Calendar limits. Use before setting current date.
 *
 * @param[in] Calendar      Calendar widget handle
 * @param[in] MinDate       Minimal date allowed for selection - inclusive (YY/MM/dd only)
 * @param[in] MaxDate       Maximal date allowed for selection - inclusive (YY/MM/dd only)
 **/
extern void
QBCalendarSetLimitDates(SvWidget Calendar, struct tm *MinDate, struct tm *MaxDate);

/**
 * @}
 **/

#endif /* QBCALENDAR_H_ */
