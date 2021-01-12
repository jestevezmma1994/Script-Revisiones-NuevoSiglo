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

#include "QBAdService.h"

#include <main.h>
#include <settings.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <SWL/icon.h>
#include <Utils/adaptField.h>

#define log_debug(fmt, ...) \
    do { if (0) { SvLogNotice(COLBEG() "%s() :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); } } while (0)

//api Version
#define INNOV8ON_REQUEST_VERSION "2"

struct QBAdService_t
{
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvXMLRPCRequest req;
    SvDataBucket    bucket;

    SvString customerId;

    struct {
        int id;
        int remainingViews;
        int views;
        SvString url;
    } currentAd;

    SvWeakList listeners;

    int lastAdUpdateDay;
};

// ------------------- Service --------------------

SvLocal void
QBAdServiceCancelRequest(QBAdService self)
{
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
QBAdServiceClearAd(QBAdService self)
{
    self->currentAd.id = 0;
    self->currentAd.remainingViews = 0;
    self->currentAd.views = 0;
    SVTESTRELEASE(self->currentAd.url);
    self->currentAd.url = NULL;
}

SvLocal void
QBAdServiceUpdateAd(QBAdService self)
{
    AppGlobals appGlobals = self->appGlobals;

    SvImmutableArray params = NULL;

    QBAdServiceCancelRequest(self);

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    self->customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(appGlobals->middlewareManager);
    if (!server || !self->customerId || !hasNetwork) {
        SvLogWarning("%s cannot create request - server=%p, customerId=%p, hasNetwork=%d", __func__, server, self->customerId, hasNetwork);
        return;
    }

    params = SvImmutableArrayCreateWithTypedValues("@ii", NULL, self->customerId,
                                                   self->currentAd.id,
                                                   self->currentAd.views);

    self->req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(self->req, server, SVSTRING("stb.GetNextAdvert"), params, NULL);
    SVRELEASE(params);

    SvXMLRPCRequestSetListener(self->req, (SvGenericObject) self, NULL);
    SvXMLRPCRequestStart(self->req, NULL);

    struct tm currentTime;
    SvTimeBreakDown(SvTimeGetCurrentTime(), true, &currentTime);
    self->lastAdUpdateDay = currentTime.tm_mday;
}

SvWidget
QBAdServiceGetAd(QBAdService self, const char* settings)
{
    struct tm currentTime;
    SvTimeBreakDown(SvTimeGetCurrentTime(), true, &currentTime);

    if (!self->customerId || currentTime.tm_mday != self->lastAdUpdateDay)
        QBAdServiceUpdateAd(self);
    if (!self->currentAd.id)
        return NULL;

    const char* settingsName = settings;
    if (!settingsName)
        settingsName = "Ad.settings";

    svSettingsPushComponent(settingsName);
    SvWidget icon = svIconNew(self->appGlobals->res, "AdIcon");

    if (icon) {
        icon->off_x = svSettingsGetInteger("AdIcon", "xOffset", 0);
        icon->off_y = svSettingsGetInteger("AdIcon", "yOffset", 0);

        svIconSetBitmapFromURI(icon, 0, SvStringCString(self->currentAd.url));

        if (self->currentAd.remainingViews > 0) {
            self->currentAd.remainingViews--;
            self->currentAd.views++;
        }
        if (self->currentAd.remainingViews <= 0) {
            return NULL;
        }
        QBAdServiceUpdateAd(self);
    }

    svSettingsPopComponent();
    return icon;
}

SvLocal void
QBAdServiceProcessData(QBAdService self, SvDBRawObject ad)
{
    SvString prefixUrl = QBMiddlewareManagerGetPrefixUrl(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (!prefixUrl) {
        return;
    }
    adaptField(ad, "thumbnail", prefixUrl);

    SvValue id = NULL, views = NULL, url = NULL;
    id = SvDBObjectGetID((SvDBObject) ad);
    views = (SvValue) SvDBRawObjectGetAttrValue(ad, "remainingViews");
    url = (SvValue) SvDBRawObjectGetAttrValue(ad, "thumbnail");

    self->currentAd.id = atoi(SvValueGetStringAsCString(id, NULL));
    self->currentAd.remainingViews = SvValueGetInteger(views);
    self->currentAd.views = 0;
    SVTESTRELEASE(self->currentAd.url);
    SvString adUrl = SvValueGetString(url);
    self->currentAd.url = SVRETAIN(adUrl);
    log_debug("Current ad { id : %d, remainingViews : %d, views : %d, url : %s }",
              self->currentAd.id, self->currentAd.remainingViews,
              self->currentAd.views, SvStringCString(self->currentAd.url));
}

SvLocal void
QBAdServiceNotify(QBAdService self)
{
    SvGenericObject listener;
    SvIterator iter = SvWeakListIterator(self->listeners);
    while ((listener = SvIteratorGetNext(&iter)))
        SvInvokeInterface(QBAdServiceListener, listener, currentAdChanged);
}

SvLocal void
QBAdServiceRequestSetup(SvGenericObject self_, SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"),
                               SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBAdServiceRequestStateChanged(SvGenericObject self_,
                               SvXMLRPCRequest req,
                               SvXMLRPCRequestState state)
{
    QBAdService self = (QBAdService) self_;

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
                SVAUTOSTRING(productsKey, "products");
                SvArray products = (SvArray) SvHashTableFind(results, (SvGenericObject) productsKey);
                if (products && SvArrayCount(products) > 0) {
                    SvValue xmlV = (SvValue) SvArrayAt(products, 0);
                    SvString xml = SvValueGetString(xmlV);
                    SvDBObject ad = SvDataBucketUpdate(self->bucket,
                                                       SvStringCString(xml),
                                                       NULL);
                    int lastId = self->currentAd.id;
                    int lastRemainingViews = self->currentAd.remainingViews;
                    SvString lastUrl = self->currentAd.url;
                    QBAdServiceProcessData(self, (SvDBRawObject) ad);
                    if (lastId != self->currentAd.id ||
                        lastRemainingViews != self->currentAd.remainingViews ||
                        !SvObjectEquals((SvGenericObject) lastUrl, (SvGenericObject) self->currentAd.url))
                        QBAdServiceNotify(self);
                    SVRELEASE(ad);
                } else {
                    log_debug("%s() : No products in reply, current advert will be cleared", __func__);
                    QBAdServiceClearAd(self);
                    QBAdServiceNotify(self);
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
    }

    if (state == SvXMLRPCRequestState_gotAnswer || state == SvXMLRPCRequestState_error ||
        state == SvXMLRPCRequestState_cancelled || state == SvXMLRPCRequestState_gotException) {

        SVTESTRELEASE(self->req);
        self->req = NULL;
    }
}

SvLocal void QBAdService__dtor__(void *self_)
{
    QBAdService self = self_;

    SVTESTRELEASE(self->req);
    SVRELEASE(self->bucket);
    SVTESTRELEASE(self->currentAd.url);
    SVTESTRELEASE(self->listeners);
}

SvLocal void QBAdServiceMiddlewareDataChanged(SvGenericObject self_, QBMiddlewareManagerType type)
{
    QBAdService self = (QBAdService) self_;
    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString id = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!server || !id || !hasNetwork) {
        return;
    }

    QBAdServiceUpdateAd(self);
}

SvLocal SvType
QBAdService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAdService__dtor__
    };
    static SvType type = NULL;

    static const struct SvXMLRPCClientListener_t listenerMethods = {
        .stateChanged   = QBAdServiceRequestStateChanged,
        .setup          = QBAdServiceRequestSetup
    };

    static const struct QBMiddlewareManagerListener_t middlewareManagerListenerMethods = {
        .middlewareDataChanged = QBAdServiceMiddlewareDataChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBAdService",
                            sizeof(struct QBAdService_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &listenerMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareManagerListenerMethods,
                            NULL);
    }

    return type;
}

QBAdService
QBAdServiceCreate(AppGlobals appGlobals)
{
    QBAdService self = (QBAdService) SvTypeAllocateInstance(QBAdService_getType(), NULL);
    self->appGlobals = appGlobals;

    self->req = NULL;
    self->bucket = SvDataBucketCreate(NULL);

    self->customerId = NULL;

    self->currentAd.id = 0;
    self->currentAd.remainingViews = 0;
    self->currentAd.views = 0;
    self->currentAd.url = NULL;

    self->listeners = SvWeakListCreate(NULL);

    return self;
}

void
QBAdServiceStart(QBAdService self)
{
    self->lastAdUpdateDay = -1;
    QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    QBAdServiceUpdateAd(self);
}

void
QBAdServiceStop(QBAdService self)
{
    QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    QBAdServiceCancelRequest(self);
}

SvInterface
QBAdServiceListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if(!interface) {
        SvInterfaceCreateManaged("QBAdServiceListener", sizeof(struct QBAdServiceListener_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

void
QBAdServiceAddListener(QBAdService self,
                       SvGenericObject listener,
                       SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (unlikely(!self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto fini;
    }

    if (!listener || !SvObjectIsImplementationOf(listener, QBAdServiceListener_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid listener passed");
        goto fini;
    }

    if (!self->listeners && unlikely(!(self->listeners = SvWeakListCreate(&error)))) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't create SvWeakList");
        goto fini;
    }

    SvWeakListPushBack(self->listeners, listener, NULL);

fini:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBAdServiceRemoveListener(QBAdService self,
                          SvGenericObject listener,
                          SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (unlikely(!self)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto fini;
    } else if (!listener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL listener passed");
        goto fini;
    }

    if (!self->listeners || !SvWeakListRemoveObject(self->listeners, listener)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "listener is not registered");
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
}
