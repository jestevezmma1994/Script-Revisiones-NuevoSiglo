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

#include "upgradeWatcher.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvLog.h>
#include <fibers/c/fibers.h>
#include <sv_tuner.h>
#include <QBTunerTypes.h>
#include <QBTuner.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvArray.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBUpgrade/QBUpgradeObject.h>
#include <QBUpgrade/QBUpgradeCatalog.h>
#include <QBUpgrade/QBUpgradeDownloader.h>
#include <QBUpgrade/QBUpgradeDownloadListener.h>
#include <QBUpgrade/QBUpgradeMonitor.h>
#include <QBUpgrade/QBUpgradeMonitorListener.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <QBUpgrade/QBUpgradeVerifier.h>
#include <QBUpgrade/QBUpgradeVerifyListener.h>
#include <QBDVBMuxesMap.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>
#include <QBUpgrade/QBDVBCarousel.h>
#include <QBUpgrade/QBDVBCarouselDemuxer.h>
#include <QBUpgrade/QBDVBCarouselMonitor.h>
#include <QBUpgrade/QBDVBCarouselDownloader.h>
#include <QBUpgrade/QBNITUpgradeMonitor.h>
#include <QBUpgrade/QBHTTPUpgradeMonitor.h>
#include <QBConf.h>
#include <QBApplicationController.h>
#include <QBContextSwitcher.h>
#include <main.h>
#include <Widgets/upgradeDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Windows/channelscanning.h>
#include <tunerReserver.h>
#include <Logic/UpgradeLogic.h>
#include <QBDVBSatellitesDB.h>
#include <QBAppKit/QBGlobalStorage.h>


SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 3, "QBUpgradeLogLevel", "" );

#define log_debug(fmt, ...) if(env_log_level() >= 5) { SvLogNotice(COLBEG() "UpgradeWatcher :: %s " fmt COLEND_COL(green), __func__, ##__VA_ARGS__); }
#define log_info(fmt, ...)  if(env_log_level() >= 3) { SvLogNotice(COLBEG() "UpgradeWatcher :: %s " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); }
#define log_error(fmt, ...) if(env_log_level() >= 1) { SvLogError(COLBEG() "UpgradeWatcher :: %s " fmt COLEND_COL(red), __func__, ##__VA_ARGS__); }

typedef enum {
    UpgradeWatcherDownloadState_idle = 0,
    UpgradeWatcherDownloadState_waiting,
    UpgradeWatcherDownloadState_downloading
} UpgradeWatcherDownloadState;

struct UpgradeWatcher_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    bool running;
    QBUpgradeService service;
    bool dvbUpgradeEnabled;

    SvString originNetworkName;

    QBUpgradeObject downloadedObject;
    QBUpgradeVerifier verifier;

    QBHTTPUpgradeMonitor HTTPMonitor;

    QBNITUpgradeMonitor NITMonitor;
    QBDVBUpgradeSelector selector;

    bool channelScanningActive;
    struct {
        SvGenericObject monitor;
        SvString versionStr;
        SvString networkName;
        SvURI upgradeURI;
        bool downloaded;
        bool forced;
    } delayedUpgradeNotification;

    UpgradeWatcherDownloadState downloadState;

    QBTunerResv *reservation;

    int tunerNumber;

    SvFiber fiber;
    SvFiberTimer timer;

    QBDVBCarouselDemuxer demuxer;
    QBDVBCarouselMonitor monitor;
    SvString versionStr;
    SvURI upgradeURI;
    bool upgradeForced;
    QBDVBCarouselDownloader downloader;

    int chosenNID;

    bool ignoreVersion;
    bool treatAllAsForced;
    bool upgradeImmediately;
};

typedef struct UpgradeWatcher_ *UpgradeWatcher;

SvLocal SvString
UpgradeWatcherCreateStringFromFile(const char *filePath)
{
    SvString result = NULL;
    char buffer[256];
    FILE *f = NULL;

    if (!(f = fopen(filePath, "r")))
        return NULL;

    if (!fgets(buffer, sizeof(buffer), f))
        goto fini;

    char *endl = strchr(buffer, '\n');
    if (endl) {
        if (endl > buffer && *(endl - 1) == '\r')
            *(endl - 1) = '\0';
        else
            *endl = '\0';
    }

    result = SvStringCreate(buffer, NULL);

fini:
    if (f)
        fclose(f);
    return result;
}

SvLocal void
UpgradeWatcherRemoveFiles(const char *dirPath)
{
    struct dirent* direntBuffer = malloc(offsetof(struct dirent, d_name) + pathconf(dirPath, _PC_NAME_MAX) + 1);
    char pathBuffer[PATH_MAX + 1];
    struct dirent *entry;
    struct stat st;
    DIR *dir;

    if (!(dir = opendir(dirPath)))
        goto fini;

    SvLogNotice("CubiTV: removing all files from '%s' directory", dirPath);

    const size_t dirPathLen = strlen(dirPath);
    memcpy(pathBuffer, dirPath, dirPathLen);
    pathBuffer[dirPathLen] = '/';

    while (readdir_r(dir, direntBuffer, &entry) == 0 && entry) {
        if (entry->d_name[0] == '.') {
            // skip "." and ".." entries
            if (entry->d_name[1] == '\0' || (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))
                continue;
        }
        snprintf(pathBuffer + dirPathLen + 1, sizeof(pathBuffer) - dirPathLen - 1, "%s", entry->d_name);
        if (stat(pathBuffer, &st) == 0 && S_ISDIR(st.st_mode)) {
            UpgradeWatcherRemoveFiles(pathBuffer);
        } else {
            unlink(pathBuffer);
        }
    }

    closedir(dir);

fini:
    free(direntBuffer);
}

SvLocal void
UpgradeWatcherScheduleDelayedNotification(UpgradeWatcher self,
                                          SvGenericObject monitor,
                                          SvString versionStr,
                                          SvString networkName,
                                          SvURI upgradeURI,
                                          bool downloaded,
                                          bool forced)
{
    SvLogNotice("CubiTV: delaying notification about new firmware version, channel scanning in progress");

    SVTESTRELEASE(self->delayedUpgradeNotification.monitor);
    self->delayedUpgradeNotification.monitor = SVTESTRETAIN(monitor);
    SVTESTRELEASE(self->delayedUpgradeNotification.versionStr);
    self->delayedUpgradeNotification.versionStr = SVTESTRETAIN(versionStr);
    SVTESTRELEASE(self->delayedUpgradeNotification.networkName);
    self->delayedUpgradeNotification.networkName = SVTESTRETAIN(networkName);
    self->delayedUpgradeNotification.downloaded = downloaded;
    self->delayedUpgradeNotification.forced = forced;
    self->delayedUpgradeNotification.upgradeURI = SVTESTRETAIN(upgradeURI);
}

SvLocal void
UpgradeWatcherStartNITMonitor(UpgradeWatcher self)
{
    SvErrorInfo error = NULL;

    if (self->NITMonitor)
        return;

    self->NITMonitor = (QBNITUpgradeMonitor) SvTypeAllocateInstance(QBNITUpgradeMonitor_getType(), NULL);
    unsigned int tunersCount = QBTunerLogicGetTunerCount(self->appGlobals->tunerLogic);

    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    QBNITUpgradeMonitorInit((QBNITUpgradeMonitor) self->NITMonitor, self->service, &(self->selector), tunersCount, satellitesDB, &error);
    QBNITUpgradeMonitorSetChosenNID(self->NITMonitor, self->chosenNID, NULL);
    if (error) {
        SvErrorInfoDestroy(error);
        SVRELEASE(self->NITMonitor);
        self->NITMonitor = NULL;
    } else {
        if (self->appGlobals->dvbMuxesMap)
             QBNITUpgradeMonitorSetMuxesMap(self->NITMonitor, self->appGlobals->dvbMuxesMap, NULL);
        SvInvokeInterface(QBUpgradeMonitor, self->NITMonitor, setListener, (SvGenericObject) self, NULL);
        SvInvokeInterface(QBUpgradeMonitor, self->NITMonitor, setCheckPeriod, 60);
    }
}

SvLocal void
UpgradeWatcherDownloadProgressChanged(SvGenericObject self_,
                                      SvGenericObject downloader,
                                      const QBUpgradeDownloadProgress *const progress)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    if (self->downloadState == UpgradeWatcherDownloadState_downloading) {
        unsigned int p = (unsigned int)(progress->approxLinearProgress * 100.0f);
        QBUpgradeLogicDownloadProgressChanged(self->appGlobals->upgradeLogic, p);
        if (progress->downloadedFilesCount == progress->totalFilesCount) {
            SvLogNotice("CubiTV: download complete");
            SvString upgradeURIStr = SVSTRING("file:///shared/upgrade/");
            SvURI upgradeURI = SvURICreateWithString(upgradeURIStr, NULL);
            if (self->channelScanningActive) {
                UpgradeWatcherScheduleDelayedNotification(self, (SvGenericObject) self->monitor, self->versionStr, self->originNetworkName, upgradeURI, true, self->upgradeForced);
            } else {
                // downloaded new firmware version via DVB - signal that firmware can be installed
                QBUpgradeLogicUpgradeAvailable(self->appGlobals->upgradeLogic, (SvGenericObject) self->monitor, self->versionStr, self->originNetworkName, upgradeURI, self->upgradeForced, self->upgradeImmediately);
            }
            SVRELEASE(upgradeURI);
        }
    }
}

SvLocal void
UpgradeWatcherFileDownloaded(SvGenericObject self_,
                             SvGenericObject downloader,
                             QBUpgradeFile file)
{
}

SvLocal void
UpgradeWatcherDownloadFailed(SvGenericObject self_,
                             SvGenericObject downloader)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;
    SvFiberActivate(self->fiber);
}

SvLocal void
UpgradeWatcherFirmwareAvailable(SvGenericObject self_,
                                SvGenericObject monitor,
                                QBFirmwareVersion newVersion)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;
    QBFirmwareVersion localVersion = NULL, activeVersion = NULL;
    SvString localVersionStr, newVersionStr;

    localVersionStr = QBUpgradeServiceGetLocalVersion(self->service);
    localVersion = QBFirmwareVersionCreate(localVersionStr, NULL);
    newVersionStr = newVersion->fullVersion;
    SvLogNotice("CubiTV: local firmware version: %s", SvStringCString(localVersionStr));
    SvLogNotice("CubiTV: new firmware version:   %s", SvStringCString(newVersionStr));

    if (!self->ignoreVersion && !localVersion->isCurrent && !newVersion->isCurrent && QBFirmwareVersionCompare(newVersion, localVersion) <= 0) {
        goto fini;
    }

    if (monitor != (SvGenericObject) self->NITMonitor) {
        goto fini;
    }

    if (self->versionStr) {
        // we are already downloading some firmware
        activeVersion = QBFirmwareVersionCreate(self->versionStr, NULL);
        if (QBFirmwareVersionCompare(newVersion, activeVersion) == 0) {
            // found the same version again, check if upgrade URI changed
            SvURI newUpgradeURI = NULL;
            bool forced = false;
            QBNITUpgradeMonitorGetUpgradeInfo(self->NITMonitor, &newUpgradeURI, &forced, NULL);
            if (SvObjectEquals((SvObject) newUpgradeURI, (SvObject) self->upgradeURI)) {
                SvLogNotice("CubiTV: already downloading version %s", SvStringCString(self->versionStr));
                goto fini;
            } else {
                SvLogNotice("CubiTV: upgrade carousel location changed");
            }
        } else if (newVersion->isCurrent || QBFirmwareVersionCompare(newVersion, activeVersion) < 0) {
            SvLogNotice("CubiTV: already downloading newer version %s", SvStringCString(self->versionStr));
            goto fini;
        } else {
            SvLogNotice("CubiTV: new firmware version is available via DVB");
            // signal that previous download should be dropped
            QBUpgradeLogicDownloadAvailable(self->appGlobals->upgradeLogic, monitor, NULL, NULL, NULL, false, self->upgradeImmediately);
        }
    }

    // remember download parameters
    self->upgradeForced = false;
    SVTESTRELEASE(self->upgradeURI);
    QBNITUpgradeMonitorGetUpgradeInfo(self->NITMonitor, &(self->upgradeURI), &(self->upgradeForced), NULL);
    SVRETAIN(self->upgradeURI);

    if (self->treatAllAsForced) {
        self->upgradeForced = true;
    }

    if (self->ignoreVersion) {
        SvString uriStr = SvURIString(self->upgradeURI);
        SvString newUriStr = SvStringCreateWithFormat("%s:%s", SvStringCString(uriStr), "any_version=yes");
        SVRELEASE(uriStr);

        SVRELEASE(self->upgradeURI);
        self->upgradeURI = SvURICreateWithString(newUriStr, NULL);
        if (!self->upgradeURI) {
            SvLogError("Cannot create URI from (%s)", SvStringCString(newUriStr));
            SVRELEASE(newUriStr);
            goto fini;
        }
        SVRELEASE(newUriStr);
    }

    SVTESTRELEASE(self->originNetworkName);
    self->originNetworkName = QBNITUpgradeMonitorGetNetworkName((QBNITUpgradeMonitor) monitor);
    SVTESTRETAIN(self->originNetworkName);
    SVTESTRELEASE(self->versionStr);
    self->versionStr = SVRETAIN(newVersionStr);

    if (self->channelScanningActive) {
        UpgradeWatcherScheduleDelayedNotification(self, monitor, newVersionStr, self->originNetworkName, self->upgradeURI, false, self->upgradeForced);
    } else {
        // signal that firmware can be downloaded
        QBUpgradeLogicDownloadAvailable(self->appGlobals->upgradeLogic, monitor, newVersionStr, self->originNetworkName, self->upgradeURI, self->upgradeForced, self->upgradeImmediately);
    }

fini:
    SVTESTRELEASE(localVersion);
    SVTESTRELEASE(activeVersion);
}

SvLocal QBUpgradeObject
UpgradeWatcherFindAvailableUpgradeObject(UpgradeWatcher self,
                                         QBUpgradeCatalog catalog)
{
    SvString localVersionStr, localHash;
    SvString newVersionStr, newHash;
    SvString imageFileName;
    QBFirmwareVersion localVersion, newVersion;
    QBUpgradeObject obj = NULL;
    QBUpgradeFile imageFile;
    bool upgradeAvailable = false;
    bool equalHashes = false;

    localVersionStr = QBUpgradeServiceGetLocalVersion(self->service);
    localVersion = QBFirmwareVersionCreate(localVersionStr, NULL);
    localHash = QBUpgradeServiceGetLocalHash(self->service);
    imageFileName = QBUpgradeServiceGetImageFileName(self->service);

    SvIterator iter = SvArrayIterator(QBUpgradeCatalogGetObjects(catalog));
    while ((obj = (QBUpgradeObject) SvIteratorGetNext(&iter))) {
        // ignore upgrades for different set-top-boxes
        SvString objectBoardName = QBUpgradeObjectGetBoardName(obj);
        // HACK BEGIN
        if (SvStringEqualToCString(objectBoardName, "intekcuc"))
            objectBoardName = SVSTRING("intek");
        // HACK END
        if (!SvObjectEquals((SvObject) objectBoardName, (SvObject) QBUpgradeServiceGetBoardName(self->service))) {
            continue;
        }

        // check if upgrade object contains firmware image for this platform
        SvImmutableArray files = QBUpgradeObjectGetFiles(obj);
        SvIterator fileIter = SvImmutableArrayIterator(files);
        while ((imageFile = (QBUpgradeFile) SvIteratorGetNext(&fileIter))) {
            SvString fileName = QBUpgradeFileGetName(imageFile);
            if (SvObjectEquals((SvObject) fileName, (SvObject) imageFileName))
                break;
        }
        if (!imageFile)
            continue;

        newVersionStr = QBUpgradeObjectGetVersion(obj);
        newVersion = QBFirmwareVersionCreate(newVersionStr, NULL);
        newHash = QBUpgradeFileGetHash(imageFile);

        // standard upgrade logic
        equalHashes = SvObjectEquals((SvObject) localHash, (SvObject) newHash);
        if (localVersion->isCurrent || newVersion->isCurrent) {
            if (localVersion->isCurrent && newVersion->isCurrent)
                upgradeAvailable = !equalHashes;
            else
                upgradeAvailable = true;
        } else {
            int diff = QBFirmwareVersionCompare(localVersion, newVersion);
            upgradeAvailable = (diff < 0) || (diff == 0 && !equalHashes);
        }

        SVRELEASE(newVersion);

        if (upgradeAvailable)
            break;
    }

    SVRELEASE(localVersion);
    return obj;
}

SvLocal void
UpgradeWatcherCatalogFound(SvGenericObject self_,
                           SvGenericObject monitor,
                           QBUpgradeCatalog catalog)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;
    QBUpgradeObject obj = NULL;
    QBFirmwareVersion newVersion = NULL, activeVersion = NULL;

    obj = UpgradeWatcherFindAvailableUpgradeObject(self, catalog);
    if (!obj)
        return;

    SvString newVersionStr = QBUpgradeObjectGetVersion(obj);
    newVersion = QBFirmwareVersionCreate(newVersionStr, NULL);
    bool forced = QBUpgradeObjectIsForced(obj);

    if(self->versionStr) {
        activeVersion = QBFirmwareVersionCreate(self->versionStr, NULL);
        if (QBFirmwareVersionCompare(newVersion, activeVersion) != 0) {
            SvLogNotice("CubiTV: wait for signal that the firmware can be downloaded");
            goto fini;
        }
    }

    SvLogNotice("CubiTV: local firmware version: %s",
                SvStringCString(QBUpgradeServiceGetLocalVersion(self->service)));
    SvLogNotice("CubiTV: new firmware version:   %s", SvStringCString(newVersionStr));
    SvLogNotice("CubiTV: new firmware version is available");

    if (monitor == (SvGenericObject) self->HTTPMonitor) {
        SvURI upgradeURI = QBUpgradeObjectGetOrigin(obj);
        if (self->channelScanningActive) {
            UpgradeWatcherScheduleDelayedNotification(self, monitor, newVersionStr, NULL, upgradeURI, true, forced);
        } else {
            // found new firmware version via HTTP - signal that firmware can be installed immediately
            QBUpgradeLogicUpgradeAvailable(self->appGlobals->upgradeLogic, monitor, newVersionStr, NULL, upgradeURI, forced, self->upgradeImmediately);
        }
    } else if (monitor == (SvGenericObject) self->monitor) {
        self->downloadState = UpgradeWatcherDownloadState_downloading;
        SVTESTRELEASE(self->versionStr);
        self->versionStr = SVRETAIN(newVersionStr);
        self->upgradeForced = forced;

        self->downloader = (QBDVBCarouselDownloader) SvTypeAllocateInstance(QBDVBCarouselDownloader_getType(), NULL);
        QBDVBCarouselDownloaderInit(self->downloader, self->service, obj, self->demuxer, NULL);
        SvInvokeInterface(QBUpgradeDownloader, self->downloader, setDestination, SVSTRING("/shared/upgrade"), NULL);
        SvInvokeInterface(QBUpgradeDownloader, self->downloader, setListener, (SvGenericObject) self, NULL);
        SvInvokeInterface(QBUpgradeDownloader, self->downloader, start, self->appGlobals->scheduler, NULL);

        if (QBUpgradeCatalogGetRawVersion(catalog)) {
            SvString networkName = QBNITUpgradeMonitorGetNetworkName(self->NITMonitor);
            FILE *f = fopen("/shared/upgrade/network", "w");
            if (f) {
                fprintf(f, "%s\n", networkName ? SvStringCString(networkName) : "");
                fclose(f);
                QBUpgradeCatalogSaveToDirectory(catalog, "/shared/upgrade", NULL);
            }
        }

        QBUpgradeLogicDownloadProgressChanged(self->appGlobals->upgradeLogic, 0);
    }

fini:
    SVTESTRELEASE(newVersion);
    SVTESTRELEASE(activeVersion);
#if 0
    SvInvokeInterface(QBUpgradeMonitor, monitor, reset);
#endif
}

SvLocal void
UpgradeWatcherTunerReservationObtained(void *self_,
                                       QBTunerReserver *reserver,
                                       QBTunerResv *reservation,
                                       int tunerNumber)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    self->tunerNumber = tunerNumber;
    QBDVBCarouselDemuxerSetTuner(self->demuxer, self->tunerNumber, NULL);

    QBTunerType tunerType = QBTunerType_unknown;
    struct QBTunerParams tunerParams;
    QBDVBCarouselParseURI(self->upgradeURI, &tunerType, &tunerParams, NULL, NULL, NULL);

    if (self->downloadState != UpgradeWatcherDownloadState_idle) {
        struct QBTunerMuxId currentMux = sv_tuner_get_curr_mux_id(self->tunerNumber);
        if (!QBTunerMuxIdEqual(&currentMux, &(tunerParams.mux_id)))
            sv_tuner_tune(self->tunerNumber, &tunerParams);
    }
}

SvLocal void
UpgradeWatcherTunerReservationRevoked(void *self_,
                                      QBTunerReserver *reserver,
                                      QBTunerResv *reservation)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    self->tunerNumber = -1;
    QBDVBCarouselDemuxerSetTuner(self->demuxer, -1, NULL);
}

SvLocal void
UpgradeWatcherStep(void *self_)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    SvFiberDeactivate(self->fiber);

    QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
    if (self->tunerNumber >= 0) {
        QBTunerReserverRelease(tunerReserver, self->reservation);
        self->tunerNumber = -1;
    }

    SVTESTRELEASE(self->demuxer);
    self->demuxer = NULL;
    SVTESTRELEASE(self->monitor);
    self->monitor = NULL;
    SVTESTRELEASE(self->downloader);
    self->downloader = NULL;
    self->downloadState = UpgradeWatcherDownloadState_idle;

    remove("/shared/upgrade/catalog.xml");

    // setup DVB firmware download
    self->demuxer = (QBDVBCarouselDemuxer) SvTypeAllocateInstance(QBDVBCarouselDemuxer_getType(), NULL);
    QBDVBCarouselDemuxerInit(self->demuxer, self->upgradeURI, &(self->selector), NULL, NULL);
    self->monitor = (QBDVBCarouselMonitor) SvTypeAllocateInstance(QBDVBCarouselMonitor_getType(), NULL);
    QBDVBCarouselMonitorInit(self->monitor, self->service, self->demuxer, &(self->selector), NULL);
    SvInvokeInterface(QBUpgradeMonitor, self->monitor, setListener, (SvGenericObject) self, NULL);
    SvInvokeInterface(QBUpgradeMonitor, self->monitor, setCheckPeriod, 60);

    self->downloadState = UpgradeWatcherDownloadState_waiting;

    struct QBTunerParams tunerParams;
    QBTunerType tunerType = QBTunerType_unknown;
    QBDVBCarouselParseURI(self->upgradeURI, &tunerType, &tunerParams, NULL, NULL, NULL);
    self->reservation->params.priority = QBTUNERRESERVER_PRIORITY_UPGRADE;
    self->reservation->params.tunerParams = tunerParams;
    self->reservation->params.tunerOwnership = false;
    self->reservation->params.immediate = false;
    self->tunerNumber = QBTunerReserverObtain(tunerReserver, self->reservation);
    if (self->tunerNumber >= 0) {
        struct QBTunerMuxId currentMux = sv_tuner_get_curr_mux_id(self->tunerNumber);
        if (!QBTunerMuxIdEqual(&currentMux, &(tunerParams.mux_id)))
            sv_tuner_tune(self->tunerNumber, &tunerParams);
        QBDVBCarouselDemuxerSetTuner(self->demuxer, self->tunerNumber, NULL);
    }
}

SvLocal void
UpgradeWatcherVerificationProgressChanged(SvGenericObject self_,
                                          QBUpgradeVerifier verifier)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;
    unsigned int totalFilesCnt = 0, verifiedFilesCnt = 0, invalidFilesCnt = 0;

    QBUpgradeVerifierGetResult(verifier, &totalFilesCnt, &verifiedFilesCnt, &invalidFilesCnt, NULL);
    if (verifiedFilesCnt != totalFilesCnt) {
        // still working
        return;
    }

    QBUpgradeVerifierStop(self->verifier, NULL);

    if (invalidFilesCnt != 0) {
        SvLogNotice("CubiTV: previous download is incomplete, has %u out of %u files downloaded",
                    verifiedFilesCnt - invalidFilesCnt, totalFilesCnt);
        SVTESTRELEASE(self->downloadedObject);
        self->downloadedObject = NULL;
        // we won't remove the files, download can be resumed from current point
    } else {
        // found complete download
        SvString newVersionStr = QBUpgradeObjectGetVersion(self->downloadedObject);
        bool forced = QBUpgradeObjectIsForced(self->downloadedObject);

        SvLogNotice("CubiTV: previously downloaded upgrade image contains new firmware version");
        SvLogNotice("CubiTV: local firmware version: %s",
                    SvStringCString(QBUpgradeServiceGetLocalVersion(self->service)));
        SvLogNotice("CubiTV: new firmware version:   %s", SvStringCString(newVersionStr));
        if (forced)
            SvLogNotice("CubiTV: upgrade is forced");

        SVTESTRELEASE(self->versionStr);
        self->versionStr = SVRETAIN(newVersionStr);

        SvString upgradeURIStr = SVSTRING("file:///shared/upgrade/");
        SvURI upgradeURI = SvURICreateWithString(upgradeURIStr, NULL);
        if (self->channelScanningActive) {
            UpgradeWatcherScheduleDelayedNotification(self, NULL, newVersionStr, self->originNetworkName, upgradeURI, true, forced);
        } else {
            QBUpgradeLogicUpgradeAvailable(self->appGlobals->upgradeLogic, NULL, newVersionStr, self->originNetworkName, upgradeURI, forced, self->upgradeImmediately);
        }
        SVRELEASE(upgradeURI);
    }

    UpgradeWatcherStartNITMonitor(self);
}

SvLocal void
UpgradeWatcherContextSwitchStarted(SvGenericObject self_,
                                   QBWindowContext from,
                                   QBWindowContext to)
{
    if (to && SvObjectIsInstanceOf((SvObject) to, QBChannelScanningContext_getType())) {
        // delay notifications about firmware upgrade availability
        ((UpgradeWatcher) self_)->channelScanningActive = true;
    }
}

SvLocal void
UpgradeWatcherContextSwitchEnded(SvGenericObject self_,
                                 QBWindowContext from,
                                 QBWindowContext to)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    if (!from || !SvObjectIsInstanceOf((SvObject) from, QBChannelScanningContext_getType()))
        return;

    self->channelScanningActive = false;

    if (self->delayedUpgradeNotification.versionStr) {
        SvGenericObject monitor = self->delayedUpgradeNotification.monitor;
        self->delayedUpgradeNotification.monitor = NULL;
        SvString versionStr = self->delayedUpgradeNotification.versionStr;
        self->delayedUpgradeNotification.versionStr = NULL;
        SvString networkName = self->delayedUpgradeNotification.networkName;
        self->delayedUpgradeNotification.networkName = NULL;
        bool forced = self->delayedUpgradeNotification.forced;
        SvURI upgradeURI = self->delayedUpgradeNotification.upgradeURI;
        self->delayedUpgradeNotification.upgradeURI = NULL;

        SvLogNotice("CubiTV: channel scanning finished, notifying about new firmware version");
        if (self->delayedUpgradeNotification.downloaded)
            QBUpgradeLogicUpgradeAvailable(self->appGlobals->upgradeLogic, monitor, versionStr, networkName, upgradeURI, forced, self->upgradeImmediately);
        else
            QBUpgradeLogicDownloadAvailable(self->appGlobals->upgradeLogic, monitor, versionStr, networkName, upgradeURI, forced, self->upgradeImmediately);

        SVTESTRELEASE(monitor);
        SVTESTRELEASE(versionStr);
        SVTESTRELEASE(networkName);
        SVTESTRELEASE(upgradeURI);
    }
}

SvLocal void
UpgradeWatcherUpdateChosenNID(UpgradeWatcher self)
{
    int confTmp = 0;
    int status = QBConfigGetInteger("NID", &confTmp);
    self->chosenNID = status ? -1 : confTmp;

    if (self->NITMonitor) {
        QBNITUpgradeMonitorSetChosenNID(self->NITMonitor, self->chosenNID, NULL);
    }
}

SvLocal void
UpgradeWatcherConfigChanged(SvGenericObject self_, const char *key, const char *value)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    if (!value) {
        return;
    }

    UpgradeWatcherUpdateChosenNID(self);
}

SvLocal void
UpgradeWatcher__dtor__(void *self_)
{
    UpgradeWatcher self = self_;

    if (self->running)
        QBUpgradeServiceStop(self->service, NULL);

    if (self->fiber)
        SvFiberDestroy(self->fiber);

    SVTESTRELEASE(self->originNetworkName);

    SVTESTRELEASE(self->HTTPMonitor);
    SVTESTRELEASE(self->NITMonitor);
    SVTESTRELEASE(self->verifier);
    SVTESTRELEASE(self->service);
    SVTESTRELEASE(self->downloadedObject);

    SVTESTRELEASE(self->delayedUpgradeNotification.monitor);
    SVTESTRELEASE(self->delayedUpgradeNotification.versionStr);
    SVTESTRELEASE(self->delayedUpgradeNotification.networkName);
    SVTESTRELEASE(self->delayedUpgradeNotification.upgradeURI);

    if (self->tunerNumber >= 0) {
        QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
        QBTunerReserverRelease(tunerReserver, self->reservation);
    }
    SVTESTRELEASE(self->reservation);

    SVTESTRELEASE(self->downloader);
    SVTESTRELEASE(self->monitor);
    SVTESTRELEASE(self->demuxer);
    SVTESTRELEASE(self->versionStr);
    SVTESTRELEASE(self->upgradeURI);
}

SvLocal SvType
UpgradeWatcher_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = UpgradeWatcher__dtor__
    };
    static const struct QBUpgradeVerifyListener_ verifyListenerMethods = {
        .progressChanged = UpgradeWatcherVerificationProgressChanged
    };
    static const struct QBUpgradeMonitorListener_ monitorListenerMethods = {
        .firmwareAvailable = UpgradeWatcherFirmwareAvailable,
        .catalogFound = UpgradeWatcherCatalogFound
    };
    static const struct QBUpgradeDownloadListener_ downloadListenerMethods = {
        .progressChanged    = UpgradeWatcherDownloadProgressChanged,
        .fileDownloaded     = UpgradeWatcherFileDownloaded,
        .downloadFailed     = UpgradeWatcherDownloadFailed
    };
    static const struct QBContextSwitcherListener_t switcherListenerMethods = {
        .started            = UpgradeWatcherContextSwitchStarted,
        .ended              = UpgradeWatcherContextSwitchEnded
    };
    static const struct QBConfigListener_t configMethods = {
        .changed = UpgradeWatcherConfigChanged
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("UpgradeWatcher",
                            sizeof(struct UpgradeWatcher_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBUpgradeVerifyListener_getInterface(), &verifyListenerMethods,
                            QBUpgradeMonitorListener_getInterface(), &monitorListenerMethods,
                            QBUpgradeDownloadListener_getInterface(), &downloadListenerMethods,
                            QBContextSwitcherListener_getInterface(), &switcherListenerMethods,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }

    return type;
}

SvGenericObject
UpgradeWatcherCreate(AppGlobals appGlobals)
{
    const QBDVBUpgradeSelector *selector;
    UpgradeWatcher self;
    const char *tmp;

    self = (UpgradeWatcher) SvTypeAllocateInstance(UpgradeWatcher_getType(), NULL);
    self->appGlobals = appGlobals;
    self->tunerNumber = -1;

    self->service = QBUpgradeServiceCreate(NULL);
    if (!self->service) {
        SvLogError("CubiTV: error creating QBUpgradeService");
        SVRELEASE(self);
        return NULL;
    }

    if (!(selector = QBUpgradeServiceGetSelector(self->service))) {
        SvLogWarning("CubiTV: unknown selector, DVB upgrade is not supported!");
    } else {
        self->dvbUpgradeEnabled = true;
        self->selector = *QBUpgradeServiceGetSelector(self->service);
    }

    if ((tmp = QBConfigGet("UPGRADE.SERVER"))) {
        // use HTTP upgrade monitor
        SvString tmpStr = SvStringCreate(tmp, NULL);
        SvURI upgradeURL = SvURICreateWithString(tmpStr, NULL);
        SVAUTORELEASE(tmpStr);
        self->HTTPMonitor = (QBHTTPUpgradeMonitor) SvTypeAllocateInstance(QBHTTPUpgradeMonitor_getType(), NULL);
        SvSSLParams sslParams = QBUpgradeLogicCreateSSLParams(self->appGlobals->upgradeLogic);
        QBHTTPUpgradeMonitorInit(self->HTTPMonitor, self->service, upgradeURL,
                                 QBUpgradeServiceGetImageFileName(self->service),
                                 QBUpgradeServiceGetBoardName(self->service),
                                 sslParams,
                                 NULL);
        SVRELEASE(upgradeURL);
        SVRELEASE(sslParams);

        SvInvokeInterface(QBUpgradeMonitor, self->HTTPMonitor, setListener, (SvGenericObject) self, NULL);
        // check for new firmware availability every 10 minutes
        SvInvokeInterface(QBUpgradeMonitor, self->HTTPMonitor, setCheckPeriod, 10 * 60);
    } else {
        SvLogWarning("CubiTV: UPGRADE.SERVER config variable not found");
    }

    if (self->dvbUpgradeEnabled) {
        if (!(tmp = QBConfigGet("DVBUPGRADE.ENABLED")) || strcmp(tmp, "enabled") != 0) {
            SvLogWarning("CubiTV: DVB upgrade explicitly disabled!");
            self->dvbUpgradeEnabled = false;
        }
    }
    if (!self->dvbUpgradeEnabled)
        return (SvGenericObject) self;

    QBApplicationControllerAddListener(appGlobals->controller, (SvGenericObject) self);

    self->downloadState = UpgradeWatcherDownloadState_idle;
    static struct QBTunerResvCallbacks_s tunerReserverMethods = {
        .tunerRevoked  = UpgradeWatcherTunerReservationRevoked,
        .tunerObtained = UpgradeWatcherTunerReservationObtained
    };
    self->reservation = QBTunerResvCreate(SVSTRING("UpgradeWatcher"), self, &tunerReserverMethods);

    self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "UpgradeWatcher",
                                UpgradeWatcherStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    // check if there is an already downloaded upgrade
    if (access("/shared/upgrade/catalog.xml", R_OK) == 0) {
        QBUpgradeCatalog catalog = QBUpgradeCatalogCreateFromFile("/shared/upgrade/catalog.xml", NULL);
        if (catalog) {
            QBUpgradeObject obj = UpgradeWatcherFindAvailableUpgradeObject(self, catalog);
            if (obj) {
                self->downloadedObject = SVRETAIN(obj);
                SVTESTRELEASE(self->originNetworkName);
                self->originNetworkName = UpgradeWatcherCreateStringFromFile("/shared/upgrade/network");
                self->verifier = QBUpgradeVerifierCreate(NULL);
            }
            SVRELEASE(catalog);
        }
    }

    QBConfigAddListener((SvGenericObject) self, "NID");
    UpgradeWatcherUpdateChosenNID(self);

    if (!self->verifier) {
        UpgradeWatcherRemoveFiles("/shared/upgrade");
        UpgradeWatcherStartNITMonitor(self);
    }

    return (SvGenericObject) self;
}

void
UpgradeWatcherStart(SvGenericObject self_)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    if (self->running || !self->service)
        return;

    self->running = true;

    QBUpgradeServiceStart(self->service, self->appGlobals->scheduler, NULL);

    if (self->HTTPMonitor) {
        // check for new firmware availability every 600 seconds
        SvInvokeInterface(QBUpgradeMonitor, self->HTTPMonitor, setCheckPeriod, 600);
    }

    if (self->verifier) {
        SvLogNotice("CubiTV: found previously downloaded upgrade, verifying");
        QBUpgradeVerifierStart(self->verifier, self->appGlobals->scheduler, (SvGenericObject) self,
                               self->downloadedObject, SVSTRING("/shared/upgrade"), NULL);
    }
}

void
UpgradeWatcherStop(SvGenericObject self_)
{
    UpgradeWatcher self = (UpgradeWatcher) self_;

    if (!self->running)
        return;

    self->running = false;

    QBUpgradeServiceStop(self->service, NULL);

    if (self->verifier)
        QBUpgradeVerifierStop(self->verifier, NULL);

    if (self->tunerNumber >= 0) {
        QBTunerReserver *tunerReserver = (QBTunerReserver *) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTunerReserver"));
        self->tunerNumber = -1;
        QBTunerReserverRelease(tunerReserver, self->reservation);
    }
}

void
UpgradeWatcherStartDownload(SvGenericObject self_,
                            SvString version)
{
    log_info();
    UpgradeWatcher self = (UpgradeWatcher) self_;
    if (self->fiber)
        SvFiberActivate(self->fiber);
}

void
UpgradeWatcherForceHTTPUpgradeCheck(SvObject self_)
{
    log_info();
    UpgradeWatcher self = (UpgradeWatcher) self_;
    if (self->HTTPMonitor) {
        QBHTTPUpgradeMonitorCheckNow((SvGenericObject) self->HTTPMonitor, NULL);
    }
}

QBDVBUpgradeSelector*
UpgradeWatcherGetSelector(SvGenericObject self_)
{
    if (!self_)
        return NULL;
    UpgradeWatcher self = (UpgradeWatcher) self_;

    if (self->dvbUpgradeEnabled) {
        return &self->selector;
    } else {
        return NULL;
    }
}

void
UpgradeWatcherRefreshNITUpgrade(SvGenericObject self_)
{
    log_info();

    if (!self_)
        return;
    UpgradeWatcher self = (UpgradeWatcher) self_;

    SVTESTRELEASE(self->versionStr);
    self->versionStr = NULL;

    // signal that previous download should be dropped
    QBUpgradeLogicDownloadAvailable(self->appGlobals->upgradeLogic, (SvObject) self->NITMonitor, NULL, NULL, NULL, false, self->upgradeImmediately);
    SvInvokeInterface(QBUpgradeMonitor, self->NITMonitor, reset);
}

void
UpgradeWatcherIgnoreVersion(SvGenericObject self_, bool enabled)
{
    log_info();

    if (!self_)
        return;
    UpgradeWatcher self = (UpgradeWatcher) self_;

    self->ignoreVersion = enabled;
}

void
UpgradeWatcherSetAllForced(SvGenericObject self_, bool forced)
{
    log_info();

    if (!self_)
        return;
    UpgradeWatcher self = (UpgradeWatcher) self_;

    self->treatAllAsForced = forced;
}

void
UpgradeWatcherUpgradeImmediately(SvGenericObject self_, bool immediately)
{
    log_info();

    if (!self_)
        return;
    UpgradeWatcher self = (UpgradeWatcher) self_;

    self->upgradeImmediately = immediately;
}
