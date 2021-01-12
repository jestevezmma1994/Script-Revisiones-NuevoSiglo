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

#include "QBSkinManager.h"

#include <stdlib.h>
#include <libintl.h>

#include <settings.h>
#include <init.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <QBConf.h>
#include <QBDataModel3/QBActiveArray.h>
#include <DataModels/loadingProxy.h>
#include <Widgets/confirmationDialog.h>
#include <Widgets/upgradeDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Logic/StandbyLogic.h>
#include <Services/QBAppVersion.h>
#include <Utils/value.h>
#include <Utils/adaptField.h>
#include <Services/core/QBMiddlewareManager.h>
#include <main.h>
#include <QBContentManager/QBLoadingStub.h>


SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBSkinManagerLogLevel", "")
#define log_debug(fmt, ...) do { if (env_log_level() >= 1) SvLogNotice( COLBEG() "QBSkinManager: " fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_fun(fmt, ...) do { if (env_log_level() >= 1) SvLogNotice( COLBEG() "QBSkinManager: %s " fmt COLEND_COL(cyan), __func__, ##__VA_ARGS__); } while (0)

//api Version
#define INNOV8ON_REQUEST_VERSION "2"

#define DEFAULT_SKIN ((SvGenericObject) SVSTRING("default"))

struct QBSkinMWError_
{
    struct SvObject_ super_;
    SvString msg;
};

SvString
QBSkinMWErrorGetMessage(QBSkinMWError self)
{
    return self->msg;
}

SvLocal void
QBSkinMWError__dtor__(void *self_)
{
    QBSkinMWError self = self_;
    SVRELEASE(self->msg);
}

SvType
QBSkinMWError_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSkinMWError__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSkinMWError",
                            sizeof(struct QBSkinMWError_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBSkinMWError
QBSkinMWErrorCreate(const char *msg)
{
    QBSkinMWError self = (QBSkinMWError) SvTypeAllocateInstance(QBSkinMWError_getType(), NULL);

    self->msg = SvStringCreate(msg, NULL);

    return self;
}

enum {
    QBSkinManagerRequest_getLayouts = 0,
    QBSkinManagerRequest_getCurrentLayout,
    QBSkinManagerRequest_setLayout,

    QBSkinManagerRequest_count
} QBSkinManagerRequests;

struct QBSkinManager_
{
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvXMLRPCRequest req[QBSkinManagerRequest_count];
    SvDataBucket storage;
    QBActiveArray skins;
    SvGenericObject forcedSkin;

    SvFiber fiber;
    SvFiberTimer timer;

    SvFiber forceSkinFiber;
    SvFiberTimer forceSkinTimer;

    int settingsCtx;
    SvWidget dialog;
};

SvLocal void
cancelRequest(SvXMLRPCRequest *req)
{
    if (req && *req) {
        SvXMLRPCRequestCancel(*req, NULL);
        SVTESTRELEASE(*req);
        *req = NULL;
    }
}

SvLocal void
QBSkinManagerSendRequest(QBSkinManager self,
                         SvXMLRPCRequest *req,
                         SvString method,
                         SvImmutableArray params)
{
    if (!req) {
        SvLogError("%s() : request = NULL", __func__);
        return;
    }

    cancelRequest(req);

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!server || !customerId || !hasNetwork) {
        SvLogWarning("%s cannot create request - server=%p, customerId=%p, hasNetwork=%d", __func__, server, customerId, hasNetwork);
        return;
    }

    *req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(*req, server, method, params, NULL);

    SvXMLRPCRequestSetListener(*req, (SvGenericObject) self, NULL);
    SvXMLRPCRequestStart(*req, NULL);
}

SvLocal void
QBSkinManagerRefreshSkins(QBSkinManager self)
{
    log_fun("");
    AppGlobals appGlobals = self->appGlobals;

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!server || !customerId) {
        log_debug("No customer id or serverInfo");
        return;
    }

    SvString version = QBAppVersionGetFullVersion(appGlobals->appVersion);

    SvImmutableArray params =
        SvImmutableArrayCreateWithTypedValues("@@", NULL, customerId, version);

    QBSkinManagerSendRequest(self, &self->req[QBSkinManagerRequest_getLayouts],
                             SVSTRING("stb.GetLayouts"), params);

    SVRELEASE(params);
    if (!QBActiveArrayCount(self->skins)) {
        SvGenericObject loadingStub = SvTypeAllocateInstance(QBLoadingStub_getType(), NULL);
        QBActiveArrayAddObject(self->skins, loadingStub, NULL);
        SVRELEASE(loadingStub);
    }
}

QBActiveArray
QBSkinManagerGetSkins(QBSkinManager self)
{
    QBSkinManagerRefreshSkins(self);
    return self->skins;
}

SvLocal void
QBSkinManagerCheckForcedSkin(QBSkinManager self)
{
    AppGlobals appGlobals = self->appGlobals;

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!server || !customerId)
        return;

    SvString version = QBAppVersionGetFullVersion(appGlobals->appVersion);

    SvImmutableArray params =
        SvImmutableArrayCreateWithTypedValues("@@", NULL, customerId, version);

    QBSkinManagerSendRequest(self, &self->req[QBSkinManagerRequest_getCurrentLayout],
                             SVSTRING("stb.GetCurrentLayout"), params);

    SVRELEASE(params);
}

void
QBSkinManagerSetSkin(QBSkinManager self, SvGenericObject skin_)
{
    AppGlobals appGlobals = self->appGlobals;
    SvString name = NULL;
    SvString url = NULL;
    SvString manifest = NULL;
    SvValue skinId = NULL;

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!server || !customerId) {
        return;
    }

    if (QBActiveArrayIndexOfObject(self->skins, skin_, NULL) == -1)
        SvLogWarning("%s() : unknown skin", __func__);

    if (SvObjectIsInstanceOf(skin_, SvDBRawObject_getType())) {
        SvDBRawObject skin = (SvDBRawObject) skin_;
        SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(skin, "name");
        SvValue urlV = (SvValue) SvDBRawObjectGetAttrValue(skin, "layout_url");
        SvValue manifestV = (SvValue) SvDBRawObjectGetAttrValue(skin, "manifest_url");

        skinId = SVRETAIN(SvDBObjectGetID((SvDBObject) skin));

        name = SvValueTryGetString(nameV);
        url = SvValueTryGetString(urlV);
        manifest = SvValueTryGetString(manifestV);

    } else if (SvObjectIsInstanceOf(skin_, SvString_getType())) {
        name = (SvString) skin_;
        skinId = SvValueCreateWithString(SVSTRING("-1"), NULL);
    } else {
        SvLogWarning("%s(): Unknown skin type [%s]", __func__, SvObjectGetTypeName(skin_));
        return;
    }

    SvImmutableArray params =
        SvImmutableArrayCreateWithTypedValues("@@", NULL, customerId, skinId);

    QBSkinManagerSendRequest(self, &self->req[QBSkinManagerRequest_setLayout],
                             SVSTRING("stb.SetLayout"), params);

    SVRELEASE(skinId);
    SVRELEASE(params);

    QBConfigSet("SKIN.NAME", name ? SvStringCString(name) : NULL);
    QBConfigSet("SKIN.URL", url ? SvStringCString(url) : NULL);
    QBConfigSet("SKIN.MANIFEST", manifest ? SvStringCString(manifest) : NULL);
    QBConfigSave();

    QBInitStopApplication(self->appGlobals, true, "skin change");
}

SvLocal void
QBSkinManagerDialogCallback(void *self_,
                            SvWidget dialog,
                            SvString buttonTag,
                            unsigned keyCode)
{
    QBSkinManager self = self_;

    if (SvStringEqualToCString(buttonTag, "OK-button")) {
        QBSkinManagerSetSkin(self, self->forcedSkin);
    }

    self->dialog = NULL;
}

// Show popup, when middleware tries to force new skin
SvLocal void
QBSkinManagerShowDialog(QBSkinManager self)
{
    svSettingsRestoreContext(self->settingsCtx);

    QBConfirmationDialogParams_t params = {
        .title = gettext("Warning"),
        .message = gettext("System reboot is required to apply new application layout."),
        .globalName = SVSTRING("SkinDialog"),
        .local = false,
        .focusOK = true,
        .isCancelButtonVisible = true
    };

    self->dialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);

    if (self->dialog) {
        QBDialogRun(self->dialog, (void *) self, QBSkinManagerDialogCallback);
    }

    svSettingsPopComponent();
}

SvLocal void
QBSkinManagerForceSkin(QBSkinManager self);

SvLocal void
QBSkinManagerForceSkinStep(void *self_)
{
    QBSkinManager self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    QBSkinManagerForceSkin(self);
}

SvLocal void
QBSkinManagerForceSkin(QBSkinManager self)
{
    QBStandbyStateData currentState = QBStandbyAgentGetCurrentDataState(self->appGlobals->standbyAgent);
    QBStandbyStateData wantedState = QBStandbyStateDataCreate(QBStandbyState_off, (SvObject) self, false, SVSTRING("QBSkin manager : switch to passive standby"));

    if (!QBStandbyAgentIsStandby(self->appGlobals->standbyAgent)) {
        if (!self->dialog)
            QBSkinManagerShowDialog(self);
    } else if (QBStandbyLogicCanSwitch(self->appGlobals->standbyLogic,
                                       currentState, wantedState)) {
        QBSkinManagerSetSkin(self, self->forcedSkin);
    } else {
        SvLogNotice("%s() : Can't force skin change, try do it later", __func__);
        if (!self->forceSkinFiber) {
            self->forceSkinFiber = SvFiberCreate(self->appGlobals->scheduler, NULL,
                                                 "SkinManager-forceSkin",
                                                 QBSkinManagerForceSkinStep, self);
            self->forceSkinTimer = SvFiberTimerCreate(self->forceSkinFiber);
        }
        SvFiberTimerActivateAfter(self->forceSkinTimer, SvTimeConstruct(60*60, 0));
    }
    SVRELEASE(wantedState);
}

SvLocal void
QBSkinManagerProcessXML(QBSkinManager self, SvArray skinsDesc, SvArray skins)
{
    static const char *attrsToAdapt[] = {
        "layout_url",
        "manifest_url",
        "thumbnail",
        NULL
    };

    SvString prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!prefixUrl) {
        return;
    }

    SvIterator it = SvArrayIterator(skinsDesc);

    SvValue skinVal = NULL;
    while ((skinVal = (SvValue) SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) skinVal, SvValue_getType()) || !SvValueIsString(skinVal)) {
            continue;
        }

        SvString text = SvValueGetString(skinVal);
        if (!text)
            continue;

        SvDBRawObject skin = (SvDBRawObject) SvDataBucketUpdate(self->storage,
                                             SvStringCString(text), NULL);
        if (!skin) {
            SvLogError("%s(): error processing a response for RPC call: '%s'",
                       __func__, SvStringCString(text));
            continue;
        }

        for (int i = 0; attrsToAdapt[i]; ++i) {
            if (!adaptField(skin, attrsToAdapt[i], prefixUrl)) {
                SVRELEASE(skin);
                skin = NULL;
                break;
            }
        }

        if (skin) {
            SvArrayAddObject(skins, (SvGenericObject) skin);
            SVRELEASE(skin);
        }
    }
}

SvLocal void
QBSkinManagerProcessSkins(QBSkinManager self, SvArray skinsDesc)
{
    if (unlikely(!skinsDesc)) {
        SvLogError("%s() : invalid answer skinsDesc", __func__);
        return;
    }

    size_t skinsDescCount = SvArrayCount(skinsDesc);
    SvArray skins = SvArrayCreateWithCapacity(skinsDescCount + 1, NULL);
    SvArrayAddObject(skins, DEFAULT_SKIN);

    QBSkinManagerProcessXML(self, skinsDesc, skins);

    SvGenericObject obj = NULL;
    const size_t newSkinsCount = SvArrayCount(skins);
    for (size_t i = 0; i < newSkinsCount; ++i) {
        obj = SvArrayObjectAtIndex(skins, i);
        if (!SvObjectEquals(obj, QBActiveArrayObjectAtIndex(self->skins, i)))
            QBActiveArraySetObjectAtIndex(self->skins, i, obj, NULL);
    }
    size_t skinsCount = QBActiveArrayCount(self->skins);
    for (size_t i = skinsCount; i > newSkinsCount; --i)
        QBActiveArrayRemoveObjectAtIndex(self->skins, i - 1, NULL);
    SVRELEASE(skins);
}

SvLocal void
QBSkinManagerProcessForcedSkin(QBSkinManager self, SvArray skinsDesc)
{
    if (unlikely(!skinsDesc)) {
        SvLogError("%s() : invalid answer skinsDesc", __func__);
        return;
    }

    SvArray skins = SvArrayCreateWithCapacity(1, NULL);
    QBSkinManagerProcessXML(self, skinsDesc, skins);

    if (!SvArrayCount(skins)) {
        SvLogError("%s() : Invalid skin", __func__);
        goto out;
    }
    SvDBRawObject skin = (SvDBRawObject) SvArrayObjectAtIndex(skins, 0);

    SvValue nameV = (SvValue) SvDBRawObjectGetAttrValue(skin, "name");
    SvString name = SvValueTryGetString(nameV);
    if (!name) {
        SvLogError("%s() : No skin name", __func__);
        goto out;
    }

    const char *currentName = QBConfigGet("SKIN.NAME");
    if (SvStringEqualToCString(name, currentName)) {
        SvLogNotice("%s() : No need to force skin change", __func__);
        goto out;
    }
    SvLogNotice("%s() : Force skin change", __func__);

    SVTESTRETAIN(skin);
    SVTESTRELEASE(self->forcedSkin);
    self->forcedSkin = (SvGenericObject) skin;

    QBSkinManagerForceSkin(self);

out:
    SVRELEASE(skins);
}

// SvXMLRPCClientListener virtual methods

SvLocal void
QBSkinManagerRequestSetup(SvGenericObject self_, SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"),
                               SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBSkinManagerRequestStateChanged(SvGenericObject self_, SvXMLRPCRequest req,
                                 SvXMLRPCRequestState state)
{
    QBSkinManager self = (QBSkinManager) self_;

    log_fun("");

    int idx = -1;
    for (int i = 0; i < QBSkinManagerRequest_count; ++i) {
        if (req == self->req[i]) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        SvLogWarning("%s() : Notification from unknown request", __func__);
        return;
    }

    log_debug("Request %p:%i has state %i", req, idx, state);

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader)
            SvXMLRPCRequestGetDataListener(self->req[idx], NULL);
        if (reader) {
            SvHashTable results = (SvHashTable) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
            if (results && SvObjectIsInstanceOf((SvObject) results, SvHashTable_getType())) {
                SVAUTOSTRING(keyVal, "products");
                SvArray products = (SvArray) SvHashTableFind(results, (SvObject) keyVal);
                if (products && SvObjectIsInstanceOf((SvObject) products, SvArray_getType())) {
                    SvString method = SvXMLRPCRequestGetMethodName(self->req[idx]);
                    if (SvStringEqualToCString(method, "stb.GetLayouts")) {
                        QBSkinManagerProcessSkins(self, products);
                    } else if (SvStringEqualToCString(method, "stb.GetCurrentLayout")) {
                        QBSkinManagerProcessForcedSkin(self, products);
                    }
                } else {
                    SvLogError("%s() : No products in reply", __func__);
                }
            }
        }
    } else if (state == SvXMLRPCRequestState_gotException) {
        SvString desc;
        int code;
        SvXMLRPCRequestGetException(req, &code, &desc, NULL);
        SvLogWarning("%s got Exception: XML-RPC call thrown exception with fault code %d: %s",
                     __func__, code, SvStringCString(desc));
        SvLogWarning("\toriginal request: %s",
                     SvStringCString(SvXMLRPCRequestGetDescription(req)));

        SvString method = SvXMLRPCRequestGetMethodName(self->req[idx]);
        if (SvStringEqualToCString(method, "stb.GetLayouts")) {
            QBActiveArraySetObjectAtIndex(self->skins, 0, DEFAULT_SKIN, NULL);
            QBSkinMWError mwError = QBSkinMWErrorCreate(gettext("Can't load skins list"));
            QBActiveArraySetObjectAtIndex(self->skins, 1, (SvGenericObject) mwError, NULL);
            int count = QBActiveArrayCount(self->skins);
            for (int i = count - 1; i >= 2; --i) // remove rest of the objects
                QBActiveArrayRemoveObjectAtIndex(self->skins, i, NULL);
            SVRELEASE(mwError);
        }
    }

    if (state == SvXMLRPCRequestState_gotAnswer || state == SvXMLRPCRequestState_error ||
        state == SvXMLRPCRequestState_cancelled || state == SvXMLRPCRequestState_gotException) {

        SVTESTRELEASE(self->req[idx]);
        self->req[idx] = NULL;
    }
}

SvLocal void
QBSkinManagerRescheduleRefresh(QBSkinManager self)
{
    if (!self->timer) {
        return;
    }

    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(24*60*60, 0));
}

SvLocal void
QBSkinManagerStep(void *self_)
{
    QBSkinManager self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    QBSkinManagerRefreshSkins(self);
    QBSkinManagerCheckForcedSkin(self);
    QBSkinManagerRescheduleRefresh(self);
}

// QBMiddlewareManagerListener virtual methods

SvLocal void
QBSkinManagerMiddlewareDataChanged(SvGenericObject self_,
                                 QBMiddlewareManagerType middlewareType)
{
    QBSkinManager self = (QBSkinManager) self_;

    if (middlewareType != QBMiddlewareManagerType_Innov8on)
        return;

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString mwId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);\
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);

    if (server && mwId && hasNetwork) {
        QBSkinManagerRefreshSkins(self);
        QBSkinManagerCheckForcedSkin(self);
        QBSkinManagerRescheduleRefresh(self);
    } else {
        for (int i = 0; i < QBSkinManagerRequest_count; ++i)
            cancelRequest(&self->req[i]);
    }
}

SvLocal void
QBSkinManager__dtor__(void *self_)
{
    QBSkinManager self = self_;

    if (self->fiber)
        SvFiberDestroy(self->fiber);
    if (self->forceSkinFiber)
        SvFiberDestroy(self->forceSkinFiber);

    for (int i = 0; i < QBSkinManagerRequest_count; ++i)
        cancelRequest(&self->req[i]);

    if (self->dialog)
        QBDialogBreak(self->dialog);

    SVRELEASE(self->storage);
    SVRELEASE(self->skins);
    SVTESTRELEASE(self->forcedSkin);
}

SvLocal SvType
QBSkinManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSkinManager__dtor__
    };
    static SvType type = NULL;
    static const struct SvXMLRPCClientListener_t listenerMethods = {
        .stateChanged   = QBSkinManagerRequestStateChanged,
        .setup          = QBSkinManagerRequestSetup
    };
    static const struct QBMiddlewareManagerListener_t middlewareListenerMethods = {
        .middlewareDataChanged = QBSkinManagerMiddlewareDataChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBSkinManager",
                            sizeof(struct QBSkinManager_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &listenerMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareListenerMethods,
                            NULL);
    }

    return type;
}

QBSkinManager
QBSkinManagerCreate(AppGlobals appGlobals)
{
    if (!appGlobals->middlewareManager) {
        SvLogError("%s() : No middleware id manager", __func__);
        return NULL;
    }
    if (!appGlobals->appVersion) {
        SvLogError("%s() : No app version", __func__);
        return NULL;
    }
    QBSkinManager self = (QBSkinManager) SvTypeAllocateInstance(QBSkinManager_getType(), NULL);
    self->appGlobals = appGlobals;

    self->storage = SvDataBucketCreate(NULL);
    self->skins = QBActiveArrayCreate(15, NULL);

    QBActiveArraySetObjectAtIndex(self->skins, 0, DEFAULT_SKIN, NULL);

    svSettingsPushComponent("Carousel_WEBTV.settings");
    self->settingsCtx = svSettingsSaveContext();
    self->dialog = NULL;
    for (int i = 0; i < QBSkinManagerRequest_count; ++i)
        self->req[i] = NULL;
    self->forcedSkin = NULL;
    self->fiber = NULL;
    self->timer = NULL;
    self->forceSkinFiber = NULL;
    self->forceSkinTimer = NULL;

    return self;
}

void
QBSkinManagerStart(QBSkinManager self)
{
    QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager,
                                     (SvGenericObject) self);

    if (!self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL,
                                    "SkinManager-poll",
                                    QBSkinManagerStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
    }

    SvFiberActivate(self->fiber);
}

void
QBSkinManagerStop(QBSkinManager self)
{
    QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager,
                                        (SvGenericObject) self);

    for (int i = 0; i < QBSkinManagerRequest_count; ++i)
        cancelRequest(&self->req[i]);
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }
    if (self->forceSkinFiber) {
        SvFiberDestroy(self->forceSkinFiber);
        self->forceSkinFiber = NULL;
        self->forceSkinTimer = NULL;
    }
}

