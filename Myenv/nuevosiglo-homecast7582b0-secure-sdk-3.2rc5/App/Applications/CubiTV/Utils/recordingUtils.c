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

#include "recordingUtils.h"

#include <main.h>
#include <Services/pvrAgent.h>
#include <Utils/QBEventUtils.h>
#include <Logic/timeFormat.h>
#include <QBPVRProvider.h>
#include <QBPVRRecording.h>
#include <QBSearch/QBSearchUtils.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvTime.h>

#include <libintl.h>

SvLocal SvEPGEvent
QBRecordingUtilsGetEvent(const QBPVRRecording recording)
{
    if (!recording) {
        SvLogError("NULL argument passed");
        return NULL;
    }

    SvEPGEvent event = recording->event;
    if (event == NULL && recording->epgEvents) {
        if (SvArrayCount(recording->epgEvents) > 0) {
            event = (SvEPGEvent) SvArrayObjectAtIndex(recording->epgEvents, 0);
            SvIterator it = SvArrayIterator(recording->epgEvents);
            SvEPGEvent ev = NULL;
            while((ev = (SvEPGEvent) SvIteratorGetNext(&it))) {
                if (SvTimeCmp(SvTimeConstruct(ev->startTime, 0), recording->endTime) < 0 &&
                    SvTimeCmp(recording->startTime, SvTimeConstruct(ev->endTime, 0)) < 0) {
                    event = ev;
                    break;
                }
            }
        }
    }
    return event;
}

SvString
QBRecordingUtilsCreateName(AppGlobals appGlobals, const QBPVRRecording recording, bool printEvent, bool printStatus)
{
    const char *stateTag = "";

    if (printStatus) {
        switch (recording->state) {
            case QBPVRRecordingState_scheduled:
                stateTag = gettext("[scheduled]"); break;
            case QBPVRRecordingState_active:
                stateTag = gettext("[active]");    break;
            case QBPVRRecordingState_completed:
                stateTag = gettext("[completed]"); break;
            case QBPVRRecordingState_interrupted:
                stateTag = gettext("interrupted"); break;
            case QBPVRRecordingState_stoppedManually:
                stateTag = gettext("[[stopped manually]"); break;
            case QBPVRRecordingState_stoppedNoSpace:
                stateTag = gettext("[stopped – no space]"); break;
            case QBPVRRecordingState_stoppedNoSource:
                stateTag = gettext("[stopped – no source]"); break;
            case QBPVRRecordingState_failed:
                stateTag = gettext("[failed]");    break;
            case QBPVRRecordingState_missed:
                stateTag = gettext("[missed]");    break;
            case QBPVRRecordingState_removed:
                stateTag = gettext("[removed]");   break;
            default:
                stateTag = gettext("[unknown]");   break;
        }
    }

    SvString channelName = recording->channelName;
    SvString eventName = NULL;
    if (printEvent) {
        SvEPGEvent event = QBRecordingUtilsGetEvent(recording);
        if (event) {
            SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(appGlobals->eventsLogic, appGlobals->langPreferences, event);
            if (desc)
                eventName = SVTESTRETAIN(desc->title);
        }
    }

    SvString caption;
    if (eventName && channelName)
        caption = SvStringCreateWithFormat("%s - %s %s",
                      SvStringCString(channelName), SvStringCString(eventName), stateTag);
    else if (eventName)
        caption = SvStringCreateWithFormat("%s %s", SvStringCString(eventName), stateTag);
    else if (channelName)
        caption = SvStringCreateWithFormat("%s %s", SvStringCString(channelName), stateTag);
    else
        caption = SvStringCreateWithFormat("%s %s", "No name", stateTag);

    SVTESTRELEASE(eventName);
    return caption;
}

SvString
QBRecordingUtilsCreateTimeInformation(QBPVRProvider pvrProvider,
                                      QBPVRRecording recording)
{
    SvLocalTime startTime = SvLocalTimeFromUTC(SvTimeGetSeconds(recording->startTime));
    struct tm start = SvLocalTimeToLocal(startTime);

    char buffer[127];
    size_t total = 0;
    total += strftime(buffer+total, sizeof(buffer)-total, QBTimeFormatGetCurrent()->PVRStartTime, &start);

    if (recording->state != QBPVRRecordingState_scheduled &&
        recording->state != QBPVRRecordingState_missed &&
        recording->state != QBPVRRecordingState_removed) {
        int duration = QBPVRProviderGetRecordingCurrentDuration(pvrProvider, recording);
        int seconds = duration % 60;
        int minutes = (duration / 60) % 60;
        int hours   = duration / 3600;

        total += snprintf(buffer+total, sizeof(buffer)-total, " (%02d:%02d:%02d)", hours, minutes, seconds);
    }

    return SvStringCreate(buffer, NULL);
}

SvArray
QBRecordingUtilsCreateNotCompletedRecordingsForEvent(QBPVRProvider provider, SvEPGEvent event)
{
    if (!event)
        return NULL;

    QBPVRProviderGetRecordingsParams params = {
        .includeRecordingsInDirectories = true,
    };
    params.inStates[QBPVRRecordingState_active] = true;
    params.inStates[QBPVRRecordingState_scheduled] = true;

    return QBPVRProviderFindRecordingsInTimeRange(provider,
                                                  SvValueGetString(event->channelID),
                                                  SvTimeConstruct(event->startTime, 0),
                                                  SvTimeConstruct(event->endTime, 0),
                                                  params);
}


QBPVRRecording
QBRecordingUtilsGetRecordingOfType(SvArray rec, QBRecordingUtilsRecType recType)
{
    QBPVRRecording ret = NULL;
    if (rec && SvArrayCount(rec) > 0) {
        SvIterator it = SvArrayIterator(rec);
        QBPVRRecording recording;
        while((recording = (QBPVRRecording) SvIteratorGetNext(&it))) {
            switch (recType) {
                case QBRecordingUtilsRecType_Keyword:
                    if (recording->type == QBPVRRecordingType_keyword)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Repeat:
                    if (recording->type == QBPVRRecordingType_repeated)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Event:
                    if (recording->type == QBPVRRecordingType_event)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Channel:
                    if (recording->type == QBPVRRecordingType_OTR)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Manual:
                    if (recording->type == QBPVRRecordingType_manual)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Scheduled:
                    if (recording->state == QBPVRRecordingState_scheduled)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Active:
                    if (recording->state == QBPVRRecordingState_active)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Completed:
                    if (recording->state == QBPVRRecordingState_completed)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Missed:
                    if (recording->state == QBPVRRecordingState_missed)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Failed:
                    if (recording->state == QBPVRRecordingState_failed)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Removed:
                    if (recording->state == QBPVRRecordingState_removed)
                        return recording;
                    break;
                case QBRecordingUtilsRecType_Single:
                case QBRecordingUtilsRecType_Multiple: {
                    bool isMultiple = false;
                    if (recording->directory) {
                        QBPVRDirectory dir = (QBPVRDirectory) SvWeakReferenceTakeReferredObject(recording->directory);
                        if (dir->type == QBPVRDirectoryType_keyword ||
                            dir->type == QBPVRDirectoryType_series ||
                            dir->type == QBPVRDirectoryType_repeated) {
                            isMultiple = true;
                        }
                        SVRELEASE(dir);
                    }

                    if ((recType == QBRecordingUtilsRecType_Single && !isMultiple) ||
                        (recType == QBRecordingUtilsRecType_Multiple && isMultiple)) {
                        return recording;
                    }
                    break;
                }
                default:
                    return recording;
                    break;
            }
        }
    }
    return ret;
}

bool
QBRecordingUtilsStopRecordingForEvent(QBPVRProvider provider, SvEPGEvent event)
{
    SvArray recordings = QBRecordingUtilsCreateNotCompletedRecordingsForEvent(provider, event);
    //Only active/single recordings can be stopped
    QBPVRRecording recording = QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Active);
    if (!recording)
        recording = QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Single);
    SVRELEASE(recordings);

    if (!recording)
        return false;

    QBPVRProviderRequest request = NULL;
    if (recording->state == QBPVRRecordingState_active) {
        request = QBPVRProviderStopRecording(provider, recording);
    } else if (recording->state == QBPVRRecordingState_scheduled) {
        request = QBPVRProviderDeleteRecording(provider, recording);
    } else {
        return false;
    }

    if (request && QBPVRProviderDiskRequestGetStatus(request) == QBPVRProviderDiskRequestStatus_success)
        return true;

    return false;
}


bool
QBRecordingUtilsPVRSearchProviderTest(void *appGlobals_, SvObject recording_, SvArray keywords)
{
    AppGlobals appGlobals = (AppGlobals) appGlobals_;
    int r1 = 0, r2 = 0;

    QBPVRRecording recording = (QBPVRRecording) recording_;
    if (recording->event) {
        SvEPGEventDesc desc = QBEventUtilsGetSearchableDescFromEvent(appGlobals->eventsLogic, appGlobals->langPreferences, recording->event);
        if (desc) {
            r1 = QBSearchGetRelevance(desc->title, keywords);
            r2 = QBSearchGetRelevance(desc->description, keywords);
        }

#if 0
        /// similar mode
#define CHARS_IN_SIMILAR_MODE 4
        SvString title = QBUTF8CreateSubString((desc && desc->title ? desc->title : SVSTRING("")), 0, CHARS_IN_SIMILAR_MODE);
        SvString titleNormalized = QBSearchCreateNormalizedString(title, NULL);
        r1 = SvObjectEquals((SvObject) titleNormalized, (SvObject) truncatedKeyword);
        SVRELEASE(title);
        SVRELEASE(titleNormalized);
#endif

        if (r1 > 0 || r2 > 0) {
            return true;
        }
    }
    return false;
}
