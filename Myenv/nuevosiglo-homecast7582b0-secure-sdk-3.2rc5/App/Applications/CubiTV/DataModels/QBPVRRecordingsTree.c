/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBPVRRecordingsTree.h"

#include <libintl.h>
#include <Logic/PVRLogic.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <Utils/QBEventUtils.h>
#include <Utils/QBNPvrUtils.h>
#include <SvFoundation/SvHashTable.h>
#include <NPvr/QBnPVRRecording.h>
#include <NPvr/QBnPVRDirectory.h>
#include <NPvr/QBnPVRProvider.h>
#include <SvCore/SvLocalTime.h>
#include <SvCore/SvLog.h>
#include <settings.h>
#include <main.h>

#define lli  long long int

#define log_debug(fmt, ...) do { if (1) SvLogNotice(          "[%s] " fmt, __func__, ##__VA_ARGS__); } while (0)

static const char* const nPVRRootNodeIdDefault = "NPvr";

static const char* const activeTreeNodeIdNPvrPrefix = "_npvr";

typedef void (*getStartEndTimeFromRecording)(SvGenericObject rec_, time_t *startTime, time_t* endTime);

SvLocal void
QBPVRRecordingsTreeNPvrDirectoryChanged(SvGenericObject self_, QBnPVRDirectory directory);

SvLocal inline void
getStartEndTimeFromNPvrRecording(SvGenericObject rec_, time_t *startTime, time_t *endTime)
{
    QBnPVRRecording rec = (QBnPVRRecording) rec_;

    *startTime = SvTimeGetSeconds(rec->startTime);
    *endTime = SvTimeGetSeconds(rec->endTime);
}

struct QBPVRRecordingsTree_s {
   struct SvObject_ super_;

   AppGlobals appGlobals;

   SvHashTable activeTreeNodeIdsToActiveArrays;
   SvHashTable recordingsToActiveTreeNodeIds;
   SvHashTable directoryIdsToActiveTreeNodeIds;
   SvHashTable activeTreeNodeIdsToDirectoryIds;

   SvString nPvrRootNodeId;

   SvString keywordIconPath;
   SvString repeatIconPath;
   SvString folderIconPath;
};

SvLocal void
QBPVRRecordingsTreeCorrectDirectoriesIndexes(QBPVRRecordingsTree self, QBActiveTreeNode node, SvString activeTreeNodeId, int newIdx)
{
    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    int oldIdx = QBActiveTreeNodeGetChildNodeIndex(parent, node);
    if (newIdx == oldIdx)
        return;

    SVRETAIN(node);
    QBActiveTreeRemoveSubTree(self->appGlobals->menuTree, node, NULL);
    int newIdx2 = newIdx > oldIdx ? newIdx - 1 : newIdx;
    QBActiveTreeInsertSubTree(self->appGlobals->menuTree, parent, node, newIdx2, NULL);

    QBActiveArray activeArray = (QBActiveArray) SvHashTableFind(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId);
    SvObject path = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, activeTreeNodeId);
    QBTreeProxy proxy = QBTreeProxyCreate((SvGenericObject) activeArray, NULL, NULL);
    QBActiveTreeMountSubTree(self->appGlobals->menuTree, (SvGenericObject) proxy, path, NULL);
    SVRELEASE(proxy);
    SVRELEASE(path);
    SVRELEASE(node);
}

SvLocal void
sprintfQuotaValue(char* buf, size_t buffSize, int64_t num, int64_t scaleNom, int64_t scaleDenom)
{
    if (num < 0) {
        snprintf(buf, buffSize,  "???");
        return;
    }

    num = num * 10 * scaleNom / scaleDenom;
    if (num % 10) {
        snprintf(buf, buffSize, "%lld.%d", (lli)num / 10, (int)(num % 10));
    } else {
        snprintf(buf, buffSize, "%lld", (lli)num / 10);
    }
}

SvLocal void
QBPVRRecordingsTreeNPVRDirectorySetNodeAttributes(QBPVRRecordingsTree self, QBActiveTreeNode node, QBnPVRDirectory directory)
{
    char usedStr[32], totalStr[32];

    QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) directory->name);

    SvString activeTreeNodeId = (SvString) SvHashTableFind(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directory->id);
    if (!activeTreeNodeId) {
        SvLogError("%s: directory with given id=[%s] not found", __func__, SvStringCString(directory->id));
        return;
    }

    QBActiveArray activeArray = (QBActiveArray) SvHashTableFind(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId);
    int count = QBActiveArrayCount(activeArray);

    SvString sub = NULL;
    if (directory->quota.maxSeconds > 0) {
        const char* header = gettext("quota used");
        const char* recText = ngettext("recording", "recordings", count);

        sprintfQuotaValue(usedStr, sizeof(usedStr), directory->quota.usedSeconds, 1, 60*60);
        sprintfQuotaValue(totalStr, sizeof(totalStr), directory->quota.maxSeconds, 1, 60*60);

        sub = SvStringCreateWithFormat("%i %s, %s : %s / %s hours", count, recText, header, usedStr, totalStr);
    } else {
        sub = QBNPvrUtilsCreateRecordingsNumberAndQuotaSubcaption(activeArray, true);
    }

    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject)sub);
    SVRELEASE(sub);

    if (directory->type == QBnPVRDirectoryType_Series) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("icon"), (SvGenericObject) self->repeatIconPath);
    } else if (directory->type == QBnPVRDirectoryType_Keyword) {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("icon"), (SvGenericObject) self->keywordIconPath);
    } else {
        QBActiveTreeNodeSetAttribute(node, SVSTRING("icon"), (SvGenericObject) self->folderIconPath);
    }

    QBActiveTreeNodeSetAttribute(node, SVSTRING("nPVRDirectory"), (SvGenericObject) directory);
}

SvLocal int
QBPVRRecordingsTreeDirectoryNodeNameCmp(SvObject directory1, SvObject directory2)
{
    return strcasecmp(SvStringCString(((QBnPVRDirectory) directory1)->name), SvStringCString(((QBnPVRDirectory) directory2)->name));
}

SvLocal int
QBPVRRecordingsTreeDirectoryCalculateIndex(QBPVRRecordingsTree self, SvObject directory)
{
    SvString dirTypeStr = SvStringCreate("nPVRDirectory", NULL);

    SvObject parentPath = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, self->nPvrRootNodeId);
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, parentPath, 0);

    int ret = 0;
    QBActiveTreeNode node = NULL;
    SvObject dir = NULL;
    bool foundItself = false;
    while ((node = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        dir = QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node, dirTypeStr);

        if (SvObjectEquals(dir, directory)) {
            foundItself = true;
            continue;
        }

        if (foundItself && QBPVRRecordingsTreeDirectoryNodeNameCmp(directory, dir) == 0) {
            break;
        }

        if (dir && !SvObjectEquals(dir, directory) && QBPVRRecordingsTreeDirectoryNodeNameCmp(directory, dir) < 0) {
            goto fini;
        }

        ret++;
    }

fini:
    SVRELEASE(parentPath);
    SVRELEASE(dirTypeStr);
    return ret;
}

SvLocal void
QBPVRRecordingsTreeCreateDirectory(QBPVRRecordingsTree self, SvGenericObject directory, SvString directoryId, SvString activeTreeNodeId)
{
    QBActiveTreeNode mainNode = NULL;

    QBActiveArray activeArray = QBActiveArrayCreate(13, NULL);
    QBTreeProxy proxy = QBTreeProxyCreate((SvGenericObject) activeArray, NULL, NULL);
    SvHashTableInsert(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId, (SvGenericObject) activeArray);
    SvHashTableInsert(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directoryId, (SvGenericObject) activeTreeNodeId);
    SvHashTableInsert(self->activeTreeNodeIdsToDirectoryIds, (SvGenericObject) activeTreeNodeId, (SvGenericObject) directoryId);

    int nodeIdx;
    QBActiveTreeNode parent_node = QBActiveTreeNodeCreate(activeTreeNodeId, NULL, NULL);
    mainNode = QBActiveTreeFindNode(self->appGlobals->menuTree, self->nPvrRootNodeId);
    nodeIdx = QBPVRRecordingsTreeDirectoryCalculateIndex(self, directory);
    QBPVRRecordingsTreeNPVRDirectorySetNodeAttributes(self, parent_node, (QBnPVRDirectory) directory);

    QBActiveTreeInsertSubTree(self->appGlobals->menuTree, mainNode, parent_node, nodeIdx, NULL);
    SVRELEASE(parent_node);

    SvObject path = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, activeTreeNodeId);
    QBActiveTreeMountSubTree(self->appGlobals->menuTree, (SvGenericObject) proxy, path, NULL);
    SVRELEASE(path);
    SVRELEASE(proxy);
    SVRELEASE(activeArray);
}

SvLocal void
QBPVRRecordingsTreeDeleteDirectory(QBPVRRecordingsTree self, SvString directoryId)
{
    SvString activeTreeNodeId = (SvString) SvHashTableFind(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directoryId);
    if (!activeTreeNodeId) {
        SvLogError("%s: no directory with given id=[%s] found", __func__, SvStringCString(directoryId));
        return;
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, activeTreeNodeId);
    if (!node) {
        SvLogError("%s: node not found in menu tree", __func__);
        return;
    }
    QBActiveTreeRemoveSubTree(self->appGlobals->menuTree, node, NULL);

    // as activeTreeNodeId might be stored only in hashtables below, it could eventually
    // be destroyed before iteration ends, triggering "invalid reads" inside loop
    SVRETAIN(activeTreeNodeId);
    SvHashTableRemove(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId);
    SvHashTableRemove(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directoryId);
    SvHashTableRemove(self->activeTreeNodeIdsToDirectoryIds, (SvGenericObject) activeTreeNodeId);

    SvGenericObject rec;
    SvHashTable tmpRecordingsToActiveTreeNodeIds = SvHashTableCreate(97, NULL);
    SvIterator it = SvHashTableKeysIterator(self->recordingsToActiveTreeNodeIds);
    while ((rec = SvIteratorGetNext(&it))) {
        SvGenericObject tmpActiveTreeNodeId = SvHashTableFind(self->recordingsToActiveTreeNodeIds, rec);
        if (SvObjectEquals(tmpActiveTreeNodeId, (SvObject) activeTreeNodeId)) {
            continue;
        }
        SvHashTableInsert(tmpRecordingsToActiveTreeNodeIds, rec, tmpActiveTreeNodeId);
    }
    SVRELEASE(activeTreeNodeId);
    SVRELEASE(self->recordingsToActiveTreeNodeIds);
    self->recordingsToActiveTreeNodeIds = tmpRecordingsToActiveTreeNodeIds;
}

SvLocal void
QBPVRRecordingsTreeUpdateDirectory(QBPVRRecordingsTree self, SvGenericObject directory, SvString directoryId, SvString activeTreeNodeId)
{
    if (!SvHashTableFind(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directoryId)) {
        SvLogError("%s: no directory with given id=[%s] found",  __func__, SvStringCString(directoryId));
        return;
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, activeTreeNodeId);
    if (!node) {
        SvLogError("%s: node not found in menu tree", __func__);
        return;
    }

    int newIdx = QBPVRRecordingsTreeDirectoryCalculateIndex(self, directory);
    QBPVRRecordingsTreeNPVRDirectorySetNodeAttributes(self, node, (QBnPVRDirectory) directory);


    QBPVRRecordingsTreeCorrectDirectoriesIndexes(self, node, activeTreeNodeId, newIdx);
    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, NULL);
}

SvLocal void
QBPVRRecordingsTreeInsertRecording(QBPVRRecordingsTree self, SvString activeTreeNodeId, SvGenericObject rec)
{
    QBActiveArray activeArray = (QBActiveArray) SvHashTableFind(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId);

    getStartEndTimeFromRecording getTimeFun = getStartEndTimeFromNPvrRecording;

    if (!getTimeFun) {
        SvLogError("%s: recording has unsupported object type", __func__);
        return;
    }

    time_t recStartTime, recEndTime, tmpStartTime, tmpEndTime;
    getTimeFun(rec, &recStartTime, &recEndTime);

    size_t idx, count = QBActiveArrayCount(activeArray);
    for (idx = 0; idx < count; ++idx) {
        SvGenericObject tmpObj = QBActiveArrayObjectAtIndex(activeArray, idx);
        getTimeFun(tmpObj, &tmpStartTime, &tmpEndTime);
        if (tmpStartTime - recStartTime < 0)
            break;
    }
    QBActiveArrayInsertObjectAtIndex(activeArray, idx, rec, NULL);
    SvHashTableInsert(self->recordingsToActiveTreeNodeIds, rec, (SvGenericObject) activeTreeNodeId);
}

SvLocal void
QBPVRRecordingsTreeRecordingAdded(QBPVRRecordingsTree self, SvGenericObject rec, SvString directoryId)
{
    if (QBnPVRProviderRecordingDirIsGlobal((QBnPVRRecording) rec)) {
        // It's handled by npvrAgent
        return;
    }

    if (SvHashTableFind(self->recordingsToActiveTreeNodeIds, rec)) {
        SvLogWarning("%s: recording is already on the list", __func__);
        return;
    }

    SvString activeTreeNodeId = (SvString) SvHashTableFind(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directoryId);
    if (!activeTreeNodeId) {
        SvLogError("%s: directory with given id=[%s] not found", __func__, SvStringCString(directoryId));
        return;
    }

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, activeTreeNodeId);
    if (!node) {
        SvLogError("%s: no activeTreeNode found for activeTreeNodeId=[%s]", __func__, SvStringCString(activeTreeNodeId));
        return;
    }

    QBPVRRecordingsTreeInsertRecording(self, activeTreeNodeId, rec);

    QBnPVRDirectory dir = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, directoryId);
    if (dir) {
        QBPVRRecordingsTreeNPvrDirectoryChanged((SvGenericObject) self, dir);
    }

    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, NULL);
}

SvLocal void
QBPVRRecordingsTreeRecordingRemoved(QBPVRRecordingsTree self, SvGenericObject rec)
{
    SvString activeTreeNodeId = (SvString) SvHashTableFind(self->recordingsToActiveTreeNodeIds, (SvGenericObject) rec);
    if (!activeTreeNodeId) {
        SvLogError("%s: recording not found", __func__);
        return;
    }

    log_debug("found in tree");

    QBActiveArray activeArray = (QBActiveArray) SvHashTableFind(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId);
    if (!activeArray) {
        SvLogError("%s: active array not found", __func__);
        return;
    }

    int idx = QBActiveArrayIndexOfObject(activeArray, (SvGenericObject) rec, NULL);
    if (idx < 0) {
        SvLogError("%s: recording not found in active array", __func__);
        return;
    }

    log_debug("removing");
    QBActiveArrayRemoveObjectAtIndex(activeArray, idx, NULL);
    SvHashTableRemove(self->recordingsToActiveTreeNodeIds, (SvGenericObject) rec);

    QBActiveTreeNode node = QBActiveTreeFindNode(self->appGlobals->menuTree, activeTreeNodeId);
    if( !node) {
        SvLogError("%s: menu tree node not found", __func__);
        return;
    }

    // we need to find old dir ID because we get globalFolder in the deleted recording from MW
    SvString oldDirId = (SvString) SvHashTableFind(self->activeTreeNodeIdsToDirectoryIds, (SvGenericObject) activeTreeNodeId);
    if (oldDirId) {
        log_debug("old dir found");
        QBnPVRDirectory dir = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, oldDirId);
        if (dir) {
            log_debug("refresh dir");
            QBPVRRecordingsTreeNPvrDirectoryChanged((SvGenericObject) self, dir);
        }
    } else {
        log_debug("it wasn't in any dir");
    }

    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, node, NULL);
}

SvLocal void
QBPVRRecordingsTreeRecordingChanged(QBPVRRecordingsTree self, SvObject rec, SvString directoryId)
{
    SvString activeTreeNodeId = (SvString) SvHashTableFind(self->recordingsToActiveTreeNodeIds, rec);
    if (!activeTreeNodeId) {
        // try to add
        QBPVRRecordingsTreeRecordingAdded(self, rec, directoryId);
        return;
    }

    QBActiveArray activeArray = (QBActiveArray) SvHashTableFind(self->activeTreeNodeIdsToActiveArrays, (SvObject) activeTreeNodeId);
    if (!activeArray) {
        SvLogError("%s: no active array found", __func__);
        return;
    }

    int idx = QBActiveArrayIndexOfObject(activeArray, rec, NULL);
    if (idx < 0) {
        SvLogError("%s: no such recording in given directory", __func__);
        return;
    }

    SvString oldDirId = (SvString) SvHashTableFind(self->activeTreeNodeIdsToDirectoryIds, (SvObject) activeTreeNodeId);

    if (!directoryId || !SvObjectEquals((SvObject) oldDirId, (SvObject) directoryId)) {
        // remove from old dir
        QBActiveArrayRemoveObjectAtIndex(activeArray, idx, NULL);
        SvHashTableRemove(self->recordingsToActiveTreeNodeIds, (SvObject) rec);

        QBnPVRDirectory dir = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, oldDirId);
        if (dir) {
            QBPVRRecordingsTreeNPvrDirectoryChanged((SvObject) self, dir);
        }

        if (SvObjectIsInstanceOf(rec, QBnPVRRecording_getType()) && !QBnPVRProviderRecordingDirIsGlobal((QBnPVRRecording) rec) && directoryId) {
            // add to the new dir

            activeTreeNodeId = (SvString) SvHashTableFind(self->directoryIdsToActiveTreeNodeIds, (SvObject) directoryId);
            if (!activeTreeNodeId) {
                SvLogError("%s: directory with given id=[%s] not found", __func__, SvStringCString(directoryId));
                return;
            }

            QBPVRRecordingsTreeInsertRecording(self, activeTreeNodeId, rec);
        }
    }

    if (directoryId) {
        QBnPVRDirectory dir = SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, getDirectoryById, directoryId);
        if (dir) {
            QBPVRRecordingsTreeNPvrDirectoryChanged((SvObject) self, dir);
        }
    }

    if (QBActiveArrayCount(activeArray) > 0) {
        QBActiveArrayPropagateObjectsChange(activeArray, idx, 1, NULL);
    }
}

SvLocal void
QBPVRRecordingsTreeNPvrRecordingAdded(SvGenericObject self_, QBnPVRRecording rec)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    QBPVRRecordingsTreeRecordingAdded(self, (SvGenericObject) rec, rec->directoryId);
}

SvLocal void
QBPVRRecordingsTreeNPvrRecordingRemoved(SvGenericObject self_, QBnPVRRecording rec)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    QBPVRRecordingsTreeRecordingRemoved(self, (SvGenericObject) rec);
}

SvLocal void
QBPVRRecordingsTreeNPvrRecordingChanged(SvGenericObject self_, QBnPVRRecording rec)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    QBPVRRecordingsTreeRecordingChanged(self, (SvGenericObject) rec, rec->directoryId);
}

SvLocal void
QBPVRRecordingsTreeNPvrDirectoryAdded(SvGenericObject self_, QBnPVRDirectory directory)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    SvString activeTreeNodeId = SvStringCreateWithFormat("%s:%s_", activeTreeNodeIdNPvrPrefix, SvStringCString(directory->id));
    QBPVRRecordingsTreeCreateDirectory(self, (SvGenericObject) directory, directory->id, activeTreeNodeId);
    SVRELEASE(activeTreeNodeId);
}

SvLocal void
QBPVRRecordingsTreeNPvrDirectoryChanged(SvGenericObject self_, QBnPVRDirectory directory)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    if (!directory || !directory->id) {   // global dir
        return;
    }

    SvString activeTreeNodeId = SvStringCreateWithFormat("%s:%s_", activeTreeNodeIdNPvrPrefix, SvStringCString(directory->id));
    QBPVRRecordingsTreeUpdateDirectory(self, (SvGenericObject) directory, directory->id, activeTreeNodeId);
    SVRELEASE(activeTreeNodeId);
}

SvLocal void
QBPVRRecordingsTreeNPvrDirectoryRemoved(SvGenericObject self_, QBnPVRDirectory directory)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    QBPVRRecordingsTreeDeleteDirectory(self, directory->id);
}

SvLocal void
QBPVRRecordingsTreeDestroy(void *self_)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    SVRELEASE(self->nPvrRootNodeId);
    SVRELEASE(self->recordingsToActiveTreeNodeIds);
    SVRELEASE(self->activeTreeNodeIdsToActiveArrays);
    SVRELEASE(self->directoryIdsToActiveTreeNodeIds);
    SVRELEASE(self->activeTreeNodeIdsToDirectoryIds);
    SVRELEASE(self->folderIconPath);
    SVRELEASE(self->keywordIconPath);
    SVRELEASE(self->repeatIconPath);
    if (self->appGlobals->nPVRProvider)
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, removeRecordingListener, (SvGenericObject) self);
}

SvType
QBPVRRecordingsTree_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPVRRecordingsTreeDestroy
    };

    // nPVR methods

    static const struct QBnPVRProviderRecordingListener_ nPVRRecordingListenerMethods = {
        .added   = QBPVRRecordingsTreeNPvrRecordingAdded,
        .changed = QBPVRRecordingsTreeNPvrRecordingChanged,
        .removed = QBPVRRecordingsTreeNPvrRecordingRemoved
    };

    static const struct QBnPVRProviderDirectoryListener_ nPVRDirectoryListenerMethods = {
        .added   = QBPVRRecordingsTreeNPvrDirectoryAdded,
        .changed = QBPVRRecordingsTreeNPvrDirectoryChanged,
        .removed = QBPVRRecordingsTreeNPvrDirectoryRemoved
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPVRRecordingsTree",
                            sizeof(struct QBPVRRecordingsTree_s),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBnPVRProviderRecordingListener_getInterface(), &nPVRRecordingListenerMethods,
                            QBnPVRProviderDirectoryListener_getInterface(), &nPVRDirectoryListenerMethods,
                            NULL);
    }

    return type;
}

QBPVRRecordingsTree
QBPVRRecordingsTreeCreate(AppGlobals appGlobals, SvString pvrRootNodeId, SvString nPvrRootNodeId)
{
    if (!appGlobals) {
        SvLogError("%s: NULL appGlobals passed", __func__);
        return NULL;
    }

    QBPVRRecordingsTree self = (QBPVRRecordingsTree) SvTypeAllocateInstance(QBPVRRecordingsTree_getType(), NULL);

    self->appGlobals = appGlobals;

    self->activeTreeNodeIdsToActiveArrays = SvHashTableCreate(97, NULL);
    self->recordingsToActiveTreeNodeIds = SvHashTableCreate(97, NULL);
    self->directoryIdsToActiveTreeNodeIds = SvHashTableCreate(97, NULL);
    self->activeTreeNodeIdsToDirectoryIds = SvHashTableCreate(97, NULL);

    self->nPvrRootNodeId = nPvrRootNodeId ? SVRETAIN(nPvrRootNodeId) : SvStringCreate(nPVRRootNodeIdDefault, NULL);

    svSettingsPushComponent("NPvr.settings");
    self->keywordIconPath = SvStringCreate(svSettingsGetString("FolderIconPaths", "keywordIconPath"), NULL);
    self->repeatIconPath = SvStringCreate(svSettingsGetString("FolderIconPaths", "repeatIconPath"), NULL);
    self->folderIconPath = SvStringCreate(svSettingsGetString("FolderIconPaths", "folderIconPath"), NULL);
    svSettingsPopComponent();

    if (self->appGlobals->nPVRProvider) {
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, addRecordingListener, (SvGenericObject) self);
        SvInvokeInterface(QBnPVRProvider, self->appGlobals->nPVRProvider, addDirectoryInfoListener, (SvGenericObject) self, NULL);
    }

    // PVR recordings and directories are still handled by the QBPVRRepeatedRecordings

    return self;
}

QBActiveArray
QBPVRRecordingsTreeGetRecordings(SvGenericObject self_, SvString directoryId)
{
    QBPVRRecordingsTree self = (QBPVRRecordingsTree) self_;

    SvString activeTreeNodeId = (SvString) SvHashTableFind(self->directoryIdsToActiveTreeNodeIds, (SvGenericObject) directoryId);
    if (!activeTreeNodeId)
        return NULL;

    return (QBActiveArray) SvHashTableFind(self->activeTreeNodeIdsToActiveArrays, (SvGenericObject) activeTreeNodeId);
}
