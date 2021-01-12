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

#include "QBTimeProgress.h"

#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <Widgets/QBRoundedLabel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <SWL/gauge.h>
#include <math.h>

struct QBTimeProgress_t {
    SvWidget marker, bar;
    SvWidget startLabel, currentLabel, endLabel;
    SvWidget contentWidget;
    bool contentEnabled;
    bool widgetEnabled;
    double maxTime, currentTime;
    int markerXMin, markerXMax;
    SvBitmap bmp;
};

typedef struct QBTimeProgress_t *QBTimeProgress;

SvLocal void
QBTimeProgressBarCleanup(SvApplication app, void *prv_)
{
    QBTimeProgress self = (QBTimeProgress)prv_;
    SVRELEASE(self->bmp);
}

SvLocal SvWidget
QBTimeProgressCreateProgressBar(SvApplication app, QBTimeProgress self, const char *name)
{
    int width = svSettingsGetInteger(name, "width", 40);
    int height = svSettingsGetInteger(name, "height", 40);
    SvWidget w = svWidgetCreateBitmap(app, width, height, NULL);

    self->bmp = SVRETAIN(svSettingsGetBitmap(name, "bg"));
    int borderWidth = self->bmp->width / 2;

    SvWidget tmp;
    tmp = svWidgetCreateBitmap(app, borderWidth, height, self->bmp);
    tmp->bm_w = borderWidth;
    svWidgetAttach(w, tmp, 0, 0, 1);

    tmp = svWidgetCreateBitmap(app, width - 2 * borderWidth, height, self->bmp);
    tmp->bm_x = borderWidth;
    tmp->bm_w = 1;
    svWidgetAttach(w, tmp, borderWidth, 0, 1);

    tmp = svWidgetCreateBitmap(app, borderWidth, height, self->bmp);
    tmp->bm_x = borderWidth + 1;
    tmp->bm_w = borderWidth;
    svWidgetAttach(w, tmp, width - borderWidth, 0, 1);

    w->prv = self;
    w->clean = QBTimeProgressBarCleanup;

    return w;
}

SvWidget
QBTimeProgressCreate(SvApplication app, const char *name, QBTextRenderer renderer)
{
    SvWidget w = svSettingsWidgetCreate(app, name);
    QBTimeProgress prv = calloc(1, sizeof(*prv));
    w->prv = prv;
    prv->widgetEnabled = true;
    char *cname = NULL;

    asprintf(&cname, "%s.marker", name);
    prv->marker = svSettingsWidgetCreate(app, cname);
    svSettingsWidgetAttach(w, prv->marker, cname, 3);
    free(cname);

    asprintf(&cname, "%s.bar", name);
    prv->bar = QBTimeProgressCreateProgressBar(app, prv, cname);
    svSettingsWidgetAttach(w, prv->bar, cname, 1);
    free(cname);

    asprintf(&cname, "%s.availableContent", name);
    prv->contentEnabled = false;
    prv->contentWidget = svGaugeNewFromSM(app, cname, SV_GAUGE_HORIZONTAL, SV_GAUGE_STYLE_BASIC, 0, 1, 0);
    if (prv->contentWidget) {
        svSettingsWidgetAttach(w, prv->contentWidget, cname, 2);
        svWidgetSetHidden(prv->contentWidget, true);
    }
    free(cname);

    asprintf(&cname, "%s.startLabel", name);
    prv->startLabel = QBAsyncLabelNew(app, cname, renderer);
    svSettingsWidgetAttach(w, prv->startLabel, cname, 1);
    free(cname);

    asprintf(&cname, "%s.currentLabel", name);
    prv->currentLabel = QBRoundedLabelNew(app, cname);
    svSettingsWidgetAttach(w, prv->currentLabel, cname, 1);
    free(cname);

    asprintf(&cname, "%s.endLabel", name);
    prv->endLabel = QBAsyncLabelNew(app, cname, renderer);
    QBAsyncLabelToggleCache(prv->endLabel, false);
    svSettingsWidgetAttach(w, prv->endLabel, cname, 1);
    free(cname);

    prv->markerXMin = svSettingsGetInteger(name, "markerXMin", 0);
    prv->markerXMax = svSettingsGetInteger(name, "markerXMax", w->width);
    prv->maxTime = 2*60*60*1000; //2h ;)

    QBAsyncLabelSetText(prv->startLabel, SVSTRING("00:00:00"));
    prv->currentTime = -1.0;
    QBTimeProgressSetCurrentTime(w, 0);
    return w;
}

SvLocal void
QBTimeProgressRefreshMarker(SvWidget w, double timeEffect)
{
    QBTimeProgress prv = (QBTimeProgress)w->prv;
    svWidgetSetHidden(prv->marker, false);
    double pos = 0;

    if (prv->maxTime > 0)
        pos = timeEffect / prv->maxTime;

    if (pos > 1)
        pos = 1;

    int newOffX = prv->markerXMin + ((prv->markerXMax - prv->markerXMin) * pos);
    if (newOffX == prv->marker->off_x)
        return;

    prv->marker->off_x = newOffX;
    svWidgetForceRender(prv->marker);

    if (prv->currentLabel) {
        prv->currentLabel->off_x = prv->marker->off_x + prv->marker->width;
        svWidgetForceRender(prv->currentLabel);
    }
}

void
QBTimeProgressSetCurrentTime(SvWidget w, double timeEffect)
{
    QBTimeProgress prv = (QBTimeProgress)w->prv;
    QBTimeProgressRefreshMarker(w, timeEffect);
    if (floor(prv->currentTime) == floor(timeEffect)) {
        return;
    }
    prv->currentTime = timeEffect;
    if (!prv->maxTime) {
        svWidgetSetHidden(prv->marker, true);
        return;
    }
    if (prv->currentLabel) {
        int sec = floor(timeEffect);
        SvString str = SvStringCreateWithFormat("%02i:%02i:%02i",
                                                (sec/60)/60,
                                                (sec/60)%60,
                                                sec%60);
        QBRoundedLabelSetText(prv->currentLabel, str);
        SVRELEASE(str);
    }
}

void
QBTimeProgressSetMaxTime(SvWidget w, double maxTime)
{
    QBTimeProgress prv = (QBTimeProgress)w->prv;
    if (floor(prv->maxTime * 100) == floor(maxTime * 100)) {
        return;
    }
    prv->maxTime = maxTime;
    int sec = floor(maxTime);
    SvString str = SvStringCreateWithFormat("%02i:%02i:%02i",
                                            (sec/60)/60,
                                            (sec/60)%60,
                                             sec%60);
    QBAsyncLabelSetText(prv->endLabel, str);
    SVRELEASE(str);
    QBTimeProgressRefreshMarker(w, prv->currentTime);
}

void
QBTimeProgressSetEnabled(SvWidget w, bool enabled)
{
    QBTimeProgress prv = (QBTimeProgress)w->prv;
    prv->widgetEnabled = enabled;

    svWidgetSetHidden(prv->marker, !enabled);
    svWidgetSetHidden(prv->startLabel, !enabled);
    svWidgetSetHidden(prv->currentLabel, !enabled);
    svWidgetSetHidden(prv->endLabel, !enabled);
    if (prv->contentWidget) {
        svWidgetSetHidden(prv->contentWidget, !prv->widgetEnabled || !prv->contentEnabled);
    }
}

void
QBTimeProgressSetAvailableContent(SvWidget w, time_t start, time_t end)
{
    QBTimeProgress prv = (QBTimeProgress) w->prv;
    if (prv->contentWidget) {
        svGaugeSetBounds(prv->contentWidget, 0, prv->maxTime);
        svGaugeSetInterval(prv->contentWidget, start, end);
    }
}

void
QBTimeProgressShowAvailableContent(SvWidget w)
{
    QBTimeProgress prv = (QBTimeProgress) w->prv;
    if (prv->contentWidget) {
        prv->contentEnabled = true;
        QBTimeProgressSetEnabled(w, prv->widgetEnabled);
    }
}

void
QBTimeProgressHideAvailableContent(SvWidget w)
{
    QBTimeProgress prv = (QBTimeProgress) w->prv;
    if (prv->contentWidget) {
        prv->contentEnabled = false;
        QBTimeProgressSetEnabled(w, prv->widgetEnabled);
    }
}
