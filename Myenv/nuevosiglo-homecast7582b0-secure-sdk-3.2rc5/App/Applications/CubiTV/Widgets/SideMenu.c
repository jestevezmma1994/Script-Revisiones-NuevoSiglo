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

#include "SideMenu.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/QBUIEvent.h>
#include <settings.h>
#include <SWL/QBFrame.h>


SvType
QBSideMenuLevelChangedEvent_getType(void)
{
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSideMenuLevelChangedEvent",
                            sizeof(struct QBSideMenuLevelChangedEvent_),
                            QBUIEvent_getType(),
                            &type,
                            NULL);
    }

    return type;
}


#define DT 1.0
#define DV 1.0
#define ST 0.3
#define SV 0.1
#define LDT 4.0

/**
 * Type describing how the effect step should be calculated.
 **/
typedef enum {
    SIDE_MENU_EFFECT_LINEAR,     ///< progress should be a linear function
    SIDE_MENU_EFFECT_SLOWIN,     ///< progress should be slower on the begining than on the end
    SIDE_MENU_EFFECT_SLOWOUT,    ///< progress should be slower on the end than on the begining
    SIDE_MENU_EFFECT_SLOWINOUT,  ///< progress should be faster in the middle than on the edges
    SIDE_MENU_EFFECT_LOGARITHM,  ///< progress should be a logarithmic function
} SideMenuEffectType;

struct SideMenuEffect {
    SideMenuEffectType type;
    double lt, lv, a, bi, bo;
    SvWidget *wgt;
    int *src_x, *dst_x;
    int *src_w, *dst_w;
    int count;
};

SvLocal double
SideMenuEffectProgress(SvEffect e, struct SideMenuEffect* m, double t)
{
    double x, v = 0.0;

    if (t > 1.0)
        t = 1.0;
    if (t < 0.0)
        t = 0.0;

    switch (m->type) {
        case SIDE_MENU_EFFECT_LINEAR:
            v = (m->lv / m->lt) * t;
            break;
        case SIDE_MENU_EFFECT_SLOWIN:
            if (t<ST)
                v = m->a * t * t + m->bi * t;
            else
                v = (m->lv / m->lt) * (t - ST) + SV;
            break;
        case SIDE_MENU_EFFECT_SLOWOUT:
            if (t > DT - ST) {
                x = t - (DT - ST);
                v = m->a * x * x + m->bo * x + (DV - SV);
            } else {
                v = (m->lv / m->lt) * t;
            }
            break;
        case SIDE_MENU_EFFECT_SLOWINOUT:
            if (t < ST) {
                v = m->a * t * t + m->bi * t;
            } else if (t > DT - ST) {
                x = t - (DT - ST);
                v = m->a * x * x + m->bo * x + (DV - SV);
            } else {
                v = (m->lv / m->lt) * (t - ST) + SV;
            }
            break;
        case SIDE_MENU_EFFECT_LOGARITHM:
            v = (1.0 - exp(-LDT * t)) / (1.0 - exp(-LDT));
    }

    return v <= 1.0 ? (v >= 0.0 ? v : 0.0) : 1.0;
}

SvLocal bool
SideMenuEffectStep(SvEffect e, double effectTime)
{
    struct SideMenuEffect* m=(struct SideMenuEffect*)e->prv;
    int new_x = 0;
    int new_w = 0;
    double delta;
    double progress;
    bool ret = true;

    delta = effectTime - e->start;
    progress = SideMenuEffectProgress(e, m, (e->duration == 0.0) ? 1.0 : delta/e->duration);


    int i;
    for (i = 0; i < m->count; ++i) {

        if (i == 0) {
            if (m->dst_x[i] - m->src_x[i] > 0)
                new_x = m->src_x[i] + (int) ceil(progress * (m->dst_x[i] - m->src_x[i]));
            else
                new_x = m->src_x[i] + (int) floor(progress * (m->dst_x[i] - m->src_x[i]));
        } else {
            new_x = new_x + new_w;
        }

        if (m->dst_w[i] - m->src_w[i] > 0)
            new_w = m->src_w[i] + (int) ceil(progress * (m->dst_w[i] - m->src_w[i]));
        else
            new_w = m->src_w[i] + (int) floor(progress * (m->dst_w[i] - m->src_w[i]));

        if (new_x != m->wgt[i]->off_x || new_w != m->wgt[i]->width) {
            m->wgt[i]->off_x = new_x;
            m->wgt[i]->width = new_w;
            svWidgetForceRender(m->wgt[i]);
        }

        if (m->wgt[i]->off_x != m->dst_x[i] || m->wgt[i]->width != m->dst_w[i])
            ret = false;
    }

    if (delta >= e->duration)
        ret = true;

    return ret;
}

SvLocal void
SideMenuEffectDone(SvEffect e, SvEffectCompletionType type)
{
    struct SideMenuEffect* m = (struct SideMenuEffect*) e->prv;
    int i;

    switch (type) {
        case SV_EFFECT_NORMAL:
        case SV_EFFECT_FINISH:
            for (i = 0; i < m->count; ++i) {
                m->wgt[i]->off_x = m->dst_x[i];
                m->wgt[i]->width = m->dst_w[i];
            }
            break;
        case SV_EFFECT_ABANDON:
            break;
        case SV_EFFECT_ROLLBACK:
            for (i = 0; i < m->count; ++i) {
                m->wgt[i]->off_x = m->src_x[i];
                m->wgt[i]->width = m->src_w[i];
            }
            break;
    }

    free(m->wgt);
    free(m->src_x);
    free(m->src_w);
    free(m->dst_x);
    free(m->dst_w);
    free(e->prv);
}

SvLocal SvEffect
SideMenuEffectNew(SvWidget *wgt, int *dst_x, int *dst_w, int count, double duration, SideMenuEffectType type)
{
    SvEffect effect = svEffectNew(*wgt, SideMenuEffectStep, SideMenuEffectDone, duration);
    struct SideMenuEffect* prv = calloc(1, sizeof(struct SideMenuEffect));

    prv->wgt = calloc(count, sizeof(SvWidget));
    prv->src_x = calloc(count, sizeof(int));
    prv->src_w = calloc(count, sizeof(int));
    prv->dst_x = calloc(count, sizeof(int));
    prv->dst_w = calloc(count, sizeof(int));

    prv->type = type;
    prv->count = count;

    int i;
    for (i = 0; i < count; ++i) {
        prv->wgt[i] = wgt[i];
        prv->src_x[i] = wgt[i]->off_x;
        prv->src_w[i] = wgt[i]->width;
        prv->dst_x[i] = dst_x[i];
        prv->dst_w[i] = dst_w[i];
    }

    switch (type) {
        case SIDE_MENU_EFFECT_LINEAR:
            prv->lt = DT;
            prv->lv = DV;
            break;
        case SIDE_MENU_EFFECT_SLOWIN:
        case SIDE_MENU_EFFECT_SLOWOUT:
            prv->lt = DT - ST;
            prv->lv = DV - SV;
            break;
        case SIDE_MENU_EFFECT_SLOWINOUT:
            prv->lt = DT - 2.0 * ST;
            prv->lv = DV - 2.0 * SV;
            break;
        case SIDE_MENU_EFFECT_LOGARITHM:
            prv->lt = prv->lv = 0.0;
            break;
        default:
            break;
    }
    prv->a = (((prv->lv / prv->lt) * ST) - SV) / (ST * ST);
    prv->bi = (SV - (prv->a * ST * ST)) / ST;
    prv->bo = prv->lv / prv->lt;
    effect->prv = prv;

    return effect;
}

typedef struct SideMenu_ *SideMenu;
typedef struct SideMenuFrame_ *SideMenuFrame;

struct SideMenu_ {
    /// super class
    struct SvObject_ super_;

    SvWidget window;
    bool leftSide;
    bool setWidth;
    int padding;
    double moveDuration;
    int inactiveWidth;
    unsigned int fullWidthCount;
    unsigned int settingsCtx;

    unsigned int currentLevel, targetLevel;

    SideMenuFrame *levels;
    size_t levelsCnt, levelsMax;

    SvEffectId effectID;
    SvWidget *toEffect;
    int *dst_x, *dst_w;
    int activeWidth;
};

SvLocal void
SideMenuNotify(SideMenu self);


// SideMenuFrame subwidget

struct SideMenuFrame_ {
    SideMenu sideMenu;

    bool    leftSide;

    SvWidget frame;
    SvWidget contentFrame;
    SvWidget contentPane;
    SvWidget border;
    SvWidget background;

    bool shown;
};

SvLocal void
SideMenuEffectEventHandler(SvWidget sideMenu,
                                SvEffectEvent event)
{
    SideMenu self = sideMenu->prv;

    if (event->id == self->effectID) {
        self->effectID = 0;
        self->currentLevel = self->targetLevel;
        SideMenuNotify(self);
        return;
    }
}

SvLocal SvWidget
SideMenuFrameNew(SvApplication app,
                 SideMenu sideMenu,
                 const char *widgetName,
                 bool leftSide, int level)
{
    const size_t bufferSize = strlen(widgetName) + 8;
    char nameBuffer[bufferSize];
    char fullWidgetName[bufferSize];
    SideMenuFrame self;
    if (!(self = calloc(1, sizeof(struct SideMenuFrame_))))
        goto err;

    self->frame = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    self->frame->prv = self;
    self->sideMenu = sideMenu;
    self->leftSide = leftSide;
    self->shown = false;

    snprintf(fullWidgetName, bufferSize, "%s%d", widgetName, level);
    if (!svSettingsIsWidgetDefined(fullWidgetName))
        snprintf(fullWidgetName, bufferSize, "%s", widgetName);

    if (!(self->contentFrame = svSettingsWidgetCreate(app, fullWidgetName)))
        goto err;

    snprintf(nameBuffer, bufferSize, "%s.bg", fullWidgetName);

    if (svSettingsIsWidgetDefined(nameBuffer)) {
        self->background = QBFrameCreateFromSM(app, nameBuffer);
        if (!(self->contentPane = svWidgetCreateBitmap(app, self->contentFrame->width, self->contentFrame->height, NULL)))
            goto err;
    } else {
        snprintf(nameBuffer, bufferSize, "%s.border", fullWidgetName);
        if (svSettingsIsWidgetDefined(nameBuffer)) {
            self->border = svSettingsWidgetCreate(app, nameBuffer);
        }
        if (!(self->contentPane = svWidgetCreateBitmap(app, svSettingsGetInteger(fullWidgetName, "width", 320), svSettingsGetInteger(fullWidgetName, "height", 510), NULL)))
            goto err;
    }

    if (leftSide) {
        svWidgetAttach(self->contentFrame, self->contentPane, 0, 0, 1);
        svWidgetAttach(self->frame, self->contentFrame, -self->contentPane->width, 0, 1);
    } else {
        svWidgetAttach(self->contentFrame, self->contentPane, 0, 0, 1);
        svWidgetAttach(self->frame, self->contentFrame, self->frame->width, 0, 1);
    }

    if (self->background) {
        svSettingsWidgetAttach(self->contentPane, self->background, nameBuffer, 0);
    }

    if (self->border) {
        svSettingsWidgetAttach(self->contentPane, self->border, nameBuffer, 0);
    }

    svWidgetSetName(self->frame, fullWidgetName);
    return self->frame;

err:
    if (self)
        svWidgetDestroy(self->frame);
    return NULL;
}

// actual SideMenu widget

SvLocal void
SideMenuNotify(SideMenu self)
{
    QBSideMenuLevelChangedEvent event = (QBSideMenuLevelChangedEvent) SvTypeAllocateInstance(QBSideMenuLevelChangedEvent_getType(), NULL);
    if (event) {
        event->currentLevel = self->currentLevel;

        QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
        QBEventBusPostEvent(eventBus, (QBPeerEvent) event, (SvObject) self, NULL);
        SVRELEASE(event);
    }
}

SvLocal void
SideMenuDestroy(void *self_)
{
    SideMenu self = self_;

    free(self->levels);
    free(self->toEffect);
    free(self->dst_x);
    free(self->dst_w);
}

SvLocal void
SideMenuDescribe(void *self_, void *outputStream_)
{
    SideMenu self = self_;
    QBOutputStream outputStream = outputStream_;

    if (self->window) {
        QBOutputStreamWriteFormatted(outputStream, "<SideMenu@%p name:%s>", self, svWidgetGetName(self->window));
    } else {
        QBOutputStreamWriteFormatted(outputStream, "<SideMenu@%p>", self);
    }
}

SvLocal SvType
SideMenu_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy  = SideMenuDestroy,
        .describe = SideMenuDescribe
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("SideMenu",
                            sizeof(struct SideMenu_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
SideMenuCleanup(SvApplication app,
                void *self_)
{
    SideMenu self = self_;

    self->window = NULL;
    SVRELEASE(self);
}

SvWidget
SideMenuNew(SvApplication app,
            const char *widgetName)
{
    SideMenu self = NULL;

    self = (SideMenu) SvTypeAllocateInstance(SideMenu_getType(), NULL);
    if (!self)
        return NULL;

    self->window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    self->window->prv = self;
    self->window->clean = SideMenuCleanup;
    svWidgetSetEffectEventHandler(self->window, SideMenuEffectEventHandler);
    self->leftSide = svSettingsGetBoolean(widgetName, "leftSide", false);
    self->padding = svSettingsGetInteger(widgetName, "padding", 0);
    self->inactiveWidth = svSettingsGetInteger(widgetName, "inactiveWidth", -1);
    self->fullWidthCount = svSettingsGetInteger(widgetName, "fullWidthCount", -1);
    self->moveDuration = svSettingsGetDouble(widgetName, "moveDuration", 0.0);

    self->settingsCtx = svSettingsSaveContext();

    self->currentLevel = self->targetLevel = 0;
    self->levels = NULL;
    self->levelsCnt = self->levelsMax = 0;
    self->effectID = 0;
    self->setWidth = false;

    svWidgetSetName(self->window, widgetName);
    return self->window;
}

SvWidget
SideMenuGetFrame(SvWidget sideMenu,
                 unsigned int level)
{
    unsigned int i;
    SideMenu self;

    if (!sideMenu || level == 0)
        return NULL;

    self = sideMenu->prv;
    if (self->levelsMax <= level) {
        size_t newCnt = level + 7;
        void *tmp = realloc(self->levels, newCnt * sizeof(SideMenuFrame));
        if (!tmp)
            return NULL;
        self->levels = tmp;
        tmp = realloc(self->toEffect, newCnt * sizeof(SvWidget));
        if (!tmp)
            return NULL;
        self->toEffect = tmp;
        tmp = realloc(self->dst_x, newCnt * sizeof(int));
        if (!tmp)
            return NULL;
        self->dst_x = tmp;
        tmp = realloc(self->dst_w, newCnt * sizeof(int));
        if (!tmp)
            return NULL;
        self->dst_w = tmp;
        self->levelsMax = newCnt;
    }

    if (level - 1 >= self->levelsCnt) {
        const size_t nameBufferSize = strlen(svWidgetGetName(sideMenu)) + 7;
        char nameBuffer[nameBufferSize];
        snprintf(nameBuffer, nameBufferSize, "%s.frame", svWidgetGetName(sideMenu));
        svSettingsRestoreContext(self->settingsCtx);

        for (i = self->levelsCnt; i < level; i++) {
            SvWidget frame = SideMenuFrameNew(sideMenu->app, self, nameBuffer, self->leftSide, level);
            if (!frame) {
                svSettingsPopComponent();
                return NULL;
            }
            self->levels[i] = frame->prv;
            self->levelsCnt = i + 1;
            if (self->leftSide)
                svWidgetAttach(self->window, frame, +self->padding, 0, i + 1);
            else {
                svWidgetAttach(self->window, frame, -self->padding, 0, i + 1);
            }
            if (i != 0) {
                svWidgetDetach(self->levels[i - 1]->frame);
                if (self->leftSide) {
                    svWidgetAttach(self->window, self->levels[i - 1]->frame,
                                   +self->padding, 0, 1);
                } else {
                    svWidgetAttach(self->window, self->levels[i - 1]->frame,
                                   -self->padding, 0, 1);
                }
            }
        }
        svSettingsPopComponent();
    }

    return self->levels[level - 1]->contentPane;
}

void
SideMenuShow(SvWidget sideMenu,
             unsigned int level,
             bool immediately)
{
    unsigned int i, j;
    SvWidget frame;
    SideMenu self;

    if (!sideMenu)
        return;

    self = sideMenu->prv;
    self->targetLevel = level;

    if (self->effectID) {
        svAppCancelEffect(sideMenu->app, self->effectID, SV_EFFECT_ABANDON);
        self->effectID = 0;
    }

    frame = SideMenuGetFrame(sideMenu, level);
    if (self->setWidth && frame && self->activeWidth == frame->width) {
        self->setWidth = false;
    } else {
        self->activeWidth = 0;
        if (frame)
            self->activeWidth = frame->width;
    }
    int count = level;
    int fullWidth = 0;
    unsigned int firstFull = level - self->fullWidthCount;
    if (self->leftSide) {
        for (i = 0; i < level; i++) {
            if ((frame = SideMenuGetFrame(sideMenu, i + 1))) {
                int idx = self->levelsCnt - i - 1;
                fullWidth = 0;
                for (j = i + 1; j < level - 1; ++j) {
                    if ((frame = SideMenuGetFrame(sideMenu, j + 1)))
                        fullWidth += self->levels[j]->contentPane->width;
                }
                self->toEffect[idx] = self->levels[i]->contentFrame;
                self->dst_x[idx] = self->activeWidth + fullWidth;
                self->dst_w[idx] = self->levels[i]->contentPane->width;
            }
        }
        if (level > 0) {
            self->dst_x[self->levelsCnt - level] = 0;
            self->dst_w[self->levelsCnt - level] = self->activeWidth;
        }
        if (level > self->fullWidthCount && self->inactiveWidth != -1) {
            for (i = 0; i < firstFull; i++) {
                int idx = self->levelsCnt - i - 1;
                self->dst_x[idx] = self->dst_x[self->fullWidthCount - 1] + self->dst_w[self->fullWidthCount - 1] + self->inactiveWidth*(firstFull - i - 1);
                self->dst_w[idx] = self->inactiveWidth;
            }
        }
        for (i = level; i < self->levelsCnt; ++i) {
            int idx = self->levelsCnt - i - 1;
            self->toEffect[idx] = self->levels[i]->contentFrame;
            self->dst_x[idx] = -self->levels[i]->contentFrame->width;
            self->dst_w[idx] = self->levels[i]->contentFrame->width;
            count += 1;
        }
    } else {
        for (i = 0; i < level; i++) {
            if ((frame = SideMenuGetFrame(sideMenu, i + 1))) {
                self->toEffect[i] = self->levels[i]->contentFrame;
                fullWidth = 0;
                for (j = i; j < level; ++j) {
                    if ((frame = SideMenuGetFrame(sideMenu, j + 1)))
                        fullWidth += self->levels[j]->contentPane->width;
                }
                self->dst_x[i] = sideMenu->width - fullWidth;
                self->dst_w[i] = self->levels[i]->contentPane->width;
            }
        }
        if (level > self->fullWidthCount && self->inactiveWidth != -1) {
            for (i = 0; i < firstFull; i++) {
                self->dst_x[i] = self->dst_x[firstFull] - self->inactiveWidth*(firstFull - i);
                self->dst_w[i] = self->inactiveWidth;
            }
        }
        if (level > 0) {
            self->dst_w[level - 1] = self->activeWidth;
        }
        for (i = level; i < self->levelsCnt; ++i) {
            self->toEffect[i] = self->levels[i]->contentFrame;
            self->dst_x[i] = sideMenu->width;
            self->dst_w[i] = self->levels[i]->contentFrame->width;
            count += 1;
        }
    }

    if (!immediately) {
        SvEffect effect = SideMenuEffectNew(self->toEffect, self->dst_x, self->dst_w, count, self->moveDuration, SIDE_MENU_EFFECT_LOGARITHM);
        svEffectSetNotificationTarget(effect, sideMenu);
        self->effectID = svAppRegisterEffect(sideMenu->app, effect);
    } else {
        for (i = 0; i < self->levelsCnt ; ++i) {
            self->levels[i]->contentFrame->off_x = self->dst_x[i];
            self->levels[i]->contentFrame->width = self->dst_w[i];
        }
    }
    self->setWidth = false;
}

int
SideMenuGetLevel(SvWidget sideMenu)
{
    SideMenu self = sideMenu->prv;
    return self->targetLevel;

}

void
SideMenuSetFrameWidth(SvWidget sideMenu,
                      unsigned int level,
                      int width,
                      bool immediately)
{
    SideMenu self;
    SvWidget frame;

    if (!sideMenu)
        return;
    if (level == 0)
        return;

    self = sideMenu->prv;

    //call SideMenuGetFrame for it's side effect (frames creation)
    frame = SideMenuGetFrame(sideMenu, level);
    if (!frame)
        return;

    int levelIndex = level - 1;
    int oldWidth = self->levels[levelIndex]->contentPane->width;
    int widthChange = width - oldWidth;

    self->levels[levelIndex]->contentPane->width = width;
    if (levelIndex == 0 && self->levels[levelIndex]->background)
        QBFrameSetWidth(self->levels[levelIndex]->background, self->levels[levelIndex]->contentPane->width);
    if ((level == self->currentLevel || level == self->targetLevel) && widthChange) {
        self->setWidth = true;
        self->activeWidth = width;
        if (self->effectID) {
            svAppCancelEffect(sideMenu->app, self->effectID, SV_EFFECT_ABANDON);
            self->effectID = 0;
        }

        int fullWidth = 0;
        unsigned int i;
        if (self->leftSide) {
            for (i = 0; i < level; i++) {
                int idx = self->levelsCnt - i - 1;
                self->toEffect[idx] = self->levels[i]->contentFrame;
                if (self->inactiveWidth != -1) {
                    self->dst_x[idx] = self->activeWidth + self->inactiveWidth*(level - i - 1 - 1);
                    self->dst_w[idx] = self->inactiveWidth;
                } else {
                    self->dst_x[idx] = self->activeWidth + fullWidth;
                    self->dst_w[idx] = self->levels[i]->contentPane->width;
                }
            }
            if (level > 0) {
                self->dst_x[self->levelsCnt - level] = 0;
                self->dst_w[self->levelsCnt - level] = self->levels[self->levelsCnt - level]->contentPane->width;
            }
            fullWidth += self->activeWidth;
        } else {
            for (i = 0; i < self->targetLevel; ++i) {
                self->toEffect[i] = self->levels[i]->contentFrame;
                self->dst_x[i] = sideMenu->width - width - self->inactiveWidth*(level - i -1);
                self->dst_w[i] = self->inactiveWidth;
            }
            if (level > 0)
                self->dst_w[level - 1] = self->levels[level - 1]->contentFrame->width;
        }

        if (!immediately) {
            SvEffect effect = SideMenuEffectNew(self->toEffect, self->dst_x, self->dst_w, level, 2, SIDE_MENU_EFFECT_LOGARITHM);
            svEffectSetNotificationTarget(effect, sideMenu);
            self->effectID = svAppRegisterEffect(sideMenu->app, effect);
        } else {
            for (i = 0; i < self->targetLevel; ++i) {
                self->levels[i]->contentFrame->off_x = self->dst_x[i];
                self->levels[i]->contentFrame->width = self->dst_w[i];
            }
        }
    }
}

int
SideMenuGetFrameWidth(SvWidget sideMenu,
                      unsigned int level)
{
    SideMenu self;
    SvWidget frame;

    if(!sideMenu)
        return 0;
    if(level == 0)
        return 0;

    self = sideMenu->prv;

    frame = SideMenuGetFrame(sideMenu, level);
    if(!frame)
        return 0;

    return self->levels[level - 1]->contentPane->width;
}

