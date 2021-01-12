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

#include <Logic/TVLogic.h>
#include <Logic/AudioTrackLogic.h>
#include <libintl.h>
#include <SvFoundation/SvURL.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>
#include <QBApplicationController.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <QBPlatformHAL/QBPlatformAudioOutput.h>
#include <SvPlayerKit/SvContentResources.h>
#include <QBTuner.h>
#include <QBConf.h>
#include <sv_tuner.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <main.h>
#include <QBSubsManager.h>
#include <QBPlayerControllers/utils/content.h>
#include <safeFiles.h>
#include <Windows/newtv.h>
#include <Windows/newtvguide.h>
#include <Widgets/audioDelayDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Services/clocks.h>
#include <tunerReserver.h>
#include <Services/core/GlobalPlayer.h>
#include <QBPlayerControllers/QBChannelPlayerController.h>
#include <QBPlayerControllers/QBDVBChannelPlayerController.h>
#include <QBPlayerControllers/QBMWChannelPlayerController.h>
#include <QBPlayerControllers/ConaxQBPlayerControllerPlugin.h>
#include <Utils/appType.h>
#include <safeFiles.h>
#include <Utils/QBPlaylistUtils.h>
#include <Widgets/NewTVOSD.h>
#include <Widgets/QBSimpleDialog.h>
#include <settings.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBListModelListener.h>
#include <fibers/c/fibers.h>
#include <error_codes.h>
#include <Logic/VideoOutputLogic.h>
#include <Logic/SubtitleTrackLogic.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <QBOSD/QBOSDMain.h>
#include <Widgets/OSD/QBOSDTSRecording.h>
#include <QBPVRProvider.h>
#include <QBStaticStorage.h>
#include <QBAppKit/QBServiceRegistry.h>

#define QBTimeLimitEnable
#define QBTimeThreshold (50 * 1000)
#include <QBTimeLimit.h>

#define CURRENT_PLAYLIST_FILE "/etc/vod/app/currentPlaylist"
#define CONFIG_PVRLIMIT_CURRENTLY_WATCHED_ONLY_NAME "USB_PVR_LIMITS.CURRENTLY_WATCHED_ONLY"

typedef enum {
    MediumType_UNKNOWN,
    MediumType_IP,
    MediumType_DVB
} MediumType;

struct QBTVLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBPlaylistManager playlists;

    SvValue channelID;
    // cached information about most recently played channel for use when it has been removed
    SvTVChannel channelInfo;
    SvURL channelSourceURL;
    SvValue alternativeID;
    SvString listID;
    SvPlayerTask playerTask;
    bool stopped;

    bool controlGranted;
    SvArray stopSources; //list of services which had stopped TV
    struct {
        SvString playlistID;
        SvTVChannel channel;
    } scheduledPlay; // used when control is not granted, to remember how to do 'resume'

    QBTunerResv* resv;

    bool enabled;
    bool instantTimeshift;
    bool audioDelaySetupMode;
    bool didPlayerFatalErrorOccur; // true if the dialog indicating network error is being shown

    /// If flag is true and there is an ongoing recording, it is forbidden to watch any channel that is not being recorded.
    bool allowWatchingOnlyRecordedChannels;

    struct {
        SvFiber fiber;
        SvFiberTimer timer;
        SvFiber checkBoundariesFiber;      // fiber used to check if timeshift buffer is full
        SvFiberTimer checkBoundariesTimer;
    } timeshift;

    SvObject controller;
    SvArray controllers;

    int hideDelay;

    bool displayLocked;
    DisplayLockMode displayLockMode;

    enum {
        FRONTPANEL_DISABLED = 0,
        FRONTPANEL_DISPLAY_CLOCK,
        FRONTPANEL_DISPLAY_CHANNEL_NAME
    } frontPanelMode;

    SvObject frontPanelClock;

    struct QBTVLogicMetaDataManagers_ metaDataManagers;
};

void QBTVLogicControlGranted(SvObject self_, SvObject controller);
void QBTVLogicControlRevoked(SvObject self_, SvObject controller);

SvLocal bool QBTVLogicAnyLocalWindowIsVisible(QBTVLogic self);
SvLocal void QBTVLogicHideAllLocalWindows(QBTVLogic self);

SvLocal void QBTVLogicInputWatcherTick(SvObject self_, QBInputWatcher inputWatcher)
{
    QBTVLogic self = (QBTVLogic) self_;
    if (QBTVLogicAnyLocalWindowIsVisible(self))
        QBTVLogicHideAllLocalWindows(self);
}

SvLocal void QBTVLogicDestroy(void *self_)
{
    QBTVLogic self = self_;
    SVTESTRELEASE(self->channelID);
    SVTESTRELEASE(self->channelInfo);
    SVTESTRELEASE(self->channelSourceURL);
    SVTESTRELEASE(self->listID);
    SVTESTRELEASE(self->playerTask);
    SVRELEASE(self->playlists);

    SVTESTRELEASE(self->scheduledPlay.playlistID);
    SVTESTRELEASE(self->scheduledPlay.channel);

    if (self->timeshift.fiber) {
        SvFiberDestroy(self->timeshift.fiber);
    }
    if (self->timeshift.checkBoundariesFiber) {
        SvFiberDestroy(self->timeshift.checkBoundariesFiber);
    }

    SVTESTRELEASE(self->controllers);
    SVTESTRELEASE(self->stopSources);
    SVTESTRELEASE(self->frontPanelClock);

    QBTVLogicMetaDataManagers managers = &self->metaDataManagers;
    SVTESTRELEASE(managers->closedCaptionSubsManager);
    SVTESTRELEASE(managers->scte27SubsManager);
    SVTESTRELEASE(managers->teletextSubsManager);
    SVTESTRELEASE(managers->teletextReceiver);
    SVTESTRELEASE(managers->teletextLineReceiver);
    SVTESTRELEASE(managers->textSubsManager);
    SVTESTRELEASE(managers->dvbSubsManager);
}


SvLocal void QBTVLogicCheckController(QBTVLogic self, bool allowChangeControllerState);

SvLocal SvObject QBTVLogicFindControllerForChannel(QBTVLogic self, SvTVChannel channel)
{
    SvIterator it = SvArrayIterator(self->controllers);
    SvObject controller;
    while ((controller = SvIteratorGetNext(&it))) {
        if (SvInvokeInterface(QBChannelPlayerController, controller, supports, channel))
            return controller;
    }
    return NULL;
}

bool QBTVLogicChannelCanPlayByIndex(QBTVLogic self, int idx)
{
    SvObject playlist = QBTVLogicGetPlaylist(self);

    if (!playlist)
        return false;

    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, idx);
    if (!channel)
        return false;
    return QBTVLogicChannelCanPlay(self, channel);
}

bool QBTVLogicChannelCanPlay(QBTVLogic self, SvTVChannel channel)
{
    return true;
}

SvLocal MediumType QBTVLogicGetControllerMediumType(QBTVLogic self)
{
    MediumType type = MediumType_UNKNOWN;

    if (!self->controller)
        goto fini;

    if (SvObjectIsInstanceOf(self->controller, QBDVBChannelPlayerController_getType()))
        type = MediumType_DVB;
    else if (SvObjectIsInstanceOf(self->controller, QBMWChannelPlayerController_getType()))
        type = MediumType_IP;

fini:
    return type;
}

SvLocal void QBTVLogicDisplayChannelNumberOnFrontPanel(QBTVLogic self);

static void QBTVLogicConfigureFrontpanelContent(QBTVLogic self, const char * mode)
{
    if ((!mode) || (strcasecmp(mode, "CHANNELNAME") == 0)) {
        self->frontPanelMode = FRONTPANEL_DISPLAY_CHANNEL_NAME;
        if (self->frontPanelClock) {
            QBFrontPanelClockStop(self->frontPanelClock);
            SVRELEASE(self->frontPanelClock);
            self->frontPanelClock = NULL;
        }
        QBTVLogicDisplayChannelNumberOnFrontPanel(self);
    } else if (strcasecmp(mode, "CLOCK") == 0) {
        self->frontPanelMode = FRONTPANEL_DISPLAY_CLOCK;
        if (!self->frontPanelClock) {
            self->frontPanelClock = QBFrontPanelClockCreate(SvSchedulerGet());
            QBFrontPanelClockStart(self->frontPanelClock);
        }
    } else {
        self->frontPanelMode = FRONTPANEL_DISABLED;
        if (self->frontPanelClock) {
            QBFrontPanelClockStop(self->frontPanelClock);
            SVRELEASE(self->frontPanelClock);
            self->frontPanelClock = NULL;
        }
        QBPlatformShowOnFrontPanel(" ");
    }
}

SvLocal void
QBTVLogicConfigChanged(SvObject self_, const char *key, const char *value)
{
    if (!value)
        return;

    QBTVLogic self = (QBTVLogic) self_;
    if (strcmp(key, "INSTANTTIMESHIFT") == 0) {
        const char* instantTimeshift = QBConfigGet("INSTANTTIMESHIFT");
        if (instantTimeshift)
            self->instantTimeshift = !strcmp(instantTimeshift, "1");

        QBAppPVRDiskState pvrState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
        if ((pvrState == QBPVRDiskState_ts_present || pvrState == QBPVRDiskState_pvr_ts_present) && self->instantTimeshift)
            SvTVContextEnableTimeshift(self->appGlobals->newTV);
    }

    if (strcmp(key, "FRONTPANELCONTENT") == 0) {
        QBTVLogicConfigureFrontpanelContent(self, value);
    }

    if (!strcmp(key, "HIDEINFORMATION")) {
        if (value)
            self->hideDelay = atoi(value);
        else
            self->hideDelay = 5;
    }

    if (!strcmp(key, CONFIG_PVRLIMIT_CURRENTLY_WATCHED_ONLY_NAME)) {
        self->allowWatchingOnlyRecordedChannels = (strcmp(value, "enabled") == 0);
    }
}

SvLocal void QBTVLogicAppStateChanged(SvObject self_, uint64_t attrs)
{
    QBTVLogic self = (QBTVLogic) self_;
    QBAppPVRDiskState pvrState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
    const char* instantTimeshift = QBConfigGet("INSTANTTIMESHIFT");
    if (pvrState == QBPVRDiskState_ts_present || pvrState == QBPVRDiskState_pvr_ts_present) {
        if (instantTimeshift)
            self->instantTimeshift = !strcmp(instantTimeshift, "1");

        if (self->instantTimeshift)
            SvTVContextEnableTimeshift(self->appGlobals->newTV);
    }
}

SvLocal void QBTVLogicTryPlayChannel(QBTVLogic self)
{
    if (!self->listID || SvTVContextGetCurrentChannel(self->appGlobals->newTV))
        return;

    QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), self->listID);
    if (!cursor || !cursor->channelID)
        return;

    SvGenericObject list = QBPlaylistManagerGetById(self->playlists, cursor->playlistID);
    if (list) {
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, list, getByID, cursor->channelID);
        if (channel) {
            int idx = SvInvokeInterface(SvEPGChannelView, list, getChannelIndex, channel);
            if (idx >= 0) {
                QBTVLogicPlayChannelByIndexFromPlaylist(self, cursor->playlistID, idx, SVSTRING("TV"));
            }
        }
    }
}

SvLocal void QBTVLogicChannelsChanged(SvObject self_, SvObject dataSource, size_t first, size_t idx)
{
    QBTVLogic self = (QBTVLogic) self_;

    if (!self->channelID) {
        QBWindowContext ctx = QBApplicationControllerGetCurrentContext(self->appGlobals->controller);
        if (SvObjectEquals((SvObject) ctx, (SvObject) self->appGlobals->newTV))
            QBTVLogicTryPlayChannel(self);
        return;
    }

    if (self->stopped)
        return;

    SvObject view = QBPlaylistManagerGetById(self->playlists, SVSTRING("TVChannels"));
    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, view, getByID, self->channelID);
    if (!channel) {
        view = QBPlaylistManagerGetById(self->playlists, SVSTRING("RadioChannels"));
        channel = SvInvokeInterface(SvEPGChannelView, view, getByID, self->channelID);
    }

    if (!channel) {
        //fallback
        view = QBPlaylistManagerGetById(self->playlists, self->listID);
        if (SvInvokeInterface(QBListModel, view, getLength) > 0) {
            QBTVLogicPlayChannelByIndex(self, 0, SVSTRING("replacement"));
            return;
        }
        view = QBPlaylistManagerGetById(self->playlists, SVSTRING("TVChannels"));
        if (SvInvokeInterface(QBListModel, view, getLength) > 0) {
            QBTVLogicPlayChannelByIndexFromPlaylist(self, SVSTRING("TVChannels"), 0, SVSTRING("replacement"));
        } else {
            QBTVLogicStopPlaying(self, SVSTRING("replacement"));
        }
        return;
    }

    SvTVChannel alternative = channel->needsAlternative && channel->alternativeID ? SvInvokeInterface(SvEPGChannelView, view, getByID, channel->alternativeID) : NULL;

    if ((!channel->needsAlternative || !channel->alternativeID) && self->alternativeID) {
        QBTVLogicPlayChannel(self, channel, SVSTRING("replacement"));
        return;
    } else if (alternative && !SvObjectEquals((SvObject) self->alternativeID, (SvObject) channel->alternativeID)) {
        QBTVLogicPlayChannel(self, channel, SVSTRING("replacement"));
        return;
    }

    size_t channelIdx = SvInvokeInterface(SvEPGChannelView, view, getChannelIndex, channel);
    size_t alternativeIdx = first + idx;  // outside

    if (alternative)
        alternativeIdx = SvInvokeInterface(SvEPGChannelView, view, getChannelIndex, alternative);

    if (((channelIdx >= first) && (channelIdx < first + idx)) ||
        ((alternativeIdx >= first) && (alternativeIdx < first + idx))) {
        QBTVLogicPlayChannel(self, channel, SVSTRING("freq change"));
    }
}

SvLocal void QBTVLogicChannelsRemoved(SvObject self_, SvObject playlist, size_t first, size_t idx)
{
    QBTVLogic self = (QBTVLogic) self_;
    if (!self->channelID || self->stopped)
        return;

    SvTVChannel channel = NULL;
    SvString playlistID = QBPlaylistManagerGetPlaylistId(self->playlists, playlist);

    if (!SvObjectEquals((SvObject) self->listID, (SvObject) playlistID)) {
        QBTVLogicChannelsChanged(self_, playlist, first, idx);
        return;
    }

    channel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, self->channelID);
    if (channel) {
        QBTVLogicChannelsChanged(self_, playlist, first, idx);
        return;
    }

    if (SvInvokeInterface(SvEPGChannelView, playlist, getCount) > 0) {
        // switching to first channel of current playlist
        QBTVLogicPlayChannelByIndex(self, 0, playlistID);
        return;
    } else {
        // switching to first channel of general playlist of the same kind as removed channel (TV/Radio)
        SvValue valueTV = (SvValue) SvTVChannelGetAttribute(self->channelInfo, SVSTRING("TV"));
        SvString fallbackPlaylistKind = (valueTV && SvValueGetBoolean(valueTV)) ? SVSTRING("TVChannels") : SVSTRING("RadioChannels");

        playlist = QBPlaylistManagerGetById(self->playlists, fallbackPlaylistKind);

        if (SvInvokeInterface(SvEPGChannelView, playlist, getCount) == 0) {
            if (SvStringEqualToCString(fallbackPlaylistKind, "RadioChannels")) {
                fallbackPlaylistKind = SVSTRING("TVChannels");
                playlist = QBPlaylistManagerGetById(self->playlists, fallbackPlaylistKind);
            } else {
                QBTVLogicPlayChannel(self, NULL, fallbackPlaylistKind);
                return;
            }
        }

        channel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, 0);
        QBTVLogicPlayChannelFromPlaylist(self, fallbackPlaylistKind, channel, SVSTRING("channelRemoved"));
    }
}

SvLocal void QBTVLogicPlayerControllerStateChanged(SvObject self_, SvObject controller_)
{
    QBTVLogic self = (QBTVLogic) self_;
    QBTVLogicCheckController(self, false);
}

SvLocal SvTVChannelMediumType QBTVLogicGetCurrentChannelMediumType(QBTVLogic self)
{
    SvTVChannelMediumType type = SvTVChannelMediumType_unknown;

    QBWindowContext ctx = self->appGlobals->newTV;
    if (!ctx)
        goto fini;

    SvTVChannel channel = SvTVContextGetCurrentChannel(ctx);

    if (!channel)
        goto fini;

    type = SvTVChannelGetMediumType(channel);

fini:
    return type;
}

SvLocal void
QBTVLogicRecordingAdded(SvObject self_,
                        QBPVRRecording rec)
{
}

SvLocal void
QBTVLogicRecordingRemoved(SvObject self_,
                          QBPVRRecording rec)
{
}

SvLocal void
QBTVLogicRecordingChanged(SvObject self_,
                          QBPVRRecording rec,
                          QBPVRRecording oldRecording)
{
    QBTVLogic self = (QBTVLogic) self_;
    // Check if scheduled recording stole our tuner
    QBTVLogicCheckController(self, true);
}

SvLocal void
QBTVLogicRecordingRestricted(SvObject self_,
                             QBPVRRecording rec)
{
}

SvLocal void
QBTVLogicQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
QBTVLogicDirectoryAdded(SvObject self, QBPVRDirectory dir)
{
}

SvType
QBTVLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTVLogicDestroy
    };
    static SvType type = NULL;
    static const struct QBConfigListener_t configMethods = {
        .changed = QBTVLogicConfigChanged
    };
    static const struct QBAppStateServiceListener_t appstateMethods = {
        .stateChanged = QBTVLogicAppStateChanged
    };

    static const struct QBListModelListener2_ listMethods = {
        .itemsAdded     = QBTVLogicChannelsChanged,
        .itemsRemoved   = QBTVLogicChannelsRemoved,
        .itemsChanged   = QBTVLogicChannelsChanged,
        .itemsReordered = QBTVLogicChannelsChanged,
        .itemsReplaced  = QBTVLogicChannelsChanged
    };

    static const struct QBInputWatcherListener_t inputMethods = {
        .tick = QBTVLogicInputWatcherTick
    };

    static const struct QBGlobalPlayerControllerListener_t globalPlayerMethods = {
        .controlGranted = QBTVLogicControlGranted,
        .controlRevoked = QBTVLogicControlRevoked,
    };

    static const struct QBChannelPlayerControllerListener_ playerControllerListener = {
        .stateChanged = QBTVLogicPlayerControllerStateChanged,
    };

    static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
        .recordingAdded      = QBTVLogicRecordingAdded,
        .recordingChanged    = QBTVLogicRecordingChanged,
        .recordingRemoved    = QBTVLogicRecordingRemoved,
        .recordingRestricted = QBTVLogicRecordingRestricted,
        .quotaChanged        = QBTVLogicQuotaChanged,
        .directoryAdded      = QBTVLogicDirectoryAdded
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTVLogic", sizeof(struct QBTVLogic_t),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &configMethods,
                            QBListModelListener2_getInterface(), &listMethods,
                            QBInputWatcherListener_getInterface(), &inputMethods,
                            QBChannelPlayerControllerListener_getInterface(), &playerControllerListener,
                            QBGlobalPlayerControllerListener_getInterface(), &globalPlayerMethods,
                            QBAppStateServiceListener_getInterface(), &appstateMethods,
                            QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal SvArray QBTVLogicCreateRecodingsIdsArray(QBTVLogic self)
{
    QBPVRProviderGetRecordingsParams params = {
        .includeRecordingsInDirectories = true,
        .inStates[QBPVRRecordingState_active] = true,
    };

    SvArray recordings = QBPVRProviderGetRecordings(self->appGlobals->pvrProvider,
                                                    params);

    if (!recordings || SvArrayCount(recordings) == 0) {
        SVTESTRELEASE(recordings);
        return NULL;
    }

    SvArray recordingsIds = SvArrayCreateWithCapacity(SvArrayCount(recordings), NULL);
    SvIterator it = SvArrayIterator(recordings);
    QBPVRRecording rec = NULL;
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        SvArrayAddObject(recordingsIds, (SvObject) (rec->channelId));
    }
    SVRELEASE(recordings);
    return recordingsIds;
}

/**
 * Check information from controller and show popup if needed
 *
 * @param[in] self TV Logic instance
 * @param[in] allowChangeControllerState indicates if it is allowed to change controller state (if called from controller callback, changes can result in infinite loop)
 */
SvLocal void QBTVLogicCheckController(QBTVLogic self, bool allowChangeControllerState)
{
    bool showNotEntitled = false;
    bool showRecordingsInfo = false;
    bool ipChannelError = false;
    MediumType controllerMediumType = QBTVLogicGetControllerMediumType(self);
    if (controllerMediumType == MediumType_IP) {
        QBMWChannelPlayerController* controller = (QBMWChannelPlayerController *) self->controller;
        QBMWProductUsageControllerRequestState state = QBMWChannelPlayerControllerGetLastErrorState(controller);

        bool needsToPlay = QBMWChannelPlayerControllerWantsToStart(controller);
        if (!needsToPlay)
            goto fini;

        if (state == QBMWProductUsageControllerRequestState_useDenied) {
            showNotEntitled = true;
        } else if (state != QBMWProductUsageControllerRequestState_useEntitled &&
                   state != QBMWProductUsageControllerRequestState_using &&
                   state != QBMWProductUsageControllerRequestState_ending &&
                   state != QBMWProductUsageControllerRequestState_useEnded) {
            ipChannelError = true;
        }
    } else if (controllerMediumType == MediumType_DVB) {
        QBDVBChannelPlayerController* controller = (QBDVBChannelPlayerController *) self->controller;
        bool needsTuner = QBDVBChannelPlayerControllerWantsToStart(controller);
        bool hasTuner = (QBDVBChannelPlayerControllerGetTunerNum(controller) != -1);

        showRecordingsInfo = needsTuner && !hasTuner;
    }

    if (self->allowWatchingOnlyRecordedChannels && self->channelID && !self->stopped) {
        SvString currentChannelId = SvValueGetString(self->channelID);
        SvArray recordings = QBTVLogicCreateRecodingsIdsArray(self);
        bool needStop = (recordings && !SvArrayContainsObject(recordings, (SvObject) currentChannelId));
        SVTESTRELEASE(recordings);

        showRecordingsInfo = showRecordingsInfo || needStop;

        if (self->controller) {
            if (needStop) {
                SvInvokeInterface(QBChannelPlayerController, self->controller, stop);
            } else {
                // If we stop playback and all recordings are canceled we need to start it again
                SvInvokeInterface(QBChannelPlayerController, self->controller, start, self->channelInfo);
            }
        }
    }

fini:;
    QBWindowContext ctx = self->appGlobals->newTV;

    if (ipChannelError) {
        int channelErrCode = 0;
        QBTVLogicHandleError(self, NULL, &channelErrCode);
        SvTVContextSetChannelErrorCode(ctx, channelErrCode);
    }

    if (showNotEntitled)
        QBTVNotEntitledShow(ctx);
    else
        QBTVNotEntitledHide(ctx);

    if (showRecordingsInfo)
        QBTVTunerInfoShow(ctx);
    else
        QBTVTunerInfoHide(ctx);
}

int QBTVLogicGetTunerNum(QBTVLogic self)
{
    if (QBTVLogicGetControllerMediumType(self) != MediumType_DVB)
        return -1;

    QBDVBChannelPlayerController* controller = (QBDVBChannelPlayerController *) self->controller;
    return QBDVBChannelPlayerControllerGetTunerNum(controller);
}

SvLocal void
QBTVLogicTimeshiftStep(void *self_)
{
    QBTVLogic self = self_;
    QBWindowContext ctx = self->appGlobals->newTV;

    SvFiberDeactivate(self->timeshift.fiber);
    SvFiberEventDeactivate(self->timeshift.timer);

    QBAppPVRDiskState state = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);

    if ((state == QBPVRDiskState_ts_present || state == QBPVRDiskState_pvr_ts_present) && self->instantTimeshift)
        SvTVContextEnableTimeshift(ctx);
}

/**
 * Fiber checking if timeshift buffer is full. Playback should be restored if it's paused and start of
 * timeshift buffer has exceeded current position in stream.
 *
 * param[in] self_      TV Logic instance
 */
SvLocal void
QBTVLogicTimeshiftCheckBoundariesStep(void *self_)
{
    QBTVLogic self = (QBTVLogic) self_;

    SvFiberDeactivate(self->timeshift.checkBoundariesFiber);
    SvFiberEventDeactivate(self->timeshift.checkBoundariesTimer);

    if (self->playerTask == NULL) {
        return;
    }

    SvPlayerTaskState taskState = SvPlayerTaskGetState(self->playerTask);
    if (taskState.currentSpeed == 0.0) {
        if (taskState.timeshift.range_start > taskState.currentPosition) {
            SvPlayerTaskEnableTimeshiftPlayback(self->playerTask, true);
            SvPlayerTaskPlay(self->playerTask, 1.0, -1.0, NULL);
        }
    }

    SvFiberTimerActivateAfter(self->timeshift.checkBoundariesTimer, SvTimeConstruct(1,0));
}

SvLocal void
QBTVLogicScheduleTimeshift(QBTVLogic self)
{
    if (self->enabled) {
        SvFiberTimerActivateAfter(self->timeshift.timer, SvTimeConstruct(20, 0));
    }
}

QBTVLogic
QBTVLogicNew(AppGlobals appGlobals)
{
    QBTVLogic self = (QBTVLogic) SvTypeAllocateInstance(QBTVLogic_getType(), NULL);
    self->appGlobals = appGlobals;
    self->audioDelaySetupMode = false;

    self->timeshift.fiber = SvFiberCreate(appGlobals->scheduler, NULL, "TVLogic.timeshift", QBTVLogicTimeshiftStep, self);
    self->timeshift.timer = SvFiberTimerCreate(self->timeshift.fiber);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->playlists = SVRETAIN(playlists);

    self->timeshift.checkBoundariesFiber = SvFiberCreate(appGlobals->scheduler, NULL, "TVLogic.timeshiftCheckBoundaries", QBTVLogicTimeshiftCheckBoundariesStep, self);
    self->timeshift.checkBoundariesTimer = SvFiberTimerCreate(self->timeshift.checkBoundariesFiber);

    const char *currentOnlyValue = QBConfigGet(CONFIG_PVRLIMIT_CURRENTLY_WATCHED_ONLY_NAME);
    self->allowWatchingOnlyRecordedChannels = currentOnlyValue && (strcmp(currentOnlyValue, "enabled") == 0);
    const char* instantTimeshift = QBConfigGet("INSTANTTIMESHIFT");
    if (instantTimeshift) {
        self->instantTimeshift = !strcmp(instantTimeshift, "1");
        QBConfigAddListener((SvObject) self, "INSTANTTIMESHIFT");
    }

    QBConfigAddListener((SvObject) self, "FRONTPANELCONTENT");
    QBConfigAddListener((SvObject) self, "HIDEINFORMATION");
    QBConfigAddListener((SvObject) self, CONFIG_PVRLIMIT_CURRENTLY_WATCHED_ONLY_NAME);

    const char * frontPanelContent = QBConfigGet("FRONTPANELCONTENT");
    QBTVLogicConfigureFrontpanelContent(self, frontPanelContent);

    char *buffer = NULL;
    QBFileToBuffer(CURRENT_PLAYLIST_FILE, &buffer);
    if (buffer && *buffer) {
        SvString listID = SvStringCreate(buffer, NULL);
        if (QBPlaylistManagerGetById(self->playlists, listID)) {
            self->listID = SVRETAIN(listID);
        }
        SVRELEASE(listID);
    }

    free(buffer);

    if (!self->listID) {
        self->listID = SVSTRING("TVChannels");
    }

    SvObject tvChannels = QBPlaylistManagerGetById(self->playlists, self->listID);
    SvInvokeInterface(QBDataModel, tvChannels, addListener, (SvObject) self, NULL);

    self->controllers = SvArrayCreate(NULL);
    self->stopSources = SvArrayCreate(NULL);
    //Order is important
    if (QBAppTypeIsDVB() || QBAppTypeIsHybrid()) {
        QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
        SvObject controller = (SvObject) QBDVBChannelPlayerControllerCreate(self->appGlobals->scheduler, self->appGlobals->catManager, tunerReserver);
        SvArrayAddObject(self->controllers, controller);
        SVRELEASE(controller);
    }

    if (QBAppTypeIsIP() || QBAppTypeIsHybrid()) {
        SvObject controller = (SvObject) QBMWChannelPlayerControllerCreate(self->appGlobals->scheduler, self->appGlobals->qbMWProductUsageController, self->appGlobals->casPopupManager);
        SvArrayAddObject(self->controllers, controller);

        SvObject conaxPlugin = (SvObject) ConaxQBPlayerControllerPluginCreate(self->appGlobals->qbMWProductUsageController, NULL);
        if (conaxPlugin) {
            QBMWChannelPlayerControllerAddPlugin((QBMWChannelPlayerController *) controller, conaxPlugin);
            SVRELEASE(conaxPlugin);
        }

        SVRELEASE(controller);
    }
    const char *hideDelayStr = QBConfigGet("HIDEINFORMATION");
    if (!hideDelayStr)
        self->hideDelay = 5;
    else
        self->hideDelay = atoi(hideDelayStr);

    QBGlobalPlayerControllerPushController(self->appGlobals->playerController, (SvObject) self);

    return self;
}

SvLocal void
QBTVLogicHideAllLocalWindows(QBTVLogic self)
{
    QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);

    QBWindowContext ctx = self->appGlobals->newTV;
    QBTVMiniFavoriteListHide(ctx);
    QBTVMiniChannelListHide(ctx);
    QBTVLangMenuHide(ctx);
    QBTVMiniTVGuideHide(ctx);
    QBTVExtendedInfoHide(ctx);
    QBTVMiniRecordingListHide(ctx);
    QBTVRecordingOptionsHide(ctx);
    QBTVTunerInfoHide(ctx);
    QBTVErrorHide(ctx);
    QBTVNotEntitledHide(ctx);
    QBTVTeletextHide(ctx);
    QBTVRedButtonOverlayHide(ctx);

    QBTVOSDLock(ctx, false, false);
}

SvLocal bool
QBTVLogicAnyLocalWindowIsVisible(QBTVLogic self)
{
    QBWindowContext ctx = self->appGlobals->newTV;
    if (QBTVMiniFavoriteListIsVisible(ctx))
        return true;
    if (QBTVMiniChannelListIsVisible(ctx))
        return true;
    if (QBTVLangMenuIsVisible(ctx))
        return true;
    if (QBTVMiniTVGuideIsVisible(ctx))
        return true;
    if (QBTVExtendedInfoIsVisible(ctx))
        return true;
    if (QBTVMiniRecordingListIsVisible(ctx))
        return true;
    if (QBTVTeletextIsVisible(ctx))
        return true;
    if (QBTVRedButtonOverlayIsVisible(ctx))
        return true;
    return false;
}

bool
QBTVLogicHandleInputEvent(QBTVLogic self, SvInputEvent ev)
{
    if (!self->enabled)
        return false;
    QBWindowContext ctx = self->appGlobals->newTV;

    QBTimeLimitBegin(timeTVLogic, QBTimeThreshold);

    switch (ev->ch) {
        case 'i':
        case QBKEY_INFO:
            if (QBTVOSDIsVisible(ctx)) {
                if (QBTVExtendedInfoIsVisible(ctx)) {
                    QBTVOSDLock(ctx, false, false);

                    QBTVExtendedInfoHide(ctx);
                    QBTVOSDHide(ctx, true, false);
                    QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
                } else if (!self->displayLocked) {
                    QBTVLogicHideAllLocalWindows(self);
                    if (QBTVExtendedInfoShow(ctx)) {
                        QBTVOSDLock(ctx, true, true);
                    } else {
                        QBTVOSDHide(ctx, false, true);
                    };
                }
            } else {
                QBTVRedButtonOverlayHide(ctx);
                QBTVOSDLock(ctx, false, false);
                QBTVOSDShow(ctx, false, true);
                if (QBTVRedButtonOverlayIsAvailable(ctx)) {
                    QBTVOSDNotificationShow(ctx, false);
                }
            }

            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.INFO");
            break;
        case 'o':
        case QBKEY_FUNCTION:
            if (QBTVIsSideMenuShown(ctx)) {
                QBTVHideSideMenu(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else if (!self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVShowSideMenu(ctx, SVSTRING(""), QBTVSideMenuType_Full);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }

            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.FUNCTION");
            break;
        case 'l':
        case QBKEY_LIST:
            if (QBTVMiniFavoriteListIsVisible(ctx)) {
                QBTVMiniFavoriteListHide(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else if (!self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVMiniFavoriteListShow(ctx);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.LIST");
            break;
        case QBKEY_ENTER:
            if (QBTVIsSideMenuShown(ctx)) {
                QBTVHideSideMenu(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            }
            if (QBTVMiniChannelListIsVisible(ctx)) {
                QBTVMiniChannelListHide(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else if (!self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVMiniChannelListShow(ctx);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.ENTER");
            break;
        case 'a':
        case QBKEY_AUDIOTRACK:
            if (QBTVLangMenuIsVisible(ctx)) {
                QBTVLangMenuHide(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else {
                QBTVLogicHideAllLocalWindows(self);
                QBTVLangMenuShow(ctx, QBLangMenuSubMenu_Audio);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.AUDIOTRACK");
            break;

        case 's':
        case QBKEY_LANGUAGE:
            if (QBTVLangMenuIsVisible(ctx)) {
                QBTVLangMenuHide(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else if (!self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVLangMenuShow(ctx, QBLangMenuSubMenu_Subtitles);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.AUDIOTRACK");
            break;

        case 'b':
        case QBKEY_BACK:
            if (QBTVLogicAnyLocalWindowIsVisible(self)) {
                QBTVLogicHideAllLocalWindows(self);
            } else {
                QBTVOSDNotificationHide(ctx, true);
                if (QBTVOSDIsVisible(ctx))
                    QBTVOSDHide(ctx, true, true);
                else
                    return false;
            }

            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.BACK");
            break;
        case 't':
        case QBKEY_TELETEXT:
            if (QBTVTeletextIsVisible(ctx)) {
                if (QBTVTeletextIsTransparent(ctx)) {
                    QBTVTeletextHide(ctx);
                    if (QBTVRedButtonOverlayIsAvailable(ctx)) {
                        QBTVOSDNotificationShow(ctx, false);
                    }
                } else {
                    QBTVTeletextSetTransparent(ctx, true);
                }
            } else if (!self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVOSDNotificationHide(ctx, true);
                QBTVTeletextSetTransparent(ctx, false);
                QBTVTeletextShow(ctx);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.TELETEXT");
            break;

        case QBKEY_LEFT:
        {
            QBInputEvent tmpev = ev->fullInfo;
            tmpev.u.key.code = QBKEY_VOLDN;

            if (QBTVLogicAnyLocalWindowIsVisible(self))
                svAppPostEvent(self->appGlobals->res, NULL, svInputEventCreate(QBKEY_PGUP));
            else if (self->appGlobals->cecService && (QBCecServiceInputEventHandler(self->appGlobals->cecService, &tmpev)))
                /* if CEC is enabled then consume */;
            else if (!self->displayLocked) {
                QBVolumeDown(self->appGlobals->volume);
                QBVolumeUp(self->appGlobals->volume);
                QBVolumeDown(self->appGlobals->volume);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.LEFT");
        }
        break;

        case QBKEY_RIGHT:
        {
            QBInputEvent tmpev = ev->fullInfo;
            tmpev.u.key.code = QBKEY_VOLUP;

            if (QBTVLogicAnyLocalWindowIsVisible(self))
                svAppPostEvent(self->appGlobals->res, NULL, svInputEventCreate(QBKEY_PGDN));
            else if (self->appGlobals->cecService && (QBCecServiceInputEventHandler(self->appGlobals->cecService, &tmpev)))
                /* if CEC is enabled then consume */;
            else if (!self->displayLocked) {
                QBVolumeUp(self->appGlobals->volume);
                QBVolumeDown(self->appGlobals->volume);
                QBVolumeUp(self->appGlobals->volume);

            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.RIGHT");
        }
        break;

        case QBKEY_STOP:
        {
            MediumType currentMedium = QBTVLogicGetControllerMediumType(self);
            if (((currentMedium != MediumType_IP) && (QBTVLogicGetTunerNum(self) < 0)) || SvTVContextIsLive(ctx)) {
                if (QBTVMiniRecordingListIsVisible(ctx)) {
                    QBTVMiniRecordingListHide(ctx);
                    QBTVRecordingOptionsHide(ctx);
                    QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
                } else {
                    if (self->appGlobals->pvrAgent && QBActiveArrayCount(QBPVRAgentGetOnGoingRecordingsList(self->appGlobals->pvrAgent)) > 0) {
                        QBTVLogicHideAllLocalWindows(self);
                        QBTVMiniRecordingListShow(ctx);
                        QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
                    }
                }
            } else {
                SvTVContextEnterLive(ctx);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.STOP");
            break;
        }
        case QBKEY_REC:
        {
            QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(self->appGlobals->appState);
            if (diskState == QBPVRDiskState_pvr_present || diskState == QBPVRDiskState_pvr_ts_present) {
                SvTVContextStartRecording(ctx);
            } else if (self->appGlobals->nPVRProvider && SvTVContextCanNPvrRecordingBeStarted(ctx) &&
                       !QBTVIsSideMenuShown(ctx) && !self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVShowSideMenu(ctx, SVSTRING(""), QBTVSideMenuType_Full);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.REC");
            break;
        }
        case QBKEY_UP:
        case QBKEY_CHUP:
            if (!self->displayLocked || (self->displayLockMode == DisplayLockMode_EnableUpDown)) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVLogicPlayChannelByOffset(self, 1, SVSTRING("KEY_UP"));
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.UP");
            break;
        case QBKEY_DOWN:
        case QBKEY_CHDN:
            if (!self->displayLocked || (self->displayLockMode == DisplayLockMode_EnableUpDown)) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVLogicPlayChannelByOffset(self, -1, SVSTRING("KEY_DOWN"));
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.DOWN");
            break;
        case QBKEY_ASPECT:
        {
            VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoOutputLogic"));
            VideoOutputLogicToggleVideoContentMode(videoOutputLogic);
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.ASPECT");
            break;
        }
        case QBKEY_RED:
            if (QBTVRedButtonOverlayIsAvailable(ctx) && !self->displayLocked) {
                QBTVLogicHideAllLocalWindows(self);
                QBTVOSDHide(ctx, true, true);
                QBTVOSDNotificationHide(ctx, true);
                QBTVRedButtonOverlayShow(ctx);
            }
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.RED");
            break;
        default:
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.OTHER");
            return false;
    }
    if (!QBTVLogicAnyLocalWindowIsVisible(self))
        QBTVLogicCheckController(self, true);

    QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.checkController");
    return true;
}

SvObject QBTVLogicGetPlaylist(QBTVLogic self)
{
    SvObject playlist = NULL;
    if (self->listID)
        playlist = QBPlaylistManagerGetById(self->playlists, self->listID);
    else
        playlist = QBPlaylistManagerGetCurrent(self->playlists);
    return playlist;
}

SvTVChannel QBTVLogicGetChannel(QBTVLogic self)
{
    if (!self->channelID)
        return NULL;

    SvObject view = QBTVLogicGetPlaylist(self);
    return view ? SvInvokeInterface(SvEPGChannelView, view, getByID, self->channelID) : NULL;
}

SvTVChannel QBTVLogicGetChannelByNumber(QBTVLogic self, int number, SvObject playlist)
{
    if (!playlist) {
        return NULL;
    }

    int size = SvInvokeInterface(SvEPGChannelView, playlist, getCount);
    if (!size) {
        return NULL;
    }

    return SvInvokeInterface(SvEPGChannelView, playlist, getByNumber, number, SvEPGViewNumberRoundingMethod_closest);
}

SvLocal void QBTVLogicDisplayChannelNumberOnFrontPanel(QBTVLogic self)
{
    SvTVChannel channel = QBTVLogicGetChannel(self);
    char buf[33];

    QBTimeLimitBegin(timeDisplayPannel, QBTimeThreshold);

    QBFrontPanelCapabilities caps = {
        .visibleCharacters = 0,
    };
    int ret = QBPlatformGetFrontPanelCapabilities(&caps);

    QBTimeLimitEnd_(timeDisplayPannel, "SvTVContext.tvLogic.FrontPanelGetCapabilities");

    if (!ret) {
        if (caps.visibleCharacters < 5) {
            SvObject currentPlaylist = QBTVLogicGetPlaylist(self);
            int maxDigits = QBPlaylistUtilsGetMaxDigits(currentPlaylist);
            snprintf(buf, caps.visibleCharacters + 1, "%0*d", maxDigits, channel ?
                     QBPlaylistManagerGetNumberOfChannel(self->playlists, channel) : 0);
            snprintf(buf + maxDigits, caps.visibleCharacters - maxDigits + 1, "    ");
        } else {
            snprintf(buf, 33, "%s", channel ? SvStringCString(channel->name) : "No Channel");
        }
        QBPlatformShowOnFrontPanel(buf);
    } else {
        SvLogError("QBPlatformGetFrontPanelCapabilities");
    }

    QBTimeLimitEnd_(timeDisplayPannel, "SvTVContext.tvLogic.ShowOnFrontPanel");
}

void QBTVLogicPlayChannel(QBTVLogic self, SvTVChannel channel, SvString source)
{
    if (!self->listID) {
        return;
    }
    QBTVLogicPlayChannelFromPlaylist(self, self->listID, channel, source);
}

void QBTVLogicPlayChannelByOffset(QBTVLogic self, int offset, SvString source)
{
    SvObject view = QBPlaylistManagerGetCurrent(self->playlists);
    if (!view)
        return;
    int count = SvInvokeInterface(SvEPGChannelView, view, getCount);
    if (!count)
        return;

    QBTimeLimitBegin(timeTVLogic, QBTimeThreshold);

    QBWindowContext ctx = self->appGlobals->newTV;
    SvTVChannel currentChannel = SvTVContextGetCurrentChannel(ctx);

    int channum = 0;
    if (currentChannel)
        channum = SvInvokeInterface(SvEPGChannelView, view, getChannelIndex, currentChannel);

    QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playChannelByOffset.getIndex");

    if (channum < 0)
        channum = 0;

    int currentNum = channum;

    channum = (channum + offset + count) % count;

    SvArray recordings = NULL;
    if (self->allowWatchingOnlyRecordedChannels) {
        recordings = QBTVLogicCreateRecodingsIdsArray(self);
    }

    int dir = (offset > 0) ? 1 : -1;
    do {
        SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, view, getByIndex, channum);

        SvObject controller = QBTVLogicFindControllerForChannel(self, channel);

        bool canStart = (controller && SvInvokeInterface(QBChannelPlayerController, controller, canStart, channel));

        if (canStart && self->allowWatchingOnlyRecordedChannels && recordings) {
            SvValue newChannelID = channel ? SvDBObjectGetID((SvDBObject) channel) : NULL;
            SvString newChannelIDs = newChannelID ? SvValueGetString(newChannelID) : NULL;
            canStart = (newChannelIDs && SvArrayContainsObject(recordings, (SvObject) newChannelIDs));
        }

        if (canStart) {
            break;
        }

        channum = (channum + dir + count) % count;
    } while (channum != currentNum);

    SVTESTRELEASE(recordings);

    QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playChannelByOffset.canStart");

    QBTVLogicPlayChannelByIndex(self, channum, source);
    QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playChannelByOffset.play");
}

void QBTVLogicCheckBackground(QBTVLogic self, QBWindowContext ctx, SvTVChannel channel)
{
    QBTVBackgroundHide(ctx);
    if (ContentIsRadioChannel(channel)) {
        QBTVBackgroundShow(ctx);
    }
}

void QBTVLogicPlayChannelByIndex(QBTVLogic self, int idx, SvString source)
{
    SvObject playlist = QBTVLogicGetPlaylist(self);

    if (!playlist)
        return;

    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, playlist, getByIndex, idx);
    QBTVLogicPlayChannel(self, channel, source);
}

void QBTVLogicPlayChannelFromPlaylist(QBTVLogic self, SvString playlistID, SvTVChannel channel, SvString source)
{
    SVTESTRELEASE(self->scheduledPlay.playlistID);
    self->scheduledPlay.playlistID = NULL;
    SVTESTRELEASE(self->scheduledPlay.channel);
    self->scheduledPlay.channel = NULL;

    if (!self->controlGranted || SvArrayCount(self->stopSources)) {
        self->scheduledPlay.playlistID = SVTESTRETAIN(playlistID);
        self->scheduledPlay.channel = SVTESTRETAIN(channel);
        return;
    }

    QBTimeLimitBegin(timeTVLogic, QBTimeThreshold);

    SvValue newChannelID = channel ? SvDBObjectGetID((SvDBObject) channel) : NULL;
    SvURL newChannelURL = channel ? channel->sourceURL : NULL;
    SvValue alternativeID = NULL;
    SvObject view = QBPlaylistManagerGetById(self->playlists, SVSTRING("TVChannels"));
    SvTVChannel alternative = NULL;

    QBAppStateServiceRemoveListener(self->appGlobals->appState, (SvObject) self);
    QBAppStateServiceAddListener(self->appGlobals->appState, (SvObject) self);

    if (channel && channel->needsAlternative && channel->alternativeID) {
        alternative = SvInvokeInterface(SvEPGChannelView, view, getByID, channel->alternativeID);
        if (alternative)
            alternativeID = channel->alternativeID;
    }

    bool channelChangedID = !SvObjectEquals((SvObject) self->channelID, (SvObject) newChannelID);
    bool channelChangedURL = !SvObjectEquals((SvObject) self->channelSourceURL, (SvObject) newChannelURL);
    bool channelChanged = channelChangedID || channelChangedURL;

    bool listChanged = !SvObjectEquals((SvObject) self->listID, (SvObject) playlistID);
    bool alternativeChanged = !SvObjectEquals((SvObject) self->alternativeID, (SvObject) alternativeID);
    bool playbackChanged = channelChanged || alternativeChanged || self->stopped;
    //If we were stopped then no matter what, something has changed
    bool anythingChanged = playbackChanged || listChanged;

    if (listChanged && playlistID) {
        QBStaticStorageWriteFile(SVSTRING(CURRENT_PLAYLIST_FILE), playlistID);
    }

    SvObject currentList = NULL;
    if (listChanged) {
        currentList = (self->listID) ? QBPlaylistManagerGetById(self->playlists, self->listID) : NULL;
        if (currentList)
            SvInvokeInterface(QBDataModel, currentList, removeListener, (SvObject) self, NULL);
    }

    // playlistID might be == self->listID
    SVTESTRETAIN(playlistID);
    SVTESTRELEASE(self->channelID);
    SVTESTRELEASE(self->channelInfo);
    SVTESTRELEASE(self->channelSourceURL);
    SVTESTRELEASE(self->listID);
    SVTESTRELEASE(self->alternativeID);
    self->channelID = SVTESTRETAIN(newChannelID);
    self->channelInfo = SVTESTRETAIN(channel);
    self->channelSourceURL = SVTESTRETAIN(newChannelURL);
    self->listID = playlistID;
    self->alternativeID = SVTESTRETAIN(alternativeID);

    if (listChanged) {
        currentList = (self->listID) ? QBPlaylistManagerGetById(self->playlists, self->listID) : NULL;
        if (currentList)
            SvInvokeInterface(QBDataModel, currentList, addListener, (SvObject) self, NULL);
    }

    QBWindowContext ctx = self->appGlobals->newTV;

    if (playbackChanged && self->controller) {
        SvInvokeInterface(QBChannelPlayerController, self->controller, stop);
        SvInvokeInterface(QBChannelPlayerController, self->controller, removeListener, (SvObject) self);
        self->controller = NULL;

        QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.stop");
    }

    if (anythingChanged || self->stopped) {
        SvTVContextSetChannel(ctx, channel, playlistID);
        if (!channel)
            QBPlatformShowOnFrontPanel(gettext("No channel"));
        self->stopped = false;

        QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.frontPanelNoChannel");
    }

    if (source && SvStringEqualToCString(source, "MiniFavoriteList")) {
        QBTVOSDShow(ctx, false, true);
    }

    if (!channelChanged && source && SvStringEqualToCString(source, "MiniChannelList")) {
        if(QBTVMiniChannelListIsVisible(ctx)) {
            QBTVMiniChannelListHide(ctx);
        }
    }

    if (channelChanged) {
        AudioTrackLogic audioTrackLogic =
            (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
        AudioTrackLogicSetPreferredAudioTrack(audioTrackLogic, NULL);

        SubtitleTrackLogic subtitleTrackLogic =
            (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
        SubtitleTrackLogicSetPreferredLang(subtitleTrackLogic, NULL);

        QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.track");
    }

    if (playbackChanged && self->channelID) {
        QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), playlistID);
        QBPlaylistCursorsUpdate(self->appGlobals->playlistCursors, cursor, channel);

        QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.cursor");

        self->controller = QBTVLogicFindControllerForChannel(self, channel);
        if (!self->controller) {
            SvLogError("%s() : Can't find controller for channel `%s'", __func__,
                       channel && channel->name ? SvStringCString(channel->name) : NULL);
            return;
        }

        SvInvokeInterface(QBChannelPlayerController, self->controller, addListener, (SvObject) self);
        SvInvokeInterface(QBChannelPlayerController, self->controller, setPlayer, (SvObject) ctx);
        QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.setPlayer");

        SvInvokeInterface(QBChannelPlayerController, self->controller, start, alternative ? alternative : channel);
        QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.startPlayer");
    }

    if (self->instantTimeshift && self->controlGranted)
        QBTVLogicScheduleTimeshift(self);

    if (anythingChanged) {
        self->didPlayerFatalErrorOccur = false;
        QBTVErrorHide(ctx);
        QBTVFatalErrorHide(ctx);
        QBTVOSDShow(ctx, false, true);
        QBTVOSDLock(ctx, false, true);
        QBTVExtendedInfoHide(ctx);
        if (self->frontPanelMode == FRONTPANEL_DISPLAY_CHANNEL_NAME) {
            QBTVLogicDisplayChannelNumberOnFrontPanel(self);
            QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.frontPanelChannel");
        }
    }

    QBTVLogicCheckController(self, true);
    QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.checkControler");

    QBTVLogicCheckBackground(self, ctx, channel);
    QBTimeLimitEnd_(timeTVLogic, "SvTVContext.tvLogic.playFromPlaylist.checkBackground");
}

void QBTVLogicPlayChannelByCursor(QBTVLogic self, QBPlaylistCursor cursor, SvString source)
{
    QBWindowContext ctx = self->appGlobals->newTV;
    bool listNotChanged = SvObjectEquals((SvObject) cursor->playlistID, (SvObject) self->listID);
    if (listNotChanged && source && SvStringEqualToCString(source, "MiniFavoriteList")) {
        if (QBTVMiniFavoriteListIsVisible(ctx)) {
            QBTVMiniFavoriteListHide(ctx);
        }
    }

    if (cursor->channelID) {
        int idx = 0;
        SvObject list = QBPlaylistManagerGetById(self->playlists, cursor->playlistID);
        if (list) {
            SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, list, getByID, cursor->channelID);
            if (channel) {
                idx = SvInvokeInterface(SvEPGChannelView, list, getChannelIndex, channel);
            }
        }
        if (idx < 0) {
            idx = 0;
        }
        QBTVLogicPlayChannelByIndexFromPlaylist(self, cursor->playlistID, idx, source);
    } else {
        QBTVLogicPlayChannelByIndexFromPlaylist(self, cursor->playlistID, 0, source);
    }
}

void QBTVLogicPlayChannelByIndexFromPlaylist(QBTVLogic self, SvString playlistID, int idx, SvString source)
{
    if (!playlistID) {
        SvLogWarning("%s (%d): NULL playlistID passed", __func__, __LINE__);
        return;
    }

    SvObject playlist = QBPlaylistManagerGetById(self->playlists, playlistID);
    if (playlist && SvInvokeInterface(QBListModel, playlist, getLength) == 0)
        playlist = NULL;

    SvTVChannel channel = NULL;
    if (playlist) {
        channel = (SvTVChannel) SvInvokeInterface(QBListModel, playlist, getObject, idx);
    }

    QBTVLogicPlayChannelFromPlaylist(self, playlistID, channel, source);
}

void QBTVLogicPlayChannelByNumber(QBTVLogic self, int selection, SvString source)
{
    if (!self->listID) {
        return;
    }
    SvObject list;
    SvTVChannel ch = NULL;
    list = QBPlaylistManagerGetById(self->playlists, self->listID);
    SvEPGViewNumberRoundingMethod roundingMethod = SvEPGViewNumberRoundingMethod_exact;
    if ((ch = SvInvokeInterface(SvEPGChannelView, list, getByNumber, selection, roundingMethod))) {
        QBTVLogicPlayChannelFromPlaylist(self, self->listID, ch, source);
        return;
    }
}

SvLocal
bool QBTVLogicResumePlayingFromScheduled(QBTVLogic self, SvString source)
{
    SvString playlistID = self->scheduledPlay.playlistID;
    SvTVChannel channel = self->scheduledPlay.channel;

    SvObject playlist = QBPlaylistManagerGetById(self->playlists, playlistID);
    if (!playlist)
        return false;

    SvValue channelID = SvDBObjectGetID((SvDBObject) channel);
    SvTVChannel cmpChannel = SvInvokeInterface(SvEPGChannelView, playlist, getByID, channelID);
    if (!cmpChannel) {
        /// take any channel from the same playlist
        if (SvInvokeInterface(SvEPGChannelView, playlist, getCount) == 0)
            return false;
        channel = (SvTVChannel) SvInvokeInterface(QBListModel, playlist, getObject, 0);
    }

    SVRETAIN(playlistID);
    SVRETAIN(channel);

    QBTVLogicPlayChannelFromPlaylist(self, playlistID, channel, source);

    SVRELEASE(playlistID);
    SVRELEASE(channel);
    return true;
}

void QBTVLogicResumePlaying(QBTVLogic self, SvString source)
{
    int idx = SvArrayIndexOfObject(self->stopSources, (SvObject) source);
    if (idx != -1) {
        SvArrayRemoveObjectAtIndex(self->stopSources, idx);
    }

    if (SvArrayCount(self->stopSources)) {
        return;
    }

    if (self->scheduledPlay.channel) {
        if (QBTVLogicResumePlayingFromScheduled(self, source))
            return;
    }

    SvObject playlist = NULL;

    if (self->listID) {
        playlist = QBPlaylistManagerGetById(self->playlists, self->listID);
    } else {
        playlist = QBPlaylistManagerGetCurrent(self->playlists);
        if (playlist) {
            self->listID = QBPlaylistManagerGetPlaylistId(self->playlists, playlist);
            SVRETAIN(self->listID);
        }
    }

    if (playlist && SvInvokeInterface(SvEPGChannelView, playlist, getCount) > 0) {
        QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), self->listID);
        QBTVLogicPlayChannelByCursor(self, cursor, source);
    } else {
        QBPlaylistCursor cursor = QBPlaylistCursorsGet(self->appGlobals->playlistCursors, SVSTRING("TV"), SVSTRING("TVChannels"));
        QBTVLogicPlayChannelByCursor(self->appGlobals->tvLogic, cursor, SVSTRING("TV"));
    }

    switch (self->frontPanelMode) {
        case FRONTPANEL_DISPLAY_CHANNEL_NAME:
            QBTVLogicDisplayChannelNumberOnFrontPanel(self);
            break;
        case FRONTPANEL_DISPLAY_CLOCK:
            if (self->frontPanelClock) {
                QBFrontPanelClockStart(self->frontPanelClock);
            }
            break;
        case FRONTPANEL_DISABLED:
            QBPlatformShowOnFrontPanel(" ");
            break;
    }
}

void QBTVLogicPlayPreviousChannel(QBTVLogic self)
{
    SvString playlist = SvTVContextGetPreviousList(self->appGlobals->newTV);
    SvTVChannel channel = SvTVContextGetPreviousChannel(self->appGlobals->newTV);

    SvObject previousList = QBPlaylistManagerGetById(self->playlists, playlist);
    if (previousList && SvInvokeInterface(SvEPGChannelView, previousList, getChannelIndex, channel) >= 0) {
        QBTVLogicPlayChannelFromPlaylist(self, playlist, channel, SVSTRING("TV"));
    }
}

void QBTVLogicSetPlayerTask(QBTVLogic self, SvPlayerTask playerTask)
{
    SVTESTRELEASE(self->playerTask);
    self->playerTask = SVTESTRETAIN(playerTask);
}

SvPlayerTask QBTVLogicGetPlayerTask(QBTVLogic self)
{
    return self->playerTask;
}

void QBTVLogicStopPlaying(QBTVLogic self, SvString source)
{
    //Check if it is already stoped by that source.
    int idx = SvArrayIndexOfObject(self->stopSources, (SvObject) source);
    if (idx == -1) {
        SvArrayAddObject(self->stopSources, (SvObject) source);
    }

    SvFiberDeactivate(self->timeshift.fiber);
    SvFiberEventDeactivate(self->timeshift.timer);
    SvFiberDeactivate(self->timeshift.checkBoundariesFiber);
    SvFiberEventDeactivate(self->timeshift.checkBoundariesTimer);

    if (self->controller) {
        SvInvokeInterface(QBChannelPlayerController, self->controller, stop);
        SvInvokeInterface(QBChannelPlayerController, self->controller, removeListener, (SvObject) self);
    }
    QBAppStateServiceRemoveListener(self->appGlobals->appState, (SvObject) self);

    SvTVContextSetChannel(self->appGlobals->newTV, NULL, NULL);
    if (self->frontPanelClock) {
        QBFrontPanelClockStop(self->frontPanelClock);
    }
    self->stopped = true;
}

void QBTVLogicControlGranted(SvObject self_, SvObject controller)
{
    QBTVLogic self = (QBTVLogic) self_;

    self->controlGranted = true;
    if (self->enabled)
        QBTVLogicResumePlaying(self, SVSTRING("PVR"));
}

void QBTVLogicControlRevoked(SvObject self_, SvObject controller)
{
    QBTVLogic self = (QBTVLogic) self_;

    self->controlGranted = false;
    QBTVLogicStopPlaying(self, SVSTRING("PVR"));
}

void QBTVLogicStart(QBTVLogic self)
{
    self->enabled = true;
    if (self->appGlobals->pvrProvider &&
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type != QBPVRProviderType_disk) {
        QBPVRProviderAddListener(self->appGlobals->pvrProvider, (SvObject) self);
    }
    SvFiberActivate(self->timeshift.checkBoundariesFiber);
}

void QBTVLogicStop(QBTVLogic self)
{
    if (self->appGlobals->pvrProvider &&
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type != QBPVRProviderType_disk) {
        QBPVRProviderRemoveListener(self->appGlobals->pvrProvider, (SvObject) self);
    }
    self->enabled = false;
    QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
    SvFiberDeactivate(self->timeshift.fiber);
    SvFiberEventDeactivate(self->timeshift.timer);
    SvFiberDeactivate(self->timeshift.checkBoundariesFiber);
    SvFiberEventDeactivate(self->timeshift.checkBoundariesTimer);

    QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvObject) self);
}

void QBTVLogicGetSpeedBoundary(QBTVLogic self, float *backwardMin, float *backwardMax, float *forwardMin, float *forwardMax)
{
    if (backwardMax)
        *backwardMax = -8;
    if (forwardMin)
        *forwardMin = 8;
    if (backwardMin)
        *backwardMin = -128;
    if (forwardMax)
        *forwardMax = 128;
}

SvLocal void QBTVLogicAudioDelayDialogCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    QBTVLogic self = ptr;
    QBTVLogicUnlockDisplay(self);
}

void QBTVLogicPostWidgetsCreate(QBTVLogic self)
{
    if (self->audioDelaySetupMode) {
        self->audioDelaySetupMode = false;
        int audioOutputID = QBPlatformFindAudioOutput(QBAudioOutputType_SPDIF, NULL);
        if (audioOutputID >= 0) {
            SvWidget dlg = QBAudioDelayDialogCreate(self->appGlobals, audioOutputID);
            if (dlg) {
                QBTVLogicLockDisplay(self, DisplayLockMode_Full);
                QBDialogRun(dlg, self, QBTVLogicAudioDelayDialogCallback);
            }
        }
    }
    QBTVLogicCheckBackground(self, self->appGlobals->newTV, SvTVContextGetCurrentChannel(self->appGlobals->newTV));
    QBTVLogicCheckController(self, true);
}

void QBTVLogicPreWidgetsDestroy(QBTVLogic self)
{
}

void QBTVLogicEnableAudioDelaySetupMode(QBTVLogic self)
{
    self->audioDelaySetupMode = true;
}

void QBTVLogicShowRecordingOptions(QBTVLogic self, QBPVRRecording rec)
{
    QBWindowContext ctx = self->appGlobals->newTV;
    QBTVExtendedInfoHide(ctx);
    QBTVRecordingOptionsShow(ctx, rec);
}

void QBTVLogicRestartPlayback(QBTVLogic self)
{
    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    if (!channel) {
        return;
    }

    SvTVChannel alternative = NULL;
    if (channel && channel->needsAlternative && channel->alternativeID) {
        SvObject view = QBPlaylistManagerGetById(self->playlists, SVSTRING("TVChannels"));
        alternative = SvInvokeInterface(SvEPGChannelView, view, getByID, channel->alternativeID);

        if (!alternative) {
            view = QBPlaylistManagerGetById(self->playlists, SVSTRING("RadioChannels"));
            alternative = SvInvokeInterface(SvEPGChannelView, view, getByID, channel->alternativeID);
        }
    }

    SvInvokeInterface(QBChannelPlayerController, self->controller, stop);
    SvInvokeInterface(QBChannelPlayerController, self->controller, start, alternative ? alternative : channel);
}

bool QBTVLogicHandleError(QBTVLogic self, SvErrorInfo errorInfo, int* channelErrorCode)
{
    if (QBTVLogicGetCurrentChannelMediumType(self) != SvTVChannelMediumType_IP) {
        *channelErrorCode = SV_ERR_DVB_FATAL;
        return false;
    }

    if (errorInfo) {
        *channelErrorCode = SvErrorInfoGetCode(errorInfo);
    } else {
        *channelErrorCode = SV_ERR_SERVER_CONN_FAILED;
    }

    SvTVContextScheduleAutoChannelRetry(self->appGlobals->newTV);

    return true;
}

void QBTVLogicLockDisplay(QBTVLogic self, DisplayLockMode displayLockMode)
{
    self->displayLocked = true;
    self->displayLockMode = displayLockMode;
    if (QBTVLogicAnyLocalWindowIsVisible(self))
        QBTVLogicHideAllLocalWindows(self);
}

void QBTVLogicUnlockDisplay(QBTVLogic self)
{
    self->displayLocked = false;
}

bool QBTVLogicIsPVRTimeshiftAllowedForChannel(QBTVLogic self, SvTVChannel channel)
{
    return true;
}

QBOSDMain
QBTVLogicOSDCreate(QBTVLogic self, SvApplication app, QBWindowContext tvContext)
{
    svSettingsPushComponent("osd.settings");

    AppGlobals appGlobals = self->appGlobals;
    QBNewTVOSDParams params = {
        .app          = app,
        .widgetName   = "OSDPane",
        .controller   = appGlobals->controller,
        .adService    = appGlobals->adService,
        .tvContext    = tvContext,
        .textRenderer = appGlobals->textRenderer,
        .playlists    = self->playlists,
        .viewport     = QBViewportGet(),
        .eventsLogic  = appGlobals->eventsLogic,
        .tvLogic      = appGlobals->tvLogic,
        .pc           = appGlobals->pc
    };

    QBNewTVOSD OSD = QBNewTVOSDCreate(&params);

    QBOSDTSRecording tsosd = QBOSDTSRecordingCreate(app, "TSOSD",
                                                    self->appGlobals->scheduler,
                                                    self->appGlobals->textRenderer,
                                                    self->appGlobals->eventsLogic,
                                                    self->appGlobals->langPreferences,
                                                    self->appGlobals->DRMManager,
                                                    NULL);
    QBOSDMainAttachOSD((QBOSDMain) OSD, (QBOSD) tsosd, QBOSDMainAttachmentPosition_bottom, 1, NULL);
    QBObservableAddObserver((QBObservable) tsosd, (SvObject) OSD, NULL);
    SVRELEASE(tsosd);

    svSettingsPopComponent();
    return (QBOSDMain) OSD;
}

SvLocal void
QBTVLogicSubsTrackChanged(void *self_, QBSubsTrack track)
{
    SubtitleTrackLogic subtitleTrackLogic =
        (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));

    SubtitleTrackLogicSubsTrackChanged(subtitleTrackLogic, track);
}

QBSubsManager QBTVLogicCreateSubsManager(QBTVLogic self)
{
    SvErrorInfo error = NULL;

    QBSubsManager subsManager = QBSubsManagerNew(self->appGlobals->res);

    QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) subsManager, &error);
    if (unlikely(error)) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }

    QBSubsManagerSetPreferenceFunction(subsManager, QBLangPreferencesSetPreferedSubtitlesTrack, self->appGlobals->langPreferences);
    QBLangPreferencesAddPreferredSubtitlesTrackListener(self->appGlobals->langPreferences, subsManager);
    QBSubsManagerSetCurrentTrackChangedCallback(subsManager, QBTVLogicSubsTrackChanged, self);
    return subsManager;
}

void QBTVLogicStartMetaDataManagers(QBTVLogic self, QBSubsManager subsManager, struct svdataformat* format,
                                    int pid, const char *hintedLang)
{
    if (!self->playerTask) {
        SvLogError("%s() : player task has not been set", __func__);
        return;
    }

    QBTVLogicMetaDataManagers managers = &self->metaDataManagers;
    managers->dvbSubsManager = QBDvbSubsManagerCreate(self->appGlobals->res, self->playerTask, format, subsManager);
    managers->textSubsManager = QBTextSubsManagerCreate(self->appGlobals->res, self->playerTask, format, subsManager);
    managers->teletextLineReceiver = QBTeletextLineReceiverNew(self->playerTask, pid);
    if (managers->teletextLineReceiver)
        QBTeletextLineReceiverStart(managers->teletextLineReceiver);
    managers->teletextReceiver = QBTeletextReceiverNew(managers->teletextLineReceiver);
    managers->teletextSubsManager = QBTeletextSubtitlesManagerNew(subsManager, managers->teletextReceiver);
    managers->scte27SubsManager = QBScte27SubsManagerCreate(subsManager, self->playerTask, format);

    SvObject inTV = QBLangPreferencesGetClosedCaptioningParam(self->appGlobals->langPreferences, SVSTRING("inTV"));
    if (inTV && SvStringEqualToCString(SvValueGetString((SvValue) inTV), "enabled")) {
        SvString ccConfig = QBInitLogicGetClosedCaptioningOptionsFileName(self->appGlobals->initLogic);
        managers->closedCaptionSubsManager = QBClosedCaptionSubsManagerCreate(self->appGlobals->res, ccConfig, self->playerTask, format, subsManager);
        SVTESTRELEASE(ccConfig);
    }
    QBTeletextSubtitlesManagerSetDataFormat(managers->teletextSubsManager, format);
    QBTeletextSubtitlesManagerSetHints(managers->teletextSubsManager, hintedLang);
}

void QBTVLogicStopMetaDataManagers(QBTVLogic self)
{
    QBTVLogicMetaDataManagers managers = &self->metaDataManagers;
    if (managers->dvbSubsManager) {
        QBDvbSubsManagerStop(managers->dvbSubsManager);
        SVRELEASE(managers->dvbSubsManager);
        managers->dvbSubsManager = NULL;
    }
    if (managers->textSubsManager) {
        QBTextSubsManagerStop(managers->textSubsManager);
        SVRELEASE(managers->textSubsManager);
        managers->textSubsManager = NULL;
    }
    if (managers->teletextSubsManager) {
        QBTeletextSubtitlesManagerSetDataFormat(managers->teletextSubsManager, NULL);
        SVRELEASE(managers->teletextSubsManager);
        managers->teletextSubsManager = NULL;
    }
    if (managers->teletextReceiver) {
        QBTeletextReceiverStop(managers->teletextReceiver);
        SVRELEASE(managers->teletextReceiver);
        managers->teletextReceiver = NULL;
    }
    if (managers->teletextLineReceiver) {
        QBTeletextLineReceiverStop(managers->teletextLineReceiver);
        SVRELEASE(managers->teletextLineReceiver);
        managers->teletextLineReceiver = NULL;
    }
    if (managers->scte27SubsManager) {
        QBScte27SubsManagerStop(managers->scte27SubsManager);
        SVRELEASE(managers->scte27SubsManager);
        managers->scte27SubsManager = NULL;
    }
    if (managers->closedCaptionSubsManager) {
        QBClosedCaptionSubsManagerStop(managers->closedCaptionSubsManager);
        SVRELEASE(managers->closedCaptionSubsManager);
        managers->closedCaptionSubsManager = NULL;
    }
}

QBTVLogicMetaDataManagers QBTVLogicGetMetaDataManagers(QBTVLogic self)
{
    return &self->metaDataManagers;
}

QBWindowContext
QBTVLogicCreateTVContext(QBTVLogic self)
{
    return SvTVContextCreate(self->appGlobals);
}
//{ NR Y AMERELES [#634] Logos de Canales en servidor
SvString QBTVLogicGetChannelLogoURL(SvTVChannel channel)
{
    const char *url = SvStringCString(SvURIString(channel->sourceURL));
    const char *tmp = QBConfigGet("LOGOS_IN_SERVER.ENABLED");
    if ((!tmp || strcmp(tmp, "enabled") != 0) ||         // NOT ENABLED FUNCTIONALLITY
        (url && (strncmp(url, "dvb://", 6) != 0)))       // THIS CHANNEL IS AN IP CHANNEL
    {
        if (channel->logoURL)
            return SvURLString(channel->logoURL);
        else
            return NULL;
    }
    
    SVRETAIN(channel);
    const char *logoLocalURL = QBConfigGet("LOGOS_IN_SERVER.LOCAL_URL") ? QBConfigGet("LOGOS_IN_SERVER.LOCAL_URL") : "";
    
    SvStringBuffer buff1 = SvStringBufferCreate(NULL);
    SvStringBufferAppendCString(buff1, "file://", NULL);
    SvStringBufferAppendCString(buff1, logoLocalURL, NULL);
    SvString logoLocalURLString = SvStringBufferCreateContentsString(buff1, NULL);
    
    SvURL baseURL = SvURLCreateWithString((logoLocalURLString) ,NULL);
    
    SvStringBuffer buff2 = SvStringBufferCreate(NULL);
    SvStringBufferAppendCString(buff2, SvStringCString(channel->name), NULL);
    SvStringBufferAppendCString(buff2, ".png", NULL);
    SvString channelName = SvStringBufferCreateContentsString(buff2, NULL);
    
    size_t len = SvStringLength(channelName);
    char *channelNameString = malloc(sizeof(char) * (len + 1));
    memcpy(channelNameString, SvStringCString(channelName), len + 1);
    //channelNameString = str_replace(channelNameString, " ", "%20");
    
    SvURL logoURL = SvURLCreateWithStringRelativeToURL(SvStringCreate(channelNameString, NULL), baseURL, NULL);
    SVTESTRELEASE(channel);
    return SvURLString(logoURL);
}
//} NR Y AMERELES [#634] Logos de Canales en servidor
