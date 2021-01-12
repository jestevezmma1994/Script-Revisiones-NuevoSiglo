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

#include "QBSubtreeNotifier.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBDataModel3/QBTreePath.h>
#include <Menus/menuchoice.h>

struct QBSubtreeNotifier_t {
    struct SvObject_ super_;

    QBTreePathMap pathMap;
    SvWeakList currentSubtreeListeners;
    SvGenericObject currentPath;
};


SvLocal void
QBSubtreeNotifier__dtor__(void *self_)
{
    QBSubtreeNotifier self = self_;

    SVTESTRELEASE(self->pathMap);
    SVTESTRELEASE(self->currentSubtreeListeners);
    SVTESTRELEASE(self->currentPath);
}


SvLocal void
QBSubtreeNotifierMenuChosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBSubtreeNotifier self = (QBSubtreeNotifier) self_;
    SvWeakList newSubtreeListeners;
    SvGenericObject parentPath = SvObjectCopy(nodePath_, NULL);
    SvInvokeInterface(QBTreePath, parentPath, truncate, -1);

    if (SvObjectEquals(self->currentPath, parentPath)) {
        SVTESTRELEASE(parentPath);
        return;
    }
    SVTESTRELEASE(self->currentPath);
    self->currentPath = parentPath;

    newSubtreeListeners = (SvWeakList) QBTreePathMapFind(self->pathMap, self->currentPath);

    if (newSubtreeListeners == self->currentSubtreeListeners)
        return;

    if (self->currentSubtreeListeners) {
        SvArray currentSubtreeListenersArray = SvWeakListCreateElementsList(self->currentSubtreeListeners, NULL);
        SvIterator listenerIter = SvArrayGetIterator(currentSubtreeListenersArray);
        SvObject curListener = NULL;
        while ((curListener = SvIteratorGetNext(&listenerIter)) != NULL) {
            SvInvokeInterface(QBSubtreeNotifierListener, curListener, subtreeLeft);
        }
        SVRELEASE(currentSubtreeListenersArray);
        SVRELEASE(self->currentSubtreeListeners);
        self->currentSubtreeListeners = NULL;
    }

    if (newSubtreeListeners) {
        self->currentSubtreeListeners = SVRETAIN(newSubtreeListeners);
        SvArray currentSubtreeListenersArray = SvWeakListCreateElementsList(self->currentSubtreeListeners, NULL);
        SvIterator listenerIter = SvArrayGetIterator(currentSubtreeListenersArray);
        SvObject curListener = NULL;
        while ((curListener = SvIteratorGetNext(&listenerIter)) != NULL) {
            SvInvokeInterface(QBSubtreeNotifierListener, curListener, subtreeEntered);
        }
        SVRELEASE(currentSubtreeListenersArray);
    }
}





SvLocal SvType
QBSubtreeNotifier_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSubtreeNotifier__dtor__
    };
    static SvType type = NULL;

    static const struct QBMenuChoice_t menuChoiceMethods = {
        .choosen = QBSubtreeNotifierMenuChosen,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSubtreeNotifier",
                            sizeof(struct QBSubtreeNotifier_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuChoiceMethods, NULL);
    }

    return type;
}



QBSubtreeNotifier
QBSubtreeNotifierCreate(void)
{
    QBSubtreeNotifier self = (QBSubtreeNotifier) SvTypeAllocateInstance(QBSubtreeNotifier_getType(), NULL);

    self->pathMap = QBTreePathMapCreate(NULL);

    return self;
}

int
QBSubtreeNotifierAddListener(QBSubtreeNotifier self, SvGenericObject path, SvGenericObject listener)
{
    if (!self || !listener)
        return -1;

    SvWeakList listeners = SVTESTRETAIN((SvWeakList) QBTreePathMapFind(self->pathMap, path));
    if (!listeners) {
        listeners = SvWeakListCreate(NULL);
        QBTreePathMapInsert(self->pathMap, path, (SvObject) listeners, NULL);
    }

    bool alreadyHaveListener = false;
    SvArray listenersArray = SvWeakListCreateElementsList(listeners, NULL);
    ssize_t idx = SvArrayIndexOfObject(listenersArray, listener);
    if (idx >= 0) {
        alreadyHaveListener = true;
    }
    SVRELEASE(listenersArray);

    if (!alreadyHaveListener) {
        SvWeakListPushBack(listeners, listener, NULL);
    }
    SVRELEASE(listeners);
    return 0;
}

int
QBSubtreeNotifierRemoveListener(QBSubtreeNotifier self, SvGenericObject path, SvGenericObject listener)
{
    if (!self)
        return -1;

    SvWeakList listeners = (SvWeakList) QBTreePathMapFind(self->pathMap, path);
    if (!listeners) {
        return -1;
    }

    if (SvWeakListRemoveObject(listeners, listener)) {
        if (SvWeakListCount(listeners) == 0) {
            QBTreePathMapRemove(self->pathMap, path, NULL);
            if (listeners == self->currentSubtreeListeners) {
                SVRELEASE(self->currentSubtreeListeners);
                self->currentSubtreeListeners = NULL;
            }
        }
    } else {
        return -1;
    }
    return 0;
}




extern SvInterface
QBSubtreeNotifierListener_getInterface(void)
{
    static SvInterface iface = NULL;

    if (unlikely(!iface)) {
        SvInterfaceCreateManaged("QBSubtreeNotifierListener",
                                 sizeof(struct QBSubtreeNotifierListener_t),
                                 NULL, &iface, NULL);
    }

    return iface;
}
