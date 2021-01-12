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

#include "QBAnimatedBackground.h"
#include <stdbool.h>
#include <stdio.h>
#include <SvCore/SvCommonDefs.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/app.h>
#include <settings.h>
#include <Logic/InitLogic.h>


struct QBAnimatedBackground_t {
    SvWidget bg;
    QBAnimatedBackgroundType type;

    SvWidget logo;
    SvTimerId logoTimer;
    double logoStepTime;

    QBInitLogic initLogic;
};
typedef struct QBAnimatedBackground_t QBAnimatedBackground;

SvLocal void QBAnimatedBackgroundTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBAnimatedBackground* self = w->prv;
    if (e->id == self->logoTimer) {
        svWidgetForceRender(w);
        self->logo->off_x = rand() % (self->bg->width - self->logo->width + 1);
        self->logo->off_y = rand() % (self->bg->height - self->logo->height + 1);
        return;
    }
}

SvLocal void QBAnimatedBackgroundClean(SvApplication app, void* self_)
{
    QBAnimatedBackground* self = self_;
    if (self->logoTimer) {
        svAppTimerStop(app, self->logoTimer);
    }
    SVRELEASE(self->initLogic);
    free(self);
}

SvWidget QBAnimatedBackgroundCreate(SvApplication app, const char *widgetName, QBAnimatedBackgroundType type, QBInitLogic initLogic)
{
    QBAnimatedBackground* self = calloc(1, sizeof(QBAnimatedBackground));
    const char *defaultBackroundName = "AnimatedBackground";
    self->type = type;
    self->initLogic = SVRETAIN(initLogic);

    if (type == QBAnimatedBackgroundType_default && svSettingsGetBoolean("AnimatedBackground", "useCustomerLogo", false))
        self->type = QBAnimatedBackgroundType_logo;

    const char *widgetName_ = widgetName ? widgetName : defaultBackroundName;

    if (svSettingsIsWidgetDefined(widgetName_))
        self->bg = svSettingsWidgetCreate(app, widgetName_);
    else
        self->bg = svSettingsWidgetCreate(app, defaultBackroundName);

    self->bg->clean = QBAnimatedBackgroundClean;
    char *logoName;
    asprintf(&logoName, "%s.logo", svWidgetGetName(self->bg));
    SvString customerlogoName = QBInitLogicGetLogoWidgetName(self->initLogic);

    switch (self->type) {
        case QBAnimatedBackgroundType_logo:
            self->logo = svSettingsWidgetCreate(app, SvStringCString(customerlogoName));
            break;
        case QBAnimatedBackgroundType_default:
        default: {
            self->logo = svSettingsWidgetCreate(app, logoName);
            break;
        }
    }

    self->logoStepTime = svSettingsGetDouble(logoName, "stepTime", 0.2);
    svWidgetAttach(self->bg, self->logo, (self->bg->width - self->logo->width) / 2, (self->bg->height - self->logo->height) / 2, 1);
    free(logoName);

    self->bg->prv = self;
    svWidgetSetTimerEventHandler(self->bg, QBAnimatedBackgroundTimerEventHandler);
    svWidgetSetHidden(self->bg, true);

    return self->bg;
}

void QBAnimatedBackgroundReinitialize(SvWidget w, SvArray itemList)
{
    QBAnimatedBackground* self = w->prv;
    if (self->type != QBAnimatedBackgroundType_logo)
        return;

    SvString logoWidgetName = QBInitLogicGetLogoWidgetName(self->initLogic);
    if (!logoWidgetName)
        return;

    int off_x = self->logo->off_x;
    int off_y = self->logo->off_y;

    svWidgetDetach(self->logo);
    svWidgetDestroy(self->logo);

    const char *widgetName = SvStringCString(logoWidgetName);
    svSettingsPushComponent("AnimatedBackground.settings");
    self->logo = svSettingsWidgetCreate(w->app, widgetName);
    svSettingsPopComponent();
    svWidgetAttach(self->bg, self->logo, off_x, off_y, 1);
}

void QBAnimatedBackgroundShow(SvWidget w)
{
    QBAnimatedBackground* self = w->prv;

    svWidgetSetHidden(w, false);
    switch (self->type) {
        case QBAnimatedBackgroundType_default:
        case QBAnimatedBackgroundType_logo:
        default: {
            self->logoTimer = svAppTimerStart(w->app, w, self->logoStepTime, 0);
            break;
        }
    }
}

void QBAnimatedBackgroundHide(SvWidget w)
{
    QBAnimatedBackground* self = w->prv;

    svWidgetSetHidden(w, true);
    switch (self->type) {
        case QBAnimatedBackgroundType_default:
        case QBAnimatedBackgroundType_logo:
        default: {
            svAppTimerStop(w->app, self->logoTimer);
            self->logoTimer = 0;
            break;
        }
    }
}

bool QBAnimatedBackgroundIsVisible(SvWidget w)
{
    return !svWidgetIsHidden(w);
}
