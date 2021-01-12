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

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <main.h>
#include <init.h>
#include <QBShellCmds.h>
#include <QBConf.h>
#include <QBUpgrade/QBHTTPUpgradeMonitor.h>
#include <Widgets/upgradeDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Services/upgradeWatcher.h>
#include <Utils/appType.h>
#include <Logic/UpgradeLogic.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 3, "QBUpgradeLogLevel", "" );

#define log_debug(fmt, ...) if(env_log_level() >= 5) { SvLogNotice(COLBEG() "QBUpgradeLogic :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); }
#define log_info(fmt, ...)  if(env_log_level() >= 3) { SvLogNotice(COLBEG() "QBUpgradeLogic :: %s " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); }
#define log_error(fmt, ...) if(env_log_level() >= 1) { SvLogError(COLBEG() "QBUpgradeLogic :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); }

struct QBUpgradeLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;

    bool autoDownload;                 /**< If upgrade package download should be started automatically when new version of firmware is found (used only when PVR is used) */
    bool autoInstall;                  /**< Tells what is default option in new firmware InstallPopup dialog (what happens if user doesn't click any option):
                                             - true : popup will disapear after fixed timeout and firmware will be installed,
                                             - false : firmware won't be installed automatically (unless firmware has 'force' option enabled) */

    SvTime finalUpdateTime;

    SvScheduler scheduler;
    SvFiber fiber;
    SvFiberTimer fiberTimer;

    SvWidgetId dialogID;

    SvString newVersionStr;            /**< Version of new firmware */
    SvString networkName;
    SvURI upgradeURI;                  /**< URI to source from where new upgrade package can be downloaded: DVB or HTTP */
    SvURI installURI;                  /**< URI to source from where new upgrade package can be installed:
                                            for PVR mode -  it is file on disk where upgrade package is already downloaded,
                                            for non-PVR (diskless) mode - it is direct URI (DVB or HTTP) and is equal to upgradeURI */
    bool detected;                     /**< new firmware was detected and can be downloaded */
    SvString upgradeType;              /**< type of upgrade (HTTP or DVB) */
    bool forced;                       /**< If upgrade is forced (when new firmware was found): true means that user can't cancel or delay */

    bool terminating;                  /**< If application is already closing (due to scheduled upgrade to be run after reboot) */

};


SvLocal bool QBUpgradeLogicIsStandbyOn(QBUpgradeLogic self)
{
    QBStandbyAgent agent = self->appGlobals->standbyAgent;

    if (agent) {
        return QBStandbyAgentIsStandby(agent);
    }

    return true;
}

SvLocal void QBUpgradeLogicSetStatus(QBUpgradeLogic self,
                                     SvString title,
                                     bool showVersion, bool active)
{
    QBActiveTreeNode node = NULL;
    SvObject action = NULL;

    if (self->appGlobals->menuTree)
        node = QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("UPDATE_STATUS"));

    if (!node)
        return;

    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvObject) title);

    SvString subcaption = NULL;
    if (showVersion && self->networkName) {
        const char *fmt = gettext("Version: %s, Network: %s");
        subcaption = SvStringCreateWithFormat(fmt, SvStringCString(self->newVersionStr), SvStringCString(self->networkName));
    } else if (showVersion) {
        subcaption = SvStringCreateWithFormat(gettext("Version: %s"), SvStringCString(self->newVersionStr));
    }
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvObject) subcaption);
    SVTESTRELEASE(subcaption);

    if (active)
        action = (SvObject) SVSTRING("Check software update");
    QBActiveTreeNodeSetAttribute(node, SVSTRING("action"), action);

    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, NULL);
}

SvLocal void QBUpgradeLogicCreateMarker(QBUpgradeLogic self,
                                        const char *markerFilePath)
{
    const char *uriCStr = NULL;
    if (self->installURI)
        uriCStr = SvStringCString(SvURIString(self->installURI));
    else if (self->upgradeURI)
        uriCStr = SvStringCString(SvURIString(self->upgradeURI));
    else
        uriCStr = "none";

    log_info("UpgradeLogic: writing '%s' to %s", uriCStr, markerFilePath);

    FILE *f = fopen(markerFilePath, "w");
    if (f) {
        if (uriCStr)
        {
            // BEGIN RAL Add band parameter to ota-uri to fix OTA upgrades
            if (self->upgradeType && SvStringEqualToCString(self->upgradeType, "DVB"))
            {
                fprintf(f, "%s&band=6\n", uriCStr);
            }
            else
            {
                fprintf(f, "%s\n", uriCStr);
            }
            //fprintf(f, "%s\n", uriCStr);
            // END RAL Add band parameter to ota-uri to fix OTA upgrades
        }
        fclose(f);
    }
}

/**
 * Schedules upgrade to be run after reboot (stores upgrade URI on flash) and stops application
 */
SvLocal void QBUpgradeLogicUpgrade(QBUpgradeLogic self)
{
    if (!self->terminating) {
        self->terminating = true;
        if (self->upgradeURI || self->installURI) {
            QBUpgradeLogicCreateMarker(self, "/etc/vod/ota-uri");
            QBShellExec("store_config.sh");
        }

        log_error("call QBInitStopApplication");
        QBInitStopApplication(self->appGlobals, true, "upgrade");
        QBWatchdogRebootAfterTimeout(self->appGlobals->watchdog, WATCHDOG_LONG_TIMEOUT_SEC, NULL);
    }
}

/**
 * Install popup callback (called when user chooses any option or timeout elapses)
 */
SvLocal void QBUpgradeLogicInstallPopupCallback(void *self_,
                                                SvWidget dialog,
                                                SvString buttonTag,
                                                unsigned keyCode)
{
    log_debug();
    QBUpgradeLogic self = self_;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
        // user has decided to run upgrade
        QBUpgradeLogicUpgrade(self);
    } else if (!buttonTag && (self->forced || self->autoInstall)) {
        // user hasn't chosen any option - but upgrade is enforced (either by upgrade package flag or by box configuration)
        QBUpgradeLogicUpgrade(self);
    }
}

SvLocal void QBUpgradeLogicShowInstallPopup(QBUpgradeLogic self)
{
    log_debug();
    SvWidget dialog = NULL;
    if (self->dialogID) {
        dialog = svAppFindWidget(self->appGlobals->res, self->dialogID);
        if (dialog)
            QBDialogBreak(dialog);
        self->dialogID = 0;
    }

    if (self->forced) {
        log_info("UpgradeLogic: upgrade is forced (upgradeType=%s)", self->upgradeType ? SvStringCString(self->upgradeType) : "NOT SET!");
        QBUpgradeLogicCreateMarker(self, "/etc/vod/ota-uri");
        dialog = QBUpgradeDialogCreate(self->appGlobals, true, self->autoInstall,
                                       self->newVersionStr, self->networkName, 30);
    } else {
        dialog = QBUpgradeDialogCreate(self->appGlobals, false, self->autoInstall,
                                       self->newVersionStr, self->networkName, 30);
    }

    if (dialog) {
        self->dialogID = svWidgetGetId(dialog);
        QBDialogRun(dialog, self, QBUpgradeLogicInstallPopupCallback);
    }
}

SvLocal void QBUpgradeLogicDestroy(void *self_)
{
    QBUpgradeLogic self = self_;
    if (self->fiber)
        SvFiberDestroy(self->fiber);

    SVTESTRELEASE(self->newVersionStr);
    SVTESTRELEASE(self->networkName);
    SVTESTRELEASE(self->upgradeURI);
    SVTESTRELEASE(self->installURI);
    SVTESTRELEASE(self->upgradeType);
}

SvLocal void
QBUpgradeLogicStep(void* self_)
{
    QBUpgradeLogic self = (QBUpgradeLogic) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->fiberTimer);

    SvTime now = SvTimeGet();
    if (SvTimeCmp(now, self->finalUpdateTime) < 0) {
        SvTime diff = SvTimeSub(self->finalUpdateTime, now);
        SvString title = NULL;
        int timeToUpgrade = SvTimeToMs(diff) / 1000;
        if (timeToUpgrade < 20) {
            title = SvStringCreate(gettext("Update Starts Soon"), NULL);
        } else {
            title = SvStringCreateWithFormat(ngettext("Update Starts In %d Minute", "Update Starts In %d Minutes", ((timeToUpgrade + 59) / 60)), ((timeToUpgrade + 59) / 60));
        }
        QBUpgradeLogicSetStatus(self, title, true, true);
        SVRELEASE(title);

        SvFiberTimerActivateAfter(self->fiberTimer, SvTimeConstruct(10, 0));
    } else {
        SvLogNotice("UpgradeLogic: new firmware version ready to be installed from %s",
                    SvStringCString(self->upgradeType));

        SvString title = SvStringCreate(gettext("Install now"), NULL);
        QBUpgradeLogicSetStatus(self, title, true, true);
        SVRELEASE(title);

        if (!QBUpgradeLogicIsStandbyOn(self)) {
            QBUpgradeLogicShowInstallPopup(self);
        }
    }
}

SvLocal SvType QBUpgradeLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBUpgradeLogicDestroy
    };
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBUpgradeLogic",
                            sizeof(struct QBUpgradeLogic_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal time_t QBUpgradeLogicGenerateRandomUpgradeDelayTime(QBUpgradeLogic self)
{
    return ((1 + (rand() % 15)) * 60);
}

QBUpgradeLogic QBUpgradeLogicCreate(AppGlobals appGlobals)
{
    QBUpgradeLogic self = (QBUpgradeLogic) SvTypeAllocateInstance(QBUpgradeLogic_getType(), NULL);

    self->appGlobals = appGlobals;

    QBUpgradeLogicReconfigure(self);

    return self;
}

void QBUpgradeLogicStart(QBUpgradeLogic self)
{
    SvScheduler scheduler = SvSchedulerGet();
    self->scheduler = scheduler;
    self->fiber = SvFiberCreate(scheduler, NULL, "QBUpgradeLogic", QBUpgradeLogicStep, self);
    self->fiberTimer = SvFiberTimerCreate(self->fiber);
}

void QBUpgradeLogicStop(QBUpgradeLogic self)
{
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->fiberTimer = NULL;
    }
}

void QBUpgradeLogicReconfigure(QBUpgradeLogic self)
{
    const char *v;

    if ((v = QBConfigGet("DVBUPGRADE.AUTODOWNLOAD")) && strcmp(v, "enabled") == 0)
        self->autoDownload = true;
    else
        self->autoDownload = false;
    if ((v = QBConfigGet("DVBUPGRADE.AUTOINSTALL")) && strcmp(v, "enabled") == 0)
        self->autoInstall = true;
    else
        self->autoInstall = false;
}

void
QBUpgradeLogicCheck(QBUpgradeLogic self)
{
    if (self->newVersionStr) {
        if (!QBAppTypeIsPVR() && self->upgradeURI) {
            QBUpgradeLogicUpgrade(self);
        } else if (self->installURI) {
            QBUpgradeLogicUpgrade(self);
        } else if (self->detected) {
            // download now
            SvString title = SvStringCreate(gettext("Status: Download scheduled"), NULL);
            QBUpgradeLogicSetStatus(self, title, true, false);
            SVTESTRELEASE(title);
            UpgradeWatcherStartDownload(self->appGlobals->upgradeWatcher, self->newVersionStr);
        }
    } else {
        UpgradeWatcherForceHTTPUpgradeCheck(self->appGlobals->upgradeWatcher);
    }
}

void QBUpgradeLogicUpgradeNotify(QBUpgradeLogic self)
{
    if (!self->newVersionStr)
        return;

    if (self->installURI) {
        SvFiberActivate(self->fiber);
    } else if (self->detected && !self->autoDownload) {
        // FIXME: do we have to show download popup?
    }
}

void QBUpgradeLogicStandbyLeft(QBUpgradeLogic self)
{
    QBUpgradeLogicUpgradeNotify(self);
}

void QBUpgradeLogicDownloadAvailable(QBUpgradeLogic self,
                                     SvObject monitor,
                                     SvString version,
                                     SvString networkName,
                                     SvURI upgradeURI,
                                     bool forced,
                                     bool upgradeImmediately)
{
    log_debug("");
    SvString title = NULL;
    bool showVersion = false;
    bool active = false;

    SVTESTRELEASE(self->newVersionStr);
    self->newVersionStr = NULL;
    SVTESTRELEASE(self->networkName);
    self->networkName = NULL;
    SVTESTRELEASE(self->upgradeURI);
    self->upgradeURI = NULL;
    SVTESTRELEASE(self->installURI);
    self->installURI = NULL;

    if (!version) {
        title = SvStringCreate(gettext("Status: Not available"), NULL);
        self->detected = false;
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->fiberTimer);
        goto fini;
    }

    self->newVersionStr = SVRETAIN(version);
    self->networkName = SVTESTRETAIN(networkName);
    self->upgradeURI = SVTESTRETAIN(upgradeURI);
    self->detected = true;
    if (upgradeImmediately) {
        self->finalUpdateTime = SvTimeGet();
    } else {
        self->finalUpdateTime = SvTimeAdd(SvTimeGet(), SvTimeConstruct(QBUpgradeLogicGenerateRandomUpgradeDelayTime(self), 0));
    }
    if (monitor && SvObjectIsInstanceOf(monitor, QBHTTPUpgradeMonitor_getType())) {
        self->upgradeType = SVSTRING("HTTP");
    } else {
        self->upgradeType = SVSTRING("DVB");
        if (self->upgradeURI) {
            // Log the current URI, just in case
            QBUpgradeLogicCreateMarker(self, "/etc/vod/ota-uri-current");
        }
    }

    log_info("UpgradeLogic: new firmware version ready to be downloaded from %s, upgradeForced = %d",
                SvStringCString(self->upgradeType), forced);

    if (!QBAppTypeIsPVR()) {
        // diskless OTA mode
        self->installURI = SVTESTRETAIN(upgradeURI);
        self->forced = forced;
        if (self->forced) {
            QBUpgradeLogicCreateMarker(self, "/etc/vod/ota-uri");
        }
        // We cheat here, it is available, but we want to tell that the user a little later on
        title = SvStringCreate(gettext("Status: Not available"), NULL);
        SvFiberActivate(self->fiber);
    } else if (self->autoDownload || forced) {
        UpgradeWatcherStartDownload(self->appGlobals->upgradeWatcher, version);
        title = SvStringCreate(gettext("Status: Download scheduled"), NULL);
        showVersion = true;
    } else {
        title = SvStringCreate(gettext("Download now"), NULL);
        showVersion = true;
        active = true;
    }
fini:
    QBUpgradeLogicSetStatus(self, title, showVersion, active);
    SVTESTRELEASE(title);
}

void
QBUpgradeLogicDownloadProgressChanged(QBUpgradeLogic self,
                                      unsigned int progress)
{
    if (!self->newVersionStr)
        return;

    SvString title = SvStringCreateWithFormat(gettext("Status: %u%% downloaded"), progress);
    QBUpgradeLogicSetStatus(self, title, true, false);
    SVRELEASE(title);
}

void QBUpgradeLogicUpgradeAvailable(QBUpgradeLogic self,
                                    SvObject monitor,
                                    SvString version,
                                    SvString networkName,
                                    SvURI installURI,
                                    bool forced,
                                    bool upgradeImmediately)
{
    SVTESTRELEASE(self->newVersionStr);
    self->newVersionStr = SVRETAIN(version);
    SVTESTRELEASE(self->networkName);
    self->networkName = SVTESTRETAIN(networkName);
    self->forced = forced;

    SVTESTRELEASE(self->installURI);
    self->installURI = SVRETAIN(installURI);

    if (monitor && SvObjectIsInstanceOf(monitor, QBHTTPUpgradeMonitor_getType())) {
        self->upgradeType = SVSTRING("HTTP");
        QBUpgradeLogicCreateMarker(self, "/etc/vod/http-upgrade-available");
    } else {
        self->upgradeType = SVSTRING("DVB");
        if (self->forced)
            QBUpgradeLogicCreateMarker(self, "/etc/vod/ota-uri");
    }


    log_info("UpgradeLogic: new firmware version ready to be installed from %s, upgradeForced = %d",
                SvStringCString(self->upgradeType), forced);

    if (upgradeImmediately) {
        self->finalUpdateTime = SvTimeGet();
    } else {
        self->finalUpdateTime = SvTimeAdd(SvTimeGet(), SvTimeConstruct(QBUpgradeLogicGenerateRandomUpgradeDelayTime(self), 0));
    }
    SvFiberActivate(self->fiber);
}

SvSSLParams QBUpgradeLogicCreateSSLParams(QBUpgradeLogic self)
{
    SvSSLParams params = (SvSSLParams) SvTypeAllocateInstance(SvSSLParams_getType(), NULL);
    SvSSLParamsInit(params, SvSSLVersion_default, NULL);
    SvSSLParamsSetClientAuth(params, "/etc/certkey.pem", SvSSLFileFormat_PEM, "/etc/cert.pem", SvSSLFileFormat_PEM, NULL);
    SvSSLParamsSetVerificationOptions(params, true, true, "/etc/cacert.pem", NULL);
    return params;
}
