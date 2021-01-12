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

#include <Logic/AnyPlayerLogic.h>

#include <QBSecureLogManager.h>
#include <libintl.h>
#include <settings.h>
#include <error_codes.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>
#include <Widgets/QBSimpleDialog.h>
#include <Widgets/QBPVROSD.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/pvrplayer.h>
#include <QBPlayerControllers/QBGenericAnyPlayerController.h>
#include <QBPlayerControllers/QBAnyPlayerController.h>
#include <QBPlayerControllers/QBPlaylistController.h>
#include <QBPlayerControllers/QBPlaybackStateController.h>
#include <QBPlayerControllers/QBVoDAnyPlayerController.h>
#include <QBPlayerControllers/QBTraxisAnyPlayerController.h>
#include <QBPlayerControllers/ConaxQBPlayerControllerPlugin.h>
#include <QBPlayerControllers/GenericQBAnyPlayerControllerPlugin.h>
#include <QBPlayerControllers/QBAnyPlayer.h>
#include <QBAppKit/QBServiceRegistry.h>

#include <TraxisWebClient/TraxisWebVoDProvider.h>
#include <QBContentManager/Innov8onProvider.h>
#include <QBOSD/QBOSD.h>
#include <QBOSD/QBOSDMain.h>
#include <QBOSD/QBOSDRecording.h>
#include <Widgets/OSD/QBPVROSDTSRecording.h>

#include <Services/core/GlobalPlayer.h>
#include <Services/QBCASManager.h>
#include <QBConf.h>
#include <QBPlatformHAL/QBPlatformFrontPanel.h>
#include <QBBookmarkService/QBBookmark.h>
#include <main.h>
#include <Logic/AudioTrackLogic.h>
#include <Logic/VideoOutputLogic.h>
#include <Logic/SubtitleTrackLogic.h>
#include <Logic/TVOSDInterface.h>
#include <SvPlayerKit/SvContentResources.h>
#include <QBJSONUtils.h>

struct QBAnyPlayerLogic_t {
    struct SvObject_ super_;
    QBWindowContext pvrPlayer;
    AppGlobals appGlobals;

    bool disableLocals;
    bool disableOSD;
    bool disableSpeedControl;
    bool disablePause;
    bool disableChannelChange;

    bool enableLoop;

    SvString vodId;
    SvString bookmarkId;

    QBBookmark lastPositionBokmark;
    double startPosition;

    SvObject controller;

    SvObject playbackStateController;

    SvWidget logicDialog;

    int errorCode;

    int hideDelay;

    bool displayLocked;
    bool showDialog;

    SvHashTable properties;

    SvString playReadyLicenceRequestParams;
    SvString playReadyLicenceRequestCustomData;
    SvSSLParams playReadyLicenceRequestSSLParams;
};

SvLocal SvObject QBAnyPlayerLogicCreateInnov8onController(QBAnyPlayerLogic self, SvObject product)
{
    SvObject controller = (SvObject) QBVoDAnyPlayerControllerCreate(self->appGlobals->scheduler, self->appGlobals->middlewareManager, self->appGlobals->qbMWProductUsageController, (SvObject) self);
    SvObject plugin = (SvObject) ConaxQBPlayerControllerPluginCreate(self->appGlobals->qbMWProductUsageController, NULL);
    if (plugin) {
        QBVoDAnyPlayerControllerAddPlugin((QBVoDAnyPlayerController) controller, plugin);
        SVRELEASE(plugin);
    }
    plugin = (SvObject) GenericQBAnyPlayerControllerPluginCreate();
    if (plugin) {
        QBVoDAnyPlayerControllerAddPlugin((QBVoDAnyPlayerController) controller, plugin);
        SVRELEASE(plugin);
    }

    if (product) {
        QBVoDAnyPlayerControllerSetProduct((QBVoDAnyPlayerController) controller, product);
    }
    return controller;
}

SvLocal SvObject QBAnyPlayerLogicCreatePlaylistController(QBAnyPlayerLogic self, QBContentInfo contentInfo)
{
    Innov8onProvider provider = (Innov8onProvider) QBContentInfoGetProvider(contentInfo);
    SvXMLRPCServerInfo serverInfo = Innov8onProviderGetServerInfo(provider, NULL);
    SvObject controller = (SvObject) QBPlaylistControllerCreate(self->appGlobals->scheduler,
                                                                self->appGlobals->middlewareManager,
                                                                self->appGlobals->qbMWProductUsageController,
                                                                (SvObject) self,
                                                                QBContentInfoGetServiceId(contentInfo),
                                                                (SvDBRawObject) QBContentInfoGetMovie(contentInfo),
                                                                serverInfo);
    SvObject conaxPlugin = (SvObject) ConaxQBPlayerControllerPluginCreate(self->appGlobals->qbMWProductUsageController, NULL);
    if (conaxPlugin) {
        QBPlaylistControllerAddPlugin((QBPlaylistController) controller, conaxPlugin);
        SVRELEASE(conaxPlugin);
    }
    return controller;
}

SvLocal SvObject QBAnyPlayerLogicCreateTraxisController(QBAnyPlayerLogic self, QBContentInfo contentInfo)
{
    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    QBTraxisAnyPlayerController controller = QBTraxisAnyPlayerControllerCreate(self->appGlobals->scheduler,
                                                                               self->appGlobals->traxisWebSessionManager,
                                                                               self->appGlobals->traxisWebPlaybackSessionManager,
                                                                               tunerReserver,
                                                                               self->appGlobals->natMonitor,
                                                                               (SvObject) self);
    if (contentInfo) {
        QBTraxisAnyPlayerControllerSetWebPlaybackSessionData(controller, (SvDBRawObject) QBContentInfoGetMovie(contentInfo));
    }
    return (SvObject) controller;
}

SvObject QBAnyPlayerLogicCreateController(QBAnyPlayerLogic self, QBContentInfo contentInfo)
{
    if (!self) {
        return NULL;
    }

    SvObject controller = NULL;
    SvObject provider = NULL;

    if (contentInfo && (provider = QBContentInfoGetProvider(contentInfo))) {
        if (SvObjectIsInstanceOf(provider, TraxisWebVoDProvider_getType())) {
            // provider type is Traxis -> setup traxis controller
            controller = QBAnyPlayerLogicCreateTraxisController(self, contentInfo);
        } else if (QBContentInfoMovieHasPlaylist(contentInfo) && SvObjectIsInstanceOf(provider, Innov8onProvider_getType())) {
            // movie has playlist -> setup playlist controller
            controller = QBAnyPlayerLogicCreatePlaylistController(self, contentInfo);
        }
    }

    if (!controller) {
        // use default controller -> QBVoDAnyPlayerController
        controller = QBAnyPlayerLogicCreateInnov8onController(self, QBContentInfoGetMovie(contentInfo));
    }

    return controller;
}

void QBAnyPlayerLogicSetupPlaybackStateController(QBAnyPlayerLogic self)
{
    if (self->appGlobals->remoteControl) {
        QBPlaybackStateController playbackStateController = QBPlaybackStateControllerCreate(self->appGlobals->scheduler,
                                                                                            self->appGlobals->middlewareManager);
        QBAnyPlayerLogicSetPlaybackStateController(self, (SvObject) playbackStateController);
        SVRELEASE(playbackStateController);
    }
}

SvLocal void QBAnyPlayerLogicControlGranted(SvObject self_, SvObject controller)
{
    QBAnyPlayerLogic self = (QBAnyPlayerLogic) self_;
    if (!self->pvrPlayer) {
        SvLogWarning("AnyLogic is requested to start playback, but it has no player");
        return;
    }

    AudioTrackLogic audioTrackLogic =
        (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    AudioTrackLogicSetPreferredAudioTrack(audioTrackLogic, NULL);

    SubtitleTrackLogic subtitleTrackLogic =
        (SubtitleTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SubtitleTrackLogic"));
    SubtitleTrackLogicSetPreferredLang(subtitleTrackLogic, NULL);
    SvInvokeInterface(QBAnyPlayerController, self->controller, stop);
    SvInvokeInterface(QBAnyPlayerController, self->controller, setPlayer, (SvObject) self->pvrPlayer);
    SvInvokeInterface(QBAnyPlayerController, self->controller, start);

    if (self->appGlobals->casPopupManager) {
        SvInvokeInterface(QBCASPopupManager, self->appGlobals->casPopupManager, contentChanged);
    }
}

SvLocal void QBAnyPlayerLogicControlRevoked(SvObject self_, SvObject controller)
{
    QBAnyPlayerLogic self = (QBAnyPlayerLogic) self_;
    if (!self->pvrPlayer) {
        SvLogWarning("AnyLogic is requested to end playback, but it has no player");
        return;
    }
    SvInvokeInterface(QBAnyPlayerController, self->controller, stop);
}

SvLocal bool QBAnyPlayerLogicAnyLocalWindowIsVisible(QBAnyPlayerLogic self);
SvLocal void QBAnyPlayerLogicHideAllLocalWindows(QBAnyPlayerLogic self);

SvLocal void QBAnyPlayerLogicInputWatcherTick(SvObject self_, QBInputWatcher inputWatcher)
{
    QBAnyPlayerLogic self = (QBAnyPlayerLogic) self_;
    if (QBAnyPlayerLogicAnyLocalWindowIsVisible(self))
        QBAnyPlayerLogicHideAllLocalWindows(self);
}

SvLocal void QBAnyPlayerLogicDestroy(void *self_)
{
    QBAnyPlayerLogic self = self_;
    SvInvokeInterface(QBAnyPlayerController, self->controller, finish);
    if (self->playbackStateController)
        SVRELEASE(self->playbackStateController);

    SVTESTRELEASE(self->vodId);
    SVTESTRELEASE(self->lastPositionBokmark);
    SVTESTRELEASE(self->bookmarkId);
    SVRELEASE(self->controller);
    SVRELEASE(self->properties);
    SVTESTRELEASE(self->playReadyLicenceRequestParams);
    SVTESTRELEASE(self->playReadyLicenceRequestCustomData);
    SVTESTRELEASE(self->playReadyLicenceRequestSSLParams);
}

SvLocal void QBAnyPlayerLogicOnControllerEvent(SvObject self_, int eventID, SvObject data);

SvLocal SvType QBAnyPlayerLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAnyPlayerLogicDestroy
    };
    static SvType type = NULL;
    static const struct QBAnyPlayerControllerListener_ controllerListenerMethods = {
        .onControllerEvent = QBAnyPlayerLogicOnControllerEvent,
    };

    static const struct QBGlobalPlayerControllerListener_t globalPlayerMethods = {
        .controlGranted = QBAnyPlayerLogicControlGranted,
        .controlRevoked = QBAnyPlayerLogicControlRevoked,
    };

    static const struct QBInputWatcherListener_t inputMethods = {
        .tick = QBAnyPlayerLogicInputWatcherTick
    };

    if (!type) {
        SvTypeCreateManaged("QBAnyPlayerLogic",
                            sizeof(struct QBAnyPlayerLogic_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBAnyPlayerControllerListener_getInterface(), &controllerListenerMethods,
                            QBInputWatcherListener_getInterface(), &inputMethods,
                            QBGlobalPlayerControllerListener_getInterface(), &globalPlayerMethods,
                            NULL);
    }
    return type;
}

QBAnyPlayerLogic QBAnyPlayerLogicNew(AppGlobals appGlobals)
{
    QBAnyPlayerLogic self = (QBAnyPlayerLogic) SvTypeAllocateInstance(QBAnyPlayerLogic_getType(), NULL);
    if (!self)
        return NULL;
    self->appGlobals = appGlobals;
    self->disableChannelChange = false;
    self->controller = (SvObject) QBGenericAnyPlayerControllerCreate(self->appGlobals->natMonitor);
    self->properties = SvHashTableCreate(11, NULL);
    self->errorCode = 0;
    self->startPosition = -1.0;

    const char *hideDelayStr = QBConfigGet("HIDEINFORMATION");
    if (!hideDelayStr)
        self->hideDelay = 5;
    else
        self->hideDelay = atoi(hideDelayStr);

    return self;
}

void QBAnyPlayerLogicToggleAll(QBAnyPlayerLogic self, bool enabled)
{
    self->disableLocals = self->disableOSD = self->disableSpeedControl = !enabled;
}

void QBAnyPlayerLogicToggleWidgets(QBAnyPlayerLogic self, bool enabled)
{
    self->disableLocals = !enabled;
    self->disableOSD = !enabled;
}

void QBAnyPlayerLogicToggleSpeedControl(QBAnyPlayerLogic self, bool enabled)
{
    self->disableSpeedControl = !enabled;
}

void QBAnyPlayerLogicTogglePause(QBAnyPlayerLogic self, bool enabled)
{
    self->disablePause = !enabled;
}

void QBAnyPlayerLogicToggleChannelChange(QBAnyPlayerLogic self, bool enabled)
{
    self->disableChannelChange = !enabled;
}

void QBAnyPlayerLogicToggleLoop(QBAnyPlayerLogic self, bool enabled)
{
    self->enableLoop = enabled;
}

bool QBAnyPlayerLogicIsLoopEnabled(QBAnyPlayerLogic self)
{
    return self->enableLoop;
}

bool QBAnyPlayerLogicHasPlaylist(QBAnyPlayerLogic self)
{
    return SvObjectIsInstanceOf(self->controller, QBPlaylistController_getType());
}

void QBAnyPlayerLogicSetContext(QBAnyPlayerLogic self, QBWindowContext pvrPlayer)
{
    self->pvrPlayer = pvrPlayer;    //do not retain, context owns us, and in its ::destroy() sets NULL here
}

SvLocal void QBAnyPlayerLogicHideAllLocalWindows(QBAnyPlayerLogic self)
{
    QBWindowContext ctx = self->pvrPlayer;

    QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);

    QBPVRPlayerExtendedInfoHide(ctx);
    QBPVRPlayerLangMenuHide(ctx);

    QBPVRPlayerOSDLock(ctx, false, false);
}


SvLocal bool QBAnyPlayerLogicAnyLocalWindowIsVisible(QBAnyPlayerLogic self)
{
    QBWindowContext ctx = self->pvrPlayer;
    if (QBPVRPlayerExtendedInfoIsVisible(ctx))
        return true;
    if (QBPVRPlayerLangMenuIsVisible(ctx))
        return true;
    return false;
}


void QBAnyPlayerLogicFFW(QBAnyPlayerLogic self)
{
    QBWindowContext ctx = self->pvrPlayer;
    if (self->disableSpeedControl)
        return;

    QBPVRPlayerFFW(ctx);
    if (self->playbackStateController)
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, changeSpeed, QBPVRPlayerGetSpeed(ctx));

    if (!self->disableOSD) {
        QBPVRPlayerOSDShow(ctx, false, false);
        QBPVRPlayerOSDLock(ctx, true, false);
    }
}


void QBAnyPlayerLogicREW(QBAnyPlayerLogic self)
{
    QBWindowContext ctx = self->pvrPlayer;
    if (self->disableSpeedControl)
        return;

    QBPVRPlayerREW(ctx);
    if (self->playbackStateController)
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, changeSpeed, QBPVRPlayerGetSpeed(ctx));

    if (!self->disableOSD) {
        QBPVRPlayerOSDShow(ctx, false, false);
        QBPVRPlayerOSDLock(ctx, true, false);
    }
}

void QBAnyPlayerLogicPause(QBAnyPlayerLogic self)
{
    QBWindowContext ctx = self->pvrPlayer;
    if (self->disablePause)
        return;

    QBPVRPlayerPause(ctx);
    if (self->playbackStateController)
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, changeSpeed, 0.0);

    if (!self->disableOSD) {
        QBPVRPlayerOSDShow(ctx, false, false);
        QBPVRPlayerOSDLock(ctx, QBPVRPlayerGetSpeed(ctx) != 1.0, false);
    }
}

void QBAnyPlayerLogicPlay(QBAnyPlayerLogic self, double position)
{
    QBWindowContext ctx = self->pvrPlayer;
    if (self->disablePause)
        return;

    QBPVRPlayerPlay(ctx, position);
    if (self->playbackStateController)
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, changeSpeed, 1.0);

    if (!self->disableOSD) {
        QBPVRPlayerOSDShow(ctx, false, false);
        QBPVRPlayerOSDLock(ctx, QBPVRPlayerGetSpeed(ctx) != 1.0, false);
    }
}

void QBAnyPlayerLogicVolumeUp(QBAnyPlayerLogic self)
{
    QBVolumeUp(self->appGlobals->volume);
}

void QBAnyPlayerLogicVolumeDown(QBAnyPlayerLogic self)
{
    QBVolumeDown(self->appGlobals->volume);
}

bool QBAnyPlayerLogicHandleInputEvent(QBAnyPlayerLogic self, SvInputEvent ev)
{
    if (SvInvokeInterface(QBAnyPlayerController, self->controller, handleInput, ev)) {
        return true;
    }

    QBWindowContext ctx = self->pvrPlayer;

    switch (ev->ch) {
        case QBKEY_UP:
        case QBKEY_CHUP:
            if (!self->disableChannelChange) {
                QBPVRPlayerNextContent(ctx);
                if (!self->disableOSD)
                    QBPVRPlayerOSDShow(ctx, false, false);
            }
            break;
        case QBKEY_DOWN:
        case QBKEY_CHDN:
            if (!self->disableChannelChange) {
                QBPVRPlayerPrevContent(ctx);
                if (!self->disableOSD)
                    QBPVRPlayerOSDShow(ctx, false, false);
            }
            break;
        case QBKEY_PLAYPAUSE:
            if (self->disablePause)
                break;

            double speed;
            if (QBPVRPlayerGetSpeed(ctx) == 1.0) {
                QBAnyPlayerLogicPause(self);
                speed = 0.0;
            } else {
                QBAnyPlayerLogicPlay(self, -1.0);
                speed = 1.0;
            }

            if (self->playbackStateController)
                SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, changeSpeed, speed);

            if (!self->disableOSD) {
                QBPVRPlayerOSDShow(ctx, false, false);
                QBPVRPlayerOSDLock(ctx, QBPVRPlayerGetSpeed(ctx) != 1.0, false);
            }
            break;
        case QBKEY_PLAY:
            QBAnyPlayerLogicPlay(self, -1.0);
            break;
        case QBKEY_PAUSE:
            QBAnyPlayerLogicPause(self);
            break;
        case QBKEY_REW:
            QBAnyPlayerLogicREW(self);
            break;
        case QBKEY_FFW:
            QBAnyPlayerLogicFFW(self);
            break;
        case QBKEY_TELETEXT:
            if (QBPVRPlayerTeletextIsAvailable(ctx)) {
                if (QBPVRPlayerTeletextIsVisible(ctx)) {
                    if (QBPVRPlayerTeletextIsTransparent(ctx)) {
                        QBPVRPlayerTeletextHide(ctx);
                    } else {
                        QBPVRPlayerTeletextSetTransparent(ctx, true);
                    }
                } else if (!self->displayLocked) {
                    QBAnyPlayerLogicHideAllLocalWindows(self);
                    QBPVRPlayerTeletextSetTransparent(ctx, false);
                    QBPVRPlayerTeletextShow(ctx);
                }
            }
            break;
        case QBKEY_INFO:
            if (self->disableLocals)
                break;

            if (QBPVRPlayerOSDIsVisible(ctx)) {
                if (QBPVRPlayerExtendedInfoIsVisible(ctx)) {
                    QBPVRPlayerExtendedInfoHide(ctx);
                    QBPVRPlayerOSDLock(ctx, false, false);
                    QBPVRPlayerOSDHide(ctx, true, false);
                } else if (!self->displayLocked) {
                    QBAnyPlayerLogicHideAllLocalWindows(self);
                    if (QBPVRPlayerExtendedInfoShow(ctx)) {
                        QBPVRPlayerOSDLock(ctx, true, false);
                    }
                }
            } else {
                QBPVRPlayerOSDLock(ctx, false, false);
                QBPVRPlayerOSDShow(ctx, false, true);
            }
            break;
        case QBKEY_AUDIOTRACK:
            if (self->disableLocals)
                break;

            if (QBPVRPlayerLangMenuIsVisible(ctx)) {
                QBPVRPlayerLangMenuHide(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else if (!self->displayLocked) {
                QBAnyPlayerLogicHideAllLocalWindows(self);
                QBPVRPlayerLangMenuShow(ctx, QBLangMenuSubMenu_Audio);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            break;
        case QBKEY_LANGUAGE:
            if (self->disableLocals)
                break;

            if (QBPVRPlayerLangMenuIsVisible(ctx)) {
                QBPVRPlayerLangMenuHide(ctx);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else if (!self->displayLocked) {
                QBAnyPlayerLogicHideAllLocalWindows(self);
                QBPVRPlayerLangMenuShow(ctx, QBLangMenuSubMenu_Subtitles);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            break;

        case QBKEY_BACK:
            if (QBAnyPlayerLogicAnyLocalWindowIsVisible(self)) {
                QBAnyPlayerLogicHideAllLocalWindows(self);
                break;
            } else {
                if (QBPVRPlayerOSDIsVisible(ctx)) {
                    QBPVRPlayerOSDHide(ctx, true, false);
                    // OSD will become hidden only if content is being played at standard speed.
                    // If that's not the case, BACK button should close playback.
                    return (!QBPVRPlayerOSDIsVisible(ctx));
                }

                return false;
            }
        case QBKEY_LEFT:
            //HACK
            if (QBAnyPlayerLogicAnyLocalWindowIsVisible(self))
                svAppPostEvent(self->appGlobals->res, NULL, svInputEventCreate(QBKEY_PGUP));
            else
                QBVolumeDown(self->appGlobals->volume);
            break;

        case QBKEY_RIGHT:
            //HACK
            if (QBAnyPlayerLogicAnyLocalWindowIsVisible(self))
                svAppPostEvent(self->appGlobals->res, NULL, svInputEventCreate(QBKEY_PGDN));
            else
                QBVolumeUp(self->appGlobals->volume);
            break;

        case QBKEY_FUNCTION:
            if (QBPVRPlayerContextIsContentSideMenuShown(self->pvrPlayer)) {
                QBPVRPlayerContextHideContentSideMenu(self->pvrPlayer);
                QBInputWatcherRemoveListener(self->appGlobals->inputWatcher, (SvObject) self);
            } else {
                QBAnyPlayerLogicHideAllLocalWindows(self);
                if (!QBPVRPlayerContextIsContentSideMenuShown(self->pvrPlayer))
                    QBPVRPlayerContextShowContentSideMenu(self->pvrPlayer);
                QBInputWatcherAddListener(self->appGlobals->inputWatcher, (SvObject) self, self->hideDelay);
            }
            break;

        case QBKEY_ASPECT:
        {
            VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoOutputLogic"));
            VideoOutputLogicToggleVideoContentMode(videoOutputLogic);
            break;
        }
        default:
            return false;
    }

    return true;
}

void QBAnyPlayerLogicStartPlayback(QBAnyPlayerLogic self, SvString source)
{
    if (!self->pvrPlayer) {
        SvLogWarning("%s(): AnyLogic is requested to start playback, but it has no player", __func__);
        return;
    }

    SvContent content = (SvContent) SvInvokeInterface(QBAnyPlayer, self->pvrPlayer, getContent);
    if (content) {
        SvContentMetaData resources = SvContentGetResources(content);
        if (self->playReadyLicenceRequestParams)
            SvContentMetaDataSetObjectProperty(resources, SVSTRING(SV_CONTENT_RESOURCE__PR_LICENCE_REQUEST_PARAMS),
                                               (SvObject) self->playReadyLicenceRequestParams);
        if (self->playReadyLicenceRequestCustomData)
            SvContentMetaDataSetObjectProperty(resources, SVSTRING(SV_CONTENT_RESOURCE__PR_LICENCE_REQUEST_CUSTOM_DATA),
                                               (SvObject) self->playReadyLicenceRequestCustomData);
        if (self->playReadyLicenceRequestSSLParams)
            SvContentMetaDataSetObjectProperty(resources, SVSTRING(SV_CONTENT_RESOURCE__PR_LICENCE_REQUEST_SSL_PARAMS),
                                               (SvObject) self->playReadyLicenceRequestSSLParams);
    }

    QBGlobalPlayerControllerPushController(self->appGlobals->playerController, (SvObject) self);
}

void QBAnyPlayerLogicEndPlayback(QBAnyPlayerLogic self, SvString source)
{
    QBGlobalPlayerControllerRemoveController(self->appGlobals->playerController, (SvObject) self);
}

void QBAnyPlayerLogicReportState(QBAnyPlayerLogic self)
{
    if (self->playbackStateController) {
        QBPlaybackStateControllerReportState(self->playbackStateController);
    }
}

void QBAnyPlayerLogicGetSpeedBoundary(QBAnyPlayerLogic self, float *backwardMin, float *backwardMax, float *forwardMin, float *forwardMax, bool *loop)
{
    if (backwardMax)
        *backwardMax = -8;
    if (forwardMin)
        *forwardMin = 8;
    if (backwardMin)
        *backwardMin = -128;
    if (forwardMax)
        *forwardMax = 128;
    if (loop)
        *loop = false;
}

SvLocal bool QBAnyPlayerLogicPlaybackTryToPlayNextCatchup(QBAnyPlayerLogic self)
{
    SvContent content = (SvContent) SvInvokeInterface(QBAnyPlayer, self->pvrPlayer, getContent);
    if (!content) {
        return false;
    }
    SvContentMetaData metadata = SvContentGetMetaData(content);
    SvObject obj = NULL;
    SvContentMetaDataGetObjectProperty(metadata, SVSTRING("continuousCUTV_event"), &obj);
    if (!obj) {
        return false;
    }
    SvEPGEvent event = (SvEPGEvent) obj;

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    if (!epgManager) {
        SvLogError("no EPG manager");
        return false;
    }

    const time_t now = SvTimeNow();
    SvEPGIterator it = SvEPGManagerCreateIterator(epgManager, event->channelID, event->startTime);
    SvEPGIteratorGetNextEvent(it);
    SvEPGEvent next = SvEPGIteratorGetNextEvent(it);
    //check if played catchup event is followed by another catchup event. Play it if so.
    if (!next || !CutvManagerIsEventAvailable(self->appGlobals->cutvManager, next, now - event->endTime + event->startTime)) {
        SVRELEASE(it);
        return false;
    }
    content = CutvManagerCreateContent(self->appGlobals->cutvManager, next);
    metadata = SvContentGetMetaData(content);
    SvContentMetaDataSetObjectProperty(metadata, SVSTRING("continuousCUTV_event"), (SvObject) next);

    SVRELEASE(it);

    SvInvokeInterface(QBAnyPlayerController, self->controller, stop);
    SvInvokeInterface(QBAnyPlayerController, self->controller, setContent, (SvObject) content);
    QBPVRPlayerContextSetMetaDataFromEvent(self->pvrPlayer, next, QBPVRPlayerContextPlaybackType_catchup);
    SvInvokeInterface(QBAnyPlayerController, self->controller, start);

    QBPVRPlayerOSDShow(self->pvrPlayer, true, true);
    QBPVRPlayerOSDLock(self->pvrPlayer, false, false);

    if (self->appGlobals->casPopupManager) {
        SvInvokeInterface(QBCASPopupManager, self->appGlobals->casPopupManager, contentChanged);
    }

    SvLogNotice("set new content");
    return true;
}

void QBAnyPlayerLogicPlaybackFinished(QBAnyPlayerLogic self)
{
    if (QBAnyPlayerLogicPlaybackTryToPlayNextCatchup(self)) {
        return;
    }

    SvLogNotice("Playback finished speed=%f", QBPVRPlayerGetSpeed(self->pvrPlayer));
    if (!QBAnyPlayerLogicHasPlaylist(self) && QBPVRPlayerGetSpeed(self->pvrPlayer) == 1.0 && !QBAnyPlayerLogicIsLoopEnabled(self)) {
        QBApplicationControllerPopContext(self->appGlobals->controller);
    }
    SvInvokeInterface(QBAnyPlayerController, self->controller, finish);
    SvInvokeInterface(QBAnyPlayerController, self->controller, onPlaybackFinished, QBAnyPlayerLogicIsLoopEnabled(self));

    if (self->playbackStateController) {
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, finish);
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, onPlaybackFinished, QBAnyPlayerLogicIsLoopEnabled(self));
    }
}

void QBAnyPlayerLogicSetLastPositionBookmark(QBAnyPlayerLogic self, QBBookmark bookmark)
{
    if (!self || !bookmark) {
        SvLogError("%s: NULL argument passed", __func__);
        return;
    }

    self->lastPositionBokmark = SVRETAIN(bookmark);
}

QBBookmark QBAnyPlayerLogicGetLastPositionBookmark(QBAnyPlayerLogic self)
{
    return self->lastPositionBokmark;
}

void QBAnyPlayerLogicSetStartPosition(QBAnyPlayerLogic self, double position)
{
    if (!self) {
        SvLogError("%s: NULL argument passed", __func__);
        return;
    }

    self->startPosition = position;
}

double QBAnyPlayerLogicGetStartPosition(QBAnyPlayerLogic self)
{
    if (!self) {
        SvLogError("%s: NULL argument passed", __func__);
        return -1;
    }
    return self->startPosition;
}

void QBAnyPlayerLogicSetBookmarkId(QBAnyPlayerLogic self, SvString bookmarkId)
{
    self->bookmarkId = SVRETAIN(bookmarkId);
}

SvString QBAnyPlayerLogicGetBookmarkId(QBAnyPlayerLogic self)
{
    return self->bookmarkId;
}

void QBAnyPlayerLogicSetVodId(QBAnyPlayerLogic self, SvString id)
{
    self->vodId = SVRETAIN(id);
}

SvString QBAnyPlayerLogicGetVodId(QBAnyPlayerLogic self)
{
    return self->vodId;
}

void QBAnyPlayerLogicSetController(QBAnyPlayerLogic self, SvObject controller)
{
    if (!controller) {
        SvLogError("%s: NULL controller passed", __func__);
        return;
    }

    if (controller == self->controller) {
        return;
    }

    SvInvokeInterface(QBAnyPlayerController, self->controller, stop);
    SVRELEASE(self->controller);
    self->controller = SVRETAIN(controller);
}

void QBAnyPlayerLogicSetPlaybackStateController(QBAnyPlayerLogic self, SvObject controller)
{
    if (!self || !SvObjectIsInstanceOf(controller, QBPlaybackStateController_getType()))
        return;

    SVTESTRELEASE(self->playbackStateController);
    self->playbackStateController = SVTESTRETAIN(controller);
}

SvLocal SvWidget QBAnyPlayerLogicCreatePopup(AppGlobals appGlobals, const char *title, const char *message)
{
    svSettingsPushComponent("Carousel_VOD.settings");
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = 0,
    };
    SvWidget dialog = QBSimpleDialogCreate(&params, title, message);
    svSettingsPopComponent();
    return dialog;
}

SvLocal void QBAnyPlayerLogicPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBAnyPlayerLogic self = (QBAnyPlayerLogic) self_;
    self->logicDialog = NULL;
    if (buttonTag && SvStringEqualToCString(buttonTag, "button")) {
        QBApplicationControllerPopContext(self->appGlobals->controller);
    }
}

SvLocal void QBAnyPlayerLogicOnControllerEvent(SvObject self_, int eventID, SvObject data)
{
    QBAnyPlayerLogic self = (QBAnyPlayerLogic) self_;
    SvString eventData = QBPVRPlayerContextCreateEventData(self->pvrPlayer, NULL);
    if (self->showDialog) {
        const char *message;
        char *escapedErrorMsg;
        char *escapedMsg;
        switch (eventID) {
            case QBAnyPlayerControllerEventID_entitlementExpired:
                // pop pvr context and show popup
                message = gettext("Rental period for this content has expired.");
                QBAnyPlayerLogicToggleAll(self, false);
                self->logicDialog = QBAnyPlayerLogicCreatePopup(self->appGlobals, gettext("Rental period expired"), message);
                QBDialogRun(self->logicDialog, self, QBAnyPlayerLogicPopupCallback);
                QBSecureLogEvent("AnyPlayer", "ErrorShown.AnyPlayer.EntitlementExpired", "JSON:{\"description\":\"%s\"%s}", message, eventData ? SvStringCString(eventData) : "");
                break;
            case QBAnyPlayerControllerEventID_notEntitled:
                QBAnyPlayerLogicToggleAll(self, false);
                message = data ? SvStringCString((SvString) data) : "";
                self->logicDialog = QBAnyPlayerLogicCreatePopup(self->appGlobals, gettext("Not entitled"), message);
                QBDialogRun(self->logicDialog, self, QBAnyPlayerLogicPopupCallback);
                escapedMsg = QBStringCreateJSONEscapedString(message);
                QBSecureLogEvent("AnyPlayer", "ErrorShown.AnyPlayer.NotEntitled", "JSON:{\"description\":\"%s\"%s}", escapedMsg, eventData ? SvStringCString(eventData) : "");
                free(escapedMsg);
                break;
            case QBAnyPlayerControllerEventID_error:
                QBAnyPlayerLogicToggleAll(self, false);
                message = gettext("An error has occurred. Please contact your service provider.\n");
                SvString errorMsg = SvStringCreateWithFormat("%s[%s]", message, data ? SvStringCString((SvString) data) : " ");
                self->logicDialog = QBAnyPlayerLogicCreatePopup(self->appGlobals, gettext("A problem occurred"), SvStringCString(errorMsg));
                QBDialogRun(self->logicDialog, self, QBAnyPlayerLogicPopupCallback);
                escapedMsg = QBStringCreateJSONEscapedString(data ? SvStringCString((SvString) data) : " ");
                escapedErrorMsg = QBStringCreateJSONEscapedString(message);
                QBSecureLogEvent("AnyPlayer", "ErrorShown.AnyPlayer.Playback", "JSON:{\"description\":\"%s[%s]\"%s,\"errorCode\":\"%d\"}", escapedErrorMsg, escapedMsg, eventData ? SvStringCString(eventData) : "", self->errorCode);
                free(escapedMsg);
                free(escapedErrorMsg);
                SvLogError("%s : %s", __FUNCTION__, SvStringCString(errorMsg));
                SVRELEASE(errorMsg);
                break;
            case QBAnyPlayerControllerEventID_noTunerAccess:
                QBAnyPlayerLogicToggleAll(self, false);
                message = gettext("Lost tuner access");
                self->logicDialog = QBAnyPlayerLogicCreatePopup(self->appGlobals, gettext("Error"), data ? SvStringCString((SvString) data) : message);
                escapedMsg = QBStringCreateJSONEscapedString(data ? SvStringCString((SvString) data) : message);
                QBSecureLogEvent("AnyPlayer", "ErrorShown.AnyPlayer.NoTunerAccess", "JSON:{\"description\":\"%s\"%s}", escapedMsg, eventData ? SvStringCString(eventData) : "");
                free(escapedMsg);
                QBDialogRun(self->logicDialog, self, QBAnyPlayerLogicPopupCallback);
                break;
            case QBAnyPlayerControllerEventID_playlistFinished:
                QBApplicationControllerPopContext(self->appGlobals->controller);
                break;
            case QBAnyPlayerControllerEventID_playlistError:
                QBAnyPlayerLogicToggleAll(self, false);
                message = gettext("Playlist error");
                self->logicDialog = QBAnyPlayerLogicCreatePopup(self->appGlobals, gettext("Error"), data ? SvStringCString((SvString) data) : message);
                escapedMsg = QBStringCreateJSONEscapedString(data ? SvStringCString((SvString) data) : message);
                QBSecureLogEvent("AnyPlayer", "ErrorShown.AnyPlayer.Playlist", "JSON:{\"description\":\"%s\"%s}", escapedMsg, eventData ? SvStringCString(eventData) : "");
                free(escapedMsg);
                QBDialogRun(self->logicDialog, self, QBAnyPlayerLogicPopupCallback);
                break;
            default:
                SvLogWarning("%s: unrecognized eventID (%d). Ignoring.", __func__, eventID);
                break;
        }
    }
    SVTESTRELEASE(eventData);
}

SvLocal SvString create_message_from_error_code(int code)
{
    if (code <= SV_ERR_INIT_DECODER && code > SV_ERR_DRM_INIT_FAILURE) {
        return SvStringCreateWithFormat(gettext("Decoder error: %d"), code);
    } else if (code <= SV_ERR_DRM_INIT_FAILURE && code > SV_ERR_SERVER_CONN_FAILED) {
        return SvStringCreateWithFormat(gettext("DRM error: %d"), code);
    } else if (code <= SV_ERR_SERVER_CONN_FAILED && code > SV_ERR_CPC_XML_SYNTAX_ERROR) {
        switch (code) {
            case SV_ERR_SERVER_FILE_NOT_FOUND:  return SvStringCreateWithFormat(gettext("File not found: %d"), code);
            case SV_ERR_UNSUPPORTED_FORMAT:     return SvStringCreateWithFormat(gettext("Format not supported: %d"), code);
            case SV_ERR_UNSUPPORTED_PROTOCOL:   return SvStringCreateWithFormat(gettext("Protocol not supported: %d"), code);
            case SV_ERR_BAD_URL:                return SvStringCreateWithFormat(gettext("Bad URL: %d"), code);
            case SV_ERR_BAD_SERVER_RESPONSE:    return SvStringCreateWithFormat(gettext("Bad server response: %d"), code);
            case SV_ERR_UNEXPECTED_SERVER_RESPONSE: return SvStringCreateWithFormat(gettext("Unexpected server response: %d"), code);
            default:                            return SvStringCreateWithFormat(gettext("Server communication error: %d"), code);
        }
    } else if (code == SV_ERR_NOT_FOUND) {
        return SvStringCreate(gettext("File not found"), NULL);
    } else if (code == SV_ERR_BAD_FORMAT) {
        return SvStringCreateWithFormat(gettext("Bad format: %d"), code);
    }
    return SvStringCreateWithFormat(gettext("Playback failed with error code: %d"), code);
}

void QBAnyPlayerLogicHandlePlayerError(QBAnyPlayerLogic self, SvErrorInfo errorInfo)
{
    self->errorCode = SvErrorInfoGetCode(errorInfo);
    if (!self->showDialog) {
        return;
    }
    SvString message = create_message_from_error_code(SvErrorInfoGetCode(errorInfo));
    QBAnyPlayerLogicOnControllerEvent((SvObject) self, QBAnyPlayerControllerEventID_error, (SvObject) message);
    SVTESTRELEASE(message);
    self->errorCode = 0;
}

void QBAnyPlayerLogicLockDisplay(QBAnyPlayerLogic self)
{
    self->displayLocked = true;
    if (QBAnyPlayerLogicAnyLocalWindowIsVisible(self))
        QBAnyPlayerLogicHideAllLocalWindows(self);
}

void QBAnyPlayerLogicPostWidgetsCreate(QBAnyPlayerLogic self)
{
    self->showDialog = true;
    if (self->errorCode) {
        SvString message = create_message_from_error_code(self->errorCode);
        QBAnyPlayerLogicOnControllerEvent((SvObject) self, QBAnyPlayerControllerEventID_error, (SvObject) message);
        SVTESTRELEASE(message);
        self->errorCode = 0;
    }
}

void QBAnyPlayerLogicUnlockDisplay(QBAnyPlayerLogic self)
{
    self->displayLocked = false;
}

void QBAnyPlayerLogicPreWidgetsDestroy(QBAnyPlayerLogic self)
{
    self->showDialog = false;
}

void QBAnyPlayerLogicGotPlayerEvent(QBAnyPlayerLogic self, SvString name, void *arg)
{
    SvInvokeInterface(QBAnyPlayerController, self->controller, gotEvent, name, arg);
    if (self->playbackStateController)
        SvInvokeInterface(QBAnyPlayerController, self->playbackStateController, gotEvent, name, arg);
}

void QBAnyPlayerLogicSetProperty(QBAnyPlayerLogic self, SvValue propertyKey, SvValue propertyValue)
{
    if (!propertyValue)
        SvHashTableRemove(self->properties, (SvObject) propertyKey);
    else
        SvHashTableInsert(self->properties, (SvObject) propertyKey, (SvObject) propertyValue);
}

void QBAnyPlayerLogicShowOnFrontPanel(QBAnyPlayerLogic self)
{
    const char *title = QBPVRPlayerContextGetTitle((QBWindowContext) self->pvrPlayer);
    if (title)
        QBPlatformShowOnFrontPanel(title);
}

void QBAnyPlayerLogicContentSideMenuRegisterCallbacks(QBAnyPlayerLogic self, QBContentSideMenu contentSideMenu)
{
}

void QBAnyPlayerLogicSetPlayReadyLicenceRequestParams(QBAnyPlayerLogic self, SvString licenceRequestParams)
{
    SVTESTRELEASE(self->playReadyLicenceRequestParams);
    self->playReadyLicenceRequestParams = SVTESTRETAIN(licenceRequestParams);
}

void QBAnyPlayerLogicSetPlayReadyLicenceRequestCustomData(QBAnyPlayerLogic self, SvString customData)
{
    SVTESTRELEASE(self->playReadyLicenceRequestCustomData);
    self->playReadyLicenceRequestCustomData = SVTESTRETAIN(customData);
}

void QBAnyPlayerLogicSetPlayReadyLicenceRequestSSLParams(QBAnyPlayerLogic self, SvSSLParams sslParams)
{
    SVTESTRELEASE(self->playReadyLicenceRequestSSLParams);
    self->playReadyLicenceRequestSSLParams = SVTESTRETAIN(sslParams);
}

QBOSDMain QBAnyPlayerLogicOSDCreate(QBAnyPlayerLogic self, SvApplication app, void *userdata)
{
    svSettingsPushComponent("pvrPlayer.settings");
    QBPVROSD OSD = QBPVROSDCreate(app, self->appGlobals, userdata);
    QBPVROSDTSRecording tsosd = QBPVROSDTSRecordingCreate(app, self->appGlobals->scheduler, NULL);
    QBOSDMainAttachOSD((QBOSDMain) OSD, (QBOSD) tsosd, QBOSDMainAttachmentPosition_bottom, 1, NULL);
    QBObservableAddObserver((QBObservable) tsosd, (SvObject) OSD, NULL);
    SVRELEASE(tsosd);
    svSettingsPopComponent();
    return (QBOSDMain) OSD;
}
