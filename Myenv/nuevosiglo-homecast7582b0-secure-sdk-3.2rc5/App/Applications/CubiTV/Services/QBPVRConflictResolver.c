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

#include <QBSecureLogManager.h>
#include <settings.h>
#include <ContextMenus/QBBasicPane.h>
#include <Logic/timeFormat.h>
#include <main.h>
#include <Utils/recordingUtils.h>
#include <QBWidgets/QBDialog.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <QBPVRProvider.h>
#include <QBPVRProviderRequest.h>
#include <QBPVRRecording.h>
#include <CUIT/Core/types.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <libintl.h>

#include "QBPVRConflictResolver.h"

#define log_debug(fmt, ...)  do { if(1) SvLogNotice(COLBEG() "%s:%d " fmt  COLEND_COL(blue), __func__,__LINE__,##__VA_ARGS__); } while (0)

#define MINIMAL_RECORDING_LENGTH_TO_SHORTEN 60

struct QBPVRConflictResolver_ {
    struct SvObject_ super_;

    QBPVRRecordingSchedParams params;
    AppGlobals appGlobals;
    SvEPGManager epgManager;
    QBContextMenu ctxMenu;
    SvArray ignored;

    int level;
    bool firstQuestion;

    SvWidget popupDialog;

    struct {
        QBPVRConflictResolverFinishCallback fun;
        void *ptr;
    } finishCallback;
    struct {
        QBPVRConflictResolverQuestionCallback fun;
        void *ptr;
    } questionCallback;
};

SvLocal void
QBPVRConflictResolver__dtor__(void *self_)
{
    log_debug();
    QBPVRConflictResolver self = (QBPVRConflictResolver) self_;
    SVTESTRELEASE(self->epgManager);
    SVRELEASE(self->params);
    SVRELEASE(self->ctxMenu);
    SVRELEASE(self->ignored);
}

SvLocal SvType
QBPVRConflictResolver_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVRConflictResolver__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRConflictResolver",
                            sizeof(struct QBPVRConflictResolver_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBPVRConflictResolverRemoveIgnored(QBPVRConflictResolver self)
{
    QBPVRRecording rec = NULL;
    SvIterator it = SvArrayIterator(self->ignored);
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        if (rec->directory)
            continue;

        if (rec->state == QBPVRRecordingState_active)
            QBPVRProviderStopRecording(self->appGlobals->pvrProvider, rec);
        else
            QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, rec);

        SvTimeRange timeRange;
        SvTimeRangeInit(&timeRange, SvTimeGetSeconds(rec->startTime), SvTimeGetSeconds(rec->endTime));
        SvValue channelId = SvValueCreateWithString(rec->channelId, NULL);
        SvEPGManagerPropagateEventsChange(self->epgManager, channelId, &timeRange, NULL);
        SVRELEASE(channelId);
    }
}

SvLocal void
QBPVRConflictResolverContextShorten(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRConflictResolver self = self_;

    QBPVRConflictResolverRemoveIgnored(self);
    SvValue shortenTimeVal = (SvValue) SvHashTableFind(item->metadata, (SvObject) SVSTRING("shortenTime"));
    self->params->duration = SvValueGetInteger(shortenTimeVal);
    QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, self->params);
    QBContextMenuPopPane(self->ctxMenu);
    if(self->finishCallback.fun)
        self->finishCallback.fun(self->finishCallback.ptr, true);
}

SvLocal void
QBPVRConflictResolverContextIgnore(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRConflictResolver self = self_;

    if (id && SvStringEqualToCString(id, "cancel")) {
        QBContextMenuPopPane(self->ctxMenu);
        if (self->finishCallback.fun)
            self->finishCallback.fun(self->finishCallback.ptr, false);
    } else {
        QBPVRRecording rec = (QBPVRRecording) SvHashTableFind(item->metadata, (SvObject) SVSTRING("rec"));
        SvArrayAddObject(self->ignored, (SvObject) rec);
        QBPVRConflictResolverResolve(self);
    }
}

SvLocal SvWidget QBPVRConflictResolverPopupCreate(QBPVRConflictResolver self, const char* title, const char* message)
{
    svSettingsPushComponent("ConflictResolverPane.settings");
    QBDialogParameters params = {
        .app        = self->appGlobals->res,
        .controller = self->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogSetTitle(dialog, title);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogAddLabel(dialog, SVSTRING("content"), message, SVSTRING("message"), 0);
    QBDialogAddButton(dialog, SVSTRING("button"), gettext("OK"), 1);
    svSettingsPopComponent();
    return dialog;
}

SvLocal void QBPVRConflictResolverPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBPVRConflictResolver self = (QBPVRConflictResolver) self_;
    self->popupDialog = NULL;
}

void
QBPVRConflictResolverResolve(QBPVRConflictResolver self)
{
    SvIterator it = SvArrayIterator(self->ignored);
    QBPVRRecording rec = NULL;
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        SvString name = QBRecordingUtilsCreateName(self->appGlobals, rec, true, false);
        log_debug("ignored: %s %p", SvStringCString(name), rec);
        SVRELEASE(name);
    }

    SvString errMsg = NULL;
    SvArray conflicts = NULL;
    int timeWithoutConflicts;
    QBPVRProviderConflictReason conflictReason;

    if (QBPVRProviderCheckRecordingConflicts(self->appGlobals->pvrProvider,
                                             self->params,
                                             self->ignored,
                                             &conflicts,
                                             &timeWithoutConflicts,
                                             &conflictReason) == 0) {
        if(!self->firstQuestion) {
            QBContextMenuPopPane(self->ctxMenu);
        }
        //resolved
        log_debug("No conflicts");

        QBPVRConflictResolverRemoveIgnored(self);

        QBPVRProviderRequest request = QBPVRProviderScheduleRecording(self->appGlobals->pvrProvider, self->params);

        if (QBPVRProviderDiskRequestGetStatus(request) == QBPVRProviderDiskRequestStatus_success) {
            if (self->params->event) {
                SvTimeRange timeRange;
                SvTimeRangeInit(&timeRange, self->params->event->startTime, self->params->event->endTime);
                SvEPGManagerPropagateEventsChange(self->epgManager, self->params->event->channelID, &timeRange, NULL);
            }

            if(self->finishCallback.fun)
                self->finishCallback.fun(self->finishCallback.ptr, true);

            return;
        } else {
            errMsg = SvStringCreateWithFormat("%s", gettext("Recording schedule failed."));
        }
    }

    if (!errMsg && conflictReason == QBPVRProviderConflictReason_unknown) {
        errMsg = SvStringCreateWithFormat("%s", gettext("Unknown conflict found."));
    }

    if (!errMsg && conflictReason == QBPVRProviderConflictReason_driveTooSlow) {
        log_debug("Couldn't add record cause of weak drive");
        const char* message = gettext("Your drive is too slow to handle this recording.");
        self->popupDialog = QBPVRConflictResolverPopupCreate(self, gettext("A problem occurred"), message);
        QBDialogRun(self->popupDialog, self, QBPVRConflictResolverPopupCallback);
        QBSecureLogEvent("PVRConflictResolver", "ErrorShown.PVR.DriveTooSlow", "JSON:{\"description\":\"%s\"}", message);
        if (!self->firstQuestion) {
            QBContextMenuPopPane(self->ctxMenu);
        }
        SVTESTRELEASE(conflicts);
        if (self->finishCallback.fun)
            self->finishCallback.fun(self->finishCallback.ptr, false);
        return;
    }

    if (!errMsg && conflictReason == QBPVRProviderConflictReason_tooManyRecordings) {
        errMsg = SvStringCreateWithFormat("%s", gettext("You have too many recordings."));
    }

    if (!errMsg && conflictReason == QBPVRProviderConflictReason_tooManySimultaneousRecordings && !conflicts) {
        errMsg = SvStringCreateWithFormat("%s", gettext("Too many simultaneous recordings."));
    }

    if (!errMsg && conflictReason == QBPVRProviderConflictReason_lackOfSpace) {
        errMsg = SvStringCreateWithFormat("%s", gettext("You don't have enough free space to make a new recording."));
    }

    if (errMsg) {
        svSettingsPushComponent("ConflictResolverPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->ctxMenu, self->level, SVSTRING("BasicPane"));
        QBBasicPaneSetTitle(options, errMsg);
        SVRELEASE(errMsg);

        SvString option = SvStringCreate(gettext("Cancel"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBPVRConflictResolverContextIgnore, self);
        SVRELEASE(option);

        if(self->firstQuestion) {
            if(self->questionCallback.fun)
                self->questionCallback.fun(self->questionCallback.ptr);
            self->firstQuestion = false;
            QBContextMenuPushPane(self->ctxMenu, (SvGenericObject)options);
        } else {
            QBContextMenuSwitchPane(self->ctxMenu, (SvGenericObject)options);
        }
        SVRELEASE(options);
        SVTESTRELEASE(conflicts);
        svSettingsPopComponent();
        return;
    }

    /// conflicts
    if (!conflicts) {
        SvLogError("%s(): NULL conflicts array but returned conflicts reason was conflicts", __func__);
        assert(conflicts);
    }
    it = SvArrayIterator(conflicts);
    log_debug("");

    bool automatic = false;
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        SvString name = QBRecordingUtilsCreateName(self->appGlobals, rec, true, false);
        log_debug("Conflict %s", SvStringCString(name));

        if (rec->directory) {
            log_debug("Automatic resolve: %s", SvStringCString(name));
            automatic = true;
            SvArrayAddObject(self->ignored, (SvObject) rec);
        }
        SVRELEASE(name);
    }

    if (automatic) {
        SVRELEASE(conflicts);
        QBPVRConflictResolverResolve(self);
        return;
    }

    svSettingsPushComponent("ConflictResolverPane.settings");
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->ctxMenu, self->level, SVSTRING("BasicPane"));
    SvString title = SvStringCreate(gettext("In order to record selected program you need to stop one of the following recordings/schedules"), NULL);
    QBBasicPaneSetTitle(options, title);
    SVRELEASE(title);

    it = SvArrayIterator(conflicts);
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        SvString name = QBRecordingUtilsCreateName(self->appGlobals, rec, true, false);
        SvString subcaption = QBRecordingUtilsCreateTimeInformation(self->appGlobals->pvrProvider, rec);
        log_debug("Add option: %s", SvStringCString(name));
        QBBasicPaneItem item = QBBasicPaneAddOptionWithSubcaption(options, SVSTRING("ignore"), name, subcaption, QBPVRConflictResolverContextIgnore, self);
        SvHashTableInsert(item->metadata, (SvGenericObject) SVSTRING("rec"), (SvObject) rec);
        SVRELEASE(name);
        SVRELEASE(subcaption);
    }

    if (self->params->type == QBPVRRecordingType_OTR && self->params->duration && timeWithoutConflicts > MINIMAL_RECORDING_LENGTH_TO_SHORTEN) {
#define MAX_DATE_LENGTH 128
        char timeBuf[MAX_DATE_LENGTH];
        snprintf(timeBuf, MAX_DATE_LENGTH, "%ih %02im", timeWithoutConflicts/3600, (timeWithoutConflicts/60)%60);
        SvString option = SvStringCreateWithFormat(gettext("Shorten recording to %s"), timeBuf);
        QBBasicPaneItem item = QBBasicPaneAddOption(options, SVSTRING("shorten"), option, QBPVRConflictResolverContextShorten, self);
        SvValue shortenTimeVal = SvValueCreateWithInteger(timeWithoutConflicts, NULL);
        SvHashTableInsert(item->metadata, (SvObject) SVSTRING("shortenTime"), (SvObject) shortenTimeVal);
        SVRELEASE(shortenTimeVal);
        SVRELEASE(option);
    }

    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBPVRConflictResolverContextIgnore, self);
    SVRELEASE(option);

    if(self->firstQuestion) {
        if(self->questionCallback.fun)
            self->questionCallback.fun(self->questionCallback.ptr);
        self->firstQuestion = false;
        QBContextMenuPushPane(self->ctxMenu, (SvGenericObject)options);
    } else {
        QBContextMenuSwitchPane(self->ctxMenu, (SvGenericObject)options);
    }
    SVRELEASE(options);

    svSettingsPopComponent();
    SVRELEASE(conflicts);
}

void
QBPVRConflictResolverSetFinishCallback(QBPVRConflictResolver self, QBPVRConflictResolverFinishCallback callback, void *ptr)
{
    self->finishCallback.fun = callback;
    self->finishCallback.ptr = ptr;
}

void
QBPVRConflictResolverSetQuestionCallback(QBPVRConflictResolver self, QBPVRConflictResolverQuestionCallback callback, void *ptr)
{
    self->questionCallback.fun = callback;
    self->questionCallback.ptr = ptr;
}


QBPVRConflictResolver
QBPVRConflictResolverCreate(AppGlobals appGlobals, QBContextMenu ctxMenu, QBPVRRecordingSchedParams params, int level)
{
    QBPVRConflictResolver self = (QBPVRConflictResolver) SvTypeAllocateInstance(QBPVRConflictResolver_getType(), NULL);

    self->appGlobals = appGlobals;
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    self->ctxMenu = SVRETAIN(ctxMenu);
    self->ignored = SvArrayCreate(NULL);
    self->level = level;
    self->firstQuestion = true;

    self->params = SVRETAIN(params);

    return self;
}
