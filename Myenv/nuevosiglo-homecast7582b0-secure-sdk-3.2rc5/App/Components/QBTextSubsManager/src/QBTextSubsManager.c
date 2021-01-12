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

#include "QBTextSubsManager.h"
#include <QBSubsManager.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvDeque.h>
#include <SvFoundation/SvArray.h>

#include <mpeg_es_decoder.h>
#include <dataformat/sv_data_format.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <CUIT/Core/app.h>
#include <settings.h>
#include <player_opts/subs_sniffer.h>
#include <fibers/c/fibers.h>

#include <SvPlayerControllers/SvPlayerSubsTrackES.h>
#include <QBSMPTETTPngDecoder.h>
#include "QBTTMLSubsParser.h"
#include "QBTTMLSubtitle.h"

#define log_error(fmt, ...)  SvLogError(COLBEG() "QBTextSubsManager : " fmt COLEND_COL(red), ## __VA_ARGS__)
#define log_warning(fmt, ...)  SvLogWarning(COLBEG() "QBTextSubsManager : " fmt COLEND_COL(yellow), ## __VA_ARGS__)
#define log_fun(fmt, ...)  do { if (0) SvLogNotice(COLBEG() " %s:%d " fmt COLEND_COL(cyan), __func__, __LINE__, ## __VA_ARGS__); } while (0)
#define log_pages(fmt, ...)  do { if (0) { SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ## __VA_ARGS__); }; } while (0)
#define log_state(fmt, ...)  do { if (0) { SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ## __VA_ARGS__); }; } while (0)
#define log_debug(fmt, ...)  do { if (0) { SvLogNotice(COLBEG() fmt COLEND_COL(blue), ## __VA_ARGS__); }; } while (0)

//-------------------------------------------------------------------------------
// QBTextSubsTrack

struct QBTextSubsTrack_ {
    struct QBSubsTrack_t super_;
};

SvLocal void QBTextSubsTrackDestroy(void *self_)
{
}

SvType QBTextSubsTrack_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTextSubsTrackDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTextSubsTrack",
                            sizeof(struct QBTextSubsTrack_),
                            QBSubsTrack_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal QBTextSubsTrack QBTextSubsTrackCreate(SvString langCode)
{
    QBTextSubsTrack self = (QBTextSubsTrack) SvTypeAllocateInstance(QBTextSubsTrack_getType(), NULL);;
    if (langCode) {
        self->super_.langCode = SVRETAIN(langCode);
    } else {
        self->super_.langCode = SVSTRING("Unknown");
    }
    return self;
}

/**
 * The QBTextSubsReceiver type.
 *
 **/
struct QBTextSubsReceiver_s {
    QBTextSubsManager manager;

    QBTTMLSubsParser ttmlParser;

    int tracksCnt;
    QBTextSubsTrack *tracks;
};
typedef struct QBTextSubsReceiver_s *QBTextSubsReceiver;


/**
 * The QBTextSubsManager type.
 *
 **/
struct QBTextSubsManager_ {
    struct SvObject_ super_;

    SvApplication app;

    QBTextSubsReceiver receiver;

    SvPlayerTask playerTask;

    const struct svdataformat *format;

    QBSubsManager subsManager;

    QBSMPTETTPngDecoder smptettPngDecoder;

    int defaultWidth, defaultHeight;

    SvFont font;

    SvFont boldFont;

    struct QBSubsManualFontParams_ manualFontParams_;
};

SvLocal void QBTextSubsReceiverStop(SvPlayerTask playerTask, QBTextSubsReceiver receiver, QBSubsManager subsManager)
{
    if (!receiver) {
        return;
    }

    int i;
    for (i = 0; i < receiver->tracksCnt; i++) {
        if (subsManager) {
            QBSubsManagerUnregisterTrack(subsManager, (QBSubsTrack) receiver->tracks[i]);
        }
        SVTESTRELEASE(receiver->tracks[i]);
    }

    struct player_opt_subs_sniffer_s params_sniffer = {
        .target = receiver,
    };

    SvPlayerTaskSetOpt(playerTask, PLAYER_OPT_SUBS_SNIFFER_STOP, &params_sniffer);
}

void QBTextSubsManagerStop(QBTextSubsManager self)
{
    log_debug("self=%p", self);

    if (self->receiver) {
        QBTextSubsReceiverStop(self->playerTask, self->receiver, self->subsManager);
    }
}

SvLocal void QBTextSubsReceiverDestroy(QBTextSubsReceiver receiver)
{
    if (!receiver) {
        return;
    }

    SVTESTRELEASE(receiver->ttmlParser);

    free(receiver->tracks);
    free(receiver);
}

SvLocal void QBTextSubsManagerDestroy(void *self_)
{
    QBTextSubsManager self = (QBTextSubsManager) self_;

    log_debug("%s self=%p", __func__, self);

    SVTESTRELEASE(self->smptettPngDecoder);

    if (self->receiver) {
        QBTextSubsReceiverDestroy(self->receiver);
    }

    if (self->format) {
        svdataformat_release(self->format);
    }

    SVTESTRELEASE(self->playerTask);
    SVTESTRELEASE(self->subsManager);
    SVTESTRELEASE(self->font);
    SVTESTRELEASE(self->boldFont);
}

SvLocal void QBTextSubsManagerSetup(SvObject self_, QBSubsManualFontParams params)
{
    QBTextSubsManager self = (QBTextSubsManager) self_;
    if (!self || !params) {
        log_error("NULL argument passed");
        return;
    }
    self->manualFontParams_ = *params;
}

SvLocal SvType QBTextSubsManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTextSubsManagerDestroy
    };
    static SvType type = NULL;
    static struct QBSubsManagerPluginInterface_ methods = {
        .setup = QBTextSubsManagerSetup
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTextSubsManager",
                            sizeof(struct QBTextSubsManager_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSubsManagerPluginInterface_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal int
compDouble(const void *a, const void *b)
{
    if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    }
    return 0;
}

SvLocal void
calculateRenderParams(QBTextSubsReceiver self, QBTTMLDocumentProperties *ttmlProperties, QBTTMLSubtitleRenderParams renderParams)
{
    renderParams->nativeWidth = self->manager->app->width * 0.8;
    renderParams->nativeHeight = self->manager->app->height * 0.8;

    renderParams->manual_ = self->manager->manualFontParams_;
    if (renderParams->manual_.useSize) {
        renderParams->lineHeight = (renderParams->manual_.size * 4) / 3;
        renderParams->baseFontSize = renderParams->manual_.size;
    } else {
        renderParams->lineHeight = renderParams->nativeHeight / ttmlProperties->cellResolution.y;
        renderParams->baseFontSize = renderParams->lineHeight * 0.75;
    }
    if (renderParams->manual_.style == SvFontStyle_oblique) {
        renderParams->font = self->manager->boldFont;
    } else if (renderParams->manual_.style == SvFontStyle_normal) {
        renderParams->font = self->manager->font;
    } else {
        renderParams->font = self->manager->font;
    }
    SvFontSetSize(renderParams->font, renderParams->baseFontSize, NULL);

    SvGlyphMetrics metricsStorage, *metrics;
    metrics = SvFontGetGlyphMetrics(renderParams->font, L'M', &metricsStorage, NULL);

    if (metrics) {
        renderParams->baseLinePos = (((metrics->minY + 63) >> 6) + renderParams->lineHeight) / 2;
        if (renderParams->manual_.useSize) {
            renderParams->baseFontAspect = SvFontGetAspect(renderParams->font);
        } else {
            renderParams->baseFontAspect = (renderParams->nativeWidth / ttmlProperties->cellResolution.x) / ((metrics->advX + 63) >> 6);
            if (renderParams->baseFontAspect > 1.0) {
                renderParams->baseFontAspect = 1.0;
            }
            SvFontSetAspect(renderParams->font, renderParams->baseFontAspect, NULL);
        }
    }
}

SvLocal void
prepareTimeStamps(SvArray containers, double * *timeStamps_, size_t *cnt_)
{
    double *timeStamps = NULL;
    size_t count = SvArrayCount(containers);

    if (count == 0) {
        return;
    }

    int startIdx = *cnt_;
    if (!(*timeStamps_)) {
        timeStamps = malloc(2 * count * sizeof(double));
        *cnt_ = 2 * count;
    } else {
        timeStamps = realloc(*timeStamps_, (*cnt_ + 2 * count) * sizeof(double));
        *cnt_ += 2 * count;
    }

    log_debug("Processed %zu ttml containers", count);
    for (int i = 0; i < (int) count; i++) {
        QBTTMLContainer cont = (QBTTMLContainer) SvArrayAt(containers, i);
        timeStamps[startIdx + 2 * i] = cont->start;
        timeStamps[startIdx + 2 * i + 1] = cont->end;
        log_debug("Part[%d] start= %lf end= %lf", i, cont->start, cont->end);
    }
    *timeStamps_ = timeStamps;
}

SvLocal void
processDivsAndParagraphs(QBTextSubsReceiver self, uint64_t pts, short stream_id, SvArray pars, SvArray divs, QBTTMLDocumentProperties *properties)
{
    size_t lastParsIdx = 0, lastDivsIdx = 0;

    struct QBTTMLSubtitleRenderParams_ renderParams_;

    calculateRenderParams(self, properties, &renderParams_);

    size_t count = 0;
    double *timeStamps = NULL;
    prepareTimeStamps(pars, &timeStamps, &count);
    prepareTimeStamps(divs, &timeStamps, &count);

    if (!timeStamps) {
        return;
    }

    qsort(timeStamps, count, sizeof(double), compDouble);

    for (int i = 0; i < (int) count - 1; i++) {
        SvIterator it = SvArrayIteratorAtIndex(pars, lastParsIdx);
        SvArray tmpPars = SvArrayCreateWithCapacity(3, NULL);
        QBTTMLContainer cont;
        log_debug("TS[%d] = %lf ", i, timeStamps[i]);

        while ((cont = (QBTTMLContainer) SvIteratorGetNext(&it))) {
            if (cont->end <= timeStamps[i]) {
                lastParsIdx++;
            } else if (cont->start >= timeStamps[i + 1]) {
                break;
            } else {
                SvArrayAddObject(tmpPars, (SvObject) cont);
            }
        }

        it = SvArrayIteratorAtIndex(divs, lastDivsIdx);
        SvArray tmpDivs = SvArrayCreateWithCapacity(3, NULL);
        while ((cont = (QBTTMLContainer) SvIteratorGetNext(&it))) {
            if (cont->end <= timeStamps[i]) {
                lastDivsIdx++;
            } else if (cont->start >= timeStamps[i + 1]) {
                break;
            } else {
                SvArrayAddObject(tmpDivs, (SvObject) cont);
            }
        }

        if (self->manager->smptettPngDecoder && (SvArrayCount(tmpPars) || SvArrayCount(tmpDivs))) {
            QBSubsManagerSubtitle subtitle = (QBSubsManagerSubtitle) QBTTMLSubtitleCreate(tmpPars,
                                                                                          tmpDivs,
                                                                                          self->manager->smptettPngDecoder,
                                                                                          &renderParams_,
                                                                                          NULL);
            subtitle->pts = pts + (uint64_t) (timeStamps[i] * 90000.0);
            subtitle->ptsBits = 33;
            subtitle->preClear = true;
            subtitle->duration = (timeStamps[i + 1] - timeStamps[i]) * 90000;
            QBSubsManagerAddSubtitleForTrack(self->manager->subsManager, (QBSubsTrack) self->tracks[stream_id], (QBSubsManagerSubtitle) subtitle);
            SVRELEASE(subtitle);
        }
        SVRELEASE(tmpPars);
        SVRELEASE(tmpDivs);
    }

    free(timeStamps);
}


SvLocal void QBTextSubsReceiverDataFun(void *self_, SvChbuf chdata, uint64_t pts, short stream_id, int64_t duration90k)
{
    QBTextSubsReceiver self = (QBTextSubsReceiver) self_;
    SvBuf sb = chdata->first;
    SvErrorInfo error = NULL;

    QBTTMLSubsParserReset(self->ttmlParser);

    for (;;) {
        QBTTMLSubsParserParse(self->ttmlParser, sb->data, sb->len, &error);
        if (sb == chdata->last || error) {
            break;
        } else {
            sb = sb->next;
        }
    }

    if (error) {
        SvErrorInfoDestroy(error);
        return;
    }

    SvArray paragraphs = QBTTMLSubsParserGetParagraphs(self->ttmlParser);
    QBTTMLDocumentProperties ttmlProperties;

    SvArray divs = QBTTMLSubsParserGetDivs(self->ttmlParser);
    SvArray divsWithBackgroundImage = NULL;
    if (divs) {
        divsWithBackgroundImage = SvArrayCreate(NULL);
        SvIterator itDivs = SvArrayIterator(divs);
        QBTTMLDiv divElem;
        while ((divElem = (QBTTMLDiv) SvIteratorGetNext(&itDivs))) {
            if (!divElem->backgroundImage) {
                continue;
            }
            if (((QBTTMLContainer) divElem)->start == ((QBTTMLContainer) divElem)->end) { // sanity check
                continue;
            }
            SvArrayAddObject(divsWithBackgroundImage, (SvObject) divElem);
        }
    }

    bool paragraphsExist = paragraphs && SvArrayCount(paragraphs);
    bool divsExist = divsWithBackgroundImage && SvArrayCount(divsWithBackgroundImage);
    if ((paragraphsExist || divsExist)) {
        ttmlProperties = QBTTMLSubsParserGetDocumentProperties(self->ttmlParser);
        processDivsAndParagraphs(self, pts, stream_id, paragraphs, divsWithBackgroundImage, &ttmlProperties);
    }

    SVTESTRELEASE(divsWithBackgroundImage);
}

SvLocal void QBTextSubsReceiverFlush(void *_receiver)
{
    QBTextSubsReceiver receiver = (QBTextSubsReceiver) _receiver;
    int idx;
    for (idx = 0; idx < receiver->tracksCnt; ++idx) {
        QBSubsManagerFlushSubtitleForTrack(receiver->manager->subsManager, (QBSubsTrack) receiver->tracks[idx]);
    }
}

SvLocal QBTextSubsReceiver QBTextSubsReceiverCreate(QBTextSubsManager manager)
{
    SvPlayerTaskControllers taskControllers = SvPlayerTaskGetControllers(manager->playerTask);
    if (!taskControllers) {
        log_warning("Couldn't get SvPlayerTaskControllers");
        return NULL;
    }

    SvPlayerTrackController trackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_subs);
    if (!trackController) {
        log_warning("Couldn't get SvPlayerTrackController");
        return NULL;
    }

    SvArray tracks = SvPlayerTrackControllerGetTracks(trackController);
    if (!tracks) {
        log_warning("Couldn't get tracks");
        return NULL;
    }

    size_t tracksCount = SvArrayCount(tracks);
    if (tracksCount == 0) {
        log_warning("Tracks array is empty");
        return NULL;
    }

    QBTextSubsReceiver receiver = NULL;
    int serv_cnt = (manager->format->type == svdataformattype_es) ? manager->format->es.txt_subs_cnt : 0;

    if (!serv_cnt) {
        return NULL;
    }

    receiver = calloc(1, sizeof(struct QBTextSubsReceiver_s));

    if (!receiver) {
        return NULL;
    }

    receiver->manager = manager;

    receiver->tracks = calloc(serv_cnt, sizeof(QBTextSubsTrack));
    receiver->tracksCnt = serv_cnt;
    receiver->ttmlParser = QBTTMLSubsParserCreate(NULL);

    SvIterator iter = SvArrayGetIterator(tracks);
    SvObject track = NULL;
    int servIdx = 0;
    while ((track = SvIteratorGetNext(&iter))) {
        if (!SvObjectIsInstanceOf(track, SvPlayerSubsTrackES_getType())) {
            continue;
        }

        SvPlayerSubsTrackES subsTrackES = (SvPlayerSubsTrackES) track;
        QBTextSubsTrack textSubsTrack = QBTextSubsTrackCreate(((SvPlayerSubsTrack) subsTrackES)->lang);
        receiver->tracks[servIdx] = textSubsTrack;
        servIdx++;
        if (manager->subsManager) {
            QBSubsManagerRegisterTrack(manager->subsManager, (QBSubsTrack) textSubsTrack, ((SvPlayerTrack) subsTrackES)->id);
        }
    }

    struct player_opt_subs_sniffer_s params_sniffer = {
        .push_data = QBTextSubsReceiverDataFun,
        .flush     = QBTextSubsReceiverFlush,
        .target    = receiver,
    };

    int res = SvPlayerTaskSetOpt(manager->playerTask, PLAYER_OPT_SUBS_SNIFFER_START, (void *) &params_sniffer);
    if (res < 0) {
        log_warning(" : got res %d from player task", res);
    }

    return receiver;
}

QBTextSubsManager QBTextSubsManagerCreate(SvApplication app, SvPlayerTask playerTask, const struct svdataformat *format, QBSubsManager subsManager)
{
    QBTextSubsManager self = NULL;

    if (!app || !playerTask || !format) {
        log_error(" wrong params ");
        return NULL;
    }

    self = (QBTextSubsManager) SvTypeAllocateInstance(QBTextSubsManager_getType(), NULL);

    self->app = app;
    self->subsManager = SVRETAIN(subsManager);
    self->format = format;
    svdataformat_retain(self->format);
    self->playerTask = SVRETAIN(playerTask);
    if (format->es.txt_subs_cnt == 0) {
        return self;
    }

    self->receiver = QBTextSubsReceiverCreate(self);
    self->smptettPngDecoder = QBSMPTETTPngDecoderCreate(SvSchedulerGet());

    svSettingsPushComponent("TextSubtitles.settings");
    self->font = svSettingsCreateFont("TextSubtitles", "font");
    self->boldFont = svSettingsCreateFont("TextSubtitles", "boldFont");
    svSettingsPopComponent();

    self->defaultWidth = app->width;
    self->defaultHeight = app->height;

    QBSubsManagerAddPlugin(self->subsManager, (SvObject) self, NULL);

    return self;
}
