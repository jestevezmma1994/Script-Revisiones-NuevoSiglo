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

#ifndef NEWTV_INTERNAL_H
#define NEWTV_INTERNAL_H

#include <QBWindowContext.h>
#include <CUIT/Core/types.h>
#include <Logic/AnyPlayerLogic.h>
#include <Services/core/AdBitrate.h>
#include <Services/langPreferences.h>
#include <Services/BoldContentTracking.h>
#include <Services/QBParentalControl/QBParentalControlHelper.h>
#include <Services/core/QBPlaybackMonitor.h>
#include <Utils/m3uDownloader.h>
#include <QBTeletextLineReceiver.h>
#include <QBTeletextSubtitlesManager.h>
#include <QBDvbSubsManager.h>
#include <QBScte27Subs.h>
#include <QBClosedCaptionSubsManager.h>
#include <QBTextSubsManager.h>
#include <Menus/QBLangMenu.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContentSideMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <QBExtSubsManager.h>
#include <Windows/tv/teletextWindow.h>
#include <NPvr/QBnPVRRecording.h>
#include <QBPVRRecording.h>
#include <QBOSD/QBOSDMain.h>
#include <Logic/QBParentalControlLogic.h>
#include "pvrplayer.h"

struct QBPVRPlayerContext_t {
    struct QBWindowContext_t super_;

    AppGlobals appGlobals;

    SvPlayerTask playerTask;
    bool canSeek;
    bool canPause;
    bool wasOpenDoneEventReceived;

    QBAdBitrate adBitrate;

    QBPVRRecording rec;

    SvEPGEvent cutvEvent;
    QBPVRPlayerContextPlaybackType playbackType; ///< type of playback the player is playing (default, start over, catchup)
    QBnPVRRecording npvrRec;

    SvEPGEvent presentEventOSD;

    SvContent content;

    SvArray musicPlaylist;
    size_t playlistPos;
    bool useBookmarks;

    bool waitingForAuthentication;

    bool externalAuthentication;

    QBParentalControlHelper pc;
    SvString accessDomain;

    QBm3uDownloader m3uDownloader;

    QBAnyPlayerLogic anyPlayerLogic;
    SvString title, coverURI;
    SvDBRawObject vodMetaData;

    QBTeletextLineReceiver teletextLineReceiver;
    QBTeletextReceiver teletextReceiver;
    QBTeletextSubtitlesManager teletextSubsManager;
    QBDvbSubsManager dvbSubsManager;
    QBDvbSubsManagerDDSMode dvbSubsManagerDDSMode;
    QBTextSubsManager textSubsManager;
    QBExtSubsManager extSubsManager;
    QBSubsManager subsManager;
    QBScte27SubsManager scte27SubsManager;
    QBClosedCaptionSubsManager closedCaptionSubsManager;

    SvGenericObject contentData;
    SvWidget expiredMessage;
    QBViewportBlockVideoReason videoBlockingReason;

    double startPosition;
    bool isEOS;
    bool playbackStateReportsEnabled;             /**< If playback state changes should be reported by external service */

    /// handle to QBPlaybackMonitor object
    QBPlaybackMonitor playbackMonitor;
    
    time_t initTimeStamp; //AMERELES [#2406] Agregar información de Bitrate al reproducir VoD
    
    // AMERELES BEGIN BOLD TRACKING
    QBPercentageDisplayed actualPercentageDisplayed;
    time_t keepAliveLastTimeSended;
    bool keepAliveError;
    time_t initTimePlay;
    bool initTimePlayWasSent;
    double initTimeInitialCurrentPosition;
    
    time_t bufferUnderrunInit;
    bool bufferUnderrunStarted;
    time_t seekInit;
    bool seekStarted;
    // AMERELES END BOLD TRACKING
};
typedef struct QBPVRPlayerContext_t *QBPVRPlayerContext;

struct QBPVRPlayer_t {
    QBPVRPlayerContext ctx;
    SvWidget window;
    QBOSDMain OSD;
    SvTimerId refreshTimer;

    SvWidget extendedInfo;
    QBLangMenu langMenu;
    struct {
        QBContextMenu ctx;
        QBContainerPane content;
        bool shown;
    } sidemenu;

    SvWidget parentalPopup;
    SvWidget HDCPAuthFailedPopup;
    SvWidget dialog;
    SvWidget prefillDialog;

    QBTeletextWindow teletext;

    struct {
        QBContextMenu ctx; ///< handle to menu context
        bool shown;        ///< @c true if side menu is shown, @c false if side menu is completely closed
    } contentSideMenu;
};
typedef struct QBPVRPlayer_t *QBPVRPlayer;

typedef enum {
    QBPlayState_TV_MODE = 0,
    QBPlayState_PLAY_MODE,
    QBPlayState_PAUSE_MODE,
    QBPlayState_PAUSE_NOT_AVAILABLE_MODE,
    QBPlayState_FW_MODE,
    QBPlayState_FW_NOT_AVAILABLE_MODE,
    QBPlayState_RW_MODE,
    QBPlayState_Count
} QBPlayState;

#endif
