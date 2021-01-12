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

#include <Logic/SearchResultsLogic.h>

#include <main.h>
#include <CUIT/Core/event.h>
#include <QBPVRProvider.h>
#include <QBInput/QBInputCodes.h>
#include <Windows/searchResults.h>
#include <SvPlayerKit/SvEPGEvent.h>

struct QBSearchResultsLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    bool extendedInfo;
};

SvLocal SvType QBSearchResultsLogic_getType(void)
{
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSearchResultsLogic",
                            sizeof(struct QBSearchResultsLogic_t),
                            SvObject_getType(), &type, NULL);
    }
    return type;
}

QBSearchResultsLogic QBSearchResultsLogicNew(AppGlobals appGlobals)
{
    QBSearchResultsLogic self = (QBSearchResultsLogic) SvTypeAllocateInstance(QBSearchResultsLogic_getType(), NULL);
    self->appGlobals = appGlobals;
    return self;
}

SvLocal bool QBSearchResultsLogicIsRecordShown(QBSearchResultsLogic self,
                                               QBWindowContext self_)
{
    SvEPGEvent event = NULL;
    QBSearchResultsGetActiveEvent(self_, &event);

    if (!event)
        return false;

    QBPVRRecording ret = NULL;
    QBPVRProviderGetRecordingsParams params = {
        .includeRecordingsInDirectories = false,
    };
    params.inStates[QBPVRRecordingState_scheduled] = true;
    params.inStates[QBPVRRecordingState_active] = true;

    SvArray results = QBPVRProviderFindRecordingsInTimeRange(self->appGlobals->pvrProvider,
                                                             SvValueGetString(event->channelID),
                                                             SvTimeConstruct(event->startTime, 0),
                                                             SvTimeConstruct(event->endTime, 0),
                                                             params);
    if (results && SvArrayCount(results) > 0) {
        SvIterator it = SvArrayIterator(results);
        QBPVRRecording recording;
        while((recording = (QBPVRRecording) SvIteratorGetNext(&it))) {
            if (recording->event) {
                SvValue evId1 = SvEPGEventCreateID(event);
                SvValue evId2 = SvEPGEventCreateID(recording->event);
                if (!evId1 || !evId2) {
                    SVTESTRELEASE(evId1);
                    SVTESTRELEASE(evId2);
                    break;
                }
                bool equalEvents = SvObjectEquals((SvObject) evId1, (SvObject) evId2);
                SVRELEASE(evId1);
                SVRELEASE(evId2);

                if (equalEvents) {
                    ret = recording;
                    break;
                }
            } else if (SvObjectEquals((SvObject) recording->channelId, (SvObject) SvValueGetString(event->channelID)) &&
                       SvTimeCmp(SvTimeConstruct(event->startTime, 0), recording->endTime) < 0 &&
                       SvTimeCmp(recording->startTime, SvTimeConstruct(event->endTime, 0)) < 0 ) {
                ret = recording;
                break;
            }
        }
    }
    SVTESTRELEASE(results);
    return ret != NULL;
}

bool QBSearchResultsLogicHandleInputEvent(QBSearchResultsLogic self, QBWindowContext ctx, SvInputEvent ev)
{
    bool isRecordOn = false;
    if (ev->ch == QBKEY_REC) {
        QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
        if (diskState != QBPVRDiskState_pvr_present && diskState != QBPVRDiskState_pvr_ts_present)
            return true;

        isRecordOn = QBSearchResultsLogicIsRecordShown(self, ctx);
        if (!isRecordOn) {
            QBSearchResultsContextShowSideMenu(ctx, SVSTRING("record event")); // set to record
            return true;
        }
    } else if (ev->ch == QBKEY_STOP) {
        if (!(isRecordOn = QBSearchResultsLogicIsRecordShown(self, ctx)))
            return true;
    } else if (ev->ch == QBKEY_INFO) {
        if (!QBSearchResultsContextIsSideMenuShown(ctx)) {
            self->extendedInfo = true;
            QBSearchResultsContextShowExtendedInfo(ctx);
        } else {
            if (self->extendedInfo) {
                self->extendedInfo = false;
                QBSearchResultsContextHideSideMenu(ctx);
            }
        }
        return true;
    }

    if (ev->ch == QBKEY_FUNCTION || isRecordOn) {
        if (QBSearchResultsContextIsSideMenuShown(ctx))
            QBSearchResultsContextHideSideMenu(ctx);
        else
            QBSearchResultsContextShowSideMenu(ctx, SVSTRING(""));
        return true;
    }

    return false;
}

SvString QBSearchResultsLogicGetDefaultSideMenuItemId(QBSearchResultsLogic self)
{
    return SVSTRING("");
}
