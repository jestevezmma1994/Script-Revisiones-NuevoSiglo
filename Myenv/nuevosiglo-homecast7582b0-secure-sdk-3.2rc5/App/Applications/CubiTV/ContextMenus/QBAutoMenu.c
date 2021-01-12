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

#include "QBAutoMenu.h"

#include <libintl.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>
#include <settings.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreeModel.h>
#include <ContextMenus/QBBasicPane.h>
#include <main.h>

SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 1, "QBAutoMenuLogLevel", "QBAutoMenu log level" );

#define log_error(fmt, ...)                                                 \
    do { SvLogError("%s() :: " fmt, __func__, ##__VA_ARGS__); } while (0)

#define log_warn(fmt, ...)                                                  \
    do { SvLogWarning("%s() :: " fmt, __func__, ##__VA_ARGS__); } while (0)

#define log_state(fmt, ...) \
    do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(cyan), __func__, ##__VA_ARGS__); } } while (0)

#define log_info(fmt, ...)                                                  \
    do { if (env_log_level() >= 2) { SvLogNotice("%s() :: " fmt, __func__, ##__VA_ARGS__); } } while (0)

#define log_debug(fmt, ...)                                                 \
    do { if (env_log_level() >= 4) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); } } while (0)

struct QBAutoMenu_ {
    struct SvObject_ super_;

    SvApplication app;
    QBContextMenu ctxMenu;

    int settingsCtx;

    QBActiveTree tree;
    QBBasicPane rootPane;
    SvWeakReference handler;
};

SvLocal QBBasicPane
QBAutoMenuCreateLevel(QBAutoMenu self, int level, SvGenericObject path);

SvLocal void
QBAutoMenuBasicPaneHandler(void *self_,
                           SvString id,
                           QBBasicPane pane,
                           QBBasicPaneItem item)
{
    QBAutoMenu self = self_;
    log_debug("Selected node : %s", SvStringCString(id));

    SvGenericObject path = SvHashTableFind(item->metadata, (SvGenericObject) SVSTRING("path"));
    SvGenericObject node = SvHashTableFind(item->metadata, (SvGenericObject) SVSTRING("node"));

    SvGenericObject handler = SvWeakReferenceTakeReferredObject(self->handler);
    if (!SvInvokeInterface(QBAutoMenuEventHandler, handler, itemSelected, node, path, pane, item)) {
        int level = QBBasicPaneGetLevel(pane);
        QBBasicPane options = QBAutoMenuCreateLevel(self, level + 1, path);
        if (options) {
            QBContextMenuPushPane(self->ctxMenu, (SvGenericObject) options);
            SVRELEASE(options);
        }
    }
    SVRELEASE(handler);
}

SvLocal QBBasicPane
QBAutoMenuCreateLevel(QBAutoMenu self, int level, SvGenericObject parentPath)
{
    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree, getIterator, parentPath, 0);
    if (QBTreeIteratorGetNodesCount(&iter) == 0)
        return NULL;

    svSettingsRestoreContext(self->settingsCtx);

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBTextRenderer"));
    QBBasicPaneInit(options, self->app, SvSchedulerGet(),
                    textRenderer, self->ctxMenu, level, SVSTRING("BasicPane"));

    svSettingsPopComponent();

    SvGenericObject nextPath = QBTreeIteratorGetCurrentNodePath(&iter);
    SvGenericObject path = NULL;
    QBActiveTreeNode node = NULL;
    while ((node = (QBActiveTreeNode) QBTreeIteratorGetNextNode(&iter))) {
        path = nextPath;
        SvString caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));


        SvString option = SvStringCreate(gettext(SvStringCString(caption)), NULL);
        QBBasicPaneItem item = QBBasicPaneAddOption(options, QBActiveTreeNodeGetID(node), option,
                                                    QBAutoMenuBasicPaneHandler, self);
        SVRELEASE(option);
        SvHashTableInsert(item->metadata, (SvGenericObject) SVSTRING("path"), (SvGenericObject) path);
        SvHashTableInsert(item->metadata, (SvGenericObject) SVSTRING("node"), (SvGenericObject) node);

        nextPath = QBTreeIteratorGetCurrentNodePath(&iter);
    }

    return options;
}

SvLocal void
QBAutoMenuShow(SvGenericObject self_)
{
    QBAutoMenu self = (QBAutoMenu) self_;
    SvInvokeInterface(QBContextMenuPane, self->rootPane, show);
}

SvLocal void
QBAutoMenuHide(SvGenericObject self_, bool immediately)
{
    QBAutoMenu self = (QBAutoMenu) self_;
    SvInvokeInterface(QBContextMenuPane, self->rootPane, hide, immediately);
}

SvLocal void
QBAutoMenuSetActive(SvGenericObject self_)
{
    QBAutoMenu self = (QBAutoMenu) self_;
    SvInvokeInterface(QBContextMenuPane, self->rootPane, setActive);
}

SvLocal bool
QBAutoMenuHandleInputEvent(SvObject self_,
                           SvObject src,
                           SvInputEvent e)
{
    return false;
}

SvLocal void
QBAutoMenuDestroy(void *self_)
{
    QBAutoMenu self = self_;

    SVRELEASE(self->tree);
    SVRELEASE(self->handler);
    SVRELEASE(self->rootPane);
}

SvLocal SvType
QBAutoMenu_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAutoMenuDestroy
    };
    static SvType type = NULL;

    static const struct QBContextMenuPane_ methods = {
        .show             = QBAutoMenuShow,
        .hide             = QBAutoMenuHide,
        .setActive        = QBAutoMenuSetActive,
        .handleInputEvent = QBAutoMenuHandleInputEvent
    };


    if (!type) {
        SvTypeCreateManaged("QBAutoMenu",
                            sizeof(struct QBAutoMenu_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

QBAutoMenu
QBAutoMenuCreate(SvApplication app,
                 QBContextMenu ctxMenu,
                 SvString menuDescriptionFile,
                 SvGenericObject eventHandler,
                 SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBAutoMenu self = NULL;

    if (!ctxMenu || !menuDescriptionFile || !eventHandler) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed [ctxMenu = %p, menuDescriptionFile = %s, eventHandler = %p]",
                                  ctxMenu, menuDescriptionFile ? SvStringCString(menuDescriptionFile) : NULL,
                                  eventHandler);
        goto out;
    }

    self = (QBAutoMenu) SvTypeAllocateInstance(QBAutoMenu_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate QBAutoMenu");
        goto out;
    }

    self->settingsCtx = svSettingsSaveContext();

    const char *path = SvStringCString(menuDescriptionFile);
    self->tree = QBActiveTreeCreateFromFile(path, &error);
    if (!self->tree) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBActiveTreeCreateFromFile(%s) failed", path);
        goto out;
    }

    self->app = app;
    self->ctxMenu = ctxMenu;
    self->handler = SvWeakReferenceCreateWithObject(eventHandler, NULL);

    self->rootPane = QBAutoMenuCreateLevel(self, 1, NULL);

out:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

SvInterface
QBAutoMenuEventHandler_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBAutoMenuEventHandler", sizeof(struct QBAutoMenuEventHandler_),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}
