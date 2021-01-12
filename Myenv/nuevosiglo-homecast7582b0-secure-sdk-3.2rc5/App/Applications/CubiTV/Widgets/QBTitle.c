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

#include "QBTitle.h"

#include <SvCore/SvCommonDefs.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <Services/core/QBTextRenderer.h>
#include <QBWidgets/QBAsyncLabel.h>

#define MAX_PARTS 4

typedef struct QBTitle_t* QBTitle;
struct QBTitle_t {
    SvWidget parts[MAX_PARTS];
    SvWidget separators[MAX_PARTS];
    SvString partsText[MAX_PARTS];      /* displayed text */
    SvString currentText[MAX_PARTS];    /* text, which should be displayed */

    QBTextRenderer renderer;
    int spacing;
    SvWidget frame;
    int separatorsOffY;
    int partsOffY;
    int maxWidth;
    int renderingNow;                   /* part currently rendered */

    QBTitleCallback callback;
    void *callbackData;
};

/**
 * Start rendering the current (self->renderingNow) part of the given QBTitle.
 *
 * The function sets the position of the separator that is preceding the current part and sets its
 * transparency to ALPHA_SOLID.  Next it sets the position of the title's part label and requests
 * a width and text change.
 *
 * The function must be called after the previous parts have been rendered.  The function doesn't
 * change the transparency or positions following the current part.  It is assumed that they are
 * already invisible (transparent).
 *
 * When the text change finishes, the QBTitleLabelCallback will be called.  This will in turn call
 * QBTitleResumeRender if necessary to draw following parts of the title.
 *
 * @param[in] self    QBTitle object
 */
SvLocal
void QBTitleResumeRender(QBTitle self)
{
    int pos = self->renderingNow;
    assert((pos >= 0) && (pos < MAX_PARTS));
    assert(!SvObjectEquals((SvObject) self->currentText[pos], (SvObject) self->partsText[pos]));

    int off_x = 0;
    if (pos > 0) {
        /* display preceding separator */
        self->separators[pos - 1]->off_x = self->parts[pos - 1]->off_x + QBAsyncLabelGetWidth(self->parts[pos - 1]) + self->spacing;
        int currentWidth = self->separators[pos - 1]->off_x + self->separators[pos - 1]->width;
        /* check if separator would fit into allowed space, then show it */
        if (self->maxWidth > currentWidth) {
            self->separators[pos - 1]->tree_alpha = ALPHA_SOLID;
            svWidgetForceRender(self->separators[pos - 1]);
        }
        off_x = currentWidth + self->spacing;
    }

    self->parts[pos]->off_x = off_x;

    SVRELEASE(self->partsText[pos]);
    self->partsText[pos] = SVRETAIN(self->currentText[pos]);

    /* display new part only if there is any space left */
    if (self->maxWidth > off_x) {
        QBAsyncLabelSetWidth(self->parts[pos], self->maxWidth - off_x);
        QBAsyncLabelSetText(self->parts[pos], self->partsText[pos]);
    }
}

/**
 * Re-evaluate the total width of the widget and launch callback if necessary.
 *
 * @param[in] self    QBTitle object
 */
SvLocal void
QBTitleUpdateWidth(QBTitle self)
{
    /* find the last visible part */
    int pos = 0;
    for (; pos < self->renderingNow; ++pos) {
        if (!SvStringLength(self->partsText[pos]))
            break;
    }

    int totalWidth = 0;
    if (pos > 0) {
        totalWidth = self->parts[pos - 1]->off_x + QBAsyncLabelGetWidth(self->parts[pos - 1]);
    }

    /* update the width and launch callback if necessary */
    if (self->frame->width != totalWidth) {
        self->frame->width = totalWidth;
        if (self->callback) {
            self->callback(self->callbackData, self->frame);
        }
    }
}

/**
 * Callback function called when one of the title's labels changes.
 *
 * If necessary, this function calls QBTitleResumeRender to start rendering the next part of
 * the title.
 *
 * Also, this function updates the frame width by running QBTitleUpdateWidth
 *
 * @param[in] self_    the QBTitle object
 * @param[in] label    the label, which changed
 */
SvLocal void
QBTitleLabelCallback(void * self_, SvWidget label)
{
    QBTitle self = (QBTitle) self_;
    int renderNow = self->renderingNow;

    /* detect which label has just finished rendering */
    int renderFinished = 0;
    for (; renderFinished < MAX_PARTS; ++renderFinished) {
        if (self->parts[renderFinished] == label) {
            break;
        }
    }

    assert(renderFinished < MAX_PARTS);

    if (renderFinished > renderNow) {
        /* this is the reply to an old request, we are going to change the rendered element soon */
        return;
    }

    if (renderNow == MAX_PARTS) {
        /* this is one of the initialization callbacks or a reply to a clearing request */
        return;
    }

    assert(renderFinished == renderNow);
    assert(SvObjectEquals((SvObject) self->currentText[renderNow], (SvObject) self->partsText[renderNow]));

    /* the current part was rendered continue with next part */
    int renderNext = renderNow + 1;

    if ((renderNext >= MAX_PARTS) || (!SvStringLength(self->currentText[renderNext]))) {
        /* rendering finished */
        self->renderingNow = MAX_PARTS;
    } else {
        /* draw the next part */
        self->renderingNow = renderNext;
        QBTitleResumeRender(self);
    }

    /* update width */
    QBTitleUpdateWidth(self);
}

SvLocal
void QBTitleCleanup(SvApplication app, void *prv)
{
    QBTitle self = prv;
    int i;
    for (i = 0; i < MAX_PARTS; i++) {
        SVRELEASE(self->partsText[i]);
        SVRELEASE(self->currentText[i]);
    }
    free(self);
}

SvWidget
QBTitleNew(SvApplication app, char const *widgetName, QBTextRenderer renderer)
{
    QBTitle self = calloc(1, sizeof(*self));
    int width = svSettingsGetInteger(widgetName, "width", 0);
    int minWidth = svSettingsGetInteger(widgetName, "minWidth", width);
    int maxWidth = svSettingsGetInteger(widgetName, "maxWidth", width);
    int height = svSettingsGetInteger(widgetName, "height", 0);
    SvWidget title = svWidgetCreate(app, minWidth, height);
    title->prv = self;
    self->renderer = renderer;
    int i;
    char *partName, *separatorName;
    asprintf(&partName, "%s.part", widgetName);
    asprintf(&separatorName, "%s.separator", widgetName);
    self->partsOffY = svSettingsGetInteger(partName, "yOffset", 0);
    self->separatorsOffY = svSettingsGetInteger(separatorName, "yOffset", 0);

    /* The QBAsyncLabels set their text during creation, we're expecting a callback
     * from each of them */
    self->renderingNow = MAX_PARTS;

    for (i = 0; i < MAX_PARTS; i++) {
        self->parts[i] = QBAsyncLabelNew(app, partName, renderer);
        svWidgetAttach(title, self->parts[i], 0, self->partsOffY, 0);
        QBAsyncLabelSetCallback(self->parts[i], QBTitleLabelCallback, self);
        if (i < MAX_PARTS - 1) {
            self->separators[i] = svSettingsWidgetCreate(app, separatorName);
            self->separators[i]->tree_alpha = ALPHA_TRANSPARENT;
            svWidgetAttach(title, self->separators[i], 0, self->separatorsOffY, 0);
        }
        self->partsText[i] = SvStringCreate("", NULL);
        self->currentText[i] = SvStringCreate("", NULL);
    }
    self->maxWidth = maxWidth;
    free(partName);
    free(separatorName);
    self->spacing = svSettingsGetInteger(widgetName, "spacing", 0);
    self->frame = title;
    title->clean = QBTitleCleanup;
    return title;
}

void
QBTitleSetText(SvWidget w, SvString text, int pos)
{
    if (!w)
        return;

    QBTitle self = (QBTitle) w->prv;

    if (pos < 0 || pos >= MAX_PARTS)
        return;

    if (SvObjectEquals((SvObject) text, (SvObject) self->currentText[pos]))
        return;

    SVRELEASE(self->currentText[pos]);
    if (text)
        self->currentText[pos] = SVRETAIN(text);
    else
        self->currentText[pos] = SvStringCreate("", NULL);

    if (pos > self->renderingNow) {
        /* we're currently rendering a part preceding the changed one pos */
        return;
    }

    for (int i = 0; i < pos; ++i) {
        if (!SvStringLength(self->currentText[i])) {
            /* one of the parts preceding the changed one is empty, we don't need to redraw anything */
            return;
        }
    }

    /* remember what part is currently rendered */
    self->renderingNow = pos;

    /* hide everything that follows current part */
    for (int i = pos + 1; i < MAX_PARTS; i++) {
        self->separators[i - 1]->tree_alpha = ALPHA_TRANSPARENT;

        if (SvStringLength(self->partsText[i])) {
            SVRELEASE(self->partsText[i]);
            self->partsText[i] = SvStringCreate("", NULL);
            /* note: this may cause an immediate callback */
            QBAsyncLabelClear(self->parts[i]);
        }
    }

    if (SvStringLength(self->currentText[pos])) {
        /* start rendering changed part */
        self->renderingNow = pos;
        QBTitleResumeRender(self);
    } else {
        /* the part has been set to an empty string, there is no need to redraw anything */
        self->separators[pos - 1]->tree_alpha = ALPHA_TRANSPARENT;
        SVRELEASE(self->partsText[pos]);
        self->partsText[pos] = SvStringCreate("", NULL);
        self->renderingNow = MAX_PARTS;
        QBAsyncLabelClear(self->parts[pos]);
        /* parts of the title disappeared -- update width */
        QBTitleUpdateWidth(self);
    }
}

void
QBTitleSetCallback(SvWidget w, QBTitleCallback callback, void *callbackData)
{
    if (!w)
        return;
    QBTitle self = (QBTitle) w->prv;
    self->callback = callback;
    self->callbackData = callbackData;
}
