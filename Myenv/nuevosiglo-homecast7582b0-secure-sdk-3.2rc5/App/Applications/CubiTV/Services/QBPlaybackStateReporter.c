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

#include "QBPlaybackStateReporter.h"

#include <Services/core/QBMiddlewareManager.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvXMLRPCClient/SvXMLRPCRequest.h>
#include <SvXMLRPCClient/SvXMLRPCRequestsQueue.h>
#include <SvXMLRPCClient/SvXMLRPCClientListener.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvCore/SvEnv.h>
#include <fibers/c/fibers.h>
#include <stdbool.h>

#if SV_LOG_LEVEL > 0
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBPlaybackStateReporterLogLevel", "");
#define log_error(fmt, ...)     do { { SvLogError(COLBEG() "%s():%d " fmt COLEND_COL(red), __func__, __LINE__, ## __VA_ARGS__); }; } while (0)
#define log_warning(fmt, ...)   do { { SvLogWarning(COLBEG() "%s:%d " fmt COLEND_COL(yellow), __func__, __LINE__, ## __VA_ARGS__); }; } while (0)
#define log_notice(fmt, ...)    do { if (env_log_level() >= 1) { SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(blue), __func__, __LINE__, ## __VA_ARGS__); }; } while (0)
#define log_debug(fmt, ...)     do { if (env_log_level() >= 2) { SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(cyan), __func__, __LINE__, ## __VA_ARGS__); }; } while (0)
#define log_fun(fmt, ...)       do { if (env_log_level() >= 3) { SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(green), __func__, __LINE__, ## __VA_ARGS__); }; } while (0)
#else
#define log_error(fmt, ...)
#define log_warning(fmt, ...)
#define log_notice(fmt, ...)
#define log_debug(fmt, ...)
#define log_fun(fmt, ...)
#endif

#define INNOV8ON_REQUEST_VERSION "2"
#define MAX_IGNORE_TIME_MS 2000 // 2s
#define SESSION_ID_SIZE_B 16 // 16 bytes for session id

typedef struct QBPlaybackStateReporterRequestParams_ {
    struct SvObject_ super_;    ///< super object

    double speed;               ///< current playback speed
    int position;               ///< current playback position
    int timeshiftDelay;         ///< current timeshift
    SvValue productId;          ///< id of played product
    SvString title;             ///< title of played product
} *QBPlaybackStateReporterRequestParams;

struct QBPlaybackStateReporter_ {
    struct SvObject_ super_;    ///< super object

    QBMiddlewareManager mwManager;  ///< middleware manager reference
    SvString sessionId;             ///< generated session id for mw communication

    QBPlaybackStateReporterRequestParams currentParams; // current playback params
    SvWeakReference currentPlayer;  ///< player that started current playback
    bool forceRequest;              ///< if request is forced to be sent despite same params

    SvXMLRPCRequestsQueue xmlRPCReqQueue;   ///< queued mw requests

    SvDeque waitingRequests;        ///< request waiting for timeout to pass
    SvFiber waitingFiber;           ///< fiber for above timeout
    SvFiberTimer waitingFiberTimer; ///< timer for above fiber

    bool started;                   ///< indicates if service was started
};

SvInterface
QBPlaybackStateReporterDataSource_getInterface(void)
{
    static SvInterface interface = NULL;
    if (unlikely(!interface)) {
        SvInterfaceCreateManaged("QBPlaybackStateReporterDataSource",
                                 sizeof(struct QBPlaybackStateReporterDataSource_),
                                 NULL, &interface, NULL);
    }
    return interface;
}


SvLocal void
QBPlaybackStateReporterRequestParamsDestroy(void *self_)
{
    log_fun();
    QBPlaybackStateReporterRequestParams self = (QBPlaybackStateReporterRequestParams) self_;
    SVTESTRELEASE(self->productId);
    SVTESTRELEASE(self->title);
}

SvLocal SvType
QBPlaybackStateReporterRequestParams_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPlaybackStateReporterRequestParamsDestroy
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPlaybackStateReporterRequestParams",
                            sizeof(struct QBPlaybackStateReporterRequestParams_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBPlaybackStateReporterRequestParams
QBPlaybackStateReporterRequestParamsCreate(void)
{
    log_fun();
    QBPlaybackStateReporterRequestParams self =
        (QBPlaybackStateReporterRequestParams) SvTypeAllocateInstance(QBPlaybackStateReporterRequestParams_getType(), NULL);
    return self;
}

SvLocal void
QBPlayBackStateReporterXMLRPCRequestStateChanged(SvObject self_,
                                                 SvXMLRPCRequest req,
                                                 SvXMLRPCRequestState state)
{
    log_fun();
    QBPlaybackStateReporter self = (QBPlaybackStateReporter) self_;

    if (state == SvXMLRPCRequestState_gotAnswer) {
        log_notice("got answer");
    } else if (state == SvXMLRPCRequestState_gotException) {
        SvString desc;
        int code;
        SvXMLRPCRequestGetException(req, &code, &desc, NULL);
        log_error("Exception: %d: %s", code, SvStringCString(desc));
    } else if (state != SvXMLRPCRequestState_error) {
        return;
    }

    log_debug("process next request");
    SvXMLRPCRequestsQueueFinishRequest(self->xmlRPCReqQueue);
    SvXMLRPCRequestsQueueProcessNext(self->xmlRPCReqQueue);
}

SvLocal void
QBPlayBackStateReporterXMLRPCRequestSetup(SvObject self_, SvXMLRPCRequest req)
{
    log_fun();
    SvHTTPRequest httpReq = SvXMLRPCRequestGetHTTPRequest(req, NULL);
    SvHTTPRequestSetHTTPHeader(httpReq, SVSTRING("X-CW-Api-Version"), SVSTRING(INNOV8ON_REQUEST_VERSION), NULL);
}

SvLocal void
QBPlaybackStateReporterDestory(void *self_)
{
    log_fun();
    QBPlaybackStateReporter self = (QBPlaybackStateReporter) self_;
    if (self->currentParams) {
        log_warning("Current playback not finished yet");
    }

    SVTESTRELEASE(self->sessionId);
    SVTESTRELEASE(self->currentParams);
    SVTESTRELEASE(self->currentPlayer);
    SVTESTRELEASE(self->xmlRPCReqQueue);
    SVTESTRELEASE(self->waitingRequests);

    if (self->waitingFiber) {
        SvFiberDestroy(self->waitingFiber);
    }
}

SvLocal SvType
QBPlaybackStateReporter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBPlaybackStateReporterDestory
    };

    static const struct SvXMLRPCClientListener_ xmlrpcMethods = {
        .stateChanged = QBPlayBackStateReporterXMLRPCRequestStateChanged,
        .setup        = QBPlayBackStateReporterXMLRPCRequestSetup,
    };

    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBPlaybackStateReporter",
                            sizeof(struct QBPlaybackStateReporter_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            SvXMLRPCClientListener_getInterface(), &xmlrpcMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBPlaybackStateReporterCreateId(QBPlaybackStateReporter self)
{
    log_fun();
    char randomValue[SESSION_ID_SIZE_B + 1];
    memset(randomValue, 0, sizeof(randomValue));

    uint32_t randNum, i;
    for (i = 0; i < SESSION_ID_SIZE_B; i += 4) {
        randNum = (uint32_t) rand();
        snprintf(randomValue + i, sizeof(randomValue) - i, "%04u", randNum);
    }
    self->sessionId = SvStringCreateWithFormat("%s", randomValue);
}

SvLocal void
QBPlaybackStateReporterStep(void *self_)
{
    log_fun();
    QBPlaybackStateReporter self = (QBPlaybackStateReporter) self_;

    SvFiberDeactivate(self->waitingFiber);
    SvFiberEventDeactivate(self->waitingFiberTimer);

    SvXMLRPCRequest req;
    while ((req = (SvXMLRPCRequest) SvDequeTakeFront(self->waitingRequests))) {
        SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, req);
        SVRELEASE(req);
    }
}

SvLocal void
QBPlaybackStateReporterSetReqOptions(QBPlaybackStateReporter self, QBPlaybackStateReporterRequestParams params, SvHashTable options)
{
    log_fun();
    SvValue speedV = SvValueCreateWithDouble(params->speed, NULL);
    SvHashTableInsert(options, (SvObject) SVSTRING("speed"), (SvObject) speedV);
    SVRELEASE(speedV);

    SvValue positionV = SvValueCreateWithDouble(params->position, NULL);
    SvHashTableInsert(options, (SvObject) SVSTRING("position"), (SvObject) positionV);
    SVRELEASE(positionV);

    SvValue timeshiftDelayV = SvValueCreateWithDouble(params->timeshiftDelay, NULL);
    SvHashTableInsert(options, (SvObject) SVSTRING("timeshift_delay"), (SvObject) timeshiftDelayV);
    SVRELEASE(timeshiftDelayV);

    if (params->productId) {
        SvHashTableInsert(options, (SvObject) SVSTRING("product_id"), (SvObject) params->productId);
    }

    if (params->title) {
        SvValue titleV = SvValueCreateWithString(params->title, NULL);
        SvHashTableInsert(options, (SvObject) SVSTRING("title"), (SvObject) titleV);
        SVRELEASE(titleV);
    }
}

SvLocal SvXMLRPCRequest
QBPlaybackStateReporterCreateRequest(QBPlaybackStateReporter self, QBPlaybackStateReporterRequestParams params)
{
    log_fun();

    SvString customerId = QBMiddlewareManagerGetId(self->mwManager, QBMiddlewareManagerType_Innov8on);
    SvXMLRPCServerInfo serverInfo = QBMiddlewareManagerGetXMLRPCServerInfo(self->mwManager, QBMiddlewareManagerType_Innov8on);
    if (!customerId || !serverInfo) {
        log_notice("required data not provided");
        return NULL;
    }

    SvHashTable options = SvHashTableCreate(7, NULL);
    SvString playbackState = NULL;
    if (params) {
        log_debug("playing state");
        playbackState = SvStringCreate("playing", NULL);
        QBPlaybackStateReporterSetReqOptions(self, params, options);
    } else {
        log_debug("idle player state");
        playbackState = SvStringCreate("idle", NULL);
    }
    SvHashTableInsert(options, (SvObject) SVSTRING("session"), (SvObject) self->sessionId);

    SvImmutableArray argumets = SvImmutableArrayCreateWithTypedValues("@@@", NULL, customerId, playbackState, options);
    SVRELEASE(playbackState);
    SVTESTRELEASE(options);

    SvXMLRPCRequest req = (SvXMLRPCRequest) SvTypeAllocateInstance(SvXMLRPCRequest_getType(), NULL);
    SvXMLRPCRequestInit(req, serverInfo, SVSTRING("stb.SetState"), argumets, NULL);
    SVRELEASE(argumets);
    SvXMLRPCRequestAddListener(req, (SvObject) self, NULL);

    log_debug("request created");
    return req;
}

SvLocal void
QBPlaybackStateReporterScheduleRequest(QBPlaybackStateReporter self, SvXMLRPCRequest request)
{
    log_fun();
    if (!SvDequeIsEmpty(self->waitingRequests)) {
        log_notice("add request to waiting queue");
        SvDequePushBack(self->waitingRequests, (SvObject) request, NULL);
    } else {
        log_notice("add request for processing");
        SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, request);
    }
}

QBPlaybackStateReporter
QBPlaybackStateReporterCreate(QBMiddlewareManager mwManager, SvErrorInfo *errorOut)
{
    log_fun();

    if (!mwManager) {
        log_warning("Middleware manager missing");
        return NULL;
    }

    SvErrorInfo error = NULL;
    QBPlaybackStateReporter self = (QBPlaybackStateReporter) SvTypeAllocateInstance(QBPlaybackStateReporter_getType(), &error);
    if (error) {
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }

    self->xmlRPCReqQueue = SvXMLRPCRequestsQueueCreate();
    if (!self->xmlRPCReqQueue) {
        log_debug("Failed to create xmlrpc requests queue");
        SVTESTRELEASE(self);
        return NULL;
    }

    // initial size is 3 (playback started, finished and one space left)
    self->waitingRequests = SvDequeCreateWithCapacity(3, NULL);

    self->mwManager = mwManager;

    QBPlaybackStateReporterCreateId(self);

    self->waitingFiber = SvFiberCreate(NULL, NULL, "QBPlaybackStateReporterFiber", QBPlaybackStateReporterStep, self);
    self->waitingFiberTimer = SvFiberTimerCreate(self->waitingFiber);

    self->started = false;
    return self;
}

void
QBPlaybackStateReporterReportPlaybackStarted(QBPlaybackStateReporter self, SvValue productId, SvString title, SvObject source)
{
    log_fun();
    if (!self || !self->started) {
        log_warning("Service not created or not started yet");
        return;
    }

#if SV_LOG_LEVEL > 0
    if (env_log_level() > 2) {
        if (productId) {
            if (SvValueIsString(productId))
                log_debug("String id: %s", SvStringCString(SvValueGetString(productId)));
            else if (SvValueIsInteger(productId))
                log_debug("Integer value: %d", SvValueGetInteger(productId));
            else if (SvValueIsDouble(productId))
                log_debug("Double value: %f", SvValueGetDouble(productId));
            else
                log_debug("Unknown product id type: %d", SvValueGetType(productId));
        }
        if (title)
            log_debug("Title: %s", SvStringCString(title));
    }
#endif

    if (!productId && !title) {
        log_notice("Neither product id nor title provided !");
        return;
    }

    if (self->currentParams) {
        log_notice("Previous playback not stopped!");
        QBPlaybackStateReporterReportPlaybackFinished(self);
    }

    log_debug("Updateing params");

    self->currentParams = QBPlaybackStateReporterRequestParamsCreate();

    self->currentParams->productId = SVTESTRETAIN(productId);
    self->currentParams->title = SVTESTRETAIN(title);
    self->currentParams->speed = 1;
    self->currentParams->position = 0;
    self->currentParams->timeshiftDelay = 0;

    if (source && SvObjectIsImplementationOf(source, QBPlaybackStateReporterDataSource_getInterface())) {
        SVTESTRELEASE(self->currentPlayer);
        self->currentPlayer = SvWeakReferenceCreate(source, NULL);
    } else {
        log_warning("Object is not implementing required interface. Forced request will not work");
    }

    SvXMLRPCRequest req = QBPlaybackStateReporterCreateRequest(self, self->currentParams);
    if (req) {
        // Do not send request immediately because it's possible that playback finished will be called soon
        // e.g. in TV during fast channel change. In that case only last channel will be reported
        SvDequePushBack(self->waitingRequests, (SvObject) req, NULL);
        SVRELEASE(req);
        SvFiberTimerActivateAfter(self->waitingFiberTimer, SvTimeFromMilliseconds(MAX_IGNORE_TIME_MS));
        log_notice("Request scheduled");
    } else {
        log_warning("failed to create xml rpc request");
    }
}

void
QBPlaybackStateReporterReportChange(QBPlaybackStateReporter self, int currentPosition, double currentSpeed)
{
    QBPlaybackStateReporterReportChangeWithTimeshift(self, currentPosition, currentSpeed, 0);
}

void
QBPlaybackStateReporterReportChangeWithTimeshift(QBPlaybackStateReporter self, int currentPosition, double currentSpeed, int timeshiftDelay)
{
    log_fun();
    if (!self || !self->started) {
        log_warning("Service not created or not started yet");
        return;
    }

    log_debug("position: %d, speed: %f, timeshift: %d", currentPosition, currentSpeed, timeshiftDelay);
    if (!self->currentParams) {
        log_notice("No playback started");
        return;
    }

    bool changed = false;
    if (self->currentParams->position != currentPosition ||
        self->currentParams->speed != currentSpeed ||
        self->currentParams->timeshiftDelay != timeshiftDelay) {
        log_notice("Params changed -> sending request");
        changed = true;
    }

    if (changed || self->forceRequest) {
        self->currentParams->position = currentPosition;
        self->currentParams->speed = currentSpeed;
        self->currentParams->timeshiftDelay = timeshiftDelay;

        SvXMLRPCRequest req = QBPlaybackStateReporterCreateRequest(self, self->currentParams);
        if (req) {
            QBPlaybackStateReporterScheduleRequest(self, req);
            SVRELEASE(req);
        } else {
            log_warning("failed to create xml rpc request");
        }
        self->forceRequest = false;
    } else {
        log_notice("nothing changed -> ignore");
    }
}

void
QBPlaybackStateReporterReportPlaybackFinished(QBPlaybackStateReporter self)
{
    log_fun();
    if (!self || !self->started) {
        log_warning("Service not created or not started yet");
        return;
    }

    if (!self->currentParams) {
        log_notice("No playback started");
        return;
    }

    SVTESTRELEASE(self->currentParams);
    self->currentParams = NULL;

    if (SvFiberEventIsArmed(self->waitingFiberTimer)) {
        // playbackStarted was not yet sent.
        // Destroy all requests for this playback as it has never existed
        log_notice("Clean waiting requests queue for this playback");
        SvFiberEventDeactivate(self->waitingFiberTimer);
        SvDequeRemoveAllObjects(self->waitingRequests);
        return;
    }

    SvXMLRPCRequest req = QBPlaybackStateReporterCreateRequest(self, NULL);
    if (req) {
        QBPlaybackStateReporterScheduleRequest(self, req);
        SVRELEASE(req);
    } else {
        log_warning("failed to create xml rpc request");
    }
}

void QBPlaybackStateReporterForceReport(QBPlaybackStateReporter self)
{
    log_fun();
    if (!self || !self->started) {
        log_warning("Service not created or not started yet");
        return;
    }

    if (!self->currentParams) {
        log_debug("No playback started -> send 'idle' state");
        SvXMLRPCRequest req = QBPlaybackStateReporterCreateRequest(self, NULL);
        if (req) {
            SvXMLRPCRequestsQueueAddToProcess(self->xmlRPCReqQueue, req);
            SVRELEASE(req);
        } else {
            log_warning("failed to create xml rpc request");
        }
        return;
    }

    SvObject player = NULL;
    if (self->currentPlayer) {
        player = SvWeakReferenceTakeReferredObject(self->currentPlayer);
    }

    if (!player) {
        log_warning("Playback started by unknown object");
        return;
    }

    self->forceRequest = true;
    SvInvokeInterface(QBPlaybackStateReporterDataSource, player, reportState);
    SVRELEASE(player);
}

void QBPlaybackStateReporterStart(QBPlaybackStateReporter self)
{
    log_fun();
    self->started = true;
}

void QBPlaybackStateReporterStop(QBPlaybackStateReporter self)
{
    log_fun();
    if (self->currentParams) {
        log_debug("Stopping current playback");
        QBPlaybackStateReporterReportPlaybackFinished(self);
    }
    self->started = false;
}
