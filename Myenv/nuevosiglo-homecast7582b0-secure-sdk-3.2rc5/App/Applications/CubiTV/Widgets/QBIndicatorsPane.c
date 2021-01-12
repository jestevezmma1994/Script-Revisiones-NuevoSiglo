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

#include "Widgets/QBIndicatorsPane.h"

#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/QBFrame.h>
#include <main.h>

#define log_debug(fmt, ...)  do { if (0) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)   do { if (1) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { if (1) SvLogError( COLBEG() fmt COLEND_COL(red),  ##__VA_ARGS__); } while (0)

typedef struct QBIndicatorsPane_t {
    struct SvObject_ super_;

    SvWidget bg;
    int spacing;

    int widthSum; /// of all indicators their spacings
} *QBIndicatorsPane;

enum {default_spacing=6};

SvLocal void
QBIndicatorsPaneClean(SvApplication app, void *ptr)
{
    QBIndicatorsPane self = ptr;
    SVRELEASE(self);
}

SvLocal void
QBIndicatorsPane__dtor__(void *self_)
{
    //QBIndicatorsPane self = self_;
    //Nothing to do...
}

SvLocal SvType
QBIndicatorsPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBIndicatorsPane__dtor__
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBIndicatorsPane",
                            sizeof(struct QBIndicatorsPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvWidget
QBIndicatorsPaneCreate(AppGlobals appGlobals, const char* widgetName)
{
    if (!appGlobals || !widgetName) {
        SvLogError("%s(%p, %p) -> NULL argument passed", __FUNCTION__, appGlobals, widgetName);
        return NULL;
    }
    if (!svSettingsIsWidgetDefined(widgetName))
        return NULL;

    //Widgets creation
    SvWidget w = svSettingsWidgetCreate(appGlobals->res, widgetName);
    if (!w) {
        log_error("%s: Can't create widget", __FUNCTION__);
        return NULL;
    }

    QBIndicatorsPane self = (QBIndicatorsPane) SvTypeAllocateInstance(QBIndicatorsPane_getType(), NULL);
    if (unlikely(!self)) {
        log_error("%s -> allocation failed", __FUNCTION__);
        svWidgetDestroy(w);
        return NULL;
    }

    self->spacing = svSettingsGetInteger(widgetName, "itemSpacing", default_spacing);

    //Widgets construction
    w->prv = self;
    w->clean = QBIndicatorsPaneClean;
    svWidgetSetFocusable(w, false);

    char buf[128];
    snprintf(buf, 128, "%s.Frame", widgetName);

    self->bg = QBFrameCreateFromSM(appGlobals->res, buf);
    svWidgetAttach(w, self->bg, 0, 0, 1);

    self->widthSum += self->spacing;

    return w;
}

void
QBIndicatorsPaneAddIndicator(SvWidget paneWgt, SvWidget indicator)
{
    if (!paneWgt || !indicator) {
        log_error("QBIndicatorsPaneAddIndicator(%p, %p)", paneWgt, indicator);
        return;
    }

    QBIndicatorsPane pane = paneWgt->prv;

    int y_off = (paneWgt->height - indicator->height) / 2;
    svWidgetAttach(paneWgt, indicator, pane->widthSum, y_off, 2);

    pane->widthSum += (indicator->width + pane->spacing);
    QBFrameSetWidth(pane->bg, pane->widthSum);
}
