/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "newtv.h"
#include "newtvInternal.h"

#include <libintl.h>
#include <assert.h>
#include <stdbool.h>
#include <QBInput/QBInputCodes.h>
#include <QBApplicationController.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <sv_tuner.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/label.h>
#include <SWL/icon.h>
#include <SvPlayerControllers/SvPlayerAudioTrack.h>
#include <SvPlayerControllers/SvPlayerTaskControllersListener.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <SvPlayerManager/SvPlayerTaskListener.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvPlayerKit/SvParsersEvents.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <player_opts/epg_event.h>
#include <error_codes.h>
#include <QBStaticStorage.h>
#include <QBViewport.h>
#include <dataformat/sv_data_format_type.h>
#include <fibers/c/fibers.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/channelchoice.h>
#include <Services/langPreferences.h>
#include <Services/QBCASManager.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/SideMenu.h>
#include <Widgets/teleosd.h>
#include <Widgets/osd.h>
#include <Widgets/QBAnimatedBackground.h>
#include <QBPlayerControllers/utils/content.h>
#include <Utils/QBPlaylistUtils.h>
#include <main.h>
#include <Services/core/volume.h>
#include <Utils/authenticators.h>
#include <Widgets/authDialog.h>
#include <Widgets/tvEPGOverlay.h>
#include <Windows/pvrplayer.h>
#include <Widgets/extendedInfo.h>
#include <Widgets/SideMenu.h>
#include <Widgets/QBPlaybackErrorPopup.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBDialog.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBReminderEditorPane.h>
#include <QBTunerMonitor.h>
#include <Logic/AudioTrackLogic.h>
#include <Logic/VideoTrackLogic.h>
#include <Logic/SubtitleTrackLogic.h>
#include <Logic/QBParentalControlLogic.h>
#include <Logic/TVOSDInterface.h>
#include <Logic/TVLogic.h>
#include <Logic/VoDLogic.h>
#include <Logic/timeFormat.h>
#include <Logs/tunerLogs.h>
#include <Logs/inputLogs.h>
#include <Utils/appType.h>
#include <QBConf.h>
#include <QBTuner.h>
#include <QBTunerTypes.h>
#include <QBTunerInfo.h>
#include <Windows/newtvguide.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/HDMIService.h>
#include <QBPlayerControllers/QBChannelPlayer.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <Services/core/appState.h>
#include <QBPVRProvider.h>
#include <NPvr/QBnPVRProvider.h>
#include <NPvr/QBnPVRRecording.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <CAGE/Core/Sv2DRect.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <QBRecordFS/file.h>
#include <Utils/QBEventUtils.h>
#include <math.h>
#include <stdint.h>
#include <Services/QBCASManager.h>
#include <Services/QBCASPopupVisibilityManager.h>
#include <QBDRMManager/QBDRMManagerListener.h>
#include <Services/core/QBPlaybackMonitor.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <QBOSD/QBOSDMain.h>
#include <QBOSD/QBOSDRecording.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvPlayerControllers/SvPlayerTrackControllerListener.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <QBJSONUtils.h>

#include <QBSecureLogManager.h>

#define QBTimeLimitEnable
#define QBTimeThreshold (50*1000)
#include <QBTimeLimit.h>

#include <Widgets/confirmationDialog.h> //RAL TimeShift cancel pop-up window
#include <Services/BoldUsageLog.h> //NR : Adding Bold DVB Usage Logs

#define AUTO_PLAYBACK_RESTART_INITIAL_DELAY   6   // [s]
#define AUTO_PLAYBACK_RESTART_FINAL_DELAY     300 // [s]
#define BUFFERING_POPUP_DELAY_SEC             8   // [s]
#define CONNECTION_ERROR_POPUP_DELAY_SEC      60  // [s]

#define BOTTOM_OSD_FAST_UPDATE_PERIOD_SEC 0.2
#define BOTTOM_OSD_NORMAL_UPDATE_PERIOD_SEC 1.0
#define BOTTOM_OSD_UPDATE_SPEED_THRESHOLD_SEC 2.0

#define CAS_POPUP_BLOCKER_CONTEXT_ID "newtv"
#define CAS_POPUP_BLOCKER_PIN_POPUP_WINDOW_ID "pinPopup"
#define CAS_POPUP_BLOCKER_PVR_RESOLVER_WINDOW_ID "pvrResolver"
#define CAS_POPUP_BLOCKER_FAVORITE_LIST_WINDOW_ID "favoriteList"
#define CAS_POPUP_BLOCKER_MINI_RECORDING_LIST_WINDOW_ID "miniRecordingList"
#define CAS_POPUP_BLOCKER_MINI_MINI_CHANNEL_LIST_WINDOW_ID "miniChannelList"
#define CAS_POPUP_BLOCKER_SIDE_MENU_WINDOW_ID "sideMenu"
#define CAS_POPUP_BLOCKER_LANG_MENU_WINDOW_ID "langMenu"
#define CAS_POPUP_BLOCKER_TV_GUIDE_WINDOW_ID "tvGuide"
#define CAS_POPUP_BLOCKER_EXTENDED_INFO_WINDOW_ID "extendedInfo"

SvLocal void QBTVUpdateFormat(QBWindowContext ctx_);
SvLocal void SvTVContextDeactivateConnectionMonitor(SvTVContext ctx);
SvLocal void SvTVContextActivateConnectionMonitor(SvTVContext ctx);
SvLocal void QBTVPlaybackProblemShow(SvTVContext ctx, int errorCode);
SvLocal void QBTVShowRecordingBlockedPopup(TVInfo *tvInfo, SvString channel);
SvLocal SvString QBTVCreateCurrentEventData(SvTVContext ctx, bool addErrorCode, const SvErrorInfo errorInfo);
SvLocal void QBTVBufferingPopupHide(SvTVContext ctx);

SvLocal void
SvTVContextRestartStep(void* ctx_)
{
    SvTVContext ctx = ctx_;
    SvFiberDeactivate(ctx->autoRestartFiber);
    SvFiberEventDeactivate(ctx->autoRestartTimer);
    QBTVLogicRestartPlayback(ctx->appGlobals->tvLogic);
}

SvLocal void tvNoSignalCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->signalPopup = NULL;
}

SvLocal void tvNoSignalShow(TVInfo* tvInfo)
{
    if (tvInfo->signalPopup)
        return;
    QBWindowContext windowCtx = (QBWindowContext) tvInfo->ctx;
    AppGlobals appGlobals = tvInfo->ctx->appGlobals;
    const char *message = gettext("NO SIGNAL");
    tvInfo->signalPopup = QBPlaybackErrorPopupCreate(appGlobals->controller, windowCtx, message, "");
    QBDialogRun(tvInfo->signalPopup, tvInfo, tvNoSignalCallback);
    SvString data = QBTVCreateCurrentEventData((SvTVContext) windowCtx, false, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.NoSignal", "JSON:{\"description\":\"%s\"%s}", message,
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

// BEGIN RAL TimeShift cancel pop-up window
SvLocal void tvTimeShiftCancelPopUpCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;

    if(!tvInfo || !tvInfo->timeShiftConfirmPopup)
        return;

    if(buttonTag && SvStringEqualToCString(buttonTag, "OK-button"))
    {
        if(tvInfo->objLogic != NULL)
        {
            QBTVLogicTimeShiftCancelPopUpCallback(tvInfo->objLogic, tvInfo->event); //perform desired operation (e.g.: change channel)
        }
        else
        {
            if(tvInfo->event == '0')
            {
                SvTVContext ctx = tvInfo->ctx;
                QBTVLogicPlayChannelByNumber(ctx->appGlobals->tvLogic, tvInfo->timeShiftChannelNumberDest, SVSTRING("ManualSelection"));
                
                //SvTVChannel channelToGo = SvInvokeInterface(SvEPGChannelView, list, getByNumber, selection, SvEPGViewNumberRoundingMethod_exact)
                //QBTVLogicPlayChannel(ctx->appGlobals->tvLogic, channelToGo, SVSTRING("ManualSelection"));
                //SvInvokeInterface(TVOSD, tvInfo->OSD, setTVChannel, channelToGo, SVSTRING("ManualSelection"));
            }
            else if(tvInfo->event == QBKEY_LOOP)
            {
                SvTVContext ctx = tvInfo->ctx;
                if(ctx->previous)
                    QBTVLogicPlayPreviousChannel(ctx->appGlobals->tvLogic);
            }
        }
    }
    else
    {
        if(tvInfo->objLogic == NULL)
        {
            if(tvInfo->event == '0')
            {
                svOSDHide(tvInfo->background, true);
                QBTVBackgroundHide((QBWindowContext) tvInfo->ctx);
            }
        }
    }
    
    tvInfo->timeShiftConfirmPopup = NULL;
    tvInfo->objLogic = NULL;
}

SvLocal void tvTimeShiftCancelPopUpShow(TVInfo* tvInfo)
{
    if (!tvInfo || tvInfo->timeShiftConfirmPopup)
        return;
    
    QBConfirmationDialogParams_t params = {
        .title = gettext("Usted está en modo 'Pausa en vivo':"),
        .message = gettext("Está a punto de cambiar de canal y perder la sesión grabada, ¿desea continuar?"),
        .local = true,
        .focusOK = false,
        .isCancelButtonVisible = true
    };
    SvTVContext ctx = tvInfo->ctx;
    tvInfo->timeShiftConfirmPopup = QBConfirmationDialogCreate(ctx->appGlobals->res, &params);
    svSettingsPopComponent();
    QBDialogRun(tvInfo->timeShiftConfirmPopup, tvInfo, tvTimeShiftCancelPopUpCallback);
}

void
QBTVTimeShiftCancelPopUpShow(SvGenericObject listenerObj, QBWindowContext ctx_, unsigned int event, int channelNumber)
{
    QBTVLogic obj = (QBTVLogic)listenerObj;
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;

    TVInfo *tvInfo = ctx->super_.window->prv;
    tvInfo->objLogic = obj;
    tvInfo->event = event;
    tvInfo->timeShiftChannelNumberDest = channelNumber;

    tvTimeShiftCancelPopUpShow(tvInfo);
}

bool
QBTVTimeShiftCancelPopUpShowApplies(QBWindowContext ctx_)
{
    SvTVContext ctx= (SvTVContext) ctx_;
    if (!ctx || !ctx->playerTask)
        return false;

    SvPlayerTaskState state= SvPlayerTaskGetState(ctx->playerTask);
    if (state.timeshift.recording)
        return true;

    return false;
}
// END RAL TimeShift cancel pop-up window

SvLocal void tvHDCPAuthFailedCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->HDCPAuthFailedPopup = NULL;
}

SvLocal void tvHDCPAuthFailedShow(TVInfo* tvInfo)
{
    QBWindowContext windowCtx = (QBWindowContext) tvInfo->ctx;
    AppGlobals appGlobals = tvInfo->ctx->appGlobals;
    const char *message = gettext("HDCP ERROR");
    tvInfo->HDCPAuthFailedPopup = QBPlaybackErrorPopupCreate(appGlobals->controller, windowCtx, message, "");
    QBDialogRun(tvInfo->HDCPAuthFailedPopup, tvInfo, tvHDCPAuthFailedCallback);
    SvString data = QBTVCreateCurrentEventData((SvTVContext) windowCtx, false, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.HDCPAuthFailed", "JSON:{\"description\":\"%s\"%s}", message,
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

SvLocal bool tvCheckSignal(int tunerNum)
{
  // If tuner is not used, then there is no problem.
  if (tunerNum < 0)
    return true;

  struct sv_tuner_state* tuner = sv_tuner_get(tunerNum);
  struct QBTunerStatus status;
  if (sv_tuner_get_status(tuner, &status))
      return false;

  //If tuner shouldn't have lock yet, then we don't tell we have no signal
  if (!status.should_have_lock)
    return true;

  //If tuner should have lock and hasn't, then tell we have no signal
  return status.full_lock;
}

SvLocal void TVInfoTunerStateChanged(SvGenericObject self_, int tunerNum)
{
    TVInfo *tvInfo = (TVInfo*) self_;
    if (QBTVLogicGetTunerNum(tvInfo->ctx->appGlobals->tvLogic) != tunerNum)
        return;

    if (tvCheckSignal(tunerNum)) {
        if (tvInfo->signalPopup)
            QBDialogBreak(tvInfo->signalPopup);
    } else {
        if (!tvInfo->signalPopup)
            tvNoSignalShow(tvInfo);
    }
}

SvLocal void SvTVContextHDCPAuthStatusChanged(SvObject self_, HDCPAuthStatus status)
{
    SvTVContext ctx = (SvTVContext) self_;
    QBWindowContext windowCtx = (QBWindowContext) ctx;
    if (!windowCtx->window)
        return;

    TVInfo* tvInfo = (TVInfo*) windowCtx->window->prv;

    if (status == HDCPAuth_error) {
        if (!tvInfo->HDCPAuthFailedPopup) {
            tvHDCPAuthFailedShow(tvInfo);
        }
    } else {
        if (tvInfo->HDCPAuthFailedPopup) {
            QBDialogBreak(tvInfo->HDCPAuthFailedPopup);
        }
    }
}

SvLocal void SvTVContextHotplugStatusChanged(SvObject self_, bool isConnected)
{
}

SvLocal SvType TVInfo_getType(void)
{
   static SvType type = NULL;
   static const struct QBTunerMonitorListener_t listener = {
     .tunerStateChanged = TVInfoTunerStateChanged
   };
   if (unlikely(!type)) {
      SvTypeCreateManaged("TVInfo",
                          sizeof(struct TVInfo_),
                          SvObject_getType(), &type,
                          QBTunerMonitorListener_getInterface(), &listener,
                          NULL);
   }
   return type;
}

SvLocal void SvTVContextGetCurrentEventFromPlayer(void* ctx_, time_t currentTime, SvEPGEvent* current, SvEPGEvent* next)
{
    SvTVContext ctx = (SvTVContext) ctx_;

    if (!ctx->playerTask) {
        return;
    }
    struct player_opt_epg_event_s  opt = { .current = NULL };
    int res = SvPlayerTaskSetOpt(ctx->playerTask, PLAYER_OPT_EPG_EVENT_GET, &opt);
    if (res < 0) {
        return;
    }
    SvTimeRange timeRange;
    SvTimeRangeInit(&timeRange, 0, 0);
    if (opt.current) {
        SvEPGEventGetTimeRange(opt.current, &timeRange, NULL);
    }
    if (SvTimeRangeContainsTimePoint(&timeRange, currentTime)) {
        if (current) {
            *current = opt.current;
        }
        if (next) {
            *next = opt.following;
        }
    }
}

SvLocal void SvTVContextGetCurrentEventFromManager(void* ctx_, SvTVChannel channel, time_t currentTime, SvEPGEvent* current, SvEPGEvent *next)
{
    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    if (!epgManager) {
        return;
    }
    SvEPGIterator it = SvEPGManagerCreateIterator(epgManager, SvDBObjectGetID((SvDBObject) channel), currentTime);
    SvEPGEvent event = SvEPGIteratorGetNextEvent(it);
    SvTimeRange timeRange;
    SvTimeRangeInit(&timeRange, 0, 0);
    if (event) {
        SvEPGEventGetTimeRange(event, &timeRange, NULL);
    }
    if (SvTimeRangeContainsTimePoint(&timeRange, currentTime)) {
        if (current) {
            *current = event;
        }
        if (next) {
            *next = SvEPGIteratorGetNextEvent(it);
        }
    } else {
        if (current) {
            *current = NULL;
        }
        if (next) {
            *next = event;
        }
    }
    SVTESTRELEASE(it);
}

SvLocal void SvTVContextGetCurrentEvent(void* ctx_, SvTVChannel channel, time_t currentTime, SvEPGEvent* current, SvEPGEvent *next)
{
    SvTVContext ctx = ctx_;

    if (ctx->currentEventSourcesPriority == SvTVContextCurrentEventSourcesPriority_PlayerThenManager) {
        SvTVContextGetCurrentEventFromPlayer(ctx_, currentTime, current, next);
        if (current && !*current) {
            SvTVContextGetCurrentEventFromManager(ctx_, channel, currentTime, current, next);
        }
    } else {
        SvTVContextGetCurrentEventFromManager(ctx_, channel, currentTime, current, next);
        if (current && !*current && ctx->currentEventSourcesPriority != SvTVContextCurrentEventSourcesPriority_ManagerOnly) {
            SvTVContextGetCurrentEventFromPlayer(ctx_, currentTime, current, next);
        }
    }
}

/** This function creates JSON-like formated string which contains information about EPG event.
 *  Example of returned string:
 *       "channel":"LOVE 97 FM","epgID":"112345","epgTitle":"LOVE 97 FM Programming","epgTimeRange":"4:00-5:00","errorCode":"10"
 **/
SvLocal SvString
QBTVCreateEventData(SvTVContext ctx, SvTVChannel channel, time_t eventTime, bool addErrorCode, const SvErrorInfo errorInfo)
{
    SvEPGEvent event = NULL;
    SvEPGEventDesc description = NULL;
    SvString timeRange = NULL;
    SvString eventID = NULL;
    SvString extraParams = NULL;
    SvString medium = NULL;
    if (!ctx || !channel) {
        return NULL;
    }

    SvTVContextGetCurrentEvent(ctx,  channel, eventTime, &event, NULL);
    if (event) {
        struct tm beginTime, endTime;
        SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), true, &beginTime);
        SvTimeBreakDown(SvTimeConstruct(event->endTime, 0), true, &endTime);
        timeRange = SvStringCreateWithFormat(",\"epgTimeRange\":\"%d:%02d-%d:%02d\"", beginTime.tm_hour, beginTime.tm_min, endTime.tm_hour, endTime.tm_min);
        description = SvEPGEventGetAnyDescription(event);
        eventID = SvInvokeVirtual(SvEPGEvent, event, createStringID);
    }

    if (SvTVChannelGetMediumType(channel) == SvTVChannelMediumType_DVB) {
        struct QBTunerParams tunerParams;
        SvString authority = SvURIAuthority(channel->sourceURL);
        if (authority && !QBTunerParamsFromString(SvStringCString(authority), &tunerParams)) {
            extraParams = SvStringCreateWithFormat(",\"frequency\":\"%d\"", tunerParams.mux_id.freq);
        }
        medium = SvStringCreate("dvb", NULL);
    } else if (SvTVChannelGetMediumType(channel) == SvTVChannelMediumType_IP) {
        SvString address = SvURLString(channel->sourceURL);
        if (address) {
            char* escapedAddress = QBStringCreateJSONEscapedString(SvStringCString(address));
            extraParams = SvStringCreateWithFormat(",\"url\":\"%s\"", escapedAddress);
            free(escapedAddress);
        }
        medium = SvStringCreate("ip", NULL);
    }

    SvString errorCode = NULL;
    if (addErrorCode) {
        int code = errorInfo ? SvErrorInfoGetCode(errorInfo) : ctx->channelErrorCode;
        errorCode = SvStringCreateWithFormat(",\"errorCode\":\"%d\"", code);
    }

    SvString channelId = NULL;
    SvValue channelIdVal = SvDBObjectGetID((SvDBObject) channel);
    if (channelIdVal && SvValueIsString(channelIdVal)) {
        channelId = SvValueGetString(channelIdVal);
    }

    char* escapedChannelName = QBStringCreateJSONEscapedString(SvStringCString(channel->name));
    char* escapedChannelId = QBStringCreateJSONEscapedString(SvStringCString(channelId));
    char* escapedEventID = eventID ? QBStringCreateJSONEscapedString(SvStringCString(eventID)) : NULL;
    char* escapedTitle = description && description->title ? QBStringCreateJSONEscapedString(SvStringCString(description->title)) : NULL;

    SvString eventData = SvStringCreateWithFormat(",\"channel\":\"%s\",\"channelId\":\"%s\",\"channelNumber\":\"%d\",\"epgID\":\"%s\",\"epgTitle\":\"%s\"%s%s%s,\"medium\":\"%s\""
                                                  ",\"playbackType\":\"live\"",
                                                  escapedChannelName,
                                                  escapedChannelId ? escapedChannelId : "",
                                                  channel->number,
                                                  escapedEventID ? escapedEventID : "",
                                                  escapedTitle ? escapedTitle : "",
                                                  timeRange ? SvStringCString(timeRange) : "",
                                                  extraParams ? SvStringCString(extraParams) : "",
                                                  errorCode ? SvStringCString(errorCode) : "",
                                                  medium ? SvStringCString(medium) : "");
    free(escapedChannelName);
    free(escapedChannelId);
    free(escapedEventID);
    free(escapedTitle);

    SVTESTRELEASE(extraParams);
    SVTESTRELEASE(timeRange);
    SVTESTRELEASE(eventID);
    SVTESTRELEASE(errorCode);
    SVTESTRELEASE(medium);
    return eventData;
}

SvLocal SvString
QBTVCreateCurrentEventData(SvTVContext ctx, bool addErrorCode, const SvErrorInfo errorInfo)
{
    return QBTVCreateEventData(ctx, ctx->channel, SvTimeNow(), addErrorCode, errorInfo);
}

SvLocal void
QBTVEventPaused(SvTVContext ctx)
{
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("newtv", "Notice.TVPlayer.EventPaused", "JSON:{\"description\":\"timeshift paused\"%s}", data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

SvLocal void
QBTVEventResumed(SvTVContext ctx)
{
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("newtv", "Notice.TVPlayer.EventResumed", "JSON:{\"description\":\"timeshift resumed\"%s}", data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

SvLocal void
SvTVContextEPGEventChanged(SvTVContext ctx, SvTVChannel channel)
{
    int64_t duration = 0;
    if (!ctx->super_.window) {
        return;
    }

    if (ctx->playbackMonitor && ctx->playerTask && QBPlaybackMonitorIsWorking(ctx->playbackMonitor)) {
        QBPlaybackMonitorPlaybackStoped(ctx->playbackMonitor, &duration, NULL);
        SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
        QBPlaybackMonitorPlaybackStarted(ctx->playbackMonitor, ctx->playerTask, SVSTRING("newtv"), SVSTRING("TVPlayer"), data, NULL);
        SVTESTRELEASE(data);
    }
}

SvLocal void tvOSDCheckForChanges(SvWidget tv);
/// -------------------------------
/// SvPlayerTask - state changes
/// -------------------------------
SvLocal void svTVContextChangedPlaySpeed(SvTVContext ctx, double wantedSpeed)
{
    if (!ctx->super_.window)
        return;

    TVInfo *tvInfo = ctx->super_.window->prv;

    QBOSD osd = QBOSDMainGetHandler(tvInfo->OSD, QBOSDHandlerType_recording, NULL);
    if (!osd)
        goto out;

    if (ctx->playerTask) {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        QBOSDRecordingUpdatePlaybackState((QBOSDRecording) osd, &state, wantedSpeed, NULL);
    } else {
        QBOSDRecordingUpdatePlaybackState((QBOSDRecording) osd, NULL, wantedSpeed, NULL);
    }

out:
    tvOSDCheckForChanges(ctx->super_.window);
}

SvLocal void handlePlayerStateChange(SvTVContext ctx)
{
  QBWindowContext super = (QBWindowContext) ctx;

  if (super->window && !((TVInfo*)super->window->prv)->playbackProblemPopup) {
     SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
     svTVContextChangedPlaySpeed(ctx, state.wantedSpeed);
  }
}

SvLocal SvWidget
QBTVSetupBufferingPopup(SvTVContext ctx)
{
    AppGlobals appGlobals = ctx->appGlobals;
    SvApplication app = appGlobals->res;

    svSettingsPushComponent("tvBufferingPopup.settings");

    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeNonFocusable);
    if (!dialog) {
        return NULL;
    }
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, gettext(svSettingsGetString("Popup", "title")));

    SvWidget animation = QBWaitAnimationCreate(app, "Popup.Animation");

    if (animation) {
        QBDialogAddWidget(dialog, SVSTRING("content"), animation, SVSTRING("animation"), 3, "Dialog.Animation");
    }

    svSettingsPopComponent();

    return dialog;
}

/// -------------------------
/// SvPlayerTask - handlers
/// -------------------------

SvLocal void SvTVContextPlayHandlerStateChanged(SvObject self_)
{
    SvTVContextPlayHandler self = (SvTVContextPlayHandler) self_;
    handlePlayerStateChange(self->ctx);
}

SvLocal void SvTVContextPlayHandlerFatalErrorOccured(SvObject self_, SvErrorInfo errorInfo)
{
    SvTVContextPlayHandler self = (SvTVContextPlayHandler) self_;
    SvTVContext ctx = self->ctx;

    if (!QBTVLogicHandleError(ctx->appGlobals->tvLogic, errorInfo, &ctx->channelErrorCode)) {
        SvLogError(COLBEG() "CubiTV: player encountered fatal error" COLEND_COL(red));
        SvString data = QBTVCreateCurrentEventData(ctx, true, errorInfo);
        QBSecureLogEvent("newtv", "Error.TVPlayer", "JSON:{\"description\":\"player encountered fatal error\"%s}", data ? SvStringCString(data) : "");
        SVTESTRELEASE(data);
        SvErrorInfoWriteLogMessage(errorInfo);

        QBTVBufferingPopupHide(ctx);
        QBTVPlaybackProblemShow(ctx, ctx->channelErrorCode);
    }

    SvErrorInfoDestroy(errorInfo);
}

SvLocal void SvTVContextPlayHandlerNonFatalErrorOccured(SvObject self_, SvErrorInfo errorInfo)
{
    SvTVContextPlayHandler self = (SvTVContextPlayHandler) self_;
    SvTVContext ctx = self->ctx;

    SvLogNotice(COLBEG() "CubiTV: player encountered error" COLEND_COL(yellow));

    if (!QBTVLogicHandleError(ctx->appGlobals->tvLogic, errorInfo, &ctx->channelErrorCode)) {
        SvString data = QBTVCreateCurrentEventData(ctx, true, errorInfo);
        QBSecureLogEvent("newtv", "Error.TVPlayer", "JSON:{\"description\":\"player encountered error\"%s}", data ? SvStringCString(data) : "");
        SVTESTRELEASE(data);
        SvErrorInfoWriteLogMessage(errorInfo);
    }

    SvErrorInfoDestroy(errorInfo);
}

SvLocal void SvTVContextPlayHandlerPlaybackFinished(SvObject self_)
{
    SvTVContextPlayHandler self = (SvTVContextPlayHandler) self_;
    SvTVContext ctx = self->ctx;
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);

    if (state.wantedSpeed < 0) {
        SvLogNotice(COLBEG() "CubiTV: player finished playback - pausing" COLEND_COL(cyan));
        SvPlayerTaskPlay(ctx->playerTask, 0.0, -1.0, NULL);
    } else {
        SvLogNotice(COLBEG() "CubiTV: player finished playback - switching to live" COLEND_COL(cyan));
        SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, false);
    }

    svTVContextChangedPlaySpeed(ctx, 1.0);
}

SvLocal void SvTVContextPlayHandlerHandleEvent(SvObject self_, SvString name, void *arg)
{
    SvTVContextPlayHandler self = (SvTVContextPlayHandler) self_;
    SvTVContext ctx = self->ctx;
    if (ctx->playbackMonitor) {
        QBPlaybackMonitorPlayerEventHandler(ctx->playbackMonitor, name, arg, NULL);
    }
    if (ctx->appGlobals->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, ctx->appGlobals->casPopupManager, playbackEvent, name, arg);

    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);

    if (!strcmp(PLAYER_EVENT_TRICK_ONGOING_REC_LIVEPOINT, SvStringCString(name))) {
        if (state.wantedSpeed != 1.0) {
            SvPlayerTaskPlay(ctx->playerTask, 1.0, -1.0, NULL);
        }
        svTVContextChangedPlaySpeed(ctx, 1.0);
    } else if (!strcmp(PLAYER_EVENT_TRICK_ONGOING_REC_STARTPOINT, SvStringCString(name))) {
        SvPlayerTaskPlay(ctx->playerTask, 0.0, -1.0, NULL);
        svTVContextChangedPlaySpeed(ctx, 0.0);
    }
}

SvLocal SvType
SvTVContextPlayHandler_getType(void)
{
    static const struct SvPlayerTaskListener_ listenerMethods = {
        .stateChanged         = SvTVContextPlayHandlerStateChanged,
        .fatalErrorOccured    = SvTVContextPlayHandlerFatalErrorOccured,
        .nonFatalErrorOccured = SvTVContextPlayHandlerNonFatalErrorOccured,
        .playbackFinished     = SvTVContextPlayHandlerPlaybackFinished,
        .handleEvent          = SvTVContextPlayHandlerHandleEvent
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("SvTVContextPlayHandler",
                            sizeof(struct SvTVContextPlayHandler_t),
                            SvObject_getType(),
                            &type,
                            SvPlayerTaskListener_getInterface(), &listenerMethods,
                            NULL);
    }

    return type;
}

SvLocal double SvTVContextGetNewPlayerPosition(SvTVContext ctx, double delta)
{
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);

    double windowStart;
    double windowEnd;

    if (ctx->timeshiftFromRecStartTime) {
        windowStart = state.timeshift.range_start;
        windowEnd = state.timeshift.range_end;
    } else {
        windowStart = state.currentPosition - state.timeshift.range_start;
        windowEnd = state.currentPosition - state.timeshift.range_end;
    }

    double newPosition = state.currentPosition + delta;

    if (newPosition < windowStart) {
        newPosition = windowStart;
    } else if (newPosition > windowEnd) {
        newPosition = windowEnd;
    }

    return newPosition;
}

void SvTVContextDisableOverlay(QBWindowContext ctx_)
{
  SvTVContext ctx = (SvTVContext) ctx_;
  ctx->activeOverlayRequest = false;
  QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);
}

void SvTVContextSetOverlay(QBWindowContext ctx_, const Sv2DRect* rect)
{
    SvTVContext ctx = (SvTVContext) ctx_;

    ctx->activeOverlayRequest = true;

    if (rect) {
        ctx->overlayRequest.rect = *rect;
        ctx->overlayRequest.fullscreen = false;
    } else {
        ctx->overlayRequest.rect = Sv2DRectCreate(0, 0, 0, 0);
        ctx->overlayRequest.fullscreen = true;
    }

    if (QBParentalControlLogicGetAuthState(ctx->appGlobals->parentalControlLogic, ctx->pc) != QBParentalControlLogicAuthState_EnableAV) {
        return;
    }

    QBViewportMode mode = ctx->overlayRequest.fullscreen ? QBViewportMode_fullScreen : QBViewportMode_windowed;
    QBViewportSetVideoWindow(QBViewportGet(), mode, &ctx->overlayRequest.rect);
}

bool SvTVContextIsLive(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->playerTask)
        return false;
    SvPlayerTaskState state  = SvPlayerTaskGetState(ctx->playerTask);
    return !state.timeshift.wanted;
}

double SvTVContextGetSpeed(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->playerTask)
        return 0.0;
    SvPlayerTaskState state  = SvPlayerTaskGetState(ctx->playerTask);
    return state.wantedSpeed;
}

SvTVChannel SvTVContextGetPreviousChannel(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    return ctx->previous;
}

SvString SvTVContextGetPreviousList(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    return ctx->previousList;
}

void SvTVContextREW(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask)
        return;
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);

    float min;
    float max;
    QBTVLogicGetSpeedBoundary(ctx->appGlobals->tvLogic, &min, &max, NULL, NULL);


    if (state.wantedSpeed >= 0) {
        SvPlayerTaskPlay(ctx->playerTask, max, -1, NULL);
        svTVContextChangedPlaySpeed(ctx, max);
    } else {
        float wantedSpeed = 2 * state.wantedSpeed;
        if (wantedSpeed < min)
            return;

        SvPlayerTaskPlay(ctx->playerTask, wantedSpeed, -1, NULL);
        svTVContextChangedPlaySpeed(ctx, wantedSpeed);
    }

    QBSubsManagerFlushSubtitleForTrack(ctx->subsManager, QBSubsManagerGetCurrentTrack(ctx->subsManager));
}

void SvTVContextFFW(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask)
        return;
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);

    float max;
    float min;
    QBTVLogicGetSpeedBoundary(ctx->appGlobals->tvLogic, NULL, NULL, &min, &max);

    if (state.wantedSpeed <= 1) {
        SvPlayerTaskPlay(ctx->playerTask, min, -1, NULL);
        svTVContextChangedPlaySpeed(ctx, min);
    } else {
        float wantedSpeed = 2 * state.wantedSpeed;
        if (wantedSpeed > max)
            return;

        SvPlayerTaskPlay(ctx->playerTask, wantedSpeed, -1, NULL);
        svTVContextChangedPlaySpeed(ctx, wantedSpeed);
    }

    QBSubsManagerFlushSubtitleForTrack(ctx->subsManager, QBSubsManagerGetCurrentTrack(ctx->subsManager));
}

void SvTVContextChangePosition(QBWindowContext ctx_, double delta)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask) {
        return;
    }

    double newPos = SvTVContextGetNewPlayerPosition(ctx, delta);

    SvPlayerTaskPlay(ctx->playerTask, 1.0, newPos, NULL);
    svTVContextChangedPlaySpeed(ctx, 1.0);
}

void
SvTVContextScheduleAutoChannelRetry(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;

    if (ctx->autoRestartDelaySec <= 0) {
        ctx->autoRestartDelaySec = AUTO_PLAYBACK_RESTART_INITIAL_DELAY;
    } else if (ctx->autoRestartDelaySec < AUTO_PLAYBACK_RESTART_FINAL_DELAY) {
        ctx->autoRestartDelaySec *= 2;
        if (ctx->autoRestartDelaySec > AUTO_PLAYBACK_RESTART_FINAL_DELAY) {
            ctx->autoRestartDelaySec = AUTO_PLAYBACK_RESTART_FINAL_DELAY;
        }
    }

    int64_t requestSchedulerIntervalUs = ctx->autoRestartDelaySec * 1000000;
    const int64_t maxTimeOffsetUs = requestSchedulerIntervalUs / 2;

    int64_t randomBreakTimeUs = requestSchedulerIntervalUs + ((rand() % (2*maxTimeOffsetUs)) - maxTimeOffsetUs);

    if (!ctx->autoRestartFiber) {
        ctx->autoRestartFiber = SvFiberCreate(ctx->appGlobals->scheduler, NULL, "SvTVContextRestartStep", SvTVContextRestartStep, ctx);
        ctx->autoRestartTimer = SvFiberTimerCreate(ctx->autoRestartFiber);
    }

    SvLogWarning(COLBEG() "CubiTV: player encountered error - retrying in %lld seconds" COLEND_COL(yellow), (long long int) randomBreakTimeUs / 1000000);
    SvString data = QBTVCreateCurrentEventData(ctx, true, NULL);
    QBSecureLogEvent("newtv", "Error.TVPlayer", "JSON:{\"description\":\"player encountered error"
                     " - retrying in %lld seconds\"%s}", (long long int) randomBreakTimeUs / 1000000, data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    SvFiberTimerActivateAfter(ctx->autoRestartTimer, SvTimeFromUs(randomBreakTimeUs));
}

void
SvTVContextSetChannelErrorCode(QBWindowContext ctx_, int errorCode)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    ctx->channelErrorCode = errorCode;
}

SvLocal void tvCheckPINPopupHide(TVInfo *tvInfo)
{
    if (!tvInfo->parentalPopup) {
        return;
    }
    SvTVContext ctx = tvInfo->ctx;
    QBDialogBreak(tvInfo->parentalPopup);

    if (ctx->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerUnblockAll(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID));
    }
}

SvLocal void tvCheckPINCallback(void *ptr, SvWidget dlg, SvString ret, unsigned key)
{
    TVInfo* tvInfo = ptr;
    SvTVContext ctx = tvInfo->ctx;

    if (!tvInfo->parentalPopup)
        return;

    tvInfo->parentalPopup = NULL;
    QBTVLogicUnlockDisplay(ctx->appGlobals->tvLogic);

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBParentalControlHelperAuthenticated(ctx->pc);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_PIN_POPUP_WINDOW_ID));
        }
    }
    SVRELEASE(tvInfo);
}

SvLocal void tvCheckPINPopupShow(TVInfo *tvInfo)
{
    SvTVContext ctx = tvInfo->ctx;
    QBTVLogicLockDisplay(ctx->appGlobals->tvLogic, DisplayLockMode_EnableUpDown);
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(ctx->appGlobals->scheduler, ctx->appGlobals->accessMgr, SVSTRING("PC"));
    SvWidget master = NULL;
    svSettingsPushComponent("TVParentalControlDialog.settings");
    SvWidget dialog = QBAuthDialogCreate(ctx->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, &master);
    svSettingsPopComponent();

    tvInfo->parentalPopup = dialog;
    SVRETAIN(tvInfo);
    QBDialogRun(dialog, tvInfo, tvCheckPINCallback);
    if (ctx->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_PIN_POPUP_WINDOW_ID));
    }
}

SvLocal void SvTVContextUpdateSubtitlesState(SvTVContext ctx, bool forcedHide)
{
    SvWindow tvWindow = ctx->super_.window;
    if (!forcedHide) {
        if (tvWindow) {   // main TV window
            SvTVContextAttachSubtitle((QBWindowContext) ctx, tvWindow);
        } else if (ctx->activeOverlayRequest) { // EPG TV preview
            if (ctx->showSubtitles && ctx->windowForSubtitles) {
                SvTVContextAttachSubtitle((QBWindowContext) ctx, ctx->windowForSubtitles);
            }
        }
    } else {
        if (tvWindow || ctx->activeOverlayRequest) {
            SvTVContextDetachSubtitle((QBWindowContext) ctx);
        }
    }
}

#define SV_TV_NAME_IN_VOLUME_MUTE_TABLE "SvTV"

SvLocal void SvTVContextMute(SvTVContext ctx, bool mute)
{
    int level;
    bool wasVolumeLocked;
    QBVolumeGetCurrentState(ctx->appGlobals->volume, &level, NULL, &wasVolumeLocked);
    QBVolumeUnlock(ctx->appGlobals->volume);
    QBVolumeMute(ctx->appGlobals->volume, SVSTRING(SV_TV_NAME_IN_VOLUME_MUTE_TABLE), mute);
    if (wasVolumeLocked)
        QBVolumeLock(ctx->appGlobals->volume, level);
}

SvLocal void SvTVContextEnableAV(SvTVContext ctx)
{
    SvTVContextMute(ctx, false);

    if (ctx->activeOverlayRequest) {
        if (ctx->overlayRequest.fullscreen) {
            QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_fullScreen, NULL);
        } else {
            SvTVContextSetOverlay((QBWindowContext) ctx, &ctx->overlayRequest.rect);
        }
    } else {
        QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_fullScreen, NULL);
    }
    if (ctx->super_.window) {
        SvTVContextAttachSubtitle((QBWindowContext) ctx, ctx->super_.window);
    }
}

SvLocal void SvTVContextDisableAV(SvTVContext ctx)
{
    SvTVContextMute(ctx, true);
    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);
    if (ctx->super_.window) {
        SvTVContextDetachSubtitle((QBWindowContext) ctx);
    }
}

SvLocal void SvTVContextStartAuthentication(SvTVContext ctx)
{
    TVInfo *tvInfo = NULL;
    if (ctx->super_.window) {
        tvInfo = ctx->super_.window->prv;
    }

    SvTVContextDisableAV(ctx);
    if (tvInfo && !tvInfo->parentalPopup) {
        tvCheckPINPopupShow(tvInfo);
    }

    ctx->waitingForAuthentication = true;
}

SvLocal void SvTVContextFinishAuthentication(SvTVContext ctx)
{
    TVInfo *tvInfo = NULL;
    if (ctx->super_.window) {
        tvInfo = ctx->super_.window->prv;
    }

    SvTVContextEnableAV(ctx);
    if (tvInfo && tvInfo->parentalPopup) {
        tvCheckPINPopupHide(tvInfo);
    }

    ctx->waitingForAuthentication = false;
}

SvLocal void SvTVContextAuthenticationStateChanged(void *_ctx)
{
    SvTVContext ctx = _ctx;
    QBParentalControlLogicAuthState state = QBParentalControlLogicGetAuthState(ctx->appGlobals->parentalControlLogic, ctx->pc);
    if (state == QBParentalControlLogicAuthState_EnableAV) {
        SvTVContextFinishAuthentication(ctx);
    } else if (state == QBParentalControlLogicAuthState_PINPopup_DisableAV) {
        SvTVContextStartAuthentication(ctx);
    } else if (state == QBParentalControlLogicAuthState_DisableAV) {
        SvTVContextDisableAV(ctx);
    }
}

SvLocal void
SvTVContextNPvrRecordingChanged(SvGenericObject self_, QBnPVRRecording recording)
{
    SvTVContext ctx = (SvTVContext) self_;

    if (!ctx->super_.window) {
        return;
    }

    SvEPGEvent event = NULL;
    SvTVContextGetCurrentEvent(ctx, ctx->channel, SvTimeNow(), &event, NULL);
    if (!event || !SvObjectEquals((SvObject) event, (SvObject) recording->event)) {
        return;
    }

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (recording->state == QBnPVRRecordingState_active) {
        SvInvokeInterface(TVOSD, tvInfo->OSD, setRecordingStatus, true);
    } else if (!ctx->currentRecording) {
        SvInvokeInterface(TVOSD, tvInfo->OSD, setRecordingStatus, false);
    }
}

SvLocal void
SvTVContextCustomerInfoMonitorInfoChanged(SvGenericObject self_, SvString customerId, SvHashTable customerInfo)
{
    SvTVContext ctx = (SvTVContext) self_;

    if (!customerInfo)
        return;

    SvValue pauseLiveTVEnabled = (SvValue) SvHashTableFind(customerInfo, (SvObject) SVSTRING("pause_live_tv_enabled"));
    if (pauseLiveTVEnabled && SvObjectIsInstanceOf((SvObject) pauseLiveTVEnabled, SvValue_getType()) && SvValueIsBoolean(pauseLiveTVEnabled)) {
        ctx->pauseLiveTVEnabled = SvValueGetBoolean(pauseLiveTVEnabled);
    } else {
        SvLogWarning("%s() Expected <pause_live_tv_enabled> wasn't provided", __func__);
    }
}

SvLocal void
SvTVContextCustomerInfoMonitorStatusChanged(SvGenericObject self_)
{
}

SvLocal bool
SvTVContextUpdateNPvrRecordingStatus(SvTVContext ctx)
{
    if (!ctx->super_.window) {
        return false;
    }

    SvEPGEvent event = NULL;
    SvTVContextGetCurrentEvent(ctx, ctx->channel, SvTimeNow(), &event, NULL);
    if (!event) {
        return false;
    }

    SvArray recordings = SvArrayCreate(NULL);
    SvInvokeInterface(QBnPVRProvider, ctx->appGlobals->nPVRProvider, listRecordingsByEvent, event, recordings);
    int count = SvArrayCount(recordings);
    SVRELEASE(recordings);

    if (count > 0) {
        TVInfo *tvInfo = ctx->super_.window->prv;
        SvInvokeInterface(TVOSD, tvInfo->OSD, setRecordingStatus, true);
        return true;
    }

    return false;
}

SvLocal void SvTVContextModifyRecordingIndicator(SvTVContext ctx)
{
    if (!ctx->super_.window)
        return;

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (ctx->currentRecording) {
        SvInvokeInterface(TVOSD, tvInfo->OSD, setRecordingStatus, true);
        return;
    } else if (ctx->appGlobals->nPVRProvider &&
               SvTVContextUpdateNPvrRecordingStatus(ctx)) {
        return;
    }

    SvInvokeInterface(TVOSD, tvInfo->OSD, setRecordingStatus, false);
}

SvLocal void
SvTVContextPVRRecordingChanged(SvGenericObject self_, QBPVRRecording recording, QBPVRRecording oldRecording)
{
    SvTVContext ctx = (SvTVContext) self_;

    if (!ctx->super_.window) {
        return;
    }

    SVTESTRELEASE(ctx->currentRecording);
    ctx->currentRecording = NULL;
    if (ctx->channel && ctx->appGlobals->pvrProvider) {
        ctx->currentRecording = SVTESTRETAIN(QBPVRProviderGetCurrentRecordingForChannel(ctx->appGlobals->pvrProvider, ctx->channel));
    }

    SvTVContextModifyRecordingIndicator(ctx);
}

SvLocal void
SvTVContextPVRRecordingRestricted(SvObject self_,
                                  QBPVRRecording rec)
{
    SvTVContext ctx = (SvTVContext) self_;
    if (!ctx || !ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    SvString channel = rec->channelName;
    if (!tvInfo->recordingBlockedPopup) {
        QBTVShowRecordingBlockedPopup(tvInfo, channel);
    }
}

SvLocal void
SvTVContextPVRRecordingAddedRemoved(SvGenericObject self_, QBPVRRecording recording)
{
    SvTVContextPVRRecordingChanged(self_, recording, NULL);
}

SvLocal void
SvTVContextPVRQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
SvTVContextPVRDirectoryAdded(SvObject self_, QBPVRDirectory dir)
{
}

SvLocal void SvTVContextPCSetChannel(SvTVContext ctx, SvTVChannel channel)
{
    if (ctx->waitingForAuthentication) {
        SvTVContextFinishAuthentication(ctx);
    }

    QBParentalControlHelperReset(ctx->pc);
    QBParentalControlHelperSetChannel(ctx->pc, channel);

    if (channel) {
        QBParentalControlHelperSetEventSource(ctx->pc, ctx, SvTVContextGetCurrentEvent);
        QBParentalControlHelperUpdateRating(ctx->pc);
        QBParentalControlLogicUpdateState(ctx->appGlobals->parentalControlLogic, ctx->pc);
        QBParentalControlHelperCheck(ctx->pc);

        // channel != NULL -> pin popup may be needed
        SvTVContextAuthenticationStateChanged(ctx);
    } else {
        // channel == NULL -> switching to prv probably
        QBParentalControlHelperSetEventSource(ctx->pc, NULL, NULL);
        QBParentalControlLogicStopHelperMonitoring(ctx->appGlobals->parentalControlLogic, ctx->pc);
        QBParentalControlHelperDeactivate(ctx->pc);

        SvTVContextFinishAuthentication(ctx);
    }
}

SvLocal void QBTVContextUpdateRedButtonOverlayAndInfo(TVInfo *self);

void SvTVContextSetChannel(QBWindowContext ctx_, SvTVChannel channel, SvString listID)
{
    SvTVContext ctx = (SvTVContext) ctx_;

    //If no change, do nothing
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    if (ctx->channel == channel && SvObjectEquals((SvObject) QBPlaylistManagerGetCurrent(playlists), (SvObject) listID)) {
        return;
    }

    SvTVContextPCSetChannel(ctx, channel);

    //HACK, we need to use playlistCursor
    //It is because radio might have already set it's own 'current' playlist
    if (listID)
        QBPlaylistManagerSetCurrent(playlists, listID);

    SvTVContextDeactivateConnectionMonitor(ctx);

    if (channel && ctx->channel && ctx->channel != ctx->previous) {
        SvTVChannel prev = SVTESTRETAIN(ctx->channel);
        SVTESTRELEASE(ctx->previous);
        ctx->previous = prev;
        SvTVContextEPGEventChanged(ctx, ctx->previous);
        SvString prevList = SVTESTRETAIN(ctx->listID);
        SVTESTRELEASE(ctx->previousList);
        ctx->previousList = prevList;
    }
    if (!channel && ctx->channel) {
        SvTVContextEPGEventChanged(ctx, ctx->channel);
    }
    SVTESTRELEASE(ctx->channel);
    ctx->channel = SVTESTRETAIN(channel);

    SVTESTRELEASE(ctx->listID);
    ctx->listID = SVTESTRETAIN(listID);

    SvGenericObject current = ctx->listID ? QBPlaylistManagerGetById(playlists, ctx->listID) : NULL;
    int channum = ctx->channel && current ? SvInvokeInterface(SvEPGChannelView, current, getChannelIndex, ctx->channel) : 0;
    if (channum < 0)
        channum = 0;
    ctx->channelIdx = channum;

    SVTESTRELEASE(ctx->currentRecording);
    ctx->currentRecording = NULL;

    if (ctx->channel) {
        QBParentalControlHelperSetEventSource(ctx->pc, ctx, SvTVContextGetCurrentEvent);
        QBParentalControlHelperCheck(ctx->pc);
    } else {
        QBParentalControlHelperSetEventSource(ctx->pc, NULL, NULL);
        QBParentalControlHelperDeactivate(ctx->pc);
    }

    if (ctx->listID) {
        QBPlaylistCursor cursor = QBPlaylistCursorsGet(ctx->appGlobals->playlistCursors, SVSTRING("TV"), ctx->listID);
        QBPlaylistCursorsUpdate(ctx->appGlobals->playlistCursors, cursor, ctx->channel);
    }
    /// Set all widgets
    if (ctx->super_.window) {
        TVInfo *tvInfo = ctx->super_.window->prv;

        SvValue subsLangVal = (SvValue) SvTVChannelGetAttribute(ctx->channel, SVSTRING("subsLang"));
        QBLangMenuSetSubsLang(tvInfo->langMenu, subsLangVal ? SvValueGetString(subsLangVal) : NULL);
        SvTVContextPVRRecordingChanged((SvObject) ctx_, NULL, NULL);

        SvTVContextModifyRecordingIndicator(ctx);

        SvEPGEvent event = NULL;
        SvEPGEvent nextEvent = NULL;
        if (ctx->playerTask)
            SvTVContextGetCurrentEvent(ctx, ctx->channel, SvTimeNow(), &event, NULL);

        if (!tvInfo->playbackProblemPopup && !tvInfo->bufferingPopup) {
            svTVContextChangedPlaySpeed(ctx, 1.0);
        }

        SvTVContextGetCurrentEvent(ctx, ctx->channel, SvTimeNow(), &event, &nextEvent);

        //setTVChannel() bool params: isFavorite
        SvInvokeInterface(TVOSD, tvInfo->OSD, setTVChannel, channel, false);
        SvInvokeInterface(TVOSD, tvInfo->OSD, setCurrentEvent, event);
        SvInvokeInterface(TVOSD, tvInfo->OSD, setFollowingEvent, nextEvent);
        SvInvokeInterface(TVOSD, tvInfo->OSD, setSubsManager, ctx->subsManager);
        //show() bool params: immediate, force
        SvInvokeInterface(TVOSD, tvInfo->OSD, show, true, true);
        if (ctx->redButtonDSMCCDataProvider) {
            QBTVContextUpdateRedButtonOverlayAndInfo(tvInfo);
        }
    }
}

void SvTVContextAttachSubtitle(QBWindowContext ctx_, SvWidget window)
{
    SvTVContext ctx = (SvTVContext) ctx_;

    if (!ctx->subsManager) {
        return;
    }

    if (ctx->subtitlesAttached) {
        return;
    }

    if (window == NULL) {
        SvLogWarning("%s:%d : NULL window passed", __FUNCTION__, __LINE__);
        return;
    }

    if (QBParentalControlLogicGetAuthState(ctx->appGlobals->parentalControlLogic, ctx->pc) != QBParentalControlLogicAuthState_EnableAV) {
        return;
    }

    ctx->showSubtitles = true;
    ctx->windowForSubtitles = window;

    QBSubsManagerAttach(ctx->subsManager, window, 0, 0, window->width, window->height, 0);
    ctx->subtitlesAttached = true;
}

void SvTVContextDetachSubtitle(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;

    if (!ctx->subsManager) {
        return;
    }

    if (!ctx->subtitlesAttached) {
        return;
    }

    ctx->showSubtitles = false;
    ctx->windowForSubtitles = NULL;

    QBSubsManagerDetach(ctx->subsManager);
    ctx->subtitlesAttached = false;
}

void SvTVContextShowSubtitles(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    SvTVContextUpdateSubtitlesState(ctx, false);
}

SvLocal void SvTVContextSetAudioTrack(void *ctx_, SvPlayerAudioTrack audioTrack)
{
    SvTVContext ctx = ctx_;

    if (!ctx->channel)
        return;

    if (!audioTrack) {
        SvTVChannelRemoveAttribute(ctx->channel, SVSTRING("audioLang"));
    } else {
        SvTVChannelSetAttribute(ctx->channel, SVSTRING("audioLang"), (SvGenericObject) audioTrack);
    }

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    SvEPGManagerPropagateChannelChange(epgManager, ctx->channel, NULL);
}

SvLocal void SvTVContextSetSubsLang(void *ctx_, QBSubsTrack lang)
{
    SvTVContext ctx = ctx_;
    if (!ctx->channel)
        return;

    if (!lang) {
        SvTVChannelRemoveAttribute(ctx->channel, SVSTRING("subsLang"));
    } else {
        SvValue langVal = SvValueCreateWithString (lang->langCode, NULL);
        SvTVChannelSetAttribute(ctx->channel, SVSTRING("subsLang"), (SvGenericObject) langVal);
        SVRELEASE(langVal);
    }

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    SvEPGManagerPropagateChannelChange(epgManager, ctx->channel, NULL);
}

SvLocal void tvManualChannelSelection(void *ptr, SvTVChannel channel)
{
    SvWidget tv = ptr;
    TVInfo *tvInfo = tv->prv;
    SvTVContext ctx = tvInfo->ctx;
    
    // BEGIN RAL TimeShift cancel pop-up window
    if (QBTVTimeShiftCancelPopUpShowApplies((QBWindowContext)ctx))
    {
        QBTVTimeShiftCancelPopUpShow(NULL, (QBWindowContext)ctx, '0', channel->number);
    }
    else
    {
        QBTVLogicPlayChannel(ctx->appGlobals->tvLogic, channel, SVSTRING("ManualSelection"));
        SvInvokeInterface(TVOSD, tvInfo->OSD, setTVChannel, channel, SVSTRING("ManualSelection"));
    }
    // END RAL TimeShift cancel pop-up window
}

SvLocal void tvResolverQuestionCallback(void *_ctx)
{
    QBTVExtendedInfoHide(_ctx);

    SvTVContext ctx = (SvTVContext)_ctx;
    // assert - help for clang false positive
    assert(ctx->super_.window);
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (!SvInvokeInterface(TVOSD, tvInfo->OSD, isSideMenuShown)) {
        SvInvokeInterface(TVOSD, tvInfo->OSD, showSideMenu, NULL, NULL);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_PVR_RESOLVER_WINDOW_ID));
        }
    }
}

bool svTVContextIsCurrentlyRecording(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (ctx->currentRecording == NULL)
        return false;

    return true;
}

void svTVContextStopRecording(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (ctx->currentRecording) {
        QBPVRProviderStopRecording(ctx->appGlobals->pvrProvider, ctx->currentRecording);
    }
}

bool SvTVContextCanNPvrRecordingBeStarted(QBWindowContext ctx_)
{
    SvTVContext self = (SvTVContext)ctx_;
    SvEPGEvent event = NULL;
    SvTVContextGetCurrentEvent(self, self->channel, SvTimeNow(), &event, NULL);

    if (!event) {
        return false;
    }

    if (!SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, canScheduleEventRecording, event)) {
        return false;
    }

    TVInfo *tvInfo = self->super_.window->prv;
    if (SvInvokeInterface(TVOSD, tvInfo->OSD, isSideMenuShown)) {
        return false;
    }

    return true;
}

SvLocal void QBTVShowRecordingBlockedPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
  TVInfo *tvInfo = (TVInfo*) self_;
  tvInfo->recordingBlockedPopup = NULL;
}

SvLocal void QBTVShowRecordingBlockedPopup(TVInfo *tvInfo, SvString channel)
{
    SvApplication app = tvInfo->ctx->super_.window->app;
    svSettingsPushComponent("Dialog.settings");
    QBDialogParameters params = {
        .app        = app,
        .controller = tvInfo->ctx->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    tvInfo->recordingBlockedPopup = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    tvInfo->recordingBlockedPopup->off_x = (tvInfo->ctx->super_.window->width - tvInfo->recordingBlockedPopup->width) / 2;
    tvInfo->recordingBlockedPopup->off_y = (tvInfo->ctx->super_.window->height - tvInfo->recordingBlockedPopup->height) / 2;
    QBDialogAddButton(tvInfo->recordingBlockedPopup, SVSTRING("OK-button"), "OK", 1);
    QBDialogAddPanel(tvInfo->recordingBlockedPopup, SVSTRING("content"), NULL, 1);
    char* buf = NULL;
    asprintf(&buf, gettext("Recording channel %s is not available due to broadcaster's limitations"), SvStringCString(channel));
    QBDialogAddLabel(tvInfo->recordingBlockedPopup, SVSTRING("content"), buf, SVSTRING("text"), 1);
    free(buf);
    svSettingsPopComponent();
    QBDialogRun(tvInfo->recordingBlockedPopup, tvInfo, QBTVShowRecordingBlockedPopupCallback);
}

void SvTVContextStartRecording(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;

    QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(ctx->appGlobals->appState);
    if ((diskState != QBPVRDiskState_pvr_present && diskState != QBPVRDiskState_pvr_ts_present)
        && !QBAppTypeIsPVR())
        return;

    TVInfo *ti = ctx->super_.window->prv;
    if (SvInvokeInterface(TVOSD, ti->OSD, isSideMenuShown))
        return;

    if (ctx->currentRecording || !ctx->channel)
        return;

    if (!ctx->appGlobals->pvrProvider)
        return;

    if (QBPVRProviderGetCapabilities(ctx->appGlobals->pvrProvider)->type != QBPVRProviderType_disk)
        return;

    time_t now = SvTimeNow();
    //One time recording
    QBPVRRecordingSchedParams params = (QBPVRRecordingSchedParams)
              SvTypeAllocateInstance(QBPVRRecordingSchedParams_getType(), NULL);
    params->channel = SVRETAIN(ctx->channel);
    params->startTime = SvTimeGetCurrentTime();
    params->type = QBPVRRecordingType_OTR;

    //end time heuristic
    int maxDuration = 3600*2;
    SvEPGEvent current = NULL;
    SvTVContextGetCurrentEvent(ctx, ctx->channel, now, &current, NULL);
    if (current) {
        int end = current->endTime - now;
        if (maxDuration < end)
            maxDuration = end;
    }
    params->duration = maxDuration;

    TVInfo *tvInfo = ctx->super_.window->prv;
    SVTESTRELEASE(tvInfo->resolver);
    tvInfo->resolver = NULL;
    SvObject sideMenuCtx = SvInvokeInterface(TVOSD, tvInfo->OSD, getSideMenuContext);
    if (sideMenuCtx && SvObjectIsInstanceOf(sideMenuCtx, QBContextMenu_getType())) {
        tvInfo->resolver = QBPVRConflictResolverCreate(ctx->appGlobals, (QBContextMenu) sideMenuCtx, params, 1);
        QBPVRConflictResolverSetQuestionCallback(tvInfo->resolver, tvResolverQuestionCallback, ctx);
        QBPVRConflictResolverResolve(tvInfo->resolver);
    }
    SVRELEASE(params);

    if (ctx->appGlobals->casPVRManager) {
        SvInvokeInterface(QBCASPVRManager, ctx->appGlobals->casPVRManager, recordingAdded, "SvTVContextStartRecording");
    }
}

SvLocal void tvOSDCheckForChanges(SvWidget tv)
{
    TVInfo *tvInfo = tv->prv;
    SvTVContext ctx = tvInfo->ctx;
    bool isFastUpdateEnabled = false;

    time_t timeshiftRangeStart = 0;
    time_t timeshiftRangeEnd = 0;
    time_t currentPosition = SvTimeNow();

    if (ctx->playerTask) {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        if (state.timeshift.recording || state.timeshift.enabled) {
            currentPosition = ctx->timeshiftReferenceTime + state.currentPosition;
            if (ctx->timeshiftFromRecStartTime) {
                // in pvr timeshift range is in the same time scale as player position
                // e.g. player pos: 7, range_start: 3, range_end: 23 [20sec pvr window]
                timeshiftRangeStart = ctx->timeshiftReferenceTime + state.timeshift.range_start;
                timeshiftRangeEnd = ctx->timeshiftReferenceTime + state.timeshift.range_end;
            } else {
                // in remote timeshift range is relative to player position
                // same e.g. player pos: 7, range_start: 4 (7 - 4 = 3),
                // range_end: -16 (7 - (-16) = 23) [same 20s pvr window]
                timeshiftRangeStart = currentPosition - state.timeshift.range_start;
                timeshiftRangeEnd = currentPosition - state.timeshift.range_end;
            }
            // make sure currentPosition is in <start, end> (connection errors, REW, etc)
            if (timeshiftRangeStart > currentPosition)
                timeshiftRangeStart = currentPosition;
            if (timeshiftRangeEnd < currentPosition)
                timeshiftRangeEnd = currentPosition;
        }
        if (abs(state.currentSpeed) >= BOTTOM_OSD_UPDATE_SPEED_THRESHOLD_SEC) {
            isFastUpdateEnabled = true;
        } else {
            isFastUpdateEnabled = false;
        }
    }

    QBOSD osd = QBOSDMainGetHandler(tvInfo->OSD, QBOSDHandlerType_recording, NULL);
    if (osd && ctx->playerTask)
        QBOSDRecordingUpdateTimes((QBOSDRecording) osd, ctx->channel, currentPosition,
                                  timeshiftRangeStart, timeshiftRangeEnd, NULL);
    SvTVContextModifyRecordingIndicator(ctx);

    if (isFastUpdateEnabled) {
        tvInfo->updateTimer = svAppTimerStart(tv->app, tv, BOTTOM_OSD_FAST_UPDATE_PERIOD_SEC, 1);
    } else {
        tvInfo->updateTimer = svAppTimerStart(tv->app, tv, BOTTOM_OSD_NORMAL_UPDATE_PERIOD_SEC, 1);
    }

    SvEPGEvent currentEvent = NULL;
    SvEPGEvent nextEvent = NULL;

    if (ctx->playerTask) {
        SvTVContextGetCurrentEvent(ctx, ctx->channel, currentPosition, &currentEvent, &nextEvent);
        SvTimeRange timeRange;
        SvTimeRangeInit(&timeRange, 0, 0);
        SvValue channelID = NULL;
        SvValue eventChannelID = NULL;
        if (ctx->channel) {
            channelID = SvTVChannelGetID(ctx->channel);
        }
        if (ctx->events.current) {
            SvEPGEventGetTimeRange(ctx->events.current, &timeRange, NULL);
            eventChannelID = ctx->events.current->channelID;
        }
        if (currentEvent != ctx->events.current) {
            SvTVContextEPGEventChanged(ctx, ctx->channel);
        }
        if (currentEvent) {
            SVTESTRELEASE(ctx->events.current);
            SVTESTRELEASE(ctx->events.following);
            ctx->events.current = SVRETAIN(currentEvent);
            ctx->events.following = SVTESTRETAIN(nextEvent);
        } else if (SvTimeRangeContainsTimePoint(&timeRange, currentPosition) && SvObjectEquals((SvObject) channelID, (SvObject) eventChannelID)) {
            currentEvent = ctx->events.current;
            nextEvent = ctx->events.following;
        } else {
            SVTESTRELEASE(ctx->events.current);
            SVTESTRELEASE(ctx->events.following);
            ctx->events.current = NULL;
            ctx->events.following = NULL;
        }
        if (currentEvent || nextEvent) {
            SvArray events = SvArrayCreate(NULL);
            if (currentEvent) {
                SvArrayAddObject(events, (SvGenericObject) currentEvent);
            }
            if (nextEvent) {
                SvArrayAddObject(events, (SvGenericObject) nextEvent);
            }
            QBExtendedInfoRefreshEvents(tvInfo->extendedInfo, events);
            SVRELEASE(events);
        }
    }

    SvInvokeInterface(TVOSD, tvInfo->OSD, setCurrentEvent, currentEvent);
    SvInvokeInterface(TVOSD, tvInfo->OSD, setFollowingEvent, nextEvent);
    
    // BEGIN AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
    if (tvInfo->sendStartUsageLog && (time(NULL) - tvInfo->sendStartUsageLogTimeStamp > 10))
    {
        struct sv_tuner_state* tuner = sv_tuner_get(0);
        struct QBTunerStatus status;
        if(!sv_tuner_get_status(tuner, &status))
        {
            BoldUsageLog(LOG_INFO, "\"App.TV\":{\"start\":\"%d\",\"tuner%i\":{\"frequency\":%i,\"symbol_rate\":%i,\"signalStrength\":%i,\"signalQuality\":%i,\"context\":\"%s\"}}",
                            tvInfo->ctx->channel->number, 0,
                            status.params.mux_id.freq, status.params.symbol_rate, status.signal_strength, status.signal_quality, 
                            SvStringCString(BoldInputLogGetCurrentContext()));
            tvInfo->sendStartUsageLog = false;
        }
    }
    // END AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
}

SvLocal void tvTimerEventHandler(SvWidget tv, SvTimerEvent te)
{
   TVInfo *tvInfo = tv->prv;
   if (te->id == tvInfo->updateTimer) {
      tvOSDCheckForChanges(tv);
      return;
   }
}

SvLocal bool tvTimeshiftInputEventHandler(TVInfo *tvInfo, SvInputEvent e)
{
    SvTVContext ctx = tvInfo->ctx;
    if (!ctx->playerTask) {
        return false;
    }
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    SvPlayerTaskCapabilities capabilities = SvPlayerTaskGetCapabilities(ctx->playerTask);
    bool isRemoteTimeshiftAllowed = capabilities.remote_timeshift == SvPlayerTaskCapability_yes && ctx->pauseLiveTVEnabled;

    if (!isRemoteTimeshiftAllowed &&
        !QBTVLogicIsPVRTimeshiftAllowedForChannel(ctx->appGlobals->tvLogic, ctx->channel)) {
        return false;
    }

    if ((e->ch == QBKEY_PLAYPAUSE || e->ch == QBKEY_PLAY || e->ch == QBKEY_PAUSE ||
         (isRemoteTimeshiftAllowed && (e->ch == QBKEY_REW || e->ch == QBKEY_FFW))) && !state.timeshift.wanted)
    {
        if (isRemoteTimeshiftAllowed) {
            SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
            state = SvPlayerTaskGetState(ctx->playerTask);
            ctx->timeshiftReferenceTime = SvTimeNow() - state.currentPosition;
            ctx->timeshiftFromRecStartTime = false;
            if (e->ch == QBKEY_PLAYPAUSE || e->ch == QBKEY_PLAY || e->ch == QBKEY_PAUSE) {
                svTVContextChangedPlaySpeed(ctx, 1.0);
                QBTVEventPaused(ctx);
            } else if (e->ch == QBKEY_REW) {
                SvTVContextREW((QBWindowContext) ctx);
            } else if (e->ch == QBKEY_FFW) {
                SvTVContextFFW((QBWindowContext) ctx);
            }
        } else {
            QBAppPVRDiskState diskState = QBAppStateServiceGetPVRDiskState(ctx->appGlobals->appState);
            if (diskState == QBPVRDiskState_ts_present || diskState == QBPVRDiskState_pvr_ts_present) {
                if (!state.timeshift.recording) {
                    SvPlayerTaskEnableTimeshiftRecording(ctx->playerTask, true);

                    state = SvPlayerTaskGetState(ctx->playerTask);
                    ctx->timeshiftReferenceTime = SvTimeNow() - state.currentPosition;
                    ctx->timeshiftFromRecStartTime = true;
                }
                if (e->ch == QBKEY_PLAYPAUSE || e->ch == QBKEY_PAUSE)
                    SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
                svTVContextChangedPlaySpeed(ctx, 0.0);
                QBTVEventPaused(ctx);
            }
        }
        return true;
    }

    if (!state.timeshift.recording) {
        return false;
    }

    switch(e->ch) {
        case QBKEY_PLAYPAUSE:
            if (!state.timeshift.enabled)
                SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
            if (state.wantedSpeed != 1.0) {
                SvPlayerTaskPlay(ctx->playerTask, 1.0, -1.0, NULL);
                svTVContextChangedPlaySpeed(ctx, 1.0);
                QBTVEventResumed(ctx);
            } else {
                SvPlayerTaskPlay(ctx->playerTask, 0.0, -1.0, NULL);
                svTVContextChangedPlaySpeed(ctx, 0.0);
                QBTVEventPaused(ctx);
            }
            return true;
        case QBKEY_PLAY:
            if (!state.timeshift.enabled)
                SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
            if (state.wantedSpeed != 1.0) {
                SvPlayerTaskPlay(ctx->playerTask, 1.0, -1.0, NULL);
            }
            svTVContextChangedPlaySpeed(ctx, 1.0);
            QBTVEventResumed(ctx);
            return true;
        case QBKEY_PAUSE:
            if (!state.timeshift.enabled)
                SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
            SvPlayerTaskPlay(ctx->playerTask, 0.0, -1.0, NULL);
            svTVContextChangedPlaySpeed(ctx, 0.0);
            QBTVEventPaused(ctx);
            return true;
        case QBKEY_REW:
            if (!state.timeshift.enabled)
                SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
            SvTVContextREW((QBWindowContext) ctx);
            return true;
        case QBKEY_FFW:
            if (!state.timeshift.enabled)
                SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);
            SvTVContextFFW((QBWindowContext) ctx);
            return true;
    }

    return false;
}

void SvTVContextEnterLive(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->playerTask)
        return;
    SvPlayerTaskState state  = SvPlayerTaskGetState(ctx->playerTask);
    if (!state.timeshift.recording || !state.timeshift.wanted)
        return;

    // sending 0.0 for live contents is needed to seek to "live" position
    // (timeshift disabling operation is handled above Smooth/DASH handlers)
    SvPlayerTaskPlay(ctx->playerTask, 1.0, 0.0, NULL);

    if (state.timeshift.enabled) {
        SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, false);
    }

    svTVContextChangedPlaySpeed(ctx, 1.0);
}

void SvTVContextEnableTimeshift(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->playerTask)
        return;
    SvPlayerTaskState state  = SvPlayerTaskGetState(ctx->playerTask);
    if (state.timeshift.recording)
        return;
    SvPlayerTaskEnableTimeshiftRecording(ctx->playerTask, true);

    state = SvPlayerTaskGetState(ctx->playerTask);
    ctx->timeshiftReferenceTime = SvTimeNow() - state.currentPosition;
    // only pvr timeshift here
    ctx->timeshiftFromRecStartTime = true;
}

void SvTVContextDisableTimeshift(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->playerTask)
        return;
    SvPlayerTaskEnableTimeshiftRecording(ctx->playerTask, false);
}

SvLocal bool tvInputEventHandler(SvWidget tv, SvInputEvent ie)
{
    TVInfo *tvInfo = (TVInfo *)tv->prv;
    SvTVContext ctx = tvInfo->ctx;

    SvErrorInfo error = NULL;
    bool isMuted = QBVolumeIsMuted(ctx->appGlobals->volume, &error);
    if ((ie->ch == QBKEY_MUTE || ie->ch == QBKEY_VOLUP || ie->ch == QBKEY_VOLDN) && isMuted) {
        if (ctx->playbackMonitor) {
            QBPlaybackMonitorVolumeUpdate(ctx->playbackMonitor, false);
        }
    } else if (ie->ch == QBKEY_MUTE && (!isMuted || error)) {
        if (ctx->playbackMonitor) {
            QBPlaybackMonitorVolumeUpdate(ctx->playbackMonitor, true);
        }
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
        }
    }

    QBTimeLimitBegin(timeSvTVContext, QBTimeThreshold);

    bool ret = SvInvokeInterface(TVOSD, tvInfo->OSD, handleInputEvent, ie);
    QBTimeLimitEnd_(timeSvTVContext, "SvTVContext.TVOSD");

    if (ret) {
        SvInvokeInterface(TVOSD, tvInfo->OSD, show, false, false);
        QBTimeLimitEnd_(timeSvTVContext, "SvTVContext.TVOSD.show");
        return ret;
    }

    ret = QBTVLogicHandleInputEvent(ctx->appGlobals->tvLogic, ie);
    QBTimeLimitEnd_(timeSvTVContext, "SvTVContext.tvLogic");
    if (ret)
        return ret;

    ret = tvTimeshiftInputEventHandler(tvInfo, ie);
    QBTimeLimitEnd_(timeSvTVContext, "SvTVContext.timeshift");
    if (ret)
        return ret;

    switch (ie->ch) {
        case QBKEY_LOOP:
        case QBKEY_HISTORY: {
            // BEGIN RAL TimeShift cancel pop-up window
            if(QBTVTimeShiftCancelPopUpShowApplies((QBWindowContext)ctx))
            {
                QBTVTimeShiftCancelPopUpShow(NULL, (QBWindowContext)ctx, QBKEY_LOOP, -1);
            }
            else
            {
                if (ctx->previous && ctx->previousList) {
                    QBTVLogicPlayPreviousChannel(ctx->appGlobals->tvLogic);
                    QBTimeLimitEnd_(timeSvTVContext, "SvTVContext.previousChannel");
                    ret = true;
                }
            }
            // END RAL TimeShift cancel pop-up window
            break;
        }
        default:
            return false;
    }

    return ret;
}

SvTVChannel SvTVContextGetCurrentChannel(QBWindowContext ctx_)
{
   SvTVContext ctx = (SvTVContext) ctx_;
   return ctx->channel;
}

void SvTVContextSetCurrentEventSourcesPriority(QBWindowContext ctx_, SvTVContextCurrentEventSourcesPriority priority)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    ctx->currentEventSourcesPriority = priority;
}

SvLocal void SvTVContextDisablePC(SvTVContext ctx)
{
    QBParentalControlLogicStopHelperMonitoring(ctx->appGlobals->parentalControlLogic, ctx->pc);
    QBParentalControlHelperDeactivate(ctx->pc);

    if (ctx->waitingForAuthentication) {
        SvTVContextFinishAuthentication(ctx);
    }
}

SvLocal void QBTVOutputFormatChanged(void *self_, QBViewport qbv)
{
    QBTVUpdateFormat((QBWindowContext)self_);
}

SvLocal void QBTVVideoInfoChanged(void *self_, QBViewport qbv, const QBViewportVideoInfo *videoInfo)
{
    SvTVContext ctx = (SvTVContext) self_;

    QBTVLogicMetaDataManagers metaDataMgrs = QBTVLogicGetMetaDataManagers(ctx->appGlobals->tvLogic);

    if (!metaDataMgrs->dvbSubsManager || !videoInfo)
        return;

    QBDvbSubsManagerDDSMode mode = QBDvbSubsManagerDDSMode_default;
    int rate = videoInfo->frameRate;
    if (rate == 50000 || rate == 25000) {
        mode = QBDvbSubsManagerDDSMode_PAL;
    } else if ((23970 < rate && rate < 23990) || (29960 < rate && rate < 29980) || (59930 < rate && rate < 59950)) {
        mode = QBDvbSubsManagerDDSMode_NTSC;
    } else if (videoInfo->height == 576) {
        mode = QBDvbSubsManagerDDSMode_PAL;
    } else if (videoInfo->height == 480) {
        mode = QBDvbSubsManagerDDSMode_NTSC;
    }

    ctx->dvbSubsManagerDDSMode = mode;
    QBDvbSubsManagerSetDDSMode(metaDataMgrs->dvbSubsManager, mode);

    if (metaDataMgrs->scte27SubsManager) {
        bool inNTSC = (ctx->dvbSubsManagerDDSMode == QBDvbSubsManagerDDSMode_NTSC);
        QBScte27SubsManagerEnableNTSCHack(metaDataMgrs->scte27SubsManager, inNTSC);
    }
}

static struct qb_viewport_callbacks viewportCallbacks = {
    .output_format_changed = QBTVOutputFormatChanged,
    .video_info_changed = QBTVVideoInfoChanged
};

SvLocal QBDSMCCDataProvider SvTVContextCreateRedButtonDSMCCDataProvider(AppGlobals appGlobals)
{
    QBDSMCCClient dsmccClient = QBDSMCCMonitorGetClient(appGlobals->dsmccMonitor);
    if (!dsmccClient) {
        SvLogError("TVContext: could not get DSM-CC Client");
        return NULL;
    }

    QBDSMCCDataProvider redButtonDSMCCDataProvider = QBDSMCCDataProviderCreate(dsmccClient, SVSTRING("red_button"));
    if (!redButtonDSMCCDataProvider) {
        SvLogError("TVContext: creating redButtonDSMCCDataProvider failed");
        return NULL;
    }

    QBContentProviderStart((QBContentProvider) redButtonDSMCCDataProvider, appGlobals->scheduler);

    return redButtonDSMCCDataProvider;
}

QBWindowContext SvTVContextCreate(AppGlobals appGlobals)
{
    SvTVContext ctx = (SvTVContext) SvTypeAllocateInstance(SvTVContext_getType(), NULL);

    ctx->playerHandler = (SvTVContextPlayHandler) SvTypeAllocateInstance(SvTVContextPlayHandler_getType(), NULL);
    ctx->playerHandler->ctx = ctx;
    ctx->appGlobals = appGlobals;
    ctx->listID = SVSTRING("TVChannels");
    ctx->waitingForAuthentication = false;
    ctx->pc = QBParentalControlHelperNew(appGlobals);
    QBParentalControlHelperSetCallback(ctx->pc, SvTVContextAuthenticationStateChanged, ctx);
    QBParentalControlLogicAddParentalControlHelper(ctx->appGlobals->parentalControlLogic, ctx->pc);

    ctx->subsManager = QBTVLogicCreateSubsManager(ctx->appGlobals->tvLogic);

    if (ctx->appGlobals->pvrProvider) {
        QBPVRProviderAddListener(ctx->appGlobals->pvrProvider, (SvObject) ctx);
    }

    if (ctx->appGlobals->nPVRProvider) {
        SvInvokeInterface(QBnPVRProvider, ctx->appGlobals->nPVRProvider, addRecordingListener, (SvGenericObject) ctx);
    }

    if (ctx->appGlobals->customerInfoMonitor) {
       CubiwareMWCustomerInfoAddListener(ctx->appGlobals->customerInfoMonitor, (SvGenericObject) ctx);
    }

    ctx->tunerMonitor = SVTESTRETAIN((QBTunerMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                  SVSTRING("QBTunerMonitor")));

    QBViewportAddCallbacks(QBViewportGet(), &viewportCallbacks, ctx);

    ctx->dvbSubsManagerDDSMode = QBDvbSubsManagerDDSMode_default;

    svSettingsPushComponent("ContextSwitcher.settings");
    ctx->flushScreenOnContextSwitch = svSettingsGetBoolean("ContextSwitcher", "flushScreen", true);
    svSettingsPopComponent();

    if (appGlobals->dsmccMonitor) {
        ctx->redButtonDSMCCDataProvider = SvTVContextCreateRedButtonDSMCCDataProvider(appGlobals);
        ctx->adsRotor = QBDSMCCAdsRotationServiceCreate();
        QBDSMCCAdsRotationServiceAddListener(ctx->adsRotor, (SvObject) ctx);
    }
    ctx->playbackMonitor = (QBPlaybackMonitor) SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBPlaybackMonitor")));

    QBEventBus bus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiver(bus, (SvObject) ctx, QBSideMenuLevelChangedEvent_getType(), NULL);

    return (QBWindowContext) ctx;
}

SvLocal void SvTVContextStopPlaying(SvGenericObject ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    int64_t duration = 0;

    if (ctx->playbackMonitor && ctx->playerTask) {
        QBPlaybackMonitorPlaybackStoped(ctx->playbackMonitor, &duration, NULL);
    }

    SVTESTRELEASE(ctx->events.current);
    SVTESTRELEASE(ctx->events.following);
    ctx->events.current = NULL;
    ctx->events.following = NULL;

    QBTVLogicStopMetaDataManagers(ctx->appGlobals->tvLogic);

    if (ctx->subsManager) {
        QBSubsManagerSetPlayerTask(ctx->subsManager, NULL);
    }

    if (ctx->appGlobals->nPVRProvider) {
        SvInvokeInterface(QBnPVRProvider, ctx->appGlobals->nPVRProvider, removeRecordingListener, (SvGenericObject) ctx);
    }

    CubiwareMWCustomerInfoRemoveListener(ctx->appGlobals->customerInfoMonitor, (SvGenericObject) ctx);

    if (ctx->super_.window) {
        TVInfo *tvInfo = ctx->super_.window->prv;
        QBTeletextWindowSetReceiver(tvInfo->teletext, NULL);
        QBTeletextWindowHide(tvInfo->teletext);
        QBLangMenuSetSubsLang(tvInfo->langMenu, NULL);

        if (tvInfo->signalPopup) {
            QBDialogBreak(tvInfo->signalPopup);
            tvInfo->signalPopup = NULL;
        }
        if (tvInfo->parentalPopup) {
            tvCheckPINPopupHide(tvInfo);
            tvInfo->parentalPopup = NULL;
        }
        if (tvInfo->tunerPopup) {
            QBDialogBreak(tvInfo->tunerPopup);
            tvInfo->tunerPopup = NULL;
        }
        if (tvInfo->fatalErrorPopup) {
            QBDialogBreak(tvInfo->fatalErrorPopup);
            tvInfo->fatalErrorPopup = NULL;
        }
        if (tvInfo->notEntitledPopup) {
            QBDialogBreak(tvInfo->notEntitledPopup);
            tvInfo->notEntitledPopup = NULL;
        }

        if (tvInfo->errorPopup) {
            QBDialogBreak(tvInfo->errorPopup);
            tvInfo->errorPopup = NULL;
        }

        if (tvInfo->recordingBlockedPopup) {
            QBDialogBreak(tvInfo->recordingBlockedPopup);
            tvInfo->recordingBlockedPopup = NULL;
        }

        if (ctx->tunerMonitor)
            QBTunerMonitorRemoveListener(ctx->tunerMonitor, (SvGenericObject) tvInfo);
    }

    if (ctx->playerTask) {
        SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
        QBSecureLogEvent("App.TV", "Notice.TVPlayer.Stop", "JSON:{\"description\":\"stop playback\"%s}", data ? SvStringCString(data) : "");
        SVTESTRELEASE(data);
        //  { NR: Log Stop Viewing Channel through BoldUsageLog Module
        BoldUsageLog(LOG_INFO, "\"App.TV\":{\"stop\":\"%d\"}", ctx->channel->number);
        QBTVLogicSetPlayerTask(ctx->appGlobals->tvLogic, NULL);
        AudioTrackLogic audioTrackLogic =
                        (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
        AudioTrackLogicSetPlayerTaskControllers(audioTrackLogic, NULL);
        VideoTrackLogic videoTrackLogic =
                        (VideoTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoTrackLogic"));
        VideoTrackLogicSetPlayerTaskControllers(videoTrackLogic, NULL);
        SubtitleTrackLogic subtitleTrackLogic =
                        (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
        SubtitleTrackLogicSetPlayerTaskControllers(subtitleTrackLogic, NULL);
        SvPlayerTaskClose(ctx->playerTask, NULL);
        SVRELEASE(ctx->playerTask);
        ctx->playerTask = NULL;
        if (ctx->appGlobals->casPopupManager)
            SvInvokeInterface(QBCASPopupManager, ctx->appGlobals->casPopupManager, contentChanged);
    }

    SvTVContextDisablePC(ctx);
    SvTVContextEnableAV(ctx);

    SVTESTRELEASE(ctx->currContent);
    ctx->currContent = NULL;
}

// SvPlayerTaskControllersListener methods

SvLocal void SvTVContextTaskControllersUpdated(SvObject self_)
{
    SvTVContext ctx = (SvTVContext) self_;

    SvPlayerTaskControllers taskControllers =  SvPlayerTaskGetControllers(ctx->playerTask);
    if (!taskControllers) {
        return;
    }
    SvPlayerTrackController trackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_subs);
    if (trackController) {
        SvPlayerTrackControllerAddListener(trackController, (SvObject) ctx, NULL);
    }
}

SvLocal void SvTVContextPlay(SvGenericObject ctx_, SvContent content, double position)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (ctx->currContent) {
        SvLogError("%s: already has playerTask", __func__);
        SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
        QBSecureLogEvent("newtv", "Error.TVPlayer.StartFailed", "JSON:{\"description\":\"%s: already has playerTask\"%s}", __func__,
                         data ? SvStringCString(data) : "");
        SVTESTRELEASE(data);
        return;
    }

    if (!content && !ctx->currContent) {
        SvLogError("%s: no content to be played", __func__);
        QBSecureLogEvent("newtv", "Error.TVPlayer.StartFailed", "JSON:{\"description\":\"%s: no content to be played\"}", __func__);
        return;
    }

    if (content) {
        ctx->currContent = SVRETAIN(content);
    } else {
        goto fini;
    }

    const struct SvPlayerTaskParams_s params = {
        .content = ctx->currContent,
        .standalone = 0,
        .max_memory_use = QBVODLogicGetAllowedSizeOfBuffer(),
        .delegate = (SvGenericObject) ctx->playerHandler,
        .viewport = QBViewportGet(),
        .speed = 1.0,
        .position = position,
        .reencrypt = false,
    };

    if (ctx->appGlobals->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, ctx->appGlobals->casPopupManager, contentChanged);

    SvValue channelId = SvDBObjectGetID((SvDBObject)ctx->channel);
    SvContentMetaData meta = SvContentGetMetaData(ctx->currContent);
    SvContentMetaDataSetObjectProperty(meta, SVSTRING(SV_PLAYER_META__CHANNEL_ID), (SvGenericObject)channelId);
    SvErrorInfo error = NULL;
    ctx->playerTask = SvPlayerManagerCreatePlayerTask(SvPlayerManagerGetInstance(), &params, &error);
    if (!ctx->playerTask) {
        SvLogError("%s: can't create player task", __func__);
        QBSecureLogEvent("newtv", "Error.TVPlayer.StartFailed", "JSON:{\"description\":\"%s: can't create player task\"}", __func__);
        return;
    }

    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("App.TV", "Notice.TVPlayer.Start", "JSON:{\"description\":\"start playback\"%s}", data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    // NR: Log Start Viewing Channel through BoldUsageLog Module
    if (!TVLogicGetSkipLog(ctx->appGlobals->tvLogic)) // AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
    {
        if (ctx->super_.window)
        {
            TVInfo *tvInfo = ctx->super_.window->prv;
            tvInfo->sendStartUsageLog = true;
            tvInfo->sendStartUsageLogTimeStamp = time(NULL);
        }
    }
    else
    {
        TVLogicSetSkipLog(ctx->appGlobals->tvLogic, false); // AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
    }

    SvPlayerTaskControllers taskControllers =  SvPlayerTaskGetControllers(ctx->playerTask);
    if (!taskControllers) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvPlayerTaskGetControllers() failed");
        SvErrorInfoPropagate(error, NULL);
    } else {
        SvPlayerTaskControllersAddListener(taskControllers, (SvObject) ctx, NULL);
    }
    SvPlayerTrackController trackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_subs);
    if (trackController) {
        SvPlayerTrackControllerAddListener(trackController, (SvObject) ctx, &error);
        if (error) {
            error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                               "SvPlayerTrackControllersAddListener() failed");
            SvErrorInfoPropagate(error, NULL);
            goto fini;
        }
    }

    AudioTrackLogic audioTrackLogic =
            (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    AudioTrackLogicSetPlayerTaskControllers(audioTrackLogic, taskControllers);
    VideoTrackLogic videoTrackLogic =
            (VideoTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoTrackLogic"));
    VideoTrackLogicSetPlayerTaskControllers(videoTrackLogic, taskControllers);
    SubtitleTrackLogic subtitleTrackLogic =
            (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
    SubtitleTrackLogicSetPlayerTaskControllers(subtitleTrackLogic, taskControllers);

    QBTVLogicSetPlayerTask(ctx->appGlobals->tvLogic, ctx->playerTask);
    if (error) {
        ctx->channelErrorCode = SvErrorInfoGetCode(error);
        QBTVPlaybackProblemShow(ctx, ctx->channelErrorCode);
        SvErrorInfoPropagate(error, NULL);
    }

    QBParentalControlHelperUpdateRating(ctx->pc);
    QBParentalControlLogicUpdateState(ctx->appGlobals->parentalControlLogic, ctx->pc);
    QBParentalControlHelperCheck(ctx->pc);

    QBParentalControlLogicStartHelperMonitoring(ctx->appGlobals->parentalControlLogic, ctx->pc);
    QBParentalControlHelperActivate(ctx->pc);
    SvTVContextAuthenticationStateChanged(ctx);

    if (SvTVChannelGetMediumType(ctx->channel) == SvTVChannelMediumType_IP) {
        SvTVContextActivateConnectionMonitor(ctx);
    }

    int tunerNum = QBTVLogicGetTunerNum(ctx->appGlobals->tvLogic);
    if (ctx->super_.window) {
        TVInfo *tvInfo = ctx->super_.window->prv;
        if (ctx->tunerMonitor)
            QBTunerMonitorAddListener(ctx->tunerMonitor, (SvGenericObject) tvInfo);
        if (!tvCheckSignal(tunerNum) && !tvInfo->signalPopup)
            tvNoSignalShow(tvInfo);
        if (!tvInfo->playbackProblemPopup && !tvInfo->bufferingPopup) {
            svTVContextChangedPlaySpeed(ctx, 1.0);
        }
    }

    if (ctx->playbackMonitor && ctx->playerTask) {
        data = QBTVCreateCurrentEventData(ctx, false, NULL);
        QBPlaybackMonitorPlaybackStarted(ctx->playbackMonitor, ctx->playerTask,SVSTRING("newtv"), SVSTRING("TVPlayer"), data, NULL);
        SVTESTRELEASE(data);
    }

fini:
    if (position > 0.0)
        SvPlayerTaskEnableTimeshiftPlayback(ctx->playerTask, true);

    if (ctx->playerTask)
        SvPlayerTaskPlay(ctx->playerTask, 1, position, NULL);
}

SvLocal void
SvTVContextTracksUpdated(SvObject self_, SvPlayerTrackController controller)
{
    QBTVUpdateFormat((QBWindowContext) self_);
}

SvLocal void
SvTVContextCurrentTrackChanged(SvObject self, SvPlayerTrackController controller, unsigned int idx)
{
}

SvLocal void SvTVContextDestroyWindow(QBWindowContext self_)
{
   SvTVContext self = (SvTVContext) self_;
   SvTVContextEPGEventChanged(self, self->channel);

   QBTVLogicPreWidgetsDestroy(self->appGlobals->tvLogic);
   HDMIServiceStatusRemoveListener((HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                     SVSTRING("HDMIService")),
                                           (SvObject) self,
                                           NULL);

   TVInfo *tvInfo = self->super_.window->prv;
   if (tvInfo->updateTimer)
      svAppTimerStop(self->super_.window->app, tvInfo->updateTimer);

   if (tvInfo->resolver) {
       SVRELEASE(tvInfo->resolver);
   }

   if (tvInfo->bufferingPopup) {
       QBDialogBreak(tvInfo->bufferingPopup);
       tvInfo->bufferingPopup = NULL;
   }

   if (tvInfo->playbackProblemPopup) {
       QBDialogBreak(tvInfo->playbackProblemPopup);
       tvInfo->playbackProblemPopup = NULL;
   }
   
   // BEGIN RAL TimeShift cancel pop-up window
   if (tvInfo->objLogic)
   {
       SVRELEASE(tvInfo->objLogic);
   }
   // END RAL TimeShift cancel pop-up window

   if (tvInfo->HDCPAuthFailedPopup) {
       QBDialogBreak(tvInfo->HDCPAuthFailedPopup);
       tvInfo->HDCPAuthFailedPopup = NULL;
   }

   if (tvInfo->extraErrorPopup) {
       QBDialogBreak(tvInfo->extraErrorPopup);
       tvInfo->extraErrorPopup = NULL;
   }

   if (self->autoRestartFiber) {
       SvFiberDestroy(self->autoRestartFiber);
       self->autoRestartFiber = NULL;
   }

   if (self->connectionMonitorFiber) {
       SvFiberDestroy(self->connectionMonitorFiber);
       self->connectionMonitorFiber = NULL;
   }

   if (self->appGlobals->dsmccMonitor) {
       QBContentProviderRemoveListener((QBContentProvider) self->redButtonDSMCCDataProvider, (SvObject) self);
       QBContentProviderRemoveListener((QBContentProvider) self->appGlobals->adsDSMCCDataProvider, (SvObject) self);
       QBContentProviderStop((QBContentProvider) self->appGlobals->adsDSMCCDataProvider);
       QBDSMCCAdsRotationServiceStop(self->adsRotor);
       QBDSMCCAdsRotationServiceClearContent(self->adsRotor);
   }

   if (self->appGlobals->DRMManager) {
       QBDRMManagerRemoveListener(self->appGlobals->DRMManager, (SvObject) self);
   }

   svWidgetDestroy(tvInfo->background);

   SVTESTRELEASE(tvInfo->langMenu);
   SVTESTRELEASE(tvInfo->teletext);
   SVTESTRELEASE(tvInfo->miniFavoriteList);
   SVTESTRELEASE(tvInfo->miniRecordingList);
   SVTESTRELEASE(tvInfo->miniChannelList);
   SVTESTRELEASE(tvInfo->adWindow);
   SVTESTRELEASE(tvInfo->redButtonOverlayWindow);
   tvInfo->langMenu = NULL;

   if (tvInfo->parentalPopup)
       QBDialogBreak(tvInfo->parentalPopup);

   SVTESTRELEASE(tvInfo->miniTVGuide);

   SVRELEASE(tvInfo->OSD);
   svWidgetDestroy(tvInfo->extendedInfo);
   tvInfo->extendedInfo = NULL;

   SvTVContextUpdateSubtitlesState(self, true);
   svWidgetDestroy(self->super_.window);
   self->super_.window = NULL;

   if (self->flushScreenOnContextSwitch)
      SvTVContextDisableOverlay(self_);

   if (self->appGlobals->casPopupVisabilityManager) {
       QBCASPopupVisibilityManagerUnblockAll(self->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID));
   }
}

SvLocal void tvClean(SvApplication app, void* ptr)
{
    SVRELEASE(ptr);
}

SvLocal void SvTVContextCreateRedButtonNotificationContent(TVInfo *tvInfo, SvApplication app)
{
    svSettingsPushComponent("osd.settings");
    const char* const notificationContentWidgetName = "OSD.Notification.Content";
    SvWidget notificationContent = svSettingsWidgetCreate(app, notificationContentWidgetName);

    if (!notificationContent) {
        svSettingsPopComponent();
        return;
    }

    svSettingsPushComponent("RedButtonOSDInfo.settings");
    {
        const char* const colorDotName = "ColorDot";
        SvWidget colorDot = svSettingsWidgetCreate(app, colorDotName);
        svSettingsWidgetAttach(notificationContent, colorDot, colorDotName, 0);
    }
    {
        const char* const textLabelName = "Text";
        SvWidget textLabel = svLabelNewFromSM(app, textLabelName);
        svSettingsWidgetAttach(notificationContent, textLabel, textLabelName, 0);
    }
    svSettingsPopComponent(); // RedButtonOSDInfo.settings

    SvInvokeInterface(TVOSD, tvInfo->OSD, attachNotificationContent, notificationContent, notificationContentWidgetName);
    svSettingsPopComponent(); // osd.settings
}

SvLocal void SvTVContextCreateWindow(QBWindowContext self_, SvApplication app)
{
   SvTVContext ctx = (SvTVContext) self_;
   TVInfo *tvInfo;
   SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
   tvInfo = (TVInfo*) SvTypeAllocateInstance(TVInfo_getType(), NULL);
   tvInfo->ctx = ctx;

   window->prv = tvInfo;
   window->clean = tvClean;
   svWidgetSetTimerEventHandler(window, tvTimerEventHandler);
   
   // AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
   tvInfo->sendStartUsageLog = true;
   tvInfo->sendStartUsageLogTimeStamp = time(NULL);

   tvInfo->OSD = QBTVLogicOSDCreate(ctx->appGlobals->tvLogic, app, (QBWindowContext) ctx);
   SvInvokeInterface(TVOSD, tvInfo->OSD, setSubsManager, ctx->subsManager);
   QBOSD osd = QBOSDMainGetHandler(tvInfo->OSD, QBOSDHandlerType_recording, NULL);
   if (osd) {
       QBOSDRecordingSetEventSource((QBOSDRecording) osd, SvTVContextGetCurrentEvent, ctx, NULL);
   } else {
       SvLogError("%s() : OSD recording handler not found", __func__);
   }
   SvInvokeInterface(TVOSD, tvInfo->OSD, setChannelSelectionCallback, tvManualChannelSelection, window);
   SvWidget osdWidget = QBOSDTakeWidget((QBOSD) tvInfo->OSD, NULL);
   svWidgetAttach(window, osdWidget, 0, 0, 10);

   if (ctx->appGlobals->dsmccMonitor) {
       SvTVContextCreateRedButtonNotificationContent(tvInfo, app);
       QBContentProviderAddListener((QBContentProvider) ctx->redButtonDSMCCDataProvider, (SvObject) ctx);
       QBContentProviderAddListener((QBContentProvider) ctx->appGlobals->adsDSMCCDataProvider, (SvObject) ctx);
       QBContentProviderStart((QBContentProvider) ctx->appGlobals->adsDSMCCDataProvider, ctx->appGlobals->scheduler);
       QBDSMCCAdsRotationServiceStart(ctx->adsRotor, ctx->appGlobals->scheduler);
   }

   svSettingsPushComponent("AnimatedBackgroundInTV.settings");

   tvInfo->background = QBAnimatedBackgroundCreate(app, "AnimatedBackground", QBAnimatedBackgroundType_default, ctx->appGlobals->initLogic);
   svSettingsPopComponent();
   svWidgetAttach(window, tvInfo->background, 0, 0, 1);

   tvInfo->miniTVGuide = NULL; // initialize on first show

   svWidgetSetInputEventHandler(window, tvInputEventHandler);
   svWidgetSetFocusable(window, true);

   tvInfo->updateTimer = svAppTimerStart(app, window, 1.0, 1);

    if (!ctx->channel) {
        QBTVLogicResumePlaying(ctx->appGlobals->tvLogic, SVSTRING("TV"));
    }

   SvValue subsLangVal = (SvValue) SvTVChannelGetAttribute(ctx->channel, SVSTRING("subsLang"));

   tvInfo->langMenu = QBLangMenuNew(ctx->appGlobals, ctx->subsManager);
   QBLangMenuSetSubsLang(tvInfo->langMenu, subsLangVal ? SvValueGetString(subsLangVal) : NULL);
   QBLangMenuSetAudioCallback(tvInfo->langMenu, SvTVContextSetAudioTrack, ctx);
   QBLangMenuSetSubsCallback(tvInfo->langMenu, SvTVContextSetSubsLang, ctx);

   QBTVLogicMetaDataManagers metaDataMgrs = QBTVLogicGetMetaDataManagers(ctx->appGlobals->tvLogic);
   tvInfo->teletext = QBTeletextWindowNew(ctx->appGlobals, metaDataMgrs->teletextReceiver);
   tvInfo->miniFavoriteList = QBMiniFavoriteListNew(ctx->appGlobals);
   tvInfo->miniChannelList = QBMiniChannelListNew(ctx->appGlobals);
   tvInfo->miniRecordingList = QBMiniRecordingListNew(ctx->appGlobals);
   tvInfo->adWindow = QBAdWindowNew(ctx->appGlobals);
   tvInfo->recordingBlockedPopup = NULL;

   svSettingsPushComponent("TVExtendedInfo.settings");
   SvWidget w = QBExtendedInfoNew(app, "ExtendedInfo", ctx->appGlobals);
   tvInfo->extendedInfo = w;
   svSettingsPopComponent();

   svSettingsPushComponent("TVLogo.settings");
   if (svSettingsIsWidgetDefined("overlayLogo")) {
       SvWidget overlayLogo = svIconNew(app, "overlayLogo");
       if (overlayLogo) {
           svSettingsWidgetAttach(window, overlayLogo, svWidgetGetName(overlayLogo), 0);
       }
   }
   svSettingsPopComponent();

   ctx->super_.window = window;

   SvTVContextAuthenticationStateChanged(ctx);
   SvTVContextUpdateSubtitlesState(ctx, false);

   //setTVChannel() bool params: isFavorite
   SvInvokeInterface(TVOSD, tvInfo->OSD, setTVChannel, ctx->channel, false);
   //show() bool params: immediate, force
   SvInvokeInterface(TVOSD, tvInfo->OSD, show, true, true);

   SvTVContextPVRRecordingChanged((SvObject) ctx, NULL, NULL);

   SvTVContextModifyRecordingIndicator(ctx);

   HDMIService hdmiService = (HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("HDMIService"));
   HDMIServiceStatusAddListener(hdmiService, (SvObject) ctx, NULL);
   SvTVContextHDCPAuthStatusChanged((SvObject) ctx, HDMIServiceGetHDCPAuthStatus(hdmiService));

   if (ctx->playerTask) {
       if (ctx->tunerMonitor)
           QBTunerMonitorAddListener(ctx->tunerMonitor, (SvGenericObject) tvInfo);
       if (!tvCheckSignal(QBTVLogicGetTunerNum(tvInfo->ctx->appGlobals->tvLogic)) && !tvInfo->signalPopup)
         tvNoSignalShow(tvInfo);
   }

   SvTVContextAttachSubtitle((QBWindowContext)ctx, window);

   svTVContextChangedPlaySpeed(ctx, 1.0);

   if (ctx->channelErrorCode != 0) {
       SvTVContextActivateConnectionMonitor(ctx);
   }
   QBTVLogicCheckBackground(ctx->appGlobals->tvLogic, (QBWindowContext) ctx, ctx->channel);
   QBTVUpdateFormat((QBWindowContext) ctx);

   if (ctx->appGlobals->dsmccMonitor) {
       QBTVContextUpdateRedButtonOverlayAndInfo(tvInfo);
   }

   if (ctx->appGlobals->DRMManager) {
        QBDRMManagerAddListener(ctx->appGlobals->DRMManager, (SvObject) ctx);
   }

   QBTVLogicPostWidgetsCreate(ctx->appGlobals->tvLogic);
}

SvLocal void SvTVContextDestroy(void *ptr)
{
   SvTVContext ctx = ptr;

   QBViewportRemoveCallbacks(QBViewportGet(), &viewportCallbacks, ctx);

   QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
   if(eventBus) {
       QBEventBusUnregisterReceiver(eventBus, (SvObject) ctx, QBSideMenuLevelChangedEvent_getType(), NULL);
   }

   if (ctx->appGlobals->pvrProvider) {
       QBPVRProviderRemoveListener(ctx->appGlobals->pvrProvider, (SvObject) ctx);
   }

   if (ctx->playerTask) {
      AudioTrackLogic audioTrackLogic =
              (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
      AudioTrackLogicSetPlayerTaskControllers(audioTrackLogic, NULL);
      VideoTrackLogic videoTrackLogic =
              (VideoTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoTrackLogic"));
      VideoTrackLogicSetPlayerTaskControllers(videoTrackLogic, NULL);
      SubtitleTrackLogic subtitleTrackLogic =
              (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
      SubtitleTrackLogicSetPlayerTaskControllers(subtitleTrackLogic, NULL);
      SvPlayerTaskClose(ctx->playerTask, NULL);
      SVRELEASE(ctx->playerTask);
   }

   if (ctx->currentRecording) {
      QBPVRProviderStopRecording(ctx->appGlobals->pvrProvider,
                                 ctx->currentRecording);
      SVRELEASE(ctx->currentRecording);
   }

   if (ctx->autoRestartFiber) {
       SvFiberDestroy(ctx->autoRestartFiber);
       ctx->autoRestartFiber = NULL;
   }

   if (ctx->connectionMonitorFiber) {
       SvFiberDestroy(ctx->connectionMonitorFiber);
       ctx->connectionMonitorFiber = NULL;
   }

   SVTESTRELEASE(ctx->currContent);
   SVTESTRELEASE(ctx->channel);
   SVTESTRELEASE(ctx->previous);
   SVRELEASE(ctx->playerHandler);

   SVTESTRELEASE(ctx->listID);
   SVTESTRELEASE(ctx->previousList);
   if (ctx->pc) {
        QBParentalControlLogicRemoveParentalControlHelper(ctx->appGlobals->parentalControlLogic, ctx->pc);
        SVRELEASE(ctx->pc);
   }

   SVTESTRELEASE(ctx->events.current);
   SVTESTRELEASE(ctx->events.following);

   SVTESTRELEASE(ctx->subsManager);

   SVTESTRELEASE(ctx->redButtonDSMCCDataProvider);
   SVTESTRELEASE(ctx->adsRotor);

   SVTESTRELEASE(ctx->playbackMonitor);
   SVTESTRELEASE(ctx->tunerMonitor);
}

SvLocal void SvTVContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    SvTVContext ctx = (SvTVContext) self_;

    if (!ctx->super_.window)
        return;

    if (itemList && SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl"))) {
        TVInfo *tvInfo = ctx->super_.window->prv;

        QBOSD osd = QBOSDMainGetHandler(tvInfo->OSD, QBOSDHandlerType_recording, NULL);
        QBOSDRecordingUpdateEvents((QBOSDRecording) osd, NULL);

        QBMiniChannelListReinitialize(tvInfo->miniChannelList, itemList);
    }
}

SvLocal void QBTVContextUpdateRedButtonOverlayAndInfo(TVInfo *tvInfo)
{
    SvTVContext ctx = tvInfo->ctx;
    SvHashTable content = NULL;

    if (ctx->channel && SvObjectIsInstanceOf((SvObject) ctx->channel, DVBEPGChannel_getType())) {
        DVBEPGChannel dvbEPGchannel = (DVBEPGChannel) ctx->channel;

        SvHashTable data = QBDSMCCDataProviderGetObject(ctx->redButtonDSMCCDataProvider);
        if (data) {
            SvString channelId = SvStringCreateWithFormat("%d:%d:%d", dvbEPGchannel->tsid, dvbEPGchannel->sid, dvbEPGchannel->onid);

            SvArray contents = (SvArray) SvHashTableFind(data, (SvObject) SVSTRING("desc"));
            SvIterator it = SvArrayGetIterator(contents);
            while ((content = (SvHashTable) SvIteratorGetNext(&it))) {
                SvArray channels = (SvArray) SvHashTableFind(content, (SvObject) SVSTRING("channels"));
                if (SvArrayFindObject(channels, (SvObject) channelId) != -1)
                    break;
            }

            SVRELEASE(channelId);
        }
    }

    if (content) {
        if (!tvInfo->redButtonOverlayWindow) {
            tvInfo->redButtonOverlayWindow = QBRedButtonOverlayWindowNew(ctx->appGlobals, content);
        } else {
            QBRedButtonOverlayWindowReplaceContent(tvInfo->redButtonOverlayWindow, content);
        }
        if (!QBRedButtonOverlayWindowIsVisible(tvInfo->redButtonOverlayWindow) && !QBTeletextWindowIsVisible(tvInfo->teletext)) {
            SvInvokeInterface(TVOSD, tvInfo->OSD, showNotification, false);
        }
    } else {
        if (tvInfo->redButtonOverlayWindow) {
            QBRedButtonOverlayWindowHide(tvInfo->redButtonOverlayWindow);
        }
        SVTESTRELEASE(tvInfo->redButtonOverlayWindow);
        tvInfo->redButtonOverlayWindow = NULL;
        SvInvokeInterface(TVOSD, tvInfo->OSD, hideNotification, true);
    }
}

SvLocal void QBTVContextDSMCCObjectReceived(SvObject self, SvString serviceID)
{
    SvTVContext ctx = (SvTVContext) self;
    TVInfo *tvInfo = NULL;
    if (ctx->super_.window) {
        tvInfo = ctx->super_.window->prv;
    }

    if (SvStringEqualToCString(serviceID, "adverts")) {
        SvHashTable data = QBDSMCCDataProviderGetObject(ctx->appGlobals->adsDSMCCDataProvider);
        QBDSMCCAdsRotationServiceSetContent(ctx->adsRotor, data);
    } else if (SvStringEqualToCString(serviceID, "red_button")) {
        if (tvInfo) {
            QBTVContextUpdateRedButtonOverlayAndInfo(tvInfo);
        }
    }
}

SvLocal void QBTVContextSetAds(SvObject self, SvHashTable images)
{
    SvTVContext ctx = (SvTVContext) self;
    TVInfo *tvInfo = ctx->super_.window->prv;

    SvRBBitmap adRB = NULL;
    if (images) {
        adRB = (SvRBBitmap) SvHashTableFind(images, (SvObject) SVSTRING("osd"));
    }

    if (adRB) {
        SvInvokeInterface(TVOSD, tvInfo->OSD, setAdvert, adRB);
    } else {
        SvInvokeInterface(TVOSD, tvInfo->OSD, clearAdvert);
    }
}

SvLocal void QBTVContextDRMDataChanged(SvObject self_, const QBDRMManagerData* data)
{
    SvTVContext ctx = (SvTVContext) self_;
    TVInfo *tvInfo = ctx->super_.window->prv;
    QBOSD osd = QBOSDMainGetHandler(tvInfo->OSD, QBOSDHandlerType_recording, NULL);
    if (osd) {
        QBOSDRecordingSetAvailabilityLimit((QBOSDRecording) osd, data->availabilityTime, data->isAvailable, NULL);
    }
}

SvLocal void QBTVContextHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    SvTVContext self = (SvTVContext) self_;

    if (!self->super_.window) {
        return;
    }

    if (SvObjectIsInstanceOf((SvObject) event_, QBSideMenuLevelChangedEvent_getType())) {
        QBSideMenuLevelChangedEvent event = (QBSideMenuLevelChangedEvent) event_;
        if (event->currentLevel == 0) {
            if (self->appGlobals->casPopupVisabilityManager) {
                QBCASPopupVisibilityManagerUnblock(self->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_SIDE_MENU_WINDOW_ID));
            }
        }
    }
}

SvType SvTVContext_getType(void)
{
  static SvType type = NULL;

  static const struct QBWindowContextVTable_ contextVTable = {
    .super_             = {
      .destroy         = SvTVContextDestroy
    },
    .reinitializeWindow = SvTVContextReinitializeWindow,
    .createWindow       = SvTVContextCreateWindow,
    .destroyWindow      = SvTVContextDestroyWindow
  };

   static const struct QBChannelPlayer_ channelPlayerMethods = {
       .play = SvTVContextPlay,
       .stop = SvTVContextStopPlaying,
   };

   static const struct SvPlayerTaskControllersListener_ taskControllersListenerMethods = {
       .controllersUpdated = SvTVContextTaskControllersUpdated,
   };

   static const struct SvPlayerTrackControllerListener_ trackControllerListenerMethods = {
       .tracksUpdated       = SvTVContextTracksUpdated,
       .currentTrackChanged = SvTVContextCurrentTrackChanged
   };

   static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
       .recordingAdded = SvTVContextPVRRecordingAddedRemoved,
       .recordingChanged = SvTVContextPVRRecordingChanged,
       .recordingRemoved = SvTVContextPVRRecordingAddedRemoved,
       .recordingRestricted = SvTVContextPVRRecordingRestricted,
       .quotaChanged = SvTVContextPVRQuotaChanged,
       .directoryAdded = SvTVContextPVRDirectoryAdded,
   };

   static const struct QBnPVRProviderRecordingListener_ nPVRRecordingMethods = {
       .added = SvTVContextNPvrRecordingChanged,
       .changed = SvTVContextNPvrRecordingChanged,
       .removed = SvTVContextNPvrRecordingChanged
   };

   static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
       .customerInfoChanged = SvTVContextCustomerInfoMonitorInfoChanged,
       .statusChanged = SvTVContextCustomerInfoMonitorStatusChanged
   };

   static const struct QBDSMCCDataProviderListener_ dataProviderListener = {
       .objectReceived = QBTVContextDSMCCObjectReceived
   };

   static const struct QBDSMCCAdsRotationServiceListener_ adsRotationServiceListenerMethods = {
       .setAds = QBTVContextSetAds,
   };

    static struct HDMIServiceStatusListener_ HDMIStatusMethods = {
        .HDCPStatusChanged    = SvTVContextHDCPAuthStatusChanged,
        .hotplugStatusChanged = SvTVContextHotplugStatusChanged
    };

   static const struct QBDRMManagerListener_ DRMManager = {
       .DRMDataChanged = QBTVContextDRMDataChanged
   };

   static const struct QBPeerEventReceiver_ eventReceiverMethods = {
       .handleEvent = QBTVContextHandlePeerEvent
   };

   if (unlikely(!type)) {
      SvTypeCreateManaged("SvTVContext",
                          sizeof(struct SvTVContext_t),
                          QBWindowContext_getType(),
                          &type,
                          QBWindowContext_getType(), &contextVTable,
                          QBChannelPlayer_getInterface(), &channelPlayerMethods,
                          SvPlayerTaskControllersListener_getInterface(), &taskControllersListenerMethods,
                          SvPlayerTrackControllerListener_getInterface(), &trackControllerListenerMethods,
                          QBnPVRProviderRecordingListener_getInterface(), &nPVRRecordingMethods,
                          QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                          CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                          QBDSMCCDataProviderListener_getInterface(), &dataProviderListener,
                          QBDSMCCAdsRotationServiceListener_getInterface(), &adsRotationServiceListenerMethods,
                          QBDRMManagerListener_getInterface(), &DRMManager,
                          QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                          HDMIServiceStatusListener_getInterface(), &HDMIStatusMethods,
                          NULL);
   }
   return type;
}

void
QBTVOSDShow(QBWindowContext ctx_, bool immediate, bool force){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, tvInfo->OSD, show, immediate, force);
}

void
QBTVOSDLock(QBWindowContext ctx_, bool lock, bool force){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (lock)
        SvInvokeInterface(TVOSD, tvInfo->OSD, lock);
    else
        SvInvokeInterface(TVOSD, tvInfo->OSD, unlock, force);
}

void
QBTVOSDHide(QBWindowContext ctx_, bool immediate, bool force){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, tvInfo->OSD, hide, immediate, force);
}

bool
QBTVOSDIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return SvInvokeInterface(TVOSD, tvInfo->OSD, isShown);
}

void QBTVOSDNotificationShow(QBWindowContext ctx_, bool immediate)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, tvInfo->OSD, showNotification, immediate);
}

void QBTVOSDNotificationHide(QBWindowContext ctx_, bool immediate)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, tvInfo->OSD, hideNotification, immediate);
}


void QBTVMiniFavoriteListShow(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (!QBTVMiniFavoriteListIsVisible(ctx_)) {
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_FAVORITE_LIST_WINDOW_ID));
        }
        QBMiniFavoriteListShow(tvInfo->miniFavoriteList);
    }
}

void QBTVMiniFavoriteListHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVMiniFavoriteListIsVisible(ctx_)) {
        QBMiniFavoriteListHide(tvInfo->miniFavoriteList);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_FAVORITE_LIST_WINDOW_ID));
        }
    }
}

bool QBTVMiniFavoriteListIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    return QBMiniFavoriteListIsVisible(tvInfo->miniFavoriteList);
}

void QBTVMiniRecordingListShow(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    tvCheckPINPopupHide(tvInfo); // special case of recording with PINPopup shown

    if (!QBTVMiniRecordingListIsVisible(ctx_)) {
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_MINI_RECORDING_LIST_WINDOW_ID));
        }
        QBMiniRecordingListShow(tvInfo->miniRecordingList);
    }
}

void QBTVMiniRecordingListHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVMiniRecordingListIsVisible(ctx_)) {
        QBMiniRecordingListHide(tvInfo->miniRecordingList);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_MINI_RECORDING_LIST_WINDOW_ID));
        }
        SvTVContextAuthenticationStateChanged(ctx); // for case of just hidden PINPopup
    }
}

bool QBTVMiniRecordingListIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    return QBMiniRecordingListIsVisible(tvInfo->miniRecordingList);
}

void QBTVMiniChannelListShow(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (!QBTVMiniChannelListIsVisible(ctx_)) {
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_MINI_MINI_CHANNEL_LIST_WINDOW_ID));
        }
        QBMiniChannelListShow(tvInfo->miniChannelList);
    }

    if (ctx->channel)
        QBMiniChannelListSelectChannel(tvInfo->miniChannelList, ctx->channel);
}

QBMiniChannelList QBTVMiniChannelListGet(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return NULL;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return tvInfo->miniChannelList;
}

void QBTVMiniChannelListHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVMiniChannelListIsVisible(ctx_)) {
        QBMiniChannelListHide(tvInfo->miniChannelList);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_MINI_MINI_CHANNEL_LIST_WINDOW_ID));
        }
    }
}

bool QBTVMiniChannelListIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    return QBMiniChannelListIsVisible(tvInfo->miniChannelList);
}

bool QBTVIsSideMenuShown(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *self = ctx->super_.window->prv;
    return SvInvokeInterface(TVOSD, self->OSD, isSideMenuShown);
}

void QBTVShowSideMenu(QBWindowContext ctx_, SvString id, QBTVSideMenuType type)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *self = ctx->super_.window->prv;
    SvTVChannel channel = SvTVContextGetCurrentChannel(ctx_);
    SvEPGEvent event = NULL;
    SvEPGEvent nextEvent = NULL;

    time_t currentPosition = SvTimeNow();
    if (ctx->playerTask) {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        if (state.timeshift.recording || state.timeshift.enabled) {
            currentPosition = ctx->timeshiftReferenceTime + state.currentPosition;
        }
    }

    SvTVContextGetCurrentEvent(ctx, ctx->channel, currentPosition, &event, &nextEvent);
    SvGenericObject sideMenuCtx = SvInvokeInterface(TVOSD, self->OSD, getSideMenuContext);
    if (!SvInvokeInterface(TVOSD, self->OSD, isSideMenuShown) && channel && sideMenuCtx && SvObjectIsInstanceOf(sideMenuCtx, QBContextMenu_getType())) {
        QBReminderEditorPane pane = NULL;
        switch (type) {
            case QBTVSideMenuType_NPvrSeries:
                pane = QBReminderEditorPaneCreateAndPushWithNPvrFromSettings("TVReminderEditorPane.settings", ctx->appGlobals, (QBContextMenu) sideMenuCtx, SVSTRING("ReminderEditorWithNPvr"), channel, event, QBReminderEditorPaneType_Series);
                break;
            case QBTVSideMenuType_NPvrKeyword:
                pane = QBReminderEditorPaneCreateAndPushWithNPvrFromSettings("TVReminderEditorPane.settings", ctx->appGlobals, (QBContextMenu) sideMenuCtx, SVSTRING("ReminderEditorWithNPvr"), channel, event, QBReminderEditorPaneType_Keyword);
                break;
            case QBTVSideMenuType_Full:
            default:
                pane = QBReminderEditorPaneCreateFromSettings("TVReminderEditorPane.settings", "TVReminderEditorPane.json", ctx->appGlobals, (QBContextMenu) sideMenuCtx, SVSTRING("ReminderEditor"), channel, event);
                break;
        }
        if (!pane)
            return;
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_SIDE_MENU_WINDOW_ID));
        }
        if (type == QBTVSideMenuType_Full) {
            SvInvokeInterface(TVOSD, self->OSD, showSideMenu, (SvGenericObject) pane, id);
        }
        SVRELEASE(pane);
    }
}

void QBTVHideSideMenu(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *self = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, self->OSD, hideSideMenu);
}

void QBTVLangMenuShow(QBWindowContext ctx_, QBLangMenuSubMenu submenu){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (!QBTVLangMenuIsVisible(ctx_)) {
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_LANG_MENU_WINDOW_ID));
        }
        QBLangMenuShow(tvInfo->langMenu, submenu);
    }
}

void QBTVLangMenuHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVLangMenuIsVisible(ctx_)) {
        QBLangMenuHide(tvInfo->langMenu);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_LANG_MENU_WINDOW_ID));
        }
    }
}

bool QBTVLangMenuIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    return QBLangMenuIsVisible(tvInfo->langMenu);
}


void QBTVMiniTVGuideShow(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (!tvInfo->miniTVGuide) {
        tvInfo->miniTVGuide = QBMiniTVGuideNew(ctx->appGlobals);
    }

    if (!QBTVMiniTVGuideIsVisible(ctx_)) {
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_TV_GUIDE_WINDOW_ID));
        }
        QBMiniTVGuideShow(tvInfo->miniTVGuide);
    }
}

void QBTVMiniTVGuideHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVMiniTVGuideIsVisible(ctx_)) {
        QBMiniTVGuideHide(tvInfo->miniTVGuide);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_TV_GUIDE_WINDOW_ID));
        }
    }
}

bool QBTVMiniTVGuideIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    return tvInfo->miniTVGuide && QBMiniTVGuideIsVisible(tvInfo->miniTVGuide);
}

bool QBTVExtendedInfoShow(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVExtendedInfoIsVisible(ctx_) || !ctx->playerTask) {
        return false;
    }

    SvEPGEvent event = NULL;
    SvEPGEvent nextEvent = NULL;
    SvTVContextGetCurrentEvent(ctx, ctx->channel, SvTimeNow(), &event, &nextEvent);

    //setTVChannel() bool params: isFavorite
    SvInvokeInterface(TVOSD, tvInfo->OSD, setTVChannel, ctx->channel, false);
    SvInvokeInterface(TVOSD, tvInfo->OSD, setCurrentEvent, event);
    SvInvokeInterface(TVOSD, tvInfo->OSD, setFollowingEvent, nextEvent);

    SvInvokeInterface(TVOSD, tvInfo->OSD, showMoreInfo);

    if (!SvInvokeInterface(TVOSD, tvInfo->OSD, isShowingMoreInfo)) {
        return false;
    }

    if (ctx->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_EXTENDED_INFO_WINDOW_ID));
    }

    return true;
}

void QBTVExtendedInfoHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (SvInvokeInterface(TVOSD, tvInfo->OSD, isShowingMoreInfo))  {
        SvInvokeInterface(TVOSD, tvInfo->OSD, showLessInfo);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_EXTENDED_INFO_WINDOW_ID));
        }
    }
}

bool QBTVExtendedInfoIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return SvInvokeInterface(TVOSD, tvInfo->OSD, isShowingMoreInfo);
}

void QBTVBackgroundShow(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (QBTVBackgroundIsVisible(ctx_))
        return;

    QBAnimatedBackgroundShow(tvInfo->background);
}

void QBTVBackgroundHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!QBTVBackgroundIsVisible(ctx_))
        return;

    QBAnimatedBackgroundHide(tvInfo->background);
}

bool QBTVBackgroundIsVisible(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;

    TVInfo *tvInfo = ctx->super_.window->prv;
    return QBAnimatedBackgroundIsVisible(tvInfo->background);
}

SvLocal void
QBTVRecordingOptionsStop(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWindowContext ctx_ = self_;
    SvTVContext ctx = (SvTVContext)ctx_;
    QBPVRRecording recording = QBBasicPaneGetPrv(pane);
    if (QBActiveArrayCount(QBPVRAgentGetOnGoingRecordingsList(ctx->appGlobals->pvrAgent))<=1)
        QBTVMiniRecordingListHide(ctx_);
    QBPVRProviderStopRecording(ctx->appGlobals->pvrProvider, recording);
    QBTVRecordingOptionsHide(ctx_);
}

SvLocal void
QBTVRecordingOptionsPlay(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWindowContext ctx_ = self_;
    SvTVContext ctx = (SvTVContext)ctx_;
    QBPVRRecording recording = QBBasicPaneGetPrv(pane);

    QBAnyPlayerLogic anyPlayerLogic = QBAnyPlayerLogicNew(ctx->appGlobals);
    // AMERELES BoldContentTracking Youbora
    //QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(ctx->appGlobals, anyPlayerLogic);
    QBWindowContext pvrPlayer = QBPVRPlayerContextCreate(ctx->appGlobals, anyPlayerLogic, time(NULL));

    if (ctx->appGlobals->remoteControl) {
        QBPlaybackStateController playbackStateController = QBRemoteControlCreatePlaybackStateController(ctx->appGlobals->remoteControl, NULL);
        QBAnyPlayerLogicSetPlaybackStateController(anyPlayerLogic, (SvGenericObject) playbackStateController);
        SVRELEASE(playbackStateController);
    }

    SvString serviceName = SvStringCreate(gettext("PVR"), NULL);
    QBPVRPlayerContextSetTitle(pvrPlayer, serviceName);
    SVRELEASE(serviceName);

    if (ctx->appGlobals->bookmarkManager) {
        QBBookmark bookmark = QBBookmarkManagerGetLastPositionBookmark(ctx->appGlobals->bookmarkManager, recording->id);
        if (!bookmark)
            bookmark = QBBookmarkManagerSetLastPositionBookmark(ctx->appGlobals->bookmarkManager, recording->id, 0.0, QBBookmarkType_Recording);

        if (bookmark)
            QBAnyPlayerLogicSetLastPositionBookmark(anyPlayerLogic, bookmark);
    }

    SVRELEASE(anyPlayerLogic);
    QBPVRPlayerContextSetRecording(pvrPlayer, recording);
    QBApplicationControllerPushContext(ctx->appGlobals->controller, pvrPlayer);
    SVRELEASE(pvrPlayer);

    QBTVRecordingOptionsHide(ctx_);
}

SvLocal void
QBTVRecordingOptionsCancel(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBWindowContext ctx_ = self_;
    QBTVRecordingOptionsHide(ctx_);
}

void QBTVRecordingOptionsShow(QBWindowContext ctx_, QBPVRRecording recording)
{
    SvTVContext ctx = (SvTVContext)ctx_;

    if (!ctx->super_.window)
        return;

    TVInfo *tvInfo = ctx->super_.window->prv;

    SvObject sideMenuCtx = SvInvokeInterface(TVOSD, tvInfo->OSD, getSideMenuContext);
    if (!SvInvokeInterface(TVOSD, tvInfo->OSD, isSideMenuShown) && sideMenuCtx && SvObjectIsInstanceOf(sideMenuCtx, QBContextMenu_getType())) {
        svSettingsPushComponent("BasicPane.settings");
        QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
        QBBasicPaneSetPrv(options, recording);
        QBBasicPaneInit(options, ctx->appGlobals->res, ctx->appGlobals->scheduler, ctx->appGlobals->textRenderer, (QBContextMenu) sideMenuCtx, 1, SVSTRING("BasicPane"));

        SvString name = SvStringCreate(gettext("Stop"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("stop"), name, QBTVRecordingOptionsStop, ctx);
        SVRELEASE(name);

        name = SvStringCreate(gettext("Play"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("play"), name, QBTVRecordingOptionsPlay, ctx);
        SVRELEASE(name);

        name = SvStringCreate(gettext("Cancel"), NULL);
        QBBasicPaneAddOption(options, SVSTRING("cancel"), name, QBTVRecordingOptionsCancel, ctx);
        SVRELEASE(name);

        SvInvokeInterface(TVOSD, tvInfo->OSD, showSideMenu, (SvGenericObject) options, NULL);
        SVRELEASE(options);
        svSettingsPopComponent();
    }
}

void QBTVRecordingOptionsHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, tvInfo->OSD, hideSideMenu);
}

bool QBTVRecordingOptionsIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return SvInvokeInterface(TVOSD, tvInfo->OSD, isSideMenuShown);
}

void QBTVTeletextSetTransparent(QBWindowContext ctx_, bool transparent){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    QBTeletextWindowSetTransparent(tvInfo->teletext, transparent);
}

bool QBTVTeletextIsTransparent(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return QBTeletextWindowIsTransparent(tvInfo->teletext);
}

bool QBTVTeletextIsVisible(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return QBTeletextWindowIsVisible(tvInfo->teletext);
}

void QBTVTeletextHide(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVTeletextIsVisible(ctx_)) {
        QBTeletextWindowHide(tvInfo->teletext);
    }

    SvTVContextUpdateSubtitlesState(ctx, false);
}

void QBTVTeletextShow(QBWindowContext ctx_){
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (QBTVTeletextIsVisible(ctx_))
        return;

    SvTVContextUpdateSubtitlesState(ctx, true);

    QBTeletextWindowShow(tvInfo->teletext);

    const char *clang = QBConfigGet("SUBTITLESLANG"); //FIXME subtitle lang, menu lang or ... ?
    QBTeletextWindowSetStartPage(tvInfo->teletext, QBViewportGetOutputFormat(QBViewportGet()), clang);
}

SvLocal void tvTunerInfoCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->tunerPopup = NULL;
}

void QBTVTunerInfoShow(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->tunerPopup)
        return;

    QBWindowContext windowCtx = (QBWindowContext) ctx;
    AppGlobals appGlobals = tvInfo->ctx->appGlobals;
    const char* message = gettext("There are too many ongoing recordings. Please stop some to watch this channel again.");
    tvInfo->tunerPopup = QBPlaybackErrorPopupCreate(appGlobals->controller,
                                                    windowCtx,
                                                    gettext("Channel not available"),
                                                    message);
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.Tuner", "JSON:{\"description\":\"%s\"%s}", message,
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    QBDialogRun(tvInfo->tunerPopup, tvInfo, tvTunerInfoCallback);
}

void QBTVTunerInfoHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->tunerPopup)
        return;
    QBDialogBreak(tvInfo->tunerPopup);
}

SvLocal void tvErrorPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->errorPopup = NULL;
}

void QBTVErrorShow(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->errorPopup)
        return;

    QBDialogParameters params = {
        .app        = tvInfo->ctx->super_.window->app,
        .controller = tvInfo->ctx->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };

    svSettingsPushComponent("Dialog.settings");
    const char* message = gettext("An error has occurred. Please contact your service provider.\n");
    tvInfo->errorPopup = QBSimpleDialogCreate(&params, gettext("A problem occurred"), message);
    svSettingsPopComponent();
    QBDialogRun(tvInfo->errorPopup, tvInfo, tvErrorPopupCallback);
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.Playback", "JSON:{\"description\":\"%s\"%s}", message,
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

void QBTVErrorHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->errorPopup)
        return;
    QBDialogBreak(tvInfo->errorPopup);
}

SvLocal void tvNotEntitledCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->notEntitledPopup = NULL;
}

void QBTVNotEntitledShow(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->notEntitledPopup)
        return;

    QBWindowContext windowCtx = (QBWindowContext) ctx;
    AppGlobals appGlobals = tvInfo->ctx->appGlobals;

    const char* message = gettext("You are not entitled to view this channel");
    tvInfo->notEntitledPopup = QBPlaybackErrorPopupCreate(appGlobals->controller,
                                                          windowCtx,
                                                          gettext("Not Entitled"),
                                                          message);
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.NotEntitled", "JSON:{\"description\":\"%s\"%s}", message,
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    QBDialogRun(tvInfo->notEntitledPopup, tvInfo, tvNotEntitledCallback);
}

void QBTVNotEntitledHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->notEntitledPopup)
        return;
    QBDialogBreak(tvInfo->notEntitledPopup);
}

SvLocal void QBTVConnectionProblemCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->playbackProblemPopup = NULL;
}

SvLocal SvString QBTVCreateErrorMessageFromErrorCode(int code)
{
    if (code == SV_ERR_DRM_NO_LICENSE) {
        return SvStringCreateWithFormat(gettext("DRM license error"));
    }

    if (code == SV_ERR_DRM_INIT_FAILURE) {
        return SvStringCreateWithFormat(gettext("DRM initialization error"));
    }

    if ( (code == SV_ERR_DRM_OTHER) ||
         (code == SV_ERR_DRM_LIC_NOT_YET_VALID) ||
         (code == SV_ERR_DRM_LIC_EXPIRED) ||
         (code == SV_ERR_DRM_CPC_CERT_INVALID) ||
         (code == SV_ERR_DRM_CONN_FAILED) ||
         (code == SV_ERR_DRM_KEY_NOT_OBTAINED) ||
         (code == SV_ERR_DRM_CERT_REVOKED) ||
         (code == SV_ERR_NO_DRM) ||
         (code == SV_ERR_DRM_CLOSE_FAILURE) ) {
        return SvStringCreateWithFormat(gettext("DRM error [%d]"), code);
    }

    if (code == SV_ERR_SERVER_FILE_NOT_FOUND) {
        return SvStringCreateWithFormat(gettext("Content could not be found in the server"));
    }

    if (code == SV_ERR_SERVER_PERM) {
        return SvStringCreateWithFormat(gettext("Server error: forbidden"));
    }

    if (code == SV_ERR_UNSUPPORTED_PROTOCOL) {
        return SvStringCreateWithFormat(gettext("Server error: unsupported protocol"));
    }

    if (code == SV_ERR_DVB_FATAL) {
        return SvStringCreateWithFormat(gettext("DVB error: no audio/video"));
    }

    return SvStringCreateWithFormat(gettext("Other error: [%d]"), code);
}

SvLocal bool QBTVPlaybackProblemPopupIsVisible(SvTVContext ctx)
{
    if (!ctx->super_.window) {
        return false;
    }

    TVInfo *tvInfo = ctx->super_.window->prv;
    return (tvInfo->playbackProblemPopup != NULL);
}

SvLocal void QBTVPlaybackProblemShow(SvTVContext ctx, int errorCode)
{
    if (!ctx->super_.window) {
        return;
    }
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->playbackProblemPopup) {
        return;
    }

    SvString message = QBTVCreateErrorMessageFromErrorCode(errorCode);
    svSettingsPushComponent("NoSignalDialog.settings");
    const char *connectionProblemTitle = svSettingsGetString("Dialog.ConnectionProblemPopup", "title");
    svSettingsPopComponent();
    QBWindowContext windowCtx = (QBWindowContext) ctx;
    AppGlobals appGlobals = tvInfo->ctx->appGlobals;
    tvInfo->playbackProblemPopup = QBPlaybackErrorPopupCreate(appGlobals->controller,
                                                              windowCtx,
                                                              connectionProblemTitle ? gettext(connectionProblemTitle) : gettext("Connection problem"),
                                                              SvStringCString(message));
    SvString data = QBTVCreateCurrentEventData(ctx, true, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.Playback", "JSON:{\"description\":\"%s\"%s}", SvStringCString(message),
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    SVRELEASE(message);

    QBDialogRun(tvInfo->playbackProblemPopup, tvInfo, QBTVConnectionProblemCallback);
}

SvLocal void QBTVConnectionProblemHide(SvTVContext ctx)
{
    if (!ctx->super_.window) {
        return;
    }

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->playbackProblemPopup) {
        return;
    }
    QBDialogBreak(tvInfo->playbackProblemPopup);
}

SvLocal void QBTVBufferingPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->bufferingPopup = NULL;
}

SvLocal void QBTVBufferingPopupShow(SvTVContext ctx)
{
    if (!ctx->super_.window) {
        return;
    }

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->bufferingPopup) {
        return;
    }

    tvInfo->bufferingPopup = QBTVSetupBufferingPopup(ctx);
    QBDialogRun(tvInfo->bufferingPopup, tvInfo, QBTVBufferingPopupCallback);
}

SvLocal void QBTVBufferingPopupHide(SvTVContext ctx)
{
    if (!ctx->super_.window) {
        return;
    }

    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->bufferingPopup) {
        return;
    }

    QBDialogBreak(tvInfo->bufferingPopup);
}

SvLocal void SvTVContextCheckConnectionStep(void* ctx_)
{
    SvTVContext ctx = ctx_;

    SvFiberDeactivate(ctx->connectionMonitorFiber);
    SvFiberEventDeactivate(ctx->connectionMonitorTimer);

    SvFiberTimerActivateAfter(ctx->connectionMonitorTimer, SvTimeConstruct(1, 0));

    double bufferedSec = 0.0;
    double currentSpeed = 1.0;
    if (ctx->playerTask) {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        bufferedSec = state.bufferedSec;
        currentSpeed = state.currentSpeed;
    }

    // bufferedSec is negative when some error occured (e.g. can't get proper calculations for direct_es mode)
    // in such case we don't want to display bufferingPopup
    if (bufferedSec > 0.1 || (currentSpeed != 1.0 && currentSpeed != 0.0) || bufferedSec < 0) {
        ctx->connectionProblemStartTime = SvTimeGetZero();
        ctx->channelErrorCode = 0;
        ctx->autoRestartDelaySec = 0;

        QBTVBufferingPopupHide(ctx);
        QBTVConnectionProblemHide(ctx);

        return;
    }

    if (QBTVPlaybackProblemPopupIsVisible(ctx)) {
        return;
    }

    if (SvTimeCmp(ctx->connectionProblemStartTime, SvTimeGetZero()) == 0) {
        ctx->connectionProblemStartTime = SvTimeGet();
        return;
    }

    int secondsPassed = SvTimeToMs(SvTimeSub(SvTimeGet(), ctx->connectionProblemStartTime)) / 1000;
    if (secondsPassed >= CONNECTION_ERROR_POPUP_DELAY_SEC) {
        QBTVBufferingPopupHide(ctx);
        QBTVPlaybackProblemShow(ctx, ctx->channelErrorCode);
        return;
    }

    if (secondsPassed < BUFFERING_POPUP_DELAY_SEC) {
        return;
    }

    QBTVBufferingPopupShow(ctx);
}

SvLocal void SvTVContextDeactivateConnectionMonitor(SvTVContext ctx)
{
    ctx->autoRestartDelaySec = 0;
    ctx->channelErrorCode = 0;
    ctx->connectionProblemStartTime = SvTimeGetZero();
    QBTVBufferingPopupHide(ctx);
    QBTVConnectionProblemHide(ctx);
    if (ctx->connectionMonitorFiber) {
        SvFiberDeactivate(ctx->connectionMonitorFiber);
        SvFiberEventDeactivate(ctx->connectionMonitorTimer);
    }

    if (ctx->autoRestartFiber) {
        SvFiberDeactivate(ctx->autoRestartFiber);
        SvFiberEventDeactivate(ctx->autoRestartTimer);
    }
}

SvLocal void SvTVContextActivateConnectionMonitor(SvTVContext ctx)
{
    if (!ctx->connectionMonitorFiber) {
        ctx->connectionMonitorFiber = SvFiberCreate(ctx->appGlobals->scheduler, NULL, "SvTVContextCheckConnectionStep", SvTVContextCheckConnectionStep, ctx);
        ctx->connectionMonitorTimer = SvFiberTimerCreate(ctx->connectionMonitorFiber);
    }

    SvFiberTimerActivateAfter(ctx->connectionMonitorTimer, SvTimeConstruct(1, 0));
}

SvLocal void tvFatalErrorCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->fatalErrorPopup = NULL;
}

void QBTVFatalErrorShow(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->fatalErrorPopup)
        return;

    QBWindowContext windowCtx = (QBWindowContext) ctx;
    AppGlobals appGlobals = tvInfo->ctx->appGlobals;
    const char *message = gettext("FATAL ERROR");
    tvInfo->fatalErrorPopup = QBPlaybackErrorPopupCreate(appGlobals->controller, windowCtx, message, "");
    QBDialogRun(tvInfo->fatalErrorPopup, tvInfo, tvFatalErrorCallback);
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.Playback", "JSON:{\"description\":\"%s\"%s}", message,
                     data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

void QBTVFatalErrorHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->fatalErrorPopup)
        return;
    QBDialogBreak(tvInfo->fatalErrorPopup);
}

SvLocal void QBTVExtraErrorPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    TVInfo *tvInfo = self_;
    tvInfo->extraErrorPopup = NULL;
}

void QBTVExtraErrorShow(QBWindowContext ctx_, const char* title, const char* content)
{
    SvTVContext ctx = (SvTVContext) ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (tvInfo->extraErrorPopup)
        return;
    if (!title)
        return;

    tvInfo->extraErrorPopup = QBPlaybackErrorPopupCreate(tvInfo->ctx->appGlobals->controller, ctx_, title, content);
    QBDialogRun(tvInfo->extraErrorPopup, tvInfo, QBTVExtraErrorPopupCallback);
    char* escapedTitle = QBStringCreateJSONEscapedString(title);
    SvString data = QBTVCreateCurrentEventData(ctx, false, NULL);
    if (content) {
        char* escapedContent = QBStringCreateJSONEscapedString(content);
        QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.Error", "JSON:{\"description\":\"%s : %s\"%s}", escapedTitle, escapedContent,
                         data ? SvStringCString(data) : "");
        free(escapedContent);
    } else {
        QBSecureLogEvent("newtv", "ErrorShown.TVPlayer.Error", "JSON:{\"description\":\"%s\"%s}", escapedTitle,
                         data ? SvStringCString(data) : "");
    }
    SVTESTRELEASE(data);
    free(escapedTitle);
}

bool QBTVExtraErrorIsVisible(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;

    return tvInfo->extraErrorPopup;
}

void QBTVExtraErrorHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;

    if (tvInfo->extraErrorPopup)
        QBDialogBreak(tvInfo->extraErrorPopup);
}

SvLocal void QBTVUpdateFormat(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;

    AudioTrackLogic audioTrackLogic =
        (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    AudioTrackLogicSetupAudioTrack(audioTrackLogic);
    TVInfo *tvInfo = NULL;
    if (ctx->super_.window) {
        tvInfo = ctx->super_.window->prv;
        if (tvInfo->langMenu) {
            QBLangMenuUpdateAudioTracks(tvInfo->langMenu);
        }
    }

    QBTVLogicStopMetaDataManagers(ctx->appGlobals->tvLogic);

    SvGenericObject audio = SvTVChannelGetAttribute(ctx->channel, SVSTRING("audioLang"));
    if (audio && SvObjectIsInstanceOf(audio, SvPlayerAudioTrack_getType())) {
        AudioTrackLogicSetPreferredAudioTrack(audioTrackLogic, (SvPlayerAudioTrack) audio);
    }

    SvGenericObject subsLang = SvTVChannelGetAttribute(ctx->channel, SVSTRING("subsLang"));
    if (subsLang && SvObjectIsInstanceOf(subsLang, SvValue_getType()) && SvValueIsString((SvValue) subsLang)) {
        SvString langStr = SvValueGetString ((SvValue)subsLang);
        SubtitleTrackLogic subtitleTrackLogic =
            (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
        SubtitleTrackLogicSetPreferredLang(subtitleTrackLogic, langStr);
    }

    struct svdataformat* format = QBViewportGetOutputFormat(QBViewportGet());

    const char *hintedLang = NULL;
    int pid = -1;

    if (format && format->ts.teletext_cnt > 0) {
        int idx = 0;
        pid = format->ts.teletext[idx]->pid;
        hintedLang = format->ts.teletext[idx]->lang;

        if (!hintedLang) {
            for (size_t i = 0; i < sizeof(format->ts.teletext[idx]->tab) / sizeof(format->ts.teletext[idx]->tab[0]); ++i) {
                if (format->ts.teletext[idx]->tab[i].lang) {
                    hintedLang = format->ts.teletext[idx]->tab[i].lang;
                    break;
                }
            }
        }
    }

    if (ctx->playerTask && format) {
        QBSubsManagerSetPlayerTask(ctx->subsManager, ctx->playerTask);
        QBTVLogicStartMetaDataManagers(ctx->appGlobals->tvLogic, ctx->subsManager, format, pid, hintedLang);

        QBTVLogicMetaDataManagers metaDataMgrs = QBTVLogicGetMetaDataManagers(ctx->appGlobals->tvLogic);

        if (tvInfo) {
            SvInvokeInterface(TVOSD, tvInfo->OSD, updateFormat, format);
            SvInvokeInterface(TVOSD, tvInfo->OSD, setSubsManager, ctx->subsManager);
        }

        QBViewportVideoInfo videoInfo;
        if (QBViewportGetVideoInfo(QBViewportGet(), &videoInfo) >= 0) {
            // use current mode
            QBTVVideoInfoChanged(ctx, QBViewportGet(), &videoInfo);
        } else {
            // use last known mode
            QBDvbSubsManagerSetDDSMode(metaDataMgrs->dvbSubsManager, ctx->dvbSubsManagerDDSMode);
            if (metaDataMgrs->scte27SubsManager) {
                bool inNTSC = (ctx->dvbSubsManagerDDSMode == QBDvbSubsManagerDDSMode_NTSC);
                QBScte27SubsManagerEnableNTSCHack(metaDataMgrs->scte27SubsManager, inNTSC);
            }
        }
    }

    if (tvInfo) {
        QBTVLogicMetaDataManagers metaDataMgrs = QBTVLogicGetMetaDataManagers(ctx->appGlobals->tvLogic);
        QBTeletextWindowSetReceiver(tvInfo->teletext, metaDataMgrs->teletextReceiver);
        QBTeletextWindowSetStartPage(tvInfo->teletext, NULL, NULL);
        QBTeletextWindowSetHints(tvInfo->teletext, hintedLang);
    }

    SvTVContextUpdateSubtitlesState(ctx, false);
}

bool QBTVRedButtonOverlayIsAvailable(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    return !!tvInfo->redButtonOverlayWindow;
}

bool QBTVRedButtonOverlayIsVisible(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return false;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->redButtonOverlayWindow)
        return false;
    return QBRedButtonOverlayWindowIsVisible(tvInfo->redButtonOverlayWindow);
}

void QBTVRedButtonOverlayShow(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    QBRedButtonOverlayWindowShow(tvInfo->redButtonOverlayWindow);
}

void QBTVRedButtonOverlayHide(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (!ctx->super_.window)
        return;
    TVInfo *tvInfo = ctx->super_.window->prv;
    if (!tvInfo->redButtonOverlayWindow)
        return;
    QBRedButtonOverlayWindowHide(tvInfo->redButtonOverlayWindow);
}

// AMERELES #2206 Canal lineal que lleve a VOD
void QBTVBoldPlayPreviousChannel(QBWindowContext ctx_)
{
    SvTVContext ctx = (SvTVContext)ctx_;
    if (ctx && ctx->previous)
        QBTVLogicPlayPreviousChannel(ctx->appGlobals->tvLogic);
}
