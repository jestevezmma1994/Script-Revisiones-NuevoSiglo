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

#include "QBDiskPVRDirectory.h"

#include "QBDiskPVRProviderInternal.h"
#include <Utils/recordingUtils.h>
#include <Services/epgWatcher.h>
#include <Services/langPreferences.h>
#include <QBConfig.h>
#include <QBSchedDesc.h>
#include <QBSchedManager.h>
#include <QBPVRRecording.h>
#include <QBDataModel3/QBSortedList2.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <fibers/c/fibers.h>
#include <SvDataBucket2/SvDBObject.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>

#include <stdlib.h>

#define MAX_KEYWORDS 10
#define MAX_DIRECTORIES 20
#define MAX_SIMULTANEOUS_KEYWORD_RECORDINGS 5
#define MAX_KEYWORD_AHEAD_SCHEDULING (60*10)
#define MAX_SUM_KEYWORD_DIRECTORY_RECORDINGS 40

#define REPEATED_RECORDINGS_FIBER_TIMER_PERIOD 15

#define DIRECORY_TYPE_KEY "type"
#define DIRECORY_ID_KEY "id"
#define DIRECORY_QUOTA_KEY "quota"
#define DIRECORY_NAME_KEY "name"
#define DIRECORY_CHANNEL_ID_KEY "channelId"
#define DIRECORY_BACKUP_CHANNEL_NAME_KEY "backupChannelName"
#define DIRECORY_KEYWORD_KEY "keyword"
#define DIRECORY_REPEAT_MODE_KEY "repeatMode"
#define DIRECORY_DAYS_KEY "days"
#define DIRECORY_START_KEY "start"
#define DIRECORY_DURATION_KEY "duration"

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBDiskPVRDirectoryLogLevel", "")
#define log_debug(fmt, ...)  if (env_log_level() >= 1) SvLogNotice(COLBEG() "%s:%d " fmt  COLEND_COL(cyan), __func__,__LINE__,##__VA_ARGS__);

struct QBDiskPVRDirectory_ {
    struct QBPVRDirectory_  super_;           ///< super type

    bool started;
    SvEPGManager epgManager;
    QBLangPreferences langPreferences;

    union {
        struct {
            uint32_t days;              ///< days bitmask
            SvFiber fiber;
            SvFiberTimer timer;
        } repeated;
        struct {
            SvHashTable blacklistCurrent, blacklistNext;
        } keyword;
    };
};

SvLocal void
QBDiskPVRDirectoryDestroy(void* self_)
{
    QBDiskPVRDirectory self = (QBDiskPVRDirectory) self_;
    if (self->started)
        QBDiskPVRDirectoryStop(self, NULL);

    SVTESTRELEASE(self->epgManager);
    SVTESTRELEASE(self->langPreferences);
}

SvLocal void
QBDiskPVRDirectoryUpdateBlacklist(QBDiskPVRDirectory self, SvString channelId, SvEPGEvent current, SvEPGEvent next)
{
    SvValue blackCurrent = (SvValue) SvHashTableFind(self->keyword.blacklistCurrent, (SvObject) channelId);
    SvValue blackNext = (SvValue) SvHashTableFind(self->keyword.blacklistNext, (SvObject) channelId);

    if (!blackNext && !blackCurrent)
        return;

    SvValue currentId = NULL;
    if (blackCurrent) {
        currentId = current ? SvEPGEventCreateID(current) : NULL;
        if (!SvObjectEquals((SvObject) blackCurrent, (SvObject) currentId)) {
            SvHashTableRemove(self->keyword.blacklistCurrent, (SvObject) channelId);
            log_debug("Removing current from blacklist");
        }
    }

    SvValue nextId = NULL;
    if (blackNext) {
        nextId = next ? SvEPGEventCreateID(next) : NULL;
        if (!SvObjectEquals((SvObject) blackNext, (SvObject) nextId)) {
            if (SvObjectEquals((SvObject) blackNext, (SvObject) currentId)) {
                SvHashTableInsert(self->keyword.blacklistCurrent, (SvObject) channelId, (SvObject) blackNext);
                log_debug("Next from blacklist moved to current");
            }
        } else {
            log_debug("Removing next from blacklist");
        }
        SvHashTableRemove(self->keyword.blacklistNext, (SvObject) channelId);
    }

    SVTESTRELEASE(currentId);
    SVTESTRELEASE(nextId);
}

bool
QBDiskPVRDirectoryMatchRepeated(QBDiskPVRDirectory self,
                                SvEPGEvent event)
{
    if (self->super_.type != QBPVRDirectoryType_repeated)
        return false;

    if (self->super_.channelId && !SvObjectEquals((SvObject) self->super_.channelId, (SvObject) SvValueGetString(event->channelID)))
        return false;

    if (event->startTime < self->super_.repeatedInfo.start)
        return false;

    bool fullMatch = true;

    time_t ts = event->startTime - (event->startTime % (24 * 60 * 60)) + (self->super_.repeatedInfo.start % (24 * 60 * 60));
    time_t te = ts + self->super_.repeatedInfo.duration;

    struct tm start;
    SvTimeBreakDown(SvTimeConstruct(event->startTime, 0), false, &start);
    start.tm_sec = self->super_.repeatedInfo.start % 60;
    start.tm_min = (self->super_.repeatedInfo.start / 60) % 60;
    start.tm_hour = (self->super_.repeatedInfo.start / 3600) % 24;
    mktime(&start); // fill m_wday
    if ((1 << start.tm_wday) & self->repeated.days) {
        if (fullMatch) {
            if (ts <= event->startTime && event->endTime <= te)
                return true;
        } else {
            if (ts < event->endTime && event->startTime < te)
                return true;
        }
    }
    return false;
}

bool
QBDiskPVRDirectoryMatchKeyword(QBDiskPVRDirectory self, SvEPGEvent event)
{
    if (!event)
        return false;

    if (self->super_.type != QBPVRDirectoryType_keyword)
        return false;

    if (self->super_.channelId && !SvObjectEquals((SvObject) self->super_.channelId, (SvObject) SvValueGetString(event->channelID)))
        return false;

    //FIXME When new language is selected and the current event matches keyword in new language
    //the current event won't be recorded
    SvEPGEventDesc desc = QBLangPreferencesGetDescFromEvent(self->langPreferences,
                                                            event);

    if (!desc || !desc->title)
        return false;

    return (strcasestr(SvStringCString(desc->title), SvStringCString(self->super_.keyword)) != NULL);
}

SvLocal bool
QBDiskPVRDirectoryIsSimultaneousKeywordRecordingsLimitExceeded(QBDiskPVRDirectory self)
{
    SvArray results = NULL;
    time_t local = SvTimeNow();
    QBSchedManagerFind(self->super_.channelId, SvLocalTimeFromUTC(local), SvLocalTimeFromUTC(local + MAX_KEYWORD_AHEAD_SCHEDULING), &results, false);
    if (!results)
        return false;

    int count = 0;

    SvIterator it = SvArrayIterator(results);
    QBSchedDesc *desc = NULL;
    while ((desc = (QBSchedDesc *) SvIteratorGetNext(&it))) {
        SvString id = NULL;
        SvContentMetaDataGetStringProperty(desc->params.meta, SVSTRING(REC_META_KEY__GROUP_ID), &id);
        if(id)
            count++;
    }

    SVRELEASE(results);
    return count > MAX_SIMULTANEOUS_KEYWORD_RECORDINGS;
}

SvLocal int64_t
QBDiskPVRDirectoryGetSize(QBDiskPVRDirectory self)
{
    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource((QBContentCategory) self);

    SvIterator it = QBSortedList2Iterator(list);
    QBDiskPVRRecording rec = NULL;
    int64_t totalSize = 0;
    while ((rec = (QBDiskPVRRecording) SvIteratorGetNext(&it))) {
        int64_t fileSize = QBDiskPVRRecordingGetFileSize(rec);
        if (fileSize <= 0)
            continue;
        totalSize += fileSize;
    }

    return totalSize;
}

SvLocal bool
QBDiskPVRDirectoryIsQuotaCleanupEnabled(void)
{
    const char *autoclean = QBConfigGet("PVR_QUOTA_AUTOCLEAN");
    return autoclean && !strcmp(autoclean, "1");
}

#define GB (1024*1024*1024)

SvLocal bool
QBDiskPVRDirectoryIsQuotaExceeded(QBDiskPVRDirectory self)
{
    if (!self->super_.quotaLimit) {
        return false;
    }

    int64_t totalSize = QBDiskPVRDirectoryGetSize(self);

    log_debug("%s totalSize = %lf and quota = %lli",
              SvStringCString(QBContentCategoryGetName((QBContentCategory) self)),
              ((double) totalSize) / GB,
              (long long int) self->super_.quotaLimit);

    if (self->super_.quotaLimit > (totalSize / GB)) {
        return false;
    }

    return true;
}

SvLocal bool
QBDiskPVRDirectoryDeleteOldestRecordings(QBDiskPVRDirectory self)
{
    int64_t totalSize = QBDiskPVRDirectoryGetSize(self);
    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource((QBContentCategory) self);
    SvIterator it = QBSortedList2Iterator(list);

    QBPVRRecording rec = NULL;
    while ((rec = (QBPVRRecording) SvIteratorGetNext(&it))) {
        if (!QBDiskPVRRecordingHasFile((QBDiskPVRRecording) rec) || rec->lock) {
            continue;
        }
        log_debug("Remove %s", SvStringCString(rec->id));

        int64_t fileSize = QBDiskPVRRecordingGetFileSize((QBDiskPVRRecording) rec);
        if (fileSize < 0)
            continue;

        totalSize -= fileSize;
        QBDiskPVRRecordingDelete((QBDiskPVRRecording) rec);

        log_debug("%s totalSize = %lf after decreasing by %lf", SvStringCString(QBContentCategoryGetName((QBContentCategory) self)),
                  ((double) totalSize) / GB, ((double) fileSize) / GB);
        if (self->super_.quotaLimit > totalSize / GB) {
            return true;
        }
    }

    return false;
}

SvLocal void
QBDiskPVRDirectoryAttemptQuotaCleanup(QBDiskPVRDirectory self)
{
    if (!QBDiskPVRDirectoryIsQuotaExceeded(self))
        return;
    if (QBDiskPVRDirectoryIsQuotaCleanupEnabled())
        QBDiskPVRDirectoryDeleteOldestRecordings(self);
}

SvLocal void
QBDiskPVRDirectoryTrySchedulingEvent(QBDiskPVRDirectory self, SvTVChannel channel, SvEPGEvent event, SvHashTable blacklist)
{
    if (SvHashTableFind(blacklist, (SvObject) SvValueGetString((SvTVChannelGetID(channel))))) {
        return;
    }

    if (!event || event->startTime - SvTimeNow() > MAX_KEYWORD_AHEAD_SCHEDULING) {
        return;
    }

    if (!QBDiskPVRDirectoryMatchKeyword(self, event)) {
        return;
    }

    log_debug("scheduling recording for event %p", event);
    SvArray results = NULL;
    QBSchedManagerFind(self->super_.channelId, SvLocalTimeFromUTC(event->startTime), SvLocalTimeFromUTC(event->endTime), &results, true);
    if (results) {
        SvIterator it = SvArrayIterator(results);
        QBSchedDesc *desc = NULL;
        while ((desc = (QBSchedDesc *) SvIteratorGetNext(&it))) {
            if (SvEPGEventHasEqualID(event, desc->params.event)) {
                log_debug("event already scheduled!");
                SVRELEASE(results);
                return;
            }
        }
        SVRELEASE(results);
    }

    QBSchedParams params;
    QBSchedParamsInit(&params);
    params.channelId = SVRETAIN(SvValueGetString(SvTVChannelGetID(channel)));
    params.priority = QBDiskPVRPriority_keyword;
    params.type = QBSchedType_record;
    params.event = SVRETAIN(event);

    params.costType = (int) QBDiskPVRProviderGetPVRCostClassFromChannel(channel);

    const char *pvrStartMarginStr = QBConfigGet("PVRSTARTMARGIN");
    int pvrStartMargin = pvrStartMarginStr ? atoi(pvrStartMarginStr)*60 : 0;
    const char *pvrEndMarginStr = QBConfigGet("PVRENDMARGIN");
    int pvrEndMargin = pvrEndMarginStr ? atoi(pvrEndMarginStr)*60 : 0;
    params.startMargin = pvrStartMargin;
    params.stopMargin = pvrEndMargin;

    SvContentMetaDataSetStringProperty(params.meta, SVSTRING(REC_META_KEY__GROUP_ID), QBContentCategoryGetId((QBContentCategory) self));

    QBSchedManagerAdd(&params, NULL);
    QBSchedParamsDestroy(&params);
}

SvLocal void
QBDiskPVRDirectoryDeleteEventFromScheduling(QBDiskPVRDirectory self, SvEPGEvent event)
{
    if (!QBDiskPVRDirectoryMatchKeyword(self, event)) {
        return;
    }

    SvArray results = NULL;

    QBSchedManagerFind(self->super_.channelId, SvLocalTimeFromUTC(event->startTime), SvLocalTimeFromUTC(event->endTime), &results, true);

    if (!results) {
        return;
    }

    SvIterator it = SvArrayIterator(results);
    QBSchedDesc *desc = NULL;

    while ((desc = (QBSchedDesc *) SvIteratorGetNext(&it))) {
        if (SvEPGEventHasEqualID(event, desc->params.event)) {
            QBSchedManagerDeleteRecording(desc);
            break;
        }
    }

    SVRELEASE(results);
}

SvLocal void
QBDiskPVRDirectoryEventsUpdate(QBDiskPVRDirectory self, SvTVChannel channel, SvEPGEvent current, SvEPGEvent next)
{
    if (self->super_.channelId && !SvObjectEquals((SvObject) self->super_.channelId, (SvObject) SvValueGetString(SvTVChannelGetID(channel))))
        return;

    if (QBDiskPVRDirectoryIsSimultaneousKeywordRecordingsLimitExceeded(self))
        return;

    QBDiskPVRDirectoryAttemptQuotaCleanup(self);

    if (QBDiskPVRDirectoryIsQuotaExceeded(self)) {
        QBDiskPVRDirectoryDeleteEventFromScheduling(self, next);
        return;
    }

    QBDiskPVRDirectoryTrySchedulingEvent(self, channel, current, self->keyword.blacklistCurrent);
    QBDiskPVRDirectoryTrySchedulingEvent(self, channel, next, self->keyword.blacklistNext);
}

SvLocal void
QBDiskPVRDirectoryEventsChanged(SvObject self_, SvTVChannel channel, SvEPGEvent current, SvEPGEvent next)
{
    QBDiskPVRDirectory self = (QBDiskPVRDirectory) self_;

    QBDiskPVRDirectoryUpdateBlacklist(self, SvValueGetString((SvDBObjectGetID((SvDBObject) channel))), current, next);
    QBDiskPVRDirectoryEventsUpdate(self, channel, current, next);
}

SvLocal void
QBDiskPVRDirectoryEventsUnchanged(SvObject self_, SvTVChannel channel, SvEPGEvent current, SvEPGEvent next)
{
    QBDiskPVRDirectory self = (QBDiskPVRDirectory) self_;
    QBDiskPVRDirectoryEventsUpdate(self, channel, current, next);
}

SvLocal SvType
QBDiskPVRDirectory_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDiskPVRDirectoryDestroy,
    };
    static const struct QBEPGWatcherListener_t epgWatcherListenerMethods = {
       .eventsChanged = QBDiskPVRDirectoryEventsChanged,
       .eventsUnchanged = QBDiskPVRDirectoryEventsUnchanged,
    };

    if (!type) {
        SvTypeCreateManaged("QBDiskPVRDirectory",
                            sizeof(struct QBDiskPVRDirectory_),
                            QBPVRDirectory_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBEPGWatcherListener_getInterface(), &epgWatcherListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal bool check(SvValue val, SvValueType type)
{
    return val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueGetType(val) == type;
}

QBDiskPVRDirectory
QBDiskPVRDirectoryCreateWithDesc(QBContentCategory parent,
                                 SvHashTable desc,
                                 bool addSearch,
                                 QBPVRSearchProviderTest searchProviderTest,
                                 SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    SvValue typeVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_TYPE_KEY));
    SvValue quota = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_QUOTA_KEY));
    SvValue id = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_ID_KEY));
    SvValue name = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_NAME_KEY));
    SvValue channelIdVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_CHANNEL_ID_KEY));
    SvValue backupChannelNameVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_BACKUP_CHANNEL_NAME_KEY));
    SvValue keyword = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_KEYWORD_KEY));

    SvValue repeatMode = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_REPEAT_MODE_KEY));
    SvValue days = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_DAYS_KEY));
    SvValue start = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_START_KEY));
    SvValue duration = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING(DIRECORY_DURATION_KEY));

    if (!check(quota, SvValueType_integer)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid quota value");
        goto err1;
    }
    if (!check(id, SvValueType_string)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid id value");
        goto err1;
    }
    if (!check(name, SvValueType_string)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid name value");
        goto err1;
    }

    struct QBDiskPVRDirectoryInitParams_ initParams = {
        .super_ = {
            .addSearch = addSearch,
            .searchProviderTest = searchProviderTest,
            .parent = parent,
            .id = SvValueGetString(id),
            .quota = SvValueGetInteger(quota),
            .name = SvValueGetString(name),
        },
        .days = 0,
    };

    if (check(typeVal, SvValueType_string)) {
        initParams.super_.type = QBPVRDirectoryTypeFromName(SvStringCString(SvValueGetString(typeVal)));
    }

    if ((initParams.super_.type == QBPVRDirectoryType_unknown ||
         initParams.super_.type == QBPVRDirectoryType_repeated) &&
        check(repeatMode, SvValueType_string)) {
        if (!(check(days, SvValueType_integer) && check(start, SvValueType_integer) && check(duration, SvValueType_integer))) {
            error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                      "invalid repeated info");
            goto err1;
        }

        initParams.days = SvValueGetInteger(days);
        initParams.super_.repeatedInfo.start = SvValueGetInteger(start);
        initParams.super_.repeatedInfo.duration = SvValueGetInteger(duration);
        initParams.super_.repeatedInfo.mode = QBPVRRepeatedRecordingsModeFromName(SvStringCString(SvValueGetString(repeatMode)));
        initParams.super_.type = QBPVRDirectoryType_repeated;
    }
    if (check(channelIdVal, SvValueType_string)) {
        initParams.super_.channelId = SvValueGetString(channelIdVal);
    }
    if (check(backupChannelNameVal, SvValueType_string)) {
        initParams.super_.channelName = SvValueGetString(backupChannelNameVal);
    }

    if ((initParams.super_.type == QBPVRDirectoryType_unknown ||
         initParams.super_.type == QBPVRDirectoryType_keyword) &&
        check(keyword, SvValueType_string)) {
        initParams.super_.keyword = SvValueGetString(keyword);
        if (initParams.super_.type == QBPVRDirectoryType_unknown)
            initParams.super_.type = QBPVRDirectoryType_keyword;
    }

    if (initParams.super_.type == QBPVRDirectoryType_unknown)
        initParams.super_.type = QBPVRDirectoryType_normal;

    QBDiskPVRDirectory self = QBDiskPVRDirectoryCreate(&initParams,
                                                       &error);
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBDiskPVRDirectoryCreate() failed");
        goto err1;
    }

    return self;
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBDiskPVRDirectorySerialize(QBDiskPVRDirectory self, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvString type = QBPVRDirectoryTypeCreateName(self->super_.type);
    SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_TYPE_KEY), (SvObject) type);
    SVRELEASE(type);

    SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_ID_KEY), (SvObject) QBContentCategoryGetId((QBContentCategory) self));
    SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_NAME_KEY), (SvObject) QBContentCategoryGetName((QBContentCategory) self));

    SvValue quota = SvValueCreateWithInteger(self->super_.quotaLimit, NULL);
    SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_QUOTA_KEY), (SvObject) quota);
    SVRELEASE(quota);

    if (self->super_.channelId) {
        SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_CHANNEL_ID_KEY), (SvObject) self->super_.channelId);
    }
    if (self->super_.channelName) {
        SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_BACKUP_CHANNEL_NAME_KEY), (SvObject) self->super_.channelName);
    }

    if (self->super_.type == QBPVRDirectoryType_keyword) {
        if (self->super_.keyword) {
            SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_KEYWORD_KEY), (SvObject) self->super_.keyword);
        }
    } else if (self->super_.type == QBPVRDirectoryType_repeated) {
        SvString repeat = QBPVRRepeatedRecordingsModeCreateName(self->super_.repeatedInfo.mode, false);
        SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_REPEAT_MODE_KEY), (SvObject) repeat);
        SVRELEASE(repeat);
        SvValue days = SvValueCreateWithInteger(self->repeated.days, NULL);
        SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_DAYS_KEY), (SvObject) days);
        SVRELEASE(days);
        SvValue start = SvValueCreateWithInteger(self->super_.repeatedInfo.start, NULL);
        SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_START_KEY), (SvObject) start);
        SVRELEASE(start);
        SvValue duration = SvValueCreateWithInteger(self->super_.repeatedInfo.duration, NULL);
        SvHashTableInsert(desc, (SvObject) SVSTRING(DIRECORY_DURATION_KEY), (SvObject) duration);
        SVRELEASE(duration);
    }
}

QBDiskPVRDirectory
QBDiskPVRDirectoryCreate(QBDiskPVRDirectoryInitParams params,
                         SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!params) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL params passed");
        goto err1;
    }

    QBDiskPVRDirectory self = (QBDiskPVRDirectory)
            SvTypeAllocateInstance(QBDiskPVRDirectory_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error,
                                           "can't allocate QBDiskPVRDirectory");
        goto err1;
    }

    if (QBPVRDirectoryInit((QBPVRDirectory) self,
                           &params->super_,
                           &error) < 0) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState,
                                           error,
                                           "QBPVRDirectoryInit() failed.");
        goto err2;
    }

    if (self->super_.type == QBPVRDirectoryType_repeated)
        self->repeated.days = params->days;

    return self;
err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

SvLocal bool
QBDiskPVRDirectoryIsMissed(QBPVRDirectory self, SvLocalTime startTime, int duration)
{
    SvLocalTime endTime = SvLocalTimeAdd(startTime, duration);
    SvLocalTime now = SvLocalTimeFromUTC(SvTimeGetUTC() % (24 * 60 * 60));

    if (SvLocalTimeCmp(endTime, now) >= 0)
        return false;
    return true;
}

SvLocal void
QBDiskPVRDirectoryStep(void *self_)
{
    QBDiskPVRDirectory self = self_;
    SvFiberDeactivate(self->repeated.fiber);

    QBDiskPVRDirectoryAttemptQuotaCleanup(self);

    struct tm start;
    SvTimeBreakDown(SvTimeConstruct(SvTimeNow(), 0), false, &start);
    start.tm_sec = self->super_.repeatedInfo.start % 60;
    start.tm_min = (self->super_.repeatedInfo.start / 60) % 60;
    start.tm_hour = (self->super_.repeatedInfo.start / 3600) % 24;

    if (QBDiskPVRDirectoryIsMissed(&self->super_, SvLocalTimeFromUTC(self->super_.repeatedInfo.start), self->super_.repeatedInfo.duration))
        goto fini;

    int count = 0;
    time_t now = SvTimeNow();
    SvLocalTime startTime = SvLocalTimeFromUTC(now - (now % (24 * 60 * 60)) + (self->super_.repeatedInfo.start % (24 * 60 * 60)));

    if (SvLocalTimeToInt(startTime) < self->super_.repeatedInfo.start) {
        // if recording's startTime did not occured yet, start scheduling from it
        // otherwise start from now
        startTime = SvLocalTimeFromUTC(self->super_.repeatedInfo.start);
    }

    for (int day = 0; day < 60 && count < 2; ++day) {
        mktime(&start); // fill tm_wday
        if ((1 << start.tm_wday) & self->repeated.days) {
            count++;
            SvArray results = NULL;
            QBSchedManagerFind(self->super_.channelId, startTime,
                               SvLocalTimeAdd(startTime,
                                              self->super_.repeatedInfo.duration),
                               &results, true);
            if (results && SvArrayCount(results)) {
                SVRELEASE(results);
                goto next;
            }
            SVTESTRELEASE(results);

            SVAUTOSTRINGVALUE(channelIdVal, self->super_.channelId);
            SvTVChannel channel = SvEPGManagerFindTVChannel(self->epgManager, channelIdVal, NULL);

            // it's normal that sometimes we're missing channels,
            // for example when they are provided by middleware and
            // we haven't received them yet, but we wouldn't like to
            // stop the recording
            if (!channel)
                goto next;

            QBSchedParams params;
            QBSchedParamsInit(&params);
            params.channelId = SVRETAIN(self->super_.channelId);
            params.priority = QBDiskPVRPriority_repeat;
            params.type = QBSchedType_record;

            params.startTime = startTime;
            params.stopTime = SvLocalTimeAdd(startTime, self->super_.repeatedInfo.duration);

            params.costType = (int) QBDiskPVRProviderGetPVRCostClassFromChannel(channel);

            const char *pvrStartMarginStr = QBConfigGet("PVRSTARTMARGIN");
            int pvrStartMargin = pvrStartMarginStr ? atoi(pvrStartMarginStr) * 60 : 0;
            const char *pvrEndMarginStr = QBConfigGet("PVRENDMARGIN");
            int pvrEndMargin = pvrEndMarginStr ? atoi(pvrEndMarginStr) * 60 : 0;

            params.startMargin = pvrStartMargin;
            params.stopMargin = pvrEndMargin;
            SvContentMetaDataSetStringProperty(params.meta, SVSTRING(REC_META_KEY__GROUP_ID),
                                               QBContentCategoryGetId((QBContentCategory) self));
            QBSchedManagerAdd(&params, NULL);
            QBSchedParamsDestroy(&params);
        }
next:
        startTime = SvLocalTimeAdd(startTime, 24 * 60 * 60);
        start.tm_mday++;
    }
fini:
    SvFiberTimerActivateAfter(self->repeated.timer,
                              SvTimeConstruct(REPEATED_RECORDINGS_FIBER_TIMER_PERIOD, 0));
}

void
QBDiskPVRDirectoryStart(QBDiskPVRDirectory self,
                        SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (self->started) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "directory already started");
        goto out;
    }

    QBLangPreferences langPreferences = (QBLangPreferences)
        QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBLangPreferences_getType(), &error);
    if (!langPreferences) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBGlobalStorageGetItem(\"QBLangPreferences_getType\") failed");
        goto out;
    }
    self->langPreferences = SVRETAIN(langPreferences);

    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    if (!epgManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"SvEPGManager\") failed");
        goto out;
    }
    self->epgManager = SVRETAIN(epgManager);

    QBEPGWatcher epgWatcher = (QBEPGWatcher)
        QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBEPGWatcher"));
    if (!epgWatcher) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"QBEPGWatcher\") failed");
        goto out;
    }

    if (self->super_.type == QBPVRDirectoryType_keyword) {
        self->keyword.blacklistCurrent = SvHashTableCreate(13, NULL);
        self->keyword.blacklistNext = SvHashTableCreate(13, NULL);
        QBEPGWatcherAddListener(epgWatcher, (SvObject) self);
    } else if (self->super_.type == QBPVRDirectoryType_repeated) {
        self->repeated.fiber = SvFiberCreate(SvSchedulerGet(), NULL, "PVRDirectory", QBDiskPVRDirectoryStep, self);
        self->repeated.timer = SvFiberTimerCreate(self->repeated.fiber);
        SvFiberSetPriority(self->repeated.fiber, 20);
        SvFiberActivate(self->repeated.fiber);
    }
    self->started = true;

out:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBDiskPVRDirectoryStop(QBDiskPVRDirectory self,
                       SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    if (!self->started) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "directory already stopped");
        goto out;
    }

    SVRELEASE(self->langPreferences);
    self->langPreferences = NULL;
    SVRELEASE(self->epgManager);
    self->epgManager = NULL;

    QBEPGWatcher epgWatcher = (QBEPGWatcher)
        QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBEPGWatcher"));
    if (!epgWatcher) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "QBServiceRegistryGetService(\"QBEPGWatcher\") failed");
        goto out;
    }

    if (self->super_.type == QBPVRDirectoryType_keyword) {
        QBEPGWatcherRemoveListener(epgWatcher, (SvObject) self);
        SVRELEASE(self->keyword.blacklistCurrent);
        SVRELEASE(self->keyword.blacklistNext);
        self->keyword.blacklistCurrent = NULL;
        self->keyword.blacklistNext = NULL;
    } else if (self->super_.type == QBPVRDirectoryType_repeated) {
        if (self->repeated.fiber) {
            SvFiberDestroy(self->repeated.fiber);
            self->repeated.fiber = NULL;
        }
    }
    self->started = false;

out:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBDiskPVRDirectoryStopRecording(QBDiskPVRDirectory self)
{
    QBDiskPVRDirectoryStop(self, NULL);
    if (self->super_.type == QBPVRDirectoryType_keyword) {
        SVRELEASE(self->super_.keyword);
    }
    self->super_.type = QBPVRDirectoryType_normal;
    QBDiskPVRDirectoryStart(self, NULL);

    QBObservableSetChangedAndNotify((QBObservable) self, NULL, NULL);
}

void
QBDiskPVRDirectoryUpdate(QBDiskPVRDirectory self,
                         QBPVRDirectoryUpdateParams params)
{
    QBDiskPVRDirectoryStop(self, NULL);
    if (params->updateName) {
        QBContentCategorySetName((QBContentCategory) self, params->name, NULL);
    }
    if (params->updateQuota) {
        self->super_.quotaLimit = params->quotaLimit;
    }
    if (self->super_.type == QBPVRDirectoryType_repeated && params->updateRepeatedInfo) {
        self->super_.repeatedInfo = params->repeatedInfo;
    }
    if (self->super_.type == QBPVRDirectoryType_keyword && params->updateKeyword) {
        SVRELEASE(self->super_.keyword);
        self->super_.keyword = SVRETAIN(params->keyword);
    }
    if (self->super_.type == QBPVRDirectoryType_series && params->updateSeriesID) {
        SVRELEASE(self->super_.seriesID);
        self->super_.seriesID = SVRETAIN(params->seriesID);
    }
    QBDiskPVRDirectoryStart(self, NULL);

    QBObservableSetChangedAndNotify((QBObservable) self, NULL, NULL);
}

void
QBDiskPVRDirectoryAddRecording(QBDiskPVRDirectory self,
                               QBDiskPVRRecording rec)
{
    ((QBPVRRecording) rec)->directory = SvWeakReferenceCreate((SvObject) self, NULL);
    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource((QBContentCategory) self);
    QBSortedList2Insert(list, (SvObject) rec);

    QBDiskPVRDirectoryAttemptQuotaCleanup(self);
}

void
QBDiskPVRDirectoryRemoveRecording(QBDiskPVRDirectory self,
                                  QBDiskPVRRecording rec)
{
    QBSortedList2 list = (QBSortedList2) QBContentCategoryGetItemsSource((QBContentCategory) self);
    QBSortedList2Remove(list, (SvObject) rec);
    SVRELEASE(((QBPVRRecording) rec)->directory);
    ((QBPVRRecording) rec)->directory = NULL;
}

void
QBDiskPVRKeywordDirectoryBlacklistRecording(QBDiskPVRDirectory self,
                                            QBDiskPVRRecording rec)
{
    if (((QBPVRRecording) rec)->state == QBPVRRecordingState_scheduled) {
        SvHashTableInsert(self->keyword.blacklistNext, (SvObject) ((QBPVRRecording) rec)->channelId, (SvObject) ((QBPVRRecording) rec)->event);
    } else if (((QBPVRRecording) rec)->state == QBPVRRecordingState_active) {
        SvHashTableInsert(self->keyword.blacklistCurrent, (SvObject) ((QBPVRRecording) rec)->channelId, (SvObject) ((QBPVRRecording) rec)->event);
    }
}
