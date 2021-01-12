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

#include <main.h>
#include "QBWebshellMenu.h"
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include "menuchoice.h"
#include <QBMenu/QBMenu.h>
#include <XMB2/XMBMenuBar.h>
#include <QBWindows/QBWebshellContext.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <QBConf.h>
#include <QBWebBrowserApp.h>


struct QBWebshellMenuHandler_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvString appName;
};

typedef struct QBWebshellMenuHandler_ *QBWebshellMenuHandler;


SvLocal void QBWebshellMenuHandlerDestroy(void *self_)
{
    QBWebshellMenuHandler self = (QBWebshellMenuHandler) self_;
    QBAppsManagerUnregisterApp(self->appGlobals->appsManager, self->appName);
    SVRELEASE(self->appName);
}

SvLocal void QBWebshellMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBWebshellMenuHandler self = (QBWebshellMenuHandler) self_;

    SvString appType = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) node_, SVSTRING("appType"));
    if (!appType || !SvStringEqualToCString(appType, "webBrowser"))
        return;

    struct QBWebshellContextParams_s webShellCtxParams;
    if (QBWebshellContextParamsInit(&webShellCtxParams, node_, self->appGlobals->appsManager, self->appName) < 0) {
        SvLogError("Failed to initialize webshell context params");
        return;
    }

    struct QBWebshellContextGlobalParams_ globals = {
        .app              = self->appGlobals->res,
        .controller       = self->appGlobals->controller,
        .appsManager      = self->appGlobals->appsManager,
        .viewport         = QBViewportGet(),
        .playerController = self->appGlobals->playerController,
        .scheduler        = self->appGlobals->scheduler
    };

    QBWindowContext wwwContext = QBWebshellContextCreate(&globals, &webShellCtxParams, NULL);
    if (!wwwContext) {
        SvLogError("Failed to create webshell context");
        return;
    }

    QBApplicationControllerPushContext(self->appGlobals->controller, wwwContext);
    SVRELEASE(wwwContext);
}

SvLocal SvType QBWebshellMenuHandler_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ vtable = {
        .destroy = QBWebshellMenuHandlerDestroy
    };
    static struct QBMenuChoice_t menuMethods = {
        .choosen = QBWebshellMenuChoosen
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBWebshellMenuHandler",
                            sizeof(struct QBWebshellMenuHandler_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            NULL);
    }
    return type;
}

SvLocal void QBWebshellMenuHideWebshellMenu(QBActiveTreeNode node)
{
    QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
}

void QBWebshellMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvGenericObject path = NULL;

    QBActiveTreeNode node = QBActiveTreeFindNodeByID(appGlobals->menuTree, SVSTRING("WEBSHELL"), &path);
    if (!path)
        return;

    if (!appGlobals->appsManager) {
        SvLogWarning("%s() : Couldn't register web browser application - QBAppsManager service not available.", __func__);
        QBWebshellMenuHideWebshellMenu(node);
        return;
    }

    SvString appName = NULL;
    const char* appNameCStr = QBConfigGet("APPS.WEBBROWSER");
    if (appNameCStr) {
        appName = SvStringCreate(appNameCStr, NULL);
    } else {
        // get first available web browser app
        appName = SVTESTRETAIN(QBAppsManagerGetAppNameByType(appGlobals->appsManager, QBAppType_WebBrowser));
    }

    if (!appName) {
        // assume that web browser app is not present
        SvLogWarning("%s() : Couldn't register web browser application - application not available.", __func__);
        QBWebshellMenuHideWebshellMenu(node);
        return;
    }

    SvObject factory = (SvObject) QBWebBrowserAppFactoryCreate();
    if (!QBAppsManagerRegisterApp(appGlobals->appsManager, appName, (SvObject) factory)) {
        SvLogError("%s() : Couldn't register web browser application.", __func__);
    } else {
        QBWebshellMenuHandler handler = (QBWebshellMenuHandler) SvTypeAllocateInstance(QBWebshellMenuHandler_getType(), NULL);
        handler->appGlobals = appGlobals;
        handler->appName = appName;

        QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);

        SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) handler, NULL);
        SVRELEASE(handler);
    }
    SVRELEASE(factory);
}
