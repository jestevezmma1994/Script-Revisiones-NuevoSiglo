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

#include "QBDiskPVRDirectoryManager.h"

#include "QBDiskPVRProviderInternal.h"
#include "QBDiskPVRDirectorySerializer.h"
#include "QBDiskPVRRecording.h"

#include <main.h>
#include <Services/core/JSONserializer.h>
#include <QBConfig.h>
#include <QBPVRDirectory.h>
#include <QBPVRDirectoryInternal.h>
#include <QBPVRRecording.h>
#include <QBPVRTypes.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBDataModel3/QBSortedList2.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvErrnoDomain.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <stdlib.h>
#include <errno.h>

#define DIRECTORIES_FILENAME "/etc/vod/groupRec"
#define MAX_KEYWORDS 10
#define MAX_NORMAL 20
#define MAX_DIRECTORIES 40

#define DAILY_DAYS_MASK 0x7F
#define WEEKENDS_DAYS_MASK 0x41
#define WEEKDAYS_DAYS_MASK 0x3e

struct QBDiskPVRDirectoryManager_ {
    struct SvObject_ super_;            ///< super type

    QBDiskPVRProvider provider;
    QBJSONSerializer jsonSerializer;

    SvHashTable idToDirectory;
    SvArray dirs;

    QBContentCategory parent;

    unsigned int keywordDirectories;
    unsigned int normalDirectories;

    unsigned int maxNormalDirsCnt;
    unsigned int maxKeywordsDirsCnt;
    unsigned int maxDirsCnt;

    bool addSearchToDirectories;
    QBPVRSearchProviderTest searchProviderTest;
};

SvLocal
void QBDiskPVRDirectoryManagerDestroy(void *self_)
{
    QBDiskPVRDirectoryManager self = (QBDiskPVRDirectoryManager) self_;

    SVTESTRELEASE(self->idToDirectory);
    SVTESTRELEASE(self->dirs);
    SVTESTRELEASE(self->parent);
    SVTESTRELEASE(self->jsonSerializer);
}

SvLocal SvType
QBDiskPVRDirectoryManager_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiskPVRDirectoryManagerDestroy,
    };

    if (!type) {
        SvTypeCreateManaged("QBDiskPVRDirectoryManager",
                            sizeof(struct QBDiskPVRDirectoryManager_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBDiskPVRDirectoryManagerAddDirectory(QBDiskPVRDirectoryManager self,
                                      QBDiskPVRDirectory dir)
{
    QBPVRDirectory dir_ = (QBPVRDirectory) dir;

    if (dir_->type == QBPVRDirectoryType_keyword) {
        ++self->keywordDirectories;
    } else if (dir_->type == QBPVRDirectoryType_normal) {
        ++self->normalDirectories;
    }

    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource(self->parent);
    QBSortedList2Insert(list, (SvObject) dir);

    SvHashTableInsert(self->idToDirectory,
                      (SvObject) QBContentCategoryGetId((QBContentCategory) dir),
                      (SvObject) dir);

    QBDiskPVRProviderNotifyDirectoryAdded(self->provider,
                                          (QBPVRDirectory) dir);
}

QBDiskPVRDirectoryManager
QBDiskPVRDirectoryManagerCreate(QBDiskPVRProvider provider,
                                QBContentCategory parent,
                                bool addSearchToDirectories,
                                QBPVRSearchProviderTest searchProviderTest,
                                QBDiskPVRProviderQuantitativeLimits limits,
                                SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!provider) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL provider passed");
        goto err1;
    }

    if (!parent) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL parent passed");
        goto err1;
    }

    QBDiskPVRDirectoryManager self = (QBDiskPVRDirectoryManager)
            SvTypeAllocateInstance(QBDiskPVRDirectoryManager_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error,
                                           "can't allocate QBDiskPVRDirectoryManager");
        goto err1;
    }

    // provider isn't retained cause directory manager is a internal service of
    // disk pvr provider and it's owned only by it
    self->provider = provider;

    self->parent = SVRETAIN(parent);
    self->addSearchToDirectories = addSearchToDirectories;
    self->searchProviderTest = searchProviderTest;

    int maxNormalDirsCnt = limits.maxNormalDirsCnt;
    int maxKeywordsDirsCnt = limits.maxKeywordsDirsCnt;
    int maxDirsCnt = limits.maxDirsCnt;
    self->maxNormalDirsCnt = maxNormalDirsCnt < 0 ? MAX_NORMAL : maxNormalDirsCnt;
    self->maxKeywordsDirsCnt = maxNormalDirsCnt < 0 ? MAX_KEYWORDS : maxKeywordsDirsCnt;
    self->maxDirsCnt = maxDirsCnt < 0 ? MAX_DIRECTORIES : maxDirsCnt;

    if (!(self->idToDirectory = SvHashTableCreate(157, &error))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "SvHashTableCreate() failed");
        goto err2;
    }

    QBDiskPVRDirectorySerializer serializer =
        QBDiskPVRDirectorySerializerCreate(self->parent,
                                           self->addSearchToDirectories,
                                           self->searchProviderTest,
                                           &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBDiskPVRDirectorySerializerCreate() failed");
        goto err2;
    }

    SvJSONRegisterClassHelper(SVSTRING("QBDiskPVRDirectory"), (SvObject) serializer, NULL);

    // for migration purpose only
    SvJSONRegisterClassHelper(SVSTRING("QBPVRDirectory"), (SvObject) serializer, NULL);
    SVRELEASE(serializer);

    SvArray dirs = (SvArray) SvJSONParseFile(DIRECTORIES_FILENAME, true, &error);
    if (error) {
        if (SvErrorInfoEquals(error, SvErrnoDomain, ENOENT)) {
            SvErrorInfoDestroy(error);
            SvLogWarning("%s(): file '%s' does not exist'", __func__, DIRECTORIES_FILENAME);
            error = NULL;
        } else {
            // unexpected type of error
            goto err2;
        }
    }

    if (dirs && SvObjectIsInstanceOf((SvObject) dirs, SvArray_getType())) {
        SvIterator iter = SvArrayIterator(dirs);
        QBDiskPVRDirectory dir;
        while ((dir = (QBDiskPVRDirectory) SvIteratorGetNext(&iter))) {
            QBDiskPVRDirectoryManagerAddDirectory(self, dir);
        }
    } else {
        SVTESTRELEASE(dirs);
        dirs = SvArrayCreate(NULL);
    }
    self->dirs = dirs;

    return self;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBDiskPVRDirectoryManagerStart(QBDiskPVRDirectoryManager self,
                               SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    if (!registry) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetInstance() failed");
        goto out;
    }

    self->jsonSerializer = SVTESTRETAIN(QBServiceRegistryGetService(registry, SVSTRING("QBJSONSerializer")));
    if (!self->jsonSerializer) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"QBJSONSerializer\") failed");
        goto out;
    }

    SvIterator it = SvHashTableGetValuesIterator(self->idToDirectory);
    QBDiskPVRDirectory dir = NULL;

    while ((dir = (QBDiskPVRDirectory) SvIteratorGetNext(&it))) {
        QBDiskPVRDirectoryStart(dir, &error);
        if (error) {
            error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                               "QBDiskPVRDirectoryStart() failed");
            goto out;
        }
    }

out:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBDiskPVRDirectoryManagerStop(QBDiskPVRDirectoryManager self,
                              SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SVTESTRELEASE(self->jsonSerializer);
    self->jsonSerializer = NULL;

    SvIterator it = SvHashTableGetValuesIterator(self->idToDirectory);
    QBDiskPVRDirectory dir = NULL;
    while ((dir = (QBDiskPVRDirectory) SvIteratorGetNext(&it))) {
        QBDiskPVRDirectoryStop(dir, &error);
        if (error) {
            error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                               "QBDiskPVRDirectoryStart() failed");
            goto out;
        }
    }

out:
    SvErrorInfoPropagate(error, errorOut);
}

SvArray
QBDiskPVRDirectoryManagerGetDirectoriesList(QBDiskPVRDirectoryManager self)
{
    return self->dirs;
}

SvLocal SvString
QBDiskPVRDirectoryManagerGenerateDirectoryId(QBDiskPVRDirectoryManager self)
{
    SvString id = SvStringCreateWithFormat("dirId-%lx", random());
    SvIterator it = SvArrayIterator(self->dirs);
    QBDiskPVRDirectory dir = NULL;
    while ((dir = (QBDiskPVRDirectory) SvIteratorGetNext(&it))) {
        if (SvObjectEquals((SvObject) QBContentCategoryGetId((QBContentCategory) dir), (SvObject) id)) {
            SVRELEASE(id);
            return QBDiskPVRDirectoryManagerGenerateDirectoryId(self);
        }
    }
    return id;
}

SvLocal int64_t
QBDiskPVRDirectoryManagerGetDefaultQuota(void)
{
    const char *quota = QBConfigGet("PVR_QUOTA_DEFAULT");
    return quota ? atoll(quota) * 1024 * 1024 * 1024 : 0;
}

SvLocal void
QBDiskPVRDirectoryManagerUpdateFile(QBDiskPVRDirectoryManager self)
{
    QBJSONSerializerAddJob(self->jsonSerializer, (SvObject) self->dirs, SVSTRING(DIRECTORIES_FILENAME));
}

QBDiskPVRDirectory
QBDiskPVRDirectoryManagerCreateDirectoryFromRecordingSchedParams(QBDiskPVRDirectoryManager self,
                                                                 QBPVRRecordingSchedParams params,
                                                                 SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvString id = QBDiskPVRDirectoryManagerGenerateDirectoryId(self);
    if (!id) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryManagerGenerateDirectoryId() failed");
        goto err1;
    }

    struct QBDiskPVRDirectoryInitParams_ initParams = {
        .super_ = {
            .id = id,
            .name = params->name,
            .parent = self->parent,
            .quota = QBDiskPVRDirectoryManagerGetDefaultQuota(),
            .channelId = params->channel ? SvValueGetString(SvTVChannelGetID(params->channel)) : NULL,
            .channelName = params->channel ? params->channel->name : NULL,
            .addSearch = self->addSearchToDirectories,
            .searchProviderTest = self->searchProviderTest,
        },
        .days = 0,
    };

    switch (params->type) {
        case QBPVRRecordingType_keyword:
            if (!initParams.super_.name)
                initParams.super_.name = params->keyword;
            initParams.super_.type = QBPVRDirectoryType_keyword;
            initParams.super_.keyword = params->keyword;
            break;
        case QBPVRRecordingType_series:
            initParams.super_.type = QBPVRDirectoryType_series;
            initParams.super_.seriesID = params->seriesID;
            break;
        case QBPVRRecordingType_repeated:
            initParams.super_.type = QBPVRDirectoryType_repeated;
            struct tm start;
            SvTimeBreakDown(params->repeatedInfo.start, false, &start);
            initParams.super_.repeatedInfo.start = SvTimeGetSeconds(params->repeatedInfo.start);
            initParams.super_.repeatedInfo.duration = params->repeatedInfo.duration;
            initParams.super_.repeatedInfo.mode = params->repeatedInfo.mode;
            switch (params->repeatedInfo.mode) {
                case QBPVRRepeatedRecordings_Daily:
                    initParams.days = DAILY_DAYS_MASK;
                    break;
                case QBPVRRepeatedRecordings_Weekly:
                    initParams.days = 1 << start.tm_wday;
                    break;
                case QBPVRRepeatedRecordings_Weekends:
                    initParams.days = WEEKENDS_DAYS_MASK;
                    break;
                case QBPVRRepeatedRecordings_Weekdays:
                    initParams.days = WEEKDAYS_DAYS_MASK;
                    break;
                default:
                    break;
            }
            break;
        default:
            initParams.super_.type = QBPVRDirectoryType_normal;
            break;
    }

    QBDiskPVRDirectory dir = QBDiskPVRDirectoryCreate(&initParams,
                                                      &error);
    SVRELEASE(id);
    if (!dir) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryCreateFromRecordingSchedParams() failed");
        goto err1;
    }

    SvArrayAddObject(self->dirs, (SvObject) dir);
    QBDiskPVRDirectoryManagerAddDirectory(self, dir);

    QBDiskPVRDirectoryStart(dir, NULL);
    QBDiskPVRDirectoryManagerUpdateFile(self);
    return dir;
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

QBDiskPVRDirectory
QBDiskPVRDirectoryManagerCreateDirectoryFromParams(QBDiskPVRDirectoryManager self,
                                                   QBPVRDirectoryCreateParams params,
                                                   SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvString id = QBDiskPVRDirectoryManagerGenerateDirectoryId(self);
    if (!id) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryManagerGenerateDirectoryId() failed");
        goto err1;
    }

    struct QBDiskPVRDirectoryInitParams_ initParams = {
        .super_ = {
            .id = id,
            .name = params->name,
            .parent = self->parent,
            .type = QBPVRDirectoryType_normal,
            .addSearch = self->addSearchToDirectories,
            .searchProviderTest = self->searchProviderTest,
        },
        .days = 0,
    };
    QBDiskPVRDirectory dir = QBDiskPVRDirectoryCreate(&initParams, &error);
    SVRELEASE(id);
    if (!dir) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBDiskPVRDirectoryCreate() failed");
        goto err1;
    }

    SvArrayAddObject(self->dirs, (SvObject) dir);
    QBDiskPVRDirectoryManagerAddDirectory(self, dir);

    QBDiskPVRDirectoryStart(dir, NULL);
    QBDiskPVRDirectoryManagerUpdateFile(self);
    return dir;
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBDiskPVRDirectoryManagerUpdateDirectory(QBDiskPVRDirectoryManager self,
                                         QBDiskPVRDirectory dir,
                                         QBPVRDirectoryUpdateParams params)
{
    QBDiskPVRDirectoryUpdate(dir, params);

    QBDiskPVRDirectoryManagerUpdateFile(self);
    QBPVRDirectoryNotifyChanged((QBPVRDirectory) dir);
}

void
QBDiskPVRDirectoryManagerStopDirectory(QBDiskPVRDirectoryManager self,
                                       QBDiskPVRDirectory dir)
{
    if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_keyword) {
        --self->keywordDirectories;
    } else if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_normal) {
        --self->normalDirectories;
    }
    QBDiskPVRDirectoryStopRecording(dir);
    if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_keyword) {
        ++self->keywordDirectories;
    } else if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_normal) {
        ++self->normalDirectories;
    }

    QBDiskPVRDirectoryManagerUpdateFile(self);
    QBPVRDirectoryNotifyChanged((QBPVRDirectory) dir);
}

void
QBDiskPVRDirectoryManagerDeleteAllFromDirectory(QBDiskPVRDirectoryManager self,
                                                QBDiskPVRDirectory dir,
                                                QBPVRDirectoryDeleteParams params)
{
    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource((QBContentCategory) dir);
    SvIterator iter = QBSortedList2Iterator(list);

    SvArray recTemp = SvArrayCreate(NULL);
    QBDiskPVRRecording rec = NULL;
    while ((rec = (QBDiskPVRRecording) SvIteratorGetNext(&iter))) {
        SvArrayAddObject(recTemp, (SvObject) rec);
    }

    iter = SvArrayIterator(recTemp);
    while ((rec = (QBDiskPVRRecording) SvIteratorGetNext(&iter))) {
        if (params->keepContents || ((QBPVRRecording) rec)->lock) {
            QBDiskPVRRecordingSetGroupId(rec, NULL);
        } else {
            QBDiskPVRRecordingDelete(rec);
        }
    }
    SVRELEASE(recTemp);
}

void
QBDiskPVRDirectoryManagerDeleteDirectory(QBDiskPVRDirectoryManager self,
                                         QBDiskPVRDirectory dir,
                                         QBPVRDirectoryDeleteParams params)
{
    if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_keyword) {
        --self->keywordDirectories;
    } else if (((QBPVRDirectory) dir)->type == QBPVRDirectoryType_normal) {
        --self->normalDirectories;
    }
    QBDiskPVRDirectoryStop(dir, NULL);
    QBDiskPVRDirectoryManagerDeleteAllFromDirectory(self, dir, params);

    SvArrayRemoveObject(self->dirs, (SvObject) dir);
    SvHashTableRemove(self->idToDirectory,
                      (SvObject) QBContentCategoryGetId((QBContentCategory) dir));
    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource(self->parent);
    QBSortedList2Remove(list, (SvObject) dir);

    QBDiskPVRDirectoryManagerUpdateFile(self);
}

bool
QBDiskPVRDirectoryManagerIsDirectoryLimitExceeded(QBDiskPVRDirectoryManager self, QBPVRDirectoryType type)
{
    if (type == QBPVRDirectoryType_keyword && self->keywordDirectories >= self->maxKeywordsDirsCnt)
        return true;
    if (type == QBPVRDirectoryType_normal && self->normalDirectories >= self->maxNormalDirsCnt)
        return true;

    if (SvArrayCount(self->dirs) >= self->maxDirsCnt)
        return true;

    return false;
}

int
QBDiskPVRDirectoryManagerAddRecording(QBDiskPVRDirectoryManager self, QBDiskPVRRecording rec)
{
    QBDiskPVRDirectory dir = (QBDiskPVRDirectory) SvHashTableFind(self->idToDirectory, (SvObject) QBDiskPVRRecordingGetGroupId(rec));
    // directories are loaded from stb storage and recordings from disk
    // because of that directory might be deleted but recordings can still indicate
    // that they belong to that category
    if (!dir) {
        SvLogError("%s(): didn't find directory with groupId %s, removing groupId from recording",
                   __func__, SvStringCString(QBDiskPVRRecordingGetGroupId(rec)));
        QBDiskPVRRecordingSetGroupId(rec, NULL);
        return -1;
    }

    if (!((QBPVRRecording) rec)->playable)
        return -1;

    QBDiskPVRDirectoryAddRecording(dir, rec);

    return 0;
}

int
QBDiskPVRDirectoryManagerRemoveRecording(QBDiskPVRDirectoryManager self, QBDiskPVRRecording rec)
{
    if (!((QBPVRRecording) rec)->directory) {
        return -1;
    }

    QBDiskPVRDirectory dir = (QBDiskPVRDirectory) SvHashTableFind(self->idToDirectory, (SvObject) QBDiskPVRRecordingGetGroupId(rec));
    if (!dir) {
        SvLogError("%s(): didn't find directory with groupId %s", __func__, SvStringCString(QBDiskPVRRecordingGetGroupId(rec)));
        return -1;
    }

    QBDiskPVRDirectoryRemoveRecording(dir, rec);

    return 0;
}
