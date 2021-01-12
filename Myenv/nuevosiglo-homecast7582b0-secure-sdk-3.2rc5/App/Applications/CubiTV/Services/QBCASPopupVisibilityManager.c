/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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
#include <Services/QBCASPopupVisibilityManager.h>
#include <Services/QBCASManager.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvSet.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBCASPopupVisibilityManager"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName, "");
    #define log_debug(fmt, ...) do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_deep(fmt, ...)  do { if (env_log_level() >= 4) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_info(fmt, ...)  do { if (env_log_level() >= 3) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...) do { if (env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(magenta), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif

#define CONTAINERS_SIZE 3

struct QBCASPopupVisibilityManager_ {
    struct SvObject_ super_;
    SvObject casPopupManager;
    SvHashTable contexts;
    SvSet globalWindows;
    bool blocked;
};

SvLocal void QBCASPopupVisibilityManagerDestroy(void *self_)
{
    QBCASPopupVisibilityManager self = (QBCASPopupVisibilityManager) self_;
    SVTESTRELEASE(self->casPopupManager);
    SVRELEASE(self->contexts);
    SVRELEASE(self->globalWindows);
}

SvLocal SvType QBCASPopupVisibilityManager_getType(void)
{
    static const struct SvObjectVTable_ objectMethods = {
        .destroy = &QBCASPopupVisibilityManagerDestroy,
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCASPopupVisibilityManager",
                            sizeof(struct QBCASPopupVisibilityManager_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectMethods,
                            NULL);
    }

    return type;
}

SvHidden QBCASPopupVisibilityManager QBCASPopupVisibilityManagerCreate(SvObject casPopupManager)
{
    QBCASPopupVisibilityManager self = (QBCASPopupVisibilityManager) SvTypeAllocateInstance(QBCASPopupVisibilityManager_getType(), NULL);
    self->casPopupManager = SVTESTRETAIN(casPopupManager);
    self->contexts = SvHashTableCreate(CONTAINERS_SIZE, NULL);
    self->globalWindows = SvSetCreate(CONTAINERS_SIZE, NULL);
    return self;
}

SvLocal void QBCASPopupVisibilityManagerUpdateStatus(QBCASPopupVisibilityManager self)
{
    bool isBlocked = SvHashTableGetCount(self->contexts) > 0 || !SvSetIsEmpty(self->globalWindows);

    if (self->blocked == isBlocked) {
        return;
    }
    self->blocked = isBlocked;

    if (self->casPopupManager) {
        SvInvokeInterface(QBCASPopupManager, self->casPopupManager, visibilityChanged, self->blocked);
    }
}

SvHidden void QBCASPopupVisibilityManagerBlock(QBCASPopupVisibilityManager self, SvString contextId, SvString windowId)
{
    SvSet windowContainer = self->globalWindows;

    if (contextId) {
        windowContainer = (SvSet) SvHashTableFind(self->contexts, (SvObject) contextId);
        if (!windowContainer) {
            windowContainer = SvSetCreate(CONTAINERS_SIZE, NULL);
            SvHashTableInsert(self->contexts, (SvObject) contextId, (SvObject) windowContainer);
            // decrease redundant retain, from now state is the same like after SvHashTableFind when object is found
            SVRELEASE(windowContainer);

            if (SvHashTableGetCount(self->contexts) > 1) {
                log_warning("More than one contetxt blocks window (new context: %s)", SvStringCString(contextId));
            }
        }
    }

    if (SvSetContainsElement(windowContainer, (SvObject) windowId)) {
        log_error("Window %s already registered block in context %s", SvStringCString(windowId), contextId ? SvStringCString(contextId) : "global Windows");
    } else {
        SvSetAddElement(windowContainer, (SvObject) windowId);
    }
    QBCASPopupVisibilityManagerUpdateStatus(self);
}

SvHidden void QBCASPopupVisibilityManagerUnblock(QBCASPopupVisibilityManager self, SvString contextId, SvString windowId)
{
    SvSet windowContainer = self->globalWindows;
    if (contextId) {
        windowContainer = (SvSet) SvHashTableFind(self->contexts, (SvObject) contextId);
        if (windowContainer == NULL) {
            log_error("Can not find context %s while unblocking window %s", SvStringCString(contextId), SvStringCString(windowId));
            return;
        }
    }

    if (!SvSetContainsElement(windowContainer, (SvObject) windowId)) {
        log_error("Window %s not registered block in context %s", SvStringCString(windowId), contextId ? SvStringCString(contextId) : "global Windows");
    } else {
        SvSetRemoveElement(windowContainer, (SvObject) windowId);
        if (SvSetIsEmpty(windowContainer) && contextId != NULL) {
            SvHashTableRemove(self->contexts, (SvObject) contextId);
        }
    }
    QBCASPopupVisibilityManagerUpdateStatus(self);
}

SvHidden void QBCASPopupVisibilityManagerUnblockAll(QBCASPopupVisibilityManager self, SvString contextId)
{
    SvHashTableRemove(self->contexts, (SvObject) contextId);
    QBCASPopupVisibilityManagerUpdateStatus(self);
}
