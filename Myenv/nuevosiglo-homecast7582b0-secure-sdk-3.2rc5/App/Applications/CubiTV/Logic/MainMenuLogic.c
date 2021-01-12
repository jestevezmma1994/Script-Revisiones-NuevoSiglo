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

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvSet.h>
#include <QBConf.h>
#include <main.h>
#include <Windows/mainmenu.h>
#include <Windows/QBDiagnosticsWindow.h>
#include <Logic/MainMenuLogic.h>
#include <Menus/QBFileBrowserMenu.h>
#include <Utils/value.h>
#include <QBXHTMLWidgets/QBXHTMLWindowsProvider.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <Menus/menuchoice.h>
#include <QBMenu/QBMenu.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBDataModel3/QBTreePath.h>
#include <CUIT/Core/event.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <libintl.h>
#include <SvCore/SvLog.h>

#include <stdbool.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBMainMenuLogic"

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "LogLevel", "");

    #define log_debug(fmt, ...)     do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...)   do { if (env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...)     do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif /* SV_LOG_LEVEL */


struct QBMainMenuLogicSecretMenuStatus_ {
    struct SvObject_ super_;
    /// Hidden menu tree node
    QBActiveTreeNode hiddenRoot;
    /// State of menu
    bool isHidden;
    /// Name of a parent menu
    SvString parentName;
    /// Name of a secret menu
    SvString childName;
};

/**
 * Secret menu status class.
 * @class QBMainMenuLogicSecretMenuStatus
 * @extends SvObject
 **/
typedef struct QBMainMenuLogicSecretMenuStatus_* QBMainMenuLogicSecretMenuStatus;

struct QBMainMenuLogic_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvHashTable secretMenu;
    SvSet handlers;
};

SvLocal void QBMainMenuLogicSecretMenuStatusDestroy(void *self_)
{
    QBMainMenuLogicSecretMenuStatus self = (QBMainMenuLogicSecretMenuStatus) self_;
    SVTESTRELEASE(self->hiddenRoot);
    SVRELEASE(self->parentName);
    SVRELEASE(self->childName);
}

SvLocal SvType QBMainMenuLogicSecretMenuStatus_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMainMenuLogicSecretMenuStatusDestroy
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMainMenuLogicSecretMenuStatus",
                            sizeof(struct QBMainMenuLogicSecretMenuStatus_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal QBMainMenuLogicSecretMenuStatus QBMainMenuLogicSecretMenuStatusCreate(SvString parentName, SvString childName)
{
    QBMainMenuLogicSecretMenuStatus menuStatus = (QBMainMenuLogicSecretMenuStatus) SvTypeAllocateInstance(QBMainMenuLogicSecretMenuStatus_getType(), NULL);
    menuStatus->isHidden = false;
    menuStatus->parentName = (SvString) SvObjectCopy((SvObject) parentName, NULL);
    menuStatus->childName = (SvString) SvObjectCopy((SvObject) childName, NULL);

    return menuStatus;
}

SvLocal bool QBMainMenuLogicIsMenuParentPosition(QBMainMenuLogic self, SvString position)
{
    SvWidget mainMenu = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvObject path = SvInvokeInterface(QBMenu, mainMenu->prv, getPosition);
    if (!path) {
        return false;
    }

    path = SvObjectCopy(path, NULL);
    SvInvokeInterface(QBTreePath, path, truncate, -1);
    QBActiveTreeNode node = QBActiveTreeFindNodeByPath(self->appGlobals->menuTree, path);
    SvString id = QBActiveTreeNodeGetID(node);
    SVRELEASE(path);

    return id ? SvObjectEquals((SvObject) id, (SvObject) position) : false;
}

SvLocal void QBMainMenuLogicShowSecretSettings(QBMainMenuLogic self, QBMainMenuLogicSecretMenuStatus status)
{
    if (status->hiddenRoot && self->appGlobals->menuTree) {
        log_debug("Activating secret menu: %s->%s", SvStringCString(status->parentName), SvStringCString(status->childName));
        QBActiveTreeNodeAddSubTree(QBActiveTreeFindNode(self->appGlobals->menuTree, status->parentName), status->hiddenRoot, NULL);
        status->isHidden = false;
    }
}

SvLocal void QBMainMenuLogicHideSecretSettings(QBMainMenuLogic self, QBMainMenuLogicSecretMenuStatus status)
{
    if (!status->isHidden && self->appGlobals->menuTree) {
        SVTESTRELEASE(status->hiddenRoot);
        status->hiddenRoot = QBActiveTreeFindNode(self->appGlobals->menuTree, status->childName);
        if (status->hiddenRoot) {
            SVRETAIN(status->hiddenRoot);
            log_debug("Deactivating secret menu: %s->%s", SvStringCString(status->parentName), SvStringCString(status->childName));
            QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(status->hiddenRoot), status->hiddenRoot, NULL);
            status->isHidden = true;
        }
    }
}

SvLocal void QBMainMenuLogicMenuChosen(SvObject self_, SvObject node, SvObject nodePath_, int position)
{
    QBMainMenuLogic self = (QBMainMenuLogic) self_;
    QBMainMenuLogicSecretMenuStatus status;
    SvIterator iter = SvHashTableGetValuesIterator(self->secretMenu);
    while ((status = (QBMainMenuLogicSecretMenuStatus) SvIteratorGetNext(&iter))) {
        if (status->isHidden) {
            continue;
        }
        SvObject pathSecret = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, status->parentName);
        if (!pathSecret)
            continue;
        int lenSecret = SvInvokeInterface(QBTreePath, pathSecret, getLength);
        SvObject path = SvObjectCopy(nodePath_, NULL);
        int len = SvInvokeInterface(QBTreePath, path, getLength);
        if (len <= lenSecret) {
            QBMainMenuLogicHideSecretSettings(self, status);
        } else {
            SvInvokeInterface(QBTreePath, path, truncate, lenSecret);
            if (!SvObjectEquals(path, pathSecret)) {
                QBMainMenuLogicHideSecretSettings(self, status);
            }
        }
        SVRELEASE(path);
        SVRELEASE(pathSecret);
    }
}

SvLocal void QBMainMenuLogicSwitchStarted(SvObject self_, QBWindowContext from, QBWindowContext to)
{
}

SvLocal void QBMainMenuLogicSwitchEnded(SvObject self_, QBWindowContext from, QBWindowContext to)
{
    QBMainMenuLogic self = (QBMainMenuLogic) self_;
    if (!from || !to) {
        return;
    }

    if (SvObjectIsInstanceOf((SvObject) from, QBMainMenuContext_getType())
        && !SvObjectIsInstanceOf((SvObject) to, QBDiagnosticsWindowContext_getType())) {
        QBMainMenuLogicSecretMenuStatus status;
        SvIterator iter = SvHashTableGetValuesIterator(self->secretMenu);
        while ((status = (QBMainMenuLogicSecretMenuStatus) SvIteratorGetNext(&iter))) {
            QBMainMenuLogicHideSecretSettings(self, status);
        }
    }
}

SvLocal void QBMainMenuLogicKeyCodeMatch(SvObject self_, QBSecretKeyCodeData code)
{
    QBMainMenuLogic self = (QBMainMenuLogic) self_;
    QBMainMenuLogicSecretMenuStatus status = (QBMainMenuLogicSecretMenuStatus) SvHashTableFind(self->secretMenu, (SvObject) code);

    if (!status) {
        log_error("Received key code cannot be found");
        return;
    }

    if (!QBMainMenuLogicIsMenuParentPosition(self, status->parentName)) {
        return;
    }

    status->isHidden ? QBMainMenuLogicShowSecretSettings(self, status) :
    QBMainMenuLogicHideSecretSettings(self, status);
}

SvLocal void
QBMainMenuLogicDestroy(void *self_)
{
    QBMainMenuLogic self = (QBMainMenuLogic) self_;
    SVRELEASE(self->secretMenu);
    SVRELEASE(self->handlers);
}

SvLocal SvType QBMainMenuLogic_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMainMenuLogicDestroy
    };

    static struct QBSecretKeyCodeServiceListener_ keyCodeMethods = {
        .keyCodeMatch = QBMainMenuLogicKeyCodeMatch
    };

    static struct QBMenuChoice_t menuMethods = {
        .choosen = QBMainMenuLogicMenuChosen
    };

    static struct QBContextSwitcherListener_t switchMethods = {
        .started = QBMainMenuLogicSwitchStarted,
        .ended   = QBMainMenuLogicSwitchEnded,
    };

    if (!type) {
        SvTypeCreateManaged("QBMainMenuLogic",
                            sizeof(struct QBMainMenuLogic_t),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBSecretKeyCodeServiceListener_getInterface(), &keyCodeMethods,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBContextSwitcherListener_getInterface(), &switchMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBMainMenuLogicAddSecretCode(QBMainMenuLogic self, QBSecretKeyCodeData secretCode, SvString parentName, SvString childName)
{
    QBMainMenuLogicSecretMenuStatus status = QBMainMenuLogicSecretMenuStatusCreate(parentName, childName);
    SvHashTableInsert(self->secretMenu, (SvObject) secretCode, (SvObject) status);
    SVRELEASE(status);
}

SvLocal void QBMainMenuLogicAddSecretCodes(QBMainMenuLogic self)
{
    uint32_t codeSecretVersion[] = {'6', '9', '8', '0'};
    QBSecretKeyCodeData dataCode = QBSecretKeyCodeDataCreate((char*)codeSecretVersion, sizeof(codeSecretVersion) / sizeof(char), NULL);
    QBMainMenuLogicAddSecretCode(self, dataCode, SVSTRING("Diagnostics"), SVSTRING("SecretVersion"));
    SVRELEASE(dataCode);
    uint32_t codeQRcode[] = {'6', '9', '8', '8'};
    QBSecretKeyCodeData dataCodeQRCode = QBSecretKeyCodeDataCreate((char*)codeQRcode, sizeof(codeQRcode) / sizeof(char), NULL);
    QBMainMenuLogicAddSecretCode(self, dataCodeQRCode, SVSTRING("SYIN"), SVSTRING("QRCodeInfo"));
    SVRELEASE(dataCodeQRCode);
}

QBMainMenuLogic QBMainMenuLogicNew(AppGlobals appGlobals)
{
    QBMainMenuLogic self = (QBMainMenuLogic) SvTypeAllocateInstance(QBMainMenuLogic_getType(), NULL);
    self->appGlobals = appGlobals;
    self->handlers = SvSetCreate(7, NULL);
    self->secretMenu = SvHashTableCreate(5, NULL);

    QBMainMenuLogicAddSecretCodes(self);

    return self;
}

bool QBMainMenuLogicHandleInputEvent(QBMainMenuLogic self, SvInputEvent e)
{
    return false;
}

void QBMainMenuLogicRegisterMenus(QBMainMenuLogic self, SvWidget menuBar, QBTreePathMap pathMap)
{
    if (self->appGlobals->fileBrowserMounter) {
        SvObject handler = QBFileBrowserMenuHandlerCreate(self->appGlobals, NULL);
        SvSetAddElement(self->handlers, handler);
        QBFileBrowserMounterRegisterHandler(self->appGlobals->fileBrowserMounter, (SvObject) handler, NULL);
        SVRELEASE(handler);
    }
}

bool QBMainMenuLogicChosen(QBMainMenuLogic self, SvObject node, SvObject path)
{
    return false;
}

void QBMainMenuLogicStart(QBMainMenuLogic self)
{
    if (!self->appGlobals->secretKeyCodeService) {
        log_warning("Starting work without secret key code service");
        return;
    }

    // to be informed about menu changes
    SvInvokeInterface(QBMainMenu, self->appGlobals->main, addGlobalHandler, (SvObject) self);
    // to be informed about window changes
    QBApplicationControllerAddListener(self->appGlobals->controller, (SvObject) self);

    QBSecretKeyCodeData keyCode;
    SvIterator iter = SvHashTableGetKeysIterator(self->secretMenu);
    while ((keyCode = (QBSecretKeyCodeData) SvIteratorGetNext(&iter))) {
        QBSecretKeyCodeServiceAddListener(self->appGlobals->secretKeyCodeService, (SvObject) self, keyCode);
    }
}

void QBMainMenuLogicStop(QBMainMenuLogic self)
{
    if (!self->appGlobals->secretKeyCodeService) {
        log_warning("Stopping work without secret key code service");
        return;
    }

    QBSecretKeyCodeData keyCode;
    SvIterator iter = SvHashTableGetKeysIterator(self->secretMenu);
    while ((keyCode = (QBSecretKeyCodeData) SvIteratorGetNext(&iter))) {
        QBSecretKeyCodeServiceRemoveListener(self->appGlobals->secretKeyCodeService, (SvObject) self, keyCode);
    }

    QBApplicationControllerRemoveListener(self->appGlobals->controller, (SvObject) self);
}

void QBMainMenuLogicPostWidgetsCreate(QBMainMenuLogic self)
{
}

void QBMainMenuLogicPreWidgetsDestroy(QBMainMenuLogic self)
{
}

SvLocal QBActiveArray
QBMainMenuLogicCreateStaticItems(void)
{
    QBActiveArray staticItems = QBActiveArrayCreate(0, NULL);
    return staticItems;
}

SvObject QBMainMenuLogicCreateProvider(QBMainMenuLogic self,
                                       SvString serviceType,
                                       SvString serviceID,
                                       SvString serviceName,
                                       SvString serviceLayout,
                                       SvDBRawObject service,
                                       bool call2Action,
                                       SvString externalUrl,
                                       SvObject *tree,
                                       SvObject *serverInfo)
{
    SvObject provider = NULL;
    SvString url = NULL;
    SvXMLRPCServerInfo serverRpcInfo = NULL;

    bool searchable = SvDBRawObjectGetBoolAttr(service, "searchable");
    bool allVisible = SvDBRawObjectGetBoolAttr(service, "all_visible");
    bool enableSlaveStaticCategories = SvDBRawObjectGetBoolAttr(service, "enableSlaveStaticCategories");

    SvString deviceID = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);

    if (serviceType && SvStringEqualToCString(serviceType, "Widget")) {
        if (!self->appGlobals->xhtmlWindowsService) {
            return NULL;
        }
        serverRpcInfo = (SvXMLRPCServerInfo) QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        SVTESTRETAIN(serverRpcInfo);
        SvValue windowsUrlVal = (SvValue) SvDBRawObjectGetAttrValue(service, "widget_url");
        url = SvValueTryGetString(windowsUrlVal);
        QBActiveArray staticItems = QBMainMenuLogicCreateStaticItems();
        QBActiveArray windowItems = QBActiveArrayCreate(10, NULL);
        *tree = (SvObject) QBTreeProxyCreate((SvObject) windowItems, (SvObject) staticItems, NULL);
        QBXHTMLWindowsClient client = QBXHTMLWindowsServiceGetXHTMLWindowsClient(self->appGlobals->xhtmlWindowsService);

        provider = (SvObject) QBXHTMLWindowsProviderCreate(self->appGlobals->sslParams, self->appGlobals->smartCardMonitor, windowItems,
                                                         serviceID, url, client,  NULL);
        SVRELEASE(staticItems);
        SVRELEASE(windowItems);
    } else {
        if (serviceLayout && SvStringEqualToCString(serviceLayout, "vod4")) {
            SvString mainName = SvStringCreate(gettext("Categories"), NULL);
            *tree = (SvObject) QBContentTreeCreate(serviceID, NULL);
            QBContentTreeAddContentRootCategory((QBContentTree) * tree, mainName, NULL);
            SVRELEASE(mainName);
        } else {
            *tree = (SvObject) QBContentTreeCreate(serviceID, NULL);
        }

        if (call2Action && externalUrl) {
            serverRpcInfo = (SvXMLRPCServerInfo) SvTypeAllocateInstance(SvXMLRPCServerInfo_getType(), NULL);
            SvXMLRPCServerInfoInit(serverRpcInfo, externalUrl, NULL);
            SvXMLRPCServerInfoSetSSLParams(serverRpcInfo, self->appGlobals->sslParams, NULL);
            url = externalUrl;
        } else {
            serverRpcInfo = (SvXMLRPCServerInfo) QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
            SVTESTRETAIN(serverRpcInfo);
            url = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        }
        SvLogNotice("Registering Innov8on service: %15s (id=%s) server=%s",
                    serviceName ? SvStringCString(serviceName) : "--",
                    SvStringCString(serviceID), SvStringCString(url));

        Innov8onProviderParams params = Innov8onProviderParamsCreate(serverRpcInfo, deviceID, 3, 30, 200, 400, 60, NULL, NULL, NULL, enableSlaveStaticCategories, NULL);
        provider = (SvObject) Innov8onProviderCreate((QBContentTree) (*tree), params, serviceID, NULL, searchable, allVisible, NULL);
        SVRELEASE(params);
    }
    *serverInfo = (SvObject) serverRpcInfo;
    return provider;
}

bool QBMainMenuLogicHandleMenuChoice(QBMainMenuLogic self, SvDBRawObject product)
{
    return false;
}
