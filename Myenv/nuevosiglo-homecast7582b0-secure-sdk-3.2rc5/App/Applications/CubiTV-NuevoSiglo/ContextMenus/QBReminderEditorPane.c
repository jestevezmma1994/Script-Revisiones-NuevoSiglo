/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBReminderEditorPane.h"
#include <ContextMenus/QBReminderEditorPaneInternal.h>

#include <libintl.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/label.h>
#include <error_codes.h>
#include <NPvr/QBnPVRProvider.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGDataPlugin.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <TraxisWebClient/TraxisEPGEvent.h>
#include <main.h>
#include <QBPlayerControllers/utils/content.h>
#include <Services/reminder.h>
#include <ContextMenus/QBBasicPane.h>
#include <Windows/newtv.h>
#include <QBSearch/QBSearchAgent.h>
#include <QBSearch/QBSearchEngine.h>
#include <QBOSK/QBOSKKey.h>
#include <Windows/searchResults.h>
#include <Utils/appType.h>
#include <QBConf.h>
#include <QBRecordFS/root.h>
#include <Services/QBPVRConflictResolver.h>
#include <Services/npvrAgent.h>
#include <QBPVRProvider.h>
#include <ContextMenus/QBRecordingEditPane.h>
#include <ContextMenus/QBSearchByCategoryPane.h>
#include <Utils/appType.h>
#include <Utils/QBNPvrUtils.h>
#include <stringUtils.h>
#include <Widgets/confirmationDialog.h>
#include <Widgets/npvrDialog.h>
#include <Widgets/authDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Utils/QBEventUtils.h>
#include <Utils/authenticators.h>
#include <Utils/QBContentInfo.h>
#include <Windows/pvrplayer.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <Logic/GUILogic.h>
#include <Logic/NPvrLogic.h>
#include <Utils/recordingUtils.h>
#include <TranslationMerger.h>
#include <SearchAgents/EPGManSearchAgent.h>
#include <SvEPGDataLayer/Plugins/Innov8onEPGEvent.h>
#include <Services/QBCASManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBPlayerControllers/QBChannelPlayer.h>
#include <SvPlayerManager/SvPlayerTask.h>

#include <QBInput/QBInputCodes.h>

/// Maximum number of QBReminder tasks
#define QBREMINDER_MAX_NUMBER_OF_TASKS 30

#define MIN_DIRECTORY_LENGTH 3

typedef enum {
    QBReminderEditorPaneDirectoryNameValidatorStatus_OK,
    QBReminderEditorPaneDirectoryNameValidatorStatus_EMPTY,
    QBReminderEditorPaneDirectoryNameValidatorStatus_TOO_SHORT,
    QBReminderEditorPaneDirectoryNameValidatorStatus_INCORRECT,
    QBReminderEditorPaneDirectoryNameValidatorStatus_EXISTS
} QBReminderEditorPaneDirectoryNameValidatorStatus;


SvLocal void QBReminderEditorPaneOnRemindMe(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item);

/**
 * Handle errors in scheduling recordings.
 */

# if 0
SvLocal void QBReminderEditorPaneRecordErrorOnShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBReminderEditorPane self = ptr;
    svSettingsRestoreContext(self->settingsCtx);
    if (!self->errorLabel) {
        self->errorLabel = svLabelNewFromSM(self->appGlobals->res, "ScheduleError");
        svLabelSetText(self->errorLabel, gettext("You have too many recordings."));
        self->errorLabel->fpolicy = true;
    }
    svSettingsWidgetAttach(frame, self->errorLabel, "ScheduleError", 0);
    svSettingsPopComponent();
}

SvLocal void QBReminderEditorPaneRecordErrorOnHide(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBReminderEditorPane self = ptr;
    svWidgetDetach(self->errorLabel);
}

SvLocal void QBReminderEditorPaneRecordErrorOnSetActive(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBReminderEditorPane self = ptr;
    svWidgetSetFocus(self->errorLabel);
}
#endif

SvLocal void QBReminderEditorPaneOnReminderResolveConflicts(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    SvDBObject obj = (SvDBObject) self->channel;

    if (self->event->startTime <= SvTimeNow()) {
        QBContextMenuHide(self->contextMenu, false);
        return;
    }

    SvArray conflicts = QBReminderResolveConflicts(self->appGlobals->reminder, self->channel, self->event);
    if (conflicts) {
        QBReminderTask task;
        SvIterator it = SvArrayIterator(conflicts);
        while ((task = (QBReminderTask) SvIteratorGetNext(&it))) {
            SvTimeRange range = {
                .startTime = task->startTime,
                .endTime   = task->endTime
            };
            SvEPGManagerPropagateEventsChange(self->epgManager, task->channelID, &range, NULL);
        }
        SVTESTRELEASE(conflicts);
    }

    conflicts = NULL;

    int result = QBReminderAdd(self->appGlobals->reminder, self->channel, self->event, &conflicts);
    if (result == 0) {
        SvTimeRange range = {
            .startTime = self->event->startTime,
            .endTime   = self->event->endTime
        };
        SvEPGManagerPropagateEventsChange(self->epgManager, obj->id, &range, NULL);
        QBContextMenuHide(self->contextMenu, false);
    } else {
        QBContextMenuPopPane(self->contextMenu);
        QBReminderEditorPaneOnRemindMe(self_, id, self->options, item);
        SVTESTRELEASE(conflicts);
    }
}

SvLocal void QBReminderEditorPaneOnReminderCancelConflicts(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal bool QBReminderEditorPaneIsUnderLimitOfTasks(QBReminderEditorPane self)
{
    return QBReminderGetTasksCount(self->appGlobals->reminder) < QBREMINDER_MAX_NUMBER_OF_TASKS;
}

SvLocal void QBReminderEditorPaneOnCancelTooManyReminders(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBReminderEditorPaneShowTooManyReminders(QBReminderEditorPane self, QBBasicPane pane)
{
    QBBasicPane subpane = QBBasicPaneCreateFromSettings("BasicPane.settings", self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, QBBasicPaneGetLevel(pane) + 1, SVSTRING("BasicPane"));

    SvWidget description = QBBasicPaneGetTitle(subpane);
    const char *descText = gettext("You have too many reminders.");
    svLabelSetText(description, descText);

    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(subpane, SVSTRING("cancel"), option, QBReminderEditorPaneOnCancelTooManyReminders, self);
    SVRELEASE(option);

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) subpane);
    SVRELEASE(subpane);
}

SvLocal void QBReminderEditorPaneOnRemindMe(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    SvArray conflicts = NULL;

    if (self->event->startTime <= SvTimeNow()) {
        QBContextMenuHide(self->contextMenu, false);
        return;
    }

    if (!QBReminderEditorPaneIsUnderLimitOfTasks(self)) {
        QBReminderEditorPaneShowTooManyReminders(self, pane);
        return;
    }

    int result = QBReminderAdd(self->appGlobals->reminder, self->channel, self->event, &conflicts);
    if (result == 0) {
        QBContextMenuHide(self->contextMenu, false);
        return;
    }

    QBBasicPane subpane = QBBasicPaneCreateFromSettings("BasicPane.settings", self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, QBBasicPaneGetLevel(pane) + 1, SVSTRING("BasicPane"));

    SvWidget title = QBBasicPaneGetTitle(subpane);

    char *buf = strdup(gettext("This reminder will override the following reminders:\n"));
    int cnt = SvArrayCount(conflicts);
    cnt = (cnt < 3) ? cnt : 3;
    size_t i;
    for (i = 0; i < SvArrayCount(conflicts) && cnt; i++) {
        QBReminderTask task = (QBReminderTask) SvArrayAt(conflicts, i);
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvGenericObject playlist = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, task->channelID);
        if (!channel) {
            continue;
        } else
            cnt--;

        char *tmp;
        asprintf(&tmp, "%s %s", buf ? buf : "", SvStringCString(channel->name));
        free(buf);
        buf = tmp;
    }
    svLabelSetText(title, buf);
    free(buf);

    SVTESTRELEASE(conflicts);

    SvString option = SvStringCreate(gettext("Override"), NULL);
    QBBasicPaneAddOption(subpane, SVSTRING("override"), option, QBReminderEditorPaneOnReminderResolveConflicts, self);
    SVRELEASE(option);
    option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(subpane, SVSTRING("cancel"), option, QBReminderEditorPaneOnReminderCancelConflicts, self);
    SVRELEASE(option);

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) subpane);
    SVRELEASE(subpane);
}

SvLocal void QBReminderEditorPaneOnCancelRemindMe(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    SvDBObject obj = (SvDBObject) self->channel;
    if (self->event->startTime <= SvTimeNow()) {
        QBContextMenuHide(self->contextMenu, false);
        return;
    }

    QBReminderRemove(self->appGlobals->reminder, self->channel, self->event);
    SvTimeRange range = {
        .startTime = self->event->startTime,
        .endTime   = self->event->endTime
    };
    SvEPGManagerPropagateEventsChange(self->epgManager, obj->id, &range, NULL);

    QBContextMenuHide(self->contextMenu, false);
}

#if 0

SvLocal void QBReminderEditorPaneOnRecord(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item);

SvLocal void QBReminderEditorPaneOnPVRResolveConflicts(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    //FIXME
#if 0
    SvArray conflicts = NULL;
    QBSchedManagerFind(NULL, self->currentReservation.startMargin, self->currentReservation.endTime, false, &conflicts);
    QBSchedManagerResolveConflicts(&self->currentReservation, NULL, QBSchedManagerResolveType_modify);
    if (conflicts) {
        int i, cnt = SvArrayCount(conflicts);
        for (i = 0; i < cnt; i++) {
            QBSchedDesc *recording = (QBSchedDesc *) SvArrayAt(conflicts, i);
            SvTimeRange range = {
                .startTime = recording->params.startTime,
                .endTime   = recording->params.endTime,
            };
            SvValue channelId = SvValueCreateWithString(recording->params.source, NULL);
            SvEPGManagerPropagateEventsChange(self->epgManager, channelId, &range, NULL);
            SVRELEASE(channelId);
        }
    }
    SVTESTRELEASE(conflicts);
#endif
    QBContextMenuPopPane(self->contextMenu);
    QBReminderEditorPaneOnRecord(self_, id, self->options, item);
}

SvLocal void QBReminderEditorPanePVRConflictsCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}
#endif

SvLocal void QBReminderEditorPaneOnShowSimilar(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    SvEPGEvent ev = self->event;

    SVTESTRELEASE(self->searchResults);
    self->searchResults = QBSearchResultsContextCreate(self->appGlobals);

    SvArray agents = SvArrayCreate(NULL);
    SvString playlistId = SvTVChannelGetAttribute(self->channel, SVSTRING("TV")) ? SVSTRING("TVChannels") : SVSTRING("RadioChannels");
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvObject view = QBPlaylistManagerGetById(playlists, playlistId);
    SvObject agent = QBInitLogicCreateSearchAgent(self->appGlobals->initLogic,
                                                  EPGManSearchAgentMode_findSimilar,
                                                  ev->category, view);

    SvArrayAddObject(agents, agent);
    SVRELEASE(agent);

    SvString title = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic,
                                                   self->appGlobals->langPreferences, ev);
    SvObject search = QBSearchEngineCreate(agents, title, NULL);
    SVRELEASE(agents);

    QBSearchResultsSetDataSource(self->searchResults, search);
    SVRELEASE(search);

    QBSearchResultsContextExecute(self->searchResults, self->contextMenu);
}

SvLocal void QBReminderEditorPaneOnShowLanguages(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    QBWindowContext  ctx = self->appGlobals->newTV;
    QBTVHideSideMenu(ctx);
    QBTVLangMenuShow(ctx, QBLangMenuSubMenu_Audio);
}

SvLocal void QBReminderEditorPaneResolverFinishCallback(void *self_, bool saved)
{
    QBReminderEditorPane self = self_;
    if (self->resolver) {

        SVRELEASE(self->resolver);
        self->resolver = NULL;
    }
    if (saved)
        QBContextMenuPopPane(self->contextMenu);
}

SvLocal void QBReminderEditorPaneOnStopRecordAlways(void *self_, SvString id, QBBasicPane pane_, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    SvArray dirs = QBPVRProviderCreateActiveDirectoriesListForEvent(self->appGlobals->pvrProvider, self->event);

    SvIterator it = SvArrayIterator(dirs);
    QBPVRDirectory dir = NULL;
    while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it))) {
        if (dir->type == QBPVRDirectoryType_keyword) {
            QBPVRProviderStopDirectory(self->appGlobals->pvrProvider, dir);
        }
    }
    SVRELEASE(dirs);

    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBReminderEditorPaneOnStopRecordRepeatedly(void *self_, SvString id, QBBasicPane pane_, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    SvArray dirs = QBPVRProviderCreateActiveDirectoriesListForEvent(self->appGlobals->pvrProvider, self->event);

    SvIterator it = SvArrayIterator(dirs);
    QBPVRDirectory dir = NULL;
    while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it))) {
        if (dir->type == QBPVRDirectoryType_repeated) {
            QBPVRProviderStopDirectory(self->appGlobals->pvrProvider, dir);
        }
    }
    SVRELEASE(dirs);

    QBContextMenuHide(self->contextMenu, false);
}

SvLocal QBRecordingEditPane QBReminderEditorPaneOnRecordPrepareRecordingPane(QBReminderEditorPane self, QBRecordingEditPaneType recordingType)
{
    struct QBRecordingEditPaneCreationSettings_ params = {
        .appGlobals       = self->appGlobals,
        .settingsFileName = "RecordingEditPane.settings",
        .optionsFileName  = "QBRecordingEditPane.json",
        .contextMenu      = self->contextMenu,
        .level            = 2
    };
    QBRecordingEditPane pane = QBRecordingEditPaneCreatePvrWithChannel(&params, self->channel, recordingType, NULL);
    if (pane) {
        SvString title = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, self->event);
        QBRecordingEditPaneSetKeyword(pane, title);
    }

    return pane;
}

SvLocal void QBReminderEditorPaneOnRecordAlways(void *self_, SvString id, QBBasicPane pane_, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    QBRecordingEditPane pane = QBReminderEditorPaneOnRecordPrepareRecordingPane(self, QBRecordingEditPane_Keyword);
    if (pane) {
        QBContextMenuPushPane(self->contextMenu, (SvGenericObject) pane);
        QBContextMenuShow(self->contextMenu);
        SVRELEASE(pane);
    }
}

SvLocal void QBReminderEditorPaneOnRecordRepeatedly(void *self_, SvString id, QBBasicPane pane_, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    QBRecordingEditPane pane = QBReminderEditorPaneOnRecordPrepareRecordingPane(self, QBRecordingEditPane_Manual);
    if (pane) {
        QBRecordingEditPaneSetTimeRange(pane, SvLocalTimeFromUTC(self->event->startTime), SvLocalTimeFromUTC(self->event->endTime));
        QBRecordingEditPaneSetRepeatMode(pane, QBPVRRepeatedRecordings_Daily);
        QBContextMenuPushPane(self->contextMenu, (SvGenericObject) pane);
        QBContextMenuShow(self->contextMenu);
        SVRELEASE(pane);
    }
}

SvLocal void QBReminderEditorPaneOnRecord(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    if (!self->event || !self->channel || !self->appGlobals->pvrProvider || self->event->endTime < SvTimeNow()) {
        QBContextMenuHide(self->contextMenu, false);
        return;
    }

    SvArray recordings = QBRecordingUtilsCreateNotCompletedRecordingsForEvent(self->appGlobals->pvrProvider, self->event);
    QBPVRRecording recording = QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Channel);

    if (recording) {
        if (recording->state == QBPVRRecordingState_active) {
            QBPVRProviderStopRecording(self->appGlobals->pvrProvider, recording);
        } else if (recording->state == QBPVRRecordingState_scheduled) {
            QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, recording);
        }
    } else if ((recording = QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Any))) {
        QBContextMenuHide(self->contextMenu, false);
        SVRELEASE(recordings);
        return;
    }

    SVRELEASE(recordings);

#if 0
    SvContent c = createContentFromChannel(self->channel);

    SvContentMetaData data = SvContentMetaDataCreate(NULL);
    SvContentMetaDataSetStringProperty(data, SVSTRING(REC_META_KEY__APP_SOURCE_NAME), self->channel->name);
    SvContentMetaDataSetBooleanProperty(data, SVSTRING(REC_META_KEY__APP_CHANNEL_BLOCKED),
                                        SvTVChannelGetAttribute(self->channel, SVSTRING("blocked")));
    SvContentMetaDataSetStringProperty(data, SVSTRING(REC_META_KEY__APP_EVENT_NAME),
                                       QBEventUtilsGetTitleFromEvent(self->appGlobals, self->event));
#endif

    if (self->appGlobals->casPVRManager) {
        SvInvokeInterface(QBCASPVRManager, self->appGlobals->casPVRManager, recordingAdded, "QBReminderEditorPane");
    }

    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
              SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
    params->channel = SVRETAIN(self->channel);
    params->event = SVRETAIN(self->event);
    params->type = QBPVRRecordingType_event;

    const char *pvrStartMarginStr = QBConfigGet("PVRSTARTMARGIN");
    int pvrStartMargin = pvrStartMarginStr ? atoi(pvrStartMarginStr) * 60 : 0;
    const char *pvrEndMarginStr = QBConfigGet("PVRENDMARGIN");
    int pvrEndMargin = pvrEndMarginStr ? atoi(pvrEndMarginStr) * 60 : 0;
    params->startMargin = pvrStartMargin;
    params->endMargin = pvrEndMargin;

    SVTESTRELEASE(self->resolver);
    self->resolver = QBPVRConflictResolverCreate(self->appGlobals, self->contextMenu, params, 2);

    SVRELEASE(params);
    QBPVRConflictResolverSetFinishCallback(self->resolver, QBReminderEditorPaneResolverFinishCallback, self);
    QBPVRConflictResolverResolve(self->resolver);

#if 0
    SvArray conflicts = NULL;
    QBSchedManagerResolveConflicts(&params, NULL, QBSchedManagerResolveType_test);
    int result = QBSchedManagerAdd(&params, c, true, &conflicts);
    SVRELEASE(c);
    SVRELEASE(data);

    if (result == 0) {
        SvTimeRange range = {
            .startTime = self->event->startTime,
            .endTime   = self->event->endTime
        };
        SvEPGManagerPropagateEventsChange(self->epgManager, obj->id, &range, NULL);
        QBContextMenuHide(self->contextMenu, false);
    } else {
        if (result == SV_ERR_RESOURCE_LIMIT) {
            QBBasicPane subpane = QBBasicPaneCreateFromSettings("PVRConflictPane.settings", self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, QBBasicPaneGetLevel(pane) + 1, SVSTRING("BasicPane"));

            SvWidget title = QBBasicPaneGetTitle(subpane);
            int cnt = SvArrayCount(conflicts);

            if (cnt > 3) {
                svLabelSetText(title, gettext("New recording cannot be scheduled due to many conflicts with previously scheduled recordings"));
                SvString option = SvStringCreate(gettext("Ok"), NULL);
                QBBasicPaneAddOption(subpane, SVSTRING("Ok"), option, QBReminderEditorPanePVRConflictsCancel, self);
                SVRELEASE(option);
            } else {
                char *buf = NULL;
                asprintf(&buf,
                         ngettext("Your new recording conflicts with %i previously scheduled recording. Would you like to:\n\n",
                                  "Your new recording conflicts with %i previously scheduled recordings. Would you like to:\n\n", cnt), cnt);
                int i;
                for (i = 0; i < cnt; i++) {
                    SvString channelName = NULL, eventName = NULL;
                    QBSchedDesc *recording = (QBSchedDesc *) SvArrayAt(conflicts, i);
                    SvContentMetaDataGetStringProperty(recording->meta, SVSTRING(REC_META_KEY__APP_SOURCE_NAME), &channelName);
                    SvContentMetaDataGetStringProperty(recording->meta, SVSTRING(REC_META_KEY__APP_EVENT_NAME), &eventName);
                    char *desc = NULL, *action = NULL;
#define MAX_DATE_LENGTH 128
                    char timeBuf[MAX_DATE_LENGTH], timeBuf2[MAX_DATE_LENGTH];
                    struct tm tmp;
                    SvTimeBreakDown(SvTimeConstruct(recording.params.startTime, 0), true, &tmp);
                    strftime(timeBuf, sizeof(timeBuf), "%H:%M", &tmp);
                    SvTimeBreakDown(SvTimeConstruct(recording.params.endTime, 0), true, &tmp);
                    strftime(timeBuf2, sizeof(timeBuf2), "%H:%M", &tmp);
                    asprintf(&desc, gettext("Channel: %s\nEvent: %s\nTime: %s-%s\n"),
                             SvStringCString(channelName),
                             SvStringCString(eventName),
                             timeBuf, timeBuf2);
                    if (recording->params.startTime >= params.startTime &&
                        recording->params.endTime <= params.endTime) {
                        asprintf(&action, gettext("Cancel scheduled recording"));
                    } else if (recording->params.startTime > params.startTime) {
                        SvTimeBreakDown(SvTimeConstruct(params.endTime, 0), true, &tmp);
                        strftime(timeBuf, sizeof(timeBuf), "%H:%M", &tmp);
                        asprintf(&action, gettext("Delay start of the recording to %s"), timeBuf);
                    } else {
                        SvTimeBreakDown(SvTimeConstruct(params.startTime, 0), true, &tmp);
                        strftime(timeBuf, sizeof(timeBuf), "%H:%M", &tmp);
                        asprintf(&action, gettext("Shorten recording to %s"), timeBuf);
                    }
                    char *tmp2 = NULL;
                    asprintf(&tmp2, "%s%s\n%s\n", buf, action, desc);
                    free(buf);
                    buf = tmp2;
                }
                svLabelSetText(title, buf);
                free(buf);

                SvString option = SvStringCreate(gettext("Resolve conflicts"), NULL);
                QBBasicPaneAddOption(subpane, SVSTRING("resolve conflicts"), option, QBReminderEditorPaneOnPVRResolveConflicts, self);
                SVRELEASE(option);
                option = SvStringCreate(gettext("Cancel"), NULL);
                QBBasicPaneAddOption(subpane, SVSTRING("cancel"), option, QBReminderEditorPanePVRConflictsCancel, self);
                SVRELEASE(option);
            }

            QBContextMenuPushPane(self->contextMenu, (SvGenericObject) subpane);
            SVRELEASE(subpane);
        } else {
            svSettingsRestoreContext(self->settingsCtx);
            QBContainerPane subpane = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
            static struct QBContainerPaneCallbacks_t errorCallbacks = {
                .onShow    = QBReminderEditorPaneRecordErrorOnShow,
                .onHide    = QBReminderEditorPaneRecordErrorOnHide,
                .setActive = QBReminderEditorPaneRecordErrorOnSetActive,
            };
            QBContainerPaneInit(subpane, self->appGlobals->res, self->contextMenu, QBBasicPaneGetLevel(pane) + 1, SVSTRING("SideMenu"), &errorCallbacks, self);
            svSettingsPopComponent();
            QBContextMenuPushPane(self->contextMenu, (SvGenericObject) subpane);
            SVRELEASE(subpane);
        }
    }
    SVTESTRELEASE(conflicts);
#endif
}


SvLocal void QBReminderEditorPaneOnRecordChannel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    time_t now = SvTimeNow();

    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
              SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
    params->channel = SVRETAIN(self->channel);
    params->startTime = SvTimeGetCurrentTime();
    params->type = QBPVRRecordingType_OTR;

    //end time heuristic
    int maxDuration = 3600 * 2;
    if (self->event) {
        int end = self->event->endTime - now;
        if (maxDuration < end)
            maxDuration = end;
    }

    params->duration = maxDuration;

    SVTESTRELEASE(self->resolver);
    self->resolver = QBPVRConflictResolverCreate(self->appGlobals, self->contextMenu, params, 2);

    SVRELEASE(params);
    QBPVRConflictResolverSetFinishCallback(self->resolver, QBReminderEditorPaneResolverFinishCallback, self);
    QBPVRConflictResolverResolve(self->resolver);
}

void QBReminderEditorPaneRecord(QBReminderEditorPane self)
{
    QBReminderEditorPaneOnRecord(self, SVSTRING("record"), NULL, NULL);
}

SvLocal void QBReminderEditorPaneOnRemove(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    //Only active/single recordings can be stopped
    SvArray recordings = QBRecordingUtilsCreateNotCompletedRecordingsForEvent(self->appGlobals->pvrProvider, self->event);
    QBPVRRecording recording = QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Active);

    if (!recording) {
        recording = (QBPVRRecording) SvArrayLastObject(recordings);
    }
    SVRELEASE(recordings);

    if (!recording)
        goto fini;

    if (recording->state == QBPVRRecordingState_active) {
        QBPVRProviderStopRecording(self->appGlobals->pvrProvider, recording);
    } else if (recording->state == QBPVRRecordingState_scheduled) {
        QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, recording);
    }

fini:
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBReminderEditorPaneOnPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    // BEGIN AMERELES #2206 Canal lineal que lleve a VOD (EPG a canal que lleva a VoD no funciona)
    QBTVLogicSetLastInputEventId(self->appGlobals->tvLogic, QBKEY_MENU);
    QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
    QBTVLogicPlayChannel(self->appGlobals->tvLogic, self->channel, SVSTRING("QBReminderEditor"));
    // END AMERELES #2206 Canal lineal que lleve a VOD (EPG a canal que lleva a VoD no funciona)
    //QBTVLogicPlayChannel(self->appGlobals->tvLogic, self->channel, SVSTRING("QBReminderEditor"));
    //QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
}

SvLocal void QBReminderEditorPaneOnRecordWithNPvr(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    if (QBNPvrLogicAreRecordingsCustomizable(self->appGlobals) && (QBNPvrLogicIsSetMarginsFeatureEnabled() || QBNPvrLogicIsRecoveryFeatureEnabled())) {
        if (!self->recordOnePane) {
            self->recordOnePane = QBNPvrRecordingEditPaneCreate(
                self->settingsCtx,
                self->appGlobals,
                self->contextMenu,
                2,
                QBNPvrRecordingEditPaneType_Manual);
        }
        QBNPvrRecordingEditPaneSetEvent(self->recordOnePane, self->event);
        QBNPvrRecordingEditPaneShow(self->recordOnePane);
    } else {
        struct QBnPVRRecordingSchedParams_ params = {
            .type = QBnPVRRecordingType_manual,
            .event = self->event,
        };
        if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
            params.recoveryOption = SvValueCreateWithInteger(QBNPvrRecordingEditPaneType_Manual, NULL);
        }

        QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, scheduleRecording, &params);
        SVTESTRELEASE(params.recoveryOption);
        QBNPvrAgentTrackRequest(self->appGlobals->npvrAgent, request, self->event);
        QBNPvrDialogCreateAndShow(self->appGlobals, request);
        QBContextMenuHide(self->contextMenu, false);
    }
}

SvLocal void QBReminderEditorPaneOnDeleteNPvrRecording(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    SvArray recordings = SvArrayCreate(NULL);
    SvArrayAddObject(recordings, (SvGenericObject) self->currentNPvrRec);
    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteRecordings, recordings);
    SVRELEASE(recordings);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBReminderEditorPaneOnStopKeywordNPvrRecording(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    QBnPVRDirectory dir = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, self->currentNPvrRec->directoryId);
    if (!dir || !dir->keywordId) {
        return;
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteKeyword, dir->keywordId);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void QBReminderEditorPaneOnStopSeriesNPvrRecording(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    QBnPVRDirectory dir = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, self->currentNPvrRec->directoryId);

    if (!dir || !dir->seriesId) {
        return;
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, deleteSeries, dir->seriesId);

    QBNPvrDialogCreateAndShow(self->appGlobals, request);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void
QBReminderEditorPaneRecordSeriesWithNPvr(QBReminderEditorPane self, SvString folderName)
{
    struct QBnPVRRecordingSchedParams_ params = {
        .type = QBnPVRRecordingType_series,
        .seriesID = folderName,
        .event = self->event,
    };
    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        params.recoveryOption = SvValueCreateWithInteger(QBnPVRRecordingSpaceRecovery_Manual, NULL);
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, scheduleSeries, &params);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);
    SVTESTRELEASE(params.recoveryOption);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void
QBReminderEditorPaneRecordKeywordWithNPvr(QBReminderEditorPane self, SvString keyword)
{
    struct QBnPVRRecordingSchedParams_ params = {
        .type = QBnPVRRecordingType_keyword,
    };
    if (QBNPvrLogicIsRecoveryFeatureEnabled()) {
        params.recoveryOption = SvValueCreateWithInteger(QBnPVRRecordingSpaceRecovery_Manual, NULL);
    }
    if (keyword) {
        params.keyword = keyword;
    }
    if (self->channel) {
        SvValue channelID = (SvValue) SvTVChannelGetAttribute(self->channel, SVSTRING("productID"));

        if (channelID && SvValueIsInteger(channelID)) {
            params.channelId = SvStringCreateWithFormat("%d", SvValueGetInteger(channelID));
        } else if (channelID && SvValueIsString(channelID)) {
            params.channelId = SVRETAIN(SvValueGetString(channelID));
        }
    }

    QBnPVRProviderRequest request = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, scheduleKeyword, &params);
    QBNPvrDialogCreateAndShow(self->appGlobals, request);
    SVTESTRELEASE(params.channelId);
    SVTESTRELEASE(params.recoveryOption);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void
QBReminderEditorPaneOnErrorMessageCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal void
QBReminderEditorPaneShowErrorMessagePaneWithLevel(QBReminderEditorPane self, int level, SvString message)
{
    svSettingsRestoreContext(self->settingsCtx);
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->contextMenu, level + 1, SVSTRING("BasicPane"));
    QBBasicPaneSetTitle(options, message);
    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBReminderEditorPaneOnErrorMessageCancel, self);
    SVRELEASE(option);
    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) options);
    SVRELEASE(options);
    QBContextMenuShow(self->contextMenu);
    svSettingsPopComponent();
}

SvLocal bool
QBReminderEditorPaneValidateDirectoryNameAndShowPaneWithLevel(QBReminderEditorPane self, int level, SvString name)
{
    SvString message = NULL;

    if (SvStringLength(name) < MIN_DIRECTORY_LENGTH) {
        message = SvStringCreateWithFormat(gettext("Directory name must be at least %d characters"), MIN_DIRECTORY_LENGTH);
        goto err;
    } else if (QBStringUtilsIsWhitespace(SvStringCString(name))) {
        message = SvStringCreate(gettext("Directory name cannot start with a space"), NULL);
        goto err;
    }

    SvArray dirList = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, listDirectories, dirList);

    const char* nameCStr = SvStringCString(name);
    SvIterator it = SvArrayIterator(dirList);
    QBnPVRDirectory dir = NULL;
    while ((dir = (QBnPVRDirectory) SvIteratorGetNext(&it))) {
        if (SvStringEqualToCString(dir->name, nameCStr)) {
            SVRELEASE(dirList);
            message = SvStringCreateWithFormat(gettext("Directory '%s' already exists"), nameCStr);
            goto err;
        }
    }

    SVRELEASE(dirList);

    return true;

err:
    QBReminderEditorPaneShowErrorMessagePaneWithLevel(self, level, message);
    SVRELEASE(message);
    return false;
}

SvLocal void
QBReminderEditorPaneCommitRecordSeriesWithLevel(QBReminderEditorPane self, int level)
{
    SvString name = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic,
                                                  self->appGlobals->langPreferences,
                                                  self->event);
    if (QBReminderEditorPaneValidateDirectoryNameAndShowPaneWithLevel(self, level, name)) {
        QBReminderEditorPaneRecordSeriesWithNPvr(self, name);
    }
}

SvLocal void
QBReminderEditorPaneShowSeriesPaneWithLevel(QBReminderEditorPane self, int level)
{
    if (!self->seriesPane) {
        self->seriesPane = QBNPvrRecordingEditPaneCreate(self->settingsCtx,
                                                         self->appGlobals,
                                                         self->contextMenu,
                                                         level,
                                                         QBNPvrRecordingEditPaneType_Series);
    }
    QBNPvrRecordingEditPaneSetEvent(self->seriesPane, self->event);
    QBNPvrRecordingEditPaneShow(self->seriesPane);
    QBContextMenuShow(self->contextMenu);
}

SvLocal void
QBReminderEditorPaneShowNpvrSeriesPane(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    if (QBNPvrLogicAreRecordingsCustomizable(self->appGlobals)) {
        QBReminderEditorPaneShowSeriesPaneWithLevel(self, 2);
    } else {
        QBReminderEditorPaneCommitRecordSeriesWithLevel(self, 1);
    }
}

SvLocal void
QBReminderEditorPaneCommitRecordKeywordWithLevel(QBReminderEditorPane self, int level)
{
    SvString keyword = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic,
                                                     self->appGlobals->langPreferences,
                                                     self->event);
    if (QBReminderEditorPaneValidateDirectoryNameAndShowPaneWithLevel(self, level, keyword)) {
        QBReminderEditorPaneRecordKeywordWithNPvr(self, keyword);
    }
}

SvLocal void
QBReminderEditorPaneShowKeywordPaneWithLevel(QBReminderEditorPane self, int level)
{
    svSettingsRestoreContext(self->settingsCtx);

    struct QBRecordingEditPaneCreationSettings_ params = {
        .appGlobals       = self->appGlobals,
        .settingsFileName = NULL,
        .optionsFileName  = "QBRecordingEditPane.json",
        .contextMenu      = self->contextMenu,
        .level            = level
    };

    QBRecordingEditPane pane = QBRecordingEditPaneCreateNPvrKeywordWithEvent(&params, self->event, NULL);
    if (pane) {
        QBContextMenuPushPane(self->contextMenu, (SvGenericObject) pane);
        QBContextMenuShow(self->contextMenu);
        SVRELEASE(pane);
    }

    svSettingsPopComponent();
}

SvLocal void
QBReminderEditorPaneShowKeywordPane(void *self_, SvString id, QBBasicPane pane_, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;
    if (QBNPvrLogicAreRecordingsCustomizable(self->appGlobals)) {
        QBReminderEditorPaneShowKeywordPaneWithLevel(self, 2);
    } else {
        QBReminderEditorPaneCommitRecordKeywordWithLevel(self, 1);
    }
}

SvLocal SvObject
QBReminderEditorPaneCreateController(QBReminderEditorPane self, QBAnyPlayerLogic anyPlayerLogic, SvValue id)
{
    QBContentInfo contentInfo = NULL;
    SvDBRawObject cutvEvent = NULL;

    switch (self->playbackType) {
        case QBReminderEditorPanePlaybackType_StartOver:
        case QBReminderEditorPanePlaybackType_StartCatchup:
            contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, NULL, NULL, (SvObject) id, NULL, NULL, NULL);
            break;

        case QBReminderEditorPanePlaybackType_WatchInCUTV:
        case QBReminderEditorPanePlaybackType_ContinuousInCUTV:
            cutvEvent = CutvManagerGetObject(self->appGlobals->cutvManager, self->event);
            contentInfo = (QBContentInfo) QBContentInfoCreate(self->appGlobals, NULL, NULL, (SvObject) cutvEvent, NULL, NULL, NULL);
            break;

        default:
            SvLogError("%s: unsupported 'playFromTheBeginningType' ", __func__);
            assert(false);
            break;
    }

    SvObject controller = QBAnyPlayerLogicCreateController(anyPlayerLogic, contentInfo);
    SVRELEASE(contentInfo);

    return controller;
}

SvLocal SvContent
QBReminderEditorPaneCreateContent(QBReminderEditorPane self)
{
    SvContent content = NULL;

    switch (self->playbackType) {
        case QBReminderEditorPanePlaybackType_StartOver:
            content = SvContentCreateFromURI(NULL, NULL);
            SvContentMetaDataSetStringProperty(SvContentGetMetaData(content), SVSTRING("playback_mode"), SVSTRING("startOver"));
            break;

        case QBReminderEditorPanePlaybackType_StartCatchup:
            content = SvContentCreateFromURI(NULL, NULL);
            SvContentMetaDataSetStringProperty(SvContentGetMetaData(content), SVSTRING("playback_mode"), SVSTRING("startCatchup"));
            break;

        case QBReminderEditorPanePlaybackType_WatchInCUTV:
            content = CutvManagerCreateContent(self->appGlobals->cutvManager, self->event);
            break;
        case QBReminderEditorPanePlaybackType_ContinuousInCUTV:
            content = CutvManagerCreateContent(self->appGlobals->cutvManager, self->event);
            SvContentMetaDataSetObjectProperty(SvContentGetMetaData(content), SVSTRING("continuousCUTV_event"), (SvObject) self->event);
            break;

        default:
            SvLogError("%s: unsupported 'playFromTheBeginningType' ", __func__);
            assert(false);
            break;
    }

    return content;
}

SvLocal void
QBReminderEditorPaneStartPlayback(QBReminderEditorPane self, bool authenticated)
{
    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(self->appGlobals);
    SvValue fullID = SvEPGEventCreateID(self->event);

    if (!anyPlayerLogic || !fullID || !SvValueIsString(fullID)) {
        SVTESTRELEASE(anyPlayerLogic);
        SVTESTRELEASE(fullID);
        return;
    }

    QBAnyPlayerLogicSetVodId(anyPlayerLogic, SvValueGetString(fullID));

    if (self->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, SvValueGetString(fullID));
        if (!bookmark) {
            bookmark = QBBookmarkManagerSetLastPositionBookmark(self->appGlobals->bookmarkManager, SvValueGetString(fullID), 0.0, QBBookmarkType_Generic);
        }

        if (bookmark) {
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
        }
    }

    SvContent content = QBReminderEditorPaneCreateContent(self);
    SvObject controller = QBReminderEditorPaneCreateController(self, anyPlayerLogic, fullID);
    SVRELEASE(fullID);

    SvInvokeInterface(QBAnyPlayerController, (SvObject) controller, setContent, (SvObject) content);
    SVTESTRELEASE(content);

    QBAnyPlayerLogicSetController(anyPlayerLogic, controller);
    SVRELEASE(controller);

    //QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic);
    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(self->appGlobals, anyPlayerLogic, time(NULL)); // AMERELES BoldContentTracking Youbora
    if (self->playbackType == QBReminderEditorPanePlaybackType_StartOver) {
        QBPVRPlayerContextSetMetaDataFromEvent(pvrPlayer, self->event, QBPVRPlayerContextPlaybackType_startOver);
    } else {
        QBPVRPlayerContextSetMetaDataFromEvent(pvrPlayer, self->event, QBPVRPlayerContextPlaybackType_catchup);
    }
    SVRELEASE(anyPlayerLogic);

    if (authenticated)
        QBPVRPlayerContextSetAuthenticated(pvrPlayer);

    QBPVRPlayerContextUseBookmarks(pvrPlayer, !self->playFromTheBeginning);
    QBApplicationControllerPushContext(self->appGlobals->controller, pvrPlayer);

    SVRELEASE(pvrPlayer);
}

SvLocal void
QBReminderEditorPaneCheckParentalControllPINCallback(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBReminderEditorPane self = ptr;

    if (!self->parentalPopup) {
        return;
    }

    self->parentalPopup = NULL;

    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        QBReminderEditorPaneStartPlayback(self, true);
        QBContextMenuHide(self->contextMenu, false);
    }
}

SvLocal void
QBReminderEditorPaneCheckParentalControllPIN(QBReminderEditorPane self)
{
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler, self->appGlobals->accessMgr, SVSTRING("PC"));
    SvWidget master = NULL;
    svSettingsPushComponent("ParentalControl.settings");
    SvWidget dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, &master);
    svSettingsPopComponent();
    self->parentalPopup = dialog;

    QBDialogRun(dialog, self, QBReminderEditorPaneCheckParentalControllPINCallback);
}

SvLocal void
QBReminderEditorPaneStartPlaybackIfPCAllowed(QBReminderEditorPane self)
{
    if (!QBParentalControlEventShouldBeBlocked(self->appGlobals->pc, self->event)) {
        QBReminderEditorPaneStartPlayback(self, false);
        QBContextMenuHide(self->contextMenu, false);
    } else {
        QBReminderEditorPaneCheckParentalControllPIN(self);
    }
}

SvLocal void
QBReminderEditorPaneNPVRStartOver(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    self->playFromTheBeginning = id && SvStringEqualToCString(id, "start-over");
    self->playbackType = QBReminderEditorPanePlaybackType_StartOver;

    QBReminderEditorPaneStartPlaybackIfPCAllowed(self);
}

SvLocal void
QBReminderEditorPaneStartCatchup(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    self->playFromTheBeginning = id && SvStringEqualToCString(id, "start-catchup");
    self->playbackType = QBReminderEditorPanePlaybackType_StartCatchup;

    QBReminderEditorPaneStartPlaybackIfPCAllowed(self);
}

SvLocal void QBReminderEditorPaneStartOver(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    time_t startTime = self->event->startTime;
    SvInvokeInterface(QBChannelPlayer, (SvObject) self->appGlobals->newTV, play, NULL, startTime);

    QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
    QBContextMenuHide(self->contextMenu, false);
}

SvLocal bool
QBReminderEditorPaneInitNPVRStartOver(QBReminderEditorPane self, const bool started, const bool ended)
{
    bool startOver = false;
    bool ret = false;
    SvValue fullID = NULL;

    if (!self->event) {
        goto fini;
    }

    fullID = SvEPGEventCreateID(self->event);
    if (!fullID || !SvValueIsString(fullID)) {
        goto fini;
    }

    if (SvObjectIsInstanceOf((SvObject) self->event, Innov8onEPGEvent_getType())) {
        startOver = SvEPGEventCanBeRemotelyStartedOver(self->event);
    }

    if (started && !ended && startOver) {
        QBBasicPaneAddOption(self->options, SVSTRING("start-over"), NULL, QBReminderEditorPaneNPVRStartOver, self);
        if (QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, SvValueGetString(fullID))) {
            QBBasicPaneAddOption(self->options, SVSTRING("continue"), NULL, QBReminderEditorPaneNPVRStartOver, self);
        }

        ret = true;
    }

fini:
    SVTESTRELEASE(fullID);
    return ret;
}

SvLocal void
QBReminderEditorPaneInitStartOver(QBReminderEditorPane self, bool started, bool ended)
{
    if (!self->event) {
        return;
    }

    SvPlayerTask playerTask = QBTVLogicGetPlayerTask(self->appGlobals->tvLogic);
    if (!playerTask) {
        return;
    }

    bool startOver = false;
    if (SvObjectIsInstanceOf((SvObject) self->event, Innov8onEPGEvent_getType())) {
        startOver = SvEPGEventCanBeRemotelyStartedOver(self->event);
    }

    if (started && !ended && startOver) {
        SvString eventName = QBEventUtilsGetTitleFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, self->event);
        QBBasicPaneAddOptionWithSubcaption(self->options, SVSTRING("start-over"), NULL, eventName, QBReminderEditorPaneStartOver, self);
    }
}

SvLocal void
QBReminderEditorPaneInitStartCatchup(QBReminderEditorPane self, const bool started, const bool ended)
{
    bool startCatchup = false;

    if (!self->event) {
        return;
    }
    SvValue fullID = SvEPGEventCreateID(self->event);
    if (!fullID || !SvValueIsString(fullID)) {
        SVTESTRELEASE(fullID);
        return;
    }

    if (SvObjectIsInstanceOf((SvObject) self->event, Innov8onEPGEvent_getType())) {
        startCatchup = SvEPGEventIsCatchupEnabled(self->event);
    }

    if (started && ended && startCatchup) {
        QBBasicPaneAddOption(self->options, SVSTRING("start-catchup"), NULL, QBReminderEditorPaneStartCatchup, self);
        if (QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, SvValueGetString(fullID))) {
            QBBasicPaneAddOption(self->options, SVSTRING("continue"), NULL, QBReminderEditorPaneStartCatchup, self);
        }
    }
    SVRELEASE(fullID);
}

SvLocal void QBReminderEditorPaneOnWatchInCutv(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    self->playFromTheBeginning = id && SvStringEqualToCString(id, "watch in CUTV");
    self->playbackType = QBReminderEditorPanePlaybackType_WatchInCUTV;

    QBReminderEditorPaneStartPlaybackIfPCAllowed(self);
}

SvLocal void QBReminderEditorPaneOnContinuousInCutv(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBReminderEditorPane self = self_;

    self->playFromTheBeginning = true;
    self->playbackType = QBReminderEditorPanePlaybackType_ContinuousInCUTV;

    QBReminderEditorPaneStartPlaybackIfPCAllowed(self);
}

/*
 * Typical stuff for pane management
 */

SvLocal void QBReminderEditorPane__dtor__(void *ptr)
{
    QBReminderEditorPane self = ptr;
    if (self->resolver) {
        QBPVRConflictResolverSetFinishCallback(self->resolver, NULL, NULL);
        SVRELEASE(self->resolver);
    }
    QBContextMenuSetInputEventHandler(self->contextMenu, NULL, NULL);
    SVRELEASE(self->options);
    SVRELEASE(self->channel);
    SVTESTRELEASE(self->event);
    if (self->errorLabel)
        svWidgetDestroy(self->errorLabel);
    SVTESTRELEASE(self->currentNPvrRec);
    SVTESTRELEASE(self->extendedInfo);
    if (self->parentalPopup) {
        QBDialogBreak(self->parentalPopup);
    }
    SVTESTRELEASE(self->seriesPane);
    SVTESTRELEASE(self->recordOnePane);
    SVTESTRELEASE(self->epgManager);
    SVTESTRELEASE(self->searchResults);
}

void QBReminderEditorPaneSetPosition(QBReminderEditorPane self, SvString id, bool immediately)
{
    QBBasicPaneSetPosition(self->options, id, immediately);
}

void QBReminderEditorPaneSetPositionByIndex(QBReminderEditorPane self, size_t idx, bool immediately)
{
    QBBasicPaneSetPositionByIndex(self->options, idx, immediately);
}

SvLocal void QBReminderEditorPaneShow(SvGenericObject self_)
{
    QBReminderEditorPane self = (QBReminderEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, show);
}

SvLocal void QBReminderEditorPaneHide(SvGenericObject self_, bool immediately)
{
    QBReminderEditorPane self = (QBReminderEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, hide, immediately);
}

SvLocal void QBReminderEditorPaneSetActive(SvGenericObject self_)
{
    QBReminderEditorPane self = (QBReminderEditorPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->options, setActive);
}

SvLocal bool QBReminderEditorPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvType QBReminderEditorPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReminderEditorPane__dtor__
    };
    static SvType type = NULL;

    static const struct QBContextMenuPane_ methods = {
        .show             = QBReminderEditorPaneShow,
        .hide             = QBReminderEditorPaneHide,
        .setActive        = QBReminderEditorPaneSetActive,
        .handleInputEvent = QBReminderEditorPaneHandleInputEvent
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReminderEditorPane",
                            sizeof(struct QBReminderEditorPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvLocal void QBReminderEditorPaneSearchKeyTyped(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key, EPGManSearchAgentMode mode)
{
    QBReminderEditorPane self = ptr;

    if (key->type == QBOSKKeyType_enter && SvStringLength(input)) {
        SVTESTRELEASE(self->searchResults);
        self->searchResults = QBSearchResultsContextCreate(self->appGlobals);

        SvArray agents = SvArrayCreate(NULL);


        SvString playlistId = SvTVChannelGetAttribute(self->channel, SVSTRING("TV")) ? SVSTRING("TVChannels") : SVSTRING("RadioChannels");
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvObject view = QBPlaylistManagerGetById(playlists, playlistId);
        SvObject agent = QBInitLogicCreateSearchAgent(self->appGlobals->initLogic,
                                                      mode, 0, view);

        SvArrayAddObject(agents, agent);
        SVRELEASE(agent);

        SvObject search = QBSearchEngineCreate(agents, input, NULL);
        SVRELEASE(agents);

        if (SvArrayCount(QBSearchEngineGetKeywords(search))) {
            QBSearchResultsSetDataSource(self->searchResults, search);
            QBSearchResultsContextExecute(self->searchResults, self->contextMenu);
        }

        SVRELEASE(search);
    }
}

SvLocal void QBReminderEditorPaneSearchKeyTypedTitle(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBReminderEditorPaneSearchKeyTyped(ptr, pane, input, layout, key, EPGManSearchAgentMode_titleOnly);
}

SvLocal void QBReminderEditorPaneSearchKeyTypedTitleAndDescription(void *ptr, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBReminderEditorPaneSearchKeyTyped(ptr, pane, input, layout, key, EPGManSearchAgentMode_normal);
}

/*
 * Initialization
 */

SvLocal void QBReminderEditorPaneInit(QBReminderEditorPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event, SvString itemNamesFilename)
{
    if (self->callbacks.init && self->callbackData) {
        self->callbacks.init(self->callbackData, (SvGenericObject) self, appGlobals, ctxMenu, widgetName, channel, event, itemNamesFilename);
    } else {
        QBReminderEditorPaneInitImplementation(self, appGlobals, ctxMenu, widgetName, channel, event, itemNamesFilename);
    }
}

SvLocal void
QBReminderEditorPaneInitNPvrOptions(QBReminderEditorPane self, SvEPGEvent event, bool pastEvent)
{
    bool canBeRecorded = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, canScheduleEventRecording, event);
    if (canBeRecorded) {
        if (pastEvent) {
            QBBasicPaneAddOption(self->options, SVSTRING("record with nPVR (past)"), NULL, QBReminderEditorPaneOnRecordWithNPvr, self);
        } else {
            QBBasicPaneAddOption(self->options, SVSTRING("record with nPVR"), NULL, QBReminderEditorPaneOnRecordWithNPvr, self);
            if (event && SvObjectIsInstanceOf((SvObject) event, Innov8onEPGEvent_getType())) {
                Innov8onEPGEvent innov8onEvent = (Innov8onEPGEvent) event;
                if (innov8onEvent->seriesID) {
                    QBBasicPaneAddOption(self->options, SVSTRING("record series with nPVR"), NULL, QBReminderEditorPaneShowNpvrSeriesPane, self);
                }
            }
            QBBasicPaneAddOption(self->options, SVSTRING("record keyword with nPVR"), NULL, QBReminderEditorPaneShowKeywordPane, self);
        }
        return;
    }

    QBnPVRRecordingType npvrRecType = QBNPvrLogicGetRecordingTypeForEvent(self->appGlobals->nPVRProvider, event);
    if (npvrRecType == QBnPVRRecordingType_none) {
        return;
    }

    QBnPVRRecording npvrRec = QBNPvrUtilsGetRecordingOfTypeForEvent(self->appGlobals, npvrRecType, event);
    SVTESTRELEASE(self->currentNPvrRec);
    self->currentNPvrRec = SVRETAIN(npvrRec);

    if (npvrRecType == QBnPVRRecordingType_manual) {
        QBBasicPaneAddOption(self->options, SVSTRING("delete nPVR recording"), NULL, QBReminderEditorPaneOnDeleteNPvrRecording, self);
    } else if (npvrRecType == QBnPVRRecordingType_keyword) {
        QBBasicPaneAddOption(self->options, SVSTRING("stop keyword nPVR recording"), NULL, QBReminderEditorPaneOnStopKeywordNPvrRecording, self);
    } else if (npvrRecType == QBnPVRRecordingType_series) {
        QBBasicPaneAddOption(self->options, SVSTRING("stop series nPVR recording"), NULL, QBReminderEditorPaneOnStopSeriesNPvrRecording, self);
    }
}

SvLocal void
QBReminderEditorPaneInit_(QBReminderEditorPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event, SvString itemNamesFilename)
{
    self->appGlobals = appGlobals;
    self->contextMenu = ctxMenu;
    self->settingsCtx = svSettingsSaveContext();
    self->channel = SVRETAIN(channel);
    self->event = SVTESTRETAIN(event);
    self->options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(self->options, appGlobals->res, appGlobals->scheduler, appGlobals->textRenderer, ctxMenu, 1, SVSTRING("BasicPane"));
    QBBasicPaneLoadOptionsFromFile(self->options, itemNamesFilename);
    SVTESTRELEASE(itemNamesFilename);
}

void QBReminderEditorPaneInitImplementation(QBReminderEditorPane self, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event, SvString itemNamesFilename)
{
    QBReminderEditorPaneInit_(self, appGlobals, ctxMenu, widgetName, channel, event, itemNamesFilename);
    const time_t now = SvTimeNow();

    if (event) {
        const bool started = (now >= event->startTime);
        const bool ended = (now > event->endTime);
        QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
        if (!ended) {
            if (!started) {
                QBBasicPaneAddOption(self->options, SVSTRING("switch to channel"), NULL, QBReminderEditorPaneOnPlay, self);

                if (QBReminderIsScheduledFor(appGlobals->reminder, channel, event)) {
                    QBBasicPaneAddOption(self->options, SVSTRING("remove reminder"), NULL, QBReminderEditorPaneOnCancelRemindMe, self);
                } else if (SvTVChannelGetAttribute(channel, SVSTRING("TV"))) {
                    SvValue attribute = (SvValue) SvTVChannelGetAttribute(channel, SVSTRING("TV"));
                    if (attribute && SvValueGetBoolean(attribute)) {
                        /* Option "remind me" is enabled only for TV channels, because "Reminders" menu entry is
                         * only on TV menu. Adding radio reminders to TV menu would be strange.
                         * It was decided not to add "Reminders" to Radio menu at this moment. */
                        QBBasicPaneAddOption(self->options, SVSTRING("remind me"), NULL, QBReminderEditorPaneOnRemindMe, self);
                    }
                }
            } else {
                QBBasicPaneAddOption(self->options, SVSTRING("play"), NULL, QBReminderEditorPaneOnPlay, self);
            }

            if (!SvTVChannelGetAttribute(channel, SVSTRING("adaptive_streaming")) &&
                (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present)) {
                SvArray recordings = QBRecordingUtilsCreateNotCompletedRecordingsForEvent(appGlobals->pvrProvider, event);
                if (QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Any)) {
                    if (QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Channel)) {
                        QBBasicPaneAddOption(self->options, SVSTRING("stop channel recording"), NULL, QBReminderEditorPaneOnRemove, self);
                        QBBasicPaneAddOption(self->options, SVSTRING("record event"), NULL, QBReminderEditorPaneOnRecord, self);
                    } else {
                        QBBasicPaneAddOption(self->options, SVSTRING("stop recording"), NULL, QBReminderEditorPaneOnRemove, self);
                    }
                } else if (!QBRecordingUtilsGetRecordingOfType(recordings, QBRecordingUtilsRecType_Multiple)) {
                    QBBasicPaneAddOption(self->options, SVSTRING("record event"), NULL, QBReminderEditorPaneOnRecord, self);
                    QBBasicPaneAddOption(self->options, SVSTRING("record channel"), NULL, QBReminderEditorPaneOnRecordChannel, self);
                }
                SVRELEASE(recordings);
            }
        } else {
            QBBasicPaneAddOption(self->options, SVSTRING("switch to channel"), NULL, QBReminderEditorPaneOnPlay, self);
        }

        if (!SvTVChannelGetAttribute(channel, SVSTRING("adaptive_streaming")) &&
            (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present)) {
            bool isKeyword = false, isRepeated = false;
            SvArray dirs = QBPVRProviderCreateActiveDirectoriesListForEvent(self->appGlobals->pvrProvider, self->event);

            SvIterator it = SvArrayIterator(dirs);
            QBPVRDirectory dir = NULL;
            while ((dir = (QBPVRDirectory) SvIteratorGetNext(&it))) {
                if (dir->type == QBPVRDirectoryType_keyword) {
                    isKeyword = true;
                } else if (dir->type == QBPVRDirectoryType_repeated) {
                    isRepeated = true;
                }
            }
            SVRELEASE(dirs);

            if (isKeyword) {
                QBBasicPaneAddOption(self->options, SVSTRING("stop recording always"), NULL, QBReminderEditorPaneOnStopRecordAlways, self);
            } else {
                QBBasicPaneAddOption(self->options, SVSTRING("record always"), NULL, QBReminderEditorPaneOnRecordAlways, self);
            }

            if (isRepeated) {
                QBBasicPaneAddOption(self->options, SVSTRING("stop recording repeatedly"), NULL, QBReminderEditorPaneOnStopRecordRepeatedly, self);
            } else {
                QBBasicPaneAddOption(self->options, SVSTRING("record repeatedly"), NULL, QBReminderEditorPaneOnRecordRepeatedly, self);
            }
        }

        time_t cutvStart, cutvEnd;
        bool pastEvent = false;
        bool cutvEnabled = false;

        QBReminderEditorPaneInitStartCatchup(self, started, ended);

        if (self->appGlobals->cutvManager && CutvManagerIsEventAvailable(self->appGlobals->cutvManager, event, now)) {
            if (CutvManagerGetEventAvailableTimes(self->appGlobals->cutvManager, event, &cutvStart, &cutvEnd)) {
                pastEvent = cutvEnd > now && cutvStart <= now;
            }

            //cutvEnabled = true;
            cutvEnabled = event->startTime < now; // AMERELES Sacar CatchUp a programas posteriores al actual
        }

        if ((!ended || pastEvent) && self->appGlobals->nPVRProvider &&
            (!started || QBNPvrLogicAreOngoingRecsAllowed())) {
            QBReminderEditorPaneInitNPvrOptions(self, event, pastEvent);
        }

        if (QBNPvrLogicShouldNPVRBeUsedForStartOver()) {
            // Add start-over option ("play from the beginning") based on NPVR
            QBReminderEditorPaneInitNPVRStartOver(self, started, ended);
        } else {
            // Add start-over option ("play from the beginning") based on service information (from manifest file)
            QBReminderEditorPaneInitStartOver(self, started, ended);
        }

        if (cutvEnabled) {
            QBBasicPaneAddOption(self->options, SVSTRING("watch in CUTV"), NULL, QBReminderEditorPaneOnWatchInCutv, self);

            SvEPGIterator it = SvEPGManagerCreateIterator(self->epgManager, self->event->channelID, self->event->startTime);
            SvEPGIteratorGetNextEvent(it);
            SvEPGEvent next = SvEPGIteratorGetNextEvent(it);
            if (CutvManagerIsEventAvailable(self->appGlobals->cutvManager, next, now - event->endTime + event->startTime)) {
                QBBasicPaneAddOption(self->options, SVSTRING("continuous playback CUTV"), NULL, QBReminderEditorPaneOnContinuousInCutv, self);
            }
            SVRELEASE(it);

            SvValue fullID = SvEPGEventCreateID(self->event);
            if (fullID && SvValueIsString(fullID) && QBBookmarkManagerGetLastPositionBookmark(self->appGlobals->bookmarkManager, SvValueGetString(fullID))) {
                QBBasicPaneAddOption(self->options, SVSTRING("watch in CUTV continue"), NULL, QBReminderEditorPaneOnWatchInCutv, self);
            }
            SVTESTRELEASE(fullID);
        }

        QBBasicPaneAddOption(self->options, SVSTRING("show similar"), NULL, QBReminderEditorPaneOnShowSimilar, self);
        QBBasicPaneAddOptionWithOSK(self->options, SVSTRING("search by name"), NULL, SVSTRING("OSKPane"), QBReminderEditorPaneSearchKeyTypedTitle, self);
        QBBasicPaneAddOptionWithOSK(self->options, SVSTRING("Search by description"), NULL, SVSTRING("OSKPane"), QBReminderEditorPaneSearchKeyTypedTitleAndDescription, self);

        const SvEPGDataPluginConfig *configDVB = self->appGlobals->epgPlugin.DVB.data ?  SvInvokeInterface(SvEPGDataPlugin, self->appGlobals->epgPlugin.DVB.data, getConfig) : NULL;
        const SvEPGDataPluginConfig *configIP = self->appGlobals->epgPlugin.IP.data ? SvInvokeInterface(SvEPGDataPlugin, self->appGlobals->epgPlugin.IP.data, getConfig) : NULL;

        if ((configDVB && configDVB->searchByCategoryEnabled) || (configIP && configIP->searchByCategoryEnabled)) {
            SvString playlists = NULL;
            const char *categories = NULL;
            bool recognizedAttribute = false;
            SvValue attribute = (SvValue) SvTVChannelGetAttribute(self->channel, SVSTRING("Radio"));
            if (attribute && SvValueGetBoolean(attribute)) {
                playlists = SVSTRING("RadioChannels");
                categories = "SearchByCategoryRadioPane.json";
                recognizedAttribute = true;
            } else {
                attribute = (SvValue) SvTVChannelGetAttribute(self->channel, SVSTRING("TV"));
                if (attribute && SvValueGetBoolean(attribute)) {
                    playlists = SVSTRING("TVChannels");
                    categories = "SearchByCategoryPane.json";
                    recognizedAttribute = true;
                }
            }
            if (recognizedAttribute) {
                QBServiceRegistry registry = QBServiceRegistryGetInstance();
                QBPlaylistManager playlistsManager = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
                SvEPGView channelList = (SvEPGView) QBPlaylistManagerGetById(playlistsManager, playlists);
                QBBasicPane searchByCategoryPane = QBSearchByCategoryPaneCreateFromSettings("SearchByCategoryPane.settings", categories, self->appGlobals, ctxMenu, SVSTRING("SearchByCategoryPane"), 2, channelList);
                QBBasicPaneAddOptionWithSubpane(self->options, SVSTRING("search by category"), NULL, (SvGenericObject) searchByCategoryPane);
                SVRELEASE(searchByCategoryPane);
            }
        }

        QBBasicPaneAddOption(self->options, SVSTRING("languages"), NULL, QBReminderEditorPaneOnShowLanguages, self);

        SVTESTRELEASE(self->extendedInfo);
        self->extendedInfo = (QBExtendedInfoPane) SvTypeAllocateInstance(QBExtendedInfoPane_getType(), NULL);
        SvArray events = SvArrayCreate(NULL);
        SvArrayAddObject(events, (SvGenericObject) event);
        QBExtendedInfoPaneInit(self->extendedInfo, appGlobals, ctxMenu, SVSTRING("ExtendedInfo"), 2, events);
        SVRELEASE(events);
        QBBasicPaneAddOptionWithSubpane(self->options, SVSTRING("more info"), NULL, (SvGenericObject) self->extendedInfo);
    } else if (channel) {
        QBBasicPaneAddOption(self->options, SVSTRING("switch to channel"), NULL, QBReminderEditorPaneOnPlay, self);
        QBBasicPaneAddOption(self->options, SVSTRING("languages"), NULL, QBReminderEditorPaneOnShowLanguages, self);
    }
}

QBReminderEditorPane QBReminderEditorPaneCreateFromSettings(const char *settings, const char *itemNamesFilename, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event)
{
    svSettingsPushComponent(settings);
    QBReminderEditorPane self = (QBReminderEditorPane) SvTypeAllocateInstance(QBReminderEditorPane_getType(), NULL);
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    QBGUILogicRemiderEditorPaneRegisterCallbacks(appGlobals->guiLogic, (SvGenericObject) self);
    QBReminderEditorPaneInit(self, appGlobals, ctxMenu, widgetName, channel, event, SvStringCreate(itemNamesFilename, NULL));
    svSettingsPopComponent();

    if (!QBBasicPaneGetOptionsCount(self->options)) {
        SVRELEASE(self);
        self = NULL;
    }

    return self;
}

QBReminderEditorPane QBReminderEditorPaneCreateAndPushWithNPvrFromSettings(const char *settings, AppGlobals appGlobals, QBContextMenu ctxMenu, SvString widgetName, SvTVChannel channel, SvEPGEvent event, QBReminderEditorPaneType type)
{
    svSettingsPushComponent(settings);
    QBReminderEditorPane self = (QBReminderEditorPane) SvTypeAllocateInstance(QBReminderEditorPane_getType(), NULL);
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    QBReminderEditorPaneInit_(self, appGlobals, ctxMenu, widgetName, channel, event, NULL);
    svSettingsPopComponent();

    QBContextMenuPushPane(self->contextMenu, (SvGenericObject) self);

    if (type == QBReminderEditorPaneType_Keyword) {
        if (QBNPvrLogicAreRecordingsCustomizable(self->appGlobals)) {
            QBReminderEditorPaneShowKeywordPaneWithLevel(self, 1);
        } else {
            QBReminderEditorPaneCommitRecordKeywordWithLevel(self, 0);
        }
    } else if (type == QBReminderEditorPaneType_Series) {
        if (QBNPvrLogicAreRecordingsCustomizable(self->appGlobals)) {
            QBReminderEditorPaneShowSeriesPaneWithLevel(self, 1);
        } else {
            QBReminderEditorPaneCommitRecordSeriesWithLevel(self, 0);
        }
    }

    return self;
}

void
QBReminderEditorPaneSetCallbacks(SvGenericObject self_, void *callbackData, QBReminderEditorPaneCallbacks callbacks)
{
    QBReminderEditorPane self = (QBReminderEditorPane) self_;

    self->callbackData = callbackData;
    self->callbacks = *callbacks;
}
