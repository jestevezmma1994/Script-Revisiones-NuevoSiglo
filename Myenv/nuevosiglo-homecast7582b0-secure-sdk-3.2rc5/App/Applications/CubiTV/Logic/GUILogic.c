/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include <Logic/GUILogic.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <QBConf.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <Windows/newtv.h>
#include <Windows/newtvguide.h>
#include <Windows/verticalEPG.h>
#include <Windows/QBVoDCategoryBrowser.h>
#include <Windows/QBVoDCarousel.h>
#include <Windows/QBVoDGridBrowser.h>
#include <Services/core/playlistManager.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <main.h>
#include <Widgets/QBRestartDialog.h>
#include <QBWidgets/QBDialog.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/widget.h>
#include <settings.h>

#define STB_MODE_CHANGE_RESTART_DIALOG_DELAY_SEC    5

#if SV_LOG_LEVEL > 0
    #define moduleName "QBGUILogic"
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "Debug", "");
    #define log_error(fmt, ...) do { if (env_log_level() > 0) { SvLogError(COLBEG() moduleName " ::" fmt COLEND_COL(red), ## __VA_ARGS__); } } while (0)
    #define log_warning(fmt, ...) do { if (env_log_level() > 0) { SvLogWarning(COLBEG() moduleName ":: " fmt COLEND_COL(yellow), ## __VA_ARGS__); } } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() moduleName ":: " fmt COLEND_COL(blue), ## __VA_ARGS__); } } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() moduleName ":: " fmt COLEND_COL(green), ## __VA_ARGS__); } } while (0)
#else
    #define log_error(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_state(fmt, ...)
    #define log_debug(fmt, ...)
#endif

struct QBGUILogic_t {
    struct SvObject_ super_;
    SvWidget dialog;
    AppGlobals appGlobals;
};

SvLocal void QBGUILogicShowRebootPopup(SvObject self_, SvObject observedObject, SvObject arg);

SvType
QBGUILogic_getType(void)
{
    static SvType type = NULL;
    static struct QBObserver_ observable = {
        .observedObjectUpdated = QBGUILogicShowRebootPopup,
    };
    if (!type) {
        SvTypeCreateManaged("QBGUILogic",
                            sizeof(struct QBGUILogic_t),
                            SvObject_getType(), &type,
                            QBObserver_getInterface(), &observable,
                            NULL);
    }
    return type;
}

QBGUILogic
QBGUILogicCreate(AppGlobals appGlobals)
{
    QBGUILogic self = (QBGUILogic) SvTypeAllocateInstance(QBGUILogic_getType(), NULL);
    self->appGlobals = appGlobals;
    return self;
}

void
QBGUILogicStart(QBGUILogic self)
{
    if (self->appGlobals->qbMWConfigHandler)
        QBMWConfigHandlerAddListener(self->appGlobals->qbMWConfigHandler, (SvObject) self);
}

void
QBGUILogicStop(QBGUILogic self)
{
    if (self->appGlobals->qbMWConfigHandler)
        QBMWConfigHandlerRemoveListener(self->appGlobals->qbMWConfigHandler, (SvObject) self);
}

QBWindowContext
QBGUILogicPrepareEPGContext(QBGUILogic self, SvString epgView, SvString playlist)
{
    const char *view = NULL;
    if (epgView)
        view = SvStringCString(epgView);
    else {
        view = QBConfigGet("EPGVIEW");
        if (!view)
            view = "grid";
    }

    QBConfigSet("EPGVIEW", view);
    QBConfigSave();

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvString previous = QBPlaylistManagerGetCurrentId(playlists);

    if (playlist && !SvObjectEquals((SvObject) playlist, (SvObject) previous)) {
        SvObject nextPlaylist = QBPlaylistManagerGetById(playlists, playlist);
        SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
        if (channel) {
            int idx = SvInvokeInterface(SvEPGChannelView, nextPlaylist, getChannelIndex, channel);
            if (idx < 0)
                idx = 0;
            QBTVLogicPlayChannelByIndexFromPlaylist(self->appGlobals->tvLogic, playlist, idx, SVSTRING("EPG"));
        } else {
            QBTVLogicResumePlaying(self->appGlobals->tvLogic, SVSTRING("EPG"));
        }
    }

    if (!strcmp(view, "grid")) {
        svNewTVGuideSetFocus(self->appGlobals->newTVGuide, NULL);
        return self->appGlobals->newTVGuide;
    } else {
        QBVerticalEPGSetFocus(self->appGlobals->verticalEPG, NULL, 0);
        return self->appGlobals->verticalEPG;
    }
}

SvLocal void
QBGUILogicRestartPopupCallback(void *self_,
                               SvWidget dialog,
                               SvString buttonTag,
                               unsigned keyCode)
{
    QBGUILogic self = (QBGUILogic) self_;

    FILE *f1 = fopen("/tmp/dropChannels", "w");
    FILE *f2 = fopen("/etc/vod/dropChannels", "w");
    if (f1 || f2) {
        if (f1) {
            fclose(f1);
        }

        if (f2) {
            fclose(f2);
        }
        log_error("%s(): channels will be dropped on startup", __func__);
    }

    log_error("%s(): call QBInitStopApplication", __func__);
    QBInitStopApplication(self->appGlobals, true, "dropping channels");
}

SvLocal void
QBGUILogicRestartPopupCallbackRebootOnly(void *self_,
                                         SvWidget dialog,
                                         SvString buttonTag,
                                         unsigned keyCode)
{
    QBGUILogic self = (QBGUILogic) self_;

    QBInitStopApplication(self->appGlobals, true, "rebooting box");
}

SvLocal void QBGUILogicShowRebootPopup(SvObject self_, SvObject observedObject, SvObject arg)
{
    QBGUILogic self = (QBGUILogic) self_;

    if (QBInitLogicIsWizardFinished(self->appGlobals->initLogic)) {
        int message = SvValueGetInteger((SvValue) arg);
        switch (message) {
            case CONFIG_STATE_READ_CHANGE:
                QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
                QBConfigSet("FIRSTBOOT", "YES");
                QBConfigSave();
                self->dialog = QBRestartDialogCreate(self->appGlobals, STB_MODE_CHANGE_RESTART_DIALOG_DELAY_SEC);
                if (self->dialog) {
                    QBDialogRun(self->dialog, self, QBGUILogicRestartPopupCallback);
                }
                break;
            case CONFIG_STATE_READ_CHANGE_NO_FIRSTBOOT:
                QBApplicationControllerSwitchToRoot(self->appGlobals->controller);
                self->dialog = QBRestartDialogCreate(self->appGlobals, STB_MODE_CHANGE_RESTART_DIALOG_DELAY_SEC);
                if (self->dialog) {
                    QBDialogRun(self->dialog, self, QBGUILogicRestartPopupCallbackRebootOnly);
                }
                break;
            default:
                break;
        }
    }
}

QBWindowContext
QBGUILogicPrepareVoDContext(QBGUILogic self,
                            SvString vodView,
                            SvObject path,
                            SvObject provider,
                            SvString serviceId,
                            SvString serviceName,
                            bool leaf,
                            QBActiveTree externalTree)
{
    const char *view = NULL;

    if (vodView && !SvStringEqualToCString(vodView, "default"))
        view = SvStringCString(vodView);
    else
        view = QBConfigGet("VODVIEW");

    if (!view)
        view = "vod1";

    if (!strcmp(view, "vod1")) {
        return NULL;
    } else if (!strcmp(view, "vod2")) {
        if (leaf) {
            return QBVoDCarouselContextCreate(self->appGlobals, path, provider,
                                              serviceId, serviceName, false, true);
        } else {
            return QBVoDCategoryBrowserContextCreate(self->appGlobals, path,
                                                     provider, serviceId,
                                                     serviceName, false, true);
        }
    } else if (!strcmp(view, "vod3")) {
        QBWindowContext ret = QBVoDGridBrowserContextCreate(self->appGlobals, path,
                                                            provider, serviceId,
                                                            serviceName, externalTree,
                                                            NULL, NULL);
        return ret;
    } else if (!strcmp(view, "vod4")) {
        QBVoDGridBrowserSettings settings = QBVoDGridBrowserSettingsCreate();
        settings->useSideMenu = true;
        QBWindowContext ret = QBVoDGridBrowserContextCreate(self->appGlobals, path,
                                                            provider, serviceId,
                                                            serviceName, externalTree,
                                                            NULL, settings);
        SVRELEASE(settings);
        return ret;
    } else if (!strcmp(view, "digitalsmiths")) {
        QBVoDGridBrowserSettings settings = QBVoDGridBrowserSettingsCreate();
        settings->useSideMenu = true;
        settings->searchInNewTab = true;
        settings->skipSearchDialog = false;
        settings->detailsScreen = QBVoDGridBrowserDetailsScreen_ColumbusDS;
        settings->defaultButtonIndex = 0;
        settings->showPopupInEmptyCategory = true;
        settings->settingsFile = SvStringCreate("VoDGridDS.settings", NULL);
        settings->refreshObject = true;
        settings->enableKeyShortcuts = true;
        settings->moviesButtonMimicsVodButton = false;
        settings->moveToRootOnMoviesButton = false;
        QBWindowContext ret = QBVoDGridBrowserContextCreate(self->appGlobals, path,
                                                            provider, serviceId,
                                                            serviceName, externalTree,
                                                            NULL, settings);
        SVRELEASE(settings);
        return ret;
    }

    return NULL;
}

void QBGUILogicMenuBarItemControllerRegisterCallbacks(QBGUILogic self, SvObject menuBarItemController)
{
}

void QBGUILogicPVRMenuContextRegisterCallbacks(QBGUILogic self, SvObject pvrMenuHandler)
{
}

void QBGUILogicRemiderEditorPaneRegisterCallbacks(QBGUILogic self, SvObject reminderEditorPane)
{
}


void QBGUILogicMiniChannelListRegisterCallbacks(QBGUILogic self, SvObject miniChannelList)
{
}

void QBGUILogicChannelMenuControllerRegisterCallbacks(QBGUILogic self, SvObject channelController_)
{
}

void QBGUILogicContentSideMenuRegisterCallbacks(QBGUILogic self, SvObject contentSideMenu)
{
}

SvWidget QBGUILogicCreateBackgroundWidget(QBGUILogic self, const char *windowName, SvErrorInfo *errorOut)
{
    SvBitmap background = svSettingsGetBitmap(windowName, "bg");
    return svWidgetCreateBitmap(self->appGlobals->res, self->appGlobals->res->width, self->appGlobals->res->height, background);
}
