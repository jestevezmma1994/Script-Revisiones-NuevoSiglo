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

#include <QBContentManager/QBContentProvider.h>
#include <sv_tuner.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>
#include <QBPlatformHAL/QBPlatformOption.h>
#include <QBTunerLNBConfig.h>
#include <QBTuner.h>
#include <QBTunerSettings.h>
#include <QBConf.h>
#include <QBDVBSatellitesDB.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <QBAppKit/QBServiceRegistry.h>

#include <main.h>
#include <Logic/TVLogic.h>
#include <Logic/StandbyLogic.h>
#include <Logic/TunerLogic.h>
#include <Logic/TunerLogicInternal.h>

/*
 * This logic is not for deciding who, when TV and Radio want tuner, should
 * get access to it, this is decided by other logic. Here all what is decided
 * is XOR on (radio,context) and (channel scanning) tuner requests.
 */

#define MAX_TUNERS 2

SvLocal void
QBTunerLogicConfigChanged(SvObject self_, const char *key, const char *value)
{
    const char* tunerType = QBConfigGet("DEFAULTTUNERTYPE");
    if (!tunerType || strncmp(tunerType, "cab", 3) != 0) {
        return;
    }
    bool isValueValid = false;
    if (!strcmp(key, "TUNER.SPECTRAL_INVERSION_MODE")) {
        if (value) {
            if (strcmp(value, "inverted") == 0) {
                QBTunerSettingsSetSpectralInversionMode(QBTunerSettingsSpectralInversionMode_inverted);
                isValueValid = true;
            } else if (strcmp(value, "auto") == 0) {
                QBTunerSettingsSetSpectralInversionMode(QBTunerSettingsSpectralInversionMode_auto);
                isValueValid = true;
            } else if (strcmp(value, "normal") == 0) {
                QBTunerSettingsSetSpectralInversionMode(QBTunerSettingsSpectralInversionMode_normal);
                isValueValid = true;
            }
        }
        if (isValueValid) {
            SvLogNotice("%s(): TunerLogic spectral inversion mode: %s", __func__, value);
        } else {
            SvLogWarning("%s(): Invalid value in config! key: \"TUNER.SPECTRAL_INVERSION_MODE\" value: %s. Spectral inversion mode set to normal.", __func__, value);
            QBTunerSettingsSetSpectralInversionMode(QBTunerSettingsSpectralInversionMode_normal);
        }
    }
}

SvLocal void QBTunerLogicDestroy(void *self_)
{
    QBTunerLogic self = self_;
    for (int i = self->tunerCnt-1; i>=0 ; i--) {
        sv_tuner_t* tuner = sv_tuner_get(i);
        if (tuner)
            sv_tuner_destroy(tuner);
    }
}

SvType QBTunerLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTunerLogicDestroy
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBTunerLogicConfigChanged
    };

    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBTunerLogic",
                            sizeof(struct QBTunerLogic_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }
    return type;
}

static int init_tuner_x(int x, int tunerId, bool allow_null)
{
  struct sv_tuner_state* tuner = sv_tuner_create("native", x, tunerId);
  if (tuner)
    return 0;
  if (!allow_null) {
    SvLogWarning("CubiTV: error initializing tuner %d (native)", x);
    return -1;
  };

  SvLogWarning("CubiTV3: error initializing tuner %d (native) - trying (null) tuner instead.", x);
  tuner = sv_tuner_create("null", x, 0);
  if (tuner)
    return 0;
  SvLogError(COLBEG() "CubiTV3: error initializing tuner %d (null) - will continue anyway!" COLEND_COL(red), x);
  return -1;
}

static void init_all_tuners(QBTunerLogic self)
{
    const struct QBTunerConfigSet* configSets = NULL;
    int configSetCnt = 0;
    QBTunerGetConfig("native", NULL, NULL, &configSets, &configSetCnt);
    const struct QBTunerConfigSet* configSet = &configSets[self->configSetIdx];

    const char* spectralInversion = QBConfigGet("TUNER.SPECTRAL_INVERSION_MODE");
    QBTunerLogicConfigChanged((SvObject)self, "TUNER.SPECTRAL_INVERSION_MODE", spectralInversion);

    self->tunerCnt = 0;
    for (int i = 0; i < configSet->cnt; i++) {
        bool allow_null = (i == 0);
        int res = init_tuner_x(i, configSet->tab[i], allow_null);
        if (res < 0)
            break;
        self->tunerCnt += 1;
    }
}

static void init_satellite(QBTunerLogic self)
{
    QBTunerType type = QBTunerType_unknown;
    int i;

    self->haveSatelliteTuner = false;

    for (i = 0; i < self->tunerCnt; i++) {
        // check if tuner is a DVB-S one
        if (sv_tuner_get_type(i, &type) >= 0 && type == QBTunerType_sat) {
            QBTunerLoadLNBConfig(NULL);

            QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
            QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
                QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

            if (!satellitesDB) {
                satellitesDB = QBDVBSatellitesDBCreate(NULL);
                assert(satellitesDB != NULL);
                QBDVBSatellitesDBSetDefaultDBDir(satellitesDB, SVSTRING("/usr/share/transponders"));
                QBDVBSatellitesDBSetUserDBDir(satellitesDB, SVSTRING("/etc/vod/app/transponders"));

                QBGlobalStorageSetItem(globalStorage, QBDVBSatellitesDB_getType(), (SvObject) satellitesDB, NULL);
                SVRELEASE(satellitesDB);
            }
            self->haveSatelliteTuner = true;
            break;
        }
    }
}

QBTunerLogic QBTunerLogicCreate(AppGlobals appGlobals)
{
    QBTunerLogic self = (QBTunerLogic) SvTypeAllocateInstance(QBTunerLogic_getType(), NULL);

    self->appGlobals = appGlobals;
    self->configSetIdx = -1;

    const struct QBTunerConfigSet* configSets = NULL;
    int configSetCnt = 0;
    QBTunerGetConfig("native", NULL, NULL, &configSets, &configSetCnt);

    const char* setName = QBConfigGet("DEFAULTTUNERTYPE");
    /// Try to find an exact match.
    int s = setName ? QBTunerFindConfigSetByName("native", setName) : -1;
    if (s < 0)
    {
        /// Try to find all with longest prefix, and then choose the one that provides desired scan-standard.
        const char* standardName = QBConfigGet("SCANSTANDARD");
        SvLogWarning("Incorrect value for [DEFAULTTUNERTYPE] var (%s), trying to deduce from [SCANSTANDARD] (%s)",
                     setName, standardName ? standardName : "unknown scan standard");

        QBTunerStandard standard = QBTunerStandard_unknown;
        if (standardName)
            standard = QBTunerStandardFromString(standardName);

        if (!setName)
            setName = "";
        s = QBTunerFindConfigSet("native", setName, standard);
        if (s < 0 && *setName) {
            SvLogWarning("Could not match any tuner configuration with [%s] prefix, trying search with no prefix", setName);
            s = QBTunerFindConfigSet("native", "", standard);
        }
        if (s < 0) {
            SvLogWarning("Could not match any tuner configuration - using config 0");
            s = 0;
        }
        setName = configSets[s].name;

        SvLogWarning("Falling back to [%s]", setName);
        QBConfigSet("DEFAULTTUNERTYPE", setName);
        QBConfigSave();
    }
    QBConfigAddListener((SvObject)self, "TUNER.SPECTRAL_INVERSION_MODE");
    SvLogNotice("TunerLogic : using config set %d [%s]", s, setName);
    self->configSetIdx = s;
    init_all_tuners(self);
    init_satellite(self);

    return self;
}

void QBTunerLogicStart(QBTunerLogic self)
{
    int masterTuner = QBTunerGetMaster("native");
    QBTunerReserver* tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    if (masterTuner >= 0) {
        QBTunerReserverSetMasterTuner(tunerReserver, masterTuner);
    }
}

void QBTunerLogicStop(QBTunerLogic self)
{
    QBConfigRemoveListener((SvObject)self, "TUNER.SPECTRAL_INVERSION_MODE");
    for (int i = 0; i < self->tunerCnt; i++)
        sv_tuner_untune(i);
}

void QBTunerLogicSwitchAllTuners(QBTunerLogic self, const char* configSetName)
{
    int configSetIdx = QBTunerFindConfigSetByName("native", configSetName);
    if (configSetIdx < 0) {
        SvLogError("QBTunerLogicSwitchAllTuners(%s) -> given tuner config set does not exist", configSetName);
        return;
    }

    if (self->configSetIdx == configSetIdx)
        return;

    const struct QBTunerConfigSet* configSet = QBTunerGetConfigSet("native", configSetIdx);

    if (self->appGlobals->pvrProvider &&
        self->appGlobals->pvrProviderInitialized &&
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type == QBPVRProviderType_disk)
        QBContentProviderStop((QBContentProvider) self->appGlobals->pvrProvider);

    QBConfigSet("DEFAULTTUNERTYPE", configSet->name);
    QBConfigSave();

    for (int i = 0; i < self->tunerCnt; i++)
        sv_tuner_untune(i);

    for (int i = self->tunerCnt-1; i>=0 ; i--) {
        sv_tuner_t* tuner = sv_tuner_get(i);
        if (tuner)
            sv_tuner_destroy(tuner);
    }
    self->configSetIdx = configSetIdx;

    init_all_tuners(self);
    init_satellite(self);

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    if (satellitesDB)
        QBDVBSatellitesDBLoad(satellitesDB, self->appGlobals->scheduler, NULL);

    if (self->appGlobals->pvrProvider &&
        self->appGlobals->pvrProviderInitialized &&
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type == QBPVRProviderType_disk)
        QBContentProviderStart((QBContentProvider) self->appGlobals->pvrProvider, self->appGlobals->scheduler);
}

int QBTunerLogicGetTunerCount(QBTunerLogic self)
{
    return self->tunerCnt;
}

bool QBTunerLogicNeedsTunersReinit(QBTunerLogic self)
{
    const char* tunerType = QBConfigGet("DEFAULTTUNERTYPE");
    bool needsTunersReinit = false;

    if (tunerType) {
        int configSetIdx = QBTunerFindConfigSetByName("native", tunerType);
        if (configSetIdx >= 0 && self->configSetIdx != configSetIdx)
            needsTunersReinit = true;
    }

    return needsTunersReinit;
}
