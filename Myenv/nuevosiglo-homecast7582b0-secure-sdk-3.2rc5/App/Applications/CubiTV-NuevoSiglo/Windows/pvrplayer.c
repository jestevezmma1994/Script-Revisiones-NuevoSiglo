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

#include "pvrplayer.h"
#include "pvrplayerInternal.h"

#include <assert.h>
#include <dataformat/sv_data_format.h>
#include <error_codes.h>
#include <libintl.h>
#include <math.h>
#include <player_events/open.h>
#include <player_opts/epg_event.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBSecureLogManager.h>
#include <QBPlayerControllers/QBAnyPlayer.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Logic/AudioTrackLogic.h>
#include <Logic/VideoTrackLogic.h>
#include <Logic/SubtitleTrackLogic.h>
#include <Logic/PVRLogic.h>
#include <Logic/QBParentalControlLogic.h>
#include <Logic/TVOSDInterface.h>
#include <NPvr/QBnPVRRecording.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBMiddlewareBookmark.h>
#include <QBConf.h>
#include <QBInput/QBInputCodes.h>
#include <QBM3uParser.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <QBRecordFS/file.h>
#include <QBRecordFS/RecMetaKeys.h>
#include <QBTextSubsManager.h>
#include <QBWindowContext.h>
#include <Services/QBCASManager.h>
#include <Services/QBCASPopupVisibilityManager.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/HDMIService.h>
#include <Services/core/QBPlaybackMonitor.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvLogQueue.h>
#include <SvCore/SvTime.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvEPGDataLayer/Plugins/Innov8onEPGEvent.h>
#include <SvFoundation/SvData.h>
#include <SvPlayerControllers/SvPlayerTaskControllersListener.h>
#include <SvPlayerControllers/SvPlayerTrackControllerListener.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerManager/SvPlayerTaskListener.h>
#include <SvPlayerKit/SvParsersEvents.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvQuirks/SvQuirks.h>
#include <SWL/icon.h>
#include <Utils/QBContentInfo.h>
#include <Utils/QBEventUtils.h>
#include <Utils/QBMovieUtils.h>
#include <Utils/QBVODUtils.h>
#include <Utils/authenticators.h>
#include <Utils/productUtils.h>
#include <Utils/recordingUtils.h>
#include <Utils/utf8utils.h>
#include <Widgets/authDialog.h>
#include <Widgets/extendedInfo.h>
#include <Widgets/teleosd.h>
#include <Widgets/SideMenu.h>
#include <Widgets/QBPlaybackErrorPopup.h>
#include <QBWidgets/QBDialog.h>
#include <Widgets/confirmationDialog.h>
#include <Windows/QBVoDMovieDetails.h>
#include <Windows/newtv.h>
#include <Menus/QBChannelChoiceMenu.h>
#include <ContextMenus/QBListPane.h>
#include <player_events/prefill.h>
#include <Logic/VoDLogic.h>
#include <QBOSD/QBOSDMain.h>
#include <QBOSD/QBOSDRecording.h>
#include <QBJSONUtils.h>
#include <float.h>
#include "main.h"



 


#define MAX_PLAYLIST_FILE_SIZE 10 * 1024

#define CAS_POPUP_BLOCKER_CONTEXT_ID "pvrplayer"
#define CAS_POPUP_BLOCKER_EXTENDED_INFO_WINDOW_ID "extendedInfo"
#define CAS_POPUP_BLOCKER_LANG_MENU_WINDOW_ID "langMenu"
#define CAS_POPUP_BLOCKER_PIN_POPUP_WINDOW_ID "pinPopup"
#define CAS_POPUP_BLOCKER_SIDE_MENU_WINDOW_ID "sideMenu"




SvLocal bool QBPVRPlayerInputEventHandler(SvWidget w, SvInputEvent ev);

static void dialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBPVRPlayer self = self_;
    self->dialog = NULL;
}

static void setupDialog(QBPVRPlayer self)
{
    self->dialog = QBAnimationDialogCreate(self->ctx->appGlobals->res, self->ctx->appGlobals->controller);
    QBDialogRun(self->dialog, self, dialogCallback);
}

// AMERELES BEGIN BOLD TRACKING
SvLocal SvString BoldGetContentID(QBPVRPlayerContext ctx)
{
    SvString vodId = QBAnyPlayerLogicGetVodId(ctx->anyPlayerLogic);
    if (!vodId && ctx->content)
    {
        vodId = SvURIString(SvContentGetURI(ctx->content));
    }
    
    return vodId;
}
// AMERELES END BOLD TRACKING

SvLocal void prefillDialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBPVRPlayer self = self_;
    self->prefillDialog = NULL;
}

SvLocal void setupPrefillDialog(QBPVRPlayer self)
{
    if (self->prefillDialog)
        return;

    self->prefillDialog = QBAnimationDialogCreate(self->ctx->appGlobals->res, self->ctx->appGlobals->controller);
    QBDialogRun(self->prefillDialog, self, prefillDialogCallback);
}

SvLocal void
QBPVRPlayerContextGetEventsFromMeta(QBPVRPlayerContext ctx, SvEPGEvent *current, SvEPGEvent *following)
{
    QBPVRRecording recording = ctx->rec;
    if (current && recording && recording->epgEvents) {
        SvTimeRange eventTimeRange;
        SvTimeRangeInit(&eventTimeRange, 0, 0);
        if (*current) {
            SvEPGEventGetTimeRange(*current, &eventTimeRange, NULL);
        }
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        time_t currentPlayTime = SvTimeGetSeconds(recording->startTime) + (long int) state.currentPosition;
        if (SvTimeRangeContainsTimePoint(&eventTimeRange, currentPlayTime) == true) {
            return;
        }

        SvIterator it = SvArrayGetIterator(recording->epgEvents);
        SvEPGEvent event = NULL;
        while ((event = (SvEPGEvent) SvIteratorGetNext(&it))) {
            SvEPGEventGetTimeRange(event, &eventTimeRange, NULL);
            if (SvTimeRangeContainsTimePoint(&eventTimeRange, currentPlayTime) == true) {
                *current = event;
                return;
            }
        }
        *current = NULL;
    }
}

SvLocal void
QBPVRPlayerContextGetPresentFollowingEvents(QBPVRPlayerContext ctx, SvEPGEvent *current, SvEPGEvent *following)
{
    struct player_opt_epg_event_s opt = {
        .current = NULL,
    };
    int res = SvPlayerTaskSetOpt(ctx->playerTask, PLAYER_OPT_EPG_EVENT_GET, &opt);
    if (res) {
        if (current) *current = NULL;
        if (following) *following = NULL;
    } else {
        if (current) *current = opt.current;
        if (following) *following = opt.following;
    }
}

SvLocal void
QBPVRPlayerContextGetEvents(QBPVRPlayerContext ctx, SvEPGEvent *current, SvEPGEvent *following)
{
    if (!ctx->playerTask) {
        if (current) *current = NULL;
        if (following) *following = NULL;
        return;
    }

    switch (QBSchedManagerGetEventsSource()) {
        case QBSchedManagerEventSource_PresentFollowing:
            QBPVRPlayerContextGetPresentFollowingEvents(ctx, current, following);
            break;
        case QBSchedManagerEventSource_EPGManager:
            QBPVRPlayerContextGetEventsFromMeta(ctx, current, following);
            break;
        default:
            if (current) *current = NULL;
            if (following) *following = NULL;
            break;
    }
}

SvLocal void
QBPVRPlayerGetCurrentEvent(void *ctx_, SvTVChannel channel, time_t now, SvEPGEvent* current, SvEPGEvent *following)
{
    QBPVRPlayerContext ctx = ctx_;
    QBPVRPlayerContextGetEvents(ctx, current, following);
    if (current && !*current)
      *current=ctx->cutvEvent;
}

SvLocal void
QBPVRPlayerUpdateExtendedInfo(SvWidget w, bool *isEmpty)
{
    QBPVRPlayer self = w->prv;
    QBPVRPlayerContext ctx = self->ctx;

    if (ctx->vodMetaData) {
        bool ret = QBExtendedInfoSetTextByVODMetaData(self->extendedInfo, ctx->vodMetaData);
        if (isEmpty)
            *isEmpty = ret;
        return;
    }
    if (ctx->npvrRec) {
        SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(ctx->appGlobals->eventsLogic,
                                                           ctx->appGlobals->langPreferences,
                                                           ctx->npvrRec->event);
        if (desc)
            QBExtendedInfoSetText(self->extendedInfo, desc->title, desc->description);
        else
            QBExtendedInfoSetText(self->extendedInfo, NULL, NULL);

        if (isEmpty)
            *isEmpty = !desc || (!desc->title && !desc->description);
        return;
    }

    SvEPGEvent current = NULL;
    SvEPGEvent following = NULL;

    if (ctx->cutvEvent) {
        current = ctx->cutvEvent;
    } else if (ctx->playerTask) {
        QBPVRPlayerContextGetEvents(ctx, &ctx->presentEventOSD, &following);
        current = ctx->presentEventOSD;
    }

    if (isEmpty)
        *isEmpty = !current && !following;

    if (current) {
        SvArray events = SvArrayCreate(NULL);
        SvArrayAddObject(events, (SvObject) current);
        if (following)
            SvArrayAddObject(events, (SvObject) following);
        QBExtendedInfoRefreshEvents(self->extendedInfo, events);
        SVRELEASE(events);
    }
}

SvLocal void
QBPVRPlayerContextSavePosition(QBPVRPlayerContext ctx)
{
    if (!ctx->appGlobals->bookmarkManager)
        return;

    QBBookmark bookmark = QBAnyPlayerLogicGetLastPositionBookmark(ctx->anyPlayerLogic);
    if (!bookmark)
        return;

    double pos = 0.0;
    if (!ctx->isEOS) {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        pos = state.currentPosition;
    }

    if (ctx->rec && ctx->playerTask) {
        /// If the recording is completed, and we have reached end-of-stream, we reset the position.
        /// If the recording is still ongoing, then, even if we reached the end (for now), we should start from this position anyway.
        if (ctx->isEOS && ctx->rec->state != QBPVRRecordingState_active) {
            QBBookmarkManagerRemoveBookmark(ctx->appGlobals->bookmarkManager, bookmark);
            return;
        }

        QBBookmarkUpdatePosition(bookmark, pos);
        QBBookmarkManagerUpdateBookmark(ctx->appGlobals->bookmarkManager, bookmark);
    } else {
        if (ctx->canSeek) {

            // reset saved position if we reached end of stream
            if (ctx->isEOS) {
                QBBookmarkManagerRemoveBookmark(ctx->appGlobals->bookmarkManager, bookmark);
            } else {
                QBBookmarkUpdatePosition(bookmark, pos);
                QBBookmarkManagerUpdateBookmark(ctx->appGlobals->bookmarkManager, bookmark);
            }
        } else {
            // remove saved position of content without seeking capability
            QBBookmarkManagerRemoveBookmark(ctx->appGlobals->bookmarkManager, bookmark);

        }

        return;
    }
}

SvLocal void OSDChangedPlaySpeed(QBPVRPlayer self, double wantedSpeed)
{
    QBPVRPlayerContext ctx = self->ctx;
    if (!ctx->playerTask)
        return;

    QBOSD osd = QBOSDMainGetHandler(self->OSD, QBOSDHandlerType_recording, NULL);
    if (!osd)
        return;

    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    QBOSDRecordingUpdatePlaybackState((QBOSDRecording) osd, &state, wantedSpeed, NULL);
}

SvLocal void OSDChangedPosition(QBPVRPlayer self, double pos, double end)
{
    SvInvokeInterface(TVOSD, self->OSD, setPosition, pos, end);
}

SvLocal void
QBPVRPlayerHDCPAuthFailedCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBPVRPlayer self = self_;
    self->HDCPAuthFailedPopup = NULL;
}

SvLocal void
QBPVRPlayerHDCPAuthFailedShow(QBPVRPlayer self)
{
    QBWindowContext windowCtx = (QBWindowContext) self->ctx;
    AppGlobals appGlobals = self->ctx->appGlobals;
    const char *message = gettext("HDCP ERROR");
    self->HDCPAuthFailedPopup = QBPlaybackErrorPopupCreate(appGlobals->controller, windowCtx, message, "");
    QBDialogRun(self->HDCPAuthFailedPopup, self, QBPVRPlayerHDCPAuthFailedCallback);
    SvString data = QBPVRPlayerContextCreateEventData((QBWindowContext) self->ctx, NULL);
    QBSecureLogEvent("PVRPlayer", "ErrorShown.PVRPlayer.HDCPAuthFailed", "JSON:{\"description\":\"%s\"%s}",
                     message, data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
}

SvLocal void
QBPVRPlayerContextHDCPAuthStatusChanged(SvObject self_, HDCPAuthStatus status)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) self_;
    QBWindowContext windowCtx = (QBWindowContext) ctx;
    QBPVRPlayer self = (QBPVRPlayer) windowCtx->window->prv;

    if (status == HDCPAuth_error) {
        if (!self->HDCPAuthFailedPopup) {
            QBPVRPlayerHDCPAuthFailedShow(self);
        }
    } else { // HDCP authorization success or this output don't support HDCP or HDCP disabled
        if (self->HDCPAuthFailedPopup) {
            QBDialogBreak(self->HDCPAuthFailedPopup);
        }
    }
}

SvLocal void
QBPVRPlayerContextHotplugStatusChanged(SvObject self_, bool isConnected)
{
}

void
QBPVRPlayerOSDShow(QBWindowContext ctx_, bool immediate, bool force)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    SvInvokeInterface(TVOSD, self->OSD, show, immediate, force);
}

void
QBPVRPlayerOSDLock(QBWindowContext ctx_, bool lock, bool force)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    if (lock)
        SvInvokeInterface(TVOSD, self->OSD, lock);
    else
        SvInvokeInterface(TVOSD, self->OSD, unlock, force);
}

void
QBPVRPlayerOSDHide(QBWindowContext ctx_, bool immediate, bool force)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    SvInvokeInterface(TVOSD, self->OSD, hide, immediate, force);
}

bool
QBPVRPlayerOSDIsVisible(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return false;
    QBPVRPlayer self = ctx->super_.window->prv;

    return SvInvokeInterface(TVOSD, self->OSD, isShown);
}

void QBPVRPlayerPlay(QBWindowContext ctx_, double position)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask)
        return;
    
    // AMERELES BEGIN BOLD TRACKING
    SvString vodId = BoldGetContentID(ctx);
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    BoldContentTrackingSendMessageCommand(vodId, SvStringCreate("resume", NULL), state.currentPosition, QBMovieUtilsGetAttr(ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp);
    // AMERELES END BOLD TRACKING
    
    QBPVRPlayer self = ctx->super_.window->prv;
    SvPlayerTaskPlay(ctx->playerTask, 1.0, position, NULL);
    OSDChangedPlaySpeed(self, 1.0);
}

void QBPVRPlayerPause(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask) {
        return;
    } else if (ctx->content && SvContentGetNoPauseConstraint(ctx->content)) {
        return;
    }
    
    // AMERELES BEGIN BOLD TRACKING
    if (!ctx->keepAliveError)
    {
        SvString vodId = BoldGetContentID(ctx);
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        BoldContentTrackingSendMessageCommand(vodId, SvStringCreate("pause", NULL), state.currentPosition, QBMovieUtilsGetAttr(ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp);
    }
    // AMERELES END BOLD TRACKING

    QBPVRPlayer self = ctx->super_.window->prv;
    SvPlayerTaskPlay(ctx->playerTask, 0.0, -1.0, NULL);
    OSDChangedPlaySpeed(self, 0.0);
}

void QBPVRPlayerREW(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask) {
        return;
    } else if (ctx->content && SvContentGetNoTrickConstraint(ctx->content)) {
        return;
    }
    QBPVRPlayer self = ctx->super_.window->prv;
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    float min;
    float max;
    bool loop = false;
    QBAnyPlayerLogicGetSpeedBoundary(ctx->anyPlayerLogic, &min, &max, NULL, NULL, &loop);
    if (self->prefillDialog) {
        QBDialogBreak(self->prefillDialog);
    }
    if (state.wantedSpeed >= 0) {
        SvPlayerTaskPlay(ctx->playerTask, max, -1, NULL);
        OSDChangedPlaySpeed(self, max);
    } else {
        float wantedSpeed = 2 * state.wantedSpeed;
        if  (wantedSpeed < min) {
            if (loop) {
                wantedSpeed = 1.0;
            } else {
                return;
            }
        }

        SvPlayerTaskPlay(ctx->playerTask, wantedSpeed, -1, NULL);
        OSDChangedPlaySpeed(self, wantedSpeed);
    }

    QBSubsManagerFlushSubtitleForTrack(ctx->subsManager, QBSubsManagerGetCurrentTrack(ctx->subsManager));
}

void QBPVRPlayerFFW(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window || !ctx->playerTask) {
        return;
    } else if (ctx->content && SvContentGetNoTrickConstraint(ctx->content)) {
        return;
    }
    QBPVRPlayer self = ctx->super_.window->prv;
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    float min;
    float max;
    bool loop = false;
    QBAnyPlayerLogicGetSpeedBoundary(ctx->anyPlayerLogic, NULL, NULL, &min, &max, &loop);
    if (self->prefillDialog) {
        QBDialogBreak(self->prefillDialog);
    }
    if (state.wantedSpeed <= 1) {
        SvPlayerTaskPlay(ctx->playerTask, min, -1, NULL);
        OSDChangedPlaySpeed(self, min);
    } else {
        float wantedSpeed = 2 * state.wantedSpeed;
        if (wantedSpeed > max) {
            if (loop) {
                wantedSpeed = 1.0;
            } else {
                return;
            }
        }

        SvPlayerTaskPlay(ctx->playerTask, wantedSpeed, -1, NULL);
        OSDChangedPlaySpeed(self, wantedSpeed);
    }

    QBSubsManagerFlushSubtitleForTrack(ctx->subsManager, QBSubsManagerGetCurrentTrack(ctx->subsManager));
}

double QBPVRPlayerContextGetLastPositon(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    double last_pos = -1.0;

    if (!ctx->super_.window)
        return last_pos;

    if (!ctx->appGlobals->bookmarkManager)
        return last_pos;

    if ((last_pos = QBAnyPlayerLogicGetStartPosition(ctx->anyPlayerLogic)) > -1)
        return last_pos;
    QBBookmark lastPositionBookmark = QBAnyPlayerLogicGetLastPositionBookmark(ctx->anyPlayerLogic);
    if (!lastPositionBookmark)
        return last_pos;

    last_pos = QBBookmarkGetPosition(lastPositionBookmark);

    SvLogNotice("QBPVRPlayer : start from position [%2.3f]", last_pos);
    return last_pos;
}

double QBPVRPlayerGetSpeed(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->playerTask)
        return 0.0;
    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    return state.wantedSpeed;
}

int QBPVRPlayerGetPlayerState(QBWindowContext ctx_, SvPlayerTaskState *state)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->playerTask || !state)
        return -1;
    *state = SvPlayerTaskGetState(ctx->playerTask);

    return 0;
}

void QBPVRPlayerNextContent(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->musicPlaylist)
        return;

    if (ctx->musicPlaylist && SvArrayCount(ctx->musicPlaylist) > ctx->playlistPos + 1) {
        ctx->playlistPos++;
        QBPVRPlayerContextStartPlayback(ctx_, 0.0);
        return;
    }
}

void QBPVRPlayerPrevContent(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->musicPlaylist)
        return;

    if (ctx->musicPlaylist && ctx->playlistPos > 0) {
        ctx->playlistPos--;
        QBPVRPlayerContextStartPlayback(ctx_, 0.0);
        return;
    }
}

SvLocal SvString QBPVRPlayerGetDescriptionFromVODMetadata(SvDBRawObject meta)
{
    SvValue value = NULL;

    value = (SvValue) SvDBRawObjectGetAttrValue(meta, "Description");
    if (!value)
        return NULL;
    return SvValueGetString(value);
}

bool QBPVRPlayerExtendedInfoShow(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return false;

    QBPVRPlayer self = ctx->super_.window->prv;
    if (!SvInvokeInterface(TVOSD, self->OSD, isShowingMoreInfo)) {
        bool isEmpty = false;
        QBPVRPlayerUpdateExtendedInfo(ctx->super_.window, &isEmpty);
        SvString description = QBPVRPlayerGetDescriptionFromVODMetadata(ctx->vodMetaData);
        SvInvokeInterface(TVOSD, self->OSD, setDescription, description);
        if (!isEmpty) {
            SvInvokeInterface(TVOSD, self->OSD, showMoreInfo);
            if (ctx->appGlobals->casPopupVisabilityManager) {
                QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_EXTENDED_INFO_WINDOW_ID));
            }

            return true;
        } else
            return false;
    }
    return true;
}

void QBPVRPlayerExtendedInfoHide(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;
    SvInvokeInterface(TVOSD, self->OSD, showLessInfo);
}

bool QBPVRPlayerExtendedInfoIsVisible(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return false;
    QBPVRPlayer self = ctx->super_.window->prv;
    return SvInvokeInterface(TVOSD, self->OSD, isShowingMoreInfo);
}

void QBPVRPlayerLangMenuShow(QBWindowContext ctx_, QBLangMenuSubMenu submenu)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    if (!QBPVRPlayerLangMenuIsVisible(ctx_)) {
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_LANG_MENU_WINDOW_ID));
        }
        QBLangMenuShow(self->langMenu, submenu);
    }
}

void QBPVRPlayerLangMenuHide(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    if (QBPVRPlayerLangMenuIsVisible(ctx_)) {
        QBLangMenuHide(self->langMenu);
        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_LANG_MENU_WINDOW_ID));
        }
    }
}

bool QBPVRPlayerLangMenuIsVisible(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return false;
    QBPVRPlayer self = ctx->super_.window->prv;

    return QBLangMenuIsVisible(self->langMenu);
}

bool QBPVRPlayerTeletextIsAvailable(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    return ctx->teletextLineReceiver ? true : false;
}

void QBPVRPlayerTeletextSetTransparent(QBWindowContext ctx_, bool transparent)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;
    QBTeletextWindowSetTransparent(self->teletext, transparent);
}

bool QBPVRPlayerTeletextIsTransparent(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return false;
    QBPVRPlayer self = ctx->super_.window->prv;
    return QBTeletextWindowIsTransparent(self->teletext);
}

bool QBPVRPlayerTeletextIsVisible(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return false;
    QBPVRPlayer self = ctx->super_.window->prv;
    return QBTeletextWindowIsVisible(self->teletext);
}

void QBPVRPlayerTeletextHide(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    if (QBPVRPlayerTeletextIsVisible(ctx_)) {
        QBTeletextWindowHide(self->teletext);
    }
}

void QBPVRPlayerTeletextShow(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->super_.window)
        return;
    QBPVRPlayer self = ctx->super_.window->prv;

    if (QBPVRPlayerTeletextIsVisible(ctx_))
        return;

    QBTeletextWindowShow(self->teletext);

    const char *clang = QBConfigGet("SUBTITLESLANG"); //FIXME subtitle lang, menu lang or ... ?
    QBTeletextWindowSetStartPage(self->teletext, QBViewportGetOutputFormat(QBViewportGet()), clang);
}

SvLocal void
QBPVRPlayerTeletextSetup(QBPVRPlayerContext ctx, struct svdataformat* format)
{
    assert(ctx);

    const char *hintedLang = NULL;
    int pid = -1;
    if (format && format->ts.teletext_cnt > 0) {
        const int idx = 0;
        hintedLang = format->ts.teletext[idx]->lang;
        pid = format->ts.teletext[idx]->pid;
    }

    if (ctx->playerTask && pid >= 0) {
        ctx->teletextLineReceiver = QBTeletextLineReceiverNew(ctx->playerTask, pid);
        QBTeletextLineReceiverStart(ctx->teletextLineReceiver);
        ctx->teletextReceiver = QBTeletextReceiverNew(ctx->teletextLineReceiver);
        ctx->teletextSubsManager = QBTeletextSubtitlesManagerNew(ctx->subsManager, ctx->teletextReceiver);
        QBTeletextSubtitlesManagerSetDataFormat(ctx->teletextSubsManager, format);
        QBTeletextSubtitlesManagerSetHints(ctx->teletextSubsManager, hintedLang);
    }

    QBWindowContext windowCtx = (QBWindowContext) ctx;
    if (windowCtx->window) {
        QBPVRPlayer self = windowCtx->window->prv;
        QBTeletextWindowSetReceiver(self->teletext, ctx->teletextReceiver);
        QBTeletextWindowSetHints(self->teletext, hintedLang);
    }
}

SvLocal void
QBPVRPlayerContextAttachSubtitle(QBWindowContext ctx_, SvWidget window)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    QBSubsManagerAttach(ctx->subsManager, window, 0, 0, window->width, window->height, 0);
}

SvLocal void
QBPVRPlayerContextDetachSubtitle(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    QBSubsManagerDetach(ctx->subsManager);
}

SvLocal void
QBPVRPlayerSetAudioTrack(void *ctx_, SvPlayerAudioTrack audioTrack)
{
}

SvLocal void QBPVRPlayerCheckPINCallback(void *ctx_, SvWidget dlg, SvString ret, unsigned wantedSpeed)
{
    QBPVRPlayerContext ctx = ctx_;
    QBPVRPlayer self = ctx->super_.window->prv;

    if (!self->parentalPopup)
        return;

    self->parentalPopup = NULL;
    if (ctx->anyPlayerLogic)
        QBAnyPlayerLogicUnlockDisplay(ctx->anyPlayerLogic);

    if (ret && SvStringEqualToCString(ret, "OK-button")) {
        QBParentalControlHelperAuthenticated(ctx->pc);

        if (ctx->appGlobals->casPopupVisabilityManager) {
            QBCASPopupVisibilityManagerUnblock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_PIN_POPUP_WINDOW_ID));
        }

    }
    SVRELEASE(ctx);
}

SvLocal void QBPVRPlayerCheckPIN(QBPVRPlayer self)
{
    QBPVRPlayerContext ctx = self->ctx;
    if (ctx->anyPlayerLogic)
        QBAnyPlayerLogicLockDisplay(ctx->anyPlayerLogic);
    SvGenericObject authenticator = QBAuthenticateViaAccessManager(ctx->appGlobals->scheduler, ctx->appGlobals->accessMgr, ctx->accessDomain);
    SvWidget master = NULL;
    svSettingsPushComponent("PVRParentalControlDialog.settings");
    SvWidget dialog = QBAuthDialogCreate(ctx->appGlobals, authenticator, gettext("Authentication required"), 0, true, NULL, &master);
    svSettingsPopComponent();

    self->parentalPopup = dialog;

    if (ctx->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerBlock(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_PIN_POPUP_WINDOW_ID));
    }
    QBDialogRun(dialog, SVRETAIN(ctx), QBPVRPlayerCheckPINCallback);
}

#define PVR_NAME_IN_VOLUME_MUTE_TABLE "PVR"

SvLocal void QBPVRPlayerContextMute(QBPVRPlayerContext ctx, bool mute)
{
    int level;
    bool wasVolumeLocked;
    QBVolumeGetCurrentState(ctx->appGlobals->volume, &level, NULL, &wasVolumeLocked);
    QBVolumeUnlock(ctx->appGlobals->volume);
    QBVolumeMute(ctx->appGlobals->volume, SVSTRING(PVR_NAME_IN_VOLUME_MUTE_TABLE), mute);
    if (wasVolumeLocked)
        QBVolumeLock(ctx->appGlobals->volume, level);
}

SvLocal void QBPVRPlayerContextEnableAV(QBPVRPlayerContext ctx)
{
    QBPVRPlayerContextMute(ctx, false);

    if (ctx->super_.window) {
        QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_fullScreen, NULL);
        QBPVRPlayerContextAttachSubtitle((QBWindowContext) ctx, ctx->super_.window);
    }
}

SvLocal void QBPVRPlayerContextDisableAV(QBPVRPlayerContext ctx)
{
    QBPVRPlayerContextMute(ctx, true);

    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);
    QBPVRPlayerContextDetachSubtitle((QBWindowContext) ctx);
}

SvLocal void QBPVRPlayerContextStartAuthentication(QBPVRPlayerContext ctx)
{
    QBPVRPlayer self = NULL;

    if (ctx->super_.window && ctx->super_.window->prv) {
        self = ctx->super_.window->prv;
    }

    QBPVRPlayerContextDisableAV(ctx);
    if (self && !self->parentalPopup) {
        QBPVRPlayerCheckPIN(self);
    }

    ctx->waitingForAuthentication = true;
}

SvLocal void QBPVRPlayerContextFinishAuthentication(QBPVRPlayerContext ctx)
{
    QBPVRPlayer self = NULL;

    if (ctx->super_.window && ctx->super_.window->prv) {
        self = ctx->super_.window->prv;
    }

    QBPVRPlayerContextEnableAV(ctx);
    if (self && self->parentalPopup) {
        QBDialogBreak(self->parentalPopup);
    }

    ctx->waitingForAuthentication = false;
}

SvLocal void QBPVRPlayerContextAuthenticationStateChanged(void *_ctx)
{
    QBPVRPlayerContext ctx = _ctx;
    QBParentalControlLogicAuthState state = QBParentalControlLogicGetAuthState(ctx->appGlobals->parentalControlLogic, ctx->pc);

    if (state == QBParentalControlLogicAuthState_EnableAV) {
        QBPVRPlayerContextFinishAuthentication(ctx);
    } else if (state == QBParentalControlLogicAuthState_PINPopup_DisableAV) {
        QBPVRPlayerContextStartAuthentication(ctx);
    } else if (state == QBParentalControlLogicAuthState_DisableAV) {
        QBPVRPlayerContextDisableAV(ctx);
    }
}

// AMERELES BEGIN BOLD TRACKING
SvLocal SvString create_message_from_error_code(int code)
{
    if (code <= SV_ERR_INIT_DECODER && code > SV_ERR_DRM_INIT_FAILURE) {
        return SvStringCreateWithFormat("Decoder error: %d", code);
    } else if (code <= SV_ERR_DRM_INIT_FAILURE && code > SV_ERR_SERVER_CONN_FAILED) {
        return SvStringCreateWithFormat("DRM error: %d", code);
    } else if (code <= SV_ERR_SERVER_CONN_FAILED && code > SV_ERR_CPC_XML_SYNTAX_ERROR) {
        switch (code) {
            case SV_ERR_SERVER_FILE_NOT_FOUND:  return SvStringCreateWithFormat("File not found: %d", code);
            case SV_ERR_UNSUPPORTED_FORMAT:     return SvStringCreateWithFormat("Format not supported: %d", code);
            case SV_ERR_UNSUPPORTED_PROTOCOL:   return SvStringCreateWithFormat("Protocol not supported: %d", code);
            case SV_ERR_BAD_URL:                return SvStringCreateWithFormat("Bad URL: %d", code);
            case SV_ERR_BAD_SERVER_RESPONSE:    return SvStringCreateWithFormat("Bad server response: %d", code);
            case SV_ERR_UNEXPECTED_SERVER_RESPONSE: return SvStringCreateWithFormat("Unexpected server response: %d", code);
            default:                            return SvStringCreateWithFormat("Server communication error: %d", code);
        }
    } else if (code == SV_ERR_NOT_FOUND) {
        return SvStringCreate("File not found", NULL);
    } else if (code == SV_ERR_BAD_FORMAT) {
        return SvStringCreateWithFormat("Bad format: %d", code);
    } else if (code == 40008) {
        return SvStringCreateWithFormat("maximum number of devices was reached");
    }
    return SvStringCreateWithFormat("Playback failed with error code: %d", code);
}

SvLocal void QBPVRPlayerContextLogError(QBPVRPlayerContext ctx, SvErrorInfo errorInfo)
{
    SvString vodId = BoldGetContentID(ctx);
    
    int errorCode =  SvErrorInfoGetCode(errorInfo);
    double position = 0;
    if (ctx->playerTask)
    {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        position = state.currentPosition;
    }
    
    BoldContentTrackingSendMessageError(vodId, SvStringCreateWithFormat("%d", errorCode), 
        create_message_from_error_code(errorCode), time(NULL) - ctx->initTimeStamp, QBMovieUtilsGetAttr(ctx->vodMetaData, "source", SVSTRING("")), position);
}

SvLocal void
concurrencyPopUpCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBPVRPlayer self = (QBPVRPlayer) self_;
    if (!self)
        return;
    
    SvInputEvent event = malloc(sizeof(SvInputEvent));
    event->type = QBInputEventType_keyTyped;
    event->ch = QBKEY_STOP;
    event->fullInfo.type = QBInputEventType_keyTyped;
    event->fullInfo.src = 0;
    QBPVRPlayerInputEventHandler(self->window, event);
    free(event);
}

SvLocal void keepAliveCallback(void * self_,  char* buff, size_t len)
{
    QBPVRPlayer self = (QBPVRPlayer) self_;
    if (!self)
        return;
    
    if (/*!buff || */(buff && strstr(buff, "ERROR")))
    {
        SvErrorInfo errorInfo = SvErrorInfoCreate(SvCoreErrorDomain, 40008, "maximum number of devices was reached");
        QBPVRPlayerContextLogError(self->ctx, errorInfo);
        
        self->ctx->keepAliveError = true;
        QBAnyPlayerLogicPause(self->ctx->anyPlayerLogic);
        
        QBConfirmationDialogParams_t params = {
                .title = "Problema de Concurrencia",
                .message = "El número máximo de dispositivos por usuario se ha excedido.",
                .local = true,
                .focusOK = true,
                .isCancelButtonVisible = false
        };

        svSettingsPushComponent("Dialog.settings");
        SvWidget dialog = QBConfirmationDialogCreate(self->ctx->appGlobals->res, &params);
        svSettingsPopComponent();

        if (dialog)
        {
            QBDialogRun(dialog, self, concurrencyPopUpCallback);
        }
    }
}
// AMERELES END BOLD TRACKING

SvLocal void
QBPVRPlayerTimerEventHandler(SvWidget w, SvTimerEvent te)
{
    QBPVRPlayer self = w->prv;
    QBPVRPlayerContext ctx = self->ctx;

    if (te->id != self->refreshTimer)
        return;

    bool isRecording = false;

    if (!ctx->playerTask)
        return;

    QBOSD osd = QBOSDMainGetHandler(self->OSD, QBOSDHandlerType_recording, NULL);
    QBOSDRecordingUpdateEvents((QBOSDRecording) osd, NULL);

    // TODO : This method should be implemented in TS InfoIcon object
    SvInvokeInterface(TVOSD, self->OSD, update);

    struct svdataformat *format = QBViewportGetOutputFormat(QBViewportGet());
    bool hideCover = (!format) || format->video_cnt;
    SvInvokeInterface(TVOSD, self->OSD, setCoverVisibility, !hideCover);

    if (ctx->rec) {
        if (!ctx->rec->playable)
            return;

        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        OSDChangedPosition(self, state.currentPosition,
                           QBPVRProviderGetRecordingCurrentDuration(ctx->appGlobals->pvrProvider, ctx->rec));

        SvString name = QBRecordingUtilsCreateName(ctx->appGlobals, ctx->rec, false, false);
        SvInvokeInterface(TVOSD, self->OSD, setTVChannelName, name);
        SVRELEASE(name);

        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        SvValue channelVal = SvValueCreateWithString(ctx->rec->channelId, NULL);
        SvTVChannel channel = SvEPGManagerFindTVChannel(epgManager, channelVal, NULL);
        if (channel) {
            SvInvokeInterface(TVOSD, self->OSD, setTVChannel, channel, false);
        }
        SVRELEASE(channelVal);

        QBPVRPlayerContextGetEvents(ctx, &ctx->presentEventOSD, NULL);
        if (ctx->presentEventOSD) {
            SvInvokeInterface(TVOSD, self->OSD, setCurrentEvent, ctx->presentEventOSD);
        }
        QBPVRPlayerUpdateExtendedInfo(w, NULL);
    }
    else if (ctx->npvrRec)
    {
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        if (ctx->npvrRec->state == QBnPVRRecordingState_active) {
            time_t npvrRecordingStart = SvTimeGetSeconds(ctx->npvrRec->startTime);
            time_t playPos = npvrRecordingStart + (int)state.currentPosition;
            double duration = state.duration;
            if (duration <= 0.0)
                duration = state.currentPosition;
            OSDChangedPosition(self, state.currentPosition, duration);
            SvTime now = SvTimeGet();
            if (SvTimeCmp(ctx->npvrRec->endTime, now) == -1 && osd) {
                QBOSDRecordingUpdateTimes((QBOSDRecording) osd, NULL, playPos, npvrRecordingStart, SvTimeGetSeconds(ctx->npvrRec->endTime), NULL);
            } else {
                QBOSDRecordingUpdateTimes((QBOSDRecording) osd, NULL, playPos, npvrRecordingStart, SvTimeNow(), NULL);
            }
        } else {
            double duration = (double) SvTimeToMs(SvTimeSub(ctx->npvrRec->endTime, ctx->npvrRec->startTime)) / 1000;
            OSDChangedPosition(self, state.currentPosition, duration);
        }

        SvInvokeInterface(TVOSD, self->OSD, setTVChannelName, ctx->npvrRec->channelName);

        if (ctx->npvrRec->event) {
            SvInvokeInterface(TVOSD, self->OSD, setCurrentEvent, ctx->npvrRec->event);
        }
        QBPVRPlayerUpdateExtendedInfo(w, NULL);
    } else if (ctx->playbackType == QBPVRPlayerContextPlaybackType_catchup || ctx->playbackType == QBPVRPlayerContextPlaybackType_startOver) {
        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        SvTVChannel channel = SvEPGManagerFindTVChannel(epgManager, ctx->cutvEvent->channelID, NULL);
        if (channel) {
            SvInvokeInterface(TVOSD, self->OSD, setTVChannelName, channel->name);
        }
        SvInvokeInterface(TVOSD, self->OSD, setCurrentEvent, ctx->cutvEvent);

        double duration = (double) (ctx->cutvEvent->endTime - ctx->cutvEvent->startTime);
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        OSDChangedPosition(self, state.currentPosition, duration);
        time_t startTime = ctx->cutvEvent->startTime;
        time_t endTime = ctx->cutvEvent->startTime + (time_t) state.duration;
        time_t playPos = startTime + (int) state.currentPosition;

        SvInvokeInterface(TVOSD, self->OSD, checkForUpdates, playPos, startTime, endTime);
    } else if (ctx->content || ctx->musicPlaylist) {
        SvContent currentContent = ctx->content ? : (SvContent) SvArrayObjectAtIndex(ctx->musicPlaylist, ctx->playlistPos);
        SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
        SvContentMetaData meta = SvContentGetMetaData(currentContent);
        double duration = state.duration;
        OSDChangedPosition(self, state.currentPosition, duration);

        SvString title = SVSTRING("");
        SvContentMetaDataGetStringProperty(meta, SVSTRING(SV_PLAYER_META__TITLE), &title);
        SvInvokeInterface(TVOSD, self->OSD, setTitle, title);
        //self->osd.channelFrame->hidden = true; /// FIXME: wtf -deffer
            
        // AMERELES BEGIN BOLD TRACKING
        SvString vodId = BoldGetContentID(ctx);
        if (self->ctx->initTimePlayWasSent)
        {
            if (state.bufferedSec <= 2)
            {
                if (!ctx->bufferUnderrunStarted)
                {
                    ctx->bufferUnderrunInit = time(NULL);
                    ctx->bufferUnderrunStarted = true;
                }
            }
            else
            {
                if (ctx->bufferUnderrunStarted)
                {
                    ctx->bufferUnderrunStarted = false;
                    BoldContentTrackingSendMessageBufferUnderrun(vodId, SvStringCreate("bufferunderrun", NULL), state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->bufferUnderrunInit);
                }
            }
        }
            
        double currentBitrate = QBAdBitrateGetCurrentBitrate(ctx->adBitrate);
        currentBitrate = currentBitrate / 1000000;
        
        double actualPercentage = duration > 0 ? (state.currentPosition / duration) * 100 : 0;
        if ((actualPercentage > 0.000001) && (!ctx->keepAliveError) && ctx->keepAliveLastTimeSended + BOLD_FOX_TRACKING_KEEPALIVE <= time(NULL))
        {
            BoldContentTrackingSendMessageKeepAlive(vodId, keepAliveCallback, (void*)self, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")));
            ctx->keepAliveLastTimeSended = time(NULL);
        }
        
        if (0.1 <= actualPercentage && actualPercentage < 0.11)
        {
            if (ctx->actualPercentageDisplayed != QBPercentageDisplayed_0)
            {
                BoldContentTrackingSendMessageProgress(vodId, SvStringCreate("progress", NULL), 
                                        state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, 0, currentBitrate);
                ctx->actualPercentageDisplayed = QBPercentageDisplayed_0;
            }
        }
        else if (9.09 < actualPercentage && actualPercentage < 10.1)
        {
            if (ctx->actualPercentageDisplayed != QBPercentageDisplayed_10)
            {
                BoldContentTrackingSendMessageProgress(vodId, SvStringCreate("progress", NULL), 
                                        state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, 10, currentBitrate);
                ctx->actualPercentageDisplayed = QBPercentageDisplayed_10;
            }
        }
        else if (24.09 < actualPercentage && actualPercentage < 25.1)
        {
            if (ctx->actualPercentageDisplayed != QBPercentageDisplayed_25)
            {
                BoldContentTrackingSendMessageProgress(vodId, SvStringCreate("progress", NULL), 
                                        state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, 25, currentBitrate);
                ctx->actualPercentageDisplayed = QBPercentageDisplayed_25;
            }
        }
        else if (49.09 < actualPercentage && actualPercentage < 50.1)
        {
            if (ctx->actualPercentageDisplayed != QBPercentageDisplayed_50)
            {
                BoldContentTrackingSendMessageProgress(vodId, SvStringCreate("progress", NULL), 
                                        state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, 50, currentBitrate);
                ctx->actualPercentageDisplayed = QBPercentageDisplayed_50;
            }
        }
        else if (74.09 < actualPercentage && actualPercentage < 75.1)
        {
            if (ctx->actualPercentageDisplayed != QBPercentageDisplayed_75)
            {
                BoldContentTrackingSendMessageProgress(vodId, SvStringCreate("progress", NULL), 
                                        state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, 75, currentBitrate);
                ctx->actualPercentageDisplayed = QBPercentageDisplayed_75;
            }
        }
        else if (94.09 < actualPercentage && actualPercentage < 95.1)
        {
            if (ctx->actualPercentageDisplayed != QBPercentageDisplayed_95)
            {
                BoldContentTrackingSendMessageProgress(vodId, SvStringCreate("progress", NULL), 
                                        state.currentPosition, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, 95, currentBitrate);
                ctx->actualPercentageDisplayed = QBPercentageDisplayed_95;
            }
        }
        else if (actualPercentage < 0.1)
        {
            // DO NOTHING
        }
        else
        {
            ctx->actualPercentageDisplayed = QBPercentageDisplayed_NONE;
        }
        
        if (!ctx->initTimePlayWasSent && 0 <= state.currentPosition)
        {
            if (state.currentPosition <= ctx->initTimeInitialCurrentPosition)
            {
                ctx->initTimeInitialCurrentPosition = state.currentPosition;
            }
            else
            {
                BoldContentTrackingSendMessageJoinTime(vodId, SvStringCreate("jointime", NULL), time(NULL) - ctx->initTimePlay, QBMovieUtilsGetAttr(self->ctx->vodMetaData, "source", SVSTRING("")));
                ctx->initTimePlayWasSent = true;
            }
        }
        // AMERELES END BOLD TRACKING
        
        // BEGIN AMERELES [#2406] Agregar información de Bitrate al reproducir VoD
        if (time(NULL) - ctx->initTimeStamp > 5)
        {  
            char bitrateActual[1024];
            sprintf(bitrateActual, " \n\n\n\n\n\n\n\n\n\nBitrate: %.1f Mbps \nBuffer: %.1f Seg.", currentBitrate, state.bufferedSec);
            
            SvString temp = SvStringCreate(bitrateActual, NULL);//  # 4149
            QBExtendedInfoSetPlayerInfoLabelText(self->extendedInfo,temp);
            SVRELEASE(temp);// #4149
             
        }
        // END AMERELES [#2406] Agregar información de Bitrate al reproducir VoD
    }

    SvInvokeInterface(TVOSD, self->OSD, setRecordingStatus, isRecording);
}

SvLocal bool
QBPVRPlayerInputEventHandler(SvWidget w, SvInputEvent ev)
{
    QBPVRPlayer self = w->prv;
    QBPVRPlayerContext ctx = self->ctx;

    SvErrorInfo error = NULL;
    bool isMuted = QBVolumeIsMuted(ctx->appGlobals->volume, &error);
    if ((ev->ch == QBKEY_MUTE || ev->ch == QBKEY_VOLUP || ev->ch == QBKEY_VOLDN) && isMuted) {
        if (ctx->playbackMonitor) {
            QBPlaybackMonitorVolumeUpdate(ctx->playbackMonitor, false);
        }
    } else if (ev->ch == QBKEY_MUTE && (!isMuted || error)) {
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
        }
        if (ctx->playbackMonitor) {
            QBPlaybackMonitorVolumeUpdate(ctx->playbackMonitor, true);
        }
    }

    if (self->parentalPopup && ev->ch == QBKEY_BACK) {
        //Return to previous view
        QBApplicationControllerPopContext(ctx->appGlobals->controller);
        return true;
    }

    // BEGIN AMERELES Setear canSeek en en link para CatchUp
    switch (ev->ch) {
        case QBKEY_REW:
            if (ctx->content && SvContentGetURI(ctx->content))
            {
                // AMERELES BEGIN BOLD TRACKING
                if (!ctx->seekStarted)
                {
                    ctx->seekStarted = true;
                    ctx->seekInit = time(NULL);
                }
                
                SvString url = SvURIString(SvContentGetURI(ctx->content));
                if (strstr(SvStringCString(url), "can_rw=false"))
                {
                    return true;
                }
            }
            break;
        case QBKEY_FFW:
            if (ctx->content && SvContentGetURI(ctx->content))
            {
                // AMERELES BEGIN BOLD TRACKING
                if (!ctx->seekStarted)
                {
                    ctx->seekStarted = true;
                    ctx->seekInit = time(NULL);
                }
                
                SvString url = SvURIString(SvContentGetURI(ctx->content));
                if (strstr(SvStringCString(url), "can_ff=false"))
                {
                    return true;
                }
            }
            break;
        case QBKEY_PAUSE:
            if (ctx->content && SvContentGetURI(ctx->content))
            {
                SvString url = SvURIString(SvContentGetURI(ctx->content));
                if (strstr(SvStringCString(url), "can_pause=false"))
                {
                    return true;
                }
            }
            break;
        case QBKEY_PLAY:
        case QBKEY_PLAYPAUSE:
            // AMERELES BEGIN BOLD TRACKING
            if (ctx->seekStarted)
            {
                SvString vodId = BoldGetContentID(ctx);
                BoldContentTrackingSendMessageSeek(vodId, time(NULL) - ctx->seekInit, QBMovieUtilsGetAttr(ctx->vodMetaData, "source", SVSTRING("")));
                ctx->seekStarted = false;
            }
            // AMERELES END BOLD TRACKING
    }
    // END AMERELES Setear canSeek en en link para CatchUp

    if (QBAnyPlayerLogicHandleInputEvent(self->ctx->anyPlayerLogic, ev))
        return true;

    switch (ev->ch) {
        case QBKEY_STOP:
            //Return to previous view
            QBApplicationControllerPopContext(ctx->appGlobals->controller);
            break;
        case QBKEY_EPG: // AMERELES [4305] Comportamiento al presionar el botón “Guía” mientras se está reproduciendo un Catchup/Startover
            if (self->ctx->cutvEvent)
            {
                //Return to previous view
                QBApplicationControllerPopContext(ctx->appGlobals->controller);
            }
            else
            {
                return false;
            }
            break;
        default:
            return false;
    }

    return true;
}


/** This function creates JSON-like formated string which contains information about played content.
 *  Example of returned string:
 *          "url":"http://bs.vod.columbuscdn.com/vod/previews/TVNX0038601303011380/stb.ism/Manifest","productId":"3528","title":"Cars (HD)","playbackType":"vod"
 *          "url":"http://bs.npvr.columbuscdn.com/npvr2/5/content/hgtvhd/17119-27565604-201603011300/stb.ism/Manifest?vbegin=0&vend=2400&devplybck=1","channel":"HGTV HD","channelNumber":"118","epgID":"1648696","epgTitle":"Flip or Flop Follow-Up","epgTimeRange":"8:00-8:30","playbackType":"startOver"
 **/
SvString
QBPVRPlayerContextCreateEventData(QBWindowContext ctx_, const SvErrorInfo errorInfo)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    SvEPGEvent event = NULL;
    SvString channelID = NULL;
    SvString playbackTypeData = NULL;

    if (!ctx) {
        return NULL;
    }

    if (ctx->rec) {
        event = ctx->rec->event;
        channelID = ctx->rec->channelId;
        playbackTypeData = SvStringCreate(",\"playbackType\":\"pvrRecording\"", NULL);
    }

    SvString npvrRecStateInfoData = NULL;
    if (ctx->npvrRec) {
        event = ctx->npvrRec->event;
        channelID = ctx->npvrRec->channelId;
        playbackTypeData = SvStringCreate(",\"playbackType\":\"nPvrRecording\"", NULL);
        npvrRecStateInfoData = SvStringCreateWithFormat(",\"recordingState\":\"%s\"", ctx->npvrRec->state == QBnPVRRecordingState_completed ? "completed" : "active");
    } else if (ctx->playbackType == QBPVRPlayerContextPlaybackType_catchup || ctx->playbackType == QBPVRPlayerContextPlaybackType_startOver) {
        event = ctx->cutvEvent;
        channelID = SvValueIsString(ctx->cutvEvent->channelID) ? SvValueGetString(ctx->cutvEvent->channelID) : NULL;
        playbackTypeData = ctx->playbackType == QBPVRPlayerContextPlaybackType_startOver ? SvStringCreate(",\"playbackType\":\"startOver\"", NULL) : SvStringCreate(",\"playbackType\":\"catchup\"", NULL);
    }

    SvString channelData = NULL;
    if (channelID) {
        SvValue channelIDVal = SvValueCreateWithString(channelID, NULL);
        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        SvTVChannel channel = SvEPGManagerFindTVChannel(epgManager, channelIDVal, NULL);
        if (channel) {
            char *escapedChannelName = channel->name ? QBStringCreateJSONEscapedString(SvStringCString(channel->name)) : NULL;
            channelData = SvStringCreateWithFormat(",\"channel\":\"%s\",\"channelNumber\":\"%d\"",
                                                   escapedChannelName ? escapedChannelName : "",
                                                   channel->number);
            free(escapedChannelName);
        }
        SVRELEASE(channelIDVal);
    }

    SvString epgEventData = NULL;
    if (event) {
        struct tm beginTime, endTime;
        SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), true, &beginTime);
        SvTimeBreakDown(SvTimeConstruct(event->endTime, 0), true, &endTime);
        SvEPGEventDesc description = SvEPGEventGetAnyDescription(event);
        SvString eventID = SvInvokeVirtual(SvEPGEvent, event, createStringID);

        char *escapedEventId = eventID ? QBStringCreateJSONEscapedString(SvStringCString(eventID)) : NULL;
        char *escapedTitle = description && description->title ? QBStringCreateJSONEscapedString(SvStringCString(description->title)) : NULL;
        epgEventData = SvStringCreateWithFormat(",\"epgID\":\"%s\",\"epgTitle\":\"%s\",\"epgTimeRange\":\"%d:%02d-%d:%02d\"",
                                                escapedEventId ? escapedEventId : "",
                                                escapedTitle ? escapedTitle : "",
                                                beginTime.tm_hour, beginTime.tm_min, endTime.tm_hour, endTime.tm_min);
        free(escapedEventId);
        free(escapedTitle);
        SVRELEASE(eventID);
    }

    SvString urlData = NULL;
    if (ctx->content && SvContentGetURI(ctx->content)) {
        SvString url = SvURIString(SvContentGetURI(ctx->content));
        char *escapedUrl = url ? QBStringCreateJSONEscapedString(SvStringCString(url)) : NULL;
        urlData = SvStringCreateWithFormat(",\"url\":\"%s\"", escapedUrl ? escapedUrl : "");
        free(escapedUrl);
    }

    SvString vodData = NULL;
    if (ctx->vodMetaData) {
        vodData = QBProductUtilsCreateEventData(ctx->vodMetaData, NULL);
        playbackTypeData = SvStringCreate(",\"playbackType\":\"vod\"", NULL);
    }

    SvString errorCode = NULL;
    if (errorInfo) {
        errorCode = SvStringCreateWithFormat(",\"errorCode\":\"%d\"", SvErrorInfoGetCode(errorInfo));
//        playbackTypeData = SvStringCreate(",\"playbackType\":\"vod\"", NULL);
    }

    SvString eventData = SvStringCreateWithFormat("%s%s%s%s%s%s%s",
                                                  urlData ? SvStringCString(urlData) : "",
                                                  vodData ? SvStringCString(vodData) : "",
                                                  channelData ? SvStringCString(channelData) : "",
                                                  epgEventData ? SvStringCString(epgEventData) : "",
                                                  playbackTypeData ? SvStringCString(playbackTypeData) : "",
                                                  npvrRecStateInfoData ?  SvStringCString(npvrRecStateInfoData) : "",
                                                  errorCode ? SvStringCString(errorCode) : "");

    SVTESTRELEASE(npvrRecStateInfoData);
    SVTESTRELEASE(channelData);
    SVTESTRELEASE(epgEventData);
    SVTESTRELEASE(urlData);
    SVTESTRELEASE(errorCode);
    SVTESTRELEASE(vodData);
    SVTESTRELEASE(playbackTypeData);

    return eventData;
}

void QBPVRPlayerContextStartPlayback(QBWindowContext ctx_, double position)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    SvLogNotice("%s Start playback", __func__);

    struct SvPlayerTaskParams_s params;
    memset(&params, 0, sizeof(params));

    QBAdBitrateStart(ctx->adBitrate);

    ctx->wasOpenDoneEventReceived = false;
    ctx->canSeek = false;
    if (ctx->content)
        params.content = SVRETAIN(ctx->content);
    else if (ctx->rec && ctx->rec->playable)
        params.content = QBPVRProviderCreateContentForRecording(ctx->appGlobals->pvrProvider, ctx->rec);
    else if (ctx->musicPlaylist && SvArrayGetCount(ctx->musicPlaylist) > ctx->playlistPos)
        params.content = SVTESTRETAIN(SvArrayObjectAtIndex(ctx->musicPlaylist, ctx->playlistPos));

    params.standalone = true;
    params.delegate = (SvGenericObject) ctx;
    params.viewport = QBViewportGet();
    params.speed = 1.0;
    params.position = 0.0; // unused
    params.max_memory_use = QBVODLogicGetAllowedSizeOfBuffer();

    if (ctx->playerTask)
        QBPVRPlayerContextFinishPlayback(ctx_);

    if (params.content) {
        ctx->playerTask = SvPlayerManagerCreatePlayerTask(SvPlayerManagerGetInstance(), &params, NULL);
        if (ctx->playerTask) {
            SvPlayerTaskControllers taskControllers = SvPlayerTaskGetControllers(ctx->playerTask);
            if (!taskControllers) {
                SvLogError("%s Can't get task controllers", __func__);
            } else {
                SvPlayerTaskControllersAddListener(taskControllers, (SvObject) ctx, NULL);
            }
            SvPlayerTrackController trackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_subs);
            if (trackController) {
                SvPlayerTrackControllerAddListener(trackController, (SvObject) ctx, NULL);
            } else {
                SvLogError("%s Can't get track controller", __func__);
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
            SvPlayerTaskPlay(ctx->playerTask, 1.0, ctx->startPosition, NULL);
        } else {
            QBApplicationControllerPopContext(ctx->appGlobals->controller);
        }
    } else {
        QBApplicationControllerPopContext(ctx->appGlobals->controller);
    }

    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_fullScreen, NULL);

    SvValue productId = NULL;
    SvString title = NULL;
    if (ctx->playerTask) {
        if (ctx->externalAuthentication) {
            QBParentalControlHelperAuthenticated(ctx->pc);
        } else {
            QBParentalControlHelperReset(ctx->pc);
        }

        if (ctx->vodMetaData) {
            if (QBVoDUtilsIsAdult((SvObject) ctx->vodMetaData))
                QBParentalControlHelperSetVodAdult(ctx->pc, true);

            SvValue val = (SvValue) SvDBRawObjectGetAttrValue(ctx->vodMetaData, "ratings");
            if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val)) {
                QBParentalControlHelperSetVodRating(ctx->pc, SvValueGetString(val));
            }

            productId = SVTESTRETAIN(SvDBObjectGetID((SvDBObject) ctx->vodMetaData));
            SvValue titleVal = (SvValue) SvDBRawObjectGetAttrValue(ctx->vodMetaData, "name");
            if (titleVal && SvObjectIsInstanceOf((SvObject) titleVal, SvValue_getType())
                && SvValueGetType(titleVal) == SvValueType_string) {
                title = SVTESTRETAIN(SvValueGetString(titleVal));
            }
        }

        if (ctx->npvrRec) {
            QBParentalControlHelperSetRatingFromEvent(ctx->pc, ctx->npvrRec->event);
            if (!productId)
                productId = SvValueCreateWithString(ctx->npvrRec->id, NULL);
            SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(ctx->appGlobals->eventsLogic,
                                                               ctx->appGlobals->langPreferences,
                                                               ctx->npvrRec->event);
            if (desc && !title)
                title = SVTESTRETAIN(desc->title);
        }

        if (ctx->cutvEvent) {
            QBParentalControlHelperSetRatingFromEvent(ctx->pc, ctx->cutvEvent);
            SvValue eventId = SvEPGEventCreateID(ctx->cutvEvent);
            if (eventId && !productId) {
                if (SvValueIsString(eventId)) {
                    productId = SVRETAIN(eventId);
                } else if (SvValueIsInteger(eventId)) {
                    productId = SVRETAIN(eventId);
                }
            }
            SVTESTRELEASE(eventId);

            SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(ctx->appGlobals->eventsLogic,
                                                               ctx->appGlobals->langPreferences,
                                                               ctx->cutvEvent);
            if (desc && !title)
                title = SVTESTRETAIN(desc->title);
        }

        // title might have been set in the content (e.g. in case of music playlist)
        if (!productId && !title) {
            if (params.content) {
                if (SvContentMetaDataGetStringProperty(SvContentGetMetaData(params.content), SVSTRING(SV_PLAYER_META__TITLE), &title) >= 0)
                    SVRETAIN(title);
            }
        }

        if (ctx->appGlobals->playbackStateReporter && (productId || title)) {
            QBPlaybackStateReporterReportPlaybackStarted(ctx->appGlobals->playbackStateReporter,
                                                         productId, title, (SvObject) ctx);
            ctx->playbackStateReportsEnabled = true;
        }
        SVTESTRELEASE(productId);
        SVTESTRELEASE(title);

        QBParentalControlHelperSetEventSource(ctx->pc, ctx, QBPVRPlayerGetCurrentEvent);
        QBParentalControlHelperUpdateRating(ctx->pc);
        QBParentalControlLogicStartHelperMonitoring(ctx->appGlobals->parentalControlLogic, ctx->pc);
        QBPVRPlayerContextAuthenticationStateChanged(ctx);
    }

    if (ctx->rec && ctx->appGlobals->casPVRManager
        && SvObjectIsImplementationOf(ctx->appGlobals->casPVRManager, QBCASPVRPlaybackMonitor_getInterface())) {
        SvInvokeInterface(QBCASPVRPlaybackMonitor, ctx->appGlobals->casPVRManager, playbackStarted, ctx->rec);
    }

    SVTESTRELEASE(params.content);

    QBParentalControlHelperActivate(ctx->pc);

    if (ctx->playbackMonitor && ctx->playerTask) {
        SvString data = QBPVRPlayerContextCreateEventData(ctx_, NULL);
        QBPlaybackMonitorPlaybackStarted(ctx->playbackMonitor, ctx->playerTask, SVSTRING("PVRPlayer"), SVSTRING("PVRPlayer"), data, NULL);
        SVTESTRELEASE(data);
    }
}

void QBPVRPlayerContextFinishPlayback(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    int64_t duration = 0;

    if (ctx->playbackMonitor) {
        QBPlaybackMonitorPlaybackStoped(ctx->playbackMonitor, &duration, NULL);
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
        QBPVRPlayerContextSavePosition(ctx);

        QBAdBitrateStop(ctx->adBitrate);
        SVRELEASE(ctx->playerTask);
        ctx->playerTask = NULL;
        ctx->externalAuthentication = false;
        QBParentalControlHelperReset(ctx->pc);
        QBParentalControlHelperSetEventSource(ctx->pc, NULL, NULL);
    }

    if (ctx->rec && ctx->appGlobals->casPVRManager
        && SvObjectIsImplementationOf(ctx->appGlobals->casPVRManager, QBCASPVRPlaybackMonitor_getInterface())) {
        SvInvokeInterface(QBCASPVRPlaybackMonitor, ctx->appGlobals->casPVRManager, playbackStopped, ctx->rec);
    }

    QBParentalControlLogicStopHelperMonitoring(ctx->appGlobals->parentalControlLogic, ctx->pc);
    QBParentalControlHelperDeactivate(ctx->pc);
    if (ctx->waitingForAuthentication) {
        QBPVRPlayerContextFinishAuthentication(ctx);
    }
    QBPVRPlayerContextEnableAV(ctx);

    QBPVRPlayerContextUpdateFormat(ctx_);
}

const char *QBPVRPlayerContextGetTitle(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;

    if (ctx->title)
        return SvStringCString(ctx->title);
    else
        return gettext("No title");
}

// context private methods

SvLocal void QBPVRPlayerContextOnm3uFinishDownload(void *ctx_, QBm3uDownloader m3uDownloader, SvArray playlist)
{
    QBPVRPlayerContext ctx = ctx_;
    if (!playlist || !SvArrayCount(playlist))
        return;
    SvIterator it = SvArrayIterator(playlist);
    QBM3uItem item;

    ctx->musicPlaylist = SvArrayCreateWithCapacity(SvArrayCount(playlist), NULL);

    while ((item = (QBM3uItem) SvIteratorGetNext(&it))) {
        SvContent c = SvContentCreateFromCString(SvStringCString(item->track), NULL);
        SvString title = NULL;
        if (item->name) {
            title = SVRETAIN(item->name);
        } else {
            const char *lastSlash = strrchr(SvStringCString(item->track), '/');
            if (lastSlash && lastSlash[1] != '\0')
                title = SvStringCreate(lastSlash + 1, NULL);
            else
                title = SVRETAIN(item->track);
        }
        SvContentMetaDataSetStringProperty(SvContentGetMetaData(c), SVSTRING(SV_PLAYER_META__TITLE), title);

        SVRELEASE(title);

        SvArrayAddObject(ctx->musicPlaylist, (SvGenericObject) c);
        SVRELEASE(c);
    }
    if (ctx->super_.window) {
        QBAnyPlayerLogicStartPlayback(ctx->anyPlayerLogic, SVSTRING("PVR m3uDownloader"));
    }

    SVRELEASE(m3uDownloader);
    ctx->m3uDownloader = NULL;
}

SvLocal void QBPVRPlayerContextOnm3uDownloadError(void *ctx_, QBm3uDownloader m3uDownloader, int errorCode)
{
    QBPVRPlayerContext ctx = ctx_;
    SvErrorInfo errorInfo = SvErrorInfoCreate(SvCoreErrorDomain, errorCode, "Error while downloading playlist from a server.");
    SvString data = QBPVRPlayerContextCreateEventData((QBWindowContext) ctx, errorInfo);
    QBSecureLogEvent("PVRPlayer", "Error.PVRPlayer", "JSON:{\"description\":\"%s\"%s}",
                     SvErrorInfoGetMessage(errorInfo), data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    QBAnyPlayerLogicHandlePlayerError(ctx->anyPlayerLogic, errorInfo);
    QBPVRPlayerContextLogError(ctx, errorInfo); // AMERELES BOLD TRACKING
    SvErrorInfoDestroy(errorInfo);
}

SvLocal void
extendedInfoOnShow(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBPVRPlayer self = ptr;
    svSettingsPushComponent("TVExtendedInfo.settings");
    svSettingsWidgetAttach(frame, self->extendedInfo, "ExtendedInfo", 1);
    svSettingsPopComponent();
}

SvLocal void
extendedInfoSetActive(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBPVRPlayer self = ptr;
    svWidgetSetFocus(self->extendedInfo);
    QBExtendedInfoResetPosition(self->extendedInfo);
}
SvLocal void
extendedInfoOnHide(void *ptr, QBContainerPane pane, SvWidget frame)
{
    QBPVRPlayer self = ptr;
    svWidgetDetach(self->extendedInfo);
}

SvLocal void TVOnSideMenuClose(void *self_, QBContextMenu menu)
{
    QBPVRPlayer self = self_;
    self->sidemenu.shown = false;

    if (self->ctx->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerUnblock(self->ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID), SVSTRING(CAS_POPUP_BLOCKER_SIDE_MENU_WINDOW_ID));
    }

    QBPVRPlayerOSDLock((QBWindowContext) self->ctx, false, false);
}

// QBWindowContext virtual methods

SvLocal void
QBPVRPlayerContextReinitializeWindow(QBWindowContext self_, SvArray itemList)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    if (!self->super_.window)
        return;
    QBPVRPlayer info = self->super_.window->prv;
    if (!itemList || SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl"))) {
        QBExtendedInfoReinitialize(info->extendedInfo);
    }
}

SvLocal void
QBPVRPlayerContextCreateWindow(QBWindowContext ctx_, SvApplication app)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    QBPVRPlayer self;

    self = calloc(1, sizeof(struct QBPVRPlayer_t));
    self->ctx = ctx;

    self->window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    self->window->prv = self;
    svWidgetSetInputEventHandler(self->window, QBPVRPlayerInputEventHandler);
    svWidgetSetTimerEventHandler(self->window, QBPVRPlayerTimerEventHandler);

    ctx->super_.window = self->window;
    
    // AMERELES BOLD TRACKING
    ctx->initTimeStamp = time(NULL);
    ctx->actualPercentageDisplayed = QBPercentageDisplayed_NONE;
    ctx->keepAliveLastTimeSended = time(NULL) - BOLD_FOX_TRACKING_KEEPALIVE;
    ctx->keepAliveError = false;
    ctx->bufferUnderrunStarted = false;
    ctx->seekStarted = false;
    ctx->initTimeInitialCurrentPosition = DBL_MAX;

    self->OSD = QBAnyPlayerLogicOSDCreate(ctx->anyPlayerLogic, app, self);

    SvWidget osdWidget = QBOSDTakeWidget((QBOSD) self->OSD, NULL);
    svWidgetAttach(self->window, osdWidget, 0, 0, 10);

    SvObject contentObj = NULL;
    SvTVChannel channel = NULL; time_t now = 0; SvEPGEvent current = NULL; SvEPGEvent following = NULL;
    QBPVRPlayerGetCurrentEvent(ctx, channel, now, &current, &following);
    if (current)
        contentObj = (SvObject) current;
    else if (ctx->content)
        SvContentMetaDataGetObjectProperty(SvContentGetMetaData(ctx->content), SVSTRING("content"), &contentObj);

    SvInvokeInterface(TVOSD, self->OSD, setDataIntoEventFrame, (SvObject) contentObj, 0);
    SvInvokeInterface(TVOSD, self->OSD, setDataIntoEventFrame, (SvObject) contentObj, 1);

    QBAnyPlayerLogicShowOnFrontPanel(ctx->anyPlayerLogic);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings("TVContextMenu.settings", ctx->appGlobals->controller, app, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, TVOnSideMenuClose, self);
    self->sidemenu.content = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);

    self->contentSideMenu.ctx = NULL;
    self->contentSideMenu.shown = false;

    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
            .onShow = extendedInfoOnShow,
            .onHide = extendedInfoOnHide,
            .setActive = extendedInfoSetActive,
    };
    svSettingsPushComponent("TVExtendedInfo.settings");
    QBContainerPaneInit(self->sidemenu.content, ctx->appGlobals->res, self->sidemenu.ctx, 1, SVSTRING("ExtendedInfo"), &moreInfoCallbacks, self);
    SvWidget w = QBExtendedInfoNew(app, "ExtendedInfo", ctx->appGlobals);
    self->extendedInfo = w;
    svSettingsPopComponent();

    svSettingsPushComponent("pvrPlayer.settings");
    setupDialog(self);
    svSettingsPopComponent();

    ctx->subsManager = SVRETAIN((QBSubsManager)QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBSubsManager")));
    QBSubsManagerSetPreferenceFunction(ctx->subsManager, QBLangPreferencesSetPreferedSubtitlesTrack, ctx->appGlobals->langPreferences);
    QBLangPreferencesAddPreferredSubtitlesTrackListener(ctx->appGlobals->langPreferences, ctx->subsManager);
    SvInvokeInterface(TVOSD, self->OSD, setSubsManager, ctx->subsManager);

    self->langMenu = QBLangMenuNew(ctx->appGlobals, ctx->subsManager);
    QBLangMenuSetAudioCallback(self->langMenu, QBPVRPlayerSetAudioTrack, ctx);

    self->teletext = QBTeletextWindowNew(ctx->appGlobals, ctx->teletextReceiver);

    self->refreshTimer = svAppTimerStart(app, self->window, 0.02, 0);

    HDMIService hdmiService = (HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("HDMIService"));
    HDMIServiceStatusAddListener(hdmiService, (SvObject) ctx, NULL);
    QBPVRPlayerContextHDCPAuthStatusChanged((SvObject) ctx, HDMIServiceGetHDCPAuthStatus(hdmiService));

    //PostWidgetsCreate is called here beacause in StartPlayback we can want to show error dialog (e.g. failed UseProduct request)
    QBAnyPlayerLogicPostWidgetsCreate(ctx->anyPlayerLogic);
    if ((ctx->rec && ctx->rec->playable) || ctx->content || ctx->musicPlaylist || ctx->cutvEvent) {
       QBAnyPlayerLogicStartPlayback(ctx->anyPlayerLogic, SVSTRING("PVR"));
    } else if (ctx->m3uDownloader) {
        SvLogNotice("%s: waiting for m3uDownloader", __func__);
    } else {
        SvLogError("%s:%d : Content not set!", __FUNCTION__, __LINE__);
        QBApplicationControllerPopContext(ctx->appGlobals->controller);
    }

    QBPVRPlayerContextAttachSubtitle((QBWindowContext) ctx, self->window);

}

SvLocal void
QBPVRPlayerContextDestroyWindow(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    QBPVRPlayer self = (QBPVRPlayer) ctx->super_.window->prv;

    HDMIServiceStatusRemoveListener((HDMIService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("HDMIService")),
                                    (SvObject) ctx, NULL);

    QBAnyPlayerLogicPreWidgetsDestroy(ctx->anyPlayerLogic);

    SVTESTRELEASE(self->contentSideMenu.ctx);
    self->contentSideMenu.ctx = NULL;
    self->contentSideMenu.shown = false;

    svAppTimerStop(ctx->super_.window->app, self->refreshTimer);

    SVTESTRELEASE(self->langMenu);
    self->langMenu = NULL;
    SVTESTRELEASE(self->teletext);
    self->teletext = NULL;
    SVTESTRELEASE(self->sidemenu.content);
    self->sidemenu.content = NULL;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = NULL;

    SVRELEASE(self->OSD);

    QBPVRPlayerContextDetachSubtitle((QBWindowContext) ctx);

    if (self->parentalPopup) {
        QBDialogBreak(self->parentalPopup);
    }

    svWidgetDestroy(self->extendedInfo);
    self->extendedInfo = NULL;
    svWidgetDestroy(ctx->super_.window);
    ctx->super_.window = NULL;

    if (ctx->waitingForAuthentication){
        QBPVRPlayerContextMute(ctx, false);
    }
    ctx->waitingForAuthentication = false;
    //useBookmarks set to true for return to the last postion in movie e.g. user return back to VOD play from EPG by selecting EXIT
    ctx->useBookmarks = true;
    QBAnyPlayerLogicEndPlayback(ctx->anyPlayerLogic, SVSTRING("PVR")); // Logic causes some widgets updates we want to skip just before
                                                                       // destruction of window so we need to call logic after destroy(window)
    if (ctx->appGlobals->casPopupVisabilityManager) {
        QBCASPopupVisibilityManagerUnblockAll(ctx->appGlobals->casPopupVisabilityManager, SVSTRING(CAS_POPUP_BLOCKER_CONTEXT_ID));
    }

    SVTESTRELEASE(ctx->subsManager);
    ctx->subsManager = NULL;
}

// SvPlayerTaskControllersListener methods

SvLocal void QBPVRPlayerTaskControllersUpdated(SvObject self_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) self_;

    SvPlayerTaskControllers taskControllers =  SvPlayerTaskGetControllers(ctx->playerTask);
    if (!taskControllers) {
        return;
    }
    SvPlayerTrackController trackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_subs);
    if (trackController) {
        SvPlayerTrackControllerAddListener(trackController, (SvObject) ctx, NULL);
    }
}

// SvPlayerTaskHandler virtual methods

SvLocal void
QBPVRPlayerContextPlaybackStateChanged(SvObject self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    if (self->super_.window) {
        SvPlayerTaskState state = SvPlayerTaskGetState(self->playerTask);
        OSDChangedPlaySpeed((QBPVRPlayer) self->super_.window->prv, state.wantedSpeed);
    }
}

SvLocal void
QBPVRPlayerContextPlaybackFailed(SvObject self_, SvErrorInfo errorInfo)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    QBPVRPlayer player = self->super_.window->prv;
    if (player->dialog) {
        QBDialogBreak(player->dialog);
    }
    SvString data = QBPVRPlayerContextCreateEventData((QBWindowContext) self, errorInfo);
    QBSecureLogEvent("PVRPlayer", "Error.PVRPlayer.Playback", "JSON:{\"description\":\"%s\"%s}",
                     SvErrorInfoGetMessage(errorInfo), data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    QBPVRPlayerContextLogError(self, errorInfo); // AMERELES BOLD TRACKING
    QBAnyPlayerLogicHandlePlayerError(self->anyPlayerLogic, errorInfo);
    SvErrorInfoWriteLogMessage(errorInfo);
    SvErrorInfoDestroy(errorInfo);
}

SvLocal void
QBPVRPlayerContextGotPlaybackError(SvObject self_, SvErrorInfo errorInfo)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    QBPVRPlayer player = self->super_.window->prv;
    if (player->dialog) {
        QBDialogBreak(player->dialog);
    }
    SvString data = QBPVRPlayerContextCreateEventData((QBWindowContext) self, errorInfo);
    QBSecureLogEvent("PVRPlayer", "Error.PVRPlayer.Playback", "JSON:{\"description\":\"%s\"%s}",
                     SvErrorInfoGetMessage(errorInfo), data ? SvStringCString(data) : "");
    SVTESTRELEASE(data);
    QBPVRPlayerContextLogError(self, errorInfo); // AMERELES BOLD TRACKING
    QBAnyPlayerLogicHandlePlayerError(self->anyPlayerLogic, errorInfo);
    SvErrorInfoWriteLogMessage(errorInfo);
    SvErrorInfoDestroy(errorInfo);
}

SvLocal void
QBPVRPlayerContextPlaybackFinished(SvObject self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    if (self->musicPlaylist && SvArrayGetCount(self->musicPlaylist) > self->playlistPos + 1) {
        QBPVRPlayerNextContent((QBWindowContext) self);
        // assert - help for clang false positive
        assert((QBPVRPlayer) self->super_.window);
        OSDChangedPlaySpeed((QBPVRPlayer) self->super_.window->prv, 0.0);
        return;
    } else {
        self->playlistPos = 0;
    }

    self->isEOS = true;
    QBAnyPlayerLogicPlaybackFinished(self->anyPlayerLogic);
}

SvLocal void
QBPVRPlayerContextGotPlayerEvent(SvObject self_, SvString name, void *arg)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    if (self->playbackMonitor) {
        QBPlaybackMonitorPlayerEventHandler(self->playbackMonitor, name, arg, NULL);
    }
    if (self->appGlobals->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, self->appGlobals->casPopupManager, playbackEvent, name, arg);
    QBAnyPlayerLogicGotPlayerEvent(self->anyPlayerLogic, name, arg);

    if (!strcmp(PLAYER_EVENT_OPEN_DONE, SvStringCString(name))) {
        if (self->super_.window) {
            QBPVRPlayer player = self->super_.window->prv;
            if (player->dialog) {
                QBDialogBreak(player->dialog);
            }
        }
        if (self->playerTask) {
            // AMERELES Setear canSeek en en link para CatchUp
            bool canSeek = true;
            /*if (self->content && SvContentGetURI(self->content))
            {
                SvString url = SvURIString(SvContentGetURI(self->content));
                canSeek = !strstr(SvStringCString(url), "can_seek=false");
            }*/
            // AMERELES Setear canSeek en en link para CatchUp
            SvPlayerTaskCapabilities cap = SvPlayerTaskGetCapabilities(self->playerTask);
            SvLogNotice("%s canPause=%s canTrickplay=%s canSeek=%s canRecord=%s",
                        __func__,
                        cap.canPause ? "true" : "false",
                        cap.canTrickplay ? "true" : "false",
                        (cap.canSeek && canSeek) ? "true" : "false",
                        cap.canRecord ? "true" : "false");
            self->canSeek = cap.canSeek && canSeek;
            self->canPause = cap.canPause;
            self->wasOpenDoneEventReceived = true;
            QBAnyPlayerLogicToggleSpeedControl(self->anyPlayerLogic, self->canSeek);
            QBAnyPlayerLogicTogglePause(self->anyPlayerLogic, self->canPause);
        }
    } else if (!strcmp(PLAYER_EVENT_TRICK_ONGOING_REC_LIVEPOINT, SvStringCString(name))) {
        QBAnyPlayerLogicPlay(self->anyPlayerLogic, -1.0);
    } else if (!strcmp(PLAYER_EVENT_TRICK_ONGOING_REC_STARTPOINT, SvStringCString(name))) {
        QBPVRPlayerPause((QBWindowContext) self);
    } else if (!strcmp(PLAYER_EVENT_PREFILL, SvStringCString(name))) {
        if (!arg || !self->super_.window)
            return;

        // If buffer is prefilling, show dialog.
        QBPVRPlayer player = self->super_.window->prv;
        bool prefill = *((bool *) arg);
        if (prefill) {
            svSettingsPushComponent("pvrPlayer_loading.settings");
            setupPrefillDialog(player);
            svSettingsPopComponent();
        } else if (player->prefillDialog) {
            QBDialogBreak(player->prefillDialog);
        }
    } else {
        QBAdBitratePlayerEvent(self->adBitrate, name, self->playerTask, arg);
    }
}
SvLocal void
QBPVRPlayerContextSetOpt(SvObject self_, SvString optName, void *arg)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    if (self->playerTask) {
        SvPlayerTaskSetOpt(self->playerTask, SvStringCString(optName), arg);
    }
}

SvLocal void QBPVRPlayerContextReportPlaybackState(SvObject ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!ctx->playerTask || !ctx->playbackStateReportsEnabled)
        return;

    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    QBPlaybackStateReporterReportChange(ctx->appGlobals->playbackStateReporter, state.currentPosition, state.wantedSpeed);
}

// window context
SvLocal void QBPVROutputFormatChanged(void *self_, QBViewport qbv)
{
    QBPVRPlayerContextUpdateFormat((QBWindowContext) self_);
}

SvLocal void QBPVRPlayerContextVideoInfoChanged(void *self_, QBViewport qbv, const QBViewportVideoInfo *videoInfo)
{
    QBPVRPlayerContext ctx = self_;

    if (!ctx->dvbSubsManager || !videoInfo)
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
    QBDvbSubsManagerSetDDSMode(ctx->dvbSubsManager, mode);

    if (ctx->scte27SubsManager) {
        bool inNTSC = (ctx->dvbSubsManagerDDSMode == QBDvbSubsManagerDDSMode_NTSC);
        QBScte27SubsManagerEnableNTSCHack(ctx->scte27SubsManager, inNTSC);
    }
}

SvLocal void QBPVRPlayerContextVideoBlockingPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBPVRPlayerContext ctx = self_;
    ctx->expiredMessage = NULL;
}

SvLocal SvWidget QBPVRPlayerContextCreateBlockingDialog(QBPVRPlayerContext ctx, QBViewportBlockVideoReason state)
{
    char *title = NULL;
    char *label = NULL;

    switch (state) {
        case QBViewportBlockVideoReason_contentExpired:
            title = gettext("Content expired");
            label = gettext("Due to Broadcaster's restrictions you do not have access to this content");
            break;
        case QBViewportBlockVideoReason_contentExpirationPossible:
            title = gettext("Content blocked");
            label = gettext("Content has time restrictions but time is not set");
            break;
        default:
            SvLogError("%s : Unsupported state (%d)", __FUNCTION__, state);
            return NULL;
    }

    svSettingsPushComponent("ExpiredMessage.settings");
    QBDialogParameters params = {
        .app        = ctx->appGlobals->res,
        .controller = ctx->appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(ctx->super_.window)
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeNonFocusable);
    dialog->off_x = (ctx->super_.window->width - dialog->width) / 2;
    dialog->off_y = (ctx->super_.window->height - dialog->height) / 2;
    QBDialogSetTitle(dialog, title);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogAddLabel(dialog, SVSTRING("content"), label, SVSTRING("text"), 1);
    svSettingsPopComponent();

    return dialog;
}

SvLocal void QBPVRPlayerContextVideoBlockingChanged(void *self_, QBViewport qbv)
{
    QBPVRPlayerContext ctx = self_;

    if (ctx->super_.window == NULL) {
        return;
    }

    SvHashTable blockers = QBViewportBlockVideoGetBlockers(qbv);
    SvIterator it = SvHashTableGetValuesIterator(blockers);
    QBViewportBlockVideoReason state;
    SvValue blocker = NULL;

    while ((blocker = (SvValue) SvIteratorGetNext(&it))) {
        state = SvValueGetInteger(blocker);

        switch (state) {
            case QBViewportBlockVideoReason_contentExpired:
            case QBViewportBlockVideoReason_contentExpirationPossible:
                if (ctx->expiredMessage) {
                    if (ctx->videoBlockingReason == state) {
                        // we are showing a proper message
                        return;
                    } else {
                        // remove current dialog
                        QBDialogBreak(ctx->expiredMessage);
                    }
                }

                ctx->videoBlockingReason = state;
                ctx->expiredMessage = QBPVRPlayerContextCreateBlockingDialog(ctx, state);
                if (ctx->expiredMessage) {
                    QBDialogRun(ctx->expiredMessage, ctx, QBPVRPlayerContextVideoBlockingPopupCallback);
                }
                return;
            default:
                // do nothing
                break;
        }
    }

    if (ctx->expiredMessage) {
        QBDialogBreak(ctx->expiredMessage);
    }
}

static struct qb_viewport_callbacks viewportCallbacks = {
    .output_format_changed  = QBPVROutputFormatChanged,
    .video_info_changed     = QBPVRPlayerContextVideoInfoChanged,
    .video_blocking_changed = QBPVRPlayerContextVideoBlockingChanged
};

SvLocal void
QBPVRPlayerContextTracksUpdated(SvObject self_, SvPlayerTrackController controller)
{
    QBPVRPlayerContextUpdateFormat((QBWindowContext) self_);
}

SvLocal void
QBPVRPlayerContextCurrentTrackChanged(SvObject self_, SvPlayerTrackController controller, unsigned int idx)
{
}

SvLocal void QBPVRPlayerContextDestroy(void *self_)
{
    QBPVRPlayerContext ctx = self_;

    QBViewportRemoveCallbacks(QBViewportGet(), &viewportCallbacks, ctx);

    SVTESTRELEASE(ctx->subsManager);
    SVTESTRELEASE(ctx->rec);
    SVTESTRELEASE(ctx->content);
    SVTESTRELEASE(ctx->musicPlaylist);
    SVTESTRELEASE(ctx->m3uDownloader);
    SVTESTRELEASE(ctx->title);
    SVTESTRELEASE(ctx->cutvEvent);
    SVTESTRELEASE(ctx->npvrRec);
    SVTESTRELEASE(ctx->vodMetaData);
    SVTESTRELEASE(ctx->coverURI);
    SVTESTRELEASE(ctx->adBitrate);
    SVTESTRELEASE(ctx->contentData);
    SVRELEASE(ctx->accessDomain);
    QBAnyPlayerLogicSetContext(ctx->anyPlayerLogic, NULL);
    SVRELEASE(ctx->anyPlayerLogic);
    if (ctx->pc) {
       QBParentalControlLogicRemoveParentalControlHelper(ctx->appGlobals->parentalControlLogic, ctx->pc);
       SVRELEASE(ctx->pc);
    }
    if (ctx->expiredMessage) {
        QBDialogBreak(ctx->expiredMessage);
    }

    SVTESTRELEASE(ctx->playbackMonitor);
}

SvLocal void QBPVRPlayerContextPlay(SvObject self_);
SvLocal void QBPVRPlayerContextStop(SvObject self_);
SvLocal void QBPVRPlayerContextPause(SvObject self_);
SvLocal void QBPVRPlayerContextReplay(SvObject self_);
SvLocal void QBPVRPlayerContextSetContent_(SvObject self_, SvObject content_);
SvLocal SvObject QBPVRPlayerContextGetContent(SvObject self_);
SvLocal int QBPVRPlayerContextGetPlayerTaskState(SvObject self_, SvPlayerTaskState *state);
SvLocal SvObject QBPVRPlayerContextGetVodMetaData(SvObject self_);

SvType QBPVRPlayerContext_getType(void)
{
    static SvType type = NULL;
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_             = {
            .destroy        = QBPVRPlayerContextDestroy
        },
        .reinitializeWindow = QBPVRPlayerContextReinitializeWindow,
        .createWindow       = QBPVRPlayerContextCreateWindow,
        .destroyWindow      = QBPVRPlayerContextDestroyWindow
    };
    static const struct SvPlayerTaskControllersListener_ taskControllersListenerMethods = {
        .controllersUpdated = QBPVRPlayerTaskControllersUpdated,
    };
    static const struct SvPlayerTrackControllerListener_ trackControllerListenerMethods = {
        .tracksUpdated       = QBPVRPlayerContextTracksUpdated,
        .currentTrackChanged = QBPVRPlayerContextCurrentTrackChanged
    };
    static const struct SvPlayerTaskListener_ playerTaskListenerMethods = {
        .stateChanged         = QBPVRPlayerContextPlaybackStateChanged,
        .fatalErrorOccured    = QBPVRPlayerContextPlaybackFailed,
        .nonFatalErrorOccured = QBPVRPlayerContextGotPlaybackError,
        .playbackFinished     = QBPVRPlayerContextPlaybackFinished,
        .handleEvent          = QBPVRPlayerContextGotPlayerEvent
    };
    static const struct QBAnyPlayer_ anyPlayerMethods = {
        .play               = QBPVRPlayerContextPlay,
        .stop               = QBPVRPlayerContextStop,
        .pause              = QBPVRPlayerContextPause,
        .replay             = QBPVRPlayerContextReplay,
        .setContent         = QBPVRPlayerContextSetContent_,
        .getContent         = QBPVRPlayerContextGetContent,
        .getPlayerTaskState = QBPVRPlayerContextGetPlayerTaskState,
        .getVodMetaData     = QBPVRPlayerContextGetVodMetaData,
        .setOpt             = QBPVRPlayerContextSetOpt,
    };

    static const struct HDMIServiceStatusListener_ HDMIStatusMethods = {
        .HDCPStatusChanged    = QBPVRPlayerContextHDCPAuthStatusChanged,
        .hotplugStatusChanged = QBPVRPlayerContextHotplugStatusChanged
    };

    static const struct QBPlaybackStateReporterDataSource_ playbackReporterMethods = {
        .reportState = QBPVRPlayerContextReportPlaybackState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRPlayerContext",
                            sizeof(struct QBPVRPlayerContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            SvPlayerTaskControllersListener_getInterface(), &taskControllersListenerMethods,
                            SvPlayerTrackControllerListener_getInterface(), &trackControllerListenerMethods,
                            SvPlayerTaskListener_getInterface(), &playerTaskListenerMethods,
                            QBAnyPlayer_getInterface(), &anyPlayerMethods,
                            HDMIServiceStatusListener_getInterface(), &HDMIStatusMethods,
                            QBPlaybackStateReporterDataSource_getInterface(), &playbackReporterMethods,
                            NULL);
    }

    return type;
}

QBWindowContext QBPVRPlayerContextCreate(AppGlobals appGlobals, QBAnyPlayerLogic anyPlayerLogic, time_t loadTime)
{
    QBPVRPlayerContext self;

    self = (QBPVRPlayerContext) SvTypeAllocateInstance(QBPVRPlayerContext_getType(), NULL);
    self->appGlobals = appGlobals;
    self->pc = QBParentalControlHelperNew(appGlobals);
    self->anyPlayerLogic = SVRETAIN(anyPlayerLogic);
    self->adBitrate = QBAdBitrateCreate();
    self->useBookmarks = true;
    self->contentData = NULL;
    self->startPosition = 0.0;
    self->content = NULL;
    self->expiredMessage = NULL;
    self->accessDomain = SvStringCreate("PC", NULL);
    self->playbackType = QBPVRPlayerContextPlaybackType_default;
    
    // AMERELES BOLD TRACKING
    self->initTimePlay = loadTime;
    self->initTimePlayWasSent = false;

    self->dvbSubsManagerDDSMode = QBDvbSubsManagerDDSMode_default;

    SvTVContextDisableOverlay(appGlobals->newTV);
    QBParentalControlHelperSetCallback(self->pc, QBPVRPlayerContextAuthenticationStateChanged, self);
    QBParentalControlLogicAddParentalControlHelper(appGlobals->parentalControlLogic, self->pc);

    QBAnyPlayerLogicSetContext(anyPlayerLogic, (QBWindowContext) self);

    QBViewportAddCallbacks(QBViewportGet(), &viewportCallbacks, self);

    self->playbackMonitor = (QBPlaybackMonitor) SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                         SVSTRING("QBPlaybackMonitor")));

    return (QBWindowContext) self;
}

SvLocal void QBPVRPlayerContextDropAllContent(QBPVRPlayerContext self)
{
    if (self->rec && self->appGlobals->casPVRManager
        && SvObjectIsImplementationOf(self->appGlobals->casPVRManager, QBCASPVRPlaybackMonitor_getInterface())) {
        SvInvokeInterface(QBCASPVRPlaybackMonitor, self->appGlobals->casPVRManager, playbackStopped, self->rec);
    }

    SVTESTRELEASE(self->rec);
    self->rec = NULL;
    self->presentEventOSD = NULL;
    SVTESTRELEASE(self->content);
    self->content = NULL;
    SVTESTRELEASE(self->musicPlaylist);
    self->musicPlaylist = NULL;
    SVTESTRELEASE(self->m3uDownloader);
    self->m3uDownloader = NULL;
    self->playlistPos = 0;
    QBParentalControlHelperSetRecording(self->pc, NULL);
}

void QBPVRPlayerContextSetRecording(QBWindowContext self_, QBPVRRecording recording)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    QBPVRPlayerContextDropAllContent(self);
    self->rec = SVTESTRETAIN(recording);
    self->presentEventOSD = NULL;
    QBParentalControlHelperSetRecording(self->pc, recording);
}

void QBPVRPlayerContextSetContent(QBWindowContext self_, SvContent content)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    QBPVRPlayerContextDropAllContent(self);
    self->content = SVTESTRETAIN(content);
}

void QBPVRPlayerContextSetTitle(QBWindowContext self_, SvString title)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    SVTESTRELEASE(self->title);
    self->title = SVTESTRETAIN(title);
}

void QBPVRPlayerContextSetCoverURI(QBWindowContext self_, SvString coverURI)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    SVTESTRELEASE(self->coverURI);
    self->coverURI = SVTESTRETAIN(coverURI);
}

SvLocal void QBPVRPlayerAddLocalPlaylistGotItem(void *self_, QBM3uParser parser, QBM3uItem item)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    if (!item || !item->track)
        return;
    SvContent content = SvContentCreateFromCString(SvStringCString(item->track), NULL);
    if (content) {
        SvArrayAddObject(self->musicPlaylist, (SvGenericObject) content);
        SVRELEASE(content);
    }
}

SvLocal void QBPVRPlayerAddLocalPlaylist(QBWindowContext self_, SvString playlistURI)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    self->musicPlaylist = SvArrayCreate(NULL);
    if (!self->musicPlaylist)
        return;
    QBM3uParser parser = QBM3uParserCreateWithFile(playlistURI, 1024, (MAX_PLAYLIST_FILE_SIZE), &QBPVRPlayerAddLocalPlaylistGotItem, NULL, NULL, self_);
    if (!parser)
        return;
    QBM3uParserRun(parser);
    SVRELEASE(parser);
}

void QBPVRPlayerContextSetMusicPlaylist(QBWindowContext self_, SvString playlistURI)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    QBPVRPlayerContextDropAllContent(self);
    const char *uri = SvStringGetCString(playlistURI);
    if (uri && strstr(uri, "http") == uri)
        self->m3uDownloader = QBm3uDownloaderCreate(self->appGlobals->scheduler, playlistURI, QBPVRPlayerContextOnm3uFinishDownload, QBPVRPlayerContextOnm3uDownloadError, self);
    else
        QBPVRPlayerAddLocalPlaylist(self_, playlistURI);
}

void QBPVRPlayerContextSetMusic(QBWindowContext self_, SvContent content)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    QBPVRPlayerContextDropAllContent(self);
    self->musicPlaylist = SvArrayCreate(NULL);
    SvArrayAddObject(self->musicPlaylist, (SvGenericObject) content);
}

void QBPVRPlayerContextStopPlaying(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
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
        QBPVRPlayerContextSavePosition(ctx);
        SVRELEASE(ctx->playerTask);
        ctx->playerTask = NULL;
        QBParentalControlHelperReset(ctx->pc);
        QBParentalControlHelperSetEventSource(ctx->pc, NULL, NULL);
    }
    QBParentalControlLogicStopHelperMonitoring(ctx->appGlobals->parentalControlLogic, ctx->pc);
    QBParentalControlHelperDeactivate(ctx->pc);
    QBPVRPlayerContextFinishAuthentication(ctx);
}

/**
 * Get metadata that can be stored under different attribute names depending on the context (E.g. Traxis and Innov8on metadata are different).
 * NOTE: list of attribute names must be NULL terminated!
 * usage: get_from_meta(meta, "name", "title", NULL)
 */
/*
SvLocal SvGenericObject
get_from_meta(SvDBRawObject meta, const char *attrName1, ...)
{
    va_list ap;
    va_start(ap, attrName1);

    SvGenericObject val = NULL;
    const char *attrName = attrName1;
    while (!val && attrName) {
        val = SvDBRawObjectGetAttrValue(meta, attrName);
        attrName = va_arg(ap, const char*);
    }

    va_end(ap);

    return val;
}
*/

void QBPVRPlayerContextSetAccessDomain(QBWindowContext ctx_, SvString accessDomain)
{
    if (!ctx_ || !accessDomain) {
        SvLogWarning("%s: null passed (ctx: %p accessDomain: %p)", __func__, ctx_, accessDomain);
        return;
    }
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    SVRELEASE(ctx->accessDomain);
    ctx->accessDomain = SVRETAIN(accessDomain);
}

void QBPVRPlayerContextSetVodMetaData(QBWindowContext ctx_, SvDBRawObject meta)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    SVTESTRELEASE(ctx->vodMetaData);
    ctx->vodMetaData = SVTESTRETAIN(meta);
}

void QBPVRPlayerContextSetAuthenticated(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    ctx->externalAuthentication = true;
}

QBAnyPlayerLogic QBPVRPlayerContextGetAnyPlayerLogic(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    return ctx->anyPlayerLogic;
}

void QBPVRPlayerContextUpdateFormat(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;

    struct svdataformat* format = QBViewportGetOutputFormat(QBViewportGet());

    AudioTrackLogic audioTrackLogic =
        (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    AudioTrackLogicSetupAudioTrack(audioTrackLogic);
    if (ctx->super_.window) {
        QBPVRPlayer self = ctx->super_.window->prv;
        if (self->langMenu) {
            QBLangMenuUpdateAudioTracks(self->langMenu);
        }
    }

    if (ctx->dvbSubsManager) {
        QBDvbSubsManagerStop(ctx->dvbSubsManager);
        SVRELEASE(ctx->dvbSubsManager);
        ctx->dvbSubsManager = NULL;
    }

    if (ctx->textSubsManager) {
        QBTextSubsManagerStop(ctx->textSubsManager);
        SVRELEASE(ctx->textSubsManager);
        ctx->textSubsManager = NULL;
    }

    if (ctx->extSubsManager) {
        QBExtSubsManagerStop(ctx->extSubsManager);
        SVRELEASE(ctx->extSubsManager);
        ctx->extSubsManager = NULL;
    }

    if (ctx->teletextSubsManager) {
        SVRELEASE(ctx->teletextSubsManager);
        ctx->teletextSubsManager = NULL;
    }

    if (ctx->teletextReceiver) {
        QBTeletextReceiverStop(ctx->teletextReceiver);
        SVRELEASE(ctx->teletextReceiver);
        ctx->teletextReceiver = NULL;
    }

    if (ctx->teletextLineReceiver) {
        QBTeletextLineReceiverStop(ctx->teletextLineReceiver);
        SVRELEASE(ctx->teletextLineReceiver);
        ctx->teletextLineReceiver = NULL;
    }

    if (ctx->scte27SubsManager) {
        QBScte27SubsManagerStop(ctx->scte27SubsManager);
        SVRELEASE(ctx->scte27SubsManager);
        ctx->scte27SubsManager = NULL;
    }

    if (ctx->closedCaptionSubsManager) {
        QBClosedCaptionSubsManagerStop(ctx->closedCaptionSubsManager);
        SVRELEASE(ctx->closedCaptionSubsManager);
        ctx->closedCaptionSubsManager = NULL;
    }

    QBPVRPlayerTeletextSetup(ctx, format);

    if (ctx->playerTask && format) {
        QBSubsManagerSetPlayerTask(ctx->subsManager, ctx->playerTask);

        ctx->dvbSubsManager = QBDvbSubsManagerCreate(ctx->appGlobals->res, ctx->playerTask, format, ctx->subsManager);
        ctx->textSubsManager = QBTextSubsManagerCreate(ctx->appGlobals->res, ctx->playerTask, format, ctx->subsManager);
        ctx->extSubsManager = QBExtSubsManagerCreate(ctx->appGlobals->res, ctx->playerTask, format, ctx->subsManager, ctx->content, NULL);
        ctx->scte27SubsManager = QBScte27SubsManagerCreate(ctx->subsManager, ctx->playerTask, format);

        SvString domain = (ctx->rec == NULL ? SVSTRING("inVOD") : SVSTRING("inTV"));
        SvGenericObject domainStatus = QBLangPreferencesGetClosedCaptioningParam(ctx->appGlobals->langPreferences, domain);
        if (domainStatus && SvStringEqualToCString(SvValueGetString((SvValue) domainStatus), "enabled")) {
            SvString ccConfig = QBInitLogicGetClosedCaptioningOptionsFileName(ctx->appGlobals->initLogic);
            ctx->closedCaptionSubsManager = QBClosedCaptionSubsManagerCreate(ctx->appGlobals->res, ccConfig, ctx->playerTask, format, ctx->subsManager);
            SVTESTRELEASE(ccConfig);
        }

        QBViewportVideoInfo videoInfo;
        if (QBViewportGetVideoInfo(QBViewportGet(), &videoInfo) >= 0) {
            // use current mode
            QBPVRPlayerContextVideoInfoChanged(ctx, QBViewportGet(), &videoInfo);
        } else {
            // use last known mode
            if (ctx->scte27SubsManager) {
                QBDvbSubsManagerSetDDSMode(ctx->dvbSubsManager, ctx->dvbSubsManagerDDSMode);
                bool inNTSC = (ctx->dvbSubsManagerDDSMode == QBDvbSubsManagerDDSMode_NTSC);
                QBScte27SubsManagerEnableNTSCHack(ctx->scte27SubsManager, inNTSC);
            }
        }

        if (ctx->extSubsManager) {
            const char *extSubsEncoding = QBConfigGet("EXT_SUBS_ENCODING");
            if (extSubsEncoding) {
                SvString encodingStr = SvStringCreate(extSubsEncoding, NULL);
                QBExtSubsManagerSetEncoding(ctx->extSubsManager, encodingStr);
                SVRELEASE(encodingStr);
            }
        }
    }

    if (ctx->super_.window) {
        QBPVRPlayer self = ctx->super_.window->prv;
        QBOSD osd = QBOSDMainGetHandler(self->OSD, QBOSDHandlerType_recording, NULL);
        QBOSDRecordingUpdateEvents((QBOSDRecording) osd, NULL);
        // TODO : This method should be implemented in TS InfoIcon object
        SvInvokeInterface(TVOSD, self->OSD, update);
    }
}

void QBPVRPlayerContextUseBookmarks(QBWindowContext ctx_, bool status)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    ctx->useBookmarks = status;
}

SvLocal
void QBPVRPlayerPaneOnShowLanguages(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRPlayerContext self = self_;
    QBPVRPlayerContextHideContentSideMenu((QBWindowContext) self);
    QBPVRPlayerLangMenuShow((QBWindowContext) self, QBLangMenuSubMenu_Audio);
}

SvLocal void QBPVRPlayerSubtitlesEncodingPaneSelected(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRPlayerContext self = self_;
    QBPVRPlayer pvrPlayer = self->super_.window->prv;

    QBExtSubsManagerSetEncoding(self->extSubsManager, id);
    QBContextMenuHide(pvrPlayer->contentSideMenu.ctx, false);

    QBPVRPlayerContextDetachSubtitle((QBWindowContext) self);
    QBPVRPlayerContextAttachSubtitle((QBWindowContext) self, self->super_.window);
}

SvLocal
void QBPVRPlayerPaneOnShowSubtitlesEncodingPane(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBPVRPlayerContext self = self_;
    QBPVRPlayer pvrPlayer = self->super_.window->prv;

    QBBasicPane paneWithEncodings = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    svSettingsPushComponent("BasicPane.settings");
    QBBasicPaneInit(paneWithEncodings, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, pvrPlayer->contentSideMenu.ctx, QBBasicPaneGetLevel(pane) + 1, SVSTRING("BasicPane"));
    svSettingsPopComponent();
    QBBasicPaneLoadOptionsFromFile(paneWithEncodings, SVSTRING("PVRPlayerSubtitlesEncodingPane.json"));

    // default encoding
    QBBasicPaneAddOption(paneWithEncodings, SVSTRING("UTF-8"), NULL, QBPVRPlayerSubtitlesEncodingPaneSelected, self);
    // list of available encodings. If you want to add new encoding here, first check if iconv library supports such encoding.
    QBBasicPaneAddOption(paneWithEncodings, SVSTRING("WINDOWS-1250"), NULL, QBPVRPlayerSubtitlesEncodingPaneSelected, self);

    QBBasicPaneSetPosition(paneWithEncodings, QBExtSubsManagerGetEncoding(self->extSubsManager), true);
    QBContextMenuPushPane(pvrPlayer->contentSideMenu.ctx, (SvObject) paneWithEncodings);
    SVRELEASE(paneWithEncodings);
}

SvLocal
void QBPVRPlayerContextShowCustomContentSideMenu(QBPVRPlayer pvrPlayer, AppGlobals appGlobals)
{
    pvrPlayer->contentSideMenu.ctx = QBContextMenuCreateFromSettings("TVContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));

    svSettingsPushComponent("BasicPane.settings");
    QBBasicPane pane = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(pane, appGlobals->res, appGlobals->scheduler, appGlobals->textRenderer, pvrPlayer->contentSideMenu.ctx, 1, SVSTRING("BasicPane"));

    SvString lang = SvStringCreate(gettext("Languages"), NULL);
    QBBasicPaneAddOption(pane, SVSTRING("Languages"), lang, QBPVRPlayerPaneOnShowLanguages, pvrPlayer->ctx);
    SVRELEASE(lang);

    if (pvrPlayer->ctx->extSubsManager) {
        SvString subtitlesCoding = SvStringCreate(gettext("Subtitles encoding"), NULL);
        QBBasicPaneAddOption(pane, SVSTRING("Subtitles encoding"), subtitlesCoding, QBPVRPlayerPaneOnShowSubtitlesEncodingPane, pvrPlayer->ctx);
        SVRELEASE(subtitlesCoding);
    }

    QBContextMenuPushPane(pvrPlayer->contentSideMenu.ctx, (SvObject) pane);
    QBContextMenuShow(pvrPlayer->contentSideMenu.ctx);
    SVRELEASE(pane);
    svSettingsPopComponent();
}

SvLocal
void PVRPlayerContextContentMenuClose(void *self_, QBContextMenu menu)
{
    QBPVRPlayer pvrPlayer = self_;
    pvrPlayer->contentSideMenu.shown = false;
}

void QBPVRPlayerContextShowContentSideMenu(QBWindowContext self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    if (!self->super_.window)
        return;

    QBPVRPlayer pvrPlayer = self->super_.window->prv;

    if (pvrPlayer->contentSideMenu.shown) {
        // wait until content side menu will be completely closed
        return;
    }

    SVTESTRELEASE(pvrPlayer->contentSideMenu.ctx);

    if (SvObjectIsInstanceOf(self->contentData, QBContentInfo_getType())) {
        pvrPlayer->contentSideMenu.ctx = QBContentSideMenuCreate(self->appGlobals, SVSTRING("PVRPlayerMenu.json"), SVSTRING("PC_MENU"));
        // AMERELES #2131 tracking de navegación VOD en STBs híbridos
        QBContentSideMenuSetContentInfo((QBContentSideMenu) pvrPlayer->contentSideMenu.ctx, self->contentData, SVSTRING(""), SVSTRING(""));
        //QBContentSideMenuSetContentInfo((QBContentSideMenu) pvrPlayer->contentSideMenu.ctx, self->contentData);
        QBContextMenuShow(pvrPlayer->contentSideMenu.ctx);
        pvrPlayer->contentSideMenu.shown = true;
    } else {
        QBPVRPlayerContextShowCustomContentSideMenu(pvrPlayer, self->appGlobals);
        pvrPlayer->contentSideMenu.shown = true;
    }
    QBContextMenuSetCallbacks(pvrPlayer->contentSideMenu.ctx, PVRPlayerContextContentMenuClose, pvrPlayer);
}

void QBPVRPlayerContextHideContentSideMenu(QBWindowContext self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    if (!self->super_.window)
        return;

    QBPVRPlayer pvrPlayer = self->super_.window->prv;

    if (!pvrPlayer->contentSideMenu.ctx)
        return;

    QBContextMenuHide(pvrPlayer->contentSideMenu.ctx, false);
}

bool QBPVRPlayerContextIsContentSideMenuShown(QBWindowContext self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    if (!self->super_.window)
        return false;

    QBPVRPlayer pvrPlayer = self->super_.window->prv;
    if (pvrPlayer->contentSideMenu.ctx)
        return QBContextMenuIsShown(pvrPlayer->contentSideMenu.ctx);
    else
        return false;
}

void QBPVRPlayerContextSetContentData(QBWindowContext self_, SvObject contentData)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    SVTESTRELEASE(self->contentData);
    self->contentData = SVTESTRETAIN(contentData);
}

SvLocal void QBPVRPlayerContextPlay(SvObject self_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) self_;
    
    // AMERELES BOLD TRACKING
    if (ctx->keepAliveError)
    {
        return;
    }
    
    double position = -1.0;
    if (ctx->useBookmarks)
        position = QBPVRPlayerContextGetLastPositon((QBWindowContext) ctx);

    QBPVRPlayerContextSetStartPosition((QBWindowContext) ctx, position);
    QBPVRPlayerContextStartPlayback((QBWindowContext) ctx, position);
    
    // AMERELES BEGIN BOLD TRACKING
    if (!ctx->playerTask)
    {
        SvErrorInfo errorInfo = SvErrorInfoCreate(SvCoreErrorDomain, 0, "url error when playing content");
        QBPVRPlayerContextLogError(ctx, errorInfo);
        return;
    }
    
    //SvString vodId = BoldGetContentID(ctx);
    //if (vodId)
    //{
    //    SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
    //    if (position > 0)
    //    {
    //        BoldContentTrackingSendMessageCommand(vodId, SvStringCreate("resume", NULL), state.currentPosition, time(NULL) - ctx->initTimeStamp);
    //    }
    //    else
    //    {
    //        BoldContentTrackingSendMessageCommand(vodId, SvStringCreate("start", NULL), state.currentPosition, time(NULL) - ctx->initTimeStamp);
    //    }
    //}
    // AMERELES END BOLD TRACKING
}

SvLocal void QBPVRPlayerContextStop(SvObject self_)
{
    // AMERELES BEGIN BOLD TRACKING
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) self_;
    //if (!ctx->keepAliveError)
    {
        SvString vodId = BoldGetContentID(ctx);
        
        if (vodId && ctx->playerTask)
        {
            SvPlayerTaskState state = SvPlayerTaskGetState(ctx->playerTask);
            double actualPercentage = state.duration > 0 ? (state.currentPosition / state.duration) * 100 : 0;
            SvString finishStop = (actualPercentage > 99 ? SVSTRING("true") : SVSTRING("false"));
            
            double currentBitrate = 0;
            if (ctx->adBitrate)
            {
                currentBitrate = QBAdBitrateGetCurrentBitrate(ctx->adBitrate);
                currentBitrate = currentBitrate / 1000000;
            }
            
            BoldContentTrackingSendMessageStop(vodId, SvStringCreate("stop", NULL), state.currentPosition, QBMovieUtilsGetAttr(ctx->vodMetaData, "source", SVSTRING("")), time(NULL) - ctx->initTimeStamp, currentBitrate, finishStop);
        }
    }
    // AMERELES END BOLD TRACKING
    
    QBPVRPlayerContextFinishPlayback((QBWindowContext) self_);
}

SvLocal void QBPVRPlayerContextPause(SvObject self_)
{
    QBPVRPlayerPause((QBWindowContext) self_);
}

SvLocal void QBPVRPlayerContextReplay(SvObject self_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) self_;
    if (ctx->playerTask) {
        SvPlayerTaskPlay(ctx->playerTask, 1.0, 0.0, NULL);
    }
}

SvLocal void QBPVRPlayerContextSetContent_(SvObject self_, SvObject content)
{
    QBWindowContext self = (QBWindowContext) self_;
    QBPVRPlayerContextSetContent(self, (SvContent) content);
}

void
QBPVRPlayerContextSetMetaDataFromEvent(QBWindowContext ctx_, SvEPGEvent event, QBPVRPlayerContextPlaybackType playbackType)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!event) {
        SvLogError("%s : NULL argument passed.", __FUNCTION__);
        return;
    }

    SVTESTRELEASE(ctx->cutvEvent);
    ctx->cutvEvent = SVRETAIN(event);
    ctx->playbackType = playbackType;
}

SvGenericObject QBPVRPlayerContextGetAdBitrate(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    return (SvGenericObject) ctx->adBitrate;
}

SvLocal
SvObject QBPVRPlayerContextGetContent(SvObject self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    return (SvObject) self->content;
}

SvObject QBPVRPlayerContextGetRecording(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    return (SvObject) ctx->rec;
}

void QBPVRPlayerContextSetMetaDataFromNPvrRecording(QBWindowContext ctx_, QBnPVRRecording obj)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    if (!obj) {
        SvLogError("%s : NULL argument passed.", __FUNCTION__);
        return;
    }
    ctx->npvrRec = SVRETAIN(obj);
}

void QBPVRPlayerContextSetStartPosition(QBWindowContext ctx_, double pos)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    ctx->startPosition = pos;
}

// AMERELES BEGIN BOLD TRACKING
bool QBPVRPlayerCanPlay(QBWindowContext ctx_)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext)ctx_;
    if(!ctx || !ctx->super_.window)
        return true;
    
    return !ctx->keepAliveError;
}
// AMERELES END BOLD TRACKING

SvLocal
int QBPVRPlayerContextGetPlayerTaskState(SvObject self_, SvPlayerTaskState *state)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;

    if (!self->playerTask || !state)
        return -1;

    *state = SvPlayerTaskGetState(self->playerTask);
    return 0;
}

SvLocal SvObject QBPVRPlayerContextGetVodMetaData(SvObject self_)
{
    QBPVRPlayerContext self = (QBPVRPlayerContext) self_;
    return (SvObject) self->vodMetaData;
}

void QBPVRPlayerSetContentSourceType(QBWindowContext ctx_, QBParentalControlHelperContentSourceType type)
{
    QBPVRPlayerContext ctx = (QBPVRPlayerContext) ctx_;
    QBParentalControlHelperSetContentSourceType(ctx->pc, type);
}
