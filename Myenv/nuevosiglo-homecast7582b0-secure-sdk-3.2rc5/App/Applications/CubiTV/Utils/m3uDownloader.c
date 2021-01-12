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

#include "m3uDownloader.h"
#include <storage_input/storage_input.h>
#include <storage_input/storage_input_http.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <SvPlayerKit/SvBufMan.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvCommonDefs.h>
#include <QBM3uParser.h>


struct QBm3uDownloader_t {
    struct SvObject_ super_;
    struct storage_input* input;
    QBM3uParser parser;

    SvFiber fiber;

    QBm3uDownloaderFinished onFinish;
    QBm3uDownloaderDownloadError onDownloadError;
    void *target;

    SvMemCounter memCounter;
    struct SvBufMan_s bufMan;

    bool finished;

    SvChbuf queue;

    SvArray result;

    int bytesDownloaded;
};

SvLocal void QBm3uDownloader__dtor__(void *self_)
{
    QBm3uDownloader self = self_;
    if (self->input)
        self->input->destroy(self->input);
    SVRELEASE(self->parser);
    SvMemCounterAutodestroy(self->memCounter);
    SvFiberDestroy(self->fiber);
    SvChbufClear(self->queue);
    free(self->queue);
    SVTESTRELEASE(self->result);
}

SvLocal SvType QBm3uDownloader_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBm3uDownloader__dtor__
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBm3uDownloader",
                            sizeof(struct  QBm3uDownloader_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal void QBm3uDownloaderGotItem(void *self_, QBM3uParser parser, QBM3uItem item)
{
    QBm3uDownloader self = self_;
    if(!self->result)
        self->result = SvArrayCreate(NULL);
    SvArrayAddObject(self->result, (SvObject) item);
}

SvLocal void QBm3uDownloaderStep(void *self_)
{
    QBm3uDownloader self = self_;
    SvFiberDeactivate(self->fiber);

    while(!SvFiberTimePassed() && self->queue->buf_cnt) {
        SvBuf buf = SvChbufPopFront(self->queue);
        QBM3uParserPush(self->parser, buf);

        if (self->queue->buf_cnt == 0)
            QBM3uParserSetEof(self->parser);

        QBM3uParserRun(self->parser);
    }

    if(self->queue->buf_cnt) {
        SvFiberActivate(self->fiber);
    } else if(self->finished) {
        self->onFinish(self->target, self, self->result);
    }
}

SvLocal int QBm3uDownloaderDataReceived(void *self_, struct storage_input* input, SvBuf buf, int64_t pos)
{
    QBm3uDownloader self = self_;

    self->bytesDownloaded += buf->len;

    SvChbufCut(self->queue, buf, 16 * 1024);
    SvFiberActivate(self->fiber);

    if(self->bytesDownloaded > 2 * 1024 * 1024 && !self->finished) {
        self->input->pause(self->input);
        self->finished = true;
    }
    return 0;
}

SvLocal void QBm3uDownloaderDataFinished(void *self_, struct storage_input* input)
{
    QBm3uDownloader self = self_;
    self->finished = true;
    if(self->queue->buf_cnt == 0)
        self->onFinish(self->target, self, self->result);
}

SvLocal void QBm3uDownloaderDataErrored(void *self_, struct storage_input* input, int err)
{
    QBm3uDownloader self = self_;
    if(self->finished)
        return;

    self->input->destroy(self->input);
    self->input = NULL;
    self->onDownloadError(self->target, self, err);
}

QBm3uDownloader QBm3uDownloaderCreate(SvScheduler scheduler, SvString url, QBm3uDownloaderFinished onFinishCallback, QBm3uDownloaderDownloadError onDownloadErrorCallback, void *target)
{
    QBm3uDownloader self = (QBm3uDownloader) SvTypeAllocateInstance(QBm3uDownloader_getType(), NULL);
    self->target = target;
    self->onFinish = onFinishCallback;
    self->onDownloadError = onDownloadErrorCallback;
    self->memCounter = calloc(1, sizeof(struct SvMemCounter_s));
    self->fiber = SvFiberCreate(scheduler, NULL, "QBm3uDownloader", QBm3uDownloaderStep, self);
    self->parser = QBM3uParserCreate(1024, &QBm3uDownloaderGotItem, NULL, NULL, self);
    self->queue = calloc(1, sizeof(struct SvChbuf_s));
    SvChbufInit(self->queue);

    self->bufMan.mem_counter  = self->memCounter;
    self->bufMan.max_mem_used = 2 * 1024 * 1024;

    struct storage_input_http_params input_params = {
        .url = SvStringCString(url),
        .buf_man = &self->bufMan,
    };

    storage_input_http_create(&input_params, &self->input, NULL);

    static struct storage_input_callbacks input_callbacks = {
        .data = QBm3uDownloaderDataReceived,
        .eos = QBm3uDownloaderDataFinished,
        .error = QBm3uDownloaderDataErrored
    };

    self->input->set_callbacks(self->input, &input_callbacks, self);

    self->input->seek(self->input, 0, -1);
    self->input->unpause(self->input);

    return self;
}
