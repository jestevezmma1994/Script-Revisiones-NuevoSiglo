/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRedirProxyManager.h"

#include <QBContentManager/Innov8onProviderTransaction.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvURL.h>
#include <SvXMLRPCClient/SvXMLRPCDataReader.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <QBConf.h>
#include <main.h>

#include <stdlib.h>

#define log_state(fmt, ...) if (1) { SvLogNotice( COLBEG() "QBRedirProxyManager :: " fmt COLEND_COL(cyan), ##__VA_ARGS__); };
#define log_debug(fmt, ...) if (0) { SvLogNotice( COLBEG() "QBRedirProxyManager :: " fmt COLEND_COL(red), ##__VA_ARGS__); };


#define INNOV8ON_REQUEST_VERSION "2"

#define MIN_INITIAL_RETRY_INTERVAL_MS (5 * 1000)
#define MIN_MAX_RETRY_INTERVAL_MS (10 * 60 * 1000)


typedef enum QBRedirProxyManagerState_ {
    QBRedirProxyManagerState_Stopped,
    QBRedirProxyManagerState_WaitingOnRedirProxy,
    QBRedirProxyManagerState_Ready
} QBRedirProxyManagerState;


struct QBRedirProxyManager_t
{
    struct SvObject_ super_;

    AppGlobals appGlobals;

    QBRedirProxyManagerState state;

    SvXMLRPCRequest rpcReq;
    SvString innov8onUrl;
    SvString mwId;

    SvFiber fiber;
    SvFiberTimer timer;

    unsigned int initialRetryIntervalMs;    ///< initial interval between requests [ms]
    unsigned int maxRetryIntervalMs;        ///< maximum interval between requests [ms]
    unsigned int retryCounter;              ///< Counts failed attempts to connect. Reset to 0 on successful connection.
};

SvLocal void
QBRedirProxyManagerDestroyRequest(QBRedirProxyManager self)
{
    if (!self->rpcReq) {
        return;
    }

    SVRELEASE(self->rpcReq);
    self->rpcReq = NULL;
}

SvLocal void
QBRedirProxyManager__dtor__(void *self_)
{
    QBRedirProxyManager self = self_;
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
    }

    QBRedirProxyManagerDestroyRequest(self);
    SVTESTRELEASE(self->innov8onUrl);
    SVTESTRELEASE(self->mwId);
}

SvLocal void
QBRedirProxyManagerChangeState(QBRedirProxyManager self, QBRedirProxyManagerState newState)
{
    if (newState == self->state)
        return;

    if (newState == QBRedirProxyManagerState_Ready) {
        QBMiddlewareManagerSetURL(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on, self->innov8onUrl);
    } else if (newState == QBRedirProxyManagerState_WaitingOnRedirProxy) {
        self->retryCounter = 0;
    }

    self->state = newState;
}

SvLocal void
QBRedirProxyManagerTryRequest(QBRedirProxyManager self)
{
    const char* mwPrefix = QBConfigGet("PREFIX");
    if (!mwPrefix) {
        SvLogWarning("CubiTV: Innov8on server URL not set!");
        return;
    }

    bool hasNetwork = QBMiddlewareManagerHasNetwork(self->appGlobals->middlewareManager);
    if (!hasNetwork) {
        log_debug("try request - waiting for network");
        return;
    }

    SvString middlewareId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    SVTESTRELEASE(self->mwId);
    self->mwId = SVTESTRETAIN(middlewareId);
    if (!middlewareId) {
        log_debug("try request - waiting for mw Id");
        return;
    }

    SvString prefix_url_innov8on = SvStringCreate(mwPrefix, NULL);

    SvXMLRPCServerInfo serverInfo = (SvXMLRPCServerInfo) SvTypeAllocateInstance(SvXMLRPCServerInfo_getType(), NULL);
    SvXMLRPCServerInfoInit(serverInfo, prefix_url_innov8on, NULL);
    SvXMLRPCServerInfoSetSSLParams(serverInfo, self->appGlobals->sslParams, NULL);
    SVRELEASE(prefix_url_innov8on);

    SvXMLRPCRequest rpcReq = SvXMLRPCRequestCreateWithJSONTemplate(serverInfo,
                                                                   SVSTRING("stb.GetAddress"), NULL,
                                                                   "[ %s ]", SvStringCString(middlewareId));
    SVRELEASE(serverInfo);

    if (unlikely(!rpcReq)) {
        SvLogError("%s failed to create XML-RPC request", __func__);
        return;
    }

    log_debug("%s(): START %s", __func__, SvStringCString(SvXMLRPCRequestGetDescription(rpcReq)));
    self->rpcReq = rpcReq;
    const unsigned int reqTimeout = (self->maxRetryIntervalMs / 1000) / 2 - 2;
    SvXMLRPCRequestSetTimeout(rpcReq, reqTimeout, NULL);
    SvXMLRPCRequestSetListener(rpcReq, (SvGenericObject) self, NULL);
    SvXMLRPCRequestStart(rpcReq, NULL);
}

SvLocal void
QBRedirProxyManagerMiddlewareDataChanged(SvGenericObject self_, QBMiddlewareManagerType idType)
{
    QBRedirProxyManager self = (QBRedirProxyManager) self_;

    if (idType != QBMiddlewareManagerType_Innov8on) {
        return;
    }

    log_debug("middleware data changed");
    SvString middlewareId = QBMiddlewareManagerGetId(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on);
    if (SvObjectEquals((SvObject) self->mwId, (SvObject) middlewareId)) {
        log_debug("middleware id hasn't changed");
        return;
    }

    if (self->fiber) {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
    }

    QBRedirProxyManagerDestroyRequest(self);

    // middleware Id has changed -> invalidate innov8onUrl
    SVTESTRELEASE(self->innov8onUrl);
    self->innov8onUrl = NULL;

    if (middlewareId) {
        log_debug("new middleware id -> new (temporary) prefix url: NULL");
        QBRedirProxyManagerTryRequest(self);
        QBRedirProxyManagerChangeState(self, QBRedirProxyManagerState_WaitingOnRedirProxy);
    } else {
        // lost middleware id
        SVRELEASE(self->mwId);
        self->mwId = NULL;
        log_debug("new middleware id = NULL -> new prefix url: NULL");
    }

    log_debug("notify middleware manager");
    QBMiddlewareManagerSetURL(self->appGlobals->middlewareManager, QBMiddlewareManagerType_Innov8on, NULL);
}

SvLocal void
QBRedirProxyManagerStep(void *self_)
{
    QBRedirProxyManager self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    self->retryCounter++;
    QBRedirProxyManagerTryRequest(self);
}

SvLocal int64_t
QBRedirProxyManagerGetIntervalToNextRequestUs(QBRedirProxyManager self)
{
    const unsigned int maxRetryCount = 32;
    const unsigned int retryCount = self->retryCounter < maxRetryCount ? self->retryCounter : maxRetryCount;
    const int64_t retryIntervalMs = self->initialRetryIntervalMs * (1ll << retryCount);
    // maxRetryCount is big enough that retryIntervalMs should exceed maxRetryIntervalMs
    // and small enough to avoid getting 0 after shifting.
    const int64_t intervalToNextRequestMs = retryIntervalMs < self->maxRetryIntervalMs ? retryIntervalMs : self->maxRetryIntervalMs;
    return intervalToNextRequestMs * 1000;
}

SvLocal int64_t
QBRedirProxyManagerGetDelay(int64_t minDelay, int64_t maxDelay)
{
    return minDelay + (((int64_t) rand() | ((int64_t) rand() << 32)) % (maxDelay - minDelay));
}

SvLocal void
QBRedirProxyManagerScheduleRequest(QBRedirProxyManager self)
{
    if (!self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBRedirProxyManager", QBRedirProxyManagerStep, self);
        self->timer = SvFiberTimerCreate(self->fiber);
    } else {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
    }

    const int64_t requestSchedulerIntervalUs = QBRedirProxyManagerGetIntervalToNextRequestUs(self);
    const int64_t maxTimeOffsetUs = requestSchedulerIntervalUs / 2;
    const int64_t randomBreakTimeUs = requestSchedulerIntervalUs + QBRedirProxyManagerGetDelay(-maxTimeOffsetUs, maxTimeOffsetUs);

    SvFiberTimerActivateAfter(self->timer, SvTimeFromUs(randomBreakTimeUs));
    log_debug("Next request after [%lld ms]", (long long int) randomBreakTimeUs / 1000);
}

SvLocal bool
QBRedirProxyManagerGetRedirData(QBRedirProxyManager self, SvHashTable results)
{
    bool retry = true;

    SVAUTOSTRING(keyVal, "qb_middleware_address");
    SvValue answer = (SvValue) SvHashTableFind(results, (SvObject) keyVal);
    if (!SvObjectIsInstanceOf((SvObject) answer, SvValue_getType()) || !SvValueIsString(answer)) {
        SvLogError("%s: invalid answer", __func__);
        goto fini;
    }

    SvString urlStr = (SvString)SvValueGetString(answer);
    if ( (strncmp(SvStringCString(urlStr), "https://", 8) != 0) &&
         (strncmp(SvStringCString(urlStr), "http://", 7) != 0) ) {
        SvLogError("%s: invalid url format: %s", __func__, SvStringCString(urlStr));
        goto fini;
    }

    // answer OK
    retry = false;

    if (SvObjectEquals((SvObject) self->innov8onUrl, (SvObject) urlStr)) {
        log_debug("innov8onUrl hasn't changed");
        goto fini;
    }

    SVTESTRELEASE(self->innov8onUrl);
    self->innov8onUrl = SVRETAIN(urlStr);
    log_state("Got Innov8ton url: %s", SvStringCString(self->innov8onUrl));

fini:
    return retry;
}

SvLocal void
QBRedirProxyManagerXMLRPCRequestStateChanged(SvGenericObject self_,
                                             SvXMLRPCRequest req,
                                             SvXMLRPCRequestState state)
{
    QBRedirProxyManager self = (QBRedirProxyManager) self_;

    if (state != SvXMLRPCRequestState_gotAnswer && state != SvXMLRPCRequestState_error &&
        state != SvXMLRPCRequestState_cancelled && state != SvXMLRPCRequestState_gotException) {
        return;
    }

    bool retry = true;

    if (state == SvXMLRPCRequestState_gotAnswer) {
        SvXMLRPCDataReader reader = (SvXMLRPCDataReader) SvXMLRPCRequestGetDataListener(req, NULL);
        SvHashTable results = (SvHashTable) SvXMLRPCDataReaderGetDataItem(reader, 0, NULL);
        if (!SvObjectIsInstanceOf((SvObject) results, SvHashTable_getType())) {
            SvLogError("%s: invalid answer", __func__);
            goto fini;
        }

        retry = QBRedirProxyManagerGetRedirData(self, results);

        if (retry) {
            goto fini;
        }

        QBRedirProxyManagerChangeState(self, QBRedirProxyManagerState_Ready);
    } else if (state == SvXMLRPCRequestState_gotException) {
        SvString desc;
        int code;
        SvXMLRPCRequestGetException(self->rpcReq, &code, &desc, NULL);
        SvLogDebug("%s Exception: %d: %s", __func__, code, SvStringCString(desc));
    }
    // if state == SvXMLRPCRequestState_cancelled or state == SvXMLRPCRequestState_error -> just retry

fini:
    if (retry) {
        QBRedirProxyManagerScheduleRequest(self);       // schedule next request
    }

    QBRedirProxyManagerDestroyRequest(self);
}

SvLocal void
QBRedirProxyManagerXMLRPCRequestSetup(SvGenericObject self_, SvXMLRPCRequest req)
{
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"), SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal SvType
QBRedirProxyManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBRedirProxyManager__dtor__
    };
    static SvType type = NULL;

    static const struct SvXMLRPCClientListener_t xmlrpcMethods = {
        .stateChanged = QBRedirProxyManagerXMLRPCRequestStateChanged,
        .setup = QBRedirProxyManagerXMLRPCRequestSetup,
    };

    static const struct QBMiddlewareManagerListener_t middlewareIdListener = {
        .middlewareDataChanged = QBRedirProxyManagerMiddlewareDataChanged
    };

    if (!type) {
        SvTypeCreateManaged("QBRedirProxyManager",
                            sizeof(struct QBRedirProxyManager_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &xmlrpcMethods,
                            QBMiddlewareManagerListener_getInterface(), &middlewareIdListener,
                            NULL);
    }

    return type;
}

QBRedirProxyManager
QBRedirProxyManagerCreate(AppGlobals appGlobals,
                          unsigned int initialRetryIntervalMs,
                          unsigned int maxRetryIntervalMs,
                          SvErrorInfo *errorOut)
{
    QBRedirProxyManager self = NULL;
    SvErrorInfo error = NULL;

    const char *status = getenv("QBNoRedir");
    if (status && !strcmp(status, "1")) {
        log_state("Redirection service disabled using QBNoRedir environment variable");
        return NULL;
    }

    if (!appGlobals) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed: appGlobals");
        goto err;
    }

    if (initialRetryIntervalMs < MIN_INITIAL_RETRY_INTERVAL_MS) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "redir proxy initial retry interval should be >= %d s", MIN_INITIAL_RETRY_INTERVAL_MS / 1000);
        goto err;
    }

    if (maxRetryIntervalMs < MIN_MAX_RETRY_INTERVAL_MS) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "redir proxy max retry interval should be >= %d min", MIN_MAX_RETRY_INTERVAL_MS / (60 * 1000));
    }

    if ( !(self = (QBRedirProxyManager) SvTypeAllocateInstance(QBRedirProxyManager_getType(), &error))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create QBRedirProxyManager");
        goto err;
    }

    self->appGlobals = appGlobals;
    self->state = QBRedirProxyManagerState_Stopped;
    self->initialRetryIntervalMs = initialRetryIntervalMs;
    self->maxRetryIntervalMs = maxRetryIntervalMs;
    self->retryCounter = 0;

err:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

void
QBRedirProxyManagerStart(QBRedirProxyManager self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    if (!self->appGlobals->middlewareManager) {
        SvLogError("%s(): failed, QBMiddlewareManager is unavailable!", __func__);
        return;
    }

    if (self->state != QBRedirProxyManagerState_Stopped) {
        SvLogError("%s RedirProxyManager is still active", __func__);
        return;
    }

    QBMiddlewareManagerAddListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    QBRedirProxyManagerDestroyRequest(self);
    QBRedirProxyManagerTryRequest(self);

    QBRedirProxyManagerChangeState(self, QBRedirProxyManagerState_WaitingOnRedirProxy);
}

void
QBRedirProxyManagerStop(QBRedirProxyManager self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return;
    }

    if (self->appGlobals->middlewareManager) {
        QBMiddlewareManagerRemoveListener(self->appGlobals->middlewareManager, (SvGenericObject) self);
    }

    SvXMLRPCRequestCancel(self->rpcReq, NULL);
    QBRedirProxyManagerDestroyRequest(self);
    SVTESTRELEASE(self->innov8onUrl);
    self->innov8onUrl = NULL;

    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    QBRedirProxyManagerChangeState(self, QBRedirProxyManagerState_Stopped);
}

SvString
QBRedirProxyManagerGetMiddlewareUrl(QBRedirProxyManager self)
{
    if (unlikely(!self)) {
        SvLogError("%s NULL argument passed", __func__);
        return NULL;
    }

    return self->innov8onUrl;
}
