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

#ifndef TIMEFORMAT_H_
#define TIMEFORMAT_H_

#include <SvFoundation/SvString.h>

struct QBTimeFormat_t {
    const char *PVRStartTime;
    const char *PVRAvailableTime;
    const char *reminderTime;

    const char *callDate;
    const char *callTime;

    const char *gridEPGDate;
    const char *verticalEPGDate;
    const char *searchResultsDate;
    const char *recordingEditPaneDate;
    const char *traxisEntitlementDate;
    const char *timeEditPaneDate;

    const char *verticalEPGTime;
    const char *gridEPGTime;
    const char *extendedInfoTime;
    const char *tvOsdTime;
    const char *searchResultsTime;
    const char *recordingEditPaneTime;
    const char *timeEditPaneTime;
    const char *frontPanelClockTime;
    const char *timeEditBox;
    const char *channelBarDetails;
    const char *PVRConflicts;

    const char *manualRecordingName;
};

typedef struct QBTimeFormat_t *QBTimeFormat;

QBTimeFormat
QBTimeFormatGetCurrent(void) __attribute__((weak));

SvString
QBTimeFormatConvertConaxMenuDate(SvString date);

#endif /* TIMEFORMAT_H_ */
