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

#include "QBDeviceNameMenu.h"
#include <QBSecureLogManager.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <Menus/menuchoice.h>
#include <QBInput/QBInputCodes.h>
#include <QBMenu/QBMenu.h>
#include <QBOSK/QBOSKKey.h>
#include <QBConf.h>
#include <main.h>
#include <libintl.h>
#include <settings.h>

#define DEFAULT_DEVICE_NAME "CubiTV Device"

typedef struct QBDeviceNameMenuHandler_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBActiveTreeNode mainNode;
    SvString deviceName;

    struct QBDeviceNameSideMenu_ {
        QBContextMenu ctx;
    } sidemenu;
} *QBDeviceNameMenuHandler;

SvLocal void QBDeviceNameMenuKeyTyped(void *self_, QBOSKPane pane, SvString input, unsigned int layout, QBOSKKey key)
{
    QBDeviceNameMenuHandler self = (QBDeviceNameMenuHandler) self_;
    if (key->type == QBOSKKeyType_enter && input && SvStringGetLength(input)) {
        if (!SvObjectEquals((SvObject) self->deviceName, (SvObject) input)) {
            QBConfigSet("DEVICE_NAME", SvStringCString(input));
            QBConfigSave();

            QBActiveTreeNodeSetAttribute(self->mainNode, SVSTRING("subcaption"), (SvObject) input);
            QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, self->mainNode, NULL);

            SVRELEASE(self->deviceName);
            self->deviceName = SVRETAIN(input);
        }
        QBContextMenuPopPane(self->sidemenu.ctx);
    }
}

SvLocal void QBDeviceNameMenuHandlerMenuChosen(SvObject self_, SvObject node_, SvObject nodePath_, int position)
{
    QBDeviceNameMenuHandler self = (QBDeviceNameMenuHandler) self_;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        SvObject options = (SvObject) QBOSKPaneCreateFromSettings("OSKPane.settings",
                                                                  self->appGlobals->scheduler,
                                                                  self->sidemenu.ctx,
                                                                  1,
                                                                  SVSTRING("OSKPane"),
                                                                  QBDeviceNameMenuKeyTyped,
                                                                  self);
        if (!options)
            return;

        QBOSKPaneSetInput((QBOSKPane) options, self->deviceName);
        QBContextMenuShow(self->sidemenu.ctx);
        QBContextMenuPushPane(self->sidemenu.ctx, options);
        SVRELEASE(options);
    }
}

SvLocal void QBDeviceNameMenuHandlerDestroy(void *self_)
{
    QBDeviceNameMenuHandler self = (QBDeviceNameMenuHandler) self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    SVTESTRELEASE(self->deviceName);
}

SvLocal SvType QBDeviceNameMenuHandler_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vtable = {
        .destroy = QBDeviceNameMenuHandlerDestroy
    };

    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBDeviceNameMenuHandlerMenuChosen
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDeviceNameMenuHandler",
                            sizeof(struct QBDeviceNameMenuHandler_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            NULL);
    }
    return type;
}

void QBDeviceNameMenuRegister(QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode mainNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("DeviceName"));
    SvObject path = mainNode ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!mainNode || !path)
        return;

    QBDeviceNameMenuHandler self = (QBDeviceNameMenuHandler)
                                   SvTypeAllocateInstance(QBDeviceNameMenuHandler_getType(), NULL);

    self->appGlobals = appGlobals;
    self->mainNode = mainNode;
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);

    const char *deviceNameCStr = QBConfigGet("DEVICE_NAME");
    if (!deviceNameCStr)
        deviceNameCStr = DEFAULT_DEVICE_NAME;
    self->deviceName = SvStringCreate(deviceNameCStr, NULL);
    QBActiveTreeNodeSetAttribute(self->mainNode, SVSTRING("subcaption"), (SvObject) self->deviceName);

    SVRELEASE(path);
    SVRELEASE(self);
}
