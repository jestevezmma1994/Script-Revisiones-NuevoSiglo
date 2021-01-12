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

#include <signal.h>
#include <ucontext.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <syslog.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <CAGE/Core/CAGEProperties.h>
#include <CAGE/Text/SvBasicTextParagraph.h>
#include <CAGE/HarfBuzz/HarfBuzzTextParagraph.h>
#include <CUIT/Core/event.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvGfxHAL/SvGfxEngine.h>
#include <QBInput/QBInputCodes.h>
#include <QBInput/QBInputService.h>
#include <QBInput/Filters/QBRecordFilter.h>
#include <QBInput/Filters/QBLongKeyPressFilter.h>
#include <QBInput/Drivers/QBEventPlayerDevice.h>
#include <QBResourceManager/QBResourceManager.h>
#include <settings.h>
#include <SvProfile.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <sv_tuner.h>
#include <QBConf.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformInit.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformRFOutput.h>
#include <QBPlatformHAL/QBPlatformUtil.h>
#include <QBPlatformHAL/QBPlatformFan.h>
#include <QBPlatformHAL/QBPlatformStandby.h>
#include <QBPlatformHAL/QBPlatformWatchdog.h>
#include <QBPlatformHAL/QBPlatformLED.h>
#include <QBContextSwitcherStandard.h>
#include <Logs/logs.h>
#include <Services/QBAccessController/ToyaAccessPlugin.h>
#include <Logic/VideoOutputLogic.h>
#include <Logic/QBPlatformLogic.h>
#include <Services/Conax/QBConaxMailIndicator.h>
#include <Services/drm.h>
#include <dvbTimeDate.h>
#include <Services/core/playlistManager.h>
#include <Services/core/QBDebugUtils.h>
#include <Services/smartcard.h>
#include <Services/upgradeWatcher.h>
#include <Configurations/QBDiskPvrStorageMode.h>
#include <Utils/capabilities.h>
#include <Utils/viewport.h>
#include <Windows/pvrplayer.h>
#include <Windows/mainmenu.h>
#include <Windows/newtv.h>
#include <Windows/newtvguide.h>
#include <Windows/standby.h>
#include <Services/sanityChecker.h>
#include <QBShellCmds.h>
#include <init.h>
#include <main.h>
#include <QBAppExtensions.h>
#include <QBShellCmds.h>
#include <QBTuner.h>
#include <Configurations/overscanMargins.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <QBMenu/QBMainMenuInterface.h>

#include <Logic/GlobalInputLogic.h>

#define HOSTMACHINE ( defined __i386__ || defined __x86_64__ )

// Use this custom log carefully! Until we can't emulate full STB environment on host machine
// (like access to /dev/, /etc/ and other files) we will be 'hide' this problems on host machines.
#if HOSTMACHINE
#define SvCustomLog_WarningOnHostErrorOnOther(format, ...) SvLogWarning(format, ## __VA_ARGS__)
#else
#define SvCustomLog_WarningOnHostErrorOnOther(format, ...) SvLogError(format, ## __VA_ARGS__)
#endif

// allow collect time statiscs only on debug version of software - this define
// have to be set before QBTimeLimit include if we interested time statistics.
#define QBTimeLimitEnable
#define QBTimeThreshold (50*1000)
#include <QBTimeLimit.h>

#define POWER_BUTTON_LOCK "/var/lock/lstvpowerbutton"
#define WATCHDOG_TIMEOUT_BEGIN 120

SV_DECL_INT_ENV_FUN(disable_watchdog, "QBDisableWatchdog", "Disables watchdog.");

SvLocal SvString findNodeWithShortcut(QBActiveTree tree, SvString shortcutName, SvString* const nodeIdTab, int cnt)
{
    int firstFound = -1;
    int i;
    for (i = 0; i < cnt; ++i) {
        QBActiveTreeNode node = QBActiveTreeFindNode(tree, nodeIdTab[i]);
        if (!node)
            continue;
        if (firstFound < 0)
            firstFound = i;

        SvObject obj = QBActiveTreeNodeGetAttribute(node, SVSTRING("shortcut"));
        if (!obj || !SvObjectIsInstanceOf(obj, SvString_getType()) || !SvObjectEquals(obj, (SvObject) shortcutName))
            continue;

        return nodeIdTab[i];
    }

    if (firstFound >= 0)
        return nodeIdTab[firstFound];
    return NULL;
}

SvLocal bool svLstvGlobalInputEventHandler(void* param, const QBInputEvent* e)
{
    // Beware that this handler might get called before QBInitCreateEverything
    AppGlobals appGlobals = param;
    if (appGlobals->globalInputLogic && QBGlobalInputLogicHandleInputEvent(appGlobals->globalInputLogic, e)) {
        return true;
    }

    if (appGlobals->cecService && QBCecServiceInputEventHandler(appGlobals->cecService, e)) {
        return true;
    }

    if (appGlobals->volumeWidget) {
        if (QBVolumeWidgetInputEventHandler(appGlobals->volumeWidget, e)) {
            unsigned keyCode = e->u.key.code;
            QBVolume volume = (QBVolume) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBVolume"));
            switch(keyCode)  {
                case QBKEY_VOLDN:
                    QBVolumeUp(volume);
                    QBVolumeDown(volume);
                    break;
                case QBKEY_VOLUP:
                    QBVolumeDown(volume);
                    QBVolumeUp(volume);
                    break;
                default:
                    break;
            }
            return true;
        }
    }

    QBDualOutputHandler dualOutputHandler =
        (QBDualOutputHandler) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                          SVSTRING("QBDualOutputHandler"));
    if (dualOutputHandler && QBDualOutputHanderInputEventHandler(dualOutputHandler, e)) {
        return true;
    }

    SvString nodeID = NULL;

    if (e->type != QBInputEventType_keyTyped) {
        return false;
    }

    QBTimeLimitBegin(time_event_handler, QBTimeThreshold);

    switch (e->u.key.code) {
        case QBKEY_MENU:
            if (appGlobals->main) {
                QBGlobalInputLogicUpdateMainMenuPosition(appGlobals->globalInputLogic);
                QBApplicationControllerSwitchToRoot(appGlobals->controller);
                QBApplicationControllerPushContext(appGlobals->controller, appGlobals->main);
                QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.MENU");
            }
            break;
        case QBKEY_BACK:
            QBApplicationControllerPopContext(appGlobals->controller);
            QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.BACK");
            break;
        case QBKEY_VOD:
            if (appGlobals->providersController) {
                SvArray services = NULL;
                for (int VoDServiceType = 0; VoDServiceType < QBProvidersControllerService_cnt && !services; ++VoDServiceType) {
                    services = QBProvidersControllerServiceGetServices(appGlobals->providersController,
                                                                       SVSTRING("VOD"), VoDServiceType);
                }
                if (services) {
                    if (SvArrayCount(services) > 1) {
                        nodeID = SVSTRING("VOD");
                    } else {
                        QBCarouselMenuItemService menu = (QBCarouselMenuItemService) SvArrayAt(services, 0);
                        if (!menu || SvStringEqualToCString(QBCarouselMenuGetServiceLayout(menu), "vod2") ||
                            (SvStringEqualToCString(QBCarouselMenuGetServiceLayout(menu), "default") && !strcmp(QBConfigGet("VODVIEW"), "vod2")) ||
                            !QBCarouselMenuPushContext(menu, QBCarouselMenuGetOwnerTree(menu)))
                           nodeID = SVSTRING("VOD");
                    }
                }
            }
            QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.VOD");
            break;
        case QBKEY_USB:
            nodeID = SVSTRING("HotplugMounts");
            break;
        case QBKEY_SETTINGS:
            nodeID = SVSTRING("STGS");
            break;
        case QBKEY_PVR:
            if (appGlobals->menuTree) {
                // shortcuts for subtrees aren't implemented yet, so 'jump to completed' is inactive
                SvString candidates[4] = { SVSTRING("PVR"), SVSTRING("NPvr"), SVSTRING("completed"), SVSTRING("npvr-completed"), };
                nodeID = findNodeWithShortcut(appGlobals->menuTree, SVSTRING("PVR"), candidates, 4);
            }
            QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.PVR");
            break;
        case QBKEY_RED:
            nodeID = SVSTRING("XTRS");
            break;
        case QBKEY_WWW:
            nodeID = SVSTRING("WEB");
            break;
        case QBKEY_FAVORITES:
            nodeID = SVSTRING("FAVS");
            break;
        default:
            break;
    }

    if (appGlobals->main && nodeID) {
        if (SvInvokeInterface(QBMainMenu, appGlobals->main, switchToNode, nodeID)) {
            QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.switchToNode");
            return true;
        }
        QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.switchToNode");
        return 0;
    }

    if (e->u.key.code == QBKEY_ABORT) {
        // on the STB you should never be able call key QBKEY_ABORT from the remote control so it's error
        SvCustomLog_WarningOnHostErrorOnOther("%s(): QBKEY_ABORT : call QBInitStopApplication", __func__);
        QBInitStopApplication(appGlobals, true, "abort key pressed");
        return true;
    }

    QBTimeLimitEnd_(time_event_handler, "GlobalInputEventHandler.onTheEnd");

    return false;
}

void QBSetupGlobalInputEventHandler(AppGlobals appGlobals)
{
    QBApplicationControllerRegisterInputHandler(appGlobals->controller, svLstvGlobalInputEventHandler, appGlobals);
}

SvLocal void setSignalHandler(bool reset, void (*handler)(int, siginfo_t *, void *))
{
   struct sigaction action = {
      .sa_sigaction = handler
   };

   if (reset) {
      action.sa_handler = SIG_DFL;
   } else {
      action.sa_flags = SA_SIGINFO;
   }

   sigaction(SIGILL, &action, NULL);
   sigaction(SIGSEGV, &action, NULL);
   sigaction(SIGABRT, &action, NULL);
   sigaction(SIGBUS, &action, NULL);
   sigaction(SIGFPE, &action, NULL);
   sigaction(SIGTERM, &action, NULL);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGSYS, &action, NULL);
   sigaction(SIGINT, &action, NULL);
   sigaction(SIGUSR1, &action, NULL);
   sigaction(SIGUSR2, &action, NULL);
}

SvLocal void signalHandler(int sig, siginfo_t *info, void *ucontext_)
{
    SvLogEmergency("!!! %s(): signal = %d, code = %d !!!", __func__, sig, info->si_code);
    if (sig == SIGILL || sig == SIGFPE || sig == SIGSEGV || sig == SIGBUS || sig == SIGTRAP) {
        SvLogEmergency("!!! %s(): fault address = %p !!!", __func__, info->si_addr);
    } else if (sig == SIGUSR1 || sig == SIGUSR2) {
        char procFile[32];
        snprintf(procFile, sizeof(procFile), "/proc/%d/comm", info->si_pid);
        char procName[32];
        procName[0] = '\0';
        FILE *fp = fopen(procFile, "r");
        if (fp) {
            fgets(procName, sizeof(procName), fp);
            char *endLine = strchr(procName, '\n');
            if (endLine)
                *endLine = '\0';
            fclose(fp);
        }
        SvLogEmergency("!!! %s(): signal from pid: %d (%s), user ID: %d !!!", __func__, info->si_pid, procName, info->si_uid);
    }

    QBSanityCheckerOnCrash();
    /* CubiTV set watchdog timeout to 30 seconds, reset watchdog
     * counter and close /dev/watchdog device without disabling
     * watchdog. If core-uploader is started by kernel, it should
     * disable watchdog until it finished uploading core file.
     */
    QBPlatformWatchdogSetTimeout(30);
    QBPlatformWatchdogContinue();
    QBPlatformWatchdogCloseWithoutDeactivation();
    setSignalHandler(true, 0);
    raise(sig);
}

SvLocal int setupTextRendering(void)
{
    SvType defaultTextParagraphType = NULL;

    const char *renderer = QBConfigGet("UI.TEXT_RENDERING.RENDERER");
    if (renderer && strcasecmp(renderer, "HarfBuzz") == 0) {
        defaultTextParagraphType = HarfBuzzTextParagraph_getType();
        SvLogNotice("CubiTV: using HarfBuzzTextParagraph as default");
    } else if (!renderer || strcasecmp(renderer, "default") == 0) {
        defaultTextParagraphType = SvBasicTextParagraph_getType();
    } else {
        SvLogError("CubiTV: invalid text renderer type '%s'!", renderer);
        return -1;
    }

    CAGEPropertiesSetDefaultTextParagraphImplementation(defaultTextParagraphType, NULL);
    return 0;
}

SvLocal void setDefaultVideoOutputsConfig(QBVideoOutputConfig *outputs,
                                          int count,
                                          QBTVSystem systemType)
{
    if (QBPlatformGetDefaultVideoOutputsConfiguration(outputs, count, systemType) == count) {
        SvLogWarning("CubiTV: using default configuration");
        QBConfigSet("HDOUTAUTOMATICMODE", "enabled");
        QBConfigSet("HDOUTORIGINALMODE", "disable");
        QBConfigSet("VIDEOOUTHD", NULL);
    } else {
        SvLogEmergency("CubiTV: Cannot get default video output configuration! Exitting...");
        exit(1);
    }
}

SvLocal void readVideoOutputsConfig(QBVideoOutputConfig *outputs,
                                    int count,
                                    QBTVSystem systemType,
                                    int *argc, char ***argv)
{
    if (QBPlatformParseVideoOutputsConfiguration(outputs, count, systemType, argc, argv) < 0) {
        SvLogError("CubiTV: invalid video outputs configuration");
        setDefaultVideoOutputsConfig(outputs, count, systemType);
        return;
    }

    for (int i = 0; i < count; i++) {
        if (!outputs[i].enabled || i != (int) outputs[i].masterID) {
            continue;
        }

        bool isSupported = false;
        if (QBPlatformOutputIsModeSupported(i, outputs[i].mode, &isSupported) < 0) {
            continue;
        }

        if (!isSupported) {
            SvLogError("CubiTV: In read video configuration, enabled master video output %s mode %s is unsupported.",
                       outputs[i].name,
                       QBPlatformGetModeName(outputs[i].mode));
            setDefaultVideoOutputsConfig(outputs, count, systemType);
            return;
        }
    }
}

SvLocal void setupVideoOutputs(AppGlobals appGlobals, int *argc, char ***argv, bool powerOff)
{
   int cnt = QBPlatformGetOutputsCount();
   QBVideoOutputConfig outputs[cnt];
   int RFChannel = -1;

   QBTVSystem systemType = QBPlatformGetTVSystemByName(QBConfigGet("TVSYSTEM"));

   if (systemType == QBTVSystem_unknown) {
       SvLogWarning("CubiTV: invalid TVSYSTEM configuration value. Using PAL.");
       systemType = QBTVSystem_PAL;
   }

   readVideoOutputsConfig(outputs, cnt, systemType, argc, argv);

   QBServiceRegistry registry = QBServiceRegistryGetInstance();
   VideoOutputLogic videoOutputLogic = VideoOutputLogicCreate(appGlobals, NULL);
   QBServiceRegistryRegisterService(registry, (SvObject) videoOutputLogic, NULL);
   VideoOutputLogicVerifyConfig((VideoOutputLogic) videoOutputLogic, outputs, cnt, &RFChannel);
   SVRELEASE(videoOutputLogic);

   QBPALSystem PALSystem = QBPALSystem_default;

   const char *PALSystemConfig = QBConfigGet("PALSYSTEM");
   if (PALSystemConfig) {
      if (strcasecmp(PALSystemConfig, "N") == 0) {
         PALSystem = QBPALSystem_N;
      } else if (strcasecmp(PALSystemConfig, "CN") == 0) {
         PALSystem = QBPALSystem_CN;
      } else if (strcasecmp(PALSystemConfig, "I") == 0) {
         PALSystem = QBPALSystem_I;
      }
   }

   // perform actual setup
   int activeCnt = 0;
   for (int i = 0; i < cnt; i++) {

      if (!outputs[i].enabled) {
         continue;
      }

      if ((outputs[i].type == QBOutputType_RF) && (RFChannel > 0)) {
          QBPlatformRFOutputSetChannelNumber(i, RFChannel);
      }

      outputs[i].PALSystem = PALSystem;

      if (QBPlatformOutputEnable(i, outputs[i].mode, outputs[i].aspectRatio) < 0) {
         if (i == (int) outputs[i].masterID) {
             SvLogError("CubiTV: can't enable %s master video output! Exitting...", outputs[i].name);
             exit(1);
         } else {
             SvLogError("CubiTV: can't enable %s  video output! Ignoring...", outputs[i].name);
         }
      } else {

          if ((outputs[i].capabilities & QBOutputCapability_HDCP) != 0) {
             QBPlatformSetHDCP(i, QBContentProtection_required, QBPlatformOutputPriority_application);
          }

          if (QBPlatformOutputSetPowerOff(i, powerOff, QBPlatformOutputPriority_application) < 0) {
              if (i == (int) outputs[i].masterID) {
                  SvLogError("CubiTV: can't power %s %s master video output! Exitting...", (powerOff) ? "off" : "on", outputs[i].name);
                  exit(1);
              } else {
                  SvLogError("CubiTV: can't power %s %s video output! Ignoring...", (powerOff) ? "off" : "on", outputs[i].name);
              }
          } else {
              if (QBPlatformSetPALSystem(i, outputs[i].PALSystem) < 0) {
                  SvLogError("CubiTV: can't setup PAL mode %s video output! Ignoring...", outputs[i].name);
              }
              activeCnt += 1;
          }
      }
   }

   if (activeCnt == 0) {
      SvLogWarning("CubiTV: no video outputs configured!");
      int HDOutputID = QBPlatformFindOutput(QBOutputType_unknown, NULL, QBOutputCapability_HD, true, false);
      if (HDOutputID >= 0) {
         // as the last resort enable at least master HD output in 720p50 mode
         outputs[HDOutputID].mode = QBOutputStandard_720p50;
         outputs[HDOutputID].aspectRatio = QBAspectRatio_16x9;
         outputs[HDOutputID].enabled = true;

         QBPlatformOutputEnable(HDOutputID, outputs[HDOutputID].mode, outputs[HDOutputID].aspectRatio);
         QBPlatformOutputSetPowerOff(HDOutputID, powerOff, QBPlatformOutputPriority_application);
      }
   }
}

SvLocal int getVerticalResolution(void)
{
   const int defaultResolution = 720;
   static const int validValues[] = { 480, 576, 720, 1080, 2160 };

   const char *confFBLines = QBConfigGet("FBLINES");
   if (confFBLines) {
      errno = 0;
      char *endptr = NULL;
      const int fblines = (int) strtol(confFBLines, &endptr, 0);
      if (errno == 0 && *endptr == '\0') {
         for (unsigned int i = 0; i < (sizeof(validValues) / sizeof(validValues[0])); ++i) {
            if (fblines == validValues[i])
               return fblines;
         }
      }

      SvLogError("CubiTV: unrecognized value for \"FBLINES\": %s. Use default value: %d", confFBLines, defaultResolution);
   }
   return defaultResolution;
}

int main(int argc, char **argv)
{
   SvApplication app;

   QBDebugUtilsRegisterHooks();

   QBShellCmdsInit("/tmp/shell_cmds", "cubitv");

   QBDropRootPrivileges(1000, 1000);

   SvProfileInit();
   SvSchedulerCreateMain();

   AppGlobals appGlobals = calloc(1, sizeof(struct AppGlobals_t));
   appGlobals->extensions = QBAppExtensionsCreate(NULL);

   //start the sanity checker before we touch any files from the box
   QBSanityChecker sanityChecker = QBSanityCheckerCreate(appGlobals);

   setSignalHandler(false, signalHandler);

   QBConfigInit();

   QBWatchdog watchdog = NULL;

   if (!disable_watchdog()) {
       watchdog = QBWatchdogCreate(NULL);
       QBWatchdogSetTimeout(watchdog, WATCHDOG_TIMEOUT_BEGIN, NULL);
   } else {
       SvLogWarning("Software watchdog is disabled.");
   }

   signal(SIGPIPE, SIG_IGN);

   appGlobals->watchdog = watchdog;
   appGlobals->sanityChecker = sanityChecker;

   appGlobals->initialStandbyState = QBStandbyStateDataCreate(QBStandbyState_unknown, NULL, false, SVSTRING("not set yet"));
   QBStandbyAgentGetInitialState(appGlobals->initialStandbyState, NULL);

   assert(QBStandbyStateDataGetState(appGlobals->initialStandbyState) != QBStandbyState_unknown);

   if (getenv("LSTV_FAKE_DATE")) {
       struct tm past = { .tm_sec = 0 };
       if (strptime(getenv("LSTV_FAKE_DATE"), "%d%m%y", &past)) {
           SvTime now = SvTimeGetCurrentTime();
           SvTime newNow = SvTimeFromBrokenDownTime(&past, 0, true);
           SvTimeSetCurrentTimeOffset(SvTimeGetSeconds(SvTimeSub(newNow, now)));
       }
   }

   remove(POWER_BUTTON_LOCK);

   QBInputServiceInitialize(NULL);

   const char* recordFile = getenv("INPUT_RECORD_FILE");
   if(recordFile && recordFile[0]){
       QBRecordFilter rf = (QBRecordFilter) SvTypeAllocateInstance(QBRecordFilter_getType(), NULL);
       QBRecordFilterInit(rf, recordFile, 0xffff, NULL);
       QBInputServiceAddGlobalFilter((SvGenericObject) rf, NULL, NULL);
       SVRELEASE(rf);
   }
   const char* playFile = getenv("INPUT_PLAY_FILE");
   if(playFile && playFile[0]){
       SvLogNotice("play file = %s", playFile);
       QBEventPlayerDevice dev;
       dev = (QBEventPlayerDevice) SvTypeAllocateInstance(QBEventPlayerDevice_getType(),NULL);
       QBEventPlayerDeviceInitWithFile(dev, playFile, 0xffff, 1, NULL);
       QBInputServiceAddDevice((SvGenericObject) dev, NULL);
   }

   QBLongKeyPressFilter filter;
   filter = (QBLongKeyPressFilter) SvTypeAllocateInstance(QBLongKeyPressFilter_getType(), NULL);
   QBLongKeyPressFilterInitExtended(filter, QBInitLogicGetKeyExtendedMap(appGlobals->initLogic), NULL);
   QBInputServiceAddGlobalFilter((SvGenericObject) filter, NULL, NULL);
   SVRELEASE(filter);

   if (setupTextRendering() < 0)
       return 1;

   QBPlatformInitConfig platformConfig;
   QBPlatformInitGetDefaultConfig(&platformConfig);

   QBDiskPvrStorageMode pvrStorageMode = QBDiskPvrStorageModeGet();
   platformConfig.features.ipTVReencryption.enabled = pvrStorageMode == QBDiskPvrStorageMode_reencryption;

   if (QBPlatformInitWithConfig(&platformConfig) < 0) {
       SvLogError("CubiTV: QBPlatformInit() failed!");
       return 1;
   }
   QBTunerPrintConfig("native");

   QBDropMkNodPrivileges();

   if (QBStandbyStateDataGetState(appGlobals->initialStandbyState) == QBStandbyState_on && QBPlatformGetLEDsCount() > 1) {
       QBPlatformSetLEDBrightness(0, QBPlatformLEDBrightness_off);
       QBPlatformSetLEDBrightness(1, QBPlatformLEDBrightness_max);
   }

   bool outputsPoweredOff = (QBStandbyStateDataGetState(appGlobals->initialStandbyState) != QBStandbyState_on);
   /* TEMP HACK - import VIDEOOUTHD, VIDEOOUTSD and ASPECTRATIOSD variables
      Seems they're used in QBPlatformHAL
   */
   const char *param = QBConfigGet("VIDEOOUTHD");
   if (param)
       setenv("VIDEOOUTHD", param, 1);
   param = QBConfigGet("VIDEOOUTSD");
   if (param)
       setenv("VIDEOOUTSD", param, 1);
   param = QBConfigGet("ASPECTRATIOSD");
   if (param)
       setenv("ASPECTRATIOSD", param, 1);

   setupVideoOutputs(appGlobals, &argc, &argv, outputsPoweredOff);
   VideoOutputLogic videoOutputLogic = (VideoOutputLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("VideoOutputLogic"));
   VideoOutputLogicAfterOutputsSetup(videoOutputLogic);

   int gfx_width = -1, gfx_height = -1;
   const char *gfx_mode = NULL;
   const int verticalResolution = getVerticalResolution();
   switch (verticalResolution) {
     case 720:
       gfx_mode     = "720p";
       gfx_width    = 1280;
       gfx_height   = 720;
       break;
     case 1080:
       gfx_mode     = "1080";
       gfx_width    = 1920;
       gfx_height   = 1080;
       break;
     case 2160:
       gfx_mode     = "2160";
       gfx_width    = 3840;
       gfx_height   = 2160;
       break;
     case 480:
       gfx_mode     = "ntsc";
       gfx_width    = 720;
       gfx_height   = 480;
       break;
     case 576:
       gfx_mode     = "pal";
       gfx_width    = 720;
       gfx_height   = 576;
       break;
     default:
       gfx_mode     = "720p";
       gfx_width    = 1280;
       gfx_height   = 720;
       break;
   }
   SvLogNotice("CubiTV: GFX mode = %s (%dx%d)", gfx_mode, gfx_width, gfx_height);

   CAGEPropertiesSetBitmapSizeLimit(gfx_width, gfx_height);

   unsigned int frameRate = 25;
   const char *val = QBConfigGet("FRAMERATE");
   if (val) {
       int tempVal = atoi(val);
       if (tempVal > 0)
           frameRate = (unsigned int) tempVal;
   }

   if (!(app = svAppCreate(gfx_width, gfx_height, frameRate, NULL))) {
      SvLogError("CubiTV: svAppCreate(%d, %d) with frame rate %d failed!", gfx_width, gfx_height, frameRate);
      return 1;
   }

   svAppEnablePartialUpdates(app, true);

   // overscan compensation: reduce GFX viewport on SD output
   VideoOutputLogicSetupOverscanCompensation(videoOutputLogic);

   appGlobals->res = app;

   QBPlatformLogicSetupInputAutoRepeat();

   QBPlatformSetFanSpeed(QBPlatformFanSpeed_max);
   QBInitStartApplication(appGlobals, argv[0], gfx_mode);

   int wakeupDelay = QBStandbyAgentGetWakeupDelay(appGlobals->standbyAgent);

   QBInitFinishApplication(appGlobals, true);
   QBPlatformSetFanSpeed(QBPlatformFanSpeed_none);

   svAppDestroy(app);

   SvAutoreleasePool pool = appGlobals->appPool;

   svSettingsDeinit();

   QBServiceRegistry registry = QBServiceRegistryGetInstance();
   QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));

   // many of services use bitmaps so we need to release resourceManager as the last service
   SVRETAIN(resourceManager);
   QBServiceRegistryRemoveAllServices(registry, NULL);
   QBResourceManagerRemoveAllResources(resourceManager, NULL);
   SVRELEASE(resourceManager);

   if (wakeupDelay > 0) {
      /// shutdown the platform, with given "wakeupDelay"
      SvLogNotice("----- APP : wakeupDelay is %d seconds", wakeupDelay);
      QBPlatformSwitchToStandby(wakeupDelay);
   }

#if SV_LOG_LEVEL == 0
    const char* gentle = getenv("CubiTVLeaveGently");
    if (!gentle || strcmp(gentle, "1"))
        raise(SIGKILL);
#endif

   QBPlatformDeinit();
   // QBInputServiceCleanup() should be after QBPlatformDeinit() because platfom call QBInputServiceRemoveDevice()
   QBInputServiceCleanup();

   SvSchedulerPrint(SvSchedulerGet());
   SvSchedulerCleanup();

   SVRELEASE(pool);

   SvProfileCleanup();

   SVRELEASE(appGlobals->sanityChecker);

   if (appGlobals->watchdog) {
       SVRELEASE(appGlobals->watchdog);
       appGlobals->watchdog = NULL;
   }
   QBInitDestroy(appGlobals->initializer);
   SVRELEASE(appGlobals->extensions);
   free(appGlobals);
   QBShellCmdsDeinit();
   closelog();
#if (!(defined __linux__ && defined __GLIBC__ && defined __UCLIBC__) && !defined DISABLE_PTHREAD_EXIT)
   pthread_exit(0);
#endif
   return 0;
}
