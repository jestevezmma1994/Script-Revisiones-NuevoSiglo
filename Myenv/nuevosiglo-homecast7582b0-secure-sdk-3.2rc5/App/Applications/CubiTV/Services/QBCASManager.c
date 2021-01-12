/*****************************************************************************
** Cubiware K.K. Software License Version 1.1
**
** Copyright (C) 2009-2015 Cubiware K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware K.K.
**
** Any User wishing to make use of this Software must contact Cubiware K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QBDRMPOPUPMANAGER_C_
#define QBDRMPOPUPMANAGER_C_

#include <main.h>
#include <Services/QBCASManager.h>
#include <Services/Conax/QBConaxMailManager.h>
#include <Services/Conax/QBConaxMailIndicator.h>
#include <Services/QBViewRightManagers/QBViewRightIPTVManager.h>
#include <Services/QBViewRightManagers/QBViewRightDVBPVRManager.h>
#include <Services/QBCryptoguardPopupManager.h>
#include <Services/QBCryptoguardVideoRulesManager.h>
#include <Services/QBCryptoguardForceTuneManager.h>
#include <Services/QBLatensPopupManager.h>
#include <Configurations/QBDiskPvrStorageMode.h>
#include <Services/QBAccessController/ViewRightAccessPlugin.h>
#include <Services/QBAccessController/ConaxAccessPlugin.h>
#include <Services/QBAccessController/LatensAccessPlugin.h>
#include <QBConfig.h>

#include <SvFoundation/SvInterface.h>
#include <QBCAS.h>
#include <QBViewRight.h>
#include <QBCASCommonCryptoguard.h>
#include <QBLatens.h>
#include <SvQuirks/SvRuntimePrefix.h>

#include <Services/Conax/QBConaxURIManager.h>
#include <Services/Conax/QBConaxPVRManager.h>

SvInterface QBCASPopupManager_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBCASPopupManager", sizeof(struct QBCASPopupManager_),
                NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

SvInterface QBCASPVRManager_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBCASPVRManager", sizeof(struct QBCASPVRManager_),
                NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

SvInterface QBCASPVRPlaybackMonitor_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBCASPVRPlaybackMonitor", sizeof(struct QBCASPVRPlaybackMonitor_),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

void QBCASCreateManagers(AppGlobals appGlobals)
{
    QBCASType casType = SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType);
    switch(casType) {
    case QBCASType_viewrightDVB:
        SvLogNotice("Creating ViewRight popup manager");
        appGlobals->casPopupManager = (SvGenericObject) QBViewRightPopupManagerCreate(appGlobals);
        SvLogNotice("Creating ViewRight trigger manager");
        SvString configFile = SvStringCreateWithFormat("%s/etc/viewRightManager", SvGetRuntimePrefix());
        appGlobals->viewRightTriggerManager = QBViewRightTriggerManagerCreate(appGlobals, configFile);
        SVRELEASE(configFile);

        QBDiskPvrStorageMode pvrStorageMode = QBDiskPvrStorageModeGet();
        bool reencryptionEnabled = pvrStorageMode == QBDiskPvrStorageMode_dvbOnlyReencryption || pvrStorageMode == QBDiskPvrStorageMode_reencryption;

        if (appGlobals->pvrProvider &&
            QBPVRProviderGetCapabilities(appGlobals->pvrProvider)->type == QBPVRProviderType_disk &&
            reencryptionEnabled) {
            SvLogNotice("Creating ViewRightDVB PVR manager");
            appGlobals->casPVRManager = (SvGenericObject) QBViewRightDVBPVRManagerCreate(appGlobals);
        }

        break;
    case QBCASType_cryptoguard:
        // CryptoGuard uses the same popup manager as Conax
        SvLogNotice("Creating conax popup manager");
        appGlobals->casPopupManager = (SvGenericObject) QBConaxPopupManagerCreate(appGlobals);
        break;
    case QBCASType_conax:
        SvLogNotice("Creating conax popup manager");
        appGlobals->casPopupManager = (SvGenericObject) QBConaxPopupManagerCreate(appGlobals);
        SvLogNotice("Creating fingerprint service");
        appGlobals->fingerprint = QBFingerprintCreate(appGlobals);
        SvLogNotice("Creating conax mail manager");
        appGlobals->conaxMailManager = QBConaxMailManagerCreate(appGlobals, NULL);
        SvLogNotice("Creating conax mail indicator");
        appGlobals->mailIndicator = ConaxMailIndicatorCreate(appGlobals);
        SvLogNotice("Creating conax URI manager");
        appGlobals->conaxURIManager = QBConaxURIManagerCreate(appGlobals);
        if (appGlobals->pvrProvider &&
            QBPVRProviderGetCapabilities(appGlobals->pvrProvider)->type == QBPVRProviderType_disk) {
            SvLogNotice("Creating conax PVR manager");
            appGlobals->casPVRManager = (SvGenericObject) QBConaxPVRManagerCreate(appGlobals);
        }
        break;
    case QBCASType_viewrightIPTV:
        SvLogNotice("Creating ViewRight iptv popup manager");
        appGlobals->casPopupManager = (SvGenericObject) QBViewRightIPTVServiceCreate(appGlobals);
        break;
    case QBCASType_cryptoguard2:
        SvLogNotice("Creating Cryptoguard popup manager");
        appGlobals->casPopupManager = (SvGenericObject) QBCryptoguardPopupManagerCreate(appGlobals);
        SvLogNotice("Creating Cryptoguard video rules manager");
        appGlobals->cryptoguardVideoRulesManager = QBCryptoguardVideoRulesManagerCreate(appGlobals);
        SvLogNotice("Creating Cryptoguard force tune manager");
        appGlobals->cryptoguardForceTuneManager = QBCryptoguardForceTuneManagerCreate(appGlobals);
        break;
    case QBCASType_latens:
        SvLogNotice("Creating Latens popup manager");
        appGlobals->casPopupManager = (SvGenericObject) QBLatensPopupManagerCreate(appGlobals);
        break;
    default:
        SvLogNotice("%s: Unknown CAS system - can not create proper managers", __func__);
        break;
    }
}
SvGenericObject QBCASCreateAccessPlugin(AppGlobals appGlobals)
{
    QBCASType casType = SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType);
    SvGenericObject plugin = NULL;

    switch(casType) {
    case QBCASType_viewrightDVB:
        plugin = SvTypeAllocateInstance(ViewRightAccessPlugin_getType(), NULL);
        ViewRightAccessPluginInit((ViewRightAccessPlugin) plugin, 5, NULL);
        break;
    case QBCASType_conax:
        plugin = SvTypeAllocateInstance(ConaxAccessPlugin_getType(), NULL);
        ConaxAccessPluginInit((ConaxAccessPlugin) plugin, appGlobals->net_manager, 5, NULL);
        break;
    case QBCASType_latens:
        SvLogNotice("CAS system is Latens");
        plugin = SvTypeAllocateInstance(LatensAccessPlugin_getType(), NULL);
        LatensAccessPluginInit((LatensAccessPlugin) plugin, NULL);
        break;
    default:
        SvLogNotice("%s: Unknown CAS system - can not create access plugin", __func__);
        break;
    }
    return plugin;
}

#endif /* QBDRMPOPUPMANAGER_C_ */
