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

#include "QBAppVersion.h"

#include <main.h>
#include <settings.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <safeFiles.h>
#include <Utils/value.h>
#include <stringUtils.h>
#include <Services/core/QBMiddlewareManager.h>

//api Version
#define INNOV8ON_REQUEST_VERSION "2"

// Use this custom log carefully! Until we can't emulate full STB environment on host machine
// (like access to /dev/, /etc/ and other files) we will be 'hide' this problems on host machines.
#if defined __i386__ || defined __x86_64__
#define SvCustomLog_WarningOnHostErrorOnOther(format, ...) SvLogWarning(format, ## __VA_ARGS__)
#else
#define SvCustomLog_WarningOnHostErrorOnOther(format, ...) SvLogError(format, ## __VA_ARGS__)
#endif

struct QBAppVersion_
{
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvFiber      fiber;
    SvFiberTimer timer;

    SvXMLRPCRequest req;
    SvDataBucket    bucket;

    SvString version;
    SvString rc;
    SvString build;

    SvString full;
};

SvLocal char *
fileToString(const char * file)
{
    char *buf = NULL;
    QBFileToBuffer(file, &buf);

    if (!buf) {
        SvCustomLog_WarningOnHostErrorOnOther("QBAppVersion : Failed to load file %s", file);
        return NULL;
    }

    char *stripped = malloc(strlen(buf) + 1);
    QBStringUtilsStripWhitespace(stripped, strlen(buf) + 1, buf);
    free(buf);

    if (!*stripped)
        SvLogWarning("QBAppVersion : Empty File %s", file);

    return stripped;
}

// ------------------- Service --------------------

SvString
QBAppVersionGetVersion(const QBAppVersion self)
{
    return self->version;
}

SvString
QBAppVersionGetRC(const QBAppVersion self)
{
    return self->rc;
}

SvString
QBAppVersionGetBuild(const QBAppVersion self)
{
    return self->build;
}

SvString
QBAppVersionGetFullVersion(const QBAppVersion self)
{
    return self->full;
}

SvLocal void
QBAppVersionCancelRequest(QBAppVersion self)
{
    if (self->req) {
        SvXMLRPCRequestCancel(self->req, NULL);
        SVTESTRELEASE(self->req);
        self->req = NULL;
    }
}

SvLocal void
QBAppVersionRequestSetup(SvGenericObject self_, SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"),
                               SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBAppVersionNotifyMiddleware(QBAppVersion self)
{
    AppGlobals appGlobals = self->appGlobals;

    if (!self->full) {
        SvLogError("Can't send app version to middleware");
        return;
    }

    QBAppVersionCancelRequest(self);

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString customerId = QBMiddlewareManagerGetId(appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!server || !customerId || !hasNetwork) {
        SvLogWarning("%s cannot create request - server=%p, customerId=%p, hasNetwork=%d", __func__, server, customerId, hasNetwork);
        return;
    }

    SvImmutableArray params =
        SvImmutableArrayCreateWithTypedValues("@@", NULL, customerId, self->full);

    self->req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(self->req, server, SVSTRING("stb.SetVersion"), params, NULL);
    SVRELEASE(params);

    SvXMLRPCRequestSetListener(self->req, (SvGenericObject) self, NULL);
    SvXMLRPCRequestStart(self->req, NULL);
}

SvLocal void
QBAppVersionRequestStateChanged(SvGenericObject self_,
                               SvXMLRPCRequest req,
                               SvXMLRPCRequestState state)
{
    QBAppVersion self = (QBAppVersion) self_;

    if (req != self->req) {
        SvLogWarning("%s() : Notification from unknown request", __func__);
        return;
    }

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader)
            SvXMLRPCRequestGetDataListener(self->req, NULL);
        if (reader) {
            SvValue result = (SvValue)
                SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
            if (!result || !SvObjectIsInstanceOf((SvObject) result, SvValue_getType()) || !SvValueIsBoolean(result) || !SvValueGetBoolean(result)) {
                SvLogError("%s() : stb.SetVersion failed", __func__);
                SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(60*60, 0));
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
    } else if (state == SvXMLRPCRequestState_error) {
        SvLogError("%s() : stb.SetVersion failed", __func__);
        SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(60*60, 0));
    }

    if (state == SvXMLRPCRequestState_gotAnswer || state == SvXMLRPCRequestState_error ||
        state == SvXMLRPCRequestState_cancelled || state == SvXMLRPCRequestState_gotException) {

        SVTESTRELEASE(self->req);
        self->req = NULL;
    }
}

SvLocal void
QBAppVersionStep(void *self_)
{
    QBAppVersion self = self_;

    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    QBAppVersionNotifyMiddleware(self);
}

// QBMiddlewareManagerListener virtual methods

SvLocal void
QBAppVersionMiddlewareDataChanged(SvGenericObject self_,
                                  QBMiddlewareManagerType middlewareType)
{
    QBAppVersion self = (QBAppVersion) self_;

    if (middlewareType != QBMiddlewareManagerType_Innov8on)
        return;

    SvXMLRPCServerInfo server = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString mwId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (server && mwId && hasNetwork) {
        QBAppVersionNotifyMiddleware(self);
    } else {
        QBAppVersionCancelRequest(self);
    }
}

SvLocal void
QBAppVersion__dtor__(void *self_)
{
    QBAppVersion self = self_;

    if (self->fiber)
        SvFiberDestroy(self->fiber);

    SVTESTRELEASE(self->req);
    SVRELEASE(self->bucket);

    SVTESTRELEASE(self->version);
    SVTESTRELEASE(self->rc);
    SVTESTRELEASE(self->build);
    SVTESTRELEASE(self->full);
}

SvLocal SvType
QBAppVersion_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAppVersion__dtor__
    };
    static SvType type = NULL;

    static const struct SvXMLRPCClientListener_t listenerMethods = {
        .stateChanged   = QBAppVersionRequestStateChanged,
        .setup          = QBAppVersionRequestSetup
    };

    static const struct QBMiddlewareManagerListener_t middlewareListenerMethods = {
        .middlewareDataChanged = QBAppVersionMiddlewareDataChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBAppVersion",
                            sizeof(struct QBAppVersion_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &listenerMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareListenerMethods,
                            NULL);
    }

    return type;
}

QBAppVersion
QBAppVersionCreate(AppGlobals appGlobals)
{
    QBAppVersion self = (QBAppVersion)
        SvTypeAllocateInstance(QBAppVersion_getType(), NULL);
    self->appGlobals = appGlobals;

    self->req = NULL;
    self->bucket = SvDataBucketCreate(NULL);

    char *version = fileToString("/etc/VERSION");
    char *rc = fileToString("/etc/RC");
    char *build = fileToString("/etc/BUILD");

    bool error = false;

    if (version && *version) {
        self->version = SvStringCreate(version, NULL);
    } else {
        SvCustomLog_WarningOnHostErrorOnOther("%s() : Can't obtain application version", __func__);
        self->version = SVSTRING("UNKNOWN");
        error = true;
    }

    if (rc) {
        self->rc = SvStringCreate(rc, NULL);
    } else {
        self->rc = SVSTRING("UNKNOWN");
        SvCustomLog_WarningOnHostErrorOnOther("%s() : Can't obtain application rc", __func__);
        error = true;
    }

    if (build && *build) {
        self->build = SvStringCreate(build, NULL);
    } else {
        self->build = SVSTRING("UNKNOWN");
        SvCustomLog_WarningOnHostErrorOnOther("%s() : Can't obtain application build", __func__);
        error = true;
    }

    if (error)
        self->full = SVSTRING("UNKNOWN");
    else if (rc && *rc)
        self->full = SvStringCreateWithFormat("%s-%src%s", build, version, rc);
    else
        self->full = SvStringCreateWithFormat("%s-%s", build, version);

    SvLogNotice("App version : %s", SvStringCString(self->full));

    free(version);
    free(rc);
    free(build);

    return self;
}

void
QBAppVersionStart(QBAppVersion self)
{
    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager,
                                       (SvGenericObject) self);
    }
    if (!self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL,
                                    "QBAppVersion",
                                    QBAppVersionStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
        SvFiberActivate(self->fiber);
    }
}

void
QBAppVersionStop(QBAppVersion self)
{
    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager,
                                          (SvGenericObject) self);
    }
    QBAppVersionCancelRequest(self);
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }
}

