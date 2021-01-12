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


#ifndef MAIN_H
#define MAIN_H

#include <CUIT/Core/types.h>
#include <QBApplicationController.h>
#include <QBWindowContext.h>
#include <QBNetManager.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <SvEPGDataLayer/Plugins/DVBEPGPlugin.h>
#include <SvEPGDataLayer/Plugins/ToyaEPGPlugin.h>
#include <SvEPGDataLayer/Plugins/GenericEITPlugin.h>
#include <SvEPGDataLayer/Plugins/GenericEITChannelMapper.h>
#include <CutvManager/CutvManager.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBCatManager.h>
#include <QBCASKeyManagerMeta.h>
#include <QBViewRight.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <QBDVBMuxesMap.h>
#include <SvEPG/SvEPGTextRenderer.h>
#include <Logic/AnyPlayerLogic.h>
#include <Logic/ChannelScanningLogic.h>
#include <Logic/OperateScanningLogic.h>
#include <Logic/EPGLogic.h>
#include <Logic/EventsLogic.h>
#include <Logic/VideoOutputLogic.h>
#include <Logic/AudioOutputLogic.h>
#include <Logic/DateTimeLogic.h>
#include <Logic/MainMenuLogic.h>
#include <Logic/GlobalInputLogic.h>
#include <Logic/PVRLogic.h>
#include <Logic/SearchResultsLogic.h>
#include <Logic/SatelliteSetupLogic.h>
#include <Logic/StandbyLogic.h>
#include <Logic/TVLogic.h>
#include <Logic/TunerLogic.h>
#include <Logic/UpgradeLogic.h>
#include <Logic/VerticalEPGLogic.h>
#include <Logic/InitLogic.h>
#include <Logic/WizardLogic.h>
#include <Logic/AccessLogic.h>
#include <Logic/PurchaseLogic.h>
#include <Logic/EventISLogic.h>
#include <Logic/GUILogic.h>
#include <Logic/MStoreLogic.h>
#include <Logic/DiagnosticsLogic.h>
#include <Logic/QBParentalControlLogic.h>
#include <Logic/QBConfigurationMenuLogic.h>
#include <Logic/QBSecureLogLogic.h>
#include <Logic/NetworkLogic.h>
#include <Hints/hints.h>
#include <Services/core/diskSchedulerChanger.h>
#include <Services/core/JSONserializer.h>
#include <Services/core/QBTextRenderer.h>
#include <Services/core/appState.h>
#include <Services/core/aspectRatioAgent.h>
#include <Services/OutputStandardService.h>
#include <Services/core/cableModemMonitor.h>
#include <Services/core/QBChannelStorage.h>
#include <Services/core/cleaner.h>
#include <Services/QBCASPopupVisibilityManager.h>
#include <Services/Conax/QBConaxPopupManager.h>
#include <Services/QBViewRightManagers/QBViewRightPopupManager.h>
#include <Services/QBViewRightManagers/QBViewRightTriggerManager.h>
#include <Services/QBViewRightManagers/QBViewRightWebManager.h>
#include <Services/Conax/QBConaxFingerprint.h>
#include <Services/hddStandbyManager.h>
#include <Services/inputWatcher.h>
#include <Services/langPreferences.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Services/core/playlistManager.h>
#include <Services/core/playlistCursor.h>
#include <Services/core/prerenderer.h>
#include <Services/pvrAgent.h>
#include <Services/reminder.h>
#include <Services/reminderAgent.h>
#include <Services/epgMonitor.h>
#include <QBDVBTableMonitor.h>
#include <Services/sdtMonitor.h>
#include <Services/operateScanningManager.h>
#include <tunerReserver.h>
#include <Services/core/QBSTBModeMonitor.h>
#include <Services/core/videoFormatMonitor.h>
#include <Services/core/volume.h>
#include <Services/QBStandbyAgent.h>
#include <Services/standbyAgent/QBNoInputStandbyPlugin.h>
#include <Services/core/watchdog.h>
#include <Services/schedManTunerReservation.h>
#include <QBSmartCardMonitor.h>
#include <Services/channelMetaStorage.h>
#include <Services/core/hotplugMounts.h>
#include <Services/QBPVRMounter.h>
#include <Services/core/QBDualOutputHandler.h>
#include <Services/QBCustomerInfoStorage.h>
#include <Services/QBProvidersControllerService.h>
#include <Services/QBHTTPEMMService.h>
#include <Services/QBDiskActivator.h>
#include <QBConaxPullEMMService.h>
#include <Services/QBRedirProxyManager.h>
#include <Services/InputTranslator.h>
#include <Services/QBAuthenticationService.h>
#include <Services/QBFileBrowserMounter.h>
#include <Services/QBSubtreeNotifier.h>
#include <Services/QBAdService.h>
#include <Services/core/GlobalPlayer.h>
#include <Services/QBDVBChannelLogos.h>
#include <Services/QBSkinManager.h>
#include <Services/sanityChecker.h>
#include <Services/QBLogWindowManager.h>
#include <Services/QBAppVersion.h>
#include <Services/TraxisWebMonitor.h>
#include <Services/QBHotplugNotifier.h>
#include <Services/core/QBPushReceiver.h>
#include <Services/QBPlaybackStateReporter.h>
#include <Services/QBRemoteControl.h>
#include <Services/QBRemoteMessagesManager.h>
#include <Services/QBReencryptionService.h>
#include <Services/QBRemoteScheduler.h>
#include <Services/QBStandByTimerTimeout.h>
#include <Services/QBPVODStorage.h>
#include <Services/QBCallerIdManager.h>
#include <Services/QBMWConfigMonitor.h>
#include <Services/QBMWConfigHandler.h>
#include <Services/Conax/QBConaxMailManager.h>
#include <Services/npvrAgent.h>
#include <Services/QBCryptoguardVideoRulesManager.h>
#include <Services/QBCryptoguardForceTuneManager.h>
#include <Services/core/QBSecureLogService.h>
#include <Services/QBXHTMLWindowsService.h>
#include <DataModels/QBPVRRecordingsTree.h>
#include <QBDRMManager/QBDRMManager.h>
#include <Services/Conax/QBConaxURIManager.h>
#include <Services/core/QBCEC.h>
#include <Services/core/QBSecretKeyCodeService.h>
#include <Services/cecService.h>
#include <Services/HDMIService.h>
#include <QBAppsManager.h>
#include <QBDIALServerManager.h>
#include <Configurations/QBDiagnosticsWindowConfManager.h>
#include <SvDataBucket2/SvDataBucket.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <TraxisWebClient/TraxisWebPlaybackSessionManager.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <CubiwareMWClient/CubiwareMWClientService.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <CubiwareMWClient/Monitor/QBMWServiceMonitor.h>
#include <Services/core/QBMiddlewareManager.h>
#include <QBPVRProvider.h>
#include <QBDSMCC/QBDSMCCMonitor.h>
#include <QBDSMCC/QBDSMCCDataProvider.h>
#include "init.h"
#include <QBAppExtensions.h>
#include <QBNetworkMonitor.h>
#include <QBNATMonitor/QBNATMonitor.h>
#include <QBWidgets/QBVolumeWidget.h>
#include "main_decl.h"

#include <Services/VOD3FakeCategories.h>
#include <Services/BoldVodApi.h>

struct QBNATMonitor_s;


struct AppGlobals_t {
    QBInit initializer;
    QBApplicationController controller;
    QBLocalWindowManager localManager;
    QBGlobalWindowManager globalManager;
    SvGenericObject switcher;
    QBFocusManager focusManager;

    QBStandbyStateData initialStandbyState;

    QBWindowContext newTV;
    QBWindowContext main;
    QBWindowContext newTVGuide;
    QBWindowContext verticalEPG;
    QBWindowContext screensaver;

    QBGlobalWindow mailIndicator;
    QBGlobalWindow logWindow;

    QBParentalControl pc;

    QBAccessManager accessMgr;

    SvDataBucket dataBucket;

    SvApplication res;

    QBNetManager net_manager;
    QBNetworkMonitor networkMonitor;
    QBNATMonitor natMonitor;
    QBCableModemMonitor cableModem;
    QBVideoFormatMonitor videoFormatMonitor;
    QBOutputStandardService outputStandardService;

    QBPlaylistCursors playlistCursors;

    struct {
        struct {
            SvGenericObject channelList;
            SvGenericObject data;
        } DVB;

        struct {
            SvGenericObject channelList;
            SvGenericObject data;
        } IP;
    } epgPlugin;

    SvGenericObject channelMerger;

    GenericEITChannelMapper mapper;

    CutvManager* cutvManager;
    SvGenericObject nPVRProvider;
    QBPVRProvider pvrProvider;
    bool pvrProviderInitialized;

    SvScheduler scheduler;

    QBTextRenderer textRenderer;

    QBStandbyAgent standbyAgent;
    QBNoInputStandbyPlugin noInputStandbyPlugin;

    QBPVRAgent pvrAgent;
    struct QBNPvrAgent_s* npvrAgent;
    QBPVRRecordingsTree recordingsTree;
    SvGenericObject  conaxMenuHandler;

    QBActiveTree menuTree;

    QBCatManager* catManager;

    QBCASKeyManagerMeta metaKeyManager;

    QBVolume volume;
    QBVolumeWidget volumeWidget;

    QBFingerprint fingerprint;

    QBCASPopupVisibilityManager casPopupVisabilityManager;
    SvGenericObject casPopupManager;

    QBViewRightTriggerManager viewRightTriggerManager;

    QBCallerIdManager callerIdManager;

    QBChannelStorage channelStorage;

    QBTVLogic tvLogic;
    QBEPGLogic epgLogic;
    QBPVRLogic pvrLogic;
    QBVerticalEPGLogic verticalEPGLogic;
    QBSearchResultsLogic searchResultsLogic;
    QBChannelScanningLogic channelScanningLogic;
    QBOperateScanningLogic operateScanningLogic;
    QBTunerLogic tunerLogic;
    QBUpgradeLogic upgradeLogic;
    QBStandbyLogic standbyLogic;
    QBGlobalInputLogic globalInputLogic;
    QBMainMenuLogic mainLogic;
    DateTimeLogic dateTimeLogic;
    QBInitLogic initLogic;
    QBWizardLogic wizardLogic;
    QBAccessLogic accessLogic;
    QBPurchaseLogic purchaseLogic;
    QBEventISLogic eventISLogic;
    QBGUILogic guiLogic;
    QBMStoreLogic mstoreLogic;
    QBSatelliteSetupLogic satelliteSetupLogic;
    QBDiagnosticsLogic diagnosticsLogic;
    QBParentalControlLogic parentalControlLogic;
    QBConfigurationMenuLogic configurationMenuLogic;
    EventsLogic eventsLogic;
    QBSecureLogLogic secureLogLogic;
    QBNetworkLogic networkLogic;

    SvGenericObject upgradeWatcher;

    QBReminder reminder;
    QBReminderAgent reminderAgent;

    QBSTBModeMonitor stbModeMonitor;

    QBInputWatcher inputWatcher;

    SvAutoreleasePool appPool;

    QBCleaner cleaner;

    QBHintManager hintManager;

    QBLangPreferences langPreferences;

    QBWatchdog watchdog;

    QBTextPreRenderer textPrerenderer;

    QBAppStateService appState;

    QBHddStandbyManager hddStandbyManager;

    QBDiagnosticsManager diagnosticsManager;

    QBSDTMonitor      sdtMonitor;

    QBOperateScanningManager operateScanningManager;

    QBEPGMonitor* epgMonitor;

    QBSchedManTunerReservation *schedManTunerReservation;

    QBHTTPEMMService httpEmm;
    QBConaxPullEMMService conaxHttpEmm;

    QBSmartCardMonitor smartCardMonitor;

    QBViewRightWebService viewRightWeb;

    QBMWProductUsageController qbMWProductUsageController;

    QBChannelMetaStorage* channelMetaStorage;

    QBHotplugMountAgent hotplugMountAgent;

    QBCustomerInfoStorage customerInfoStorage;

    CubiwareMWCustomerInfo customerInfoMonitor;

    QBPVRMounter PVRMounter;

    QBHotplugNotifier hotplugNotifier;

    QBFileBrowserMounter fileBrowserMounter;

    QBProvidersControllerService providersController;
    QBMWServiceMonitor serviceMonitor;

    QBMiddlewareManager middlewareManager;
    QBRedirProxyManager redirProxyManager;

    CubiwareMWClientService middlewareClientService;

    QBKeyInputTranslator inputTranslator;
    QBSanityChecker sanityChecker;

    QBAuthenticationService authenticationService;

    QBAdService adService;

    QBSkinManager skinManager;
    QBSecretKeyCodeService secretKeyCodeService;
    QBAppVersion appVersion;

    QBPVODStorageService pvodStorage;

    QBDVBChannelLogosService channelLogosSetter;

    SvSSLParams sslParams;

    bool lstvDVBTimeDateInitialized;
    bool appLogsInitialized;

    QBSubtreeNotifier subtreeNotifier;
    QBGlobalPlayerController playerController;

    TraxisWebSessionManager traxisWebSessionManager;
    TraxisWebMonitor traxisWebMonitor;
    TraxisWebPlaybackSessionManager traxisWebPlaybackSessionManager;

    QBPushReceiver pushReceiver;
    QBDVBMuxesMap dvbMuxesMap;
    QBStandByTimerTimeout standByTimerTimeout;

    QBReencryptionService reencryptionService;

    QBRemoteControl remoteControl;
    QBPlaybackStateReporter playbackStateReporter;
    QBRemoteMessagesManager remoteMessagesManager;
    QBBookmarkManager bookmarkManager;

    QBRemoteScheduler remoteScheduler;

    QBDiskSchedulerChanger diskSchedulerChanger;
    QBDiskActivator diskActivator;

    QBCEC cec;
    QBCecService cecService;

    QBMWConfigMonitor qbMWConfigMonitor;
    QBMWConfigHandler qbMWConfigHandler;
    QBDRMManager DRMManager;
    QBConaxURIManager conaxURIManager;
    SvGenericObject casPVRManager;
    QBCryptoguardVideoRulesManager cryptoguardVideoRulesManager;
    QBCryptoguardForceTuneManager cryptoguardForceTuneManager;
    QBSecureLogService secureLogService;
    QBConaxMailManager conaxMailManager;

    QBAppExtensions extensions; ///< custom extensions per app

    QBDSMCCMonitor dsmccMonitor;
    QBDSMCCDataProvider adsDSMCCDataProvider;
    QBXHTMLWindowsService xhtmlWindowsService;
    QBAppsManager appsManager;
    QBDIALServerManager dialServerManager;
    
    VOD3FakeCategories vod3FakeCategories;
    BoldVodApi    boldVodApi;
    bool vodGridBrowserInitAsSearch;
};

void QBSetupGlobalInputEventHandler(AppGlobals appGlobals);

#endif
