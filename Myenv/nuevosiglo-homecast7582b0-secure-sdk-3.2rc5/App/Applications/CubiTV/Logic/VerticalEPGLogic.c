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

#include <Logic/VerticalEPGLogic.h>

#include <main.h>
#include <QBInput/QBInputCodes.h>
#include <QBPVRProvider.h>
#include <Windows/verticalEPG.h>
#include <Logic/NPvrLogic.h>

struct QBVerticalEPGLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    bool extendedInfo;
};

SvLocal SvType QBVerticalEPGLogic_getType(void)
{
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBVerticalEPGLogic",
                            sizeof(struct QBVerticalEPGLogic_t),
                            SvObject_getType(), &type, NULL);
    }
    return type;
}

SvLocal bool QBVerticalEPGLogicIsRecordShown(QBVerticalEPGLogic self,
                                             QBWindowContext self_)
{
    if (!self->appGlobals->pvrProvider) {
        return false;
    }

    SvEPGEvent event = NULL;
    QBVerticalEPGGetActiveEvent(self_, &event);

    if (!event) {
        return false;
    }

    QBPVRRecording ret = NULL;
    QBPVRProviderGetRecordingsParams params = {
        .playable = false,
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

QBVerticalEPGLogic QBVerticalEPGLogicNew(AppGlobals appGlobals)
{
    QBVerticalEPGLogic self = (QBVerticalEPGLogic) SvTypeAllocateInstance(QBVerticalEPGLogic_getType(), NULL);
    self->appGlobals = appGlobals;

    return self;
}

SvLocal bool QBVerticalEPGLogicTryScheduleNPvrRecordingForEvent(QBVerticalEPGLogic self)
{
    QBWindowContext epg = self->appGlobals->verticalEPG;
    SvEPGEvent event = NULL;
    bool isSideMenuShown = false;
    bool isScheduled = false;

    if (self->appGlobals->nPVRProvider == NULL) {
        goto fini;
    }
    QBnPVRRecordingType scheduledRecordingType = QBnPVRRecordingType_none;
    QBVerticalEPGGetActiveEvent(epg, &event);
    if (event) {
        scheduledRecordingType = QBNPvrLogicTryScheduleRecordingForEvent(self->appGlobals, event, &isScheduled);
        if ((scheduledRecordingType != QBnPVRRecordingType_none) && !QBVerticalEPGIsSideMenuShown(epg)) {
            if (scheduledRecordingType == QBnPVRRecordingType_keyword) {
                isSideMenuShown = true;
                QBVerticalEPGShowSideMenu(epg, SVSTRING(""), QBVerticalEPGSideMenuType_NPvrKeyword);
            } else if (scheduledRecordingType == QBnPVRRecordingType_series) {
                isSideMenuShown = true;
                QBVerticalEPGShowSideMenu(epg, SVSTRING(""), QBVerticalEPGSideMenuType_NPvrSeries);
            }
        }
    }

fini:
    return isSideMenuShown || isScheduled;
}

bool QBVerticalEPGLogicInputHandler(QBVerticalEPGLogic self, const QBInputEvent* ev)
{
    QBWindowContext epg = self->appGlobals->verticalEPG;
    uint32_t keyCode = ev->u.key.code;

    bool isRecordOn = false;
    if (keyCode == QBKEY_REC) {
        isRecordOn = QBVerticalEPGLogicIsRecordShown(self, epg);
        if (isRecordOn) {
            return true;
        }
        QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
        if (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present) {
            QBVerticalEPGShowRecord(epg); // set to record
            return true;
        }
        bool isScheduled = QBVerticalEPGLogicTryScheduleNPvrRecordingForEvent(self);
        if (isScheduled) {
            return true;
        }
    } else if (keyCode == QBKEY_STOP) {
        if (!(isRecordOn = QBVerticalEPGLogicIsRecordShown(self, epg)))
            return true;
    } else if (keyCode == QBKEY_INFO) {
        if (!QBVerticalEPGIsSideMenuShown(epg)) {
            self->extendedInfo = true;
            QBVerticalEPGShowExtendedInfo(epg);
        } else {
            if (self->extendedInfo) {
                self->extendedInfo = false;
                QBVerticalEPGHideSideMenu(epg);
            }
        }
        return true;
    }

    if (keyCode == QBKEY_FUNCTION || keyCode == QBKEY_ENTER || isRecordOn) {
        if (QBVerticalEPGIsSideMenuShown(epg))
            QBVerticalEPGHideSideMenu(epg);
        else
            QBVerticalEPGShowSideMenu(epg, SVSTRING(""), QBVerticalEPGSideMenuType_Full);
        return true;
    }

    return false;
}

void QBVerticalEPGLogicPostWidgetsCreate(QBVerticalEPGLogic self)
{
}

void QBVerticalEPGLogicPreWidgetsDestroy(QBVerticalEPGLogic self)
{
}

