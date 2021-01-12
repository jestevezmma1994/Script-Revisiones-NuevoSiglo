/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include <Logic/NPvrLogic.h>

#include <Widgets/npvrDialog.h>
#include <NPvr/QBnPVRProvider.h>
#include <Services/npvrAgent.h>
#include <SvEPGDataLayer/Plugins/Innov8onEPGEvent.h>
#include <SvFoundation/SvArray.h>
#include <main.h>

bool
QBNPvrLogicAreOngoingRecsAllowed(void)
{
    return true;
}

bool
QBNPvrLogicAreRecordingsCustomizable(AppGlobals appGlobals)
{
    return true;
}

bool
QBNPvrLogicShouldNPVRBeUsedForStartOver(void)
{
    return true;
}

bool
QBNPvrLogicIsPremiereFeatureEnabled(void)
{
    return true;
}

bool
QBNPvrLogicIsSetMarginsFeatureEnabled(void)
{
    return false;
}

bool
QBNPvrLogicIsRecoveryFeatureEnabled(void)
{
    return true;
}

SvString
QBNPvrLogicCreateCustomQuotaFormat(uint64_t usedQuota, uint64_t totalQuota, QBnPVRQuotaUnits quotaUnits)
{
    return SVSTRING("");
}

static QBnPVRRecordingType recTypesByPriority[] = {
    QBnPVRRecordingType_none,
    QBnPVRRecordingType_keyword,
    QBnPVRRecordingType_series,
    QBnPVRRecordingType_event,
    QBnPVRRecordingType_OTR,
    QBnPVRRecordingType_manual
};

static size_t recTypesByPrioritySize = sizeof(recTypesByPriority) / sizeof(recTypesByPriority[0]);

SvLocal inline int
QBNPvrLogicFindRecTypeInPrioritiesTable(QBnPVRRecordingType recType)
{
    for (size_t x = 0; x < recTypesByPrioritySize; ++x) {
        if (recType == recTypesByPriority[x]) {
            return x;
        }
    }

    return 0;
}

QBnPVRRecordingType
QBNPvrLogicGetRecordingTypeForEvent(SvObject nPVRProvider, SvEPGEvent event)
{
    QBnPVRRecordingType recType = QBnPVRRecordingType_none;

    SvArray recordings = SvArrayCreate(NULL);
    if (!nPVRProvider) {
        SvLogError("%s(): NULL nPVRProvider argument passed.", __func__);
        goto fini;
    }
    SvInvokeInterface(QBnPVRProvider, nPVRProvider, listRecordingsByEvent, event, recordings);

    if (SvArrayCount(recordings) == 0) {
        goto fini;
    }

    if (SvArrayCount(recordings) == 1) {
        QBnPVRRecording rec = (QBnPVRRecording) SvArrayAt(recordings, 0);
        recType = rec->type;
        goto fini;
    }

    int recTypePriority = QBNPvrLogicFindRecTypeInPrioritiesTable(recType);

    QBnPVRRecording npvrRec;
    SvIterator it = SvArrayIterator(recordings);
    while ((npvrRec = (QBnPVRRecording) SvIteratorGetNext(&it))) {
        int nextRecTypePriority = QBNPvrLogicFindRecTypeInPrioritiesTable(npvrRec->type);
        if (nextRecTypePriority > recTypePriority) {
            recTypePriority = nextRecTypePriority;
            recType = npvrRec->type;
        }
    }

fini:
    SVRELEASE(recordings);
    return recType;
}

QBnPVRRecordingType QBNPvrLogicTryScheduleRecordingForEvent(AppGlobals appGlobals, SvEPGEvent event, bool *isScheduled)
{
    bool scheduled = false;
    QBnPVRRecordingType scheduledRecordingType = QBnPVRRecordingType_none;
    if (event && appGlobals->nPVRProvider) {
        QBnPVRRecordingType nPVRRecordingType = QBNPvrLogicGetRecordingTypeForEvent(appGlobals->nPVRProvider, event);

        if (nPVRRecordingType == QBnPVRRecordingType_none) {
            const time_t now = SvTimeNow();
            const bool ended = (now > event->endTime);
            bool canBeRecorded = SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, canScheduleEventRecording, event);

            if (!ended && canBeRecorded) {
                struct QBnPVRRecordingSchedParams_ params = {
                    .type  = QBnPVRRecordingType_manual,
                    .event = event,
                };
                if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
                    params.recoveryOption = SvValueCreateWithInteger(QBnPVRRecordingSpaceRecovery_Manual, NULL);
                }

                QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, scheduleRecording, &params);
                QBNPvrAgentTrackRequest(appGlobals->npvrAgent, request, event);
                QBNPvrDialogCreateAndShow(appGlobals, request);
                SVTESTRELEASE(params.recoveryOption);
                scheduled = true;
                scheduledRecordingType = QBnPVRRecordingType_manual;
                goto fini;
            }
        }
    }

fini:
    if (isScheduled) {
        *isScheduled = scheduled;
    }
    return scheduledRecordingType;
}
