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

#include <Logic/timeFormat.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>

static const char timeWithoutDate[] = "%k:%M";

static struct QBTimeFormat_t defaultFormat = {       //RAL: Modified display time format
    .PVRStartTime          = "%A - %d/%m/%Y, %H:%M", //.PVRStartTime = "%A / %d.%m.%Y, %H:%M", //RAL
    .PVRAvailableTime      = "%A - %d/%m/%Y",        //.PVRAvailableTime = "%A / %d.%m.%Y", // AMERELES
    .reminderTime          = "%A - %d/%m/%Y, %H:%M", //.reminderTime = "%A / %d.%m.%Y, %H:%M", //RAL

    .gridEPGDate           = "%A - %d/%m/%Y",        //.gridEPGDate = "%A / %d.%m.%Y", //RAL
    .verticalEPGDate       = "%A - %d/%m/%Y",        //.verticalEPGDate = "%A / %d.%m.%Y", //RAL
    .searchResultsDate     = "%d/%m/%Y",             //.searchResultsDate = "%d.%m.%Y", //RAL
    .recordingEditPaneDate = "%A / %b %d, %Y",
    .timeEditPaneDate      = "%A / %b %d, %Y",
    .traxisEntitlementDate = NULL,
    .verticalEPGTime       = timeWithoutDate,
    .gridEPGTime           = timeWithoutDate,
    .extendedInfoTime      = timeWithoutDate,
    .tvOsdTime             = timeWithoutDate,
    .searchResultsTime     = timeWithoutDate,
    .recordingEditPaneTime = timeWithoutDate,
    .timeEditPaneTime      = timeWithoutDate,
    .timeEditBox           = timeWithoutDate,
    .channelBarDetails     = "%k:%M",
    .PVRConflicts          = timeWithoutDate,
    .manualRecordingName   = timeWithoutDate,

    .callTime              = timeWithoutDate,
    .callDate              = "%A / %b %d, %Y"
};

QBTimeFormat
QBTimeFormatGetCurrent(void)
{
    if (!defaultFormat.frontPanelClockTime) {
        const char* format = "%H%M";
        QBFrontPanelCapabilities caps = {
            .visibleCharacters = 0,
        };
        int ret = QBPlatformGetFrontPanelCapabilities(&caps);
        if (!ret) {
            if (caps.visibleCharacters >= 8) {
                format = "%I:%M %p";
            } else if (caps.visibleCharacters >= 5) {
                format = "%H:%M";
            }
        }
        defaultFormat.frontPanelClockTime = format;
    }

    return &defaultFormat;
}

SvString
QBTimeFormatConvertConaxMenuDate(SvString date)
{
    return SVRETAIN(date);
}