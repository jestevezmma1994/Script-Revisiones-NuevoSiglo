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

#include "QBTTMLSubtitle.h"

#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>

#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvTextLayout.h>
#include <CAGE/Text/SvBasicTextParagraph.h>
#include <CAGE/Text/SvTextRender.h>
#include <CAGE/Core/SvColor.h>
#include <CUIT/Core/widget.h>

#include <SvGfxHAL/SvGfxUtils.h>

#include <QBSubsManager.h>
#include <QBSMPTETTPngDecoder.h>

struct region_ {
    SvWidget background;
    SvWidget captions;
    SvString id;
    int renderedLines;
};

struct QBTTMLSubtitle_ {
    struct QBSubsManagerSubtitle_ super_;

    QBSMPTETTPngDecoder pngDecoder;

    SvArray paragraphs;
    SvArray divs;
    struct QBTTMLSubtitleRenderParams_ params_;

    SvWidget w;

    SvHashTable divsToPngDecoderTasks;
    SvHashTable pngDecoderTasksToDivs;

    SvHashTable bitmapIdsToGlobalBitmaps;
    SvHashTable globalBitmapIdsToTasks;

    SvHashTable divsToBackgroundBitmaps;

    struct region_ *regions;
    unsigned int regionsSize;
    unsigned int regionCnt;
};


SvLocal int lengthDescToPx(struct lengthDesc *desc, int absValue)
{
    switch (desc->unitType) {
        case unitType_percentage:
            return (absValue * desc->v) / 100;
        case unitType_pixel:
            return (int) desc->v;
        default:
            return 0;
    }
}

SvLocal int
getLineCount(SvString text_)
{
    const char *text = SvStringCString(text_);
    int newLineCnt = 1;
    for (size_t i = 0; i < strlen(text); ++i) {
        if (text[i] == '\n') {
            newLineCnt++;
        }
    }
    return newLineCnt;
}

SvLocal void
drawParagraph(QBTTMLSubtitle self, QBTTMLParagraph par, struct region_ *region, SvString text)
{
    SvBitmap canvas;
    SvBasicTextParagraph paragraph = NULL;
    SvTextLayout layout = NULL;

    SvColor textColor = self->params_.manual_.useColor ?
                        self->params_.manual_.color :
                        ((QBTTMLContainer) ((QBTTMLContainer) par)->region)->style->textColor;

    SvErrorInfo error = NULL;

    if (!region->captions || !(canvas = region->captions->bmp)) {
        return;
    }

    if (text) {
        paragraph = SvBasicTextParagraphCreate(SvStringCString(text), -1, self->params_.font, SvFontGetSize(self->params_.font), textColor, &error);
        if (unlikely(!paragraph)) {
            goto out;
        }
        SvBasicTextParagraphSetLimits(paragraph, canvas->width, 5, &error);
        if (unlikely(error)) {
            goto out;
        }

        layout = SvTextLayoutCreate(&error);
        if (unlikely(!layout)) {
            goto out;
        }

        SvTextLayoutProcessCharacters(layout, (SvObject) paragraph, SvFontGetRenderingMode(self->params_.font), &error);
        if (unlikely(error)) {
            goto out;
        }

        unsigned int i, cnt;
        cnt = SvTextLayoutGetLinesCount(layout);
        for (i = 0; i < cnt; i++) {
            const SvTextLayoutLine *line = SvTextLayoutGetLine(layout, i);
            canvas->color = textColor;
            SvBitmapDrawTextLine(canvas, (canvas->width - line->boundingBox.width) / 2 - line->boundingBox.x,
                                 self->params_.baseLinePos + region->renderedLines * self->params_.lineHeight, line, NULL);
            ++region->renderedLines;
        }
    }
out:
    SVTESTRELEASE(paragraph);
    SVTESTRELEASE(layout);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal SvWidget
initBackgroundImage(QBTTMLSubtitle self, SvBitmap bmp, QBTTMLStyle style)
{
    int off_x = lengthDescToPx(&(style->origin.x), self->w->width);
    int off_y = lengthDescToPx(&(style->origin.y), self->w->height);

    int w = lengthDescToPx(&(style->extent.w), self->w->width);
    int h = lengthDescToPx(&(style->extent.h), self->w->height);

    SvWidget backgroundImage = svWidgetCreateBitmap(self->w->app, w, h, bmp);
    svWidgetAttach(self->w, backgroundImage, off_x, off_y, 1);
    SvGfxBlitBitmap(backgroundImage->bmp, NULL);
    SvBitmapBlank(backgroundImage->bmp, NULL);
    svWidgetForceRender(self->w);
    return backgroundImage;
}

SvLocal void
initRegion(QBTTMLSubtitle self, struct region_ *region, QBTTMLStyle style)
{
    int off_x, off_y, w, h, captionW, captionH;
    SvBitmap bmp;

    off_x = lengthDescToPx(&(style->origin.x), self->w->width);
    off_y = lengthDescToPx(&(style->origin.y), self->w->height);

    w = lengthDescToPx(&(style->extent.w), self->w->width);
    h = lengthDescToPx(&(style->extent.h), self->w->height);

    captionW = lengthDescToPx(&(style->extent.w), self->params_.nativeWidth);
    captionH = lengthDescToPx(&(style->extent.h), self->params_.nativeHeight);

    region->background = svWidgetCreateWithColor(self->w->app, w, h, COLOR(0, 0, 0, 0x00));
    region->captions = svWidgetCreateWithColor(self->w->app, w, h, COLOR(0, 0, 0, 0));

    svWidgetAttach(self->w, region->background, off_x, off_y, 1);
    svWidgetAttach(region->background, region->captions, 0, 0, 1);

    bmp = SvBitmapCreateAndFill(captionW, captionH, SV_CS_MONO, 0);
    if (bmp) {
        svWidgetSetBitmap(region->captions, bmp);
        SVRELEASE(bmp);
    }
}

SvLocal struct region_ *
findRegion(QBTTMLSubtitle self, QBTTMLRegion region)
{
    for (unsigned int i = 0; i < self->regionCnt; i++) {
        if (SvObjectEquals((SvObject) self->regions[i].id, (SvObject) ((QBTTMLContainer) region)->id)) {
            return &self->regions[i];
        }
    }

    // make sure that we wont exceed array size
    assert(self->regionCnt < self->regionsSize);

    self->regions[self->regionCnt].id = ((QBTTMLContainer) region)->id;
    self->regions[self->regionCnt].renderedLines = 0;

    initRegion(self, &self->regions[self->regionCnt], ((QBTTMLContainer) region)->style);

    ++self->regionCnt;

    return &self->regions[self->regionCnt - 1];
}


SvLocal void
QBTTMLSubtitlePngDecoderCallback(SvObject self_, QBSMPTETTPngDecoderTask task)
{
    QBTTMLSubtitle self = (QBTTMLSubtitle) self_;

    SvObject divElem = SvHashTableFind(self->pngDecoderTasksToDivs, (SvObject) task);
    if (!divElem) {
        return;
    }

    SvString backgroundImageId = ((QBTTMLDiv) divElem)->backgroundImageId;
    if (QBSMPTETTPngDecoderTaskGetStatus(task) == QBSMPTETTPngDecoderTaskStatus_completed) {
        if (backgroundImageId) {
            SvHashTableInsert(self->bitmapIdsToGlobalBitmaps, (SvObject) backgroundImageId, (SvObject) QBSMPTETTPngDecoderTaskGetBitmap(task));
        } else {
            SvHashTableInsert(self->divsToBackgroundBitmaps, divElem, (SvObject) QBSMPTETTPngDecoderTaskGetBitmap(task));
        }
    }

    SvHashTableRemove(self->divsToPngDecoderTasks, (SvObject) divElem);
    SvHashTableRemove(self->pngDecoderTasksToDivs, (SvObject) task);
    if (backgroundImageId) {
        SvHashTableRemove(self->globalBitmapIdsToTasks, (SvObject) backgroundImageId);
    }
}

SvLocal void
QBTTMLSubtitleDecode(QBSubsManagerSubtitle self_)
{
    QBTTMLSubtitle self = (QBTTMLSubtitle) self_;

    SvIterator it = SvArrayIterator(self->divs);
    QBTTMLDiv divElem;

    while ((divElem = (QBTTMLDiv) SvIteratorGetNext(&it))) {
        if (!divElem->backgroundImage || SvHashTableFind(self->divsToPngDecoderTasks, (SvObject) divElem)) {
            continue;
        }

        if (divElem->backgroundImageId) {
            SvBitmap bmp = (SvBitmap) SvHashTableFind(self->bitmapIdsToGlobalBitmaps, (SvObject) divElem->backgroundImageId);
            if (bmp) {
                continue;
            }
            if (SvHashTableFind(self->globalBitmapIdsToTasks, (SvObject) divElem->backgroundImageId)) {
                continue;
            }
        }

        QBSMPTETTPngDecoderTask pngDecoderTask = QBSMPTETTPngDecoderAddTask(self->pngDecoder, divElem->backgroundImage, NULL);
        SvHashTableInsert(self->divsToPngDecoderTasks, (SvObject) divElem, (SvObject) pngDecoderTask);
        SvHashTableInsert(self->pngDecoderTasksToDivs, (SvObject) pngDecoderTask, (SvObject) divElem);

        if (divElem->backgroundImageId) {
            SvHashTableInsert(self->globalBitmapIdsToTasks, (SvObject) divElem->backgroundImageId, (SvObject) pngDecoderTask);
        }
    }
}

SvLocal SvWidget
QBTTMLSubtitleShow(QBSubsManagerSubtitle self_, SvWidget w, SvDeque subtitlesDisplayed)
{
    QBTTMLSubtitle self = (QBTTMLSubtitle) self_;

    self->w = svWidgetCreateBitmap(w->app, w->width, w->height, NULL);
    svWidgetAttach(w, self->w, (w->width - self->w->width) / 2, (w->height - self->w->height) / 2, 1);

    QBTTMLParagraph par;
    SvIterator it = SvArrayIterator(self->paragraphs);
    while ((par = (QBTTMLParagraph) SvIteratorGetNext(&it))) {
        SvString text = QBTTMLParagraphCreateText(par);
        int lineCnt = getLineCount(text);
        QBTTMLStyle style = ((QBTTMLContainer) ((QBTTMLContainer) par)->region)->style;
        unsigned int captionH = (unsigned int) lengthDescToPx(&(style->extent.h), self->params_.nativeHeight);
        if (captionH < self->params_.lineHeight * lineCnt) {
            // in case of multiple lines we scale extent and origin
            int targetHeight = self->params_.lineHeight * lineCnt;
            double newExtent = targetHeight * style->extent.h.v / captionH;
            style->extent.h.v = newExtent;
            int off_y = lengthDescToPx(&(style->origin.y), self->w->height);
            int newOff_y = off_y - (targetHeight - captionH);
            int newOrigin = newOff_y * style->origin.y.v / off_y;
            style->origin.y.v = newOrigin;
        }
        struct region_ *region = findRegion(self, ((QBTTMLContainer) par)->region);
        //SvLogNotice("draw region: %s", SvStringCString(region->id));
        drawParagraph(self, par, region, text);
        SVRELEASE(text);
    }

    QBTTMLDiv divElem;
    it = SvArrayIterator(self->divs);
    while ((divElem = (QBTTMLDiv) SvIteratorGetNext(&it))) {
        SvBitmap backgroundBitmap = NULL;
        if (divElem->backgroundImageId) {
            backgroundBitmap = (SvBitmap) SvHashTableFind(self->bitmapIdsToGlobalBitmaps, (SvObject) divElem->backgroundImageId);
        } else {
            backgroundBitmap = (SvBitmap) SvHashTableFind(self->divsToBackgroundBitmaps, (SvObject) divElem);
        }

        if (!backgroundBitmap) {
            continue;
        }

        if (!((QBTTMLContainer) ((QBTTMLContainer) divElem)->region)->id) {
            QBTTMLStyle style = ((QBTTMLContainer) divElem)->style;
            initBackgroundImage(self, (SvBitmap) backgroundBitmap, style);
        } else {
            struct region_ *region = findRegion(self, ((QBTTMLContainer) divElem)->region);
            svWidgetSetBitmap(region->background, (SvBitmap) backgroundBitmap);
        }
        SvHashTableRemove(self->divsToBackgroundBitmaps, (SvObject) divElem);
    }

    for (unsigned int i = 0; i < self->regionCnt; i++) {
        if (self->regions[i].captions && self->regions[i].captions->bmp) {
            SvGfxBlitBitmap(self->regions[i].captions->bmp, NULL);
            SvBitmapBlank(self->regions[i].captions->bmp, NULL);
        }
        if (self->regions[i].background && self->regions[i].background->bmp) {
            SvGfxBlitBitmap(self->regions[i].background->bmp, NULL);
            SvBitmapBlank(self->regions[i].background->bmp, NULL);
        }
    }

    return self->w;
}

SvLocal void
QBTTMLSubtitleRemove(QBSubsManagerSubtitle self_)
{
    QBTTMLSubtitle self = (QBTTMLSubtitle) self_;

    if (self->pngDecoderTasksToDivs && SvHashTableCount(self->pngDecoderTasksToDivs)) {
        SvIterator it = SvHashTableKeysIterator(self->pngDecoderTasksToDivs);
        QBSMPTETTPngDecoderTask task;
        while ((task = (QBSMPTETTPngDecoderTask) SvIteratorGetNext(&it)))
            QBSMPTETTPngDecoderCancelTask(self->pngDecoder, task);

        SvHashTableRemoveAllObjects(self->pngDecoderTasksToDivs);
        SvHashTableRemoveAllObjects(self->divsToPngDecoderTasks);
        SvHashTableRemoveAllObjects(self->divsToBackgroundBitmaps);
        SvHashTableRemoveAllObjects(self->bitmapIdsToGlobalBitmaps);
        SvHashTableRemoveAllObjects(self->globalBitmapIdsToTasks);
    }

    if (self->w) {
        svWidgetDetach(self->w);
        svWidgetDestroy(self->w);
        self->w = NULL;
    }
}

SvLocal void
QBTTMLSubtitleDestroy(void *self_)
{
    QBTTMLSubtitle self = self_;

    if (self->pngDecoderTasksToDivs && SvHashTableCount(self->pngDecoderTasksToDivs)) {
        SvIterator it = SvHashTableKeysIterator(self->pngDecoderTasksToDivs);
        QBSMPTETTPngDecoderTask task;
        while ((task = (QBSMPTETTPngDecoderTask) SvIteratorGetNext(&it))) {
            QBSMPTETTPngDecoderCancelTask(self->pngDecoder, task);
        }
    }

    QBSMPTETTPngDecoderRemoveListener(self->pngDecoder, (SvObject) self);

    SVTESTRELEASE(self->paragraphs);
    SVTESTRELEASE(self->divs);
    SVTESTRELEASE(self->params_.font);
    SVTESTRELEASE(self->pngDecoder);
    SVTESTRELEASE(self->divsToPngDecoderTasks);
    SVTESTRELEASE(self->pngDecoderTasksToDivs);
    SVTESTRELEASE(self->divsToBackgroundBitmaps);
    SVTESTRELEASE(self->bitmapIdsToGlobalBitmaps);
    SVTESTRELEASE(self->globalBitmapIdsToTasks);

    free(self->regions);
}


SvLocal SvType QBTTMLSubtitle_getType(void)
{
    static SvType type = NULL;
    static const struct QBSubsManagerSubtitleVTable_ subtitleVTable = {
        .super_      = {
            .destroy = QBTTMLSubtitleDestroy
        },
        .decode      = QBTTMLSubtitleDecode,
        .show        = QBTTMLSubtitleShow,
        .remove      = QBTTMLSubtitleRemove,
    };
    static const struct QBSMPTETTPngDecoderListener_t pngDecoderListenerMethods = {
        .taskCompleted = QBTTMLSubtitlePngDecoderCallback
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTTMLSubtitle",
                            sizeof(struct QBTTMLSubtitle_),
                            QBSubsManagerSubtitle_getType(),
                            &type,
                            QBSubsManagerSubtitle_getType(), &subtitleVTable,
                            QBSMPTETTPngDecoderListener_getInterface(), &pngDecoderListenerMethods,
                            NULL);
    }

    return type;
}

QBTTMLSubtitle QBTTMLSubtitleCreate(SvArray paragraphs,
                                    SvArray divs,
                                    QBSMPTETTPngDecoder pngDecoder,
                                    QBTTMLSubtitleRenderParams params,
                                    SvErrorInfo *errorOut)
{
    QBTTMLSubtitle self = NULL;
    SvErrorInfo error = NULL;
    SvIterator it;


    if (!(self = (QBTTMLSubtitle) SvTypeAllocateInstance(QBTTMLSubtitle_getType(), &error))) {
        goto out;
    }

    it = SvArrayIterator(paragraphs);
    self->paragraphs = SvArrayCreateWithValues(&it, NULL);

    it = SvArrayIterator(divs);
    self->divs = SvArrayCreateWithValues(&it, NULL);

    self->divsToPngDecoderTasks = SvHashTableCreate(100, NULL);
    self->pngDecoderTasksToDivs = SvHashTableCreate(100, NULL);
    self->divsToBackgroundBitmaps = SvHashTableCreate(100, NULL);
    self->bitmapIdsToGlobalBitmaps = SvHashTableCreate(100, NULL);
    self->globalBitmapIdsToTasks = SvHashTableCreate(100, NULL);

    self->params_ = *params;

    SVRETAIN(self->params_.font);

    self->pngDecoder = SVTESTRETAIN(pngDecoder);
    QBSMPTETTPngDecoderAddListener(self->pngDecoder, (SvObject) self);

    self->regionsSize = SvArrayCount(self->paragraphs) >= SvArrayCount(self->divs) ?
                        SvArrayCount(self->paragraphs) : SvArrayCount(self->divs);
    self->regions = calloc(self->regionsSize, sizeof(struct region_));

out:
    SvErrorInfoPropagate(error, errorOut);

    if (error) {
        SVTESTRELEASE(self);
        self = NULL;
    }

    return self;
}
