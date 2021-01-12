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

#include <libintl.h>

#include <main.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <DataModels/loadingProxy.h>
#include <QBContentManager/QBContentProvider.h>
#include <QBContentManager/QBLoadingStub.h>

#include "QBSocialMediaRemoteAction.h"

//api Version
#define INNOV8ON_REQUEST_VERSION "2"

SvLocal SvString
SvHashTableFindString(SvHashTable self, SvString key)
{
    SvGenericObject value = SvHashTableFind(self, (SvGenericObject) key);
    if (!value)
        return NULL;
    if (SvObjectIsInstanceOf(value, SvValue_getType())) {
        if (SvValueIsString((SvValue) value))
            return SvValueGetString((SvValue) value);
        else
            return NULL;
    } else if (SvObjectIsInstanceOf(value, SvString_getType())) {
        return (SvString) value;
    }
    return NULL;
}

struct QBSocialMediaRemoteAction_t {
    struct SvObject_ super_;

    AppGlobals       appGlobals;

    SvXMLRPCRequest  req;
    Innov8onProvider provider;
    SvDataBucket     bucket;

    SvHashTable      action;

    QBSocialMediaRemoteActionCallback callback;
    void *callbackData;
};

// QBSocialMediaRemoteAction

void
QBSocialMediaRemoteActionDoAction(QBSocialMediaRemoteAction self,
                                  SvHashTable action,
                                  int serviceId,
                                  QBSocialMediaRemoteActionCallback callback,
                                  void *callbackData)
{
    AppGlobals appGlobals = self->appGlobals;

    QBSocialMediaRemoteActionCancelAction(self);

    SVTESTRELEASE(self->action);
    self->action = SVRETAIN(action);

    SvString methodName = SvHashTableFindString(action, SVSTRING("method"));

    SvString login = NULL, password = NULL;
    QBAuthenticationServiceGetCredentials(appGlobals->authenticationService,
                                          serviceId, &login, &password, NULL);

    if (!strcmp(SvStringCString(methodName), "GetObjects")) {
        Innov8onProviderParams params = NULL;
        SvString serviceID = SvStringCreateWithFormat("%d", serviceId);
        QBContentTree tree = QBContentTreeCreate(serviceID, NULL);
        SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
        SvString deviceID = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);

        params = Innov8onProviderParamsCreate(serverInfo, deviceID,
                                              3, 10, 200, 200, 60,
                                              login, password, NULL, false, NULL);

        if (self->provider) {
            QBContentProviderStop((QBContentProvider) self->provider);
            SVRELEASE(self->provider);
        }
        SvString categoryId = SvHashTableFindString(action, SVSTRING("data"));
        self->provider = Innov8onProviderCreate(tree, params, serviceID,
                                                categoryId, false, false, NULL);
        QBContentProviderSetup((QBContentProvider) self->provider);
        QBContentProviderStart((QBContentProvider) self->provider,
                               appGlobals->scheduler);
        SvLogNotice("%s() : provider %p created for service %s",
                    __func__, self->provider, SvStringCString(serviceID));
        SVRELEASE(params);
        SVRELEASE(serviceID);

        SvGenericObject proxy = (SvGenericObject) QBLoadingProxyCreate((SvGenericObject) tree, NULL);
        callback(callbackData, action, proxy);
        SVRELEASE(proxy);
        SVRELEASE(tree);

        return;
    }

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(appGlobals->middlewareManager);
    if (!server || !customerId || !hasNetwork) {
        return;
    }

    SvImmutableArray params = NULL;

    SvString name = SvHashTableFindString(action, SVSTRING("action_name"));
    SvString data = SvHashTableFindString(action, SVSTRING("data"));
    SvString type = SvHashTableFindString(action, SVSTRING("type"));
    SvString method = SvStringCreateWithFormat("stb.%s", SvStringCString(methodName));

    params = SvImmutableArrayCreateWithTypedValues("@i@@@@@", NULL,
                                                   customerId, serviceId,
                                                   name, data, type,
                                                   login, password);

    self->req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(self->req, server, method, params, NULL);
    SVRELEASE(params);

    SvLogNotice("%s() : call %s", __func__, SvStringCString(method));
    SVRELEASE(method);

    self->callback = callback;
    self->callbackData = callbackData;

    SvGenericObject loadingStub =
        SvTypeAllocateInstance(QBLoadingStub_getType(), NULL);
    callback(callbackData, action, loadingStub);
    SVRELEASE(loadingStub);

    SvXMLRPCRequestSetListener(self->req, (SvGenericObject) self, NULL);
    SvXMLRPCRequestStart(self->req, NULL);
}

void
QBSocialMediaRemoteActionCancelAction(QBSocialMediaRemoteAction self)
{
    if (self->provider) {
        QBContentProviderStop((QBContentProvider) self->provider);
        SVRELEASE(self->provider);
        self->provider = NULL;
    }

    if (self->req) {
        SvErrorInfo error = NULL;
        SvXMLRPCRequestCancel(self->req, &error);
        if (error)
            SvErrorInfoDestroy(error);
        SVTESTRELEASE(self->req);
        self->req = NULL;
    }
}

SvLocal void
QBSocialMediaRemoteActionRequestSetup(SvGenericObject self_,
                                      SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"),
                               SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBSocialMediaRemoteActionRequestStateChanged(SvGenericObject self_,
                                             SvXMLRPCRequest req,
                                             SvXMLRPCRequestState state)
{
    QBSocialMediaRemoteAction self = (QBSocialMediaRemoteAction) self_;

    // cancel pending requests
    if (req != self->req) {
        SvXMLRPCRequestCancel(req, NULL);
        SVTESTRELEASE(req);
        SvLogWarning("%s() : Notification from unknown request", __func__);
        return;
    }

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader)
            SvXMLRPCRequestGetDataListener(self->req, NULL);
        if (reader) {
            SvHashTable results = (SvHashTable) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
            if (results) {
                SVAUTOSTRING(keyVal, "products");
                SvArray products = (SvArray) SvHashTableFind(results, (SvGenericObject) keyVal);
                if (products && SvArrayCount(products) > 0) {
                    SvValue xmlV = (SvValue) SvArrayAt(products, 0);
                    SvString xml = SvValueGetString(xmlV);
                    SvDBObject product = SvDataBucketUpdate(self->bucket, SvStringCString(xml), NULL);
                    self->callback(self->callbackData, self->action, (SvGenericObject) product);
                    SVTESTRELEASE(product);
                } else {
                    SvLogWarning("%s() : No products in reply", __func__);
                    self->callback(self->callbackData, self->action, NULL);
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
        self->callback(self->callbackData, self->action, NULL);
    }

    if (state == SvXMLRPCRequestState_gotAnswer || state == SvXMLRPCRequestState_error ||
        state == SvXMLRPCRequestState_cancelled || state == SvXMLRPCRequestState_gotException) {

        SVTESTRELEASE(self->req);
        self->req = NULL;
    }
}

SvLocal void QBSocialMediaRemoteActionDestroy(void *self_)
{
    QBSocialMediaRemoteAction self = self_;

    QBSocialMediaRemoteActionCancelAction(self);

    SVRELEASE(self->bucket);

    SVTESTRELEASE(self->action);
    SVTESTRELEASE(self->provider);
}

SvType QBSocialMediaRemoteAction_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaRemoteActionDestroy
    };
    static SvType type = NULL;
    static const struct SvXMLRPCClientListener_t listenerMethods = {
        .stateChanged   = QBSocialMediaRemoteActionRequestStateChanged,
        .setup          = QBSocialMediaRemoteActionRequestSetup
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaRemoteAction",
                            sizeof(struct QBSocialMediaRemoteAction_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &listenerMethods,
                            NULL);
    }

    return type;
}

QBSocialMediaRemoteAction
QBSocialMediaRemoteActionCreate(AppGlobals appGlobals)
{
    QBSocialMediaRemoteAction self = (QBSocialMediaRemoteAction)
        SvTypeAllocateInstance(QBSocialMediaRemoteAction_getType(), NULL);

    QBSocialMediaRemoteActionInit(self, appGlobals);

    return self;
}

void
QBSocialMediaRemoteActionInit(QBSocialMediaRemoteAction self,
                              AppGlobals appGlobals)
{
    self->appGlobals = appGlobals;
    self->bucket = SvDataBucketCreate(NULL);
    self->req = NULL;
    self->provider = NULL;
}

