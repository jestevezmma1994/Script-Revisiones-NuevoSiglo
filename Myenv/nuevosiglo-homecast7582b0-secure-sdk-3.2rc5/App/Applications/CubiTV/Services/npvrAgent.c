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

#include "npvrAgent.h"

#include <main.h>
#include <NPvr/QBnPVRProvider.h>
#include <NPvr/QBnPVRRecording.h>
#include <Logic/NPvrLogic.h>
#include <Menus/menuchoice.h>
#include <Windows/mainmenu.h>
#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <fibers/c/fibers.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <XMB2/XMBMenuBar.h>
#include <Utils/appType.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

#define log_debug(fmt, ...)  do { if(0) SvLogNotice(COLBEG() "NPvrAgent :: " fmt  COLEND_COL(blue), ##__VA_ARGS__); } while (0)

#define NPVR_MENU_NODE_ID "NPvr"

struct QBNPvrAgent_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvEPGManager epgManager;
    QBActiveTree menuTree;
    SvGenericObject menuPath;

    SvHashTable revMap; /// NPvrRecording -> QBActiveArray

    QBActiveArray completedRecordings;
    QBActiveArray onGoingRecordings;
    QBActiveArray scheduledRecordings;
    QBActiveArray failedRecordings;
    QBActiveArray removedRecordings;

    SvFiber fiber;
    SvFiberTimer timer;
    SvWidget mainMenu; // main menu widget

    SvHashTable trackedEvents; //QBnPVRProviderRequest -> SvEPGEvent

    bool visitedNPvrMenu;
    unsigned int npvrMenuRefreshDelaySec;
};

SvLocal void
QBNPvrAgentNPvrMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    SvGenericObject rootPath = SvObjectCopy(nodePath_, NULL);
    SvInvokeInterface(QBTreePath, rootPath, truncate, 1);
    QBActiveTreeNode rootNode = QBActiveTreeFindNodeByPath(self->appGlobals->menuTree, rootPath);
    SvString nodeID = QBActiveTreeNodeGetID(rootNode);
    SVRELEASE(rootPath);

    if (nodeID && SvStringEqualToCString(nodeID, NPVR_MENU_NODE_ID)) {
        if (!self->visitedNPvrMenu) {
            self->visitedNPvrMenu = true;

            //refresh directories and all recordings
            SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectories);
            SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, refreshAllRecordings);

            //refresh quota
            CubiwareMWCustomerInfoRefresh(self->appGlobals->customerInfoMonitor);

            //next refresh is available after delay
            SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(self->npvrMenuRefreshDelaySec * 1000));
        }
    }
}

SvLocal void
QBNPvrAgentCustomerInfoMonitorInfoChanged(SvGenericObject self_, SvString customerId, SvHashTable customerInfo)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    if (!customerInfo)
        return;

    SvValue npvrMenuRefreshDelaySec = (SvValue) SvHashTableFind(customerInfo, (SvObject) SVSTRING("npvr_refresh_cycle"));
    if (npvrMenuRefreshDelaySec && SvObjectIsInstanceOf((SvObject) npvrMenuRefreshDelaySec, SvValue_getType()) && SvValueIsInteger(npvrMenuRefreshDelaySec)) {
        self->npvrMenuRefreshDelaySec = SvValueGetInteger(npvrMenuRefreshDelaySec);
    } else {
        SvLogWarning("%s() Expected <npvr_refresh_cycle> wasn't provided", __func__);
    }
}

SvLocal void
QBNPvrAgentCustomerInfoMonitorStatusChanged(SvGenericObject self_)
{
}

SvLocal void
QBNPvrAgentStep(void *self_)
{
    QBNPvrAgent self = self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (self->visitedNPvrMenu) {
        self->visitedNPvrMenu = false;
    }
}

SvLocal void
QBNPvrAgentUpdateItemsCount(QBNPvrAgent self, QBActiveArray list)
{
    const char *id = NULL;
    if (list == self->onGoingRecordings)
        id = "npvr-ongoing";

    if (list == self->completedRecordings)
        id = "npvr-completed";

    if (list == self->scheduledRecordings)
        id = "npvr-scheduled";

    if (list == self->failedRecordings)
        id = "npvr-failed";

    if (list == self->removedRecordings)
        id = "npvr-deleted";

    if (!id)
        return;

    QBActiveTreeNode node = NULL;
    SvString strId = SvStringCreate(id, NULL);
    node = QBActiveTreeFindNode(self->menuTree, strId);
    if (node) {
        QBActiveTreePropagateNodeChange(self->menuTree, node, NULL);
    }
    SVRELEASE(strId);
}

SvLocal size_t
find_object_in_array(QBActiveArray array,
                     const QBnPVRRecording rec)
{
    return QBActiveArrayIndexOfObjectIdenticalTo(array, (SvGenericObject) rec, NULL);
}

SvLocal bool
has_correct_place_in_array(QBActiveArray array,
                           const QBnPVRRecording rec)
{
    bool found = false;
    size_t i;
    size_t cnt = QBActiveArrayCount(array);

    for (i = 0; i < cnt; i++) {
        const QBnPVRRecording rec2 = (const QBnPVRRecording) QBActiveArrayObjectAtIndex(array, i);
        if (!found) {
            if (rec == rec2) {
                found = true;
                continue;
            }

            if (rec->state != QBnPVRRecordingState_scheduled) {
                if (SvTimeCmp(rec->startTime, rec2->startTime) > 0) {
                    return false;
                }
            } else {
                if (SvTimeCmp(rec->startTime, rec2->startTime) < 0) {
                    return false;
                }
            }
        } else {
            if (rec->state != QBnPVRRecordingState_scheduled) {
                if (SvTimeCmp(rec->startTime, rec2->startTime) < 0) {
                    return false;
                }
            } else {
                if (SvTimeCmp(rec->startTime, rec2->startTime) > 0) {
                    return false;
                }
            }
        }
    }

    return true;
}

SvLocal ssize_t
find_correct_place_in_array(QBActiveArray array,
                            const QBnPVRRecording rec)
{
    ssize_t i;
    ssize_t cnt = QBActiveArrayCount(array);

    for (i = 0; i < cnt; i++) {
        const QBnPVRRecording rec2 = (const QBnPVRRecording) QBActiveArrayObjectAtIndex(array, i);

        if (rec->state != QBnPVRRecordingState_scheduled) {
            if (SvTimeCmp(rec->startTime, rec2->startTime) >= 0) {
                return i;
            }
        } else {
            if (SvTimeCmp(rec->startTime, rec2->startTime) <= 0) {
                return i;
            }
        }
    }
    return -1;
}


SvLocal QBActiveArray
QBNPvrAgentGetArrayFromState(QBNPvrAgent self, QBnPVRRecordingState state)
{
    if (state == QBnPVRRecordingState_active)
        return self->onGoingRecordings;

    if (state == QBnPVRRecordingState_completed)
        return self->completedRecordings;

    if (state == QBnPVRRecordingState_scheduled)
        return self->scheduledRecordings;

    if (state == QBnPVRRecordingState_failed)
        return self->failedRecordings;

    if (state == QBnPVRRecordingState_removed)
        return self->removedRecordings;

    return NULL;
}

SvLocal void
QBNPvrAgentRecordingStateChanged(QBNPvrAgent self, QBnPVRRecording rec, QBActiveArray oldArray, QBActiveArray newArray)
{
    log_debug("self->onGoingRecordings %p", self->onGoingRecordings);
    log_debug("self->completedRecordings %p", self->completedRecordings);
    log_debug("self->scheduledRecordings %p", self->scheduledRecordings);
    log_debug("self->failedRecordings %p", self->failedRecordings);
    log_debug("self->removedRecordings %p", self->removedRecordings);

    log_debug("oldArray %p , newArray %p", oldArray, newArray);

    size_t oldPos = -1;
    if (oldArray) {
        oldPos = find_object_in_array(oldArray, rec);
    }

    if (!QBnPVRProviderRecordingDirIsGlobal(rec)) {
        log_debug("It's not global dir now");
        if ((oldArray == self->completedRecordings) || (newArray == self->completedRecordings) ||
            (oldArray == self->scheduledRecordings) || (newArray == self->scheduledRecordings)) {
            log_debug("so we will not add it anywhere");
            // completed recordings with directory do not show in the "Completed recordings"
            newArray = NULL;
        }
    }

    if (oldArray == newArray) {
        if (!oldArray) {
            return;
        }

        if (has_correct_place_in_array(oldArray, rec)) {
            log_debug("rec in place - just update");
            QBActiveArrayPropagateObjectsChange(oldArray, oldPos, 1, NULL);
            return;
        }
    }

    if (oldArray) {
        log_debug("remove from old dir");
        SvHashTableRemove(self->revMap, (SvGenericObject)rec);
        QBActiveArrayRemoveObjectAtIndex(oldArray, oldPos, NULL);

        QBNPvrAgentUpdateItemsCount(self, oldArray);
    }

    if (newArray) {
        log_debug("new folder");

        ssize_t newPos = find_correct_place_in_array(newArray, rec);
        if (newPos >= 0) {
            QBActiveArrayInsertObjectAtIndex(newArray, newPos, (SvGenericObject)rec, NULL);
        } else {
            QBActiveArrayAddObject(newArray, (SvGenericObject)rec, NULL);
        }

        SvHashTableInsert(self->revMap, (SvGenericObject)rec, (SvGenericObject)newArray);

        QBNPvrAgentUpdateItemsCount(self, newArray);
    }
}

SvLocal void
QBNPvrAgentRecordingRemoved(SvGenericObject self_, QBnPVRRecording rec)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    QBActiveArray oldArray = (QBActiveArray) SvHashTableFind(self->revMap, (SvGenericObject)rec);

    if (QBNPvrLogicIsRecoveryFeatureEnabled() && (QBActiveArrayIndexOfObjectIdenticalTo(self->removedRecordings, (SvGenericObject) rec, NULL) == -1) &&
        (rec->deleteReason == QBnPVRRecordingDeleteReason_DeleteOldest || rec->deleteReason == QBnPVRRecordingDeleteReason_Manual)) {
        QBNPvrAgentRecordingStateChanged(self, rec, NULL, self->removedRecordings);
    } else {
        QBNPvrAgentRecordingStateChanged(self, rec, oldArray, NULL);
    }
}

SvLocal void
QBNPvrAgentRecordingChanged(SvGenericObject self_, QBnPVRRecording rec)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    QBActiveArray oldArray = (QBActiveArray) SvHashTableFind(self->revMap, (SvGenericObject)rec);
    QBActiveArray newArray = QBNPvrAgentGetArrayFromState(self, rec->state);
    QBNPvrAgentRecordingStateChanged(self, rec, oldArray, newArray);
}

SvLocal void
QBNPvrAgentRecordingAdded(SvGenericObject self_, QBnPVRRecording rec)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    QBActiveArray newArray = QBNPvrAgentGetArrayFromState(self, rec->state);
    QBNPvrAgentRecordingStateChanged(self, rec, NULL, newArray);
}

SvLocal void
QBNPvrAgentRequestStateChanged(SvGenericObject self_, QBnPVRProviderRequest request)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    if (QBnPVRProviderRequestGetException(request) == QBnPVRProviderRequestException_eventNotFound) {
        SvTimeRange timeRange;
        SvEPGEvent trackedEvent = (SvEPGEvent) SvHashTableFind(self->trackedEvents, (SvGenericObject)request);
        SvTimeRangeInit(&timeRange, trackedEvent->startTime, trackedEvent->endTime);
        SvEPGDataWindow epgWindow = SvEPGManagerCreateDataWindow(self->epgManager, NULL);
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        SvGenericObject master = QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels"));
        SvEPGDataWindowSetChannelView(epgWindow, (SvGenericObject) master, NULL);
        SvEPGDataWindowSetChannelsRange(epgWindow, 0, 0, NULL);
        SvEPGDataWindowSetTimeRange(epgWindow, timeRange, NULL);
        SvEPGDataWindowInvalidateEvents(epgWindow, NULL);
        SVRELEASE(epgWindow);
    }

    SvHashTableRemove(self->trackedEvents, (SvGenericObject)request);
    QBnPVRProviderRequestRemoveListener(request, (SvGenericObject) self);
}

SvLocal void
QBNPvrAgent__dtor__(void *self_)
{
    QBNPvrAgent self = (QBNPvrAgent) self_;

    SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, removeRecordingListener, (SvGenericObject) self);

    SVRELEASE(self->completedRecordings);
    SVRELEASE(self->onGoingRecordings);
    SVRELEASE(self->scheduledRecordings);
    SVRELEASE(self->failedRecordings);
    SVTESTRELEASE(self->removedRecordings);

    SVRELEASE(self->revMap);
    SVRELEASE(self->trackedEvents);

    SVTESTRELEASE(self->menuTree);
    SVTESTRELEASE(self->menuPath);
    SVTESTRELEASE(self->epgManager);

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }
}

SvLocal SvType
QBNPvrAgent_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNPvrAgent__dtor__
    };
    static SvType type = NULL;

    static const struct QBnPVRProviderRecordingListener_ nPVRRecordingMethods = {
        .added = QBNPvrAgentRecordingAdded,
        .changed = QBNPvrAgentRecordingChanged,
        .removed = QBNPvrAgentRecordingRemoved
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBNPvrAgentNPvrMenuChoosen
    };

    static const struct CubiwareMWCustomerInfoListener_ customerInfoMethods = {
        .customerInfoChanged = QBNPvrAgentCustomerInfoMonitorInfoChanged,
        .statusChanged = QBNPvrAgentCustomerInfoMonitorStatusChanged
    };

    static const struct QBnPVRProviderRequestListener_ nPVRRequestMethods = {
        .stateChanged = QBNPvrAgentRequestStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrAgent",
                            sizeof(struct QBNPvrAgent_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBnPVRProviderRecordingListener_getInterface(), &nPVRRecordingMethods,
                            QBMenuChoice_getInterface(), &menuMethods,
                            CubiwareMWCustomerInfoListener_getInterface(), &customerInfoMethods,
                            QBnPVRProviderRequestListener_getInterface(), &nPVRRequestMethods,
                            NULL);
    }
    return type;
}

QBNPvrAgent
QBNPvrAgentCreate(AppGlobals appGlobals, unsigned int npvrMenuRefreshDelaySec)
{
    if (!appGlobals->nPVRProvider) {
        if (QBAppTypeIsDVB()) {
            SvLogNotice("%s(): don't create agent for DVB mode", __func__);
        } else {
            SvLogError("%s() : npvrManager not created!", __func__);
        }

        return NULL;
    };

    QBNPvrAgent self = (QBNPvrAgent) SvTypeAllocateInstance(QBNPvrAgent_getType(), NULL);

    self->appGlobals = appGlobals;
    self->menuTree = NULL;
    self->menuPath = NULL;
    self->mainMenu = NULL;

    self->completedRecordings = QBActiveArrayCreate(97, NULL);
    self->onGoingRecordings = QBActiveArrayCreate(11, NULL);
    self->scheduledRecordings = QBActiveArrayCreate(97, NULL);
    self->failedRecordings = QBActiveArrayCreate(97, NULL);
    if (QBNPvrLogicIsRecoveryFeatureEnabled())
        self->removedRecordings = QBActiveArrayCreate(97, NULL);

    self->revMap = SvHashTableCreate(97, NULL);
    self->trackedEvents = SvHashTableCreate(11, NULL);

    if (npvrMenuRefreshDelaySec != 0) {
        self->npvrMenuRefreshDelaySec = npvrMenuRefreshDelaySec;
    } else {
        //disable NPvr menu refresh
        self->visitedNPvrMenu = true;
    }

    return self;
}


QBActiveArray
QBNPvrAgentGetCompletedRecordingsList(QBNPvrAgent self)
{
    return self->completedRecordings;
}

QBActiveArray
QBNPvrAgentGetScheduledRecordingsList(QBNPvrAgent self)
{
    return self->scheduledRecordings;
}

QBActiveArray
QBNPvrAgentGetOnGoingRecordingsList(QBNPvrAgent self)
{
    return self->onGoingRecordings;
}

QBActiveArray
QBNPvrAgentGetFailedRecordingsList(QBNPvrAgent self)
{
    return self->failedRecordings;
}

QBActiveArray
QBNPvrAgentGetRemovedRecordingsList(QBNPvrAgent self)
{
    return self->removedRecordings;
}

void
QBNPvrAgentStart(QBNPvrAgent self,
                 QBActiveTree menuTree,
                 SvGenericObject menuPath)

{
    SVTESTRELEASE(self->epgManager);
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    SVTESTRELEASE(self->menuTree);
    self->menuTree = SVTESTRETAIN(menuTree);
    SVTESTRELEASE(self->menuPath);
    self->menuPath = SVTESTRETAIN(menuPath);
    self->mainMenu = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);

    if (self->mainMenu) {
        XMBMenuBarLogFirstMoveFilterAdd(self->mainMenu, SVSTRING(NPVR_MENU_NODE_ID));
    }

    if (menuTree) {
        QBNPvrAgentUpdateItemsCount(self, self->completedRecordings);
        QBNPvrAgentUpdateItemsCount(self, self->onGoingRecordings);
        QBNPvrAgentUpdateItemsCount(self, self->scheduledRecordings);
        QBNPvrAgentUpdateItemsCount(self, self->failedRecordings);
        QBNPvrAgentUpdateItemsCount(self, self->removedRecordings);
    }

    if (self->appGlobals->nPVRProvider) {
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, addRecordingListener, (SvGenericObject) self);
    }

    SvInvokeInterface(QBMainMenu, self->appGlobals->main, addGlobalHandler, (SvObject) self);

    if (self->appGlobals->customerInfoMonitor) {
        CubiwareMWCustomerInfoAddListener(self->appGlobals->customerInfoMonitor, (SvGenericObject) self);
    }

    if (self->npvrMenuRefreshDelaySec != 0 && !self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBNPvrAgent", QBNPvrAgentStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
    }
}

void
QBNPvrAgentStop(QBNPvrAgent self)
{
    if (self->appGlobals->nPVRProvider) {
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, removeRecordingListener, (SvGenericObject) self);
    }

    if (self->appGlobals->customerInfoMonitor) {
        CubiwareMWCustomerInfoRemoveListener(self->appGlobals->customerInfoMonitor, (SvGenericObject) self);
    }

    if (self->mainMenu) {
        XMBMenuBarLogFirstMoveFilterRemove(self->mainMenu, SVSTRING(NPVR_MENU_NODE_ID));
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }
}

extern void
QBNPvrAgentTrackRequest(QBNPvrAgent self, QBnPVRProviderRequest request, SvEPGEvent event)
{
    if (!self || !request || !event) {
        return;
    }

    SvHashTableInsert(self->trackedEvents, (SvGenericObject)request, (SvGenericObject)event);
    QBnPVRProviderRequestAddListener(request, (SvGenericObject)self);
}
