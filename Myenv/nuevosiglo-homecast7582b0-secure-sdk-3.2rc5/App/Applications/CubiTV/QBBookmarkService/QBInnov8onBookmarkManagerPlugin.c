/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBInnov8onBookmarkManagerPlugin.h"
#include <QBBookmarkService/QBBookmarkManagerPlugin.h>
#include <QBBookmarkService/QBBookmark.h>
#include <Services/core/QBPushReceiver.h>
#include <Services/core/QBMiddlewareManager.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvCore/SvErrorInfo.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCRequestsQueue.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <fibers/c/fibers.h>
#include <main_decl.h>
#include <main.h>
#include <QBSecureLogManager.h>

#define INNOV8ON_REQUEST_VERSION "2"

// TODO: (currently not supported by MW)
// - Inform MW that bookmark has changed
//   currently used only in follow-me, we have to change bookmark type so this info will be sent only once
//   (MW creates new bookmark for every setBookmark command, it should check if that bookmark exists and update it if necessary)
// - Inform MW that bookmark was removed
// - Handle "update" command from push server and change the name to more descriptive

// For these bookmarks we have to support two additional fields:
// deviceID - device id received from middleware - id of the calling device
// tag - unique bookmark id received from middleware

struct QBInnov8onBookmarkManagerPlugin_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvXMLRPCRequestsQueue xmlRPCReqQueue;

    QBBookmarkType *handledTypes;
    size_t handledTypesCnt;
};

SvLocal int
QBInnov8onBookmarkManagerPluginStart(SvObject self_,
                                     SvScheduler scheduler)
{
    SvErrorInfo error = NULL;
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    if (self->appGlobals->pushReceiver) {
        QBPushReceiverAddListener(self->appGlobals->pushReceiver, (SvObject) self, SVSTRING("update"), &error);
    }

    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        return -1;
    }

    return 0;
}

SvLocal int
QBInnov8onBookmarkManagerPluginStop(SvObject self_)
{
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;
    SvErrorInfo error = NULL;

    SvXMLRPCRequestsQueueCancelAllRequests(self->xmlRPCReqQueue);

    if (self->appGlobals->pushReceiver) {
        QBPushReceiverRemoveListener(self->appGlobals->pushReceiver, (SvObject) self, &error);
    }

    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        return -1;
    }

    return 0;
}

SvLocal int
QBInnov8onBookmarkManagerPluginSetup(SvObject self_,
                                     SvHashTable storage,
                                     QBBookmarkType *handledTypes,
                                     size_t typesCnt)
{
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    if (typesCnt > 0) {
        self->handledTypes = calloc(typesCnt, sizeof(QBBookmarkType));
        if (unlikely(!self->handledTypes)) {
            SvLogError("%s: can't allocate memory for array of types handled by innov8on bookmark manager plugin", __func__);
            return -1;
        }

        memcpy(self->handledTypes, handledTypes, typesCnt * sizeof(QBBookmarkType));
        self->handledTypesCnt = typesCnt;
    }

    return 0;
}

SvLocal int
QBInnov8onBookmarkManagerPluginSendBookmark(QBInnov8onBookmarkManagerPlugin self,
                                            QBBookmark bookmark)
{
    if (!self->appGlobals->middlewareManager) {
        SvLogError("%s: middlewareIdManager doesn't exist", __func__);
        return -1;
    }

    SvString customerId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!customerId || !serverInfo || !hasNetwork) {
        SvLogError("%s cannot create request - server=%p, customerId=%p, hasNetwork=%d", __func__, serverInfo, customerId, hasNetwork);
        return -1;
    }

    SvString productId = QBBookmarkGetContentId(bookmark);
    SvString tag = (SvString) QBBookmarkGetParameter(bookmark, (SvObject) SVSTRING("tag"));
    double position = QBBookmarkGetPosition(bookmark);
    SvImmutableArray params = SvImmutableArrayCreateWithTypedValues("@@d@", NULL, customerId, productId, position, tag);
    SvXMLRPCRequest req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(req, serverInfo, SVSTRING("stb.SetBookmark"), params, NULL);
    SvXMLRPCRequestSetListener(req, (SvObject) self, NULL);

    SvString jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
    QBSecureLogEvent("QBInnov8onBookmarkManagerPlugin", "Notice.CubiMW.SetBookmark", SvStringCString(jsonData));
    SVRELEASE(jsonData);
    SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, req);
    SVRELEASE(req);
    SVRELEASE(params);

    return 0;
}

SvLocal int
QBInnov8onBookmarkManagerPluginOnChangeBookmark(SvObject self_,
                                                QBBookmark bookmark)
{
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    if (!QBBookmarkManagerPluginIsTypeHandled(self->handledTypes, self->handledTypesCnt, QBBookmarkGetType(bookmark)))
        return 0;

    int retval = QBInnov8onBookmarkManagerPluginSendBookmark(self, bookmark);
    // HACK, read TODO at the top
    QBBookmarkRemoveParameter(bookmark, (SvObject) SVSTRING("tag"));
    QBBookmarkChangeType(bookmark, QBBookmarkType_Generic);
    //
    return retval;
}

SvLocal int
QBInnov8onBookmarkManagerPluginOnAddBookmark(SvObject self_,
                                             QBBookmark bookmark)
{
    return QBInnov8onBookmarkManagerPluginOnChangeBookmark(self_, bookmark);
}

SvLocal int
QBInnov8onBookmarkManagerPluginOnRemoveBookmark(SvObject self_,
                                                QBBookmark bookmark)
{
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    if (!QBBookmarkManagerPluginIsTypeHandled(self->handledTypes, self->handledTypesCnt, QBBookmarkGetType(bookmark)))
        return 0;

    return 0;
}

SvLocal void
QBInnov8onBookmarkManagerPluginPushDataReceived(SvObject self_,
                                                SvString type,
                                                SvObject data_)
{
    //QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    //SvHashTable data = (SvHashTable) data_;
    if (!SvStringEqualToCString(type, "update"))
        return;
}

SvLocal void
QBInnov8onBookmarkManagerPluginPushStatusChanged(SvObject self_,
                                                 QBPushReceiverStatus status)
{
}

SvLocal void
QBInnov8onBookmarkManagerPluginReqStateChanged(SvObject self_,
                                               SvXMLRPCRequest req,
                                               SvXMLRPCRequestState state)
{
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader) SvXMLRPCRequestGetDataListener(req, NULL);
        if (reader) {
            SvHashTable results = (SvHashTable) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
            if (results) {
                // TODO: Parse results
                SvString jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
                QBSecureLogEvent("QBInnov8onBookmarkManagerPlugin", "Notice.CubiMW.SetBookmark_OK", SvStringCString(jsonData));
                SVRELEASE(jsonData);
            }
        }
    } else if (state == SvXMLRPCRequestState_gotException) {
        SvString desc;
        int code;
        SvXMLRPCRequestGetException(req, &code, &desc, NULL);
        SvLogDebug("%s Exception: %d: %s", __func__, code, SvStringCString(desc));
        SvString jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
        QBSecureLogEvent("QBInnov8onBookmarkManagerPlugin", "Error.CubiMW.SetBookmark_Failed", SvStringCString(jsonData));
        SVRELEASE(jsonData);
    } else if (state == SvXMLRPCRequestState_error) {
        SvString jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
        QBSecureLogEvent("QBInnov8onBookmarkManagerPlugin", "Error.CubiMW.SetBookmark_Failed", SvStringCString(jsonData));
        SVRELEASE(jsonData);
    } else {
        SvString jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
        QBSecureLogEvent("QBInnov8onBookmarkManagerPlugin", "Error.CubiMW.SetBookmark_Failed", SvStringCString(jsonData));
        SVRELEASE(jsonData);
        return;
    }

    SvXMLRPCRequestsQueueFinishRequest(self->xmlRPCReqQueue);
    SvXMLRPCRequestsQueueProcessNext(self->xmlRPCReqQueue);
}

SvLocal void
QBInnov8onBookmarkManagerPluginReqSetup(SvObject self_,
                                        SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"), SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBInnov8onBookmarkManagerPluginDestroy(void *self_)
{
    QBInnov8onBookmarkManagerPlugin self = (QBInnov8onBookmarkManagerPlugin) self_;

    if (self->handledTypesCnt > 0)
        free(self->handledTypes);

    SVTESTRELEASE(self->xmlRPCReqQueue);
}

SvType
QBInnov8onBookmarkManagerPlugin_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onBookmarkManagerPluginDestroy
    };
    static SvType type = NULL;

    static const struct QBPushReceiverListener_t pushReceiverMethods = {
        .dataReceived = QBInnov8onBookmarkManagerPluginPushDataReceived,
        .statusChanged = QBInnov8onBookmarkManagerPluginPushStatusChanged
    };

    static const struct QBBookmarkManagerPlugin_t BMMethods = {
        .start = QBInnov8onBookmarkManagerPluginStart,
        .stop = QBInnov8onBookmarkManagerPluginStop,
        .setup = QBInnov8onBookmarkManagerPluginSetup,
        .onAddBookmark = QBInnov8onBookmarkManagerPluginOnAddBookmark,
        .onRemoveBookmark = QBInnov8onBookmarkManagerPluginOnRemoveBookmark,
        .onChangeBookmark = QBInnov8onBookmarkManagerPluginOnChangeBookmark
    };

    static const struct SvXMLRPCClientListener_t xmlrpcMethods = {
        .stateChanged = QBInnov8onBookmarkManagerPluginReqStateChanged,
        .setup = QBInnov8onBookmarkManagerPluginReqSetup,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onBookmarkManagerPlugin",
                            sizeof(struct QBInnov8onBookmarkManagerPlugin_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPushReceiverListener_getInterface(), &pushReceiverMethods,
                            QBBookmarkManagerPlugin_getInterface(), &BMMethods,
                            SvXMLRPCClientListener_getInterface(), &xmlrpcMethods,
                            NULL);
    }

    return type;
}

QBInnov8onBookmarkManagerPlugin
QBInnov8onBookmarkManagerPluginInit(QBInnov8onBookmarkManagerPlugin self,
                                    AppGlobals appGlobals)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!appGlobals) {
        SvLogError("%s: NULL appGlobals passed", __func__);
        return NULL;
    }

    SvXMLRPCRequestsQueue reqQueue = SvXMLRPCRequestsQueueCreate();
    if (!reqQueue) {
        SvLogError("%s: can't create SvXMLRPCRequestsQueue", __func__);
        return NULL;
    }

    self->appGlobals = appGlobals;
    self->xmlRPCReqQueue = reqQueue;

    return self;
}
