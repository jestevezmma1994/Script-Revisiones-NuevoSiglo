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

#include "QBConaxMailManager.h"

#include <main.h>
#include <QBSmartcard2Interface.h>
#include <QBICSmartcardMail.h>
#include <QBCAS.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>

#include <fibers/c/fibers.h>
#include <time.h>

#define MAIL_NODE_ATTRIBUTE_ID SVSTRING("conaxMail:obj")
#define NO_MESSAGES_NODE_ATTRIBUTE_ID SVSTRING("conaxMail:noMessages")
#define SCHEDULED_MAIL_CHECK_PERIOD 1000

#define log_state(fmt, ...) do { if (1) { SvLogNotice("ConaxMailManager : " fmt, ## __VA_ARGS__); } } while (0)
#define log_debug(fmt, ...) do { if (0) { SvLogNotice("ConaxMailManager : " fmt, ## __VA_ARGS__); } } while (0)
#define log_error(fmt, ...) do { if (1) { SvLogError("ConaxMailManager : %s " fmt, __func__, ## __VA_ARGS__); } } while (0)

SvInterface
QBConaxMailManagerListener_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBConaxMailManagerListener",
                                 sizeof(struct QBConaxMailManagerListener_),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

struct QBConaxMailManager_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    size_t mailCnt;
    size_t unreadMailCnt;
    SvWeakList listeners;

    /// for scheduled mails.
    SvArray scheduledMail;
    SvFiber fiber;
    SvFiberTimer timer;

    bool isStarted;
    bool isProperTime;

    QBActiveTreeNode mailNode;
    SvObject mailPath;
};

SvLocal void
QBConaxMailManagerMainMenuChangeNotify(QBConaxMailManager self)
{
    // notify mail node change
    QBActiveTreePropagateNodeChange(self->appGlobals->menuTree,
                                    self->mailNode,
                                    NULL);

    // notify mail node children change
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel,
                                            self->appGlobals->menuTree,
                                            getIterator, self->mailPath, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(self->appGlobals->menuTree,
                                     self->mailPath,
                                     0,
                                     nodesCount,
                                     NULL);
}

SvLocal void
QBConaxMailManagerNotifyMailChanged(QBConaxMailManager self)
{
    SvArray listenersArray = SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvArrayGetIterator(listenersArray);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBConaxMailManagerListener, listener, mailChanged, self->mailCnt, self->unreadMailCnt);
    }
    SVRELEASE(listenersArray);
    QBConaxMailManagerMainMenuChangeNotify(self);
}

SvLocal void
QBConaxMailManagerDestroy(void *self_)
{
    QBConaxMailManager self = (QBConaxMailManager) self_;

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }

    SVRELEASE(self->listeners);
    SVRELEASE(self->scheduledMail);

    SVTESTRELEASE(self->mailNode);
    SVTESTRELEASE(self->mailPath);
}

SvLocal void
QBConaxMailManagerTimeUpdated(SvObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
    QBConaxMailManager self = (QBConaxMailManager) self_;
    log_debug("TimeUpdated, firstTime: %d, self->isStarted %d, self->isProperTime %d, scheduledMailCnt %zd",
              firstTime, self->isStarted, self->isProperTime, SvArrayCount(self->scheduledMail));

    if (!self->isStarted)
        return;

    if (self->isProperTime)
        return;

    self->isProperTime = true;
    if (SvArrayCount(self->scheduledMail) != 0) {
        log_debug("TimeUpdated: activating fiber");
        SvFiberActivate(self->fiber);
    }
}

SvLocal SvType
QBConaxMailManager_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxMailManagerDestroy
    };
    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = QBConaxMailManagerTimeUpdated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxMailManager",
                            sizeof(struct QBConaxMailManager_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            NULL);
    }
    return type;
}

SvLocal void
QBConaxMailManagerRemoveNode(QBActiveTreeNode node)
{
    QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
}

SvLocal void
QBConaxMailManagerAddMailNode(QBConaxMailManager self, QBICSmartcardMail *mail)
{
    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    QBActiveTreeNodeSetAttribute(node, MAIL_NODE_ATTRIBUTE_ID, (SvObject) mail);
    QBActiveTreeNodeAddSubTree(self->mailNode, node, NULL);
    SVRELEASE(node);
}

SvLocal void
QBConaxMailManagerAddNoMessagesNode(QBConaxMailManager self)
{
    QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);
    SvValue trueVal = SvValueCreateWithBoolean(true, NULL);
    QBActiveTreeNodeSetAttribute(node, NO_MESSAGES_NODE_ATTRIBUTE_ID, (SvObject) trueVal);
    SVRELEASE(trueVal);
    QBActiveTreeNodeAddSubTree(self->mailNode, node, NULL);
    SVRELEASE(node);
}

SvLocal QBActiveTreeNode
QBConaxMailManagerGetFirstChild(QBActiveTreeNode parentNode)
{
    SvIterator iter = QBActiveTreeNodeChildNodesIterator(parentNode);
    return (QBActiveTreeNode) SvIteratorGetNext(&iter);
}

SvLocal void
QBConaxMailRemoveNoMessagesNode(QBConaxMailManager self)
{
    QBActiveTreeNode firstNode = QBConaxMailManagerGetFirstChild(self->mailNode);
    if (firstNode && QBConaxMailManagerIsNodeNoMessages(firstNode)) {
        QBConaxMailManagerRemoveNode(firstNode);
    }
}

SvLocal void
QBConaxMailManagerHandleNewMail(QBConaxMailManager self, QBICSmartcardMail *mail)
{
    ++self->mailCnt;
    if (!mail->is_open) {
        ++self->unreadMailCnt;
    }

    QBConaxMailManagerAddMailNode(self, mail);
    QBConaxMailRemoveNoMessagesNode(self);
}

SvLocal void
QBConaxMailManagerStep(void *self_)
{
    QBConaxMailManager self = (QBConaxMailManager) self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    time_t now = SvTimeNow();
    int cnt = SvArrayCount(self->scheduledMail);
    for (int i = cnt - 1; i >= 0; --i) {
        QBICSmartcardMail *mail = (QBICSmartcardMail *) SvArrayAt(self->scheduledMail, i);
        if (mail->display_start > now)
            continue;

        log_state("mail at slot %d can now be added to menu", mail->slot);

        SVRETAIN(mail);
        SvArrayRemoveObjectAtIndex(self->scheduledMail, i);
        QBConaxMailManagerHandleNewMail(self, mail);
        QBConaxMailManagerNotifyMailChanged(self);

        SVRELEASE(mail);
    }

    if (SvArrayCount(self->scheduledMail) != 0)
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(SCHEDULED_MAIL_CHECK_PERIOD));
}

SvLocal void
QBConaxMailManagerRemoveOldMail(QBConaxMailManager self, int slot)
{
    /// remove scheduled mail with same slot number, if present.
    int cnt = SvArrayCount(self->scheduledMail);
    for (int i = 0; i < cnt; ++i) {
        QBICSmartcardMail *mail = (QBICSmartcardMail *) SvArrayAt(self->scheduledMail, i);
        if (mail->slot != slot)
            continue;

        log_state("scheduled mail at slot %d is supposed be removed", slot);
        SvArrayRemoveObjectAtIndex(self->scheduledMail, i);
        return;
    }

    /// remove mail node from menu
    if (self->mailCnt > 0) {
        SvIterator iter = QBActiveTreeNodeChildNodesIterator(self->mailNode);
        QBActiveTreeNode node = NULL;
        while ((node = (QBActiveTreeNode) SvIteratorGetNext(&iter))) {
            QBICSmartcardMail *mail = (QBICSmartcardMail *) QBConaxMailManagerGetMailFromNode(node);
            if (mail->slot != slot)
                continue;

            log_state("mail at slot %d is supposed be removed from menu", slot);
            if (!mail->is_open)
                --self->unreadMailCnt;
            --self->mailCnt;

            // create no messages empty node
            if (self->mailCnt == 0)
                QBConaxMailManagerAddNoMessagesNode(self);

            QBConaxMailManagerRemoveNode(node);
            break;
        }
    }
}

SvLocal void
QBConaxMailManagerHandleMail(QBConaxMailManager self, int slot, QBICSmartcardMail *mail)
{
    QBConaxMailManagerRemoveOldMail(self, slot);

    if (!mail)
        goto out;

    /// schedule mail if it should be displayed in the future or time is not yet set
    if (mail->display_start != 0) {
        time_t now = SvTimeNow();
        if (self->isProperTime) {
            log_state("mail at slot %d should be shown after %lu seconds", mail->slot, mail->display_start - now);
        } else {
            struct tm *timeinfo;
            timeinfo = localtime(&mail->display_start);
            char buf[256];
            strftime(buf, 256, "%c", timeinfo);
            log_state("mail at slot %d should be shown at %s", mail->slot, buf);
        }
        if (mail->display_start >= now || !self->isProperTime) {
            SvArrayAddObject(self->scheduledMail, (SvObject) mail);
            if (self->isProperTime &&
                !SvFiberEventIsArmed(self->timer)) {
                SvFiberTimerActivateAfter(self->timer, SvTimeFromMs(SCHEDULED_MAIL_CHECK_PERIOD));
            }
            return;
        }
    }

    log_state("mail at slot %d should be added to menu immediately", slot);
    QBConaxMailManagerHandleNewMail(self, mail);
out:
    QBConaxMailManagerNotifyMailChanged(self);
}

SvLocal void
QBConaxMailManagerMailCallback(void *self_, SvObject mail_)
{
    QBConaxMailManager self = (QBConaxMailManager) self_;
    QBICSmartcardMail *mail = (QBICSmartcardMail *) mail_;

    QBConaxMailManagerHandleMail(self, mail->slot, mail);
}

SvLocal void
QBConaxMailManagerMessageRemovedCallback(void* self_, int slot)
{
    QBConaxMailManager self = (QBConaxMailManager) self_;

    QBConaxMailManagerHandleMail(self, slot, NULL);
}

static const struct QBICSmartcardCallbacks_s QBConaxMailManagerSmartcardCallbacks = {
    .mail            = &QBConaxMailManagerMailCallback,
    .message_removed = &QBConaxMailManagerMessageRemovedCallback,
};

QBConaxMailManager
QBConaxMailManagerCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    if (!SvObjectIsImplementationOf(QBCASGetInstance(), QBSmartcard2Interface_getInterface())) {
        log_error("Conax Mail Manager should be only created for Conax CAS type");
        return NULL;
    }

    SvErrorInfo error = NULL;
    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL appGlobals passed");
        goto err1;
    }

    QBConaxMailManager self = (QBConaxMailManager) SvTypeAllocateInstance(QBConaxMailManager_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate CCInfoBar");
        goto err1;
    }

    self->listeners = SvWeakListCreate(&error);
    if (!self->listeners) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "SvWeakListCreate() failed");
        goto err2;
    }

    self->scheduledMail = SvArrayCreate(&error);
    if (!self->scheduledMail) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "SvArrayCreate() failed");
        goto err2;
    }

    self->fiber = SvFiberCreate(NULL, NULL, "QBConaxMailManager", &QBConaxMailManagerStep, self);
    if (!self->fiber) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvFiberCreate() failed");
        goto err2;
    }
    self->timer = SvFiberTimerCreate(self->fiber);
    if (!self->timer) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvFiberTimerCreate() failed");
        goto err2;
    }
    self->appGlobals = appGlobals;

    return self;

err2:
    SVRELEASE(self);
err1:
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBConaxMailManagerStart(QBConaxMailManager self)
{
    if (self->isStarted) {
        log_error("QBConaxMailManager already started");
        return;
    }

    if (!(self->mailNode = QBActiveTreeFindNode(self->appGlobals->menuTree, SVSTRING("ConaxMail")))) {
        log_error("ConaxMail node not found, mails not supported");
        return;
    }
    SVRETAIN(self->mailNode);
    self->mailPath = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, SVSTRING("ConaxMail"));

    self->isStarted = true;
    if (!self->mailCnt) {
        QBConaxMailManagerAddNoMessagesNode(self);
    }

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);

    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), addConaxCallbacks, &QBConaxMailManagerSmartcardCallbacks, self, "QBConaxMailManager");
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), reportAllMails);
}

void
QBConaxMailManagerStop(QBConaxMailManager self)
{
    if (!self->isStarted) {
        log_error("QBConaxMailManager not started");
        return;
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
    }

    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), removeConaxCallbacks, &QBConaxMailManagerSmartcardCallbacks, self);
    self->isStarted = false;
}

void
QBConaxMailManagerAddListener(QBConaxMailManager self, SvObject listener)
{
    if (!SvObjectIsImplementationOf(listener, QBConaxMailManagerListener_getInterface())) {
        log_error("given listener doesn't implement QBConaxMailManagerListener interface");
        return;
    }

    SvWeakListPushFront(self->listeners, listener, NULL);
}

void
QBConaxMailManagerMarkMailOpened(QBConaxMailManager self, SvObject mail_)
{
    QBICSmartcardMail *mail = (QBICSmartcardMail *) mail_;
    if (mail->is_open) {
        log_state("%s(): mail from slot %d is already opened", __func__, mail->slot);
        return;
    }
    --self->unreadMailCnt;
    SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), markMail, mail);
    QBConaxMailManagerNotifyMailChanged(self);
}

bool
QBConaxMailManagerIsMailOpened(QBConaxMailManager self, SvObject mail_)
{
    QBICSmartcardMail *mail = (QBICSmartcardMail *) mail_;
    return mail->is_open;
}

SvString
QBConaxMailManagerGetMailText(QBConaxMailManager self, SvObject mail_)
{
    QBICSmartcardMail *mail = (QBICSmartcardMail *) mail_;
    return mail->text;
}

int
QBConaxMailManagerGetMailSlot(QBConaxMailManager self, SvObject mail_)
{
    QBICSmartcardMail *mail = (QBICSmartcardMail *) mail_;
    return mail->slot;
}

size_t
QBConaxMailManagerGetUnreadMailCount(QBConaxMailManager self)
{
    return self->unreadMailCnt;
}

SvObject
QBConaxMailManagerGetMailFromNode(QBActiveTreeNode node)
{
    return QBActiveTreeNodeGetAttribute(node, MAIL_NODE_ATTRIBUTE_ID);
}

bool
QBConaxMailManagerIsNodeNoMessages(QBActiveTreeNode node)
{
    SvObject noMessages = QBActiveTreeNodeGetAttribute(node, NO_MESSAGES_NODE_ATTRIBUTE_ID);
    return noMessages &&
           SvObjectIsInstanceOf(noMessages, SvValue_getType()) &&
           SvValueGetType((SvValue) noMessages) == SvValueType_boolean &&
           SvValueGetBoolean((SvValue) noMessages) == true;
}
