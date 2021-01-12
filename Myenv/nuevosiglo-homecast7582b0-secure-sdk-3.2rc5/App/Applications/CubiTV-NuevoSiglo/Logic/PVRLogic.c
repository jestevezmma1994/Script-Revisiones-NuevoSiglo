/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include <errno.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerKit/SvContentResources.h>
#include <main.h>
#include <Services/QBPVODStorage.h>
#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <Windows/pvrplayer.h>
#include <Logic/PVRLogic.h>
#include <QBRecordFS/file.h>
#include <QBRecordFS/root.h>
#include <QBUSBPerfTester.h>
#include <SvFoundation/SvData.h>
#include <libintl.h>
#include <QBConf.h>
#include <settings.h>

SV_DECL_STRING_ENV_FUN(env_recordfs_data_root, "RecordFsDataRoot", "MSTORE_FAKE_DIR");

//Disks up to this size allow for timeshift only
#define MAX_TIMESHIFT_DISK_SIZE 1ll * 1024 * 1024 * 1024
//Disks only up from this size allow for MStore
#define MIN_MSTORE_DISK_SIZE 1ll * 1024 * 1024 * 1024

//Disks are formatted with 1MB start offset
#define FIRST_PARTITION_OFFSET 1ll * 1024 * 1024
//If a disk uses MSTORE, then how much should it use
#define MSTORE_PARTITION_SIZE 95ll * 1024 * 1024 * 1024
//If a disk uses timeshift, then how many % of total storage should it use
#define TIMESHIFT_PARTTITION_PERCENT 5
//If a disk uses PVR/TS, then how much should meta partition use
#define META_PARITION_SIZE 1ll * 1024 * 1024 * 1024
//if a disk uses fat32 partition, then how much should it use
#define README_PARTITION_SIZE 1ll * 1024 * 1024

#define PVR_FAT_README_FILE "/tmp/qb_pvr_fat_readme"
#define PVR_FAT_README_CONF_LINE "readme_file=" PVR_FAT_README_FILE "\n"
#define PVR_FAT_README_CONTENT gettext("This device is used as PVR storage")

#define CONFIG_RECLIMIT_NAME "USB_PVR_LIMITS.MAX_COST"
#define CONFIG_RECLIMIT_SPEED_TEST_NAME "USB_PVR_LIMITS.SPEED_TEST"

struct QBPVRLogic_t {
    struct SvObject_ super_;
    int hddRecLimit;
    int configRecLimit;
    AppGlobals appGlobals;
};

int QBPVRLogicGetRecLimitFromUSBPerfTest(QBPVRLogic self, QBUSBPerfTest test)
{
    unsigned int hdCnt = QBUSBPerfTestGetHDCount(test);
    unsigned int sdCnt = QBUSBPerfTestGetSDCount(test);
    bool ts = QBUSBPerfTestHasTS(test);

    // (HD RECORDINGS + TS) *  PVR_COST_HD + (SD RECORDINGS) * PVR_COST_SD
    int res = (hdCnt + (ts ? 1 : 0)) * QBPVRLogicGetChannelCost(self, QBDiskPVRProviderChannelCostClass_HDTV)
            + sdCnt * QBPVRLogicGetChannelCost(self, QBDiskPVRProviderChannelCostClass_SDTV);

    return res;
}

SvLocal void
QBPVRLogicCheckRecording(QBPVRLogic self, QBPVRRecording rec)
{
    /// delete completed recordings shorter than 5 seconds
    if (rec->playable && rec->state != QBPVRRecordingState_active &&
        QBPVRProviderGetRecordingCurrentDuration(self->appGlobals->pvrProvider,
                                                 rec) < 5) {
        QBPVRProviderDeleteRecording(self->appGlobals->pvrProvider, rec);
    }
}

int QBPVRGetOperatorRecordLimit(QBPVRLogic self)
{
    // unset
    if (self->configRecLimit < 0) {
        return self->configRecLimit;
    }
    // recordings limit from config + TS
    return self->configRecLimit + QBPVRLogicGetChannelCost(self, QBDiskPVRProviderChannelCostClass_HDTV);
}

SvLocal void
QBPVRLogicRecordingAdded(SvObject self_,
                         QBPVRRecording rec)
{
    QBPVRLogic self = (QBPVRLogic) self_;
    QBPVRLogicCheckRecording(self, rec);
}

SvLocal void
QBPVRLogicRecordingRemoved(SvObject self_,
                           QBPVRRecording rec)
{
}

SvLocal void
QBPVRLogicRecordingChanged(SvObject self_,
                           QBPVRRecording rec,
                           QBPVRRecording oldRecording)
{
    QBPVRLogic self = (QBPVRLogic) self_;
    QBPVRLogicCheckRecording(self, rec);
}

SvLocal void
QBPVRLogicRecordingRestricted(SvObject self_,
                              QBPVRRecording rec)
{
}

SvLocal void
QBPVRLogicQuotaChanged(SvObject self, QBPVRQuota quota)
{
}

SvLocal void
QBPVRLogicDirectoryAdded(SvObject self, QBPVRDirectory dir)
{
}

SvLocal void
QBPVRLogicConfigChanged(SvObject self_, const char *key, const char *value)
{
    QBPVRLogic self = (QBPVRLogic) self_;

    if (!value) {
        return;
    }

    if (!strcmp(key, CONFIG_RECLIMIT_NAME)) {
        QBConfigGetInteger(CONFIG_RECLIMIT_NAME, &self->configRecLimit);

        int schedManagerCostLimit = self->configRecLimit;
        if (self->hddRecLimit >= 0) {
            int hddLimit = self->hddRecLimit - QBPVRLogicGetChannelCost(self, QBDiskPVRProviderChannelCostClass_HDTV);

            if (hddLimit < schedManagerCostLimit) {
                schedManagerCostLimit = hddLimit;
            }
        }

        QBSchedManagerSetRecCostLimit(schedManagerCostLimit);
    }
}

SvLocal SvType QBPVRLogic_getType(void)
{
    static SvType type = NULL;

    static const struct QBPVRProviderListener_ pvrProviderListenerMethods = {
        .recordingAdded = QBPVRLogicRecordingAdded,
        .recordingChanged = QBPVRLogicRecordingChanged,
        .recordingRemoved = QBPVRLogicRecordingRemoved,
        .recordingRestricted = QBPVRLogicRecordingRestricted,
        .quotaChanged = QBPVRLogicQuotaChanged,
        .directoryAdded = QBPVRLogicDirectoryAdded,
    };

    static const struct QBConfigListener_t configMethods = {
        .changed = QBPVRLogicConfigChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBPVRLogic",
                            sizeof(struct QBPVRLogic_t),
                            SvObject_getType(), &type,
                            QBPVRProviderListener_getInterface(), &pvrProviderListenerMethods,
                            QBConfigListener_getInterface(), &configMethods,
                            NULL);
    }
    return type;
}

QBPVRLogic
QBPVRLogicNew(AppGlobals appGlobals)
{
    QBPVRLogic self = (QBPVRLogic) SvTypeAllocateInstance(QBPVRLogic_getType(), NULL);
    self->appGlobals = appGlobals;
    self->hddRecLimit = -1;
    self->configRecLimit = -1;
    return self;
}

void QBPVRLogicStart(QBPVRLogic self)
{
    if (!self->appGlobals->pvrProvider ||
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type != QBPVRProviderType_disk)
        return;

    /// max:
    /// 2 HD + 1 SD = 32
    /// 1 HD + 2 SD = 31
    ///        3 SD = 30
    QBConfigAddListener((SvObject) self, CONFIG_RECLIMIT_NAME);
    if (QBPVRLogicHasInternalStorage()) {
        SvLogNotice("%s: has builtin hdd, allow for rec limit %i", __func__, 32);
        QBDiskPVRProviderSetRecordingsLimit((QBDiskPVRProvider) self->appGlobals->pvrProvider, 32);
    } else {
        QBConfigGetInteger(CONFIG_RECLIMIT_NAME, &self->configRecLimit);
        SvLogNotice("%s: doesn't have builtin hdd, allow for rec limit %i", __func__,
                (self->configRecLimit < 0) ? 22 : self->configRecLimit);

        QBDiskPVRProviderSetRecordingsLimit((QBDiskPVRProvider) self->appGlobals->pvrProvider,
                                            (self->configRecLimit < 0) ? 22 : self->configRecLimit);
    }

    QBPVRProviderAddListener(self->appGlobals->pvrProvider, (SvObject) self);
}

void QBPVRLogicStop(QBPVRLogic self)
{
    if (!self->appGlobals->pvrProvider ||
        QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type != QBPVRProviderType_disk)
        return;

    QBPVRProviderRemoveListener(self->appGlobals->pvrProvider, (SvObject) self);
    QBConfigRemoveListener((SvObject) self, CONFIG_RECLIMIT_NAME);

}

int QBPVRLogicGetChannelCost(QBPVRLogic self, QBDiskPVRProviderChannelCostClass channelPVRCostClass)
{
    if (channelPVRCostClass == QBDiskPVRProviderChannelCostClass_radio)
        return 10;
    if (channelPVRCostClass == QBDiskPVRProviderChannelCostClass_SDTV)
        return 10;
    if (channelPVRCostClass == QBDiskPVRProviderChannelCostClass_HDTV)
        return 11;

    return QBPVRLogicGetChannelCost(self, QBDiskPVRProviderChannelCostClass_SDTV);
}

bool QBPVRLogicIsSpeedTestEnabled(QBPVRLogic self)
{
    const char *pvrSpeedTest = QBConfigGet(CONFIG_RECLIMIT_SPEED_TEST_NAME);
    return (pvrSpeedTest && !strcmp(pvrSpeedTest, "enabled"));
}

SvLocal bool QBPVRLogicIsPvrReadmeEnabled(QBPVRLogic self)
{
    const char *pvrReadme = QBConfigGet("PVR_README");
    return (pvrReadme && !strcmp(pvrReadme, "enabled"));
}

void QBPVRLogicSetMaxRecordingSessionsLimit(QBPVRLogic self, int maxRecordingSessions)
{
    struct QBPVRLogicSupportedFeatures_ supportedFeatures;

    QBPVRLogicGetSupportedFeatures(self, &supportedFeatures);

    int pvrRecordingSessionsLimit = 0;
    if (!supportedFeatures.timeshift) {
        pvrRecordingSessionsLimit = maxRecordingSessions;
    } else {
        // Reserve one session for timeshift
        pvrRecordingSessionsLimit = maxRecordingSessions - 1;
    }

    if (pvrRecordingSessionsLimit < 0) {
        SvLogError("PvrRecordingSessionsLimit is negative. Set up to 0 - pvr recording will be not allowed");
        pvrRecordingSessionsLimit = 0;
    }

    QBDiskPVRProviderSetRecordingSessionsLimit((QBDiskPVRProvider) self->appGlobals->pvrProvider,
                                               pvrRecordingSessionsLimit);
}

void QBPVRLogicSetRecLimit(QBPVRLogic self, int reclimit)
{
    if (!QBPVRLogicIsSpeedTestEnabled(self) || reclimit < 0)
        return;

    self->hddRecLimit = reclimit;
    // One HD channel reserved for TimeShift

    int costLimit = self->hddRecLimit - QBPVRLogicGetChannelCost(self, QBDiskPVRProviderChannelCostClass_HDTV);

    if (self->configRecLimit >= 0 && self->configRecLimit < costLimit) {
        costLimit = self->configRecLimit;
    }

    QBDiskPVRProviderSetRecordingsLimit((QBDiskPVRProvider) self->appGlobals->pvrProvider,
                                        costLimit);
}

bool QBPVRLogicIsDiskSizeEnoughForPVR(QBPVRLogic self, QBDiskInfo diskInfo)
{
    return diskInfo->size >= MAX_TIMESHIFT_DISK_SIZE;
}

int QBPVRLogicCreatePVRFormatConf(QBPVRLogic self, SvString filename, SvString diskID)
{
    //use default
    QBDiskStatus diskStatus = QBPVRMounterGetDiskStatus(self->appGlobals->PVRMounter, diskID);
    if (!diskStatus || !diskStatus->disk)
        return -1;
    QBDiskInfo disk = diskStatus->disk;
    char *buf = NULL;
    int bufLen = 0;

    int64_t diskSize = disk->size - FIRST_PARTITION_OFFSET;

    if (diskSize < MAX_TIMESHIFT_DISK_SIZE) {
        int64_t metaSize = 10ll * 1024 * 1024;
        int64_t timeshiftSize = diskSize - metaSize - README_PARTITION_SIZE;
        if (timeshiftSize < 0) {
            SvLogError("Not enough disk space");
            return -1;
        }

        bufLen = asprintf(&buf, "version=1\ntimeshift=%lld\nmeta=%lld\nreclimit=%d\n%s",
                          (long long int) timeshiftSize / (1024 * 1024), (long long int) metaSize / (1024 * 1024), self->hddRecLimit, QBPVRLogicIsPvrReadmeEnabled(self) ? PVR_FAT_README_CONF_LINE : "");
    } else if (diskSize >= MIN_MSTORE_DISK_SIZE && self->appGlobals->pvodStorage &&
               QBPVODStorageServiceIsDiskAllowed(self->appGlobals->pvodStorage, disk) &&
               QBPVODStorageServiceIsStorageSupported(self->appGlobals->pvodStorage, SVSTRING("MStoreVoD"))) {
        int64_t metaSize = META_PARITION_SIZE;
        int64_t mstoreSize = MSTORE_PARTITION_SIZE;
        int64_t timeshiftSize = diskSize / 100 * TIMESHIFT_PARTTITION_PERCENT;
        int64_t pvrSize = diskSize - mstoreSize - metaSize - timeshiftSize - README_PARTITION_SIZE;

        bufLen = asprintf(&buf, "version=1\nfiles=%lli\nmeta=%lli\ntimeshift=%lli\n"
                          "label=MStoreVoD:type=ext3:size=%lli\nreclimit=%d\n%s",
                          (long long int) pvrSize / (1024 * 1024), (long long int) metaSize / (1024 * 1024),
                          (long long int) timeshiftSize / (1024 * 1024), (long long int) mstoreSize / (1024 * 1024), self->hddRecLimit, QBPVRLogicIsPvrReadmeEnabled(self) ? PVR_FAT_README_CONF_LINE : "");
    } else {
        bufLen = asprintf(&buf, "version=1\nreclimit=%d\n%s", self->hddRecLimit, QBPVRLogicIsPvrReadmeEnabled(self) ? PVR_FAT_README_CONF_LINE : "");
    }

    if (buf) {
        FILE *f = fopen(SvStringCString(filename), "w");
        if (!f) {
            SvLogError("%s :: fopen call failed : %s", __func__, strerror(errno));
            free(buf);
            return -1;
        }
        fwrite(buf, bufLen, 1, f);
        free(buf);
        fclose(f);
    }

    if (QBPVRLogicIsPvrReadmeEnabled(self)) {
        SvData readmeContent = SvDataCreateWithBytesAndLength(PVR_FAT_README_CONTENT, strlen(PVR_FAT_README_CONTENT), NULL);
        SvDataWriteToFile(readmeContent, PVR_FAT_README_FILE, true, NULL);
        SVRELEASE(readmeContent);
    }

    return 0;
}

bool QBPVRLogicHasInternalStorage(void)
{
    const char* recordFsDataRoot = env_recordfs_data_root();
    if (!recordFsDataRoot)
        recordFsDataRoot = "/tmp/qb_internal_storage";
    return QBRecordFSUtilsIsDeviceInternal(recordFsDataRoot) == 1;
}

int QBPVRLogicGetSDVideoAverageBitrate(QBPVRLogic self)
{
    return 3480 * 1024; // 3,4 Mbit/s
}

int QBPVRLogicGetHDVideoAverageBitrate(QBPVRLogic self)
{
    return 11 * 1024 * 1024; // 11 Mbit/s
}

bool QBPVRLogicIsDiskPVRCompatible(QBDiskInfo diskInfo)
{
    return true;
}

void QBPVRLogicGetSupportedFeatures(QBPVRLogic self, struct QBPVRLogicSupportedFeatures_ *supportedFeatures)
{
    memset(supportedFeatures, 0, sizeof(struct QBPVRLogicSupportedFeatures_));
    supportedFeatures->pvr = true;
    supportedFeatures->timeshift = true;
}
