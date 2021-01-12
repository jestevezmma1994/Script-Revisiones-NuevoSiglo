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

#include "QBScrollBar.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <settings.h>
#include <SWL/icon.h>
#include <SWL/QBFrame.h>
#include <SWL/rangelistener.h>
#include <CUIT/Core/widget.h>
#include <string.h>


typedef enum {
    QBScrollBarMode_Vertical,
    QBScrollBarMode_Horizontal,
} QBScrollBarMode;

struct QBScrollBar_t {
    struct SvObject_ super_;
    SvWidget arrowLow, arrowHigh;
    bool arrowLowActive, arrowHighActive;
    size_t lower, upper, activeLower, activeUpper;
    double arrowFadeDuration;
    SvWidget bar, barBg;
    int barMaxOffset, barMinOffset, barMinSize;
    QBScrollBarMode mode;
    SvWidget box;
    size_t viewSize;
};

typedef struct QBScrollBar_t *QBScrollBar;

SvLocal void
QBScrollBar__dtor__(void *self_){
    //QBScrollBar self = (QBScrollBar)self_;
}

SvLocal void
QBScrollBarRefresh(QBScrollBar self){
    bool enable;
    enable = self->activeLower!=self->lower || self->activeUpper != self->upper;

    if(enable){
        self->arrowLow->tree_alpha = ALPHA_SOLID;
        self->arrowHigh->tree_alpha = ALPHA_SOLID;
        bool active;
        active = self->activeLower!=self->lower;
        if(active!=self->arrowLowActive){
            self->arrowLowActive = active;
            svIconSwitch(self->arrowLow, active ? 1 : 0, 0, -1.0);
        }

        active = self->activeUpper!=self->upper;
        if(active!=self->arrowHighActive){
            self->arrowHighActive = active;
            svIconSwitch(self->arrowHigh, active ? 1 : 0, 0, -1.0);
        }
    }else{
        self->arrowLow->tree_alpha = ALPHA_TRANSPARENT;
        self->arrowHigh->tree_alpha = ALPHA_TRANSPARENT;
    }

    if(self->bar){
        if(self->viewSize > 0 && self->upper - self->lower <= self->viewSize)
            enable = false;
        svWidgetSetHidden(self->bar, !enable);
        if(enable) {
            size_t sizeAll, sizeActive;
            size_t upper = self->upper;
            size_t activeUpper = self->activeUpper;
            size_t lower = self->lower;
            size_t activeLower = self->activeLower;

            if(self->viewSize > 0 && self->activeUpper - self->activeLower < self->viewSize) {
                if(activeLower == lower)
                    activeUpper = lower + self->viewSize;
                if(activeUpper == upper)
                    activeLower = upper - self->viewSize;
                if(activeLower < lower)
                    lower = activeLower;
                if(activeUpper > upper)
                    upper = activeUpper;
            }

            if((sizeAll = upper - self->lower) > 0 &&
                    (sizeActive = activeUpper - activeLower) > 0){
                int barSpace = self->barMaxOffset - self->barMinOffset;
                int barSize = sizeActive * barSpace / sizeAll;
                if(barSize < self->barMinSize)
                    barSize = self->barMinSize;
                int space = sizeAll - sizeActive;
                int offset = space ? (activeLower - lower) * (barSpace - barSize) / space : 0;
                offset += self->barMinOffset;
                if(self->mode == QBScrollBarMode_Vertical){
                    QBFrameSetHeight(self->bar, barSize);
                    self->bar->off_y = offset;
                }else{
                    QBFrameSetWidth(self->bar, barSize);
                    self->bar->off_x = offset;
                }
            }
        }
    }
}

SvLocal void
QBScrollBarRangeChanged(SvGenericObject self_, size_t lower, size_t upper){
    QBScrollBar self = (QBScrollBar)self_;
    self->lower = lower;
    self->upper = upper;
    //QBScrollBarRefresh(self);
}

SvLocal void
QBScrollBarActiveRangeChanged(SvGenericObject self_, size_t lower, size_t upper){
    QBScrollBar self = (QBScrollBar)self_;
    self->activeLower = lower;
    self->activeUpper = upper;
    QBScrollBarRefresh(self);
}

SvLocal SvType QBScrollBar_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBScrollBar__dtor__
    };
    static SvType type = NULL;

    static const struct SvRangeListener_t rangeListener = {
        .rangeChanged = QBScrollBarRangeChanged,
        .activeRangeChanged = QBScrollBarActiveRangeChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBScrollBar",
                            sizeof(struct QBScrollBar_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            SvRangeListener_getInterface(), &rangeListener,
                            NULL);
    }

    return type;
}

SvLocal SvWidget
createArrowIcon(SvApplication app, const char *name, int width_, int height_)
{
    SvBitmap inactiveBmp = svSettingsGetBitmap(name, "inactive");
    SvBitmap activeBmp = svSettingsGetBitmap(name, "active");
    int width = width_;
    int height = height_;

    if (width == -1)
        width = (inactiveBmp->width > activeBmp->width) ? inactiveBmp->width : activeBmp->width;
    if (height == -1)
        height = (inactiveBmp->height > activeBmp->height) ? inactiveBmp->height : activeBmp->height;

    SvWidget icon = svIconNewWithSize(app, width, height, true);
    svIconSetBitmap(icon, 0, inactiveBmp);
    svIconSetBitmap(icon, 1, activeBmp);

    return icon;
}

SvLocal void
QBScrollBarClean(SvApplication app, void *prv){
    QBScrollBar self = (QBScrollBar)prv;
    SVRELEASE(self);
}

SvGenericObject QBScrollBarGetRangeListener(SvWidget w){
    return (SvGenericObject)w->prv;

}

SvWidget
QBScrollBarNew(SvApplication app, const char *name)
{
    SvWidget box = svSettingsWidgetCreate(app, name);
    QBScrollBar self = (QBScrollBar)SvTypeAllocateInstance(QBScrollBar_getType(), NULL);

    box->clean = QBScrollBarClean;
    self->box = box;
    box->prv = self;

    char buf[128];

    snprintf(buf, sizeof(buf), "%s.%s", name, "arrowLow");
    int arrowWidth = svSettingsGetInteger(buf, "width", -1);
    int arrowHeight = svSettingsGetInteger(buf, "height", -1);
    self->arrowLow = createArrowIcon(app, buf, arrowWidth, arrowHeight);
    svSettingsWidgetAttach(box, self->arrowLow, buf, 10);

    snprintf(buf, sizeof(buf), "%s.%s", name, "arrowHigh");
    arrowWidth = svSettingsGetInteger(buf, "width", -1);
    arrowHeight = svSettingsGetInteger(buf, "height", -1);
    self->arrowHigh = createArrowIcon(app, buf, arrowWidth, arrowHeight);
    svSettingsWidgetAttach(box, self->arrowHigh, buf, 10);

    self->arrowFadeDuration = svSettingsGetDouble(name, "fadeDuration", 0.0);

    const char *mode = svSettingsGetString(name, "mode");
    if(mode && !strcmp(mode, "horizontal"))
        self->mode = QBScrollBarMode_Horizontal;
    else
        self->mode = QBScrollBarMode_Vertical;

    snprintf(buf, sizeof(buf), "%s.%s", name, "barBg");
    if(svSettingsIsWidgetDefined(buf)) {
        self->barBg = QBFrameCreateFromSM(app, buf);
        svSettingsWidgetAttach(box, self->barBg, buf, 1);
        snprintf(buf, sizeof(buf), "%s.%s", name, "bar");
        self->bar = QBFrameCreateFromSM(app, buf);
        svSettingsWidgetAttach(self->barBg, self->bar, buf, 0);

        self->barMaxOffset = svSettingsGetInteger(buf, "maxOffset",
               self->mode==QBScrollBarMode_Horizontal ? self->barBg->height : self->barBg->width);
        self->barMinOffset = svSettingsGetInteger(buf, "minOffset", 0);
        self->barMinSize = svSettingsGetInteger(buf, "minSize", 3);
        self->viewSize = svSettingsGetInteger(buf, "viewSize", 0);
    }


    QBScrollBarRefresh(self);

    return box;
}

SvWidget
QBScrollBarNewWithRatio(SvApplication app,
                        unsigned int parentWidth,
                        unsigned int parentHeight,
                        const char *name)
{
    SvWidget box = svSettingsWidgetCreateWithRatio(app, name, parentWidth, parentHeight);
    QBScrollBar self = (QBScrollBar)SvTypeAllocateInstance(QBScrollBar_getType(), NULL);

    box->clean = QBScrollBarClean;
    self->box = box;
    box->prv = self;

    char buf[128];

    snprintf(buf, sizeof(buf), "%s.%s", name, "arrowLow");
    double arrowWidthRatio = svSettingsGetDouble(buf, "widthRatio", -1.0);
    double arrowHeightRatio = svSettingsGetDouble(buf, "heightRatio", -1.0);
    int arrowWidth = -1;
    int arrowHeight = -1;
    if (arrowWidthRatio > 0.0)
        arrowWidth = (int) (arrowWidthRatio * (double) box->width);
    if (arrowHeightRatio > 0.0)
        arrowHeight = (int) (arrowHeightRatio * (double) box->height);
    self->arrowLow = createArrowIcon(app, buf, arrowWidth, arrowHeight);
    svSettingsWidgetAttachWithRatio(box, self->arrowLow, buf, 10);

    snprintf(buf, sizeof(buf), "%s.%s", name, "arrowHigh");
    arrowWidthRatio = svSettingsGetDouble(buf, "widthRatio", -1.0);
    arrowHeightRatio = svSettingsGetDouble(buf, "heightRatio", -1.0);
    arrowWidth = -1;
    arrowHeight = -1;
    if (arrowWidthRatio > 0.0)
        arrowWidth = (int) (arrowWidthRatio * (double) box->width);
    if (arrowHeightRatio > 0.0)
        arrowHeight = (int) (arrowHeightRatio * (double) box->height);
    self->arrowHigh = createArrowIcon(app, buf, arrowWidth, arrowHeight);
    svSettingsWidgetAttachWithRatio(box, self->arrowHigh, buf, 10);

    self->arrowFadeDuration = svSettingsGetDouble(name, "fadeDuration", 0.0);

    const char *mode = svSettingsGetString(name, "mode");
    if(mode && !strcmp(mode, "horizontal"))
        self->mode = QBScrollBarMode_Horizontal;
    else
        self->mode = QBScrollBarMode_Vertical;

    snprintf(buf, sizeof(buf), "%s.%s", name, "barBg");
    if(svSettingsIsWidgetDefined(buf)) {
        SvLogError("%s(): scroll bar background is not supported for ratio settings yet", __func__);
    }

    QBScrollBarRefresh(self);

    return box;
}
