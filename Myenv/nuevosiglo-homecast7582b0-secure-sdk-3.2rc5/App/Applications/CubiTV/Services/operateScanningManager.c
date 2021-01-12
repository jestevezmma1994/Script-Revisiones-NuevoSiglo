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

#include <main.h>
#include <mpeg_tables/bat.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvArray.h>
#include <SvEPGDataLayer/SvEPGChannelListListener.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <QBTSSection.h>
#include <Widgets/scanningDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Services/core/JSONserializer.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <Logic/OperateScanningLogic.h>
#include <safeFiles.h>
#include <stdint.h>
#include <SvCore/SvLog.h>

#define QB_OPERATE_SCANNING_MANAGER_RESURRECTION_TIME_MS 3600000
#define QB_OPERATE_SCANNING_MANAGER_MIN_CHANGE_INTERVAL_MS 5000


struct QBOperateScanningManagerVersionHistory_t {
    struct SvObject_ super_;
    int lastVersionDetected;
    int currentVersion;
    int previousVersion;
    SvTime lastVersionChangeTime;
    SvFiber fiber;
    SvFiberTimer timer;
    QBOperateScanningManager scanningManager;
    unsigned int tunerNum;
};
typedef struct QBOperateScanningManagerVersionHistory_t *QBOperateScanningManagerVersionHistory;

struct QBOperateScanningManager_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvString historyFilePath;
    SvString confFilePath;

    QBDVBTableMonitor tableMonitor;

    SvWidget dialog;

    SvHashTable idToNITActualVersion;
    SvHashTable idToNITOtherVersion;
    SvHashTable idToSDTActualVersion;
    SvHashTable idToSDTOtherVersion;
    SvHashTable idToBATVersion;

    SvArray versionsToBeStored;

    SvFiber delayFiber;
    SvFiberTimer delayTimer;

    SvFiber fixedNotificationTimeFiber;
    SvFiberTimer fixedNotificationTimeTimer;

    struct tm fixedNotificationTime;

    unsigned int popupTimeout;

    bool popupShowScheduled;

    struct {
        bool nit_actual;
        bool nit_other;
        bool sdt_actual;
        bool sdt_other;
        bool bat;
    } tablesToBeParsed;

    bool immediateNotify;

    int channelCounter;
};

SvLocal void
QBOperateScanningManagerRequestChannelScanning(void *self_);

SvLocal void
QBOperateScanningManagerSaveHistory(QBOperateScanningManager self);

SvLocal void
QBOperateScanningManagerClean(QBOperateScanningManager self);

SvLocal void
QBOperateScanningManagerVersionHistory__dtor__(void *self_)
{
    QBOperateScanningManagerVersionHistory self = (QBOperateScanningManagerVersionHistory) self_;
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
    }
}

SvLocal SvType
QBOperateScanningManagerVersionHistory_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBOperateScanningManagerVersionHistory__dtor__
    };
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBOperateScanningManagerVersionHistory",
                            sizeof(struct QBOperateScanningManagerVersionHistory_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBOperateScanningManagerVersionHistory
QBOperateScanningManagerVersionHistoryCreate(int lastVersionDetected,
                                             int currentVersion,
                                             int previousVersion)
{
    QBOperateScanningManagerVersionHistory self =
            (QBOperateScanningManagerVersionHistory) SvTypeAllocateInstance(QBOperateScanningManagerVersionHistory_getType(), NULL);

    self->lastVersionDetected = lastVersionDetected;
    self->currentVersion = currentVersion;
    self->previousVersion = previousVersion;
    self->lastVersionChangeTime = SvTimeGet();

    return self;
}

SvLocal void
QBOperateScanningManagerVersionHistoryInit(QBOperateScanningManagerVersionHistory self, QBOperateScanningManager scanningManager)
{
    self->scanningManager = scanningManager;
}

SvLocal void
QBOperateScanningManagerVersionHistoryStep(void *self_)
{
    QBOperateScanningManagerVersionHistory self = (QBOperateScanningManagerVersionHistory) self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    if (QBOperateScanningLogicScanningNeeded(self->scanningManager->appGlobals->operateScanningLogic, self->tunerNum, true, NULL, NULL)) {
        if (!self->scanningManager->popupShowScheduled)
            QBOperateScanningManagerRequestChannelScanning(self->scanningManager);
    }
}

SvLocal void
QBOperateScanningManagerVersionHistoryStart(QBOperateScanningManagerVersionHistory self, SvScheduler scheduler)
{
    if (!self->fiber) {
        self->fiber = SvFiberCreate(scheduler, NULL, "QBOperateScanningManagerVersionHistory", QBOperateScanningManagerVersionHistoryStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
    }
}

SvLocal void
QBOperateScanningManagerVersionHistoryStop(QBOperateScanningManagerVersionHistory self)
{
    if (self->fiber) {
        SvFiberEventDeactivate(self->timer);
        SvFiberDeactivate(self->fiber);
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }
}

SvLocal void
QBOperateScanningManagerVersionHistorySerialize(SvGenericObject helper_, SvGenericObject self_, SvHashTable desc, SvErrorInfo *errorOut)
{
    QBOperateScanningManagerVersionHistory self = (QBOperateScanningManagerVersionHistory) self_;

    SvValue tmp = SvValueCreateWithInteger(self->lastVersionDetected, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("lastVersionDetected"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithInteger(self->currentVersion, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("currentVersion"), (SvGenericObject) tmp);
    SVRELEASE(tmp);

    tmp = SvValueCreateWithInteger(self->previousVersion, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("previousVersion"), (SvGenericObject) tmp);
    SVRELEASE(tmp);
}

SvLocal SvGenericObject
QBOperateScanningManagerVersionHistoryDeserialize(SvGenericObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    SvValue lastVersionDetectedVal = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("lastVersionDetected"));
    SvValue currentVersionVal = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("currentVersion"));
    SvValue previousVersionVal = (SvValue) SvHashTableFind(desc, (SvGenericObject) SVSTRING("previousVersion"));

    if (!lastVersionDetectedVal || !currentVersionVal || !previousVersionVal) {
        SvLogError("Operate scanning manager history file is invalid!");
        SvLogError("%s : ptr=(%p,%p,%p)", __func__, lastVersionDetectedVal, currentVersionVal, previousVersionVal);
        return NULL;
    }


    if (!SvObjectIsInstanceOf((SvObject) lastVersionDetectedVal, SvValue_getType()) ||
        !SvObjectIsInstanceOf((SvObject) currentVersionVal, SvValue_getType()) ||
        !SvObjectIsInstanceOf((SvObject) previousVersionVal, SvValue_getType())) {
        SvLogError("Operate scanning manager history file is invalid!");
        SvLogError("%s : type=(%s,%s,%s)", __func__,
                   SvObjectGetTypeName((SvObject) lastVersionDetectedVal),
                   SvObjectGetTypeName((SvObject) currentVersionVal),
                   SvObjectGetTypeName((SvObject) previousVersionVal));
        return NULL;
    }


    if (!SvValueIsInteger(lastVersionDetectedVal) ||
        !SvValueIsInteger(currentVersionVal) ||
        !SvValueIsInteger(previousVersionVal)) {
        SvLogError("Operate scanning manager history file is invalid!");
        SvLogError("%s : SvValueType=(%d,%d,%d)", __func__,
                   SvValueGetType(lastVersionDetectedVal),
                   SvValueGetType(currentVersionVal),
                   SvValueGetType(previousVersionVal));
        return NULL;
    }

    return (SvGenericObject) QBOperateScanningManagerVersionHistoryCreate(
        SvValueGetInteger(lastVersionDetectedVal),
        SvValueGetInteger(currentVersionVal),
        SvValueGetInteger(previousVersionVal));
}


SvLocal void
QBOperateScanningManagerPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBOperateScanningManager self = self_;
    bool standby = QBStandbyAgentIsStandby(self->appGlobals->standbyAgent);

    if (!standby && ((buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) ||
            (!(buttonTag && SvStringEqualToCString(buttonTag, "cancel-button")) && self->popupTimeout))) {
        self->popupShowScheduled = false;
        QBChannelScanningLogicShowChannelScanning(self->appGlobals->channelScanningLogic);
    } else {
        SvFiberTimerActivateAfter(self->delayTimer, SvTimeFromMilliseconds(QB_OPERATE_SCANNING_MANAGER_RESURRECTION_TIME_MS));
        self->popupShowScheduled = true;
    }

    QBOperateScanningManagerSaveHistory(self);
    self->dialog = NULL;
}

SvLocal void
QBOperateScanningManagerDisplayPopup(QBOperateScanningManager self)
{
    self->dialog = QBScanningDialogCreate(self->appGlobals, self->popupTimeout);
    QBDialogRun(self->dialog, self, QBOperateScanningManagerPopupCallback);
}

SvLocal void
QBOperateScanningManagerRequestChannelScanning(void *self_)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    SvFiberDeactivate(self->delayFiber);
    SvFiberEventDeactivate(self->delayTimer);

    struct tm now;
    SvTimeBreakDown(SvTimeGetCurrentTime(), true, &now);
    if (!self->immediateNotify) {
        int popupDelaySec = (self->fixedNotificationTime.tm_hour - now.tm_hour) * 60 * 60 +
                            (self->fixedNotificationTime.tm_min - now.tm_min) * 60 +
                            self->fixedNotificationTime.tm_sec - now.tm_sec;

        if (popupDelaySec < 0)
            popupDelaySec += 24 * 60 * 60;
        SvFiberTimerActivateAfter(self->fixedNotificationTimeTimer, SvTimeConstruct(abs(popupDelaySec), 0));
    } else {
        QBOperateScanningManagerDisplayPopup(self);
    }

    self->popupShowScheduled = true;
}

SvLocal void QBOperateScanningManagerFixedNotificationTimeReached(void *self_)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    SvFiberDeactivate(self->fixedNotificationTimeFiber);
    SvFiberEventDeactivate(self->fixedNotificationTimeTimer);

    QBOperateScanningManagerDisplayPopup(self);

    self->popupShowScheduled = true;
}

SvLocal SvGenericObject
QBOperateScanningManagerSerializeHistory(QBOperateScanningManager self)
{
    SvHashTable desc = SvHashTableCreate(4, NULL);

    if (self->idToNITActualVersion)
        SvHashTableInsert(desc, (SvGenericObject) SVSTRING("idToNITActualVersion"), (SvGenericObject) self->idToNITActualVersion);
    if (self->idToNITOtherVersion)
        SvHashTableInsert(desc, (SvGenericObject) SVSTRING("idToNITOtherVersion"), (SvGenericObject) self->idToNITOtherVersion);
    if (self->idToSDTActualVersion)
        SvHashTableInsert(desc, (SvGenericObject) SVSTRING("idToSDTActualVersion"), (SvGenericObject) self->idToSDTActualVersion);
    if (self->idToSDTOtherVersion)
        SvHashTableInsert(desc, (SvGenericObject) SVSTRING("idToSDTOtherVersion"), (SvGenericObject) self->idToSDTOtherVersion);
    if (self->idToBATVersion)
        SvHashTableInsert(desc, (SvGenericObject) SVSTRING("idToBATVersion"), (SvGenericObject) self->idToBATVersion);

    SvValue popupShowScheduledVal = SvValueCreateWithBoolean(self->popupShowScheduled, NULL);
    SvHashTableInsert(desc, (SvGenericObject) SVSTRING("popupShowScheduled"), (SvGenericObject) popupShowScheduledVal);
    SVRELEASE(popupShowScheduledVal);

    return (SvGenericObject) desc;
}

SvLocal SvHashTable
QBOperateScanningManagerDeserializeTableHistory(QBOperateScanningManager self, SvHashTable desc, SvString idToTableVersionKey)
{
    SvGenericObject idToTableVersion = SVTESTRETAIN(SvHashTableFind(desc, (SvGenericObject) idToTableVersionKey));
    if (idToTableVersion && !SvObjectIsInstanceOf((SvObject) idToTableVersion, SvHashTable_getType())) {
        SvLogError("File %s is invalid!", SvStringCString(self->historyFilePath));
        SvLogError("%s : type=(%s)", __func__, SvObjectGetTypeName(idToTableVersion));
        SVRELEASE(idToTableVersion);
        idToTableVersion = NULL;
    }
    return (SvHashTable) idToTableVersion;
}

SvLocal void
QBOperateScanningManagerDeserializeHistory(QBOperateScanningManager self, SvHashTable desc)
{
    self->idToNITActualVersion = QBOperateScanningManagerDeserializeTableHistory(self, desc, SVSTRING("idToNITActualVersion"));
    self->idToNITOtherVersion = QBOperateScanningManagerDeserializeTableHistory(self, desc, SVSTRING("idToNITOtherVersion"));
    self->idToSDTActualVersion = QBOperateScanningManagerDeserializeTableHistory(self, desc, SVSTRING("idToSDTActualVersion"));
    self->idToSDTOtherVersion = QBOperateScanningManagerDeserializeTableHistory(self, desc, SVSTRING("idToSDTOtherVersion"));
    self->idToBATVersion = QBOperateScanningManagerDeserializeTableHistory(self, desc, SVSTRING("idToBATVersion"));

    SvValue popupShowScheduledVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("popupShowScheduled"));
    if (popupShowScheduledVal && !SvObjectIsInstanceOf((SvObject) popupShowScheduledVal, SvValue_getType())) {
        SvLogError("File %s is invalid!", SvStringCString(self->historyFilePath));
        SvLogError("%s : type=(%s)", __func__, SvObjectGetTypeName((SvObject) popupShowScheduledVal));
        self->popupShowScheduled = false;
    } else {
        self->popupShowScheduled = popupShowScheduledVal ? SvValueGetBoolean(popupShowScheduledVal) : false;
    }
}

SvLocal bool
QBOperateScanningManagerDeserializeConfField(QBOperateScanningManager self, SvHashTable desc, SvString fieldId)
{
    SvGenericObject toBeParsed = SvHashTableFind(desc, (SvGenericObject) fieldId);
    if (!toBeParsed || (!SvObjectIsInstanceOf(toBeParsed, SvValue_getType()) || !SvValueIsBoolean((SvValue) toBeParsed))) {
        SvLogError("File %s is invalid!", SvStringCString(self->confFilePath));
        SvLogError("%s : type=(%s)", __func__, toBeParsed ? SvObjectGetTypeName(toBeParsed) : "null");
        return true;
    }

    return SvValueGetBoolean((SvValue) toBeParsed);
}

SvLocal void
QBOperateScanningManagerDeserializeConf(QBOperateScanningManager self,  SvHashTable desc)
{
    self->tablesToBeParsed.nit_actual = QBOperateScanningManagerDeserializeConfField(self, desc, SVSTRING("nit_actual"));
    self->tablesToBeParsed.nit_other = QBOperateScanningManagerDeserializeConfField(self, desc, SVSTRING("nit_other"));
    self->tablesToBeParsed.sdt_actual = QBOperateScanningManagerDeserializeConfField(self, desc, SVSTRING("sdt_actual"));
    self->tablesToBeParsed.sdt_other = QBOperateScanningManagerDeserializeConfField(self, desc, SVSTRING("sdt_other"));
    self->tablesToBeParsed.bat = QBOperateScanningManagerDeserializeConfField(self, desc, SVSTRING("bat"));

    self->immediateNotify = QBOperateScanningManagerDeserializeConfField(self, desc, SVSTRING("immediateNotify"));

    if (!self->immediateNotify) {
        SvGenericObject fixedNotificationTimeVal = SvHashTableFind(desc, (SvGenericObject) SVSTRING("notifyTime"));
        if (!fixedNotificationTimeVal || (!SvObjectIsInstanceOf(fixedNotificationTimeVal, SvValue_getType()) || !SvValueIsInteger((SvValue) fixedNotificationTimeVal))) {
            SvLogError("File %s is invalid!", SvStringCString(self->confFilePath));
            SvLogError("%s : type=(%s)", __func__, fixedNotificationTimeVal ? SvObjectGetTypeName(fixedNotificationTimeVal) : "null");
            self->immediateNotify = true;
        } else {
            int fixedNotificationTimeMin = SvValueGetInteger((SvValue) fixedNotificationTimeVal);
            self->fixedNotificationTime.tm_hour = fixedNotificationTimeMin / 60;
            self->fixedNotificationTime.tm_min = fixedNotificationTimeMin % 60;
            self->fixedNotificationTime.tm_sec = 0;
        }
    }

    SvGenericObject popupTimeoutVal = SvHashTableFind(desc, (SvGenericObject) SVSTRING("popupTimeout"));
    if (!popupTimeoutVal || (!SvObjectIsInstanceOf(popupTimeoutVal, SvValue_getType()) || !SvValueIsInteger((SvValue) popupTimeoutVal))) {
        SvLogError("File %s is invalid!", SvStringCString(self->confFilePath));
        SvLogError("%s : type=(%s)", __func__, popupTimeoutVal ? SvObjectGetTypeName(popupTimeoutVal) : "null");
        self->popupTimeout = 0;
    } else {
        self->popupTimeout = SvValueGetInteger((SvValue) popupTimeoutVal);
    }
}

SvLocal void
QBOperateScanningManagerSaveHistory(QBOperateScanningManager self)
{
    SvHashTable desc = (SvHashTable) QBOperateScanningManagerSerializeHistory(self);
    QBJSONSerializer JSONserializer = (QBJSONSerializer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBJSONSerializer"));
    QBJSONSerializerAddJob(JSONserializer, (SvGenericObject) desc, self->historyFilePath);
    SVRELEASE(desc);
}

SvLocal void
QBOperateScanningManagerLoadHistory(QBOperateScanningManager self)
{
    char *buffer = NULL;
    QBFileToBuffer(SvStringCString(self->historyFilePath), &buffer);
    if (!buffer)
        return;

    SvGenericObject desc = SvJSONParseString(buffer, true, NULL);
    free(buffer);
    if (desc) {
        if (SvObjectIsInstanceOf((SvObject) desc, SvHashTable_getType()))
            QBOperateScanningManagerDeserializeHistory(self, (SvHashTable) desc);
        SVRELEASE(desc);
    }
}

SvLocal void
QBOperateScanningManagerLoadConf(QBOperateScanningManager self)
{
    char *buffer = NULL;
    QBFileToBuffer(SvStringCString(self->confFilePath), &buffer);
    if (!buffer)
        return;

    SvGenericObject desc = SvJSONParseString(buffer, true, NULL);
    free(buffer);
    if (desc) {
        if (SvObjectIsInstanceOf((SvObject) desc, SvHashTable_getType()))
            QBOperateScanningManagerDeserializeConf(self, (SvHashTable) desc);
        SVRELEASE(desc);
    }
}

SvLocal void
QBOperateScanningManagerTableVersionChanged(QBOperateScanningManager self, SvHashTable idToTableVersion, int versionNumber, int tsid, int onid, unsigned int tunerNum)
{
    bool saveNeeded = false;
    bool scanningNeeded = false;
    bool laterCheckNeeded = false;

    SvString tsidStr = SvStringCreateWithFormat("%d", tsid);
    SvString onidStr = SvStringCreateWithFormat("%d", onid);

    SvHashTable onidVersionHistory = (SvHashTable) SvHashTableFind(idToTableVersion, (SvGenericObject) onidStr);
    if (!onidVersionHistory) {
        onidVersionHistory = SvHashTableCreate(10, NULL);
        SvHashTableInsert(idToTableVersion, (SvGenericObject) onidStr, (SvGenericObject) onidVersionHistory);
        SVRELEASE(onidVersionHistory);
    }

    QBOperateScanningManagerVersionHistory versionHistory =
            (QBOperateScanningManagerVersionHistory) SvHashTableFind(onidVersionHistory, (SvGenericObject) tsidStr);
    if (!versionHistory) {
        versionHistory = QBOperateScanningManagerVersionHistoryCreate(versionNumber, versionNumber, versionNumber);
        SvHashTableInsert(onidVersionHistory, (SvGenericObject) tsidStr, (SvGenericObject) versionHistory);
        SVRELEASE(versionHistory);
        saveNeeded = true;
    }
    SVRELEASE(tsidStr);
    SVRELEASE(onidStr);

    if (!versionHistory->scanningManager) {
        QBOperateScanningManagerVersionHistoryInit(versionHistory, self);
    }

    // analize version history

    SvTime versionPreviousChangeTime = versionHistory->lastVersionChangeTime;

    if (versionHistory->lastVersionDetected != versionNumber) {
        versionHistory->lastVersionDetected = versionNumber;
        versionHistory->lastVersionChangeTime = SvTimeGet();
        saveNeeded = true;
    }

    if (versionHistory->currentVersion == versionNumber) {
        if (versionHistory->fiber)
            QBOperateScanningManagerVersionHistoryStop(versionHistory);
        goto fini;
    }

    saveNeeded = true;
    if (versionHistory->previousVersion == versionNumber) {
        laterCheckNeeded = true;
    } else {
        if (SvTimeToMilliseconds(SvTimeSub(SvTimeGet(), versionPreviousChangeTime)) > QB_OPERATE_SCANNING_MANAGER_MIN_CHANGE_INTERVAL_MS) {
            scanningNeeded = true;
        } else {
            laterCheckNeeded = true;
        }
    }

fini:
    if (laterCheckNeeded) {
        versionHistory->tunerNum = tunerNum;
        QBOperateScanningManagerVersionHistoryStart(versionHistory, self->appGlobals->scheduler);
        SvFiberTimerActivateAfter(versionHistory->timer, SvTimeFromMilliseconds(QB_OPERATE_SCANNING_MANAGER_MIN_CHANGE_INTERVAL_MS));
    } else if (scanningNeeded) {
        if (QBOperateScanningLogicScanningNeeded(self->appGlobals->operateScanningLogic, tunerNum, scanningNeeded, NULL, NULL)) {
            if (!self->popupShowScheduled)
                QBOperateScanningManagerRequestChannelScanning(self);
        }
    }

    if (saveNeeded) {
        if (!SvArrayContainsObject(self->versionsToBeStored, (SvObject) versionHistory))
            SvArrayAddObject(self->versionsToBeStored, (SvObject) versionHistory);
    }
}

SvLocal void
QBOperateScanningManagerNITActualVersionChanged(QBOperateScanningManager self, const struct SvChbuf_s* chb, unsigned int tunerNum)
{
    if (!self->tablesToBeParsed.nit_actual)
        return;

    int versionNumber = QBTSSectionGetVersionNumber(chb->first->data);
    int nid = QBTSSectionGetTableIDExtension(chb->first->data);
    /* In NIT tables nid is unique identifier so we pass nid as func parameter twice to keep the same memory structure as for SDT tables */
    QBOperateScanningManagerTableVersionChanged(self, self->idToNITActualVersion, versionNumber, nid, nid, tunerNum);
}

SvLocal void
QBOperateScanningManagerNITOtherVersionChanged(QBOperateScanningManager self, const struct SvChbuf_s* chb, unsigned int tunerNum)
{
    if (!self->tablesToBeParsed.nit_other)
        return;

    int versionNumber = QBTSSectionGetVersionNumber(chb->first->data);
    int nid = QBTSSectionGetTableIDExtension(chb->first->data);
    /* In NIT tables nid is unique identifier so we pass nid as func parameter twice to keep the same memory structure as for SDT tables */
    QBOperateScanningManagerTableVersionChanged(self, self->idToNITOtherVersion, versionNumber, nid, nid, tunerNum);
}

SvLocal void
QBOperateScanningManagerNITVersionChanged(SvGenericObject self_, unsigned int tunerNum, mpeg_nit_parser* nit_parser)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    if (mpeg_nit_parser_get_table_id(nit_parser) == MPEG_TABLE_ID_NIT_ACTUAL)
        QBOperateScanningManagerNITActualVersionChanged(self, mpeg_nit_parser_get_orig_chb(nit_parser), tunerNum);
    else
        QBOperateScanningManagerNITOtherVersionChanged(self, mpeg_nit_parser_get_orig_chb(nit_parser), tunerNum);
}

SvLocal void
QBOperateScanningManagerSDTActualVersionChanged(QBOperateScanningManager self, const struct SvChbuf_s* chb, unsigned int tunerNum)
{
    if (!self->tablesToBeParsed.sdt_actual)
        return;

    int versionNumber = QBTSSectionGetVersionNumber(chb->first->data);
    int tsid = QBTSSectionGetTableIDExtension(chb->first->data);
    int onid = get32(chb->first->data + 8, 0, 16);
    QBOperateScanningManagerTableVersionChanged(self, self->idToSDTActualVersion, versionNumber, tsid, onid, tunerNum);
}

SvLocal void
QBOperateScanningManagerSDTOtherVersionChanged(QBOperateScanningManager self, const struct SvChbuf_s* chb, unsigned int tunerNum)
{
    if (!self->tablesToBeParsed.sdt_other)
        return;

    int versionNumber = QBTSSectionGetVersionNumber(chb->first->data);
    int tsid = QBTSSectionGetTableIDExtension(chb->first->data);
    int onid = get32(chb->first->data + 8, 0, 16);
    QBOperateScanningManagerTableVersionChanged(self, self->idToSDTOtherVersion, versionNumber, tsid, onid, tunerNum);
}

SvLocal void
QBOperateScanningManagerSDTVersionChanged(SvGenericObject self_, unsigned int tunerNum, mpeg_sdt_parser* sdt_parser)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    if (mpeg_sdt_parser_get_table_id(sdt_parser) == MPEG_TABLE_ID_SDT_ACTUAL)
        QBOperateScanningManagerSDTActualVersionChanged(self, mpeg_sdt_parser_get_orig_chb(sdt_parser), tunerNum);
    else
        QBOperateScanningManagerSDTOtherVersionChanged(self, mpeg_sdt_parser_get_orig_chb(sdt_parser), tunerNum);
}

SvLocal void
QBOperateScanningManagerCATVersionChanged(SvGenericObject self_, unsigned int tunerNum, mpeg_cat_parser* sdt_parser)
{
    // don't care
}

SvLocal void
QBOperateScanningManagerPATVersionChanged(SvGenericObject self_, unsigned int tunerNum, mpeg_pat_parser* sdt_parser)
{
    // don't care
}

SvLocal void
QBOperateScanningManagerBATVersionChanged(SvObject self_, unsigned int tunerNum, mpeg_bat_parser *bat_parser)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    if (!self->tablesToBeParsed.bat)
        return;
    int versionNumber = QBTSSectionGetVersionNumber(mpeg_bat_parser_get_orig_chb(bat_parser)->first->data);
    int bouquetid = QBTSSectionGetTableIDExtension(mpeg_bat_parser_get_orig_chb(bat_parser)->first->data);
    /* In BAT tables bouquetid is unique identifier so we pass bouquetid as func parameter twice to keep the same memory structure as for SDT tables */
    QBOperateScanningManagerTableVersionChanged(self, self->idToBATVersion, versionNumber, bouquetid, bouquetid, tunerNum);
}

SvLocal void
QBOperateScanningManagerDVBTableRepeated(SvGenericObject self_, unsigned int tunerNum, SvBuf chb, QBDVBTableMonitorTableID id)
{
    // don't care
}

SvLocal void
QBOperateScanningManagerChannelFound(SvGenericObject self_, SvTVChannel channel)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    self->channelCounter++;
}

SvLocal void
QBOperateScanningManagerChannelLost(SvGenericObject self_, SvTVChannel channel)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    self->channelCounter--;

    if (self->channelCounter == 0)
        QBOperateScanningManagerClean(self);
}

SvLocal void
QBOperateScanningManagerChannelModified(SvGenericObject self_, SvTVChannel channel)
{
}

SvLocal void
QBOperateScanningManagerChannelListCompleted(SvObject self_, int pluginID)
{
}

SvLocal void
QBOperateScanningManager__dtor__(void *self_)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;

    SVTESTRELEASE(self->idToNITActualVersion);
    SVTESTRELEASE(self->idToNITOtherVersion);
    SVTESTRELEASE(self->idToSDTActualVersion);
    SVTESTRELEASE(self->idToSDTOtherVersion);
    SVTESTRELEASE(self->idToBATVersion);
    SVRELEASE(self->versionsToBeStored);
    SVRELEASE(self->historyFilePath);
    SVRELEASE(self->confFilePath);

    if (self->dialog)
        svWidgetDestroy(self->dialog);

    if (self->delayFiber)
        SvFiberDestroy(self->delayFiber);

    if (self->fixedNotificationTimeFiber)
        SvFiberDestroy(self->fixedNotificationTimeFiber);
}

SvLocal
void QBOperateScanningManagerTimeUpdated(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    QBOperateScanningManager self = (QBOperateScanningManager) self_;
    if (self->popupShowScheduled && !self->immediateNotify)
        QBOperateScanningManagerRequestChannelScanning(self);
}

SvLocal SvType
QBOperateScanningManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBOperateScanningManager__dtor__
    };
    static SvType type = NULL;
    if (!type) {
        static const struct QBDVBTableMonitorListener_t tableMonitorMethods = {
            .NITVersionChanged = QBOperateScanningManagerNITVersionChanged,
            .SDTVersionChanged = QBOperateScanningManagerSDTVersionChanged,
            .CATVersionChanged = QBOperateScanningManagerCATVersionChanged,
            .PATVersionChanged = QBOperateScanningManagerPATVersionChanged,
            .BATVersionChanged = QBOperateScanningManagerBATVersionChanged,
            .tableRepeated = QBOperateScanningManagerDVBTableRepeated,
        };

        static const struct SvEPGChannelListListener_ dvbChannelListListenerMethods = {
            .channelFound         = QBOperateScanningManagerChannelFound,
            .channelLost          = QBOperateScanningManagerChannelLost,
            .channelModified      = QBOperateScanningManagerChannelModified,
            .channelListCompleted = QBOperateScanningManagerChannelListCompleted
        };

        static struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
            .systemTimeSet = QBOperateScanningManagerTimeUpdated
        };

        SvTypeCreateManaged("QBOperateScanningManager",
                            sizeof(struct QBOperateScanningManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBDVBTableMonitorListener_getInterface(), &tableMonitorMethods,
                            SvEPGChannelListListener_getInterface(), &dvbChannelListListenerMethods,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            NULL);
    }

    return type;
}

QBOperateScanningManager
QBOperateScanningManagerCreate(AppGlobals appGlobals, SvString historyFilePath, SvString confFilePath)
{
    QBOperateScanningManager self = (QBOperateScanningManager) SvTypeAllocateInstance(QBOperateScanningManager_getType(), NULL);

    self->appGlobals = appGlobals;
    self->historyFilePath = SVRETAIN(historyFilePath);
    self->confFilePath = SVRETAIN(confFilePath);
    self->popupShowScheduled = false;

    SvJSONClass helper = SvJSONClassCreate(QBOperateScanningManagerVersionHistorySerialize, QBOperateScanningManagerVersionHistoryDeserialize, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBOperateScanningManagerVersionHistory"), (SvGenericObject) helper, NULL);
    SVRELEASE(helper);

    QBOperateScanningManagerLoadConf(self);
    QBOperateScanningManagerLoadHistory(self);

    if (!self->idToNITActualVersion)
        self->idToNITActualVersion = SvHashTableCreate(20, NULL);

    if (!self->idToNITOtherVersion)
        self->idToNITOtherVersion = SvHashTableCreate(20, NULL);

    if (!self->idToSDTActualVersion)
        self->idToSDTActualVersion = SvHashTableCreate(20, NULL);

    if (!self->idToSDTOtherVersion)
        self->idToSDTOtherVersion = SvHashTableCreate(20, NULL);

    if (!self->idToBATVersion)
        self->idToBATVersion = SvHashTableCreate(20, NULL);

    self->versionsToBeStored = SvArrayCreateWithCapacity(8, NULL);
    self->channelCounter = 0;

    return self;
}

void QBOperateScanningManagerStart(QBOperateScanningManager self)
{
    if (!self->immediateNotify) {
        QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                            SVSTRING("QBTimeDateMonitor"));
        QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);
    }

    self->tableMonitor =
        SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBDVBTableMonitor")));
    if (self->tableMonitor) {
        self->delayFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBOperateScanningManagerPopupDelay", QBOperateScanningManagerRequestChannelScanning, self);
        self->delayTimer = SvFiberTimerCreate(self->delayFiber);

        if (!self->immediateNotify) {
            self->fixedNotificationTimeFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBOperateScanningManagerFixedNotificationTime",
                    QBOperateScanningManagerFixedNotificationTimeReached, self);
            self->fixedNotificationTimeTimer = SvFiberTimerCreate(self->fixedNotificationTimeFiber);
        }

        if (self->appGlobals->epgPlugin.DVB.channelList) {
            QBDVBTableMonitorAddListener(self->tableMonitor, (SvGenericObject) self);
            SvInvokeInterface(SvEPGChannelListPlugin, self->appGlobals->epgPlugin.DVB.channelList, addListener, (SvGenericObject) self, false, NULL);
        }

        if (self->popupShowScheduled)
            QBOperateScanningManagerDisplayPopup(self);
    } else {
        SvLogError("DVB table monitor is NULL");
    }
}

SvLocal void QBOperateScanningManagerStopTableHistoryTracing(SvHashTable idToTableVersion)
{
    if (idToTableVersion) {
        SvIterator it = SvHashTableValuesIterator(idToTableVersion);
        SvHashTable onidHistory = NULL;
        while ((onidHistory = (SvHashTable) SvIteratorGetNext(&it))) {
            SvIterator itHist = SvHashTableValuesIterator(onidHistory);
            QBOperateScanningManagerVersionHistory versionHistory = NULL;
            while((versionHistory = (QBOperateScanningManagerVersionHistory) SvIteratorGetNext(&itHist)))
                QBOperateScanningManagerVersionHistoryStop(versionHistory);
        }
    }
}

void QBOperateScanningManagerStop(QBOperateScanningManager self)
{
    if (self->delayFiber) {
        SvFiberEventDeactivate(self->delayTimer);
        SvFiberDeactivate(self->delayFiber);
        SvFiberDestroy(self->delayFiber);
        self->delayFiber = NULL;
    }

    if (self->fixedNotificationTimeFiber) {
        SvFiberEventDeactivate(self->fixedNotificationTimeTimer);
        SvFiberDeactivate(self->fixedNotificationTimeFiber);
        SvFiberDestroy(self->fixedNotificationTimeFiber);
        self->fixedNotificationTimeFiber = NULL;
    }

    QBOperateScanningManagerStopTableHistoryTracing(self->idToNITActualVersion);
    QBOperateScanningManagerStopTableHistoryTracing(self->idToNITOtherVersion);
    QBOperateScanningManagerStopTableHistoryTracing(self->idToSDTActualVersion);
    QBOperateScanningManagerStopTableHistoryTracing(self->idToSDTOtherVersion);
    QBOperateScanningManagerStopTableHistoryTracing(self->idToBATVersion);

    if (self->tableMonitor) {
        QBDVBTableMonitorRemoveListener(self->tableMonitor, (SvObject) self);
        SVRELEASE(self->tableMonitor);
        self->tableMonitor = NULL;
    }
}

void QBOperateScanningManagerChannelScanningFinished(QBOperateScanningManager self)
{
    if (self->dialog) {
        QBDialogBreak(self->dialog);
        self->dialog = NULL;
    }

    self->popupShowScheduled = false;

    if (self->fixedNotificationTimeFiber) {
        SvFiberEventDeactivate(self->fixedNotificationTimeTimer);
        SvFiberDeactivate(self->fixedNotificationTimeFiber);
    }

    // store all pending versions
    if (SvArrayCount(self->versionsToBeStored) > 0) {
        SvIterator iter = SvArrayGetIterator(self->versionsToBeStored);
        QBOperateScanningManagerVersionHistory version = NULL;
        while ((version = (QBOperateScanningManagerVersionHistory) SvIteratorGetNext(&iter))) {
            version->previousVersion = version->currentVersion;
            version->currentVersion = version->lastVersionDetected;
        }
        SvArrayRemoveAllObjects(self->versionsToBeStored);
        QBOperateScanningManagerSaveHistory(self);
    }
}

SvLocal void QBOperateScanningManagerClean(QBOperateScanningManager self)
{
    remove(SvStringCString(self->historyFilePath));

    SvHashTableRemoveAllObjects(self->idToNITActualVersion);
    SvHashTableRemoveAllObjects(self->idToNITOtherVersion);
    SvHashTableRemoveAllObjects(self->idToSDTActualVersion);
    SvHashTableRemoveAllObjects(self->idToSDTOtherVersion);
    SvHashTableRemoveAllObjects(self->idToBATVersion);
    SvArrayRemoveAllObjects(self->versionsToBeStored);

    self->popupShowScheduled = false;
    if (self->delayFiber) {
        SvFiberEventDeactivate(self->delayTimer);
        SvFiberDeactivate(self->delayFiber);
    }

    if (self->fixedNotificationTimeFiber) {
        SvFiberEventDeactivate(self->fixedNotificationTimeTimer);
        SvFiberDeactivate(self->fixedNotificationTimeFiber);
    }
}
