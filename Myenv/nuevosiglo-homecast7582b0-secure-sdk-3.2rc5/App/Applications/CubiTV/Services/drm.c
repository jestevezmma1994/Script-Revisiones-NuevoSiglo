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

#include "drm.h"
#include <main.h>
#include <sys/stat.h>
#include <errno.h>

#include <Configurations/QBDiskPvrStorageMode.h>
#include <SvDRM/SvDRMManager.h>
#include <QBTuner.h>
#include <QBCatManager.h>
#include <QBConf.h>
#include <QBCAS.h>
#include <QBCASKeyManagerMeta.h>
#include <QBPlatformHAL/QBPlatformWatchdog.h>
#include <QBShellCmds.h>
#include <QBCryptoEngine.h>
#include <QBSmartcard2Interface.h>
#include <SvFoundation/SvObject.h>

SvLocal void QBDRMIndividualize(void)
{
  // check if an individualization was already done
  QBCASIndividualizationState ind_state = SvInvokeInterface(QBCAS, QBCASGetInstance(), getIndividualizationState);
  switch (ind_state) {
    case QBCASIndividualizationState_individualizationNeeded:
    {
      // disable watchdog
      QBPlatformWatchdogStop();
      // do an individualization
      QBCASIndividualizationInfo ind_info = SvInvokeInterface(QBCAS, QBCASGetInstance(), performIndividualization);
      switch (ind_info) {
        case QBCASIndividualizationInfo_success:
          // continue application booting, application can be used without reboot
          break;
        case QBCASIndividualizationInfo_successRebootNeeded:
          SvLogNotice("Individualization succeeded, reboot required");
          QBShellExec("reboot"); // Nothing to do more...
          break;
        case QBCASIndividualizationInfo_error:
          SvLogError("Individualization failed box will be rebooted");
          QBShellExec("reboot"); // Nothing to do more...
          break;
        default:
          SvLogError("Unsupported individualization return code (%d)", (int)ind_info);
          break;
      }
      QBPlatformWatchdogStart();
      break;
    }
    case QBCASIndividualizationState_notNeeded:
    {
      // individualization not needed, continue application booting
      break;
    }
    default:
      SvLogError("Unsupported individualization state (%d)", (int)ind_state);
      break;
  }
}

SvLocal void QBDRM_want_to_reboot(void *self_)
{
  SvLogNotice("DRM wants to reboot, rebooting...");
  AppGlobals appGlobals = (AppGlobals) self_;

  QBWatchdogReboot(appGlobals->watchdog, NULL);
}

SvLocal void QBDRM_want_factory_reset(void *self_)
{
  SvLogNotice("DRM wants to reset factory defaults, restoring to factory defaults...");
  AppGlobals appGlobals = (AppGlobals) self_;
  QBInitFactoryResetAndStopApplication(appGlobals);
}

static const struct QBCASCallbacks_s s_cas_callbacks = {
  .want_to_reboot     = &QBDRM_want_to_reboot,
  .want_factory_reset = &QBDRM_want_factory_reset,
};

SvLocal void QBDRMAddCallbacks(AppGlobals appGlobals)
{
  SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks) & s_cas_callbacks, appGlobals, "QBDRM");
}

SvLocal int QBDRMInitCryptoEngine(void)
{
  QBCryptoCASystemType cryptoCaSystemType = QBCryptoCASystemType_noCA;

  // If CA system is conax enable conax's cryptographic specific features
  if (SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
    cryptoCaSystemType = QBCryptoCASystemType_conax;
  }

  int res = QBCryptoEngineInit(QBCryptoEngineGetInstance(), SvSchedulerGet(), cryptoCaSystemType);
  if (res < 0) {
    SvLogError("Couldn't initialize crypto engine, res = %d ", res);
    return res;
  }
  return 0;
}

int QBDRMInit(AppGlobals appGlobals)
{
  SvScheduler sched = SvSchedulerGet();

  int res = QBCASInit();
  if (res < 0)
    return res;

  // Initialize crypto engine before DRM engines.
  res = QBDRMInitCryptoEngine();
  if (res < 0) {
    SvLogError("Couldn't initialize crypto engine, res = %d ", res);
    return res;
  }

  QBDiskPvrStorageMode pvrStorageMode = QBDiskPvrStorageModeGet();
  bool reencryptionEnabled = pvrStorageMode == QBDiskPvrStorageMode_reencryption || pvrStorageMode == QBDiskPvrStorageMode_dvbOnlyReencryption;

  // Creating meta key manager instance
  appGlobals->metaKeyManager = QBCASKeyManagerMetaCreate();

  QBDRMIndividualize();

  struct QBCASAdditionalInitParams_s additionalParams;
  res = svAppGetDimensions(appGlobals->res, &additionalParams.width, &additionalParams.height);
  additionalParams.tuners_cnt = QBTunerLogicGetTunerCount(appGlobals->tunerLogic);
  additionalParams.metaKeyManager = appGlobals->metaKeyManager;
  additionalParams.reencryptionEnabled = reencryptionEnabled;
  if (res < 0)
    return res;

  additionalParams.casServer.address = QBConfigGet("CASSERVER.ADDRESS");
  QBConfigGetInteger("CASSERVER.PORT", &additionalParams.casServer.port);

  additionalParams.casServer.address = QBConfigGet("CASSERVER.ADDRESS");
  QBConfigGetInteger("CASSERVER.PORT" , &additionalParams.casServer.port);

  res = mkdir("/etc/vod/casData/", 0755);
  if (res < 0) {
    if (errno != EEXIST) {
        SvLogError("Couldn't create casData directory, errno = %d", errno);
        return res;
    }
  }

  additionalParams.casDataDirectory = "/etc/vod/casData/";

  res = SvInvokeInterface(QBCAS, QBCASGetInstance(), init, sched, &additionalParams);
  if (res < 0)
    return res;

  QBDRMAddCallbacks(appGlobals);

  /// do not start with any default frequency, wait for some service (tv/pvr) to set desired frequency first.
  appGlobals->catManager = QBCatManagerCreate(sched);

  int tunerCnt = QBTunerLogicGetTunerCount(appGlobals->tunerLogic);
  for (int i = 0; i < tunerCnt; i++) {
    QBCatManagerAddTuner(appGlobals->catManager, i, QBTunerGetEmptyMuxId());
  }

  return 0;
}

void QBDRMDeinit(AppGlobals appGlobals)
{
  if (appGlobals->catManager) {
    QBCatManagerDestroy(appGlobals->catManager);
    appGlobals->catManager = NULL;
  }

  SvInvokeInterface(QBCAS, QBCASGetInstance(), deinit);
  QBCryptoEngineDeinit(QBCryptoEngineGetInstance());
  SVTESTRELEASE(appGlobals->metaKeyManager);
  appGlobals->metaKeyManager = NULL;
}

void QBDRMStart(void)
{
  SvInvokeInterface(QBCAS, QBCASGetInstance(), start);
}

void QBDRMStop(void)
{
  SvInvokeInterface(QBCAS, QBCASGetInstance(), stop);
}

void QBDRMCleanup(void)
{
  QBCryptoEngineCleanUp();
  SvDRMManagerCleanup();
  QBCASDeinit();
}
