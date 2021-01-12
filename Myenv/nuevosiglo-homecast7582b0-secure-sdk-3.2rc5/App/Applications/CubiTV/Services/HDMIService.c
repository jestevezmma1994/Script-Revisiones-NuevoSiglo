/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <Services/HDMIService.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformEDID.h>
#include <QBPlatformHAL/QBPlatformHDMIOutput.h>
#include <QBPlatformHAL/QBPlatformImpl.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformUtil.h>
#include <QBPlatformHAL/QBPlatformEvent.h>
#include <QBPlatformHAL/QBPlatformImpl.h>
#include <QBConfig.h>
#include <QBViewport.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>
#include <QBSecureLogManager.h>
#include <Logic/TVLogic.h>
#include <Logic/AudioOutputLogic.h>
#include <Services/OutputStandardService.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvWeakList.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

struct HDMIService_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    bool outputPresent;
    unsigned int outputID;
    bool automaticModeEnabled;
    bool originalModeEnabled;
    bool automaticFrameRateSelectionEnabled;
    QBOutputStandard selectedMode;

    bool videoInfoPresent;
    QBViewportVideoInfo videoInfo;

    QBTVSystem tvSystem;

    /** HDMI port status, true if HDMI cable is connected */
    bool hdmiConnected;

    bool hdcpErrorFlag;     ///< flag which indicates if HDCP error occured or not
    SvTime hdcpErrorTime;   ///< time of occuring last HDCP error

    SvWeakList listeners;
    QBAsyncServiceState state;
};

SvLocal void HDMIServiceTVSystemConfigurationChanged(SvObject self_, const char *key, const char *value);

SvLocal inline QBOutputStandard
HDMIServiceGetNorDigSafeDefault(QBTVSystem tvSystem)
{
    // 'safe default', which is 1080i according to NorDig, use as fallback only
    if (tvSystem == QBTVSystem_NTSC)
        return QBOutputStandard_1080i59;
    else
        return QBOutputStandard_1080i50;
}

SvLocal void
HDMIServiceApplyVideoMode(HDMIService self, QBOutputStandard newMode)
{
    if (!self->outputPresent)
        return;
    QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);

    bool supportedOnPlatform = false;
    bool supportedOnTV = false;

    if (EDID) {
        supportedOnTV = QBPlatformEDIDIsVideoModeSupported(EDID, newMode);
    }

    if (QBPlatformOutputIsModeSupported(self->outputID, newMode, &supportedOnPlatform) == -1) {
        SvLogError("[%s]: Can't check if platform supports %s QBOutputStandard. Assuming NO.",
                   __func__, QBPlatformGetModeName(newMode));
        supportedOnPlatform = false;
    }

    if (self->hdmiConnected && (!EDID || EDID->videoFormatsCnt) && (!supportedOnTV || !supportedOnPlatform)) {
        SvLogError("[%s]: Platform has %ssupport, and TV has %ssupport of %s QBOutputStandard. Using fallback.",
                   __func__, supportedOnPlatform ? "" : "no ", supportedOnTV ? "" : "no ",
                   QBPlatformGetModeName(newMode));
        newMode = HDMIServiceGetNorDigSafeDefault(HDMIServiceGetTVSystem(self));
    }

    QBOutputStandardServiceChangeMode(self->appGlobals->outputStandardService, newMode, self->outputID);
    self->selectedMode = newMode;
    free(EDID);
}

SvLocal void
HDMIServiceDumpEDID(QBEDIDBlock *EDID)
{
    size_t len = EDID->videoFormatsCnt * 24;
    char buffer[len + 1];
    size_t i, l, used = 0;
    const char *modeName;

    for (i = 0; i < EDID->videoFormatsCnt; i++) {
        QBEDIDVideoFormat *fmt = &(EDID->videoFormats[i]);
        if (!(modeName = QBPlatformGetModeName(fmt->mode)))
            continue;
        if (i > 0 && len >= 2) {
            buffer[used++] = ',';
            buffer[used++] = ' ';
            len -= 2;
        }
        l = snprintf(buffer + used, len, "%s%s", modeName, fmt->isNative ? " (native)" : "");
        if (l <= len) {
            len -= l;
            used += l;
        }
    }
    buffer[used] = '\0';

    SvLogNotice("HDMIService: supported video modes: %s", buffer);
}

SvLocal QBOutputStandard
HDMIServiceSelectFirstMode(HDMIService self, QBEDIDBlock *EDID, const QBOutputStandard *modes)
{
    unsigned int i;

    for (i = 0; modes[i] != QBOutputStandard_none; i++) {
        if (QBPlatformEDIDIsVideoModeSupported(EDID, modes[i]))
            return modes[i];
    }

    return QBOutputStandard_none;
}

QBTVSystem
HDMIServiceGetTVSystem(HDMIService self)
{
    if (!self->automaticFrameRateSelectionEnabled || !self->videoInfoPresent) {
        return self->tvSystem;
    }

    // when automatic frame rate selection is enabled, we try to select
    // 50 Hz (PAL) or 59.94 Hz (NTSC) system depending on the currently
    // played content instead of the value of TVSYSTEM variable
    QBTVSystem tvSystem = QBPlatformGetTVSystemByVideoParams(self->videoInfo.height, self->videoInfo.frameRate);

    if (tvSystem == QBTVSystem_unknown)
        tvSystem = self->tvSystem;

    return tvSystem;
}

SvLocal QBOutputStandard
HDMIServiceSelectOutputMode(HDMIService self, QBOutputStandard hdMode, QBEDIDBlock *EDID)
{
    QBTVSystem tvSystem = HDMIServiceGetTVSystem(self);
    QBOutputStandard selectedMode = hdMode;

    if (hdMode == QBOutputStandard_none) {
        const char *hdConfig = QBConfigGet("VIDEOOUTHD");
        selectedMode = QBPlatformFindModeByName(hdConfig, tvSystem);
    }

    if (EDID) {
        // automatic replacement of modes with refresh rate not suitable for current TV system
        if (tvSystem == QBTVSystem_NTSC) {
            if (hdMode == QBOutputStandard_1080i50 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_1080i59))
                selectedMode = QBOutputStandard_1080i59;
            else if (hdMode == QBOutputStandard_720p50 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_720p59))
                selectedMode = QBOutputStandard_720p59;
            else if (hdMode == QBOutputStandard_1080p50 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_1080p59))
                selectedMode = QBOutputStandard_1080p59;
            else if (hdMode == QBOutputStandard_2160p25 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_2160p24))
                selectedMode = QBOutputStandard_2160p24;
            else if (hdMode == QBOutputStandard_2160p25 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_2160p30))
                selectedMode = QBOutputStandard_2160p30;
        } else {
            if (hdMode == QBOutputStandard_1080i59 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_1080i50))
                selectedMode = QBOutputStandard_1080i50;
            else if (hdMode == QBOutputStandard_720p59 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_720p50))
                selectedMode = QBOutputStandard_720p50;
            else if (hdMode == QBOutputStandard_1080p59 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_1080p50))
                selectedMode = QBOutputStandard_1080p50;
            else if (hdMode == QBOutputStandard_2160p24 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_2160p25))
                selectedMode = QBOutputStandard_2160p25;
            else if (hdMode == QBOutputStandard_2160p30 && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_2160p25))
                selectedMode = QBOutputStandard_2160p25;
        }
    }

    if (selectedMode == QBOutputStandard_none) {
        selectedMode = HDMIServiceGetNorDigSafeDefault(tvSystem);
    }

    if (selectedMode != hdMode && self->automaticFrameRateSelectionEnabled && self->videoInfoPresent) {
        SvLogNotice("HDMIService: using mode %s for %dx%d %d.%03d FPS content in automatic frame rate selection mode",
                    QBPlatformGetModeName(selectedMode),
                    self->videoInfo.width, self->videoInfo.height,
                    self->videoInfo.frameRate / 1000, self->videoInfo.frameRate % 1000);
    }

    return selectedMode;
}

SvLocal QBOutputStandard
HDMIServiceGetBestQualityMode(HDMIService self, QBEDIDBlock *EDID)
{
    QBOutputStandard palFallbackModes[] = {
        QBOutputStandard_1080i50, QBOutputStandard_720p50, QBOutputStandard_576p50,
        QBOutputStandard_1080i59, QBOutputStandard_720p59, QBOutputStandard_480p59,
        QBOutputStandard_none
    };
    QBOutputStandard ntscFallbackModes[] = {
        QBOutputStandard_1080i59, QBOutputStandard_720p59, QBOutputStandard_480p59,
        QBOutputStandard_1080i50, QBOutputStandard_720p50, QBOutputStandard_576p50,
        QBOutputStandard_none
    };
    QBTVSystem tvSystem = HDMIServiceGetTVSystem(self);
    if (tvSystem == QBTVSystem_PAL) {
        return HDMIServiceSelectFirstMode(self, EDID, palFallbackModes);
    } else {
        return HDMIServiceSelectFirstMode(self, EDID, ntscFallbackModes);
    }
}

SvLocal void
HDMIServiceSetAutomaticFormatMode(HDMIService self, QBEDIDBlock *EDID)
{
    QBOutputStandard newMode = QBOutputStandard_none;
    unsigned int i, nativeModesCnt = 0;
    QBTVSystem systemType = HDMIServiceGetTVSystem(self);

    if (!EDID) {
        newMode = HDMIServiceGetNorDigSafeDefault(systemType);
        goto setup;
    }

    // try to use one of native modes first
    for (i = 0; i < EDID->videoFormatsCnt; i++) {
        unsigned int width, linesCnt = 0;
        QBOutputStandard videoMode = EDID->videoFormats[i].mode;
        if (videoMode == QBOutputStandard_none || !EDID->videoFormats[i].isNative)
            continue;

        bool isSupported = false;
        if (QBPlatformOutputIsModeSupported(self->outputID, videoMode, &isSupported) < 0) {
            SvLogError("HDMIService: can't check that mode is supported");
            continue;
        }

        if (!isSupported) {
            SvLogWarning("HDMIService: Omitting native mode (%s) not supported by platform", QBPlatformGetModeName(videoMode));
            continue;
        }

        if (QBPlatformGetModeDimensions(videoMode, &width, &linesCnt) < 0)
            continue;

        // don't use any 1080p mode
        if (linesCnt == 1080 && (videoMode != QBOutputStandard_1080i50 && videoMode != QBOutputStandard_1080i59))
            continue;

        nativeModesCnt++;

        // automatic replacement of modes with refresh rate not suitable for local TV system
        if (videoMode == QBOutputStandard_1080i59) {
            if (systemType == QBTVSystem_PAL && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_1080i50))
                videoMode = QBOutputStandard_1080i50;
        } else if (videoMode == QBOutputStandard_1080i50) {
            if (systemType == QBTVSystem_NTSC && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_1080i59))
                videoMode = QBOutputStandard_1080i59;
        } else if (videoMode == QBOutputStandard_720p59) {
            if (systemType == QBTVSystem_PAL && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_720p50))
                videoMode = QBOutputStandard_720p50;
        } else if (videoMode == QBOutputStandard_720p50) {
            if (systemType == QBTVSystem_NTSC && QBPlatformEDIDIsVideoModeSupported(EDID, QBOutputStandard_720p59))
                videoMode = QBOutputStandard_720p59;
        }

        if (newMode == QBOutputStandard_none) {
            newMode = videoMode;
        } else {
            // use this native mode if better than previously found native mode
            unsigned int prevLinesCnt = 0;
            QBPlatformGetModeDimensions(newMode, &width, &prevLinesCnt);
            if (linesCnt > prevLinesCnt)
                newMode = videoMode;
        }
    }

    if (nativeModesCnt == 0) {
        SvLogNotice("HDMIService: no native video modes in EDID");
    } else if (newMode == QBOutputStandard_none) {
        SvLogNotice("HDMIService: native video mode(s) not supported");
    } else {
        if (self->automaticFrameRateSelectionEnabled && self->videoInfoPresent) {
            SvLogNotice("HDMIService: using native mode %s for %dx%d %d.%03d FPS content in automatic frame rate selection mode",
                        QBPlatformGetModeName(newMode),
                        self->videoInfo.width, self->videoInfo.height,
                        self->videoInfo.frameRate / 1000, self->videoInfo.frameRate % 1000);
        } else {
            SvLogNotice("HDMIService: using native mode %s", QBPlatformGetModeName(newMode));
        }
        QBConfigSet("VIDEOOUTHD", QBPlatformGetModeName(newMode));
        QBConfigSave();
        goto setup;
    }

    // select "best quality" fallback mode
    newMode = HDMIServiceGetBestQualityMode(self, EDID);

    if (newMode == QBOutputStandard_none) {
        newMode = HDMIServiceGetNorDigSafeDefault(systemType);
    }
    SvLogNotice("HDMIService: using fallback mode %s", QBPlatformGetModeName(newMode));

setup:
    HDMIServiceApplyVideoMode(self, newMode);
}

SvLocal void
HDMIServiceSetOriginalFormatMode(HDMIService self, QBEDIDBlock *EDID)
{
    QBOutputStandard newMode = QBOutputStandard_current;
    unsigned int lines = 0;

    if (!EDID) {
        // fallback to 'safe default', which is 1080i according to NorDig
        QBTVSystem tvSystem = HDMIServiceGetTVSystem(self);
        newMode = HDMIServiceGetNorDigSafeDefault(tvSystem);
        goto setup;
    }

    if (!self->videoInfoPresent) {
        // select "best quality" fallback mode
        newMode = HDMIServiceGetBestQualityMode(self, EDID);
        goto setup;
    }

    if (self->videoInfo.height <= 480 && self->tvSystem == QBTVSystem_NTSC) {
        lines = 480;
    } else if (self->videoInfo.height <= 576) {
        lines = 576;
    } else if (self->videoInfo.height == 720) {
        lines = 720;
    } else if (self->videoInfo.height == 1080 || self->videoInfo.height == 1088) {
        lines = 1080;
    }

    if (lines == 480) {
        // 480i59 (NTSC)
        QBOutputStandard modes[] = {
            QBOutputStandard_480p59, QBOutputStandard_720p59,
            QBOutputStandard_1080i59, QBOutputStandard_none
        };
        newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
    } else if (lines == 576) {
        // 576i50 (PAL)
        QBOutputStandard modes[] = {
            QBOutputStandard_576p50, QBOutputStandard_720p50,
            QBOutputStandard_1080i50, QBOutputStandard_none
        };
        newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
    } else if (lines == 720 && !self->videoInfo.interlaced) {
        if (self->videoInfo.frameRate >= 59940) {
            // 720p59
            QBOutputStandard modes[] = {
                QBOutputStandard_720p59, QBOutputStandard_1080i59,
                QBOutputStandard_480p59, QBOutputStandard_none
            };
            newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
        } else if (self->videoInfo.frameRate >= 25000) {
            // 720p50
            QBOutputStandard modes[] = {
                QBOutputStandard_720p50, QBOutputStandard_1080i50,
                QBOutputStandard_576p50, QBOutputStandard_none
            };
            newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
        }
    } else if (lines == 1080 && self->videoInfo.interlaced) {
        if (self->videoInfo.frameRate >= 59940) {
            // 1080i59
            QBOutputStandard modes[] = {
                QBOutputStandard_1080i59, QBOutputStandard_720p59,
                QBOutputStandard_480p59, QBOutputStandard_none
            };
            newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
        } else {
            // 1080i50
            QBOutputStandard modes[] = {
                QBOutputStandard_1080i50, QBOutputStandard_720p50,
                QBOutputStandard_576p50, QBOutputStandard_none
            };
            newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
        }
    } else if (lines == 1080 && !self->videoInfo.interlaced && self->videoInfo.frameRate >= 25000) {
        // 1080p25
        QBOutputStandard modes[] = {
            QBOutputStandard_1080p25, QBOutputStandard_1080i50,
            QBOutputStandard_720p50, QBOutputStandard_576p50, QBOutputStandard_none
        };
        newMode = HDMIServiceSelectFirstMode(self, EDID, modes);
    }

    if (newMode == QBOutputStandard_current || newMode == QBOutputStandard_none) {
        // fallback
        newMode = HDMIServiceGetBestQualityMode(self, EDID);
        if (newMode == QBOutputStandard_none) {
            QBTVSystem tvSystem = HDMIServiceGetTVSystem(self);
            newMode = HDMIServiceGetNorDigSafeDefault(tvSystem);
        }
        SvLogNotice("HDMIService: using fallback mode %s", QBPlatformGetModeName(newMode));
    }

setup:
    HDMIServiceApplyVideoMode(self, newMode);
}

SvLocal void
HDMIServiceHotplugStatusNotify(HDMIService self, bool isConnected)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(HDMIServiceStatusListener, listener, hotplugStatusChanged, isConnected);
    }
}

SvLocal void
HDMIServiceHDCPStatusNotify(HDMIService self, HDCPAuthStatus status)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvObject listener;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(HDMIServiceStatusListener, listener, HDCPStatusChanged, status);
    }
}

SvLocal void
HDMIServiceHotplugCallback(void *self_,
                           unsigned int outputID,
                           bool isConnected)
{
    HDMIService self = self_;
    QBEDIDBlock *EDID = NULL;
    self->hdmiConnected = isConnected;

    if (isConnected) {
        SvLogNotice("HDMIService: HDMI receiver connected");
        EDID = QBPlatformHDMIOutputGetEDID(outputID);
        if (EDID)
            HDMIServiceDumpEDID(EDID);
    } else {
        SvLogNotice("HDMIService: HDMI receiver disconnected");
        HDMIServiceHDCPStatusNotify(self, HDCPAuth_disabled);
    }

    HDMIServiceHotplugStatusNotify(self, isConnected);
    // change audio output preferences according to E-EDID info
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    AudioOutputLogicHDMIReceiverChanged((AudioOutputLogic) QBServiceRegistryGetService(registry, SVSTRING("AudioOutputLogic")),
                                        outputID, EDID);

    if (self->automaticModeEnabled) {
        HDMIServiceSetAutomaticFormatMode(self, EDID);
    } else if (self->originalModeEnabled) {
        HDMIServiceSetOriginalFormatMode(self, EDID);
    } else if (self->automaticFrameRateSelectionEnabled) {
        QBOutputStandard hdMode = HDMIServiceSelectOutputMode(self, QBOutputStandard_none, EDID);
        HDMIServiceApplyVideoMode(self, hdMode);
    } else if (isConnected) {
        QBTVSystem tvSystem = HDMIServiceGetTVSystem(self);
        const char *hdConfig = QBConfigGet("VIDEOOUTHD");
        HDMIServiceApplyVideoMode(self, QBPlatformFindModeByName(hdConfig, tvSystem));
    }

    free(EDID);
}

SvLocal void
HDMIServiceVideoFormatInfoUpdated(SvObject self_,
                                  const QBViewportVideoInfo *videoInfo)
{
}

SvLocal void
HDMIServiceVideoFormatChanged(SvObject self_,
                              const QBViewportVideoInfo *videoInfo)
{
    HDMIService self = (HDMIService) self_;

    self->videoInfo = *videoInfo;
    self->videoInfoPresent = true;

    if (!self->outputPresent) {
        return;
    }

    if (self->automaticModeEnabled) {
        QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);
        HDMIServiceSetAutomaticFormatMode(self, EDID);
        free(EDID);
    } else if (self->originalModeEnabled) {
        QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);
        HDMIServiceSetOriginalFormatMode(self, EDID);
        free(EDID);
    } else if (self->automaticFrameRateSelectionEnabled) {
        QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);
        QBOutputStandard hdMode = HDMIServiceSelectOutputMode(self, QBOutputStandard_none, EDID);
        HDMIServiceApplyVideoMode(self, hdMode);
        free(EDID);
    }
}

SvLocal void HDMIServicePlatformEventCallback(void *self_, QBPlatformEvent event, long long int argA, SvObject argB)
{
    if (event != QBPlatformEvent_HDCPStatusChanged && event != QBPlatformEvent_outputChanged) {
        return;
    }

    HDMIService self = (HDMIService) self_;
    unsigned outputID = (unsigned) argA;
    if (outputID != self->outputID) {
        return;
    }

    HDMIServiceHDCPStatusNotify(self, HDMIServiceGetHDCPAuthStatus(self));
}

SvLocal void
HDMIServiceDestroy(void *self_)
{
    HDMIService self = self_;
    if (self->outputPresent) {
        QBPlatformHDMIOutputSetHotplugCallback(self->outputID, NULL, NULL);
        QBPlatformRemoveEventCallback(HDMIServicePlatformEventCallback, self);
    }
    SVRELEASE(self->listeners);
}

SvLocal SvString
HDMIServiceGetName(SvObject self_)
{
    return SVSTRING("HDMIService");
}

SvLocal QBAsyncServiceState
HDMIServiceGetState(SvObject self_)
{
    HDMIService self = (HDMIService) self_;
    return self->state;
}

SvLocal SvImmutableArray
HDMIServiceGetDependencies(SvObject self_)
{
    return NULL;
}

void
HDMIServiceStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    HDMIService self = (HDMIService) self_;

    if (!self->outputPresent) {
        self->state = QBAsyncServiceState_running;
        return;
    }

    bool isConnected = false;
    if (QBPlatformHDMIOutputCheckConnection(self->outputID, &isConnected) == 0 && isConnected)
        HDMIServiceHotplugCallback(self, self->outputID, true);

    QBConfigAddListener((SvObject) self, "TVSYSTEM");

    HDMIServiceTVSystemChanged(self, QBPlatformGetTVSystemByName(QBConfigGet("TVSYSTEM")));
    self->state = QBAsyncServiceState_running;
}

void
HDMIServiceStop(SvObject self_, SvErrorInfo *errorOut)
{
    HDMIService self = (HDMIService) self_;
    QBConfigRemoveListener((SvObject) self, "TVSYSTEM");
    self->state = QBAsyncServiceState_idle;
}

SvLocal SvType
HDMIService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = HDMIServiceDestroy
    };
    static SvType t = NULL;

    static const struct QBAsyncService_ serviceMethods = {
        .getName         = HDMIServiceGetName,
        .getState        = HDMIServiceGetState,
        .getDependencies = HDMIServiceGetDependencies,
        .start           = HDMIServiceStart,
        .stop            = HDMIServiceStop,
    };

    static const struct QBVideoFormatListener_ methods = {
        .formatChanged = HDMIServiceVideoFormatChanged,
        .infoUpdated   = HDMIServiceVideoFormatInfoUpdated
    };

    static const struct QBConfigListener_t config_methods = {
        .changed = HDMIServiceTVSystemConfigurationChanged,
    };

    if (unlikely(!t)) {
        t = SvTypeCreateManaged("HDMIService",
                                sizeof(struct HDMIService_),
                                SvObject_getType(),
                                &t,
                                SvObject_getType(), &objectVTable,
                                QBVideoFormatListener_getInterface(), &methods,
                                QBConfigListener_getInterface(), &config_methods,
                                QBAsyncService_getInterface(), &serviceMethods,
                                NULL);
    }

    return t;
}

SvInterface
HDMIServiceStatusListener_getInterface(void)
{
    static SvInterface iface = NULL;
    if (unlikely(!iface)) {
        SvInterfaceCreateManaged("HDMIServiceHDCPStatusListener",
                                 sizeof(struct HDMIServiceStatusListener_),
                                 NULL, &iface, NULL);
    }
    return iface;
}

HDMIService HDMIServiceCreate(AppGlobals appGlobals)
{
    const char *v;

    HDMIService self = (HDMIService) SvTypeAllocateInstance(HDMIService_getType(), NULL);
    self->appGlobals = appGlobals;

    v = QBConfigGet("HDOUTORIGINALMODE");
    self->originalModeEnabled = (v && !strcmp(v, "enabled"));

    v = QBConfigGet("HDOUTAUTOMATICMODE");
    self->automaticModeEnabled = (v && !strcmp(v, "enabled"));

    v = QBConfigGet("AUTOMATICFRAMERATE");
    self->automaticFrameRateSelectionEnabled = (v && !strcmp(v, "enabled"));

    self->selectedMode = QBOutputStandard_none;

    self->tvSystem = QBPlatformGetTVSystemByName(QBConfigGet("TVSYSTEM"));

    self->listeners = SvWeakListCreate(NULL);

    self->videoInfoPresent = false;
    QBVideoFormatMonitorAddListener(appGlobals->videoFormatMonitor, (SvObject) self);

    int i = QBPlatformFindOutput(QBOutputType_HDMI, NULL, 0, true, false);
    if (i >= 0) {
        self->outputPresent = true;
        self->outputID = i;
        QBPlatformHDMIOutputSetHotplugCallback(i, HDMIServiceHotplugCallback, self);
        QBPlatformAddEventCallback(HDMIServicePlatformEventCallback, self);
    }

    self->hdcpErrorFlag = false;

    return self;
}

QBOutputStandard HDMIServiceGetSelectedVideoMode(HDMIService self)
{
    return self->selectedMode;
}

void HDMIServiceSetVideoMode(HDMIService self, QBOutputStandard newMode)
{
    if (!self->outputPresent)
        return;

    QBOutputStandard hdMode = newMode;

    if (self->automaticFrameRateSelectionEnabled) {
        // adjust frame rate
        QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);
        hdMode = HDMIServiceSelectOutputMode(self, newMode, EDID);
        free(EDID);
    }

    HDMIServiceApplyVideoMode(self, hdMode);
}

bool HDMIServiceIsOriginalModeActive(HDMIService self)
{
    return self->originalModeEnabled;
}

void HDMIServiceDisableOriginalMode(HDMIService self)
{
    self->originalModeEnabled = false;
}

void HDMIServiceEnableOriginalMode(HDMIService self)
{
    self->automaticModeEnabled = false;
    self->originalModeEnabled = true;
    if (self->outputPresent) {
        QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);
        HDMIServiceSetOriginalFormatMode(self, EDID);
        free(EDID);
    }
}

bool HDMIServiceIsAutomaticModeActive(HDMIService self)
{
    return self->automaticModeEnabled;
}

void HDMIServiceDisableAutomaticMode(HDMIService self)
{
    self->automaticModeEnabled = false;
}

void HDMIServiceEnableAutomaticMode(HDMIService self)
{
    self->originalModeEnabled = false;
    self->automaticModeEnabled = true;
    if (self->outputPresent) {
        QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);
        HDMIServiceSetAutomaticFormatMode(self, EDID);
        free(EDID);
    }
}

void HDMIServiceTVSystemChanged(HDMIService self, QBTVSystem tvSystem)
{
    if (!self || tvSystem == QBTVSystem_unknown) {
        SvLogError("%s: invalid arguments passed %p %d", __func__, self, tvSystem);
        return;
    }

    if (tvSystem == self->tvSystem)
        return;

    self->tvSystem = tvSystem;

    QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);

    if (self->automaticModeEnabled) {
        HDMIServiceSetAutomaticFormatMode(self, EDID);
    } else if (self->originalModeEnabled) {
        HDMIServiceSetOriginalFormatMode(self, EDID);
    } else {
        QBOutputStandard hdMode = HDMIServiceSelectOutputMode(self, QBOutputStandard_none, EDID);
        HDMIServiceApplyVideoMode(self, hdMode);
    }

    free(EDID);
}

void HDMIServiceDisableAutomaticFrameRateSelection(HDMIService self)
{
    self->automaticFrameRateSelectionEnabled = false;
}

void HDMIServiceEnableAutomaticFrameRateSelection(HDMIService self)
{
    self->automaticFrameRateSelectionEnabled = true;

    if (self->originalModeEnabled) {
        // original mode must have preference over automatic frame rate selection,
        // otherwise we would break NorDig compatibility
        return;
    }

    QBEDIDBlock *EDID = QBPlatformHDMIOutputGetEDID(self->outputID);

    if (self->automaticModeEnabled) {
        HDMIServiceSetAutomaticFormatMode(self, EDID);
    } else {
        QBOutputStandard hdMode = HDMIServiceSelectOutputMode(self, QBOutputStandard_none, EDID);
        HDMIServiceApplyVideoMode(self, hdMode);
    }

    free(EDID);
}

void HDMIServiceStatusAddListener(HDMIService self, SvObject listener, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "%s: NULL argument passed", __func__);
        goto err;
    }
    if (!listener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "%s: NULL listener passed", __func__);
        goto err;
    }
    if (!SvObjectIsImplementationOf(listener, HDMIServiceStatusListener_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain,
                                  SvCoreError_invalidArgument,
                                  "%s: interface HDMIServiceHDCPStatusListener is not implemented by object",
                                  __func__);
        goto err;
    }
    SvWeakListPushBack(self->listeners, listener, NULL);
err:
    SvErrorInfoPropagate(error, errorOut);
}

void HDMIServiceStatusRemoveListener(HDMIService self, SvObject listener, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "%s: NULL argument passed", __func__);
        goto err;
    }
    if (!listener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument, "%s: NULL listener passed", __func__);
        goto err;
    }
    SvWeakListRemoveObject(self->listeners, listener);
err:
    SvErrorInfoPropagate(error, errorOut);
}

HDCPAuthStatus HDMIServiceGetHDCPAuthStatus(HDMIService self)
{
    if (!self->outputPresent) {
        return HDCPAuth_disabled;
    }
    bool isConnected = false;
    if (QBPlatformHDMIOutputCheckConnection(self->outputID, &isConnected) == 0 && isConnected) {
        const HDCPAuthStatus status = QBPlatformGetHDCPStatus(self->outputID);
        if (status == HDCPAuth_error || status == HDCPAuth_disabled) {
            QBHDCPErrorCode errorCode = QBPlatformGetHDCPErrorCode(self->outputID);
            if (status == HDCPAuth_error || errorCode != QBHDCPErrorCode_none) {
                // Set flag and time of occuring of HDCP error
                self->hdcpErrorFlag = true;
                self->hdcpErrorTime = SvTimeGet();
                
                QBSecureLogEvent("HDMIService", "Error.HDMIService.HDCPAuthStatus",
                                 "JSON:{\"description\":\"HDCPErrorCode\",\"errorCode\":\"%d\"}", errorCode);
            }
        } else if (status == HDCPAuth_success) {
            // If HDCP was in error state and now it was changed to successful state -> send log
            if (self->hdcpErrorFlag) {
                self->hdcpErrorFlag = false;
                const SvTime hdcpErrorDuration = SvTimeSub(SvTimeGet(), self->hdcpErrorTime);

                QBSecureLogEvent("HDMIService", "Status.HDMIService.HDCPAuthStatus",
                                 "JSON:{\"description\":\"HDCP success\",\"HDCPErrorDuration\":\"%" PRIu64 "\"}",
                                 SvTimeToMilliseconds64(hdcpErrorDuration));
            }
        }
        return status;
    } else {
        return HDCPAuth_disabled;
    }
}

SvLocal void HDMIServiceTVSystemConfigurationChanged(SvObject self_, const char *key, const char *value)
{
    assert(self_);
    assert(key);
    assert(strcmp(key, "TVSYSTEM") == 0);

    HDMIService self = (HDMIService) self_;

    HDMIServiceTVSystemChanged(self, QBPlatformGetTVSystemByName(QBConfigGet(key)));
}
