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

#include "QBMWConfigMonitor.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <Utils/appType.h>
#include <QBSecureLogManager.h>
#include <main.h>

#if SV_LOG_LEVEL > 0
    SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "QBMWConfigLogLevel", "" );
    #define log_error(fmt, ...) do { if (env_log_level() >= 0) { SvLogError( COLBEG() "QBMWConfigMonitor :: " fmt COLEND_COL(red), ##__VA_ARGS__); } } while(0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice( COLBEG() "QBMWConfigMonitor :: " fmt COLEND_COL(yellow), ##__VA_ARGS__); } } while(0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 2) { SvLogNotice( COLBEG() "QBMWConfigMonitor :: " fmt COLEND_COL(cyan), ##__VA_ARGS__); } } while(0)
#else
    #define log_error(fmt, ...)
    #define log_state(fmt, ...)
    #define log_debug(fmt, ...)
#endif

#define INNOV8ON_REQUEST_VERSION "1"

#define REQUEST_TIMEOUT_MARGIN_SEC  2
#define MIN_INITIAL_RETRY_INTERVAL_MS 5000
#define MIN_MAX_RETRY_INTERVAL_MS (10 * 60 * 1000)
#define MIN_REQUESTS_INTERVAL_MS (10 * 60 * 1000)

struct QBMWConfigMonitor_t
{
    struct SvObject_ super_;

    AppGlobals appGlobals;

    bool started;

    SvXMLRPCRequest rpcReq;

    SvFiber fiber;
    SvFiberTimer timer;

    SvWeakList listeners;

    QBMWConfigMonitorRetryPolicy retryPolicy;
    unsigned int retryCounter;                  ///< Counts failed attempts to connect. Reset to 0 on successful connection.
    bool retriesStage;                          ///< @c true if we're looping trying to connect.
    bool hasConfiguration;                      ///< true if we get proper answer
};

SvLocal void
QBMWConfigMonitorStep(void *self_);
SvLocal void
QBMWConfigMonitorXMLRPCRequestStateChanged(SvGenericObject self_,
                                           SvXMLRPCRequest req,
                                           SvXMLRPCRequestState state);

SvLocal void
QBMWConfigMonitorDestroyRequest(QBMWConfigMonitor self)
{
    if (!self->rpcReq) {
        return;
    }

    SVRELEASE(self->rpcReq);
    self->rpcReq = NULL;
}

SvLocal void
QBMWConfigMonitor__dtor__(void *self_)
{
    QBMWConfigMonitor self = self_;

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
    }
    QBMWConfigMonitorDestroyRequest(self);

    SVRELEASE(self->listeners);
}

SvLocal void
QBMWConfigMonitorNotifyGotResponse(QBMWConfigMonitor self, SvHashTable response)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvGenericObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBMWConfigMonitorListener, listener, gotResponse, response);
    }
}

SvLocal void
QBMWConfigMonitorNotifyNoResponse(QBMWConfigMonitor self)
{
    SvIterator it = SvWeakListIterator(self->listeners);
    SvGenericObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBMWConfigMonitorListener, listener, noResponse);
    }
}

SvLocal void
QBMWConfigMonitorTryRequest(QBMWConfigMonitor self)
{
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString middlewareId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!serverInfo || !middlewareId || !hasNetwork) {
        QBMWConfigMonitorXMLRPCRequestStateChanged((SvObject) self, NULL, SvXMLRPCRequestState_error);
        return;
    }

    SvXMLRPCRequest rpcReq = SvXMLRPCRequestCreateWithJSONTemplate(serverInfo,
                                                                   SVSTRING("stb.GetConfiguration"), NULL,
                                                                   "[ %s ]", SvStringCString(middlewareId));

    if (unlikely(!rpcReq)) {
        SvLogError("%s failed to create XML-RPC request", __func__);
        QBMWConfigMonitorXMLRPCRequestStateChanged((SvObject) self, NULL, SvXMLRPCRequestState_error);
        QBSecureLogEvent("QBMWConfigMonitor", "Error.CubiMW.GetConfiguration_CreateFailed", "JSON:{\"description\":\"failed to create XML-RPC request\"}");
        return;
    }

    log_debug("%s(): START %s", __func__, SvStringCString(SvXMLRPCRequestGetDescription(rpcReq)));
    SvString jsonData = SvXMLRPCRequestCreateJSONLog(rpcReq, "");
    QBSecureLogEvent("QBMWConfigMonitor", "Notice.CubiMW.GetConfiguration", SvStringCString(jsonData));
    SVRELEASE(jsonData);

    self->rpcReq = rpcReq;
    const unsigned int reqTimeout = (self->retryPolicy.requestsIntervalMs / 1000) / 2 - REQUEST_TIMEOUT_MARGIN_SEC;     // real retryDelayMs is 16s minimum
    SvXMLRPCRequestSetTimeout(rpcReq, reqTimeout, NULL);
    SvXMLRPCRequestSetListener(rpcReq, (SvGenericObject) self, NULL);
    SvXMLRPCRequestStart(rpcReq, NULL);
}

SvLocal void QBMWConfigMonitorMiddlewareDataChanged(SvGenericObject self_, QBMiddlewareManagerType middlewareType)
{
    if (middlewareType != QBMiddlewareManagerType_Innov8on) {
        return;
    }

    QBMWConfigMonitor self = (QBMWConfigMonitor) self_;
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SvString id = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!serverInfo || !id || !hasNetwork) {
        QBMWConfigMonitorDestroyRequest(self);
        return;         // still waiting
    }

    if (self->fiber) {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
    }

    QBMWConfigMonitorDestroyRequest(self);

    self->retryCounter = 0;
    self->retriesStage = true;
    QBMWConfigMonitorTryRequest(self);
}

SvLocal int64_t QBMWConfigMonitorGetIntervalToNextRequestUs(QBMWConfigMonitor self)
{
    if (self->retriesStage) {
        const unsigned int maxRetryCount = 32;
        const unsigned int retryCount = self->retryCounter < maxRetryCount ? self->retryCounter : maxRetryCount;
        const uint64_t retryIntervalMs = self->retryPolicy.initialRetryIntervalMs * (1ll << retryCount);
        // maxRetryCount is big enough that retryIntervalMs should exceed maxRetryIntervalMs
        // and small enough to avoid getting 0 after shifting.
        const uint64_t intervalToNextRequestMs = retryIntervalMs < self->retryPolicy.maxRetryIntervalMs ? retryIntervalMs : self->retryPolicy.maxRetryIntervalMs;
        return intervalToNextRequestMs * 1000;
    } else {
        return self->retryPolicy.requestsIntervalMs * 1000;
    }
}

SvLocal int64_t
QBMWConfigMonitorGetDelay(int64_t minDelay, int64_t maxDelay)
{
    return minDelay + (((int64_t) rand() | ((int64_t) rand() << 32)) % (maxDelay - minDelay));
}

SvLocal void
QBMWConfigMonitorScheduleNextRequest(QBMWConfigMonitor self)
{
    if (!self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBMWConfigMonitor", QBMWConfigMonitorStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
    } else {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
    }

    const int64_t requestSchedulerIntervalUs = QBMWConfigMonitorGetIntervalToNextRequestUs(self);
    const int64_t maxTimeOffsetUs = requestSchedulerIntervalUs / 2;
    const int64_t randomBreakTimeUs = requestSchedulerIntervalUs + QBMWConfigMonitorGetDelay(-maxTimeOffsetUs, maxTimeOffsetUs);

    SvFiberTimerActivateAfter(self->timer, SvTimeFromUs(randomBreakTimeUs));
    log_debug("Next request after [%lld ms]", (long long int) randomBreakTimeUs / 1000);
}

SvLocal void
QBMWConfigMonitorStep(void *self_)
{
    QBMWConfigMonitor self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    self->retryCounter++;

    QBMWConfigMonitorTryRequest(self);
}

SvLocal void
QBMWConfigMonitorXMLRPCRequestStateChanged(SvGenericObject self_,
                                             SvXMLRPCRequest req,
                                             SvXMLRPCRequestState state)
{
    QBMWConfigMonitor self = (QBMWConfigMonitor) self_;
    if (state != SvXMLRPCRequestState_gotAnswer && state != SvXMLRPCRequestState_error &&
        state != SvXMLRPCRequestState_cancelled && state != SvXMLRPCRequestState_gotException) {
        return;
    }
    SvString jsonData = NULL;

    switch (state) {
        case SvXMLRPCRequestState_gotAnswer: {
            SvXMLRPCDataReader reader = (SvXMLRPCDataReader) SvXMLRPCRequestGetDataListener(req, NULL);
            SvHashTable results = (SvHashTable) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
            if (!SvObjectIsInstanceOf((SvGenericObject) results, SvHashTable_getType())) {
                QBMWConfigMonitorNotifyNoResponse(self);
                SvLogError("%s: invalid answer", __func__);
                jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
                QBSecureLogEvent("QBMWConfigMonitor", "Error.CubiMW.GetConfiguration_Failed", SvStringCString(jsonData));
                SVRELEASE(jsonData);
                goto fini;
            }
            jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
            QBSecureLogEvent("QBMWConfigMonitor", "Notice.CubiMW.GetConfiguration_OK", SvStringCString(jsonData));
            SVRELEASE(jsonData);

            QBMWConfigMonitorNotifyGotResponse(self, results);
            self->hasConfiguration = true;
            self->retriesStage = false;
            break;
        }
        case SvXMLRPCRequestState_gotException: {
            SvString desc;
            int code;
            SvXMLRPCRequestGetException(self->rpcReq, &code, &desc, NULL);
            QBMWConfigMonitorNotifyNoResponse(self);
            SvLogDebug("%s Exception: %d: %s", __func__, code, desc ? SvStringCString(desc) : "--");
            jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
            QBSecureLogEvent("QBMWConfigMonitor", "Error.CubiMW.GetConfiguration_Failed", SvStringCString(jsonData));
            SVRELEASE(jsonData);
        }
        default:
            jsonData = SvXMLRPCRequestCreateJSONLog(req, "");
            QBSecureLogEvent("QBMWConfigMonitor", "Notice.CubiMW.GetConfiguration_NoAnswer", SvStringCString(jsonData));
            SVRELEASE(jsonData);
            QBMWConfigMonitorNotifyNoResponse(self);
            self->hasConfiguration = false;
            if (!self->retriesStage) {
                self->retryCounter = 0;
                self->retriesStage = true;
            }
    }

fini:
    QBMWConfigMonitorDestroyRequest(self);
    QBMWConfigMonitorScheduleNextRequest(self);
}

SvLocal void
QBMWConfigMonitorXMLRPCRequestSetup(SvGenericObject self_, SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"), SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal SvType
QBMWConfigMonitor_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMWConfigMonitor__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        static struct SvXMLRPCClientListener_t xmlrpcMethods = {
            .stateChanged = QBMWConfigMonitorXMLRPCRequestStateChanged,
            .setup = QBMWConfigMonitorXMLRPCRequestSetup,
        };

        static struct QBMiddlewareManagerListener_t middlewareManagerListenerMethods = {
            .middlewareDataChanged = QBMWConfigMonitorMiddlewareDataChanged
        };

        SvTypeCreateManaged("QBMWConfigMonitor",
                            sizeof(struct QBMWConfigMonitor_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &xmlrpcMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareManagerListenerMethods,
                            NULL);
    }

    return type;
}

QBMWConfigMonitor
QBMWConfigMonitorCreate(AppGlobals appGlobals,
                        QBMWConfigMonitorRetryPolicy retryPolicy,
                        SvErrorInfo *errorOut)
{
    QBMWConfigMonitor self = NULL;
    SvErrorInfo error = NULL;

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed: appGlobals");
        goto err;
    }

    if (retryPolicy.initialRetryIntervalMs < MIN_INITIAL_RETRY_INTERVAL_MS) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Initial retry interval should be >= %d sec", MIN_INITIAL_RETRY_INTERVAL_MS / 1000);
        goto err;
    }

    if (retryPolicy.maxRetryIntervalMs < MIN_MAX_RETRY_INTERVAL_MS) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Maximum retry interval should be >= %d min", MIN_MAX_RETRY_INTERVAL_MS / (60 * 1000));
        goto err;
    }

    if (retryPolicy.requestsIntervalMs < MIN_REQUESTS_INTERVAL_MS) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Requests interval should be >= %d min", MIN_REQUESTS_INTERVAL_MS/ (60 * 1000));
        goto err;
    }

    if ( !(self = (QBMWConfigMonitor) SvTypeAllocateInstance(QBMWConfigMonitor_getType(), &error))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create QBMWConfigMonitor");
        goto err;
    }

    self->appGlobals = appGlobals;
    self->retryPolicy = retryPolicy;

    self->listeners = SvWeakListCreate(NULL);

err:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

void
QBMWConfigMonitorStart(QBMWConfigMonitor self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    if (self->started) {
        return;
    }

    if (!self->appGlobals->middlewareManager) {
        SvLogError("%s(): failed, QBMiddlewareManager is unavailable!", __func__);
        return;
    }

    QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    QBMWConfigMonitorDestroyRequest(self);

    self->retryCounter = 0;
    self->retriesStage = true;

    self->hasConfiguration = false;

    QBMWConfigMonitorTryRequest(self);

    self->started = true;
}

void
QBMWConfigMonitorStop(QBMWConfigMonitor self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    if (!self->started) {
        return;
    }

    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    }

    if (self->rpcReq) {
        SvXMLRPCRequestCancel(self->rpcReq, NULL);
        QBMWConfigMonitorDestroyRequest(self);
    }

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    self->started = false;
}

void
QBMWConfigMonitorAddListener(QBMWConfigMonitor self, SvGenericObject listener)
{
    if (!self || !listener) {
        SvLogError("%s() failed. NULL self passed.", __func__);
        return;
    }

    if (SvObjectIsImplementationOf(listener, QBMWConfigMonitorListener_getInterface())) {
        SvWeakListPushBack(self->listeners, listener, NULL);
    } else {
        SvLogWarning("%s(): listener %p of type %s doesn't implement QBMWConfigMonitorListener interface.", __func__, listener, listener ? SvObjectGetTypeName(listener) : "--");
    }
}

void
QBMWConfigMonitorRemoveListener(QBMWConfigMonitor self, SvGenericObject listener)
{
    if (!self || !listener) {
        SvLogError("%s() failed. NULL self passed.", __func__);
        return;
    }

    SvWeakListRemoveObject(self->listeners, listener);
}

SvInterface
QBMWConfigMonitorListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBMWConfigMonitorListener",
                                 sizeof(struct QBMWConfigMonitorListener_),
                                 NULL, &interface, NULL);
    }
    return interface;
}

bool
QBMWConfigMonitorGetRetryPolicy(QBMWConfigMonitor self, QBMWConfigMonitorRetryPolicy* outPolicy)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return false;
    }

    *outPolicy = self->retryPolicy;
    return true;
}

bool
QBMWConfigMonitorHasConfiguration(QBMWConfigMonitor self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return false;
    }

    return self->hasConfiguration;
}
