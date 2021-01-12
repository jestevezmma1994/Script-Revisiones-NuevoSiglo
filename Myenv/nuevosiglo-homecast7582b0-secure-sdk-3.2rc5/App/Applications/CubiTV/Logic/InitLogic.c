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

#include <Logic/InitLogic.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvURL.h>
#include <settings.h>
#include <QBConf.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <QBTuner.h>
#include <sv_tuner.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <SvEPGDataLayer/Views/SvEPGPersistentView.h>
#include <SvEPGDataLayer/Views/SvEPGHashTableFilter.h>
#include <SvEPGDataLayer/Views/SvEPGNegativeFilter.h>
#include <SvEPGDataLayer/Views/SvEPGSortingView.h>
#include <SvEPGDataLayer/Views/SvEPGTVOrRadioFilter.h>
#include <SvEPGDataLayer/Views/SvEPGSourcePluginFilter.h>
#include <main.h>
#include <Services/QBAccessController/Innov8onAccessPlugin.h>
#include <Services/QBAccessController/PCAccessPlugin.h>
#include <Services/QBAccessController/ConaxAccessPlugin.h>
#include <Services/QBAccessController/ViewRightAccessPlugin.h>
#include <QBApplicationController.h>
#include <Windows/newtv.h>
#include <Windows/standby.h>
#include <Windows/wizard.h>
#include <Windows/appStartupWaiter.h>
#include <Windows/appStartupWaiter/SvEPGChannelListWaiter.h>
#include <Windows/appStartupWaiter/QBViewRightEMMWaiter.h>
#include <Windows/appInitWaiter.h>
#include <Windows/QBNetworkSettingsWindow.h>
#include <QBRecordFS/types.h>
#include <QBRecordFS/root.h>
#include <Services/core/QBDVBBouquets.h>
#include <Services/core/prerenderer.h>
#include <Services/QBCASManager.h>
#include <Services/QBMWConfigHandlerPlugins/ConaxQBMWConfigHandlerPlugin.h>
#include <Services/QBMWConfigHandlerPlugins/QBMWConfigHandlerPluginAppMode.h>
#include <Services/QBMWConfigHandlerPlugins/SeawellQBMWConfigHandlerPlugin.h>
#include <Middlewares/Innov8on/Innov8onCutvPlugin.h>
#include <DataModels/NordigLCN.h>
#include <QBPlatformHAL/QBPlatformOption.h>
#include <SvEPGDataLayer/SvChannelMerger.h>
#include <SvEPGDataLayer/Plugins/Innov8onEPGPlugin.h>
#include <SvEPGDataLayer/Plugins/GenericEITChannelMapper.h>
#include <SvEPGDataLayer/ChannelMergers/SvChannelMergerBasic.h>
#include <Utils/appType.h>
#include <Utils/channelMetaFill.h>
#include <Utils/value.h>
#include <Utils/recordingUtils.h>
#include <Logic/TunerLogic.h>
#include <QBInput/QBInputCodes.h>
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManagerPlugin.h>
#include <QBBookmarkService/QBInnov8onBookmarkManagerPlugin.h>
#include <QBBookmarkService/QBGenericBookmarkManagerPlugin.h>
#include <QBBookmarkService/QBRecordingBookmarkManagerPlugin.h>
#include <SvQuirks/SvQuirks.h>
#include <QBContentManager/Innov8onContentManager.h>
#include <NPvr/QBnPVRProvider.h>
#include <CubiwareMWClient/NPvr/CubiwareMWNPvrProvider.h>
#include <CubiwareMWClient/CubiwareMWClientService.h>
#include <CubiwareMWClient/CubiwareMWClientServiceConaxPlugin.h>
#include <TraxisWebClient/TraxisWebCutvPlugin.h>
#include <TraxisWebClient/TraxisWebEPGDataPlugin.h>
#include <TraxisWebClient/TraxisWebEPGSearchPlugin.h>
#include <TraxisWebClient/TraxisWebNPvrPlugin.h>
#include <QBLatensLibLoader.h>
#include <QBLatens.h>
#include <QBMWClient/QBMWClientContentManager.h>
#include <QBMWClient/DigitalsmithsContentManager.h>
#include <QBMWClient/Utils.h>
#include <QBPVRProvider.h>
#include <Services/core/QBContentManagers.h>
#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Configurations/QBDiskPvrStorageMode.h>
#include <safeFiles.h>
#include <QBSecureLogManager.h>

#include <QBPlayReadyManager.h>
#include <QBConaxPlayReady/QBConaxPlayReady.h>

#define DEFAULT_CONTENT_MANAGER_RANGE_SIZE 20
#define DEFAULT_CONTENT_MANAGER_RANGE_TIMEOUT_MS 600000
#define DEFAULT_CONTENT_MANAGER_RANGE_MARGIN_MS 60000

#define DVB_CHANNELS_RESTORE_TIMEOUT_MS 2000
#define IP_CHANNELS_RESTORE_TIMEOUT_MS 20000
#define EMMS_WAITER_TIMEOUT_S 60             // EMMs waiter timeout in seconds

struct QBInitLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    bool wizardEntered;
    bool wasWizardEntered;
};

SvLocal void QBInitLogicInnov8onEPGChannelListPluginSetup(Innov8onEPGChannelListPlugin channelListPlugin, SvXMLRPCServerInfo serverInfo, SvString prefixUrl, SvString id)
{
    Innov8onEPGChannelListPluginSetServerInfo(channelListPlugin, serverInfo, NULL);

    if (prefixUrl) {
        SvURL baseURL = SvURLCreateWithString(prefixUrl, NULL);
        Innov8onEPGChannelListPluginSetBaseURL(channelListPlugin, baseURL, NULL);
        SVRELEASE(baseURL);
    }

    Innov8onEPGChannelListPluginSetUserID(channelListPlugin, id, NULL);
}

SvLocal void QBInitLogicInnov8onEPGDataPluginSetup(Innov8onEPGDataPlugin dataPlugin, SvXMLRPCServerInfo serverInfo, SvString id)
{
    Innov8onEPGDataPluginSetServerInfo(dataPlugin, serverInfo, NULL);
    Innov8onEPGDataPluginSetUserID(dataPlugin, id, NULL);
}

SvLocal void
QBInitLogicCreateCubiwareMWContentManager(QBInitLogic self, SvXMLRPCServerInfo serverInfo)
{
    SvLogNotice("Creating innov8on content manager");
    Innov8onContentManager manager = (Innov8onContentManager) SvTypeAllocateInstance(Innov8onContentManager_getType(), NULL);
    Innov8onContentManagerInit(manager, serverInfo, NULL);
    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    QBContentManagersServiceAdd(contentManagers, SvXMLRPCServerInfoGetBaseURL(serverInfo, NULL),
                                QBContentManagerType_cubiware, (SvObject) manager);
    SVRELEASE(manager);
}

SvLocal void
QBInitLogicCreateDigitalSmithsContentManager(QBInitLogic self)
{
    SvLogNotice("Creating DigitalSmiths content manager");

    const char *dataPath = QBMWClientGetDataPath();

    if (!dataPath) {
        SvLogWarning("%s (%d): Missing QBMWClient data path", __func__, __LINE__);
        return;
    }

    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    if (!contentManagers) {
        SvLogWarning("%s (%d): Missing content managers service", __func__, __LINE__);
        return;
    }

    QBMWClientContentManager mainManager = (QBMWClientContentManager) SvTypeAllocateInstance(DigitalsmithsContentManager_getType(), NULL);
    QBMWClientContentManagerParams contentManagerParams = (QBMWClientContentManagerParams) SvTypeAllocateInstance(QBMWClientContentManagerParams_getType(), NULL);

    SvString deviceId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Digitalsmiths);
    SvString baseURLString = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Digitalsmiths);
    SvString imagesBaseURLString = QBMiddlewareManagerGetThumbnailsPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Digitalsmiths);

    SvURL baseURL = SvURLCreateWithString(baseURLString, NULL);

    contentManagerParams->baseUrl = SVTESTRETAIN(baseURL);
    contentManagerParams->imagesBaseUrl = SvURLCreateWithString(imagesBaseURLString, NULL);
    contentManagerParams->deviceId = SVTESTRETAIN(deviceId);
    contentManagerParams->rangeSize = DEFAULT_CONTENT_MANAGER_RANGE_SIZE;
    contentManagerParams->rangeTimeout = SvTimeFromMs(DEFAULT_CONTENT_MANAGER_RANGE_TIMEOUT_MS);
    contentManagerParams->rangeMargin = SvTimeFromMs(DEFAULT_CONTENT_MANAGER_RANGE_MARGIN_MS);
    contentManagerParams->mapperConf = SvStringCreateWithFormat("%s/%s/digitalsmithsAttributes.map", SvGetRuntimePrefix(), dataPath);
    QBMWClientContentManagerInit(mainManager, contentManagerParams, NULL);

    QBContentManagersServiceAdd(contentManagers, baseURL, QBContentManagerType_mwClient, (SvObject) mainManager);

    SVTESTRELEASE(baseURL);
    SVRELEASE(contentManagerParams);
    SVRELEASE(mainManager);
}

SvLocal void QBInitLogicMiddlewareDataChanged(SvObject self_, QBMiddlewareManagerType type)
{
    QBInitLogic self = (QBInitLogic) self_;
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString id = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!serverInfo || !prefixUrl || !id || !hasNetwork) {
        return;         // still waiting
    }

    if (self->appGlobals->epgPlugin.IP.channelList) {
        QBInitLogicInnov8onEPGChannelListPluginSetup((Innov8onEPGChannelListPlugin) self->appGlobals->epgPlugin.IP.channelList, serverInfo, prefixUrl, id);
    }

    if (self->appGlobals->epgPlugin.IP.data) {
        QBInitLogicInnov8onEPGDataPluginSetup((Innov8onEPGDataPlugin) self->appGlobals->epgPlugin.IP.data, serverInfo, id);
    }

    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    SvObject contentManager = QBContentManagersServiceFindFirst(contentManagers, QBContentManagerType_cubiware);
    if (!contentManager) {
        QBInitLogicCreateCubiwareMWContentManager(self, serverInfo);
    }
}

SvLocal void QBInitLogicAdaptService(SvObject self_, SvObject service)
{
    SvString serviceLayout = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) service, "service_layout"));

    bool stopUnfoldingLayoutSelected = false;
    const char *vodView = QBConfigGet("VODVIEW");
    bool configHasStopUnfoldingView = (vodView && (!strncmp(vodView, "vod3", 4) || !strncmp(vodView, "vod4", 4)));
    if (serviceLayout) {
        stopUnfoldingLayoutSelected = (SvStringEqualToCString(serviceLayout, "vod3") || SvStringEqualToCString(serviceLayout, "vod4") ||
                              (SvStringEqualToCString(serviceLayout, "default") && configHasStopUnfoldingView));
    } else {
        stopUnfoldingLayoutSelected = configHasStopUnfoldingView;
    }

    SvValue val = NULL;
    SvValue unfoldableVal = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) service, "unfoldable");
    if (unfoldableVal && SvObjectIsInstanceOf((SvObject) unfoldableVal, SvValue_getType())) {
        if (SvValueIsBoolean(unfoldableVal))
            val = SvValueCreateWithBoolean(!stopUnfoldingLayoutSelected, NULL);
        else if (SvValueIsInteger(unfoldableVal))
            val = SvValueCreateWithInteger(!stopUnfoldingLayoutSelected, NULL);

        if (val)
            SvDBRawObjectSetAttrValue((SvDBRawObject) service, "unfoldable", (SvObject) val);
    }

    if (!val)
        val = SvValueCreateWithBoolean(!stopUnfoldingLayoutSelected, NULL);

    SvDBRawObjectSetAttrValue((SvDBRawObject) service, "enableSlaveStaticCategories", (SvObject) val);
    SVRELEASE(val);
}

void
QBInitLogicServiceMonitorCreate(QBInitLogic self)
{
    self->appGlobals->serviceMonitor = QBMWServiceMonitorCreate(self->appGlobals->middlewareManager, self->appGlobals->scheduler, 12 * 3600, NULL);
    QBMWServiceMonitorSetServiceAdapter(self->appGlobals->serviceMonitor, (SvObject) self, QBInitLogicAdaptService);
}

SvLocal void QBInitLogicSwitchStarted(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBInitLogic self = (QBInitLogic) self_;
    if (self->appGlobals->newTV && self->appGlobals->newTV == to) {
        QBGlobalPlayerControllerResumeCurrentController(self->appGlobals->playerController);
        QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvObject) self);
    }
}

SvLocal void QBInitLogicSwitchEnded(SvObject self_, QBWindowContext from, QBWindowContext to)
{
}

SvType QBInitLogic_getType(void)
{
    static SvType type = NULL;
    static const struct QBMiddlewareManagerListener_t middlewareManagerListenerMethods = {
        .middlewareDataChanged = QBInitLogicMiddlewareDataChanged
    };
    static const struct QBContextSwitcherListener_t switcherMethods = {
        .started = QBInitLogicSwitchStarted,
        .ended   = QBInitLogicSwitchEnded
    };

    if (!type) {
        SvTypeCreateManaged("QBInitLogic",
                            sizeof(struct QBInitLogic_t),
                            SvObject_getType(), &type,
                            QBMiddlewareManagerListener_getInterface(), &middlewareManagerListenerMethods,
                            QBContextSwitcherListener_getInterface(), &switcherMethods,
                            NULL);
    }
    return type;
}

SvLocal SvString QBInitLogicChannelNamePreRenderCallback(SvObject self_, SvTVChannel channel)
{
    QBInitLogic self = (QBInitLogic) self_;
    SvString str = SVRETAIN(channel->name);
    QBTextRendererAddPersistentText(self->appGlobals->textRenderer, str);
    return str;
}

SvLocal SvString QBInitLogicChannelNumberPreRenderCallback(SvObject self_, SvTVChannel channel)
{
    QBInitLogic self = (QBInitLogic) self_;
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    int number = QBPlaylistManagerGetGlobalNumberOfChannel(playlists, channel);
    SvString str = SvStringCreateWithFormat("%03d", number);
    QBTextRendererAddPersistentText(self->appGlobals->textRenderer, str);
    return str;
}

QBInitLogic QBInitLogicCreate(AppGlobals appGlobals)
{
    QBInitLogic self = (QBInitLogic) SvTypeAllocateInstance(QBInitLogic_getType(), NULL);
    self->appGlobals = appGlobals;
    self->wizardEntered = false;

    return self;
}

const char** QBInitLogicGetTextDomains(QBInitLogic self)
{
    static const char* domains[] = {"CubiTV", "CubiTV-Client", "iso-codes", NULL};
    return domains;
}

void QBInitLogicCreatePVRProvider(QBInitLogic self, SvHashTable whitelist)
{
    if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBDiskPVRProvider"))) {
        SvLogNotice("Creating disk PVR provider");
        QBDiskPVRProviderParams params = {
            .super_ = {
                .layout = QBPVRProviderLayout_CubiTV,
                .addSearchToStaticCategories = true,
                .addSearchToDirectories = true,
                .searchProviderTest = {
                    .callback = QBRecordingUtilsPVRSearchProviderTest,
                    .prv = self->appGlobals,
                },
            },
            .tunerCnt = QBTunerLogicGetTunerCount(self->appGlobals->tunerLogic),
            .limits = {
                .maxNormalDirsCnt = -1,     // default count
                .maxKeywordsDirsCnt = -1,   // default count
                .maxDirsCnt = -1,           // default count
                .maxRecordingsCnt = -1      // default count
            }
        };

        self->appGlobals->pvrProvider = (QBPVRProvider) QBDiskPVRProviderCreate(self->appGlobals,
                                                                                self->appGlobals->scheduler,
                                                                                params,
                                                                                NULL);

        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        self->appGlobals->schedManTunerReservation = QBSchedManTunerReservationCreate(self->appGlobals, epgManager);
        self->appGlobals->pvrAgent = QBPVRAgentCreate(self->appGlobals);
        if (!QBPVRLogicHasInternalStorage() && (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBPVRMounter")))) {
            SvLogNotice("No internal disk - switching to PVR-ready mode");
            SvString possiblePartitionsPath = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/possiblePartitions", SvGetRuntimePrefix());
            self->appGlobals->PVRMounter = QBPVRMounterCreate(self->appGlobals->scheduler, possiblePartitionsPath);
            SVRELEASE(possiblePartitionsPath);
        }

        if (!whitelist || SvHashTableFind(whitelist, (SvObject) SVSTRING("QBReencryptionService"))) {
            QBDiskPvrStorageMode pvrStorageMode = QBDiskPvrStorageModeGet();
            bool pvrReencryptionEnabled = pvrStorageMode == QBDiskPvrStorageMode_dvbOnlyReencryption || pvrStorageMode == QBDiskPvrStorageMode_reencryption;
            if (pvrReencryptionEnabled) {
                SvLogNotice("Creating Reencryption Service");
                self->appGlobals->reencryptionService = QBReencryptionServiceCreate(self->appGlobals->scheduler);
            }
        }
    }
}

void QBInitLogicPostInit(QBInitLogic self, SvHashTable whitelist)
{
    QBTextPreRenderer prerenderer = self->appGlobals->textPrerenderer;
    if (prerenderer) {
        // for OSD
        QBTextPreRendererAddChannelsPreRender(prerenderer, QBInitLogicChannelNamePreRenderCallback, (SvObject) self, "osd.settings", "TVOSD.frame.name");
        QBTextPreRendererAddChannelsPreRender(prerenderer, QBInitLogicChannelNumberPreRenderCallback, (SvObject) self, "osd.settings", "TVOSD.frame.number");
        // for channel list (EPG has same font settings)
        QBTextPreRendererAddChannelsPreRender(prerenderer, QBInitLogicChannelNamePreRenderCallback, (SvObject) self, "ChannelMenu.settings", "ChannelName");
        QBTextPreRendererAddChannelsPreRender(prerenderer, QBInitLogicChannelNumberPreRenderCallback, (SvObject) self, "ChannelMenu.settings", "ChannelName");
    }

    QBCASType casType = SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType);
    if (casType == QBCASType_latens) {
        QBLatensMode latensMode;
        const char* latensModeStr = QBConfigGet("LATENS_MODE");
        if (latensModeStr && !strcmp(latensModeStr, "IP")) {
            latensMode = QBLatensMode_ip;
        } else if (latensModeStr && !strcmp(latensModeStr, "DVB")) {
            latensMode = QBLatensMode_dvb;
        } else {
            latensMode = QBLatensMode_ott;
        }

        QBLatensSetMode(QBLatensGetInstance(), latensMode);
        if (latensMode == QBLatensMode_ip || latensMode == QBLatensMode_ott) {
            QBLatensLibLoaderLoadLibrary(QBLatensLibLoaderLibraryType_ip);
        } else {
            QBLatensLibLoaderLoadLibrary(QBLatensLibLoaderLibraryType_dvb);
        }
    }

    if (self->appGlobals->qbMWConfigHandler) {
        SvObject conaxPlugin = (SvObject) ConaxQBMWConfigHandlerPluginCreate(NULL);
        if (conaxPlugin) {
            QBMWConfigHandlerAddPlugin(self->appGlobals->qbMWConfigHandler, conaxPlugin, SVSTRING(QBMW_CONFIG_HANDLER_PLUGIN_CONAX));
            SVRELEASE(conaxPlugin);
        }
    }
}

void QBInitLogicMinimalPostInit(QBInitLogic self, SvHashTable whitelist)
{
    if (self->appGlobals->qbMWConfigHandler) {
        SvObject appModePlugin = (SvObject) QBMWConfigHandlerPluginAppModeCreate(NULL);
        if (appModePlugin) {
            QBMWConfigHandlerAddPlugin(self->appGlobals->qbMWConfigHandler, appModePlugin, SVSTRING(QBMW_CONFIG_HANDLER_PLUGIN_APPMODE));
            SVRELEASE(appModePlugin);
        }
    }
}

void QBInitLogicPostStart(QBInitLogic self)
{
}

void QBInitLogicPreStop(QBInitLogic self)
{
}

void QBInitLogicPreDestroy(QBInitLogic self, bool terminate)
{
}

void QBInitLogicCreateAccessPlugin(QBInitLogic self)
{
    SvObject plugin = SvTypeAllocateInstance(PCAccessPlugin_getType(), NULL);
    PCAccessPluginInit((PCAccessPlugin) plugin, 5, NULL);
    QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("PC"), NULL);
    QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("PC_MENU"), NULL);
    QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("PC_VOD"), NULL);
    QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("VOD_AUTH"), NULL);
    QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("VOD_Login"), NULL);
    SVRELEASE(plugin);

    plugin = QBCASCreateAccessPlugin(self->appGlobals);

    if (plugin) {

        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("DRM"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("DRM"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("ConaxCachedPIN"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("ConaxCachedPIN"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("ConaxMaturity"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("ConaxMaturity"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("ConaxTokenDebit"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("ConaxTokenDebit"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("ConaxAcceptViewing"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("ConaxAcceptViewing"));

        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_ScInternalPin"), NULL);
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_NonScInternalPin"), NULL);
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_PC"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("QBViewRightPinType_PC"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_NonScPin"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("QBViewRightPinType_NonScPin"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_IPPV_PC_1"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("QBViewRightPinType_IPPV_PC_1"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_STBLockPin"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("QBViewRightPinType_STBLockPin"));
        QBAccessManagerSetPluginByDomain(self->appGlobals->accessMgr, plugin, SVSTRING("QBViewRightPinType_HomeShopping"), NULL);
        QBAccessManagerSetPinAttemptsAsUnlimited(self->appGlobals->accessMgr, SVSTRING("QBViewRightPinType_HomeShopping"));
    }

    SVTESTRELEASE(plugin);
}

SvLocal void QBInitLogicCreateDVBChannelsPlugin(QBInitLogic self);
SvLocal void QBInitLogicCreateEITEpgPlugin(QBInitLogic self);
SvLocal void QBInitLogicCreateInnov8onChannelsPlugin(QBInitLogic self);
SvLocal void QBInitLogicCreateInnov8onEPGPlugin(QBInitLogic self);
SvLocal void QBInitLogicCreateCubiwareNPVRPlugin(QBInitLogic self, SvEPGManager epgManager);
SvLocal void QBInitLogicCreateTraxisEPGPlugin(QBInitLogic self);
SvLocal void QBInitLogicCreateTraxisNPVRPlugin(QBInitLogic self, SvEPGManager epgManager);
SvLocal void QBInitLogicCreateTraxisCUTVPlugin(QBInitLogic self, SvEPGManager epgManager);

void QBInitLogicCreateEPGManager(QBInitLogic self)
{
    self->appGlobals->mapper = (GenericEITChannelMapper)SvTypeAllocateInstance(GenericEITChannelMapper_getType(), NULL);
    GenericEITChannelMapperInit(self->appGlobals->mapper, false, false, NULL);

    {
        const char *channelsProviderDVB = QBConfigGet("PROVIDERS.CHANNELS.DVB");
        const char *channelsProviderIP = QBConfigGet("PROVIDERS.CHANNELS.IP");

        if (channelsProviderDVB || channelsProviderIP) {
            if (channelsProviderDVB && strcmp(channelsProviderDVB, "enabled") == 0 && (QBAppTypeIsDVB() || QBAppTypeIsHybrid())) {
                QBInitLogicCreateDVBChannelsPlugin(self);
            }
            if (channelsProviderIP && strcmp(channelsProviderIP, "cubimw") == 0 && (QBAppTypeIsIP() || QBAppTypeIsHybrid())) {
                QBInitLogicCreateInnov8onChannelsPlugin(self);
            }
        } else {
            SvLogWarning("Info about channels provider missing. Falling back to appType based selection.");
            if (QBAppTypeIsDVB() || QBAppTypeIsHybrid())
                QBInitLogicCreateDVBChannelsPlugin(self);
            if (QBAppTypeIsIP() || QBAppTypeIsHybrid())
                QBInitLogicCreateInnov8onChannelsPlugin(self);
        }
    }

    {
        const char *epgProviderDVB = QBConfigGet("PROVIDERS.EPG.DVB");
        const char *epgProviderIP = QBConfigGet("PROVIDERS.EPG.IP");

        if (epgProviderDVB || epgProviderIP) {
            if ((QBAppTypeIsDVB() || QBAppTypeIsHybrid()) && epgProviderDVB && strcmp(epgProviderDVB, "eit") == 0)
                QBInitLogicCreateEITEpgPlugin(self);
            if ((QBAppTypeIsIP() || QBAppTypeIsHybrid()) && epgProviderIP) {
                if (strcmp(epgProviderIP, "cubimw") == 0)
                    QBInitLogicCreateInnov8onEPGPlugin(self);
                if (strcmp(epgProviderIP, "traxis") == 0)
                    QBInitLogicCreateTraxisEPGPlugin(self);
            }
        } else {
            SvLogWarning("Info about EPG provider missing. Falling back to appType based selection.");
            if (QBAppTypeIsDVB() || QBAppTypeIsHybrid())
                QBInitLogicCreateEITEpgPlugin(self);
            if (QBAppTypeIsIP() || QBAppTypeIsHybrid())
                QBInitLogicCreateInnov8onEPGPlugin(self);
        }
    }

    SvLogNotice("Creating EPG manager and channel merger (Basic)");
    self->appGlobals->channelMerger = (SvObject) SvTypeAllocateInstance(SvChannelMergerBasic_getType(), NULL);
    SvChannelMergerBasicInit((SvChannelMergerBasic) self->appGlobals->channelMerger);
    SvEPGManager epgManager = SvEPGManagerCreate(self->appGlobals->channelMerger, NULL);
    bool anyPluginAdded = false;
    if (self->appGlobals->epgPlugin.IP.channelList && self->appGlobals->epgPlugin.IP.data) {
        SvEPGManagerAddPlugin(epgManager, self->appGlobals->epgPlugin.IP.channelList, self->appGlobals->epgPlugin.IP.data, NULL);
        anyPluginAdded = true;
    }
    if (self->appGlobals->epgPlugin.DVB.channelList && self->appGlobals->epgPlugin.DVB.data) {
        SvEPGManagerAddPlugin(epgManager, self->appGlobals->epgPlugin.DVB.channelList, self->appGlobals->epgPlugin.DVB.data, NULL);
        anyPluginAdded = true;
    }
    if (self->appGlobals->epgPlugin.IP.channelList && !self->appGlobals->epgPlugin.IP.data &&
        !self->appGlobals->epgPlugin.DVB.channelList && self->appGlobals->epgPlugin.DVB.data) {
        SvEPGManagerAddPlugin(epgManager, self->appGlobals->epgPlugin.IP.channelList, self->appGlobals->epgPlugin.DVB.data, NULL);
        anyPluginAdded = true;
    }
    if (!self->appGlobals->epgPlugin.IP.channelList && self->appGlobals->epgPlugin.IP.data &&
        self->appGlobals->epgPlugin.DVB.channelList && !self->appGlobals->epgPlugin.DVB.data) {
        SvEPGManagerAddPlugin(epgManager, self->appGlobals->epgPlugin.DVB.channelList, self->appGlobals->epgPlugin.IP.data, NULL);
        anyPluginAdded = true;
    }

    if (!anyPluginAdded) {
        SvLogError("%s()[%d]: No plugin added to SvEPGManager", __func__, __LINE__);
        abort();
    }

    if (self->appGlobals->epgPlugin.DVB.channelList) {
        int pluginID = SvInvokeInterface(SvEPGChannelListPlugin, self->appGlobals->epgPlugin.DVB.channelList, getID);
        SvInvokeInterface(SvChannelMerger, self->appGlobals->channelMerger, setPreferredPluginID, pluginID, NULL);
    }

    /// customize EPG memory-management

    SvEPGManagerSetOOMStrategy(epgManager,
                               true,   // adjust time-limits dynamically
                               false); // do not drop unused chunks

    // don't keep EPG events older than 7 days and further in the future than 8 days
    SvEPGManagerSetTimeLimit(epgManager, -7 * 24 * 3600, 8 * 24 * 3600, NULL);

    // limit memory usage for EPG events to 14 MB: on average EPG event received from EIT
    // uses 700 B of memory
    SvEPGManagerSetMaxEventsCount(epgManager, 20 * 1000, NULL);

    {
        const char *npvrProvider = QBConfigGet("PROVIDERS.NPVR");
        if (npvrProvider) {
            if (strcmp(npvrProvider, "cubimw") == 0 && (QBAppTypeIsIP() || QBAppTypeIsHybrid()))
                QBInitLogicCreateCubiwareNPVRPlugin(self, epgManager);
            if (strcmp(npvrProvider, "traxis") == 0 && (QBAppTypeIsIP() || QBAppTypeIsHybrid()))
                QBInitLogicCreateTraxisNPVRPlugin(self, epgManager);
            // other npvr plugins here
        } else {
            SvLogWarning("Info about nPVR provider missing. Falling back to appType based selection.");
            if (QBAppTypeIsIP() || QBAppTypeIsHybrid())
                QBInitLogicCreateCubiwareNPVRPlugin(self, epgManager);
        }
    }

    {
        const char *cutvProvider = QBConfigGet("PROVIDERS.CUTV");
        if (cutvProvider) {
            if (strcmp(cutvProvider, "traxis") == 0 && (QBAppTypeIsIP() || QBAppTypeIsHybrid())) {
                QBInitLogicCreateTraxisCUTVPlugin(self, epgManager);
            }
            if (strcmp(cutvProvider, "cubimw") == 0 && (QBAppTypeIsIP() || QBAppTypeIsHybrid())) {
                SvLogNotice("Creating Cubiware CatchUpTV manager");
                Innov8onCutvPlugin cutvPlugin = (Innov8onCutvPlugin) SvTypeAllocateInstance(Innov8onCutvPlugin_getType(), NULL);
                Innov8onCutvPluginInit(cutvPlugin, NULL);
                self->appGlobals->cutvManager = (CutvManager *) cutvPlugin;
            }
            // other cutv plugins here
        } else {
            SvLogWarning("Info about CUTV provider missing.");
        }
    }
    QBSchedManagerSetEventsSource(QBSchedManagerEventSource_PresentFollowing);

    QBServiceRegistryRegisterService(QBServiceRegistryGetInstance(), (SvObject) epgManager, NULL);
    SVRELEASE(epgManager);
}

SvLocal void QBInitLogicCreateDVBChannelsPlugin(QBInitLogic self)
{
    SvLogNotice("Creating DVB EPG plugin");

    SvErrorInfo error = NULL;
    DVBEPGPlugin plugin = (DVBEPGPlugin) SvTypeAllocateInstance(DVBEPGPlugin_getType(), NULL);
    DVBEPGPluginInit(plugin, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(plugin);
        return;
    }

    {
        SvLogNotice("Creating NorDig TV channels list merger");
        QBNordigLCN tvMerger = (QBNordigLCN) SvTypeAllocateInstance(QBNordigLCN_getType(), NULL);
        QBNordigLCNInit(tvMerger, (SvObject) plugin, SVSTRING("TV"), -1);
        DVBEPGPluginAddChannelNumberMerger(plugin, (SvObject) tvMerger);
        SVRELEASE(tvMerger);
    }

    {
        SvLogNotice("Creating radio merger");
        QBNordigLCN radioMerger = (QBNordigLCN) SvTypeAllocateInstance(QBNordigLCN_getType(), NULL);
        QBNordigLCNInit(radioMerger, (SvObject) plugin, SVSTRING("Radio"), -1);
        DVBEPGPluginAddChannelNumberMerger(plugin, (SvObject) radioMerger);
        SVRELEASE(radioMerger);
    }

    self->appGlobals->epgPlugin.DVB.channelList = (SvObject) plugin;
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBGlobalStorageSetItem(globalStorage, DVBEPGPlugin_getType(), (SvObject) plugin, NULL);

    SvInvokeInterface(SvEPGChannelListPlugin, plugin, setChannelMetaFillHelper, QBChannelMetaFill, self->appGlobals);
    SvInvokeInterface(SvEPGChannelListPlugin, plugin, addListener, (SvObject) self->appGlobals->mapper, true, NULL);
}

SvLocal void QBInitLogicCreateEITEpgPlugin(QBInitLogic self)
{
    SvLogNotice("Creating generic EIT EPG plugin");

    SvErrorInfo error = NULL;
    GenericEITPlugin plugin = (GenericEITPlugin) SvTypeAllocateInstance(GenericEITPlugin_getType(), NULL);
    GenericEITPluginInit(plugin, (SvObject)self->appGlobals->mapper, QBTunerLogicGetTunerCount(self->appGlobals->tunerLogic), &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(plugin);
        return;
    }

    self->appGlobals->epgPlugin.DVB.data = (SvObject) plugin;
}

SvLocal void QBInitLogicCreateInnov8onChannelsPlugin(QBInitLogic self)
{
    bool obtainEntitledContentOnly = true;

    SvLogNotice("Creating Innov8on EPG channel list plugin");

    SvErrorInfo error = NULL;
    Innov8onEPGChannelListPlugin plugin = (Innov8onEPGChannelListPlugin) SvTypeAllocateInstance(Innov8onEPGChannelListPlugin_getType(), NULL);
    Innov8onEPGChannelListPluginInit(plugin, NULL, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(plugin);
        return;
    }

    Innov8onEPGChannelListPluginSetRefreshPeriod(plugin, 1 * 60 * 60 * 1000, NULL);
    Innov8onEPGChannelListPluginObtainEntitledOnly(plugin, obtainEntitledContentOnly);
    {
        QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager, (SvObject) self);
        SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        SvString prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        SvString id = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        QBInitLogicInnov8onEPGChannelListPluginSetup(plugin, serverInfo, prefixUrl, id);
    }
    self->appGlobals->epgPlugin.IP.channelList = (SvObject) plugin;

    SvInvokeInterface(SvEPGChannelListPlugin, plugin, setChannelMetaFillHelper, QBChannelMetaFill, self->appGlobals);
    SvInvokeInterface(SvEPGChannelListPlugin, plugin, addListener, (SvObject) self->appGlobals->mapper, true, NULL);
}

SvLocal void QBInitLogicCreateInnov8onEPGPlugin(QBInitLogic self)
{
    SvLogNotice("Creating Innov8on EPG data plugin");

    SvErrorInfo error = NULL;
    Innov8onEPGDataPlugin plugin = (Innov8onEPGDataPlugin) SvTypeAllocateInstance(Innov8onEPGDataPlugin_getType(), NULL);
    Innov8onEPGDataPluginInit(plugin, NULL, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(plugin);
        return;
    }

    {
        SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        SvString id = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        QBInitLogicInnov8onEPGDataPluginSetup(plugin, serverInfo, id);
    }
    self->appGlobals->epgPlugin.IP.data = (SvObject) plugin;
}

SvLocal void QBInitLogicCreateCubiwareNPVRPlugin(QBInitLogic self, SvEPGManager epgManager)
{
    SvLogNotice("Creating cubiware nPVR provider");

    if (!self->appGlobals->epgPlugin.IP.channelList) {
        SvLogError("IP channel list plugin is not enabled. Couldn't create cubiware nPVR provider");
        return;
    }

    SvString id = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);

    SvErrorInfo error = NULL;
    CubiwareMWNPvrProvider cubiwareNPVRProvider = (CubiwareMWNPvrProvider) SvTypeAllocateInstance(CubiwareMWNPvrProvider_getType(), NULL);
    CubiwareMWNPvrProviderInit(cubiwareNPVRProvider, self->appGlobals->customerInfoMonitor, id, self->appGlobals->middlewareManager, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(cubiwareNPVRProvider);
        return;
    }

    self->appGlobals->nPVRProvider = (SvObject) cubiwareNPVRProvider;

    unsigned int ipPluginID = SvInvokeInterface(SvEPGChannelListPlugin, self->appGlobals->epgPlugin.IP.channelList, getID);
    QBnPVRProviderInitParams params = {
        .refreshRecordingsAfterMs               = 120 * 60 * 1000, // 2 h
        .refreshRecordingsAfterChannelsChangeMs = 5 * 1000,        // 5 s
        .firstRecordingRefreshDelayMs           = 15 * 60 * 1000,  // 15 min
        .channelsPluginID                       = ipPluginID
    };
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, init, params, epgManager);
}

SvLocal void QBInitLogicCreateTraxisEPGPlugin(QBInitLogic self)
{
    SvLogNotice("Creating Traxis EPG plugin");
    TraxisWebEPGDataPluginInitConfig  initConfig = {
        .productsRefreshPeriod              = 30 * 60,
        .productsRefreshPeriodFailedRetry   = 2,
        .eventsExpirationTimeout            = -1,
        .eventsExpirationTimeoutFailedRetry = -1,
    };

    SvErrorInfo error = NULL;
    TraxisWebEPGDataPlugin traxisDataPlugin = (TraxisWebEPGDataPlugin) SvTypeAllocateInstance(TraxisWebEPGDataPlugin_getType(), NULL);
    TraxisWebEPGDataPluginInit(traxisDataPlugin, self->appGlobals->traxisWebSessionManager, &initConfig, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(traxisDataPlugin);
        return;
    }

    self->appGlobals->epgPlugin.IP.data = (SvObject) traxisDataPlugin;
}

SvLocal void QBInitLogicCreateTraxisNPVRPlugin(QBInitLogic self, SvEPGManager epgManager)
{
    SvLogNotice("Creating Traxis nPVR plugin");
    TraxisWebNPvrProviderInitConfig initConfig = {
        .recordingsRefreshPeriod            = 60 * 60,
        .recordingsRefreshPeriodFailedRetry = 5,
        .quotaRefreshPeriod                 = 60 * 60,
        .quotaRefreshPeriodFailedRetry      = 5,
        .playableWhenRecording              = false, // unable to play until is "completed"
    };

    SvErrorInfo error = NULL;
    TraxisWebNPVRProvider* traxisNPvrPlugin = (TraxisWebNPVRProvider*) SvTypeAllocateInstance(TraxisWebNPVRProvider_getType(), NULL);
    TraxisWebNPVRProviderInit(traxisNPvrPlugin, self->appGlobals->traxisWebSessionManager, &initConfig, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(traxisNPvrPlugin);
        return;
    }

    self->appGlobals->nPVRProvider = (SvObject) traxisNPvrPlugin;

    NPvrStatsConfig  statsConfig = {
        /// 1 quota unit = 1/30 GB
        .quota = {
            .unitType   = NPvrQuotaUnit_GB,
            .scaleNom   = 1,
            .scaleDenom = 30,
        }
    };

    TraxisWebNPVRProviderSetStatsConfig(traxisNPvrPlugin, &statsConfig);

    QBnPVRProviderInitParams params = {
        .refreshRecordingsAfterMs = 0,
    };
    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, init, params, epgManager);
}

SvLocal void QBInitLogicCreateTraxisCUTVPlugin(QBInitLogic self, SvEPGManager epgManager)
{
    SvLogNotice("Creating Traxis CUTV plugin");

    SvErrorInfo error = NULL;
    TraxisWebCutvPlugin traxisCutvPlugin = (TraxisWebCutvPlugin) SvTypeAllocateInstance(TraxisWebCutvPlugin_getType(), NULL);
    TraxisWebCutvPluginInit(traxisCutvPlugin, self->appGlobals->traxisWebSessionManager, epgManager, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(traxisCutvPlugin);
        return;
    }

    self->appGlobals->cutvManager = (CutvManager*) traxisCutvPlugin;
}

SvLocal SvObject QBInitLogicLoadPlaylist(void *self_, SvString id, SvString name, SvString type, SvString filename)
{
    SvEPGPersistentView list = (SvEPGPersistentView) SvTypeAllocateInstance(SvEPGPersistentView_getType(), NULL);
    SvEPGPersistentViewInit((SvEPGPersistentView) list, filename, SvEPGViewChannelNumbering_byNumber, NULL);

    return (SvObject) list;
}

void QBInitLogicCreatePlaylists(QBInitLogic self)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    QBPlaylistManagerSetLoadFunc(playlists, QBInitLogicLoadPlaylist, self);

    SvValue trueValue = SvValueCreateWithBoolean(true, NULL);

    SvLogNotice("Creating tv playlist");
    SvEPGHashTableFilter tvFilter = (SvEPGHashTableFilter) SvTypeAllocateInstance(SvEPGHashTableFilter_getType(), NULL);
    SvEPGHashTableFilterInit(tvFilter, NULL);
    SvEPGHashTableFilterAddFilterAttribute((SvEPGHashTableFilter) tvFilter, (SvObject) SVSTRING("TV"), (SvObject) trueValue);
    SvObject channelView = SvTypeAllocateInstance(SvEPGSortingView_getType(), NULL);
    SvEPGSortingViewInit((SvEPGSortingView) channelView, (SvObject) tvFilter, SvEPGViewChannelNumbering_byNumber, NULL);
    SvString listName = SvStringCreate("TV", NULL);
    QBPlaylistManagerAdd(playlists, (SvObject) channelView, SVSTRING("TVChannels"), listName, SVSTRING("TV"), true);
    SVRELEASE(listName);
    SVRELEASE(channelView);
    SVTESTRELEASE(tvFilter);

    SvLogNotice("Creating radio playlist");
    SvObject radioView = NULL;
    SvEPGHashTableFilter radioFilter = (SvEPGHashTableFilter) SvTypeAllocateInstance(SvEPGHashTableFilter_getType(), NULL);
    SvEPGHashTableFilterInit(radioFilter, NULL);
    SvEPGHashTableFilterAddFilterAttribute((SvEPGHashTableFilter) radioFilter, (SvObject) SVSTRING("Radio"), (SvObject) trueValue);
    radioView = SvTypeAllocateInstance(SvEPGSortingView_getType(), NULL);
    SvEPGSortingViewInit((SvEPGSortingView) radioView, (SvObject) radioFilter, SvEPGViewChannelNumbering_byNumber, NULL);
    SVTESTRELEASE(radioFilter);
    listName = SvStringCreate("Radio", NULL);
    QBPlaylistManagerAdd(playlists, (SvObject) radioView, SVSTRING("RadioChannels"), listName, SVSTRING("Radio"), true);
    SVRELEASE(listName);
    SVRELEASE(radioView);

    if ((QBAppTypeIsIP() || QBAppTypeIsHybrid()) && self->appGlobals->epgPlugin.IP.channelList) {
        SvLogNotice("Creating IP TV playlist");
        unsigned int ipPluginID = SvInvokeInterface(SvEPGChannelListPlugin, self->appGlobals->epgPlugin.IP.channelList, getID);

        SvObject iptvView = NULL;
        SvEPGSourcePluginFilter iptvFilter = (SvEPGSourcePluginFilter) SvTypeAllocateInstance(SvEPGSourcePluginFilter_getType(), NULL);
        SvEPGSourcePluginFilterInit(iptvFilter, ipPluginID, NULL);

        iptvView = SvTypeAllocateInstance(SvEPGSortingView_getType(), NULL);
        SvEPGSortingViewInit((SvEPGSortingView) iptvView, (SvObject) iptvFilter, SvEPGViewChannelNumbering_byNumber, NULL);

        SVRELEASE(iptvFilter);

        listName = SvStringCreate("IPTV", NULL);
        QBPlaylistManagerAdd(playlists, (SvObject) iptvView, SVSTRING("IPChannels"), listName, SVSTRING("TV"), true);
        SVRELEASE(listName);

        SvValue isInternalValue = SvValueCreateWithBoolean(true, NULL);
        QBPlaylistManagerSetPlaylistMetaData(playlists, (SvObject) iptvView, (SvObject) SVSTRING("isInternal"), (SvObject) isInternalValue);
        SVRELEASE(isInternalValue);

        SVRELEASE(iptvView);
    }

    SvLogNotice("Creating pvr playlist");
    SvObject pvrView = SvTypeAllocateInstance(SvEPGSortingView_getType(), NULL);
    SvEPGHashTableFilter pvrHashFilter = (SvEPGHashTableFilter) SvTypeAllocateInstance(SvEPGHashTableFilter_getType(), NULL);
    SvEPGNegativeFilter pvrFilter = (SvEPGNegativeFilter) SvTypeAllocateInstance(SvEPGNegativeFilter_getType(), NULL);
    SvEPGHashTableFilterInit(pvrHashFilter, NULL);
    SvEPGHashTableFilterAddFilterAttribute(pvrHashFilter, (SvObject) SVSTRING("adaptive_streaming"), (SvObject) trueValue);
    SvEPGNegativeFilterInit(pvrFilter, (SvObject) pvrHashFilter, NULL);
    SvEPGSortingViewInit((SvEPGSortingView) pvrView, (SvObject) pvrFilter, SvEPGViewChannelNumbering_byNumber, NULL);
    listName = SvStringCreate("PVR", NULL);
    QBPlaylistManagerAdd(playlists, (SvObject) pvrView, SVSTRING("PVRChannels"), listName, SVSTRING("PVR"), true);
    SVRELEASE(listName);
    QBPlaylistManagerSetPlaylistMetaData(playlists, (SvObject) pvrView, (SvObject) SVSTRING("isInternal"), (SvObject) trueValue);
    SVTESTRELEASE(pvrFilter);
    SVRELEASE(pvrHashFilter);
    SVRELEASE(pvrView);

    SvLogNotice("Creating list of all radio and TV channels");
    SvEPGTVOrRadioFilter filter = (SvEPGTVOrRadioFilter) SvTypeAllocateInstance(SvEPGTVOrRadioFilter_getType(), NULL);
    SvEPGTVOrRadioFilterInit(filter, NULL);
    channelView = SvTypeAllocateInstance(SvEPGSortingView_getType(), NULL);
    SvEPGSortingViewInit((SvEPGSortingView) channelView, (SvObject) filter, SvEPGViewChannelNumbering_byNumber, NULL);
    SVRELEASE(filter);
    QBPlaylistManagerAdd(playlists, (SvObject) channelView, SVSTRING("AllChannels"),
                         SVSTRING("AllChannels"), SVSTRING("AllChannels"), true);
    QBPlaylistManagerSetPlaylistMetaData(playlists, (SvObject) channelView, (SvObject) SVSTRING("isInternal"), (SvObject) trueValue);
    SVRELEASE(trueValue);
    SVRELEASE(channelView);

    QBPlaylistManagerSetCurrent(playlists, SVSTRING("TVChannels"));
}

void QBInitLogicCreateContentManagersServices(QBInitLogic self)
{
    QBContentManagersService contentManagers = (QBContentManagersService) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                      SVSTRING("QBContentManagersService"));
    if (!contentManagers) {
        return;
    }

    const char* vodProvider = QBConfigGet("PROVIDERS.VOD");
    const char* webServicesProvider = QBConfigGet("PROVIDERS.WEBSERVICES");

    if ((vodProvider && strcmp(vodProvider, "cubimw") == 0) ||
        (webServicesProvider && strcmp(webServicesProvider, "cubimw") == 0)) {
        // if both exists and none is 'cubimw' then don't create CubiwareMWContentManager.

        SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager,
                                                                               QBMiddlewareManagerType_Innov8on);
        SvObject contentManager = QBContentManagersServiceFindFirst(contentManagers,
                                                                    QBContentManagerType_cubiware);
        if (!contentManager && serverInfo) {
            QBInitLogicCreateCubiwareMWContentManager(self, serverInfo);
        }
    }
    if (vodProvider && strncmp(vodProvider, "digitalsmiths", 13) == 0) {
        QBInitLogicCreateDigitalSmithsContentManager(self);
    }
}

bool QBInitLogicShouldEnterWizard(QBInitLogic self)
{
    const char *firstBoot = QBConfigGet("FIRSTBOOT");
    if (firstBoot && !strcmp(firstBoot, "YES"))
        return true;
    else
        return false;
}

void QBInitLogicEnterWizard(QBInitLogic self)
{
    QBWindowContext ctx = QBWizardContextCreate(self->appGlobals);

    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
    SVRELEASE(ctx);
    self->wizardEntered = true;
    self->wasWizardEntered = true;

    QBWizardLogicEnterRCUPairing(self->appGlobals->wizardLogic);
}

void QBInitLogicLeaveWizard(QBInitLogic self)
{
    self->wizardEntered = false;
}

bool QBInitLogicIsWizardFinished(QBInitLogic self)
{
    return !self->wizardEntered;
}

SvLocal bool QBInitLogicCheckUpgradeMarker(const char *markerFilePath, bool *isUpgradeOK)
{
    char *upgradeStatusStr = NULL;
    QBFileToBuffer(markerFilePath, &upgradeStatusStr);

    if (upgradeStatusStr == NULL)
        return false;

    *isUpgradeOK = (strcmp(upgradeStatusStr, "OK") == 0);

    free(upgradeStatusStr);
    return true;
}

void QBInitLogicEnterApplication(QBInitLogic self)
{
    AppGlobals appGlobals = self->appGlobals;
    QBInitLogicLeaveWizard(self);
    QBApplicationControllerCleanContextStack(self->appGlobals->controller);

    /// Gather context stack
    QBWindowContext ctxTab[10] = {NULL};
    int ctxCnt = 0;

    if (appGlobals->newTV) {
        ctxTab[ctxCnt++] = SVRETAIN(appGlobals->newTV);
        QBApplicationControllerAddListener(appGlobals->controller, (SvObject) self);
    }

    const int channelsRestoreTimeoutMs = QBAppTypeIsDVB() ? DVB_CHANNELS_RESTORE_TIMEOUT_MS : IP_CHANNELS_RESTORE_TIMEOUT_MS;
    ctxTab[ctxCnt] = QBAppStartupWaiterContextCreate(appGlobals);
    SvObject listener;
    listener = (SvObject) SvEPGChannelListWaiterCreate(self->appGlobals->scheduler, channelsRestoreTimeoutMs, NULL);
    if (listener) {
        QBAppStartupWaiterContextAddPlugin((QBAppStartupWaiterContext) ctxTab[ctxCnt], listener);
        SVRELEASE(listener);
    }
    ctxCnt++;

    bool isUpgradeOK = false;
    const char *upgradeMarkerPath = "/etc/vod/upgrade.stat";
    if (QBInitLogicCheckUpgradeMarker(upgradeMarkerPath, &isUpgradeOK)) {
        const char *message = isUpgradeOK ? "Firmware upgrade successful!" : "Firmware upgrade failed!";
        QBSecureLogEvent("InitLogic", isUpgradeOK ? "NoticeShown.FirmwareUpgrade.OK" : "ErrorShown.FirmwareUpgrade.Failure",
                         "JSON:{\"description\":\"%s\"}", message);
        remove(upgradeMarkerPath);
    }

    SvLogNotice("QBInitLogic : EnterApplication() : initialStandbyState=[%s]", QBStandbyStateDataToString(appGlobals->initialStandbyState));
    if (QBStandbyStateDataGetState(appGlobals->initialStandbyState) != QBStandbyState_on) {
        ctxTab[ctxCnt++] = QBStandbyContextCreate(appGlobals);
    }

    bool networkSettingsNeeded = false;
    QBNetworkWatcher networkWatcher = (QBNetworkWatcher) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBNetworkWatcher"));
    if (networkWatcher) {
        networkSettingsNeeded = (QBNetworkWatcherGetOverallDiagnosis(networkWatcher) != QBNetworkWatcherOverallDiagnosis_ok);
    }
    if (self->wasWizardEntered && networkSettingsNeeded) {
        ctxTab[ctxCnt++] = QBNetworkSettingsContextCreate(appGlobals->res,
                                                          appGlobals->scheduler,
                                                          self,
                                                          appGlobals->controller,
                                                          appGlobals->textRenderer,
                                                          appGlobals->networkMonitor,
                                                          false);
    }

    assert(ctxCnt > 0); // we have to PUSH at least one context
    /// Pass the stack to the controller
    int i;
    for (i=0; i < ctxCnt-1; ++i) {
        QBApplicationControllerPutContext(appGlobals->controller, ctxTab[i]);
        SVRELEASE(ctxTab[i]);
    }

    QBApplicationControllerPushContext(appGlobals->controller, ctxTab[i]);
    SVRELEASE(ctxTab[i]);

    QBWizardLogicEnterRCUPairing(self->appGlobals->wizardLogic);
}

SvLocal
SvString QBInitLogicGetTunerTypeString(void)
{
    const char* tunerConfigSetName = QBConfigGet("DEFAULTTUNERTYPE");
    if (!tunerConfigSetName)
        tunerConfigSetName = "cab";

    /// take first three letters, as we're interested only in general tuner type
    return SvStringCreateWithCStringAndLength(tunerConfigSetName, 3, NULL);
}

SvString QBInitLogicCreateSecureLogConfigFileName(QBInitLogic self)
{
ONLY_ON_NONSECURE({
    SvString tmpPath = SvStringCreateWithFormat("/etc/vod/Configurations/secureLogManager");
    struct stat buf;
    if (stat(SvStringCString(tmpPath), &buf) == 0) {
        return tmpPath;
    } else {
        SVRELEASE(tmpPath);
    }
});
    SvString confFileName = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/secureLogManager", SvGetRuntimePrefix());
    return confFileName;
}

SvString QBInitLogicGetPlayerConfigFileName(QBInitLogic self)
{
ONLY_ON_NONSECURE({

    SvString tmpPath = SvStringCreateWithFormat("/etc/vod/Configurations/playerConf");
    struct stat buf;
    if (stat(SvStringCString(tmpPath), &buf) == 0)  {
        return tmpPath;
    } else {
        SVRELEASE(tmpPath);
    }
});

    SvString confFileName = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/playerConf", SvGetRuntimePrefix());

    return confFileName;
}

SvString QBInitLogicGetChannelScanningConfFileName(QBInitLogic self)
{
    SvString suffix = QBInitLogicGetTunerTypeString();
    SvString confFileName = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/channelScanning-%s", SvGetRuntimePrefix(), SvStringCString(suffix));
    SVRELEASE(suffix);
    return confFileName;
}

void QBInitLogicSetLocale(QBInitLogic self, SvString optVal)
{
    if (!optVal)
        return;
    setlocale(LC_CTYPE, SvStringCString(optVal));
    setlocale(LC_TIME, SvStringCString(optVal));
    setlocale(LC_MESSAGES, SvStringCString(optVal));
    setlocale(LC_COLLATE, SvStringCString(optVal));
    setenv("LANG", SvStringCString(optVal), 1);
    if (self->appGlobals->appState)
        QBAppStateServiceSetLanguage(self->appGlobals->appState);
}

SvString QBInitLogicGetServiceListFileName(QBInitLogic self)
{
    const char *cfg = NULL;
    bool isHybrid = QBAppTypeIsHybrid();
    bool isDVB = QBAppTypeIsDVB();

    if (isHybrid) {
        cfg = "hybrid";
    } else if (!isDVB) {
        cfg = "ip";
    } else if (QBTunerFindConfigSetByName("native", "sat") >= 0) {
        // FIXME: this is HACK, it assumes that if STB has any satellite
        // tuner present we are for sure using only satellite tuners;
        // won't work correctly for hybrid DVB-S+DVB-T or DVB-S+DVB-C boxes
        cfg = "sat";
    } else {
        cfg = "dvb";
    }

    return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/services-%s", SvGetRuntimePrefix(), cfg);
}

SvString QBInitLogicGetLogoWidgetName(QBInitLogic self)
{
    return SVSTRING("CustomerLogo");
}

SvString QBInitLogicGetDiagnosticsOptionsFileName(QBInitLogic self)
{
    return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/diagnostics", SvGetRuntimePrefix());
}

void QBInitLogicCreateBookmarkPlugins(QBInitLogic self)
{
    if (!self->appGlobals->bookmarkManager)
        return;

    // Innov8on plugin

    QBBookmarkType innov8onPluginTypes[] = {QBBookmarkType_Innov8on};
    size_t pluginTypesCnt = sizeof(innov8onPluginTypes)/sizeof(QBBookmarkType);

    QBInnov8onBookmarkManagerPlugin innov8onPlugin = (QBInnov8onBookmarkManagerPlugin) SvTypeAllocateInstance(QBInnov8onBookmarkManagerPlugin_getType(), NULL);
    QBInnov8onBookmarkManagerPluginInit(innov8onPlugin, self->appGlobals);
    QBBookmarkManagerAddPlugin(self->appGlobals->bookmarkManager, (SvObject) innov8onPlugin, innov8onPluginTypes, pluginTypesCnt);
    SVRELEASE(innov8onPlugin);

    // Generic plugin

    QBBookmarkType genericPluginTypes[] = {QBBookmarkType_Generic, QBBookmarkType_Innov8on};
    pluginTypesCnt = sizeof(genericPluginTypes)/sizeof(QBBookmarkType);

    QBGenericBookmarkManagerPlugin genericPlugin = (QBGenericBookmarkManagerPlugin) SvTypeAllocateInstance(QBGenericBookmarkManagerPlugin_getType(), NULL);
    QBGenericBookmarkManagerPluginInit(genericPlugin, SVSTRING("/etc/vod/app/bookmarks"));
    QBBookmarkManagerAddPlugin(self->appGlobals->bookmarkManager, (SvObject) genericPlugin, genericPluginTypes, pluginTypesCnt);
    SVRELEASE(genericPlugin);

    // Disk PVR Recording plugin
    if (self->appGlobals->pvrProvider &&
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type == QBPVRProviderType_disk) {
        QBBookmarkType recordingPluginTypes[] = {QBBookmarkType_Recording};
        pluginTypesCnt = sizeof(recordingPluginTypes)/sizeof(QBBookmarkType);

        QBRecordingBookmarkManagerPlugin recordingPlugin = (QBRecordingBookmarkManagerPlugin) SvTypeAllocateInstance(QBRecordingBookmarkManagerPlugin_getType(), NULL);
        QBRecordingBookmarkManagerPluginInit(recordingPlugin, self->appGlobals->pvrProvider);
        QBBookmarkManagerAddPlugin(self->appGlobals->bookmarkManager, (SvObject) recordingPlugin, recordingPluginTypes, pluginTypesCnt);
        SVRELEASE(recordingPlugin);
    }
}

const QBLongKeyPressExtendedMapping *QBInitLogicGetKeyExtendedMap(QBInitLogic self)
{
   static const QBLongKeyPressExtendedMapping map[] = {
      { QBKEY_EPG,   QBKEY_EPG,   QBKEY_EPG,     1,               false },
      { QBKEY_POWER, QBKEY_POWER, QBKEY_STANDBY, 2 * 1000 * 1000, true  },
      { 0,           0,           0,             0,               false },
    };
    return map;
}

void QBInitLogicCreateMWClientServices(AppGlobals appGlobals)
{
    if (!appGlobals) {
        SvLogError("%s: null appGlobals passed", __func__);
        return;
    }

    appGlobals->middlewareClientService = CubiwareMWClientServiceCreate(appGlobals->middlewareManager);

    if (!appGlobals->middlewareClientService) {
        SvLogError("%s: Can't create CubiwareMWClientService", __func__);
        return;
    }

    CubiwareMWClientServiceConaxPlugin conaxPlugin = CubiwareMWClientServiceCreateConaxPlugin(appGlobals->scheduler,
                                                                                              appGlobals->middlewareManager,
                                                                                              CubiwareMWClientServiceConaxPluginMode_GenerateID);
    if (conaxPlugin) {
        SvObject conaxPlayReady = QBPlayReadyManagerGetImplementation(QBPlayReadyImplementation_Conax, NULL);
        QBConaxPlayReadySetHelperService(conaxPlayReady, (SvObject) conaxPlugin, NULL);

        CubiwareMWClientServiceAddPlugin(appGlobals->middlewareClientService, (SvObject) conaxPlugin);
        SVRELEASE(conaxPlugin);
    }
}

QBPropertiesMap QBInitLogicCreateServiceProperties(QBInitLogic self, SvString serviceName)
{
    QBPropertiesMap propertiesMap = NULL;
    if (serviceName && SvStringEqualToCString(serviceName, "QBDVBBouquets")) {
        propertiesMap = QBPropertiesMapCreate(NULL);
        QBPropertiesMapSetIntProperty(propertiesMap, SVSTRING("bouquetsMaskType"),
            QBDVBBouquetsMaskType_unknown, NULL);
    }
    return propertiesMap;
}

SvString QBInitLogicGetClosedCaptioningOptionsFileName(QBInitLogic self)
{
    return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/closedCaptioning", SvGetRuntimePrefix());
}

SvString QBInitLogicGetUserScanningParamsFileName(QBInitLogic self)
{
    SvString suffix = QBInitLogicGetTunerTypeString();

    SvString scanningParamsFileName = SvStringCreateWithFormat("/etc/vod/app/scanUserParams-%s", SvStringCString(suffix));
    SVRELEASE(suffix);
    return scanningParamsFileName;
}

SvString QBInitLogicGetDefaultScanningParamsFileName(QBInitLogic self)
{
    SvString suffix = QBInitLogicGetTunerTypeString();

    SvString scanningParamsFileName = SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/scanDefaultParams-%s", SvGetRuntimePrefix(), SvStringCString(suffix));
    SVRELEASE(suffix);
    return scanningParamsFileName;
}

SvString QBInitLogicCreateFavoritesConfigurationFileName(QBInitLogic self)
{
    return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/favoritesMenu", SvGetRuntimePrefix());
}

SvString QBInitLogicGetStandbyAgentPluginsFileName(QBInitLogic self)
{
    return SvStringCreateWithFormat("%s/usr/local/share/CubiTV/Configurations/standbyAgentPlugins", SvGetRuntimePrefix());
}

bool QBInitLogicGetAppStartupWaiterTimeout(QBInitLogic self, int* timeout)
{
    return false;
}

void QBInitLogicLoadHlsSSLAuthParams(QBInitLogic self)
{
    QBGlobalStorage gs = QBGlobalStorageGetInstance();
    SvSSLParams sslParamsDefault = SvSSLParamsCreateDefault(NULL);
    if (sslParamsDefault) {
        QBGlobalStorageSetItemByName(gs, SVSTRING("SSLParams"), (SvObject) sslParamsDefault ,NULL);
        SVRELEASE(sslParamsDefault);
    }
}

const char* QBInitLogicGetMWTimeoutText(QBInitLogic self)
{
    return gettext("Unable to connect to MW.");
}

SvObject
QBInitLogicCreateSearchAgent(QBInitLogic self,
                             EPGManSearchAgentMode mode,
                             int category,
                             SvObject view)
{
    SvObject agent = EPGManSearchAgentCreate(self->appGlobals);

    EPGManSearchAgentSetMode(agent, mode);
    EPGManSearchAgentSetCategory(agent, category);
    EPGManSearchAgentSetView(agent, view);

    return agent;
}

void
QBInitLogicSetupMuxes(QBInitLogic self)
{
}
