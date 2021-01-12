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
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/

#define SvLogModule "com.sentivision.SvHTTPClient.SvHTTPGet"
#define SvLogModuleCustomized

#include "QBHTTPEMMService.h"
#include <QBConf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <expat.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvArray.h>
#include <fibers/c/fibers.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvHTTPClient/SvHTTPRequest.h>
#include <SvHTTPClient/SvHTTPClientListener.h>
#include <QBCAS.h>
#include <SvProfile.h>

// listener class


struct QBHTTPEMMService_ {
    struct SvObject_ super_;

    SvHTTPClientEngine engine;
    SvHTTPRequest request;
    SvHTTPRequestID reqID;
    SvFiber cardFiber;
    SvFiber updateFiber;
    SvFiberTimer updateTimer;
    bool started;
    XML_Parser expatParser;
    SvArray emmList;

    QBCASCmd cmd;
    SvString id;
    SvString emmUrl;
    unsigned int timeUpdate;
    unsigned int timeRepeat;
};

//XML parser methods


SvLocal SvString QBHTTPEMMServiceParseCardID(SvString card_number)
{
    const char* id = SvStringCString(card_number);
    int size = SvStringLength(card_number);
    char* newId = calloc(1, (size + 1) * sizeof(char));
    int i, j = 0;
    for (i = 0; i < size; i++)
        if (id[i] != ' ' && id[i] != '-') {
            newId[j] = id[i];
            j++;
        }

    SvString ret = SvStringCreate(newId, NULL);
    free(newId);
    return ret;
}

SvLocal void QBHTTPEMMServiceStartElementHandler(void *self_,
                                                 const XML_Char *name, const XML_Char **attrs)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    if (!strcmp(name, "EMMData") && !strcmp(attrs[0], "EMMSection")) {
        if (!self->emmList)
            self->emmList = SvArrayCreate(NULL);

        SvString emm = SvStringCreate(attrs[1], NULL);
        SvArrayAddObject(self->emmList, (SvGenericObject) emm);
        SVRELEASE(emm);
    }
}

SvLocal void QBHTTPEMMServiceEndElementHandler(void *self_,
                                               const XML_Char *name)
{
}
// private methods

static void QBHTTPEMMServiceCleanHTTPRequest(QBHTTPEMMService self)
{
    if (self->request)
        SvHTTPRequestCancel(self->request, NULL);
    SVTESTRELEASE(self->request);
    self->request = NULL;

    if (self->expatParser) {
        XML_ParserFree(self->expatParser);
        self->expatParser = NULL;
    }
}

// virtual methods

static void QBHTTPEMMServiceHeaderReceived(SvGenericObject self_,
                                           SvHTTPRequest req, const char *name, const char *value)
{
}

static void QBHTTPEMMServiceDataChunkReceived(SvGenericObject self_,
                                              SvHTTPRequest req, off_t offset, const uint8_t *buffer, size_t length)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    if (self->expatParser)
        XML_Parse(self->expatParser, (const char *) buffer, length, 1);
}

static void QBHTTPEMMServiceSendCallback(void* arg, const QBCASCmd cmd, int status)
{
    const char* name = SvInvokeInterface(QBCAS, QBCASGetInstance(), getCommandName, cmd);

    if (status == 0) {
        SvLogNotice("Command [%s] -> ok", name);
    } else {
        SvLogError("Command [%s] -> %d", name, status);
    }
}

SvLocal void QBHTTPEMMServiceSmartCardState(void* self_, QBCASSmartcardState state);

static void QBHTTPEMMServiceCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info);

// FIXME: this should be const!
static struct QBCASCallbacks_s s_smartcard_callbacks = {
    .smartcard_state = &QBHTTPEMMServiceSmartCardState
};

void QBHTTPEMMServiceStart(QBHTTPEMMService self)
{
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, &s_smartcard_callbacks, self, NULL);

    SvFiberActivate(self->cardFiber);
    SvFiberTimerActivateAfter(self->updateTimer,
                              SvTimeFromMs(self->timeUpdate * 1000));
    self->started = true;

    if (SvInvokeInterface(QBCAS, QBCASGetInstance(), getState) == QBCASSmartcardState_correct) {
        if (self->cmd) {
            SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
            self->cmd = NULL;
        }
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBHTTPEMMServiceCallbackInfo, self, &self->cmd);
    }
}

void QBHTTPEMMServiceStop(QBHTTPEMMService self)
{
    SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, &s_smartcard_callbacks, self);
    QBHTTPEMMServiceCleanHTTPRequest(self);
    SvFiberEventDeactivate(self->updateTimer);
    SvFiberDeactivate(self->cardFiber);
    SvFiberDeactivate(self->updateFiber);

    self->started = false;
}

static void QBHTTPEMMServiceSendStep(void *self_)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;

    if (self->emmList && SvArrayCount(self->emmList)) {
        SvString message = (SvString) SvArrayLastObject(self->emmList);
        int len = SvStringLength(message) / 2;

        SvData emmSection = SvDataCreateWithLength(len, NULL);

        const char* byte = SvStringCString(message);
        int i, j;
        for (i = 0, j = 0; i < 2 * len; i += 2, j++) {
            char temp[3] =
            { };
            temp[0] = byte[i];
            temp[1] = byte[i + 1];
            SvDataGetBytes(emmSection)[j] = strtol(temp, NULL, 16);
        }

        //send data to card
        SvInvokeInterface(QBCAS, QBCASGetInstance(), pushEMM, QBHTTPEMMServiceSendCallback, NULL, NULL, -1, emmSection, true);
        SVRELEASE(emmSection);

        SvArrayRemoveLastObject(self->emmList);
    } else {
        SvFiberDeactivate(self->cardFiber);
    }
}

static QBHTTPEMMService
QBHTTPEMMServiceStartRequest(QBHTTPEMMService self, const char *URL,
                             SvHTTPClientEngine engine, SvErrorInfo *errorOut);

static void QBHTTPEMMServiceUpdateStep(void *self_)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    SvFiberDeactivate(self->updateFiber);
    SvFiberEventDeactivate(self->updateTimer);

    if (self->id) {
        char* url;
        asprintf(&url, "%s?CardNumber=%s", SvStringCString(self->emmUrl),
                 SvStringCString(self->id));
        QBHTTPEMMServiceCleanHTTPRequest(self);
        QBHTTPEMMServiceStartRequest(self, url, self->engine, NULL);
        free(url);
    }

    SvFiberTimerActivateAfter(self->updateTimer,
                              SvTimeFromMs(self->timeUpdate * 1000));
}

static void QBHTTPEMMServiceTransferFinished(SvGenericObject self_,
                                             SvHTTPRequest req, off_t total)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    QBHTTPEMMServiceCleanHTTPRequest(self);
    SvFiberActivate(self->cardFiber);
}

static void QBHTTPEMMServiceTransferCancelled(SvGenericObject self_,
                                              SvHTTPRequest req)
{
    SvLogDebug("[%02u] cancelled", ((QBHTTPEMMService) self_)->reqID);
}

static void QBHTTPEMMServiceErrorOccured(SvGenericObject self_,
                                         SvHTTPRequest req, int httpError)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    SvLogError("[%02u] failed with error %d", self->reqID, httpError);
    QBHTTPEMMServiceCleanHTTPRequest(self);
    SvFiberTimerActivateAfter(self->updateTimer,
                              SvTimeFromMs(self->timeRepeat * 1000));
}

// public methods

static void QBHTTPEMMServiceDestroy(void *self_);

static SvType QBHTTPEMMService_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBHTTPEMMServiceDestroy
    };
    static const struct SvHTTPClientListener_ listenerMethods = {
        .headerReceived    = QBHTTPEMMServiceHeaderReceived,
        .dataChunkReceived = QBHTTPEMMServiceDataChunkReceived,
        .transferFinished  = QBHTTPEMMServiceTransferFinished,
        .errorOccured      = QBHTTPEMMServiceErrorOccured,
        .transferCancelled = QBHTTPEMMServiceTransferCancelled
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBHTTPEMMService",
                            sizeof(struct QBHTTPEMMService_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvHTTPClientListener_getInterface(), &listenerMethods,
                            NULL);
    }
    return type;
}

static QBHTTPEMMService QBHTTPEMMServiceStartRequest(QBHTTPEMMService self,
                                                     const char *URL,
                                                     SvHTTPClientEngine engine,
                                                     SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SvHTTPRequest req = NULL;

    if (!self || !URL) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto fini;
    }

    req = (SvHTTPRequest) SvTypeAllocateInstance(SvHTTPRequest_getType(), &error);
    if (!req || !SvHTTPRequestInit(req, engine, SvHTTPRequestMethod_GET, URL, &error)) {
        SVRELEASE(req);
        goto fini;
    }

    if (!strncmp(URL, "https://", 8)) {
        SvSSLParams ssl = (SvSSLParams) SvTypeAllocateInstance(SvSSLParams_getType(), &error);
        SvSSLParamsInit(ssl, SvSSLVersion_default, &error);
        SvSSLParamsSetVerificationOptions(ssl, false, false, NULL, NULL);
        SvHTTPRequestSetSSLParams(req, ssl, NULL);
        SVRELEASE(ssl);
    }

    SvHTTPRequestSetUserAgent(req, SVSTRING("SvHTTPGet"), NULL);
    SvHTTPRequestSetListener(req, (SvGenericObject) self, true, NULL);
    SvHTTPRequestSetPriority(req, SvHTTPRequestPriority_max, NULL);
    SvHTTPRequestSetBufferSize(req, 256 * 1024, NULL);
    SvHTTPRequestStart(req, NULL);

    self->request = req;
    self->reqID = SvHTTPRequestGetID(req);

    self->expatParser = XML_ParserCreate("UTF-8");
    XML_SetHashSalt(self->expatParser, rand());
    XML_SetUserData(self->expatParser, self);
    XML_SetElementHandler(self->expatParser,
                          QBHTTPEMMServiceStartElementHandler,
                          QBHTTPEMMServiceEndElementHandler);

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

static void QBHTTPEMMServiceDestroy(void *self_)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    QBHTTPEMMServiceCleanHTTPRequest(self_);

    SvFiberDestroy(self->cardFiber);
    SvFiberDestroy(self->updateFiber);
    SVRELEASE(self->engine);

    SVTESTRELEASE(self->emmList);
    SVTESTRELEASE(self->emmUrl);
    SVTESTRELEASE(self->id);
}

//smart card interface

static void QBHTTPEMMServiceCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    self->cmd = NULL;
    if (status != 0) {
        SvLogNotice("%s: Unable to get card number", __func__);
        return;
    }

    SvString oldID = self->id;

    if (!info->card_number_part_unique) {
        SvLogError("NULL card number");
        QBHTTPEMMServiceCleanHTTPRequest(self);
        goto fini;
    }

    self->id = QBHTTPEMMServiceParseCardID(info->card_number_part_unique);
    if (SvObjectEquals((SvObject) oldID, (SvObject) self->id))
        goto fini;

    SVTESTRELEASE(self->emmList);
    self->emmList = NULL;

    char* url;
    asprintf(&url, "%s?CardNumber=%s", SvStringCString(self->emmUrl),
             SvStringCString(self->id));
    QBHTTPEMMServiceStartRequest(self, url, self->engine, NULL);
    free(url);

fini:
    SVTESTRELEASE(oldID);
}

SvLocal void QBHTTPEMMServiceSmartCardState(void* self_,
                                            QBCASSmartcardState state)
{
    QBHTTPEMMService self = (QBHTTPEMMService) self_;
    if (!self->started)
        return;

    if (state == QBCASSmartcardState_correct) {
        if (self->cmd) {
            SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
            self->cmd = NULL;
        }
        SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &QBHTTPEMMServiceCallbackInfo, self, &self->cmd);
    } else {
        SVTESTRELEASE(self->emmList);
        self->emmList = NULL;
        QBHTTPEMMServiceCleanHTTPRequest(self);
    }
}

QBHTTPEMMService QBHTTPEMMServiceCreate(SvScheduler scheduler, SvString url,
                                        unsigned int timeUpdate, unsigned int timeRepeat)
{
    QBHTTPEMMService self = (QBHTTPEMMService) SvTypeAllocateInstance(
        QBHTTPEMMService_getType(), NULL);

    self->emmUrl = SVRETAIN(url);
    self->engine = (SvHTTPClientEngine) SvTypeAllocateInstance(
        SvHTTPClientEngine_getType(), NULL);
    SvHTTPClientEngineInit(self->engine, 1, NULL);

    self->cardFiber = SvFiberCreate(scheduler, NULL, "EMMSend",
                                    QBHTTPEMMServiceSendStep, self);
    self->emmList = SvArrayCreate(NULL);
    self->updateFiber = SvFiberCreate(scheduler, NULL, "EMMUpdate",
                                      QBHTTPEMMServiceUpdateStep, self);
    self->updateTimer = SvFiberTimerCreate(self->updateFiber);
    self->timeUpdate = timeUpdate;
    self->timeRepeat = timeRepeat;

    return self;
}
