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

#include <CAGE/Core/Sv2DRect.h>
#include <CUIT/Core/types.h>
#include <QBSubsManager.h>
#include <Windows/tv/miniTVGuide.h>
#include <Windows/tv/teletextWindow.h>
#include <Widgets/miniFavoriteList.h>
#include <Windows/tv/miniRecordingList.h>
#include <Services/QBPVRConflictResolver.h>
#include <Services/QBParentalControl/QBParentalControlHelper.h>
#include <Services/core/QBPlaybackMonitor.h>
#include <Windows/tv/adWindow.h>
#include <Windows/tv/miniChannelList.h>
#include <ContextMenus/QBBasicPane.h>
#include <Menus/QBLangMenu.h>
#include <Windows/newtv.h>
#include <QBDSMCC/QBDSMCCDataProvider.h>
#include <QBDSMCC/QBDSMCCAdsRotationService.h>
#include <Windows/tv/QBRedButtonOverlayWindow.h>
#include <QBOSD/QBOSDMain.h>
#include <QBTunerMonitor.h>
#include <stdbool.h>
#include <Logic/TVLogic.h>  // AMERELES TimeShift cancel pop-up window

struct SvTVContextOverlayRequest_ {
    bool fullscreen;
    Sv2DRect rect;
};
typedef struct SvTVContextOverlayRequest_ SvTVContextOverlayRequest;

struct SvTVContext_t {
    struct QBWindowContext_t super_;
    SvTVChannel channel;
    SvTVChannel previous;
    SvString previousList;
    SvPlayerTask playerTask;
    struct SvTVContextPlayHandler_t* playerHandler;

    QBDvbSubsManagerDDSMode dvbSubsManagerDDSMode;

    QBTunerMonitor tunerMonitor;

    QBSubsManager subsManager;
    bool showSubtitles;
    SvWidget windowForSubtitles;

    SvContent currContent;

    SvTVContextOverlayRequest overlayRequest;
    bool activeOverlayRequest;

    unsigned int channelIdx;
    SvString listID;

    bool waitingForAuthentication;

    bool pauseLiveTVEnabled;

    QBPVRRecording currentRecording;

    QBParentalControlHelper pc;

    time_t timeshiftReferenceTime;  ///< absolute time of timeshift start minus current player position
    bool timeshiftFromRecStartTime; //determines if timeshift should be marked on OSD from the place it was started (true),
                                    //or from the place reported by underlying infrastructure (false).

    SvTVContextCurrentEventSourcesPriority currentEventSourcesPriority;
    struct {
        SvEPGEvent current, following;
    } events;

    bool flushScreenOnContextSwitch;              /**< If screen should be flushed during context switching */

    bool subtitlesAttached;

    SvFiber autoRestartFiber;
    SvFiberTimer autoRestartTimer;
    int autoRestartDelaySec;
    int channelErrorCode;
    SvTime connectionProblemStartTime;

    SvFiber connectionMonitorFiber;
    SvFiberTimer connectionMonitorTimer;

    QBDSMCCDataProvider redButtonDSMCCDataProvider;
    QBDSMCCAdsRotationService adsRotor;

    AppGlobals appGlobals;

    /// handle to QBPlaybackMonitor object
    QBPlaybackMonitor playbackMonitor;
};
typedef struct SvTVContext_t* SvTVContext;

typedef struct TVInfo_ {
    struct SvObject_ super_;
    QBOSDMain OSD;
    SvTVContext ctx;
    SvTimerId updateTimer;
    SvTimerId selectTimer;
    SvWidget parentalPopup;
    SvWidget signalPopup;
    SvWidget timeShiftConfirmPopup; // AMERELES Time-shift cancel warning pop-up
    SvWidget HDCPAuthFailedPopup;
    SvWidget tunerPopup;
    SvWidget fatalErrorPopup;
    SvWidget notEntitledPopup;
    SvWidget playbackProblemPopup;
    SvWidget bufferingPopup;
    SvWidget errorPopup;
    SvWidget recordingBlockedPopup;
    SvWidget extraErrorPopup;

    int selection;
    int selectionDigits;
    int selectionDigitsMax;
    bool isRecording;
    QBLangMenu langMenu;
    QBTeletextWindow teletext;
    QBMiniTVGuide miniTVGuide;
    QBMiniFavoriteList miniFavoriteList;
    QBMiniChannelList miniChannelList;
    QBMiniRecordingList miniRecordingList;
    QBAdWindow adWindow;
    QBRedButtonOverlayWindow redButtonOverlayWindow;

    SvWidget extendedInfo;
    QBPVRConflictResolver resolver;

    SvWidget background;
    
    // BEGIN RAL Time-shift cancel warning pop-up
    QBTVLogic objLogic; //RAL: be careful to include in destructor
    unsigned int event;
    int timeShiftChannelNumberDest;
    // END RAL Time-shift cancel warning pop-up
    
    // AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
    bool sendStartUsageLog;
    time_t sendStartUsageLogTimeStamp;
} TVInfo;

struct SvTVContextPlayHandler_t {
    struct SvObject_ super_;
    SvTVContext ctx;
};
typedef struct SvTVContextPlayHandler_t* SvTVContextPlayHandler;


#endif
