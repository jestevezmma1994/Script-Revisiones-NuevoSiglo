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

#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libintl.h>
#include <syslog.h>
#include <QBResourceManager/QBResourceManager.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <settings.h>
#include <QBSecureLogManager.h>
#include <SvFoundation/SvData.h>
#include <SvPlayerKit/SvPlayerConfig.h>
#include <QBStaticStorage.h>
#include <QBConf.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <SvJSON/SvJSONParse.h>
#include <QBContextSwitcherStandard.h>
#include <NPvr/QBnPVRProvider.h>
#include <SvEPGDataLayer/SvEPGDataPlugin.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <Logs/logs.h>
#include <Services/drm.h>
#include <Services/QBCASManager.h>
#include <Services/upgradeWatcher.h>
#include <Services/channelMetaStorage.h>
#include <Services/core/QBDVBBouquets.h>
#include <QBInput/QBInputService.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Services/QBAccessController/ToyaAccessPlugin.h>
#include <Utils/viewport.h>
#include <safeFiles.h>
#include <Utils/channelMetaFill.h>
#include <Menus/QBConaxMenuHandler.h>
#include <Menus/QBEnvMenuHandler.h>
#include <Windows/newtv.h>
#include <Windows/pvrplayer.h>
#include <Windows/screensaver.h>
#include <Windows/standby.h>
#include <Windows/verticalEPG.h>
#include <Windows/appInitWaiter.h>
#include <dvbTimeDate.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <Services/core/watchdog.h>
#include <Services/QBCustomerInfoStorage.h>
#include <Services/QBRemoteControl.h>
#include <Services/QBRemoteMessagesManager.h>
#include <Services/QBRedirProxyManager.h>
#include <Services/InputTranslator.h>
#include <Services/QBSubtreeNotifier.h>
#include <Services/core/GlobalPlayer.h>
#include <Services/sanityChecker.h>
#include <Services/QBLogWindowManager.h>
#include <Services/QBHotplugNotifier.h>
#include <Services/core/QBPushReceiver.h>
#include <Services/QBAutoTest.h>
#include <Services/QBCallerIdManager.h>
#include <Services/core/QBSTBModeMonitor.h>
#include <Services/standbyAgent/QBNoInputStandbyPlugin.h>
#include <Services/core/QBCEC.h>
#include <Services/cecService.h>
#include <Services/core/QBContentManagers.h>
#include <Services/core/QBDebugUtils.h>
#include <Services/core/QBConnectionMonitor.h>
#include <Services/core/QBPlaybackMonitor.h>
#include <Services/QBViewRightManagers/QBViewRightWebManager.h>
#include <Services/QBCryptoguardVideoRulesManager.h>
#include <Services/QBCryptoguardForceTuneManager.h>
#include <Services/Conax/QBConaxMailManager.h>
#include <Services/QBCASPopupVisibilityManager.h>
#include <Services/core/QBStandardRastersManager.h>
#include <Services/core/QBDVBBouquets.h>
#include <Services/epgWatcher.h>
#include <QBDRMManager/QBDRMManager.h>
#include <Services/pvrAgent.h>
#include <Services/QBXHTMLWindowsService.h>
#include <Configurations/QBDiskPvrStorageMode.h>
#include <QBAppsManager.h>
#include <QBDIALServerManager.h>
#include <Utils/appType.h>
#include <QBNATMonitor/QBNATMonitor.h>
#include <Services/npvrAgent.h>
#include <DataModels/QBPVRRecordingsTree.h>
#include <Services/QBRemoteScheduler.h>
#include <QBDecoder.h>
#include <sv_tuner.h>
#include <SvProfile.h>
#include <QBTunerMonitor.h>
#include <main.h>
#include <QBOpenSSL.h>
#include <QBRecordFS/root.h>
#include <Widgets/QBRecordingIcon.h>
#include <Utils/QBCacheSize.h>
#include <Utils/QBProfileConfig.h>
#include <Utils/QBAudioStreamDesc.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <Services/core/QBMiddlewareManager.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBSmartCardMonitor.h>
#include <QBMenu/QBMainMenuInterface.h>
#include "init.h"
#include <utfconv/mpeg.h>
#include <fibers/c/fibers.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBPlayReadyManager.h>
#include <QBConaxPlayReady/QBConaxPlayReady.h>
#include <QBNetManager.h>
#include <QBDVBSatellitesDB.h>
#include <Services/QBRCUPairingService.h>

#include <Logic/AudioTrackLogic.h>
#include <Logic/VideoTrackLogic.h>
#include <Logic/SubtitleTrackLogic.h>
#include <Logic/MainMenuInitLogic.h>
#include <Logic/NewTVGuide.h>
#include <Logic/InitLogic.h>
#include <Logic/NetworkLogic.h>

#define QB_CALLER_ID_HISTORY_SIZE   40
#define QB_REBOOT_REASON_UNKNOWN    "unknown"
#define QB_REBOOT_REASON_FILE_PATH  "/etc/vod/rebootReason"

struct QBInit_t {
    SvWidget window;
    AppGlobals appGlobals;
    SvFiber fiber;
    SvFiberTimer timer;

    SvFiber loadingFiber;
    SvFiberTimer loadingTimer;

    SvFiber stoppingFiber;
    bool terminating;

    bool started;
    bool firstBoot;
    bool confChanged;

    SvString lastLoadedConfiguration;
};

SvLocal void QBInitStep(void* self_);
SvLocal void QBInitCreateMinimalSet(QBInit self);
SvLocal void QBInitStartMinimalSet(QBInit self);
SvLocal void QBInitCreateEverything(QBInit self);
SvLocal void QBInitStartEverything(QBInit self);
SvLocal void QBInitFinish(QBInit self);

SvLocal void QBInitRebootReasonLog(void)
{
    char* buffer = NULL;
    QBFileToBuffer(QB_REBOOT_REASON_FILE_PATH, &buffer);
    QBSecureLogEvent("Init", "Notice.System.PowerOn", "JSON:{\"rebootReason\":\"%s\"}", buffer ? : "unknown");
    free(buffer);
}

SvLocal void QBInitRebootReasonStore(const char* rebootReason)
{
    QBBufferToFile(QB_REBOOT_REASON_FILE_PATH, rebootReason);
}

SvLocal int addSearchPath(char *buf, int maxLength, const char *path)
{
    int len = snprintf(buf, maxLength, ":%s", path);
    SvLogNotice("  %s", path);
    return len;
}

SvLocal void QBBindTextDomain(const char *defaultDomain, const char **domains)
{
    const char *prefix = SvGetRuntimePrefix();
    if (!prefix)
        prefix = "";

    char localePath[PATH_MAX + 1];
    snprintf(localePath, PATH_MAX, "%s%s", prefix, "/usr/local/share/locale");

    for (int i = 0; domains[i]; ++i) {
        char *tmp = bindtextdomain(domains[i], localePath);
        SvLogDebug("CubiTV: bindtextdomain(\"%s\", \"%s\"): %s", domains[i], localePath, tmp);
    }

    if (defaultDomain) {
        char *tmp = textdomain(defaultDomain);
        SvLogDebug("CubiTV: textdomain(\"%s\"): %s", defaultDomain, tmp);
    }
}

SvLocal void setSearchPaths(QBResourceManager resourceManager, const char *executableName, const char *settingsPrefix)
{
    SvLogNotice("CubiTV: setting resource manager search paths:");

    const char *prefix = SvGetRuntimePrefix();
    char settingsPrefixBuffer[256];

    snprintf(settingsPrefixBuffer, sizeof(settingsPrefixBuffer),
             "%s/usr/local/share/CubiTV/data/%s", prefix, settingsPrefix);

    char buf[4096];
    int maxLength = sizeof(buf);
    int len = 0;

    len += snprintf(buf + len, maxLength - len, "./");
    len += addSearchPath(buf + len, maxLength - len, settingsPrefixBuffer);

    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s/usr/local/share/CubiTV", prefix);
    len += addSearchPath(buf + len, maxLength - len, tmp);
    snprintf(tmp, sizeof(tmp), "%s/usr/local/share/fonts/", prefix);
    len += addSearchPath(buf + len, maxLength - len, tmp);
    snprintf(tmp, sizeof(tmp), "%s/opt/share", prefix);
    len += addSearchPath(buf + len, maxLength, tmp);

    QBResourceManagerSetSearchPaths(resourceManager, buf, executableName, NULL);
}

SvLocal void initRandom(void)
{
    int seed = 0;
    FILE *f = fopen("/dev/urandom", "r");
    if(f) {
        fread(&seed, sizeof(int), 1, f);
        fclose(f);
    } else {
        SvLogWarning("Unable to open /dev/urandom!");
        seed = SvTimeNow();
    }
    SvLogNotice("rand seed = 0x%8x", seed);
    srand(seed);
}

SvLocal void initLangMaps(void)
{
    char fileName[PATH_MAX];
    char *buffer = NULL;
    SvHashTable maps = NULL;
    SvArray fallback = NULL;
    int mapsSize = 0;
    int fallbackSize = 0;
    SvIterator it;
    SvString mapId;
    SvErrorInfo errinfo = NULL;
    int i;

    snprintf(fileName, PATH_MAX - 1, "%s/usr/local/share/CubiTV/Configurations/langMap", SvGetRuntimePrefix());
    QBFileToBuffer(fileName, &buffer);
    if (!buffer) {
        SvLogNotice("Using default utfconv language map and fallbacks");
        return;
    }

    maps = (SvHashTable) SvJSONParseString(buffer, true, &errinfo);
    free(buffer);
    if (!maps) {
        SvErrorInfoDestroy(errinfo);
        SvLogNotice("Clearing utfconv language fallbacks");
        utfconv_set_fallbacks(NULL, 0);
        SvLogNotice("Clearing utfconv language map");
        utfconv_set_mappings(NULL, 0);
        return;
    }

    fallback = (SvArray) SvHashTableFind(maps, (SvGenericObject) SVSTRING("fallback"));
    if (fallback)
        fallbackSize = SvArrayCount(fallback);
    if (fallbackSize > 0) {
        const char *fallbackTable[fallbackSize];

        for (i = 0; i < fallbackSize; i++) {
            SvValue item = (SvValue) SvArrayAt(fallback, i);
            fallbackTable[i] = SvValueGetStringAsCString(item, NULL);
        }
        SvLogNotice("Setting utfconv language fallbacks:");
        utfconv_set_fallbacks(fallbackTable, fallbackSize);
        mapsSize = SvHashTableCount(maps) - 1;
    } else {
        SvLogNotice("Clearing utfconv language fallbacks");
        utfconv_set_fallbacks(NULL, 0);
        mapsSize = SvHashTableCount(maps);
    }

    if (mapsSize > 0) {
        it = SvHashTableKeysIterator(maps);
        QBLangMapping mapsTable[mapsSize];
        i = 0;
        SvLogNotice("Setting utfconv language map:");
        while ((mapId = (SvString) SvIteratorGetNext(&it))) {
            if(!SvStringEqualToCString(mapId, "fallback")) {
                SvValue encoding = (SvValue) SvHashTableFind(maps, (SvGenericObject) mapId);
                mapsTable[i].lang = SvStringCString(mapId);
                mapsTable[i].encoding = SvValueGetStringAsCString(encoding, NULL);
                i++;
            }
        }
        utfconv_set_mappings(mapsTable, mapsSize);
    } else {
        SvLogNotice("Clearing utfconv language map");
        utfconv_set_mappings(NULL, 0);
    }
    SVTESTRELEASE(maps);
}

SvLocal void QBInitMinimalSetup(QBInit self, const char* executableName, const char *settingsPrefix)
{
    AppGlobals appGlobals = self->appGlobals;
    SvSSLParams sslParams = NULL, sslParams2 = NULL;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();

    SvLogNotice("QBInit: minimal setup started");

    initRandom();

    QBOpenSSLInit();

    QBProfileConfigInit(NULL);

    appGlobals->scheduler = SvSchedulerGet();
    SvProfileInit();
    appGlobals->appPool = SvAutoreleasePoolCreate(NULL);

    QBStaticStorageStart(appGlobals->scheduler, 5000);
    QBStaticStorageSetFilesystemSync(false);

    const char* language = QBConfigGet("LANG");

    // setup locale and remember language for messages
    appGlobals->initLogic = QBInitLogicCreate(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBInitLogic_getType(), (SvObject) appGlobals->initLogic, NULL);
    SvString languageStr = SvStringCreate(language, NULL);
    QBInitLogicSetLocale(appGlobals->initLogic, languageStr);
    SVTESTRELEASE(languageStr);

    const char** domains = QBInitLogicGetTextDomains(appGlobals->initLogic);
    QBBindTextDomain(domains[0], domains);

    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    QBResourceManagerSetCacheSize(resourceManager, QBCacheSizeGetResourceBundleSoftSize(), QBCacheSizeGetResourceBundleHardSize(), NULL);
    QBResourceManagerSetMaxDownloadSize(resourceManager, 10 * 1024 * 1024, NULL); // 10MB
    QBResourceManagerSetHTTPCacheSize(resourceManager, 4 * 1024 * 1024, NULL);
    setSearchPaths(resourceManager, executableName, settingsPrefix);

    svSettingsInit(appGlobals->res);

    svAppSetupMainLoop(appGlobals->res, appGlobals->scheduler);
    svAppSetupInput(appGlobals->res, NULL);     // input will be handled by the Application Controller

    sslParams = (SvSSLParams) SvTypeAllocateInstance(SvSSLParams_getType(), NULL);
    SvSSLParamsInit(sslParams, SvSSLVersion_default, NULL);
    SvSSLParamsSetClientAuth(sslParams,
                             "/etc/certs/private/key.pem", SvSSLFileFormat_PEM,
                             "/etc/certs/private/cert.pem", SvSSLFileFormat_PEM,
                             NULL);
    SvSSLParamsSetVerificationOptions(sslParams, true, true, "/etc/certs/public/cacert.pem", NULL);
    appGlobals->sslParams = sslParams;

    sslParams2 = (SvSSLParams) SvTypeAllocateInstance(SvSSLParams_getType(), NULL);
    SvSSLParamsInit(sslParams2, SvSSLVersion_default, NULL);
    SvSSLParamsSetClientAuth(sslParams2,
                             "/etc/certs/private/key.pem", SvSSLFileFormat_PEM,
                             "/etc/certs/private/cert.pem", SvSSLFileFormat_PEM, NULL);
    SvSSLParamsSetVerificationOptions(sslParams2, true, true, "/etc/certs/public/cacert.pem", NULL);
    QBResourceManagerSetSSLParams(resourceManager, sslParams2, NULL);
    SVRELEASE(sslParams2);

    QBInitLogicLoadHlsSSLAuthParams(appGlobals->initLogic);

    if (appGlobals->watchdog)
        QBWatchdogContinue(appGlobals->watchdog, NULL);

    SvLogNotice("QBInit: minimal setup finished");
}

SvLocal void QBInitSetupFirstFrame(QBInit self)
{
    SvLogNotice("QBInit: setup first frame");

    svSettingsPushComponent("Init.settings");
    SvWidget w = svSettingsWidgetCreate(self->appGlobals->res, "Init");
    svSettingsPopComponent();
    self->window = w;
    svAppAddWindow(self->appGlobals->res, NULL, self->window, 0, 0);
    svAppForceRender(self->appGlobals->res);

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "CubiTV.QBInit", QBInitStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);
    SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(50));

    if (self->appGlobals->watchdog)
        QBWatchdogStart(self->appGlobals->watchdog, self->appGlobals->scheduler, WATCHDOG_TIMEOUT_FIRST_FRAME, NULL);
}

SvLocal void QBInitStep(void* self_)
{
    QBInit self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);


    if(!self->appGlobals->res->rendered_frames) {
        SvLogNotice("QBInit: frame not yet rendered, sleeping");
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(50));
        return;
    }


    if(!self->started) {
        svAppStop(self->appGlobals->res);
        svAppRemoveWindow(self->appGlobals->res, self->window);
        svWidgetDestroy(self->window);

        if (self->appGlobals->watchdog) {
            QBWatchdogStop(self->appGlobals->watchdog, NULL);
            QBWatchdogSetTimeout(self->appGlobals->watchdog, WATCHDOG_TIMEOUT_BEFORE_INIT, NULL);
        }

        QBInitCreateMinimalSet(self);
        QBInitStartMinimalSet(self);
        if (!self->firstBoot) {
            QBInitCreateEverything(self);
            QBInitStartEverything(self);
        }

        if (self->appGlobals->watchdog)
            QBWatchdogStart(self->appGlobals->watchdog, self->appGlobals->scheduler, WATCHDOG_TIMEOUT, NULL);

        self->started = true;
        SvFiberActivate(self->fiber);
        return;
    }

    QBInitFinish(self);
}

SvLocal SvHashTable QBInitParseWhitelistFile(AppGlobals appGlobals, SvString fileName)
{
    if (!fileName) {
        return NULL;
    }

    const char *modulesToLoad = SvStringCString(fileName);
    char *buffer = NULL;
    QBFileToBuffer(modulesToLoad, &buffer);

    SvArray modules = (SvArray) SvJSONParseString(buffer, false, NULL);
    free(buffer);
    if (modules && !SvObjectIsInstanceOf((SvObject) modules, SvArray_getType())) {
        SvLogError("%s: 'modules' is not an array", __func__);
        SVRELEASE(modules);
        modules = NULL;
    } else if (modules) {
        SvLogNotice("Using whitelist [%s]", modulesToLoad);
    } else {
        SvLogError("Could not parse whitelist [%s]", modulesToLoad);
    };

    SvHashTable whitelist = NULL;

    if (modules) {
        SvIterator it = SvArrayIterator(modules);
        SvValue val;
        while ((val = (SvValue) SvIteratorGetNext(&it))) {
            if (!SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) || !SvValueIsString(val)) {
                SvLogError("%s: one of services is not a string", __func__);
                SVTESTRELEASE(whitelist);
                whitelist = NULL;
                break;
            }

            if (!whitelist) {
                whitelist = SvHashTableCreate(100, NULL);
            }

            SvHashTableInsert(whitelist, (SvGenericObject) SvValueGetString(val), (SvGenericObject) SvValueGetString(val));
        }
        SVRELEASE(modules);
    }

    if (whitelist) {
        const char *path = SvStringCString(fileName);
        const char *name = strrchr(path, '/') ? strrchr(path, '/') + 1 : path;
        SvLogNotice("QBInit: using whitelist of services from file %s", name);
    }

    return whitelist;
}

SvLocal SvHashTable QBInitCreateStandbyAgentPlugins(QBInit self)
{
    AppGlobals appGlobals = self->appGlobals;
    SvString pluginsFileName = QBInitLogicGetStandbyAgentPluginsFileName(appGlobals->initLogic);
    SvHashTable whitelist = QBInitParseWhitelistFile(appGlobals, pluginsFileName);
    SVTESTRELEASE(pluginsFileName);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBNoInputStandbyPlugin"))) {
        SvLogNotice("%s: adding QBNoInputStandbyPlugin", __func__);
        appGlobals->noInputStandbyPlugin = QBNoInputStandbyPluginCreate(appGlobals, NULL);
        if (appGlobals->noInputStandbyPlugin) {
            QBStandbyAgentAddService(appGlobals->standbyAgent, (SvGenericObject) appGlobals->noInputStandbyPlugin);
        }
    }

    return whitelist;
}

SvLocal SvPlayerConfig QBInitCreatePlayerConfig(QBInit self)
{
    SvPlayerConfig playerConfig = NULL;
    SvData settingsData = NULL;
    SvHashTable settings = NULL;

    SvString playerConfigFilePath = QBInitLogicGetPlayerConfigFileName(self->appGlobals->initLogic);

    if (!playerConfigFilePath) {
        SvLogError("%s: PlayerConfigFilePath is null.", __func__);
        goto fini;
    }

    settingsData = SvDataCreateFromFile(SvStringCString(playerConfigFilePath), -1, NULL);

    if (!settingsData) {
        SvLogError("%s: Couldn't load file: %s", __func__, SvStringCString(playerConfigFilePath));
        goto fini;
    }

    settings = (SvHashTable) SvJSONParseData(SvDataGetBytes(settingsData), SvDataGetLength(settingsData), true, NULL);

    if (!settings || !SvObjectIsInstanceOf((SvObject) settings, SvHashTable_getType())) {
        SvLogError("%s: Player config file has invalid format", __func__);
        goto fini;
    }

    playerConfig = SvPlayerConfigCreateFromSettings(settings);

    if (!playerConfig) {
        SvLogError("%s: Couldn't load player configuration", __func__);
        goto fini;
    }

fini:
    SVTESTRELEASE(playerConfigFilePath);
    SVTESTRELEASE(settingsData);
    SVTESTRELEASE(settings);

    assert(playerConfig);
    return playerConfig;
}

SvLocal void QBInitReadApplicationControllerConfig(QBApplicationControllerParams params)
{
    assert(params);
    char *buffer = NULL;
    params->windowTypesToRemember = NULL;

    SvString fileName = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/applicationControllerConf", SvGetRuntimePrefix());
    if (unlikely(!fileName))
        return;
    SvArray windowTypes = NULL;
    if (QBFileToBuffer(SvStringCString(fileName), &buffer) >= 0) {
        windowTypes = (SvArray) SvJSONParseString(buffer, false, NULL);
        free(buffer);
    }
    SVRELEASE(fileName);

    if (windowTypes && !SvObjectIsInstanceOf((SvObject) windowTypes, SvArray_getType())) {
        SvLogError("%s: windowsTypes is not an array", __func__);
        SVRELEASE(windowTypes);
        windowTypes = NULL;
    }

    if (windowTypes) {
        SvIterator it = SvArrayIterator(windowTypes);
        SvValue val;
        while ((val = (SvValue) SvIteratorGetNext(&it))) {
            if (!SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) || !SvValueIsString(val)) {
                SvLogError("%s One of windowType is not a string",__func__);
                SVTESTRELEASE(params->windowTypesToRemember);
                params->windowTypesToRemember = NULL;
                break;
            }

            if (!params->windowTypesToRemember)
                params->windowTypesToRemember = SvArrayCreate(NULL);

            SvArrayAddObject(params->windowTypesToRemember, (SvGenericObject) SvValueGetString(val));
        }
        SVRELEASE(windowTypes);
    }

    if (params->windowTypesToRemember) {
        SvLogNotice("QBInit: Read list of windows to remember");
    } else {
        SvLogWarning("QBInit: Didn't read list of windows to remember");
    }
}

SvLocal void QBInitMkdirIfNotExists(const char* path)
{
    struct stat st;
    if (stat(path, &st) < 0) {
        if (errno != ENOENT) {
            SvLogError("stat('%s') failed: %s", path, strerror(errno));
            return;
        }
        if (mkdir(path, 0755) < 0) {
            SvLogError("mkdir('%s', 0755) failed: %s", path, strerror(errno));
            return;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        SvLogError("'%s' is not a directory", path);
        return;
    }
}

SvLocal void QBInitCreateMinimalSet(QBInit self)
{
    AppGlobals appGlobals = self->appGlobals;
    SvScheduler scheduler = appGlobals->scheduler;
    SvApplication app = appGlobals->res;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();

    initLangMaps();

    SvString servicesFileName = QBInitLogicGetServiceListFileName(appGlobals->initLogic);
    SvHashTable whitelist = QBInitParseWhitelistFile(appGlobals, servicesFileName);
    SVTESTRELEASE(servicesFileName);

    SvLogNotice("QBInit: creating minimal set");

    QBInitMkdirIfNotExists("/etc/vod/app");

    QBServiceRegistryRegisterServiceType(registry, SVSTRING("QBAppStateService"), QBAppStateService_getType(), NULL, NULL);
    appGlobals->appState = SVRETAIN(QBServiceRegistryGetService(registry, SVSTRING("QBAppStateService")));
    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_initializing);

    SvLogNotice("Creating application controller");
    QBFocusManager fm = QBFocusManagerCreate(app);
    QBLocalWindowManager lm = QBLocalWindowManagerCreate(app, fm);
    svSettingsPushComponent("GlobalWindowManager.settings");
    QBGlobalWindowManager gm = QBGlobalWindowManagerCreate(app, "GlobalWindowManager", fm);
    svSettingsPopComponent();

    QBContextSwitcherStandard switcher = (QBContextSwitcherStandard) SvTypeAllocateInstance(QBContextSwitcherStandard_getType(), NULL);
    svSettingsPushComponent("ContextSwitcher.settings");
    double contextSwitchTime = svSettingsGetDouble("ContextSwitcher", "contextSwitchTime", 0.0);
    QBContextSwitcherStandardInitialize(switcher, app, scheduler, gm, lm, (float) contextSwitchTime);
    svSettingsPopComponent();

    struct QBApplicationControllerParams_t params = {
       .app = app,
       .localManager = lm,
       .globalManager = gm,
       .switcher = (SvGenericObject) switcher,
       .focusManager = fm,
       .inputQueue =  QBInputServiceGetOutputQueue(NULL)
    };
    QBInitReadApplicationControllerConfig(&params);
    appGlobals->controller = QBApplicationControllerCreate(&params);
    QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) appGlobals->controller, NULL);
    appGlobals->localManager = lm;
    appGlobals->globalManager = gm;
    appGlobals->switcher = (SvGenericObject) switcher;
    appGlobals->focusManager = fm;

    SvLogNotice("Creating text renderer");
    QBPropertiesMap textRendererPropertiesMap = QBPropertiesMapCreate(NULL);
    QBPropertiesMapSetIntProperty(textRendererPropertiesMap,
                                  SVSTRING("TextRendererCacheSize"),
                                  QBPlatformLogicGetTextRendererCacheSize(),
                                  NULL);
    QBPropertiesMapSetIntProperty(textRendererPropertiesMap,
                                  SVSTRING("TextRendererPersistentCacheSize"),
                                  QBPlatformLogicGetTextRendererCacheSize(),
                                  NULL);
    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBTextRenderer"),
                                         QBTextRenderer_getType(),
                                         textRendererPropertiesMap,
                                         NULL);
    SVRELEASE(textRendererPropertiesMap);
    appGlobals->textRenderer = SVRETAIN(QBServiceRegistryGetService(registry, SVSTRING("QBTextRenderer")));

    SvLogNotice("Creating dual output handler");
    //TODO We should use QBServiceRegistryRegisterServiceType, when SvApplication will be either a service or SvObject.
    QBDualOutputHandler dualOutputHandler = QBDualOutputHandlerCreate(appGlobals->res,
                                                                      appGlobals->appState);
    QBServiceRegistryRegisterService(registry, (SvObject) dualOutputHandler, NULL);
    SVRELEASE(dualOutputHandler);

    SvLogNotice("Creating volume service");
    appGlobals->volume = QBVolumeCreate();
    QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) appGlobals->volume, NULL);
    appGlobals->volumeWidget = QBVolumeWidgetCreate(appGlobals->res, NULL);

    SvLogNotice("Creating tuner logic");
    appGlobals->tunerLogic = QBTunerLogicCreate(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBTunerLogic_getType(), (SvObject) appGlobals->tunerLogic, NULL);

    SvLogNotice("Checking HYBRID mode");
    if (!QBTunerLogicGetTunerCount(appGlobals->tunerLogic)) {
        const char *currentHybridMode = QBConfigGet("HYBRIDMODE");
        if (strcmp(currentHybridMode, "IP")) {
            SvLogWarning("No available tuners and HYBRIDMODE=%s. Switching to IP", currentHybridMode);
            QBConfigSet("HYBRIDMODE", "IP");
            QBConfigSave();
        }
    }

    SvLogNotice("Initializing DRM");
    QBDRMInit(appGlobals);

    SvLogNotice("Creating wizard logic");
    appGlobals->wizardLogic = QBWizardLogicCreate(appGlobals);

    SvLogNotice("Creating standby agent and plugins");
    appGlobals->standbyAgent = QBStandbyAgentCreate(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBStandbyAgent_getType(), (SvObject) appGlobals->standbyAgent, NULL);
    SvHashTable standbyAgentPluginsWhitelist = QBInitCreateStandbyAgentPlugins(self);

    SvLogNotice("Creating standby logic");
    appGlobals->standbyLogic = QBStandbyLogicCreate(appGlobals, standbyAgentPluginsWhitelist, NULL);
    SVTESTRELEASE(standbyAgentPluginsWhitelist);

    SvLogNotice("Creating objects cleaning service");
    appGlobals->cleaner = QBCleanerCreate(scheduler, appGlobals->appPool);

    // Upgrade Watcher needs it
    SvLogNotice("Creating QBDVBMuxesMap");
    appGlobals->dvbMuxesMap = QBDVBMuxesMapCreate("/etc/vod/ota-muxes", NULL);

    // upgrade watcher needs upgrade logic
    SvLogNotice("Creating upgrade watcher service");
    appGlobals->upgradeWatcher = UpgradeWatcherCreate(appGlobals);

    SvLogNotice("Creating video format monitor");
    appGlobals->videoFormatMonitor = QBVideoFormatMonitorCreate(QBViewportGet(), 1000);
    QBServiceRegistryRegisterService(registry, (SvObject) appGlobals->videoFormatMonitor, NULL);

    SvLogNotice("Creating aspect ratio agent");
    VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(registry, SVSTRING("VideoOutputLogic"));
    bool separatedOutputConfiguration = videoOutputLogic && VideoOutputLogicIsOutputConfigurationSeparated(videoOutputLogic);
    QBAspectRatioAgent aspectRatioAgent = QBAspectRatioAgentCreate(self->appGlobals->videoFormatMonitor, separatedOutputConfiguration);
    QBServiceRegistryRegisterService(registry, (SvObject) aspectRatioAgent, NULL);
    SVRELEASE(aspectRatioAgent);

    SvLogNotice("Creating audio output logic");
    SvObject audioOutputLogic = (SvObject) AudioOutputLogicCreate(NULL);
    QBServiceRegistryRegisterService(registry, audioOutputLogic, NULL);
    SVRELEASE(audioOutputLogic);

    SvLogNotice("Creating HDMI output standard service");
    appGlobals->outputStandardService = QBOutputStandardServiceCreate(appGlobals->scheduler, QBViewportGet(), videoOutputLogic);

    SvLogNotice("Creating HDMI service");
    SvObject hdmiService = (SvObject) HDMIServiceCreate(appGlobals);
    QBServiceRegistryRegisterService(registry, hdmiService, NULL);
    SVRELEASE(hdmiService);

    SvLogNotice("Creating GUI logic");
    appGlobals->guiLogic = QBGUILogicCreate(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBGUILogic_getType(), (SvObject) appGlobals->guiLogic, NULL);

    SvLogNotice("Creating network monitor");
    appGlobals->networkMonitor = QBNetworkMonitorCreate(appGlobals->scheduler);

    SvLogNotice("Creating network logic");
    bool createQBNetManager = !whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBNetManager"));
    appGlobals->networkLogic = QBNetworkLogicCreate(appGlobals->res,
                                                    appGlobals->scheduler,
                                                    appGlobals->initLogic,
                                                    appGlobals->controller,
                                                    appGlobals->textRenderer,
                                                    appGlobals->networkMonitor,
                                                    createQBNetManager ? &appGlobals->net_manager : NULL,
                                                    NULL);

    SvLogNotice("Creating upgrade logic");
    appGlobals->upgradeLogic = QBUpgradeLogicCreate(appGlobals);

    SvLogNotice("Creating input watcher service");
    appGlobals->inputWatcher = QBInputWatcherCreate(scheduler);

    SvLogNotice("Creating hint manager");
    appGlobals->hintManager = QBHintManagerCreate(appGlobals->res, appGlobals->controller, scheduler, appGlobals->inputWatcher);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBHintManager_getType(), (SvObject) appGlobals->hintManager, NULL);

    if (appGlobals->hintManager) {
        SvLogNotice("Initializing hints"); //hint manager is necessary for having hints available in QBQWizard
        SvString hintPath = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/hints/hints.json", SvGetRuntimePrefix());
        QBHintManagerLoadHints(appGlobals->hintManager, hintPath);
        SVRELEASE(hintPath);
    }

    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBTunerMonitor"),
                                         QBTunerMonitor_getType(),
                                         NULL,
                                         NULL);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBAppLogs"))) {
        SvLogNotice("Creating logs services");
        SvString path = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/appLogs", SvGetRuntimePrefix());
        QBAppLogsCreate(appGlobals, path);
        SVRELEASE(path);
        appGlobals->appLogsInitialized = true;
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBSmartCardMonitor"))) {
        SvLogNotice("Creating SmartCardMonitor service");
        appGlobals->smartCardMonitor = QBSmartCardMonitorCreate();
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBMiddlewareManager"))) {
        if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBRedirProxyManager"))) {
            SvLogNotice("Initializing redirection proxy manager");
            appGlobals->redirProxyManager = QBRedirProxyManagerCreate(appGlobals,
                                                                      60 * 1000,            // initialRetryIntervalMs = 60s
                                                                      2 * 60 * 60 * 1000,   // maxRetryIntervalMs = 2h
                                                                      NULL);
        }

        SvLogNotice("Initializing middleware manager");
        const char* mwPrefix = QBConfigGet("PREFIX");
        SvString cmdLineUrl = SvStringCreate(mwPrefix ? mwPrefix : "", NULL);
        SvString prefixUrl = appGlobals->redirProxyManager ? QBRedirProxyManagerGetMiddlewareUrl(appGlobals->redirProxyManager) : NULL;
        QBMiddlewareManagerInitParams middlewareManagerParams = {
                .redirProxyEnabled = (appGlobals->redirProxyManager != NULL),
                .prefixUrl = prefixUrl,
                .cmdLineUrl = cmdLineUrl,
                .sslParams = appGlobals->sslParams
        };
        appGlobals->middlewareManager = QBMiddlewareManagerCreate();
        QBMiddlewareManagerInit(appGlobals->middlewareManager, &middlewareManagerParams);
        SVRELEASE(cmdLineUrl);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBMWConfigMonitor"))) {
        SvLogNotice("Initializing QBMWConfigMonitor");
        QBMWConfigMonitorRetryPolicy policy = {
            .initialRetryIntervalMs = 5 * 1000,
            .maxRetryIntervalMs = 1 * 60 * 60 * 1000,
            .requestsIntervalMs = 2 * 60 * 60 * 1000
        };
        appGlobals->qbMWConfigMonitor = QBMWConfigMonitorCreate(appGlobals, policy, NULL);
        if (appGlobals->qbMWConfigMonitor) {
            appGlobals->qbMWConfigHandler = QBMWConfigHandlerCreate(appGlobals);
            // QBMWConfigHandlerPlugins are added in the InitLogic
        }
    }

    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBRCUPairingService"),
                                         QBRCUPairingService_getType(),
                                         NULL,
                                         NULL);

    QBInitLogicMinimalPostInit(appGlobals->initLogic, whitelist);
    SVTESTRELEASE(whitelist);

    SvLogNotice("QBInit: finished creating minimal set");
}

SvLocal void QBInitStartMinimalSet(QBInit self)
{
    AppGlobals appGlobals = self->appGlobals;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();

    SvLogNotice("QBInit: starting minimal set");

    QBServiceRegistryStartServices(registry, appGlobals->scheduler, NULL);
    QBServiceRegistrySaveState(registry, SVSTRING("MinimalSet"), NULL);

    QBOutputStandardServiceStart(appGlobals->outputStandardService);

    QBGUILogicStart(appGlobals->guiLogic);

    if (appGlobals->networkMonitor)
        QBNetworkMonitorStart(appGlobals->networkMonitor);

    QBNetworkLogicStart(appGlobals->networkLogic);

    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerStart(self->appGlobals->middlewareManager, self->appGlobals->smartCardMonitor);
    }

    if (self->appGlobals->qbMWConfigHandler) {
        QBMWConfigHandlerStart(self->appGlobals->qbMWConfigHandler);
    }

    if (appGlobals->hintManager) {
        QBHintManagerStart(appGlobals->hintManager);
    }

    SvLogNotice("QBInit: finished starting minimal set");
}

SvLocal void QBInitAddProvider(AppGlobals appGlobals,
                               SvString vodServiceType,
                               QBActiveTreeNode menuNode,
                               bool configVODServiceTypeIsCubimw,
                               bool configVODServiceTypeIsTraxis,
                               bool configVODServiceTypeIsMWClient,
                               bool configWEBServiteTypeIsCubimw)
{
    const bool vodServiceTypeIsConfig = SvStringEqualToCString(vodServiceType, "CONFIG");
    if (configVODServiceTypeIsCubimw && (SvStringEqualToCString(vodServiceType, "cubimw") || vodServiceTypeIsConfig)) {
        QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_INNOV8ON);
    } else if (configVODServiceTypeIsTraxis && (SvStringEqualToCString(vodServiceType, "traxis") || vodServiceTypeIsConfig)) {
        QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_TRAXIS);
    } else if (configVODServiceTypeIsMWClient && (SvStringEqualToCString(vodServiceType, "digitalsmiths") || vodServiceTypeIsConfig)) {
        QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_MWClient);
    } else if (SvStringEqualToCString(vodServiceType, "DLNA")) {
        QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_DLNA);
    } else if (SvStringEqualToCString(vodServiceType, "MStore")) {
        QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_MSTORE);
    } else if (SvStringEqualToCString(vodServiceType, "DSMCC")) {
        QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_DSMCC);
    } else {
        SvLogWarning("%s(): unknown VOD service type %s", __func__, SvStringCString(vodServiceType));
    }
    SvString webServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("WebServiceType"));
    if (webServiceType && SvObjectIsInstanceOf((SvObject) webServiceType, SvString_getType())) {
        const bool webServiceTypeIsConfig = SvStringEqualToCString(webServiceType, "CONFIG");

        if (configWEBServiteTypeIsCubimw && (SvStringEqualToCString(webServiceType, "cubimw") || webServiceTypeIsConfig)) {
            QBProvidersControllerServiceAddProvider(appGlobals->providersController, QBProvidersControllerService_INNOV8ON);
        } else {
            SvLogWarning("%s(): unknown web service type %s", __func__, SvStringCString(webServiceType));
        }
    }
}

SvLocal void QBInitAddProviders(AppGlobals appGlobals)
{
    if (!appGlobals->providersController)
        return;

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, appGlobals->menuTree, getIterator, NULL, 0);
    QBActiveTreeNode menuNode = NULL;

    const char *configVODServiceType = QBConfigGet("PROVIDERS.VOD");
    const bool configVODServiceTypeIsCubimw = (!configVODServiceType || strcmp(configVODServiceType, "cubimw") == 0);
    const bool configVODServiceTypeIsTraxis = (configVODServiceType && strcmp(configVODServiceType, "traxis") == 0);
    const bool configVODServiceTypeIsMWClient = (!configVODServiceType || strcmp(configVODServiceType, "digitalsmiths") == 0);
    const char *configWEBServiteType = QBConfigGet("PROVIDERS.WEBSERVICES");
    const bool configWEBServiteTypeIsCubimw = (!configWEBServiteType || strcmp(configWEBServiteType, "cubimw") == 0);

    while ((menuNode = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        SvString vodServiceType = (SvString) QBActiveTreeNodeGetAttribute(menuNode, SVSTRING("VODServiceType"));
        if (!vodServiceType || !SvObjectIsInstanceOf((SvObject) vodServiceType, SvString_getType()))
            continue;

        if (SvStringEqualToCString(vodServiceType, "Composite")) {
            SvIterator childIterator = QBActiveTreeNodeChildNodesIterator(menuNode);
            QBActiveTreeNode childNode = NULL;
            while ((childNode = (QBActiveTreeNode) SvIteratorGetNext(&childIterator))) {
                SvString vodType = (SvString) QBActiveTreeNodeGetAttribute(childNode, SVSTRING("VODServiceType"));
                if (!vodType || !SvObjectIsInstanceOf((SvObject) vodType, SvString_getType()))
                    continue;
                QBInitAddProvider(appGlobals,
                                  vodType,
                                  childNode,
                                  configVODServiceTypeIsCubimw,
                                  configVODServiceTypeIsTraxis,
                                  configVODServiceTypeIsMWClient,
                                  configWEBServiteTypeIsCubimw);
            }
        } else {
            QBInitAddProvider(appGlobals,
                              vodServiceType,
                              menuNode,
                              configVODServiceTypeIsCubimw,
                              configVODServiceTypeIsTraxis,
                              configVODServiceTypeIsMWClient,
                              configWEBServiteTypeIsCubimw);
        }
    }
}

SvLocal void QBInitSetupChannelMetaStorage(QBChannelMetaStorage *channelMetaStorage, SvObject DVBPlugin, SvObject IPPlugin)
{
    if(DVBPlugin) {
        const int DVBPluginID = SvInvokeInterface(SvEPGChannelListPlugin, DVBPlugin, getID);
        if (DVBPluginID >= 0) {
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("blocked"), DVBPluginID);
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("isAdult"), DVBPluginID);
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("audioLang"), DVBPluginID);
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("subsLang"), DVBPluginID);
        }
    }
    if (IPPlugin) {
        const int IPPluginID = SvInvokeInterface(SvEPGChannelListPlugin, IPPlugin, getID);
        if (IPPluginID >= 0) {
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("blocked"), IPPluginID);
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("audioLang"), IPPluginID);
            QBChannelMetaStorageAddKeyForPlugin(channelMetaStorage, SVSTRING("subsLang"), IPPluginID);
        }
    }
}

SvLocal void QBInitCreateEverything(QBInit self)
{
    AppGlobals appGlobals = self->appGlobals;
    SvScheduler scheduler = appGlobals->scheduler;

    SvString servicesFileName = QBInitLogicGetServiceListFileName(appGlobals->initLogic);
    SvHashTable whitelist = QBInitParseWhitelistFile(appGlobals, servicesFileName);
    SVTESTRELEASE(servicesFileName);

    SvLogNotice("QBInit: creating everything");

    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_initializing);
    QBServiceRegistry registry = QBServiceRegistryGetInstance();

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("CableModemMonitor"))) {
        SvLogNotice("Creating cable modem");
        appGlobals->cableModem = (QBCableModemMonitor) QBCableModemMonitorCreate(scheduler, 5000);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBNATMonitor"))) {
        SvLogNotice("Initializing NAT monitor");
        if (appGlobals->net_manager)
            appGlobals->natMonitor = QBNATMonitorCreate(appGlobals->net_manager);
    }

    SvLogNotice("Creating and initializing access control");
    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBAccessManager"),
                                         QBAccessManager_getType(),
                                         NULL,
                                         NULL);
    appGlobals->accessMgr = SVRETAIN(QBServiceRegistryGetService(registry, SVSTRING("QBAccessManager")));

    SvLogNotice("Creating epg text renderer");
    QBPropertiesMap epgTextRendererPropertiesMap = QBPropertiesMapCreate(NULL);
    QBPropertiesMapSetIntProperty(epgTextRendererPropertiesMap, SVSTRING("maxMemoryUsage"),
                                  QBPlatformLogicGetEPGTextRendererCacheSize(), NULL);
    QBPropertiesMapSetIntProperty(epgTextRendererPropertiesMap, SVSTRING("channelsCount"), 200, NULL);
    QBServiceRegistryRegisterServiceType(registry, SVSTRING("SvEPGTextRenderer"), SvEPGTextRenderer_getType(),
                                         epgTextRendererPropertiesMap, NULL);
    SVRELEASE(epgTextRendererPropertiesMap);

    SvLogNotice("Creating lang preferences service");
    appGlobals->langPreferences = QBLangPreferencesCreate(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBLangPreferences_getType(), (SvObject) appGlobals->langPreferences, NULL);

    SvString ccConfig = QBInitLogicGetClosedCaptioningOptionsFileName(appGlobals->initLogic);
    QBLangPreferencesLoadClosedCaptioningConf(appGlobals->langPreferences, ccConfig);
    SVRELEASE(ccConfig);

    SvLogNotice("Creating main databucket");
    appGlobals->dataBucket = SvDataBucketCreate(NULL);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("CubiwareMWCustomerInfo"))) {
        SvLogNotice("Initializing customer info monitor");
        appGlobals->customerInfoMonitor = CubiwareMWCustomerInfoCreate(appGlobals->middlewareManager, 2 * 60 * 60);
        CubiwareMWCustomerInfoSetWorkingMode(appGlobals->customerInfoMonitor, false);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBFileBrowserMounter"))) {
        SvLogNotice("Creating file browser mounter");
        appGlobals->fileBrowserMounter = (QBFileBrowserMounter)QBFileBrowserMounterCreate();
    }

    int tunerCnt = QBTunerLogicGetTunerCount(appGlobals->tunerLogic);

    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBTunerReserver"),
                                         QBTunerReserver_getType(),
                                         NULL,
                                         NULL);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBSTBModeMonitor"))) {
        SvLogNotice("Creating STB mode monitor service");
        appGlobals->stbModeMonitor = QBSTBModeMonitorCreate(NULL);
    }

    if (tunerCnt > 0) {
        QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(registry, SVSTRING("QBTunerReserver"));
        if (tunerReserver) {
            for (int i = 0; i < tunerCnt; i++) {
                QBTunerReserverAddTuner(tunerReserver, i);
            }
        }

        QBTunerMonitor tunerMonitor = (QBTunerMonitor) QBServiceRegistryGetService(registry, SVSTRING("QBTunerMonitor"));
        if (tunerMonitor) {
            for (int i = 0; i < tunerCnt; i++) {
                QBTunerMonitorAddTuner(tunerMonitor, i);
            }
        }
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("lstvDVBTimeDate"))) {
        SvLogNotice("Creating DVB time/date monitor");
        lstvDVBTimeDateCreate();
        appGlobals->lstvDVBTimeDateInitialized = true;
    }

    QBPropertiesMap dateTimePropertiesMap = QBPropertiesMapCreate(NULL);
    QBPropertiesMapSetBooleanProperty(dateTimePropertiesMap,
                                      SVSTRING("useNTP"),
                                      QBAppTypeIsIP() || QBAppTypeIsHybrid(),
                                      NULL);

    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBTimeDateMonitor"),
                                         QBTimeDateMonitor_getType(),
                                         dateTimePropertiesMap,
                                         NULL);
    SVRELEASE(dateTimePropertiesMap);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBHotplugNotifier"))) {
        SvLogNotice("Creating hotplug notifier");
        appGlobals->hotplugNotifier = QBHotplugNotifierCreate(appGlobals);
    }

    SvTVChannelRegisterMerger(appGlobals->dataBucket);

    if (appGlobals->middlewareManager && (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("TraxisWebSessionManager")))) {
        SvString traxisURLStr = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Traxis);
        if (traxisURLStr) {
            SvLogNotice("Creating Traxis.Web session manager");
            SvURL traxisServerURL = SvURLCreateWithString(traxisURLStr, NULL);
            appGlobals->traxisWebSessionManager = TraxisWebSessionManagerCreate(traxisServerURL, NULL);
            SVRELEASE(traxisServerURL);

            SvSSLParams sslParams = (SvSSLParams) SvTypeAllocateInstance(SvSSLParams_getType(), NULL);
            SvSSLParamsInit(sslParams, SvSSLVersion_default, NULL);
            TraxisWebSessionManagerSetSSLParams(appGlobals->traxisWebSessionManager, sslParams, NULL);
            SVRELEASE(sslParams);

            appGlobals->traxisWebMonitor = TraxisWebMonitorCreate(appGlobals->middlewareManager, appGlobals->traxisWebSessionManager, NULL);
        } else {
            SvLogWarning("Traxis.Web session manager enabled, but TRAXIS_SERVER_URL is not set");
        }
    }

    QBInitLogicCreateEPGManager(appGlobals->initLogic);
    QBInitLogicCreatePVRProvider(appGlobals->initLogic, whitelist);

    SvLogNotice("Creating parental control logic and service");
    appGlobals->parentalControlLogic = QBParentalControlLogicCreate(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(),
                           QBParentalControlLogic_getType(),
                           (SvObject) appGlobals->parentalControlLogic,
                           NULL);
    QBPropertiesMap pcPropertiesMap = QBPropertiesMapCreate(NULL);
    QBPropertiesMapSetProperty(pcPropertiesMap,
                               SVSTRING("QBParentalControlLogic"),
                               (SvObject) appGlobals->parentalControlLogic,
                               NULL);
    QBServiceRegistryRegisterServiceType(registry, SVSTRING("QBParentalControl"), QBParentalControl_getType(), pcPropertiesMap, NULL);
    SVRELEASE(pcPropertiesMap);
    appGlobals->pc = SVTESTRETAIN((QBParentalControl) QBServiceRegistryGetService(registry, SVSTRING("QBParentalControl")));

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBCustomerInfoStorage"))) {
        SvLogNotice("Initializing customer info storage");
        appGlobals->customerInfoStorage = QBCustomerInfoStorageCreate(appGlobals, NULL);
    }

    if (appGlobals->traxisWebSessionManager && (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("TraxisWebPlaybackSessionManager")))) {
        SvLogNotice("Creating Traxis.Web playback session manager");
        appGlobals->traxisWebPlaybackSessionManager = TraxisWebPlaybackSessionManagerCreate(appGlobals->traxisWebSessionManager);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBChannelStorage"))) {
        SvLogNotice("Creating channel storage service");
        QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
        appGlobals->channelStorage = QBChannelStorageCreate(SVSTRING("/etc/vod/channels/"), JSONserializer);
        QBChannelStorageAddChannelsSource(appGlobals->channelStorage, appGlobals->epgPlugin.DVB.channelList);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBChannelMetaStorage"))) {
        SvLogNotice("Creating channel meta storage service");
        appGlobals->channelMetaStorage = QBChannelMetaStorageCreate(appGlobals->channelMerger, SVSTRING("/etc/vod/app/channelMeta"));
    }

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(registry, SVSTRING("QBTunerReserver"));
    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBEPGMonitor"))) {
        if (appGlobals->epgPlugin.DVB.channelList || appGlobals->epgPlugin.IP.channelList) {
            SvLogNotice("Creating EPG monitor service");
            appGlobals->epgMonitor = QBEPGMonitorCreate(appGlobals->channelMerger, tunerReserver, tunerCnt);
            QBEPGMonitorSetReadingDuration(appGlobals->epgMonitor, 7);
        }
    }

    appGlobals->dateTimeLogic = DateTimeLogicCreate(appGlobals);
    QBInitLogicSetupMuxes(appGlobals->initLogic);

    SvLogNotice("Creating playlists manager");

    QBPropertiesMap playlistManagerPropertiesMap = QBPropertiesMapCreate(NULL);
    QBPropertiesMapSetStringProperty(playlistManagerPropertiesMap,
                                     SVSTRING("confgurationDirectory"),
                                     SVSTRING("/etc/vod/playlists/"),
                                     NULL);
    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBPlaylistManager"),
                                         QBPlaylistManager_getType(),
                                         playlistManagerPropertiesMap,
                                         NULL);
    SVRELEASE(playlistManagerPropertiesMap);
    QBPlaylistManager playlist = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));

    QBInitLogicCreatePlaylists(appGlobals->initLogic);
    QBPlaylistManagerLoad(playlist);
    QBPlaylistManagerAddGlobalPlaylist(playlist, SVSTRING("TVChannels"));
    QBPlaylistManagerAddGlobalPlaylist(playlist, SVSTRING("RadioChannels"));
    SvString favoritesMenuFile = QBInitLogicCreateFavoritesConfigurationFileName(appGlobals->initLogic);
    QBPlaylistManagerLoadStaticPlaylists(playlist, favoritesMenuFile, NULL, NULL);
    SVTESTRELEASE(favoritesMenuFile);

    SvLogNotice("Creating playlist cursors");
    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    appGlobals->playlistCursors = QBPlaylistCursorsCreate(SVSTRING("/etc/vod/app/playlistCursors"), JSONserializer);
    QBPlaylistCursorsLoad(appGlobals->playlistCursors);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBTextPreRenderer"))) {
        SvLogNotice("Creating text prerenderer");
        appGlobals->textPrerenderer = QBTextPreRendererCreate(appGlobals->textRenderer);
    }

    SvObject channelsList = QBPlaylistManagerGetById(playlist, SVSTRING("AllChannels"));

    SvLogNotice("Creating epg watcher service");

    QBPropertiesMap epgWatcherPropertiesMap = QBPropertiesMapCreate(NULL);
    QBPropertiesMapSetProperty(epgWatcherPropertiesMap,
                               SVSTRING("master"),
                               channelsList,
                               NULL);
    QBServiceRegistryRegisterServiceType(registry,
                                         SVSTRING("QBEPGWatcher"),
                                         QBEPGWatcher_getType(),
                                         epgWatcherPropertiesMap,
                                         NULL);
    SVRELEASE(epgWatcherPropertiesMap);

    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBSecretKeyCodeService"))) {
        SvLogNotice("Initializing QBSecretKeyCodeService");
        appGlobals->secretKeyCodeService = QBSecretKeyCodeServiceCreate(appGlobals->controller);
    }

    if (QBPVRLogicHasInternalStorage()) {
        SvLogNotice("Creating hdd standby manager");
        appGlobals->hddStandbyManager = QBHddStandbyManagerCreate(appGlobals);
    }

    QBInitLogicCreateAccessPlugin(self->appGlobals->initLogic);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBCASManagers"))) {
        QBCASCreateManagers(self->appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBViewRightWeb"))) {
        SvLogNotice("Creating ViewRightWeb service");
        appGlobals->viewRightWeb = QBViewRightWebServiceCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBStandByTimerTimeout"))) {
        SvLogNotice("Creating QBStandByTimerTimeout service");
        appGlobals->standByTimerTimeout = QBStandByTimerTimeoutCreate(appGlobals->scheduler, appGlobals->standbyAgent);
    }

    SvLogNotice("Creating player manager");
    SvPlayerManager playerManager = SvPlayerManagerGetInstance();
    QBDiskPvrStorageMode pvrStorageMode = QBDiskPvrStorageModeGet();
    bool pvrReencryptionEnabled = pvrStorageMode == QBDiskPvrStorageMode_dvbOnlyReencryption || pvrStorageMode == QBDiskPvrStorageMode_reencryption;
    SvPlayerManagerSetReencryption(playerManager, pvrReencryptionEnabled);
    SvPlayerConfig playerConfig = QBInitCreatePlayerConfig(self);

    if (playerConfig) {
        SvPlayerManagerSetPlayerConfig(playerManager, playerConfig);
        SVRELEASE(playerConfig);
    } else {
        SvLogWarning("Couldn't load player configuration, default one will be used");
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBCallerIdManager"))) {
        SvLogNotice("Creating Caller ID manager");
        appGlobals->callerIdManager = QBCallerIdManagerCreate(appGlobals, SVSTRING("/etc/vod/app/callsHistory"), JSONserializer, QB_CALLER_ID_HISTORY_SIZE);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBDSMCCService"))) {
        SvLogNotice("Initializing QBDSMCCService");
        SvString filePath = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/dsmcc", SvGetRuntimePrefix());
        appGlobals->dsmccMonitor = QBDSMCCMonitorCreateWithConfig(tunerCnt, filePath);
        SVRELEASE(filePath);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBMWProductUsageController"))) {
        SvLogNotice("Initializing QBMWProductUsageController");
        appGlobals->qbMWProductUsageController = QBMWProductUsageControllerCreate(appGlobals->middlewareManager);
    }

    SvLogNotice("Creating application logic");
    appGlobals->playerController = QBGlobalPlayerControllerCreate();
    appGlobals->tvLogic = QBTVLogicNew(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBTVLogic_getType(), (SvObject) appGlobals->tvLogic, NULL);

    appGlobals->epgLogic = QBEPGLogicNew(appGlobals);
    appGlobals->verticalEPGLogic = QBVerticalEPGLogicNew(appGlobals);
    appGlobals->searchResultsLogic = QBSearchResultsLogicNew(appGlobals);
    appGlobals->pvrLogic = QBPVRLogicNew(appGlobals);
    appGlobals->channelScanningLogic = QBChannelScanningLogicCreate(appGlobals, NULL);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBChannelScanningLogic_getType(), (SvObject) appGlobals->channelScanningLogic, NULL);
    appGlobals->operateScanningLogic = QBOperateScanningLogicCreate(appGlobals);

    AudioTrackLogic audioTrackLogic = AudioTrackLogicCreate(NULL);
    QBServiceRegistryRegisterService(registry, (SvObject) audioTrackLogic, NULL);
    SVRELEASE(audioTrackLogic);

    VideoTrackLogic videoTrackLogic = VideoTrackLogicCreate(NULL);
    QBServiceRegistryRegisterService(registry, (SvObject) videoTrackLogic, NULL);
    SVRELEASE(videoTrackLogic);

    appGlobals->mainLogic = QBMainMenuLogicNew(appGlobals);
    appGlobals->globalInputLogic = QBGlobalInputLogicCreate(appGlobals, NULL);
    appGlobals->accessLogic = QBAccessLogicCreate(appGlobals);
    appGlobals->purchaseLogic = QBPurchaseLogicCreate(appGlobals);
    appGlobals->eventISLogic = QBEventISLogicCreate(appGlobals);
    appGlobals->mstoreLogic = QBMStoreLogicCreate(appGlobals, NULL);
    appGlobals->satelliteSetupLogic = QBSatelliteSetupLogicNew(appGlobals);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), QBSatelliteSetupLogic_getType(), (SvObject) appGlobals->satelliteSetupLogic, NULL);
    appGlobals->configurationMenuLogic = QBConfigurationMenuLogicCreate(appGlobals);
    appGlobals->eventsLogic = EventsLogicCreate(appGlobals->pc, appGlobals->parentalControlLogic, appGlobals->langPreferences, NULL);
    QBGlobalStorageSetItem(QBGlobalStorageGetInstance(), EventsLogic_getType(), (SvObject) appGlobals->eventsLogic, NULL);

    appGlobals->secureLogLogic = NULL;

    QBAudioStreamDescRegisterJSONSerializers();

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBDiagnostics"))) {
        appGlobals->diagnosticsLogic = QBDiagnosticsLogicCreate(appGlobals);
    }

    SvPlayerAudioTrackRegisterJSONSerializers();

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBReminder"))) {
        SvLogNotice("Creating reminder service");
        appGlobals->reminder = QBReminderCreate(appGlobals->res, SVSTRING("/etc/vod/reminderTasks"), 180, NULL);
        QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) appGlobals->reminder, NULL);
        appGlobals->reminderAgent = QBReminderAgentCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBDVBTableMonitor"))) {
        if (appGlobals->epgPlugin.DVB.channelList) {
            SvLogNotice("Creating DVB table monitor");
            QBPropertiesMap dvbTableMonitorPropertiesMap = QBPropertiesMapCreate(NULL);
            QBPropertiesMapSetIntProperty(dvbTableMonitorPropertiesMap, SVSTRING("tunerCnt"), tunerCnt, NULL);
            QBServiceRegistryRegisterServiceType(registry, SVSTRING("QBDVBTableMonitor"), QBDVBTableMonitor_getType(),
                                                 dvbTableMonitorPropertiesMap, NULL);
            SVRELEASE(dvbTableMonitorPropertiesMap);
            appGlobals->sdtMonitor = QBSDTMonitorCreate((SvGenericObject) appGlobals->epgPlugin.DVB.channelList);
        }
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBDVBBouquets"))) {
        QBPropertiesMap dvbBouquetsProperties = QBInitLogicCreateServiceProperties(self->appGlobals->initLogic, SVSTRING("QBDVBBouquets"));
        QBServiceRegistryRegisterServiceType(registry, SVSTRING("QBDVBBouquets"), QBDVBBouquets_getType(), dvbBouquetsProperties, NULL);
        SVTESTRELEASE(dvbBouquetsProperties);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBOperateScanningManager"))) {
        QBDVBTableMonitor tableMonitor = (QBDVBTableMonitor)
            QBServiceRegistryGetService(registry, SVSTRING("QBDVBTableMonitor"));
        if (tableMonitor) {
            SvLogNotice("Creating operate scanning manager");
            SvString operateScanningConfFilePath = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/operateScanningConf", SvGetRuntimePrefix() );
            appGlobals->operateScanningManager = QBOperateScanningManagerCreate(appGlobals, SVSTRING("/etc/vod/app/scanningManager"), operateScanningConfFilePath);
            SVRELEASE(operateScanningConfFilePath);
        }
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBDVBChannelLogos"))) {
        SvLogNotice("Creating Channel Logo over DVB service");
        SvString cfg_path = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/channelLogosDSMCC", SvGetRuntimePrefix());
        appGlobals->channelLogosSetter = QBDVBChannelLogosServiceCreate(cfg_path, appGlobals->tunerLogic, playlist);
        SVRELEASE(cfg_path);
    }

    SvLogNotice("Preloading settings");
    const char *settingsToPreload[] = {
        "osd.settings", "TVguide.settings", "ContextMenu.settings",
        "extendedInfo.settings", "tvMiniGuide.settings",
        "MovieInfo.settings"
    };
    size_t i;
    for(i = 0; i < sizeof(settingsToPreload) / sizeof(settingsToPreload[0]); i++) {
        svSettingsPushComponent(settingsToPreload[i]);
        svSettingsPreloadComponent();
        svSettingsPopComponent();
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBAuthenticationService"))) {
        SvLogNotice("Initializing QBAuthenticationService");
        appGlobals->authenticationService = QBAuthenticationServiceCreate();
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBAdService"))) {
        SvLogNotice("Initializing QBAdService");
        appGlobals->adService = QBAdServiceCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBContentManagers"))) {
        SvLogNotice("Creating content managers service");
        QBServiceRegistryRegisterServiceType(registry,
                                             SVSTRING("QBContentManagersService"),
                                             QBContentManagersService_getType(),
                                             NULL, NULL);
    }

    QBInitLogicCreateContentManagersServices(appGlobals->initLogic);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBMWServiceMonitor"))) {
        SvLogNotice("Creating MW service monitor");
        QBInitLogicServiceMonitorCreate(appGlobals->initLogic);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBProvidersControllerService"))) {
        SvLogNotice("Creating providers controller service");
        appGlobals->providersController = QBProvidersControllerServiceCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBLogWindowManager"))) {
        SvLogNotice("Creating QBLogWindowManager");
        appGlobals->logWindow = QBLogWindowManagerCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBPushReceiver"))) {
        SvLogNotice("Creating QBPushReceiver");
        appGlobals->pushReceiver = QBPushReceiverCreate(appGlobals->middlewareManager, appGlobals->sslParams, 2 * 60 * 60, NULL);     // 2h
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBRemoteControl"))) {
        SvLogNotice("Initializing remote control service");
        appGlobals->remoteControl = QBRemoteControlCreate(appGlobals, NULL);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBRemoteMessagesManager"))) {
        SvLogNotice("Initializing remote messages manager service");
        appGlobals->remoteMessagesManager = QBRemoteMessagesManagerCreate(appGlobals, NULL);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBPlaybackMonitor"))) {
        SvLogNotice("Initializing playback monitor service");
        QBPlaybackMonitor playbackMonitorService = QBPlaybackMonitorCreate(appGlobals->volume, NULL);
        if (playbackMonitorService) {
            QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) playbackMonitorService, NULL);
            SVRELEASE(playbackMonitorService);
        } else {
            SvLogWarning("Couldn't create playback monitor service");
        }
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBBookmarkManager"))) {
        SvLogNotice("Initializing bookmark manager service");
        appGlobals->bookmarkManager = QBBookmarkManagerCreate(appGlobals);
        QBInitLogicCreateBookmarkPlugins(appGlobals->initLogic);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBDiskSchedulerChanger"))) {
        SvLogNotice("Initializing disk scheduler changer");
        appGlobals->diskSchedulerChanger = QBDiskSchedulerChangerCreate();
        QBDiskSchedulerChangerSetNoopSchedulerMaxDiskSize(appGlobals->diskSchedulerChanger, 64ll * 1024 * 1024 * 1024);
    }

    appGlobals->inputTranslator = QBKeyInputTranslatorCreate(appGlobals, NULL);

    SvLogNotice("Creating persistent contexts");
    appGlobals->main = QBMainMenuInitLogicContextCreate(appGlobals);

    appGlobals->newTV = QBTVLogicCreateTVContext(appGlobals->tvLogic);
    appGlobals->newTVGuide = QBNewTVGuideContextCreate(appGlobals);
    appGlobals->verticalEPG = QBVerticalEPGContextCreate(appGlobals);
    appGlobals->screensaver = QBScreensaverContextCreate(appGlobals);

    SubtitleTrackLogic subtitleTrackLogic = SubtitleTrackLogicCreate(appGlobals);
    QBServiceRegistryRegisterService(registry, (SvObject) subtitleTrackLogic, NULL);
    SVRELEASE(subtitleTrackLogic);

    SvLogNotice("Creating XMB tree data model");
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    SvRBLocator locator = QBResourceManagerGetResourceLocator(resourceManager);

    SvString menuFileName = QBMainMenuInitLogicGetMenuFileName(appGlobals);
    SvLogNotice("Using appmenu file: %s", SvStringCString(menuFileName));

    char* filePath = SvRBLocatorFindFile(locator, SvStringCString(menuFileName));
    SVRELEASE(menuFileName);

    appGlobals->menuTree = QBActiveTreeCreateFromFile(filePath, NULL);
    if (!appGlobals->menuTree)
      SvLogError("QBActiveTreeCreateFromFile(%s) -> failed!", filePath);

    free(filePath);

    appGlobals->subtreeNotifier = QBSubtreeNotifierCreate();
    SvInvokeInterface(QBMainMenu, self->appGlobals->main, addGlobalHandler, (SvObject) appGlobals->subtreeNotifier);

    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBHotplugMountAgent"))) {
        SvLogNotice("Initializing hotplug mount agent");
        appGlobals->hotplugMountAgent = QBHotplugMountAgentCreate(scheduler);
    }

    if ((!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBPVODStorage"))) && appGlobals->PVRMounter) {
        SvLogNotice("Initializing QBPVODStorage");
        appGlobals->pvodStorage = QBPVODStorageServiceCreate(appGlobals, NULL);
    }

    QBInitAddProviders(appGlobals);

    QBActiveTreeNode node = NULL;
    if ((node = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("NPvr"))) &&
        (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("npvrAgent")))) {
        SvLogNotice("Creating nPVR agent");
        appGlobals->npvrAgent = QBNPvrAgentCreate(appGlobals, 60);  //set default NPvr menu refresh delay to 60 seconds

        SvLogNotice("Creating recordings tree");
        appGlobals->recordingsTree = QBPVRRecordingsTreeCreate(appGlobals, NULL, NULL);     // use default node IDs
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBDiagnostics"))) {
        SvLogNotice("Initializing Diagnostics Manager");
        appGlobals->diagnosticsManager = QBDiagnosticsManagerCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBAppVersion"))) {
        SvLogNotice("Initializing QBAppVersion");
        appGlobals->appVersion = QBAppVersionCreate(appGlobals);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBSkinManager"))) {
        SvLogNotice("Initializing QBSkinManager");
//        appGlobals->skinManager = QBSkinManagerCreate(appGlobals);
    }

    QBStandardRastersManager standardRastersManager = NULL;
    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBStandardRastersManager"))) {
        SvLogNotice("Initializing standard rasters manager");
        standardRastersManager = QBStandardRastersManagerCreate();
        QBStandardRastersManagerLoad(standardRastersManager);
        QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
        QBGlobalStorageSetItem(globalStorage, QBStandardRastersManager_getType(),
                               (SvObject) standardRastersManager, NULL);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBChannelScanningConfManager"))) {
        SvLogNotice("Initializing channel scanning configuration");
        QBServiceRegistryRegisterServiceType(QBServiceRegistryGetInstance(),
                                             SVSTRING("QBChannelScanningConfManager"),
                                             QBChannelScanningConfManager_getType(),
                                             NULL,
                                             NULL);
        QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                      SVSTRING("QBChannelScanningConfManager"));
        SvString channelScanningConfPath = QBInitLogicGetChannelScanningConfFileName(appGlobals->initLogic);
        if (channelScanningConfPath)
            QBChannelScanningConfManagerLoadConfiguration(channelScanningConf, channelScanningConfPath);
        SVRELEASE(channelScanningConfPath);

        SvString scanDefaultParamsFileName = QBInitLogicGetDefaultScanningParamsFileName(appGlobals->initLogic);
        SvString scanUserParamsFileName = QBInitLogicGetUserScanningParamsFileName(appGlobals->initLogic);
        QBChannelScanningConfManagerLoadUserParams(channelScanningConf, scanDefaultParamsFileName, scanUserParamsFileName);
        SVRELEASE(scanDefaultParamsFileName);
        SVRELEASE(scanUserParamsFileName);
    }
    SVTESTRELEASE(standardRastersManager);

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBXHTMLWindowsService"))) {
        SvLogNotice("Initializing XHTML windows service");
        appGlobals->xhtmlWindowsService = QBXHTMLWindowsServiceCreate(appGlobals, NULL);
    }

    if (appGlobals->casPopupManager) {
        SvLogNotice("Initializing CAS Popup Visibility Service");
        appGlobals->casPopupVisabilityManager = QBCASPopupVisibilityManagerCreate(appGlobals->casPopupManager);
    }

    const char *url = QBConfigGet("EMMURL");
    const char *emmHttpVariant = QBConfigGet("EMMHTTPVARIANT");
    const char *emmEachMinTimePull = QBConfigGet("EMMEACHMINTIMEPULL");
    if ((url != NULL) && emmHttpVariant && (strcasecmp(emmHttpVariant, "Conax-without-pollconfig") == 0)) {
        if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBConaxPullEMMService"))) {
            SvLogNotice("Initializing QBConaxPullEMMService without pollconfig");
            appGlobals->conaxHttpEmm = QBConaxPullEMMServiceCreate(appGlobals->smartCardMonitor);
            QBConaxPullEMMServiceSetNotificationListener(appGlobals->conaxHttpEmm, (SvGenericObject) appGlobals->casPopupManager);
            SvString urlStr = SvStringCreate(url, NULL);
            QBConaxPullEMMServiceSetPullConfig(appGlobals->conaxHttpEmm, urlStr,
                                               rand() % (24*60*60*1000) /* random recurrent poll time */,
                                               60*1000 /* minTimeBetweenRequest - 60s is in documentations example */);
            if (emmEachMinTimePull && strcasecmp(emmEachMinTimePull, "enabled") == 0) {
                QBConaxPullEMMServiceSetSheduledPullAfterMinTime(appGlobals->conaxHttpEmm, true);
            }
            SVRELEASE(urlStr);
            QBConaxPullEMMServiceSetRequestTokenFile(appGlobals->conaxHttpEmm, SVSTRING("/etc/vod/app/ConaxPullEmmLastRequestToken.txt"));
        }
    } else if ((url != NULL) && emmHttpVariant && (strcasecmp(emmHttpVariant, "Conax") == 0)) {
        if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBConaxPullEMMService"))) {
            SvLogNotice("Initializing QBConaxPullEMMService");
            appGlobals->conaxHttpEmm = QBConaxPullEMMServiceCreate(appGlobals->smartCardMonitor);
            QBConaxPullEMMServiceSetNotificationListener(appGlobals->conaxHttpEmm, (SvGenericObject) appGlobals->casPopupManager);
            if (emmEachMinTimePull && strcasecmp(emmEachMinTimePull, "enabled") == 0) {
                QBConaxPullEMMServiceSetSheduledPullAfterMinTime(appGlobals->conaxHttpEmm, true);
            }
            SvString urlStr = SvStringCreate(url, NULL);
            QBConaxPullEMMServiceSetPullConfigServerUrl(appGlobals->conaxHttpEmm, urlStr);
            SVRELEASE(urlStr);
            QBConaxPullEMMServiceSetPullConfigFile(appGlobals->conaxHttpEmm, SVSTRING("/etc/vod/app/ConaxPullEmmLastConfig.xml"));
            QBConaxPullEMMServiceSetRequestTokenFile(appGlobals->conaxHttpEmm, SVSTRING("/etc/vod/app/ConaxPullEmmLastRequestToken.txt"));
        }
    } else if (url != NULL) {
        if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBHTTPEMMService"))) {
            SvLogNotice("Initializing QBHTTPEMMService");
            const char *emmTimeChrs = QBConfigGet("EMMREPEAT");
            const char *httpTimeChrs = QBConfigGet("EMMHTTPREPEAT");
            unsigned int emmTime, httpTime;
            if (emmTimeChrs == NULL)
                emmTime = 3600;
            else
                emmTime = (unsigned int)strtol ( emmTimeChrs, NULL, 10);

            if (httpTimeChrs == NULL)
                httpTime = 60;
            else
                httpTime = (unsigned int)strtol ( httpTimeChrs, NULL, 10);

            SvString urlStr = SvStringCreate(url, NULL);
            appGlobals->httpEmm = QBHTTPEMMServiceCreate(appGlobals->scheduler, urlStr, emmTime, httpTime);
            SVRELEASE(urlStr);
        }
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBRemoteScheduler"))) {
        SvLogNotice("Initializing QBRemoteScheduler");
        appGlobals->remoteScheduler = QBRemoteSchedulerCreate(appGlobals, 5*1000);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBDiskActivator"))) {
        SvLogNotice("Creating QBDiskActivator");
        appGlobals->diskActivator = QBDiskActivatorCreate(appGlobals->standbyAgent, appGlobals->hotplugMountAgent);
        if (appGlobals->diskActivator && appGlobals->standbyAgent) {
            QBStandbyAgentAddService(appGlobals->standbyAgent, (SvObject) appGlobals->diskActivator);
        }
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBSecureLogManager"))) {
        SvLogNotice("Initializing QBSecureLogServcie");
        SvString path = QBInitLogicCreateSecureLogConfigFileName(appGlobals->initLogic);
        appGlobals->secureLogLogic = QBSecureLogLogicCreate();
        appGlobals->secureLogService = QBSecureLogServiceCreate(appGlobals->pushReceiver, (SvObject) appGlobals->secureLogLogic, path);
        SVRELEASE(path);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject)SVSTRING("QBConnectionMonitor"))) {
        SvLogNotice("Initializing QBConnectionMonitor");
        QBConnectionMonitor connectionMonitor = QBConnectionMonitorCreate(appGlobals->scheduler);
        QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) connectionMonitor, NULL);
    }

    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBAppsManager"))) {
        SvLogNotice("Initializing QBAppsManager");
        appGlobals->appsManager = QBAppsManagerCreate();

        // QBDIALServerManager is functional only if QBAppsManager exists
        if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBDIALServerManager"))) {
            SvLogNotice("Initializing QBDIALServerManager");
            struct QBDIALServerManagerParams_ dialServerManagerParams = {
                .appsManager      = appGlobals->appsManager,
                .networkMonitor   = appGlobals->networkMonitor,
                .app              = appGlobals->res,
                .controller       = appGlobals->controller,
                .viewport         = QBViewportGet(),
                .playerController = appGlobals->playerController,
                .scheduler        = appGlobals->scheduler
            };
            appGlobals->dialServerManager = QBDIALServerManagerCreate(&dialServerManagerParams, NULL);
        }
    }

    QBPlayReadyManagerInit(NULL);
    if (!whitelist || SvHashTableFind(whitelist, (SvGenericObject) SVSTRING("QBConaxPlayReady"))) {
        SvLogNotice("Initializing QBConaxPlayReady");
        SvObject conaxPlayReady = QBConaxPlayReadyCreate(NULL);
        QBPlayReadyManagerAddImplementation(QBPlayReadyImplementation_Conax, conaxPlayReady, NULL);
        SVTESTRELEASE(conaxPlayReady);
    }

    QBInitLogicCreateMWClientServices(appGlobals);

    SvLogNotice("Creating DRM manager");
    appGlobals->DRMManager = QBDRMManagerCreate();
    QBServiceRegistryRegisterService(registry, (SvObject) appGlobals->DRMManager, NULL);

    SvLogNotice("Creating CEC");
    appGlobals->cec = QBCECCreate();

    SvLogNotice("Creating cecService");
    appGlobals->cecService = QBCecServiceCreate(appGlobals);

    QBAutoTestCreate();

    SvLogNotice("InitLogic post init");
    QBInitLogicPostInit(appGlobals->initLogic, whitelist);

    if (appGlobals->channelMetaStorage)
        QBInitSetupChannelMetaStorage(appGlobals->channelMetaStorage, appGlobals->epgPlugin.DVB.channelList, appGlobals->epgPlugin.IP.channelList);

    SVTESTRELEASE(whitelist);
    SvLogNotice("QBInit: finished creating everything");
}

SvLocal void QBInitStartEverything(QBInit self)
{
    AppGlobals appGlobals = self->appGlobals;
    SvScheduler scheduler = appGlobals->scheduler;

    SvLogNotice("QBInit: starting everything");

    QBSetupGlobalInputEventHandler(appGlobals);

    SvLogNotice("Initializing main menu");
    SvInvokeInterface(QBMainMenu, appGlobals->main, init);
    QBApplicationControllerAddPersistentContext(appGlobals->controller, (QBWindowContext) appGlobals->main);

    QBDRMStart();

    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_starting);

    QBSanityCheckerStart(appGlobals->sanityChecker);

    QBViewportSetVideoWindow(QBViewportGet(), QBViewportMode_disabled, NULL);

    QBPVRLogicStart(appGlobals->pvrLogic);

    QBStandbyLogicStart(appGlobals->standbyLogic);

    /// Import settings from conf.
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(registry, SVSTRING("VideoOutputLogic"));
    VideoOutputLogicSetVideoContentMode(videoOutputLogic, "SD", QBConfigGet("VIDEO_CONTENT_MODE_SD"));
    VideoOutputLogicSetVideoContentMode(videoOutputLogic, "HD", QBConfigGet("VIDEO_CONTENT_MODE_HD"));

    QBTunerLogicStart(appGlobals->tunerLogic);

    if (appGlobals->channelMetaStorage) {
        QBChannelMetaStorageStart(appGlobals->channelMetaStorage);
    }
    if (appGlobals->channelStorage) {
        QBChannelStorageLoad(appGlobals->channelStorage);
        SvArray channels = QBChannelStorageGetChannels(appGlobals->channelStorage);
        if (QBChannelStorageSourceIsRegistered(appGlobals->channelStorage, appGlobals->epgPlugin.DVB.channelList)) {
            SvInvokeInterface(SvEPGChannelListPlugin, appGlobals->epgPlugin.DVB.channelList, restoreChannels, channels);
        }
    }

    if (appGlobals->fingerprint) {
        QBFingerprintStart(appGlobals->fingerprint);
    }

    if (appGlobals->hotplugMountAgent)
        QBHotplugMountAgentStart(appGlobals->hotplugMountAgent, appGlobals->menuTree);

    if (appGlobals->pvodStorage)
        QBPVODStorageServiceStart(appGlobals->pvodStorage);

    if (appGlobals->pvrProvider) {
        QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) appGlobals->pvrProvider, NULL);
        if (QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type == QBPVRProviderType_disk &&
            QBPVRLogicHasInternalStorage()) {
            const char* recordFsMetaRoot = getenv("RecordFsMetaRoot");
            if (!recordFsMetaRoot)
                recordFsMetaRoot = "/shared/";
            const char* recordFsDataRoot = getenv("RecordFsDataRoot");
            if (!recordFsDataRoot)
                recordFsDataRoot = "/tmp/qb_internal_storage";

            QBRecordFSRoot* recordFsRoot = QBRecordFSRootOpen(recordFsMetaRoot, recordFsDataRoot, NULL);
            if (recordFsRoot) {
                QBDiskPVRProviderSetStorage((QBDiskPVRProvider) appGlobals->pvrProvider, recordFsRoot);
                QBAppStateServiceSetPVRDiskState(appGlobals->appState, QBPVRDiskState_pvr_ts_present);
            }
        }

        appGlobals->pvrProviderInitialized = true;
    }

    if (appGlobals->PVRMounter)
        QBPVRMounterStart(appGlobals->PVRMounter, appGlobals);

    if (appGlobals->pvrAgent)
        QBPVRAgentStart(appGlobals->pvrAgent);

    if (appGlobals->hotplugNotifier)
        QBHotplugNotifierStart(appGlobals->hotplugNotifier);

    if (appGlobals->logWindow)
        QBLogWindowManagerStart(appGlobals->logWindow);

    if (appGlobals->reminderAgent)
        QBReminderAgentStart(appGlobals->reminderAgent);

    if (appGlobals->textPrerenderer)
        QBTextPreRendererStart(appGlobals->textPrerenderer);

    if (appGlobals->epgMonitor)
        QBEPGMonitorStart(appGlobals->epgMonitor, scheduler);

    if (appGlobals->upgradeWatcher)
        UpgradeWatcherStart(appGlobals->upgradeWatcher);

    if (appGlobals->conaxMailManager)
        QBConaxMailManagerStart(appGlobals->conaxMailManager);

    if (appGlobals->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, appGlobals->casPopupManager, start);

    if (appGlobals->conaxURIManager)
        QBConaxURIManagerStart(appGlobals->conaxURIManager);

    if (appGlobals->casPVRManager)
        SvInvokeInterface(QBCASPVRManager, appGlobals->casPVRManager, start);

    if (appGlobals->cryptoguardVideoRulesManager)
        QBCryptoguardVideoRulesManagerStart(appGlobals->cryptoguardVideoRulesManager);

    if (appGlobals->cryptoguardForceTuneManager)
        QBCryptoguardForceTuneManagerStart(appGlobals->cryptoguardForceTuneManager);

    if (appGlobals->viewRightTriggerManager)
        QBViewRightTriggerManagerStart(appGlobals->viewRightTriggerManager);

    if (appGlobals->viewRightWeb)
        QBViewRightWebServiceStart(appGlobals->viewRightWeb);

    if (appGlobals->net_manager)
        QBNetManagerStart(appGlobals->net_manager, scheduler, 5000, NULL);

    if (appGlobals->natMonitor)
        QBNATMonitorStart(appGlobals->natMonitor, scheduler);

    if (appGlobals->cableModem)
        QBCableModemMonitorStart(appGlobals->cableModem);

    if (appGlobals->stbModeMonitor)
        QBSTBModeMonitorStart(appGlobals->stbModeMonitor, NULL);

    if (appGlobals->serviceMonitor)
        QBMWServiceMonitorStart(appGlobals->serviceMonitor);

    if (appGlobals->providersController)
        QBProvidersControllerServiceStart(appGlobals->providersController);

    if (appGlobals->redirProxyManager)
        QBRedirProxyManagerStart(appGlobals->redirProxyManager);

    if (appGlobals->customerInfoStorage)
        QBCustomerInfoStorageStart(appGlobals->customerInfoStorage);

    if (appGlobals->customerInfoMonitor)
        CubiwareMWCustomerInfoStart(appGlobals->customerInfoMonitor);

    if (appGlobals->channelStorage)
        QBChannelStorageStart(appGlobals->channelStorage);

    if (appGlobals->authenticationService)
        QBAuthenticationServiceStart(appGlobals->authenticationService);

    if (appGlobals->adService)
        QBAdServiceStart(appGlobals->adService);

    if (appGlobals->traxisWebSessionManager)
        TraxisWebSessionManagerStart(appGlobals->traxisWebSessionManager, scheduler, NULL);

    if (appGlobals->parentalControlLogic)
        QBParentalControlLogicStart(appGlobals->parentalControlLogic);

    if (appGlobals->traxisWebPlaybackSessionManager)
        TraxisWebPlaybackSessionManagerStart(appGlobals->traxisWebPlaybackSessionManager, scheduler);

    if (appGlobals->nPVRProvider)
        SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, start);

    SvObject path = NULL;
    if (appGlobals->npvrAgent && (path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("NPvr")))) {
        QBNPvrAgentStart(appGlobals->npvrAgent, appGlobals->menuTree, path);
        SVRELEASE(path);
    }

    if (appGlobals->cutvManager)
        CutvManagerStart(appGlobals->cutvManager, scheduler);


    QBInputWatcherStart(appGlobals->inputWatcher);

    QBCleanerStart(appGlobals->cleaner);

    if (appGlobals->appLogsInitialized)
        QBAppLogsStart();

    if (appGlobals->smartCardMonitor)
        QBSmartCardMonitorStart(appGlobals->smartCardMonitor);

    if (appGlobals->lstvDVBTimeDateInitialized) {
        int tunerCnt = QBTunerLogicGetTunerCount(appGlobals->tunerLogic);
        lstvDVBTimeDateStart(tunerCnt);
    }

    if (appGlobals->sdtMonitor)
        QBSDTMonitorStart(appGlobals->sdtMonitor);

    if (appGlobals->operateScanningManager)
        QBOperateScanningManagerStart(appGlobals->operateScanningManager);

    if(appGlobals->httpEmm)
        QBHTTPEMMServiceStart(appGlobals->httpEmm);

    if (appGlobals->conaxHttpEmm) {
        QBConaxPullEMMServiceStart(appGlobals->conaxHttpEmm, appGlobals->scheduler);
    }

    if (appGlobals->inputTranslator) {
        SvString confFile = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/InputTranslate.json", SvGetRuntimePrefix());
        QBKeyInputTranslatorStart(appGlobals->inputTranslator, confFile, NULL);
        SVRELEASE(confFile);
    }

    if (appGlobals->standByTimerTimeout) {
        QBStandByTimerTimeoutStart(appGlobals->standByTimerTimeout);
    }

    if (appGlobals->fileBrowserMounter) {
        QBFileBrowserMounterStart(appGlobals->fileBrowserMounter, appGlobals);
    }

//    if (appGlobals->skinManager) {
//        QBSkinManagerStart(appGlobals->skinManager);
//    }

    if (appGlobals->appVersion) {
        QBAppVersionStart(appGlobals->appVersion);
    }

    if (appGlobals->pushReceiver) {
        QBPushReceiverStart(appGlobals->pushReceiver,
                            appGlobals->scheduler,
                            NULL);
    }

    if (appGlobals->remoteControl) {
        QBRemoteControlStart(appGlobals->remoteControl, NULL);
    }

    if (appGlobals->remoteMessagesManager) {
        QBRemoteMessagesManagerStart(appGlobals->remoteMessagesManager, NULL);
    }

    if (appGlobals->diskSchedulerChanger) {
        QBDiskSchedulerChangerStart(appGlobals->diskSchedulerChanger, appGlobals->hotplugMountAgent);
    }

    if (appGlobals->callerIdManager) {
        QBCallerIdManagerStart(appGlobals->callerIdManager, NULL);
    }

    if (appGlobals->bookmarkManager) {
        QBBookmarkManagerStart(appGlobals->bookmarkManager,
                               appGlobals->scheduler);
    }

    if (appGlobals->remoteScheduler) {
        QBRemoteSchedulerStart(appGlobals->remoteScheduler,
                appGlobals->scheduler);
    }

    if (appGlobals->reencryptionService) {
        QBReencryptionServiceStart(appGlobals->reencryptionService);
    }

    if (appGlobals->qbMWConfigMonitor) {
        QBMWConfigMonitorStart(appGlobals->qbMWConfigMonitor);
    }

    if (appGlobals->qbMWConfigHandler) {
        QBMWConfigHandlerStart(appGlobals->qbMWConfigHandler);
    }

    if (appGlobals->epgLogic) {
        QBEPGLogicStart(appGlobals->epgLogic);
    }

    if (appGlobals->appsManager) {
        QBAppsManagerStart(appGlobals->appsManager);
    }

    if (appGlobals->dialServerManager) {
        QBDIALServerManagerStart(appGlobals->dialServerManager, NULL);
    }

    if (appGlobals->dsmccMonitor) {
        QBDSMCCMonitorStart(appGlobals->dsmccMonitor);
        appGlobals->adsDSMCCDataProvider = QBDSMCCDataProviderCreate(QBDSMCCMonitorGetClient(appGlobals->dsmccMonitor), SVSTRING("adverts"));
    }

    if (appGlobals->channelLogosSetter) {
        QBDVBChannelLogosServiceStart(appGlobals->channelLogosSetter);
    }

    if (appGlobals->xhtmlWindowsService) {
        QBXHTMLWindowsServiceStart(appGlobals->xhtmlWindowsService);
    }

    QBPlaylistCursorsStart(appGlobals->playlistCursors);

    QBMainMenuLogicStart(appGlobals->mainLogic);

    QBConfigurationMenuLogicStart(appGlobals->configurationMenuLogic);

    QBPlaylistCursorsStart(appGlobals->playlistCursors);

    CubiwareMWClientServiceStart(appGlobals->middlewareClientService);

    QBAccessLogicStart(appGlobals->accessLogic);
    QBTVLogicStart(appGlobals->tvLogic);

    if (appGlobals->diagnosticsLogic) {
        QBDiagnosticsLogicStart(appGlobals->diagnosticsLogic);
    }

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);
    if (satellitesDB)
        QBDVBSatellitesDBLoad(satellitesDB, appGlobals->scheduler, NULL);

    QBDebugUtilsListObjectsStart(appGlobals->scheduler, appGlobals->res);
    QBDebugUtilsStartMemoryTrace(appGlobals->scheduler);

    if (appGlobals->upgradeLogic)
        QBUpgradeLogicStart(appGlobals->upgradeLogic);

    if (appGlobals->cec)
        QBCECStart(appGlobals->cec);

    if (appGlobals->cecService)
        QBCecServiceStart(appGlobals->cecService);

    if (appGlobals->secretKeyCodeService) {
        QBSecretKeyCodeServiceStart(appGlobals->secretKeyCodeService);
    }

    if (appGlobals->diskActivator) {
        QBDiskActivatorStart(appGlobals->diskActivator);
    }

    if (appGlobals->secureLogService) {
        QBSecureLogServiceStart(appGlobals->secureLogService, appGlobals->scheduler, NULL);
    }

    if (appGlobals->secureLogLogic) {
        struct QBSecureLogLogicParams_ params = {
            .net_manager = appGlobals->net_manager,
            .smartCardMonitor = appGlobals->smartCardMonitor,
            .middlewareManager = appGlobals->middlewareManager,
            .appVersion = appGlobals->appVersion,
            .standbyAgent = appGlobals->standbyAgent,
            .controller = appGlobals->controller,
            .networkMonitor = appGlobals->networkMonitor,
        };
        QBSecureLogLogicStart(appGlobals->secureLogLogic, &params);

        QBInitRebootReasonLog();
    }

    QBInitRebootReasonStore(QB_REBOOT_REASON_UNKNOWN); // cleanup last reboot reason

    QBConaxPlayReadyStart(QBPlayReadyManagerGetImplementation(QBPlayReadyImplementation_Conax, NULL), NULL);

    QBInitLogicPostStart(appGlobals->initLogic);

    SVTESTRELEASE(self->lastLoadedConfiguration);
    self->lastLoadedConfiguration = QBInitLogicGetServiceListFileName(appGlobals->initLogic);

    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_working);

    SvLogNotice("QBInit: finished starting everything");
}

SvLocal void QBInitLoadAllServicesStep(void* self_)
{
     QBInit self = self_;
     SvFiberDeactivate(self->loadingFiber);
     SvFiberEventDeactivate(self->loadingTimer);

     if (QBApplicationControllerIsSwitchInProgress(self->appGlobals->controller)) {
         SvLogNotice("QBInitLogic: frame not yet rendered, sleeping");
         SvFiberTimerActivateAfter(self->loadingTimer, SvTimeFromMs(50));
         return;
     }

     SvFiberDestroy(self->loadingFiber);
     self->loadingFiber = NULL;

     if (!self->lastLoadedConfiguration || self->confChanged) {
         QBInitCreateEverything(self);
         QBInitStartEverything(self);
     }

     if (QBTunerLogicNeedsTunersReinit(self->appGlobals->tunerLogic)) {
         const char* tunerType = QBConfigGet("DEFAULTTUNERTYPE");
         if (tunerType) {
             if (self->appGlobals->watchdog)
                 QBWatchdogContinue(self->appGlobals->watchdog, NULL);

             QBTunerLogicSwitchAllTuners(self->appGlobals->tunerLogic, tunerType);
         }

         QBAppStateServiceSetAppState(self->appGlobals->appState, QBAppState_working);
     }

     QBApplicationControllerPopContext(self->appGlobals->controller);;
     QBWizardLogicLoadingAllServicesFinished(self->appGlobals->wizardLogic);
}

SvLocal void QBInitFinish(QBInit self)
{
    AppGlobals appGlobals = self->appGlobals;
    svAppSetupMainLoop(appGlobals->res, appGlobals->scheduler);
    svAppSetupInput(appGlobals->res, NULL);     // input will be handled by the Application Controller


    if (appGlobals->watchdog)
        QBWatchdogSetTimeout(appGlobals->watchdog, WATCHDOG_TIMEOUT, NULL);

    SvLogNotice("QBInit: finished init");

    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_working);

    if (self->firstBoot)
        QBInitLogicEnterWizard(appGlobals->initLogic);
    else
        QBInitLogicEnterApplication(appGlobals->initLogic);

    if (appGlobals->standbyAgent)
        QBStandbyAgentStart(appGlobals->standbyAgent, appGlobals->scheduler);

    SvFiberDestroy(self->fiber);
}

void QBInitDestroy(QBInit self)
{
    SVTESTRELEASE(self->lastLoadedConfiguration);
    free(self);
}

void QBInitLoadAllServices(QBInit self)
{
    QBAppStateServiceSetAppState(self->appGlobals->appState, QBAppState_starting);

    if (self->lastLoadedConfiguration) {
        SvString mode = QBInitLogicGetServiceListFileName(self->appGlobals->initLogic);
        self->confChanged = !SvObjectEquals((SvObject) self->lastLoadedConfiguration, (SvObject) mode);
        SVTESTRELEASE(mode);
        if (self->confChanged) {
            QBInitRebootReasonStore("config mode change");
            QBInitStopApplicationImmediately(self->appGlobals, false);
            QBInitFinishApplication(self->appGlobals, false);
        } else if (!QBTunerLogicNeedsTunersReinit(self->appGlobals->tunerLogic)) {
            QBAppStateServiceSetAppState(self->appGlobals->appState, QBAppState_working);
            QBWizardLogicLoadingAllServicesFinished(self->appGlobals->wizardLogic);
            return;
        }
    }

    QBWindowContext ctx = QBAppInitWaiterContextCreate(self->appGlobals);
    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
    SVRELEASE(ctx);

    if (!self->loadingFiber) {
        self->loadingFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "CubiTV.QBInitLoadAllServices", QBInitLoadAllServicesStep, self);
        self->loadingTimer = SvFiberTimerCreate(self->loadingFiber);
    }

    SvFiberActivate(self->loadingFiber);
}

void QBInitStartApplication(AppGlobals appGlobals, const char* executableName, const char *settingsPrefix)
{
    QBInit self = calloc(1, sizeof(struct QBInit_t));
    self->appGlobals = appGlobals;
    appGlobals->initializer = self;
    QBInitMinimalSetup(self, executableName, settingsPrefix);
    QBInitSetupFirstFrame(self);

    self->firstBoot = QBInitLogicShouldEnterWizard(appGlobals->initLogic);

    SvLogNotice("QBInit: starting scheduler");
    SvSchedulerLoop(self->appGlobals->scheduler, false);
}

SvLocal void QBInitStopApplicationStep(void* self_)
{
    QBInit self = self_;
    SvFiberDestroy(self->stoppingFiber);
    self->stoppingFiber = NULL;
    QBInitStopApplicationImmediately(self->appGlobals, self->terminating);
}

void QBInitStopApplication(AppGlobals appGlobals, bool terminate, const char* rebootReason)
{
    if (appGlobals->initializer->terminating) {
        SvLogWarning("Application has already been stopped.");
        return;
    }

    if (rebootReason) {
        QBInitRebootReasonStore(rebootReason);
    } else {
        SvLogError("Reboot reason not set");
    }

    // only promote to full termination
    if (!appGlobals->initializer->terminating)
        appGlobals->initializer->terminating = terminate;
    if (!appGlobals->initializer->stoppingFiber)
        appGlobals->initializer->stoppingFiber = SvFiberCreate(appGlobals->scheduler, NULL, "CubiTV.QBInitStopApplication", QBInitStopApplicationStep, appGlobals->initializer);
    SvFiberActivate(appGlobals->initializer->stoppingFiber);
}

void QBInitFactoryResetAndStopApplication(AppGlobals appGlobals)
{
    QBRCUPairingService rcuPairingService = (QBRCUPairingService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBRCUPairingService"));
    if (rcuPairingService)
        QBRCUPairingServiceDropAllDevices(rcuPairingService, NULL);

    FILE *f1 = fopen("/tmp/factoryReset", "w");
    FILE *f2 = fopen("/etc/vod/factoryReset", "w");
    if (f1 || f2) {
        if (f1)
            fclose(f1);
        if (f2)
            fclose(f2);
        QBInitStopApplication(appGlobals, true, "factory reset");
    }
}

void QBInitStopApplicationImmediately(AppGlobals appGlobals, bool terminate)
{
    SvLogNotice("%s(): terminate is %s", __func__, terminate ? "true" : "false");
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    if (terminate) {
        QBDebugUtilsStopMemoryTrace();
        QBDebugUtilsListObjectsStop();
        QBInitLogicPreStop(appGlobals->initLogic);
        if (appGlobals->guiLogic)
            QBGUILogicStop(appGlobals->guiLogic);
    }

    QBConfigurationMenuLogicStop(appGlobals->configurationMenuLogic);

    QBRecordingIconUpdaterStop();

    if (appGlobals->secretKeyCodeService) {
        QBSecretKeyCodeServiceStop(appGlobals->secretKeyCodeService);
    }

    if (terminate) {
        SVRELEASE(appGlobals->initialStandbyState);
        if (appGlobals->standbyAgent)
            QBStandbyAgentStop(appGlobals->standbyAgent);
        if (appGlobals->standbyLogic)
            QBStandbyLogicStop(appGlobals->standbyLogic);
    }

    if (appGlobals->xhtmlWindowsService) {
        QBXHTMLWindowsServiceStop(appGlobals->xhtmlWindowsService);
    }

    if (appGlobals->secureLogLogic) {
        QBSecureLogLogicStop(appGlobals->secureLogLogic);
    }

    if (appGlobals->networkLogic)
        QBNetworkLogicStop(appGlobals->networkLogic);

    if (appGlobals->diskActivator) {
        QBDiskActivatorStop(appGlobals->diskActivator);
    }

    if (appGlobals->upgradeLogic) {
        QBUpgradeLogicStop(appGlobals->upgradeLogic);
    }

    if (appGlobals->channelLogosSetter) {
        QBDVBChannelLogosServiceStop(appGlobals->channelLogosSetter);
    }

    if (appGlobals->dsmccMonitor) {
        QBDSMCCMonitorStop(appGlobals->dsmccMonitor);
    }

    if (appGlobals->epgMonitor)
        QBEPGMonitorStop(appGlobals->epgMonitor);

    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_stoping);

    if (appGlobals->inputWatcher)
        QBInputWatcherStop(appGlobals->inputWatcher);

    if (terminate) {
        if (appGlobals->tunerLogic)
            QBTunerLogicStop(appGlobals->tunerLogic);
    }

    if (appGlobals->stbModeMonitor)
        QBSTBModeMonitorStop(appGlobals->stbModeMonitor, NULL);

    if (appGlobals->logWindow)
        QBLogWindowManagerStop(appGlobals->logWindow);

    if (appGlobals->serviceMonitor)
        QBMWServiceMonitorStop(appGlobals->serviceMonitor);

    if (appGlobals->providersController)
        QBProvidersControllerServiceStop(appGlobals->providersController);

    if (appGlobals->customerInfoMonitor)
        CubiwareMWCustomerInfoStop(appGlobals->customerInfoMonitor);

    if (appGlobals->customerInfoStorage)
        QBCustomerInfoStorageStop(appGlobals->customerInfoStorage);

    if (appGlobals->traxisWebSessionManager)
        TraxisWebSessionManagerStop(appGlobals->traxisWebSessionManager, NULL);

    if (appGlobals->traxisWebPlaybackSessionManager)
        TraxisWebPlaybackSessionManagerStop(appGlobals->traxisWebPlaybackSessionManager);

    if (appGlobals->middlewareManager)
        QBMiddlewareManagerStop(appGlobals->middlewareManager);

    if (appGlobals->redirProxyManager)
        QBRedirProxyManagerStop(appGlobals->redirProxyManager);

    if (appGlobals->appLogsInitialized)
        QBAppLogsStop();

    if (appGlobals->smartCardMonitor)
        QBSmartCardMonitorStop(appGlobals->smartCardMonitor);

    if (appGlobals->sdtMonitor)
        QBSDTMonitorStop(appGlobals->sdtMonitor);

    if (appGlobals->operateScanningManager)
        QBOperateScanningManagerStop(appGlobals->operateScanningManager);

    if (appGlobals->reminderAgent)
        QBReminderAgentStop(appGlobals->reminderAgent);

    if (appGlobals->natMonitor)
        QBNATMonitorStop(appGlobals->natMonitor);

    if (appGlobals->net_manager)
        QBNetManagerStop(appGlobals->net_manager, NULL);

    if (appGlobals->networkMonitor)
        QBNetworkMonitorStop(appGlobals->networkMonitor);

    if (appGlobals->cableModem)
        QBCableModemMonitorStop(appGlobals->cableModem);

    if (appGlobals->authenticationService)
        QBAuthenticationServiceStop(appGlobals->authenticationService);

    if (appGlobals->adService)
        QBAdServiceStop(appGlobals->adService);

    if (terminate)
        QBInputServiceStop(NULL);

    if (appGlobals->lstvDVBTimeDateInitialized) {
        lstvDVBTimeDateStop();
        appGlobals->lstvDVBTimeDateInitialized = false;
    }

    if (appGlobals->fingerprint)
        QBFingerprintStop(appGlobals->fingerprint);

    if (appGlobals->conaxMailManager)
        QBConaxMailManagerStop(appGlobals->conaxMailManager);

    if (appGlobals->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, appGlobals->casPopupManager, stop);

    if (appGlobals->conaxURIManager)
        QBConaxURIManagerStop(appGlobals->conaxURIManager);

    if (appGlobals->casPVRManager)
        SvInvokeInterface(QBCASPVRManager, appGlobals->casPVRManager, stop);

    if (appGlobals->cryptoguardVideoRulesManager)
        QBCryptoguardVideoRulesManagerStop(appGlobals->cryptoguardVideoRulesManager);

    if (appGlobals->cryptoguardForceTuneManager)
        QBCryptoguardForceTuneManagerStop(appGlobals->cryptoguardForceTuneManager);

    if (appGlobals->conaxMenuHandler)
        QBConaxMenuHandlerStop(appGlobals->conaxMenuHandler);

    if (appGlobals->reencryptionService) {
        QBReencryptionServiceStop(appGlobals->reencryptionService);
    }

    if (appGlobals->viewRightTriggerManager)
        QBViewRightTriggerManagerStop(appGlobals->viewRightTriggerManager);

    if (appGlobals->viewRightWeb)
        QBViewRightWebServiceStop(appGlobals->viewRightWeb);

    if (appGlobals->parentalControlLogic)
        QBParentalControlLogicStop(appGlobals->parentalControlLogic);

    if (appGlobals->tvLogic) {
        QBTVLogicStopPlaying(appGlobals->tvLogic, SVSTRING("init"));
        QBTVLogicStop(appGlobals->tvLogic);
    }

    if (appGlobals->pvrLogic)
        QBPVRLogicStop(appGlobals->pvrLogic);

    if (appGlobals->hotplugNotifier)
        QBHotplugNotifierStop(appGlobals->hotplugNotifier);

    if (appGlobals->fileBrowserMounter)
        QBFileBrowserMounterStop(appGlobals->fileBrowserMounter);

    if (appGlobals->pvrAgent)
        QBPVRAgentStop(appGlobals->pvrAgent);

    if (appGlobals->pvrProvider) {
        appGlobals->pvrProviderInitialized = false;
    }

    if (appGlobals->npvrAgent)
        QBNPvrAgentStop(appGlobals->npvrAgent);

    if (appGlobals->nPVRProvider)
        SvInvokeInterface(QBnPVRProvider, appGlobals->nPVRProvider, stop);

    if (appGlobals->cutvManager)
        CutvManagerStop(appGlobals->cutvManager);

    if (appGlobals->hotplugMountAgent)
        QBHotplugMountAgentStop(appGlobals->hotplugMountAgent);

    if (terminate) {
        QBPlayReadyManagerDeinit();
        QBDRMStop();
        QBDRMDeinit(appGlobals);
    }

    if (appGlobals->inputTranslator)
        QBKeyInputTranslatorStop(appGlobals->inputTranslator, NULL);

    if (appGlobals->textPrerenderer)
        QBTextPreRendererStop(appGlobals->textPrerenderer);

    if (terminate && appGlobals->upgradeWatcher)
        UpgradeWatcherStop(appGlobals->upgradeWatcher);

    if (appGlobals->main) {
        QBApplicationControllerRemovePersistentContext(appGlobals->controller, appGlobals->main);
        SvInvokeInterface(QBMainMenu, appGlobals->main, deinit);
    }

    if (terminate) {
        QBGlobalWindowManagerStop(appGlobals->globalManager);
        QBLocalWindowManagerStop(appGlobals->localManager);
        QBApplicationController controller = (QBApplicationController) QBServiceRegistryGetService(registry, SVSTRING("QBApplicationController"));
        QBApplicationControllerStop(controller, NULL);

        QBDecoderCleanup();
    }

    if (appGlobals->hintManager)
        QBHintManagerStop(appGlobals->hintManager);

    if(appGlobals->httpEmm)
        QBHTTPEMMServiceStop(appGlobals->httpEmm);

    if (appGlobals->conaxHttpEmm) {
        QBConaxPullEMMServiceStop(appGlobals->conaxHttpEmm);
    }

    if (terminate)
        if (appGlobals->cleaner)
            QBCleanerStop(appGlobals->cleaner);

    if (appGlobals->mainLogic) {
        QBMainMenuLogicStop(appGlobals->mainLogic);
    }

    if (appGlobals->playlistCursors)
        QBPlaylistCursorsStop(appGlobals->playlistCursors);

    if (appGlobals->middlewareClientService)
        CubiwareMWClientServiceStop(appGlobals->middlewareClientService);

    if (terminate && appGlobals->watchdog)
        QBWatchdogStop(appGlobals->watchdog, NULL);

    if (appGlobals->accessLogic)
        QBAccessLogicStop(appGlobals->accessLogic);

    if (appGlobals->diagnosticsLogic)
        QBDiagnosticsLogicStop(appGlobals->diagnosticsLogic);

//    if (appGlobals->skinManager)
//        QBSkinManagerStop(appGlobals->skinManager);

    if (appGlobals->standByTimerTimeout)
        QBStandByTimerTimeoutStop(appGlobals->standByTimerTimeout);

    if (appGlobals->appVersion)
        QBAppVersionStop(appGlobals->appVersion);

    if (appGlobals->pvodStorage)
        QBPVODStorageServiceStop(appGlobals->pvodStorage);

    if (appGlobals->remoteScheduler)
        QBRemoteSchedulerStop(appGlobals->remoteScheduler);

    if (appGlobals->diskSchedulerChanger)
        QBDiskSchedulerChangerStop(appGlobals->diskSchedulerChanger);

    if (appGlobals->qbMWConfigMonitor)
        QBMWConfigMonitorStop(appGlobals->qbMWConfigMonitor);

    if (appGlobals->qbMWConfigHandler)
        QBMWConfigHandlerStop(appGlobals->qbMWConfigHandler);

    if (appGlobals->epgLogic)
        QBEPGLogicStop(appGlobals->epgLogic);

    if (appGlobals->bookmarkManager)
        QBBookmarkManagerStop(appGlobals->bookmarkManager);

    if (appGlobals->remoteControl)
        QBRemoteControlStop(appGlobals->remoteControl, NULL);

    if (appGlobals->remoteMessagesManager) {
        QBRemoteMessagesManagerStop(appGlobals->remoteMessagesManager, NULL);
    }

    if (appGlobals->callerIdManager) {
        QBCallerIdManagerStop(appGlobals->callerIdManager, NULL);
    }

    if (appGlobals->pushReceiver)
        QBPushReceiverStop(appGlobals->pushReceiver, NULL);

    if (appGlobals->secureLogService)
        QBSecureLogServiceStop(appGlobals->secureLogService, NULL);

    if (appGlobals->sanityChecker)
        QBSanityCheckerStop(appGlobals->sanityChecker);

    if (appGlobals->outputStandardService)
        QBOutputStandardServiceStop(appGlobals->outputStandardService);

    if (appGlobals->cecService)
        QBCecServiceStop(appGlobals->cecService);

    if (appGlobals->appsManager)
        QBAppsManagerStop(appGlobals->appsManager);

    if (appGlobals->dialServerManager)
        QBDIALServerManagerStop(appGlobals->dialServerManager, NULL);

    if (appGlobals->cec)
        QBCECStop(appGlobals->cec);

    if (terminate) {
        svAppPostEvent(appGlobals->res, NULL, svQuitEventCreate());

        SvSchedulerPrint(SvSchedulerGet());
        SvSchedulerEnableLoopPrintFibers(appGlobals->scheduler);
    }

    if (terminate) {
        QBServiceRegistryStopServices(registry, -1, NULL);

        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(registry, SVSTRING("EventBus"));
        QBEventBusFlushEvents(eventBus, NULL);

        QBGlobalStorageClear(QBGlobalStorageGetInstance(), NULL);
    } else {
        QBServiceRegistryRestoreState(registry, SVSTRING("MinimalSet"), -1, NULL);
    }

    if (terminate)
        QBStaticStorageStop();
}

void QBInitFinishApplication(AppGlobals appGlobals, bool terminate)
{
    SvLogNotice("%s(): terminate is %s", __func__, terminate ? "true" : "false");
    QBAppStateServiceSetAppState(appGlobals->appState, QBAppState_destroying);
    if (terminate && appGlobals->watchdog)
        QBWatchdogContinue(appGlobals->watchdog, NULL);

    QBInitLogicPreDestroy(appGlobals->initLogic, terminate);

    SVTESTRELEASE(appGlobals->logWindow);
    appGlobals->logWindow = NULL;

    if (terminate) {
        SvAutoreleasePoolDrain(appGlobals->appPool);
    }

    if (terminate && appGlobals->appLogsInitialized)
        QBAppLogsDestroy();

    if (terminate) {
        SVRELEASE(appGlobals->appState);
        appGlobals->appState = NULL;
    }

    if (!terminate)
        QBAppStateServiceRemoveAllListeners(appGlobals->appState);

    SVTESTRELEASE(appGlobals->xhtmlWindowsService);
    appGlobals->xhtmlWindowsService = NULL;
    SVTESTRELEASE(appGlobals->noInputStandbyPlugin);
    appGlobals->noInputStandbyPlugin = NULL;
    SVTESTRELEASE(appGlobals->secureLogLogic);
    appGlobals->secureLogLogic = NULL;
    SVTESTRELEASE(appGlobals->diskActivator);
    appGlobals->diskActivator = NULL;
    SVTESTRELEASE(appGlobals->remoteScheduler);
    appGlobals->remoteScheduler = NULL;
    SVTESTRELEASE(appGlobals->hddStandbyManager);
    appGlobals->hddStandbyManager = NULL;
    SVTESTRELEASE(appGlobals->textPrerenderer);
    appGlobals->textPrerenderer = NULL;
    SVTESTRELEASE(appGlobals->secretKeyCodeService);
    appGlobals->secretKeyCodeService = NULL;
    SVTESTRELEASE(appGlobals->stbModeMonitor);
    appGlobals->stbModeMonitor = NULL;
    SVTESTRELEASE(appGlobals->serviceMonitor);
    appGlobals->serviceMonitor = NULL;
    SVTESTRELEASE(appGlobals->providersController);
    appGlobals->providersController = NULL;
    if (terminate) {
        SVTESTRELEASE(appGlobals->inputWatcher);
        appGlobals->inputWatcher = NULL;
        SVTESTRELEASE(appGlobals->middlewareManager);
        appGlobals->middlewareManager = NULL;
    }
    SVTESTRELEASE(appGlobals->redirProxyManager);
    appGlobals->redirProxyManager = NULL;
    SVTESTRELEASE(appGlobals->reminder);
    appGlobals->reminder = NULL;
    SVTESTRELEASE(appGlobals->reminderAgent);
    appGlobals->reminderAgent = NULL;
    SVTESTRELEASE(appGlobals->fingerprint);
    appGlobals->fingerprint = NULL;
    SVTESTRELEASE(appGlobals->conaxMailManager);
    appGlobals->conaxMailManager = NULL;
    SVTESTRELEASE(appGlobals->casPopupManager);
    appGlobals->casPopupManager = NULL;
    SVTESTRELEASE(appGlobals->conaxURIManager);
    appGlobals->conaxURIManager = NULL;
    SVTESTRELEASE(appGlobals->DRMManager);
    appGlobals->DRMManager = NULL;
    SVTESTRELEASE(appGlobals->casPVRManager);
    appGlobals->casPVRManager = NULL;
    SVTESTRELEASE(appGlobals->cryptoguardVideoRulesManager);
    appGlobals->cryptoguardVideoRulesManager = NULL;
    SVTESTRELEASE(appGlobals->cryptoguardForceTuneManager);
    appGlobals->cryptoguardForceTuneManager = NULL;
    SVTESTRELEASE(appGlobals->viewRightTriggerManager);
    appGlobals->viewRightTriggerManager = NULL;
    SVTESTRELEASE(appGlobals->viewRightWeb);
    appGlobals->viewRightWeb = NULL;
    SVTESTRELEASE(appGlobals->mailIndicator);
    appGlobals->mailIndicator = NULL;
    SVTESTRELEASE(appGlobals->sdtMonitor);
    appGlobals->sdtMonitor = NULL;
    SVTESTRELEASE(appGlobals->operateScanningManager);
    appGlobals->operateScanningManager = NULL;
    SVTESTRELEASE(appGlobals->epgMonitor);
    appGlobals->epgMonitor = NULL;
    SVTESTRELEASE(appGlobals->qbMWProductUsageController);
    appGlobals->qbMWProductUsageController = NULL;
    SVTESTRELEASE(appGlobals->customerInfoMonitor);
    appGlobals->customerInfoMonitor = NULL;
    SVTESTRELEASE(appGlobals->customerInfoStorage);
    appGlobals->customerInfoStorage = NULL;
    SVTESTRELEASE(appGlobals->authenticationService);
    appGlobals->authenticationService = NULL;
    SVTESTRELEASE(appGlobals->adService);
    appGlobals->adService = NULL;
    SVTESTRELEASE(appGlobals->appVersion);
    appGlobals->appVersion = NULL;
    SVTESTRELEASE(appGlobals->pvodStorage);
    appGlobals->pvodStorage = NULL;
    SVTESTRELEASE(appGlobals->traxisWebSessionManager);
    appGlobals->traxisWebSessionManager = NULL;
    SVTESTRELEASE(appGlobals->traxisWebPlaybackSessionManager);
    appGlobals->traxisWebPlaybackSessionManager = NULL;
    SVTESTRELEASE(appGlobals->traxisWebMonitor);
    appGlobals->traxisWebMonitor = NULL;
    SVTESTRELEASE(appGlobals->dvbMuxesMap);
    appGlobals->dvbMuxesMap = NULL;
    SVTESTRELEASE(appGlobals->diagnosticsManager);
    appGlobals->diagnosticsManager = NULL;

    if (terminate) {
        SVRELEASE(appGlobals->controller);
        appGlobals->controller = NULL;
        SVTESTRELEASE(appGlobals->switcher);
        appGlobals->switcher = NULL;
        QBGlobalWindowManagerDestroy(appGlobals->globalManager);
        appGlobals->globalManager = NULL;
        SVRELEASE(appGlobals->localManager);
        appGlobals->localManager = NULL;
        SVRELEASE(appGlobals->focusManager);
        appGlobals->focusManager = NULL;
    }

    SVTESTRELEASE(appGlobals->accessLogic);
    appGlobals->accessLogic = NULL;

    SVTESTRELEASE(appGlobals->eventISLogic);
    appGlobals->eventISLogic = NULL;

    SVTESTRELEASE(appGlobals->satelliteSetupLogic);
    appGlobals->satelliteSetupLogic = NULL;

    if (terminate) {
        SVTESTRELEASE(appGlobals->wizardLogic);
        appGlobals->wizardLogic = NULL;
    }

    SVTESTRELEASE(appGlobals->purchaseLogic);
    appGlobals->purchaseLogic = NULL;
    SVTESTRELEASE(appGlobals->tvLogic);
    appGlobals->tvLogic = NULL;

    if (terminate) {
        SVTESTRELEASE(appGlobals->initLogic);
        appGlobals->initLogic = NULL;
    }

    SVTESTRELEASE(appGlobals->epgLogic);
    appGlobals->epgLogic = NULL;
    SVTESTRELEASE(appGlobals->pvrLogic);
    appGlobals->pvrLogic = NULL;
    SVTESTRELEASE(appGlobals->verticalEPGLogic);
    appGlobals->verticalEPGLogic = NULL;
    SVTESTRELEASE(appGlobals->searchResultsLogic);
    appGlobals->searchResultsLogic = NULL;
    SVTESTRELEASE(appGlobals->operateScanningLogic);
    appGlobals->operateScanningLogic = NULL;
    SVTESTRELEASE(appGlobals->channelScanningLogic);
    appGlobals->channelScanningLogic = NULL;
    if (terminate) {
        SVTESTRELEASE(appGlobals->tunerLogic);
        appGlobals->tunerLogic = NULL;
        SVTESTRELEASE(appGlobals->upgradeLogic);
        appGlobals->upgradeLogic = NULL;
        SVTESTRELEASE(appGlobals->standbyLogic);
        appGlobals->standbyLogic = NULL;
    }
    SVTESTRELEASE(appGlobals->dateTimeLogic);
    appGlobals->dateTimeLogic = NULL;
    SVTESTRELEASE(appGlobals->mainLogic);
    appGlobals->mainLogic = NULL;
    SVTESTRELEASE(appGlobals->globalInputLogic);
    appGlobals->globalInputLogic = NULL;
    SVTESTRELEASE(appGlobals->standByTimerTimeout);
    appGlobals->standByTimerTimeout = NULL;
    SVTESTRELEASE(appGlobals->callerIdManager);
    appGlobals->callerIdManager = NULL;
    if (terminate) {
        SVTESTRELEASE(appGlobals->guiLogic);
        appGlobals->guiLogic = NULL;
    }
    SVTESTRELEASE(appGlobals->eventsLogic);
    appGlobals->eventsLogic = NULL;
    SVTESTRELEASE(appGlobals->configurationMenuLogic);
    appGlobals->configurationMenuLogic = NULL;
    SVTESTRELEASE(appGlobals->mstoreLogic);
    appGlobals->mstoreLogic = NULL;
    SVTESTRELEASE(appGlobals->diagnosticsLogic);
    appGlobals->diagnosticsLogic = NULL;
    SVTESTRELEASE(appGlobals->httpEmm);
    appGlobals->httpEmm = NULL;
    SVTESTRELEASE(appGlobals->conaxHttpEmm);
    appGlobals->conaxHttpEmm = NULL;
    SVTESTRELEASE(appGlobals->casPopupVisabilityManager);
    appGlobals->casPopupVisabilityManager = NULL;

    if (terminate) {
        SVTESTRELEASE(appGlobals->upgradeWatcher);
        appGlobals->upgradeWatcher = NULL;
    }

    SVTESTRELEASE(appGlobals->newTV);
    appGlobals->newTV = NULL;
    SVTESTRELEASE(appGlobals->parentalControlLogic);
    appGlobals->parentalControlLogic = NULL;
    SVTESTRELEASE(appGlobals->newTVGuide);
    appGlobals->newTVGuide = NULL;
    SVTESTRELEASE(appGlobals->verticalEPG);
    appGlobals->verticalEPG = NULL;
    SVTESTRELEASE(appGlobals->pvrProvider);
    appGlobals->pvrProvider = NULL;
    SVTESTRELEASE(appGlobals->pvrAgent);
    appGlobals->pvrAgent = NULL;
    SVTESTRELEASE(appGlobals->npvrAgent);
    appGlobals->npvrAgent = NULL;
    SVTESTRELEASE(appGlobals->recordingsTree);
    appGlobals->recordingsTree = NULL;
    SVTESTRELEASE(appGlobals->hotplugMountAgent);
    appGlobals->hotplugMountAgent = NULL;
    SVTESTRELEASE(appGlobals->reencryptionService);
    appGlobals->reencryptionService = NULL;
    SVTESTRELEASE(appGlobals->PVRMounter);
    appGlobals->PVRMounter = NULL;
    SVTESTRELEASE(appGlobals->fileBrowserMounter);
    appGlobals->fileBrowserMounter = NULL;
    SVTESTRELEASE(appGlobals->main);
    appGlobals->main = NULL;
    SVTESTRELEASE(appGlobals->screensaver);
    appGlobals->screensaver = NULL;

    if (terminate) {
        SVTESTRELEASE(appGlobals->standbyAgent);
        appGlobals->standbyAgent = NULL;
    }

    SVTESTRELEASE(appGlobals->hotplugNotifier);
    appGlobals->hotplugNotifier = NULL;
    SVTESTRELEASE(appGlobals->smartCardMonitor);
    appGlobals->smartCardMonitor = NULL;
    SVTESTRELEASE(appGlobals->accessMgr);
    appGlobals->accessMgr = NULL;

    if (terminate) {
        SVTESTRELEASE(appGlobals->volumeWidget);
        appGlobals->volumeWidget = NULL;
        SVTESTRELEASE(appGlobals->volume);
        appGlobals->volume = NULL;
        QBCleanerDestroy(appGlobals->cleaner);
        appGlobals->cleaner = NULL;
    }

    SVTESTRELEASE(appGlobals->natMonitor);
    appGlobals->natMonitor = NULL;
    SVTESTRELEASE(appGlobals->net_manager);
    appGlobals->net_manager = NULL;
    SVTESTRELEASE(appGlobals->networkLogic);
    appGlobals->networkLogic = NULL;

    if (terminate) {
        SVTESTRELEASE(appGlobals->networkMonitor);
        appGlobals->networkMonitor = NULL;
    }

    QBNetManagerSetGlobal(NULL);
    SVTESTRELEASE(appGlobals->cableModem);
    appGlobals->cableModem = NULL;
    SVTESTRELEASE(appGlobals->videoFormatMonitor);
    appGlobals->videoFormatMonitor = NULL;
    SVTESTRELEASE(appGlobals->outputStandardService);
    appGlobals->outputStandardService = NULL;
    SVTESTRELEASE(appGlobals->cutvManager);
    appGlobals->cutvManager = NULL;
    SVTESTRELEASE(appGlobals->nPVRProvider);
    appGlobals->nPVRProvider = NULL;
    SVTESTRELEASE(appGlobals->channelMerger);
    appGlobals->channelMerger = NULL;
    SVTESTRELEASE(appGlobals->epgPlugin.DVB.channelList);
    appGlobals->epgPlugin.DVB.channelList = NULL;
    SVTESTRELEASE(appGlobals->epgPlugin.DVB.data);
    appGlobals->epgPlugin.DVB.data = NULL;
    SVTESTRELEASE(appGlobals->epgPlugin.IP.channelList);
    appGlobals->epgPlugin.IP.channelList = NULL;
    SVTESTRELEASE(appGlobals->epgPlugin.IP.data);
    appGlobals->epgPlugin.IP.data = NULL;
    SVTESTRELEASE(appGlobals->mapper);
    appGlobals->mapper = NULL;
    SVTESTRELEASE(appGlobals->playlistCursors);
    appGlobals->playlistCursors = NULL;
    SVTESTRELEASE(appGlobals->middlewareClientService);
    appGlobals->middlewareClientService = NULL;

    SVTESTRELEASE(appGlobals->channelStorage);
    appGlobals->channelStorage = NULL;
    SVTESTRELEASE(appGlobals->channelMetaStorage);
    appGlobals->channelMetaStorage = NULL;
    SVTESTRELEASE(appGlobals->hintManager);
    appGlobals->hintManager = NULL;
    SVTESTRELEASE(appGlobals->pushReceiver);
    appGlobals->pushReceiver = NULL;
    SVTESTRELEASE(appGlobals->secureLogService);
    appGlobals->secureLogService = NULL;
    SVTESTRELEASE(appGlobals->remoteControl);
    appGlobals->remoteControl = NULL;
    SVTESTRELEASE(appGlobals->remoteMessagesManager);
    appGlobals->remoteMessagesManager = NULL;
    SVTESTRELEASE(appGlobals->bookmarkManager);
    appGlobals->bookmarkManager = NULL;
    SVTESTRELEASE(appGlobals->diskSchedulerChanger);
    appGlobals->diskSchedulerChanger = NULL;
    SVTESTRELEASE(appGlobals->inputTranslator);
    appGlobals->inputTranslator = NULL;
    SVTESTRELEASE(appGlobals->langPreferences);
    appGlobals->langPreferences = NULL;
    SVTESTRELEASE(appGlobals->pc);
    appGlobals->pc = NULL;
    SVTESTRELEASE(appGlobals->dataBucket);
    appGlobals->dataBucket = NULL;
    SVTESTRELEASE(appGlobals->remoteScheduler);
    appGlobals->remoteScheduler = NULL;
    SVTESTRELEASE(appGlobals->qbMWConfigMonitor);
    appGlobals->qbMWConfigMonitor = NULL;
    SVTESTRELEASE(appGlobals->qbMWConfigHandler);
    appGlobals->qbMWConfigHandler = NULL;
    SVTESTRELEASE(appGlobals->appsManager);
    appGlobals->appsManager = NULL;
    SVTESTRELEASE(appGlobals->dialServerManager);
    appGlobals->dialServerManager = NULL;

    if (appGlobals->schedManTunerReservation) {
        QBSchedManTunerReservationDestroy(appGlobals->schedManTunerReservation);
        appGlobals->schedManTunerReservation = NULL;
    }

    if (terminate) {
        QBConfigDestroy();
        QBProfileConfigDestroy();
        SVRELEASE(appGlobals->textRenderer);
        appGlobals->textRenderer = NULL;
    }

    SVTESTRELEASE(appGlobals->menuTree);
    appGlobals->menuTree = NULL;

    SVTESTRELEASE(appGlobals->subtreeNotifier);
    appGlobals->subtreeNotifier = NULL;

    if (terminate)
        QBDRMCleanup();

    SvPlayerManagerCleanup();

    if (terminate) {
        SVTESTRELEASE(appGlobals->sslParams);
        appGlobals->sslParams = NULL;
    }

    QBOpenSSLDeinit();

    SVTESTRELEASE(appGlobals->playerController);
    appGlobals->playerController = NULL;
    appGlobals->pvrAgent = NULL;

    SVTESTRELEASE(appGlobals->adsDSMCCDataProvider);
    appGlobals->adsDSMCCDataProvider = NULL;
    SVTESTRELEASE(appGlobals->dsmccMonitor);
    appGlobals->dsmccMonitor = NULL;

    SVTESTRELEASE(appGlobals->channelLogosSetter);
    appGlobals->channelLogosSetter = NULL;

    if (terminate)
        QBAutoTestDestroy();

    SVTESTRELEASE(appGlobals->cecService);
    appGlobals->cecService = NULL;

    SVTESTRELEASE(appGlobals->cec);
    appGlobals->cec = NULL;
}
