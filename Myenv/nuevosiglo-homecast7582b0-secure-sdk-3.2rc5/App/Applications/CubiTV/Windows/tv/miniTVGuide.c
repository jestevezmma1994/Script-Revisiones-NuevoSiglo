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

#include "miniTVGuide.h"

#include <libintl.h>
#include <QBInput/QBInputCodes.h>
#include <QBApplicationController.h>
#include <SWL/fade.h>
#include <settings.h>
#include <Menus/QBChannelMenu.h>
#include <Windows/newtv.h>
#include <Widgets/tvEPGOverlay.h>
#include <Utils/QBEventUtils.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/QBFrame.h>
#include <SvEPG/SvEPGGrid.h>
#include <QBConf.h>
#include <dvbTimeDate.h>
#include <Widgets/QBEPGDisplayer.h>
#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBTimeDateMonitor.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvCore/SvCoreErrorDomain.h>

struct QBMiniTVGuide_t {
    struct QBLocalWindow_t super_;
    AppGlobals appGlobals;

    SvEPGTextRenderer EPGtextRenderer;
    SvEPGView EPGview;
    SvEPGDataWindow EPGwindow;
    SvWidget EPGGrid;

    SvEffectId fadeId;
    double fadeTime;
    bool visible;
};

SvLocal void
QBMiniTVGuideDestroy(void *self_)
{
    QBMiniTVGuide self = self_;
    SVTESTRELEASE(self->EPGtextRenderer);
    SVTESTRELEASE(self->EPGview);
    SVTESTRELEASE(self->EPGwindow);
    svWidgetDestroy(self->super_.window);
}

SvLocal void
QBMiniTVGuideSetDefaultPosition(SvGenericObject self_)
{
    QBMiniTVGuide self = (QBMiniTVGuide)self_;
    SvEPGGridSetActivePosition(self->EPGGrid, SvTimeNow(), NULL);
    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    SvEPGGridSetActiveChannel(self->EPGGrid, SvDBObjectGetID((SvDBObject) channel));
}

SvLocal
void QBMiniTVGuideTimeUpdated(SvGenericObject self_, bool firstTime, QBTimeDateUpdateSource source)
{
  if (firstTime)
      QBMiniTVGuideSetDefaultPosition(self_);
}

SvLocal SvType QBMiniTVGuide_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBMiniTVGuideDestroy
        }
    };
    static SvType type = NULL;

    static const struct QBTimeDateMonitorListener_ timeDateListenerMethods = {
        .systemTimeSet = QBMiniTVGuideTimeUpdated
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiniTVGuide",
                            sizeof(struct QBMiniTVGuide_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            QBTimeDateMonitorListener_getInterface(), &timeDateListenerMethods,
                            NULL);
    }

    return type;
}

SvLocal bool QBMiniTVGuideInputEventHandler(SvWidget w, SvInputEvent ie)
{
    QBMiniTVGuide self = w->prv;
    switch(ie->ch){
        case QBKEY_ENTER:{
            SvTVChannel ch = SvEPGGridGetActiveChannel(self->EPGGrid);
            QBTVLogicPlayChannel(self->appGlobals->tvLogic, ch, SVSTRING("MiniTVGuide"));
            break;
        }
        case QBKEY_UP:
        case QBKEY_DOWN:
        case QBKEY_CHUP:
        case QBKEY_CHDN:
            break;
        default:
            return false;
    }
    return true;
}

SvLocal void
QBMiniTVGuideClean(SvApplication app, void *prv){

}

void QBMiniTVGuideSelectChannel(SvGenericObject self_, SvTVChannel channel){
//    QBMiniTVGuide self = (QBMiniTVGuide) self_;
//    self->tvChannels
}

QBMiniTVGuide QBMiniTVGuideNew(AppGlobals appGlobals){
    SvApplication app = appGlobals->res;
    SvErrorInfo error = NULL;
    QBMiniTVGuide self = (QBMiniTVGuide) SvTypeAllocateInstance(QBMiniTVGuide_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    const char* widgetName = "EPGWidget";
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeFocusable);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBMiniTVGuideInputEventHandler);
    window->clean = QBMiniTVGuideClean;

    self->appGlobals = appGlobals;
    svSettingsPushComponent("tvMiniGuide.settings");

    SvGenericObject displayer = QBEPGDisplayerCreate(self->appGlobals, widgetName);
    SvWidget localWindow = svSettingsWidgetCreate(app, "localWindow");
    svSettingsWidgetAttach(window, localWindow, svWidgetGetName(localWindow), 1);
    SvWidget bg = QBFrameCreateFromSM(app, "localWindow.bg");
    svSettingsWidgetAttach(localWindow, bg, "localWindow.bg", 0);

    self->fadeTime = svSettingsGetDouble("localWindow", "fadeTime", 0.3);

    self->EPGtextRenderer =
        SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGTextRenderer")));
    unsigned int timeScale = svSettingsGetInteger("EPGGrid", "timeScale", 180);
    self->EPGGrid = SvEPGGridNew(app, "EPGGrid", NULL, self->EPGtextRenderer, timeScale, displayer, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "unable to create SvEPGgrid widget");
        svSettingsPopComponent();
        goto out;
    }
    SvEPGGridSetDescFunc(self->EPGGrid, QBEventUtilsGetDescFromEvent_, (void*)self->appGlobals);

    svSettingsWidgetAttach(localWindow, self->EPGGrid, svWidgetGetName(self->EPGGrid), 1);

    SvEPGGridSetActivePosition(self->EPGGrid, SvTimeNow(), NULL);
    svWidgetSetFocusable(self->EPGGrid, true);

    svSettingsPopComponent();

    QBTimeDateMonitor timeDateMonitor = (QBTimeDateMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                        SVSTRING("QBTimeDateMonitor"));
    QBTimeDateMonitorAddListener(timeDateMonitor, (SvObject) self, NULL);
    SVTESTRELEASE(displayer);
    return self;

out:
    SvLogError("%s(): Got error:", __func__);
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    SVTESTRELEASE(self);
    return NULL;
}

void QBMiniTVGuideShow(QBMiniTVGuide self)
{

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject current = QBPlaylistManagerGetCurrent(playlists);
    if(self->fadeId)
        svAppCancelEffect(self->appGlobals->res, self->fadeId, SV_EFFECT_ABANDON);
    self->super_.window->tree_alpha = ALPHA_TRANSPARENT;
    SvEffect effect = svEffectFadeNew(self->super_.window, ALPHA_SOLID, 1, self->fadeTime, SV_EFFECT_FADE_LOGARITHM);
    self->fadeId = svAppRegisterEffect(self->appGlobals->res, effect);

    if(current != (SvGenericObject) self->EPGview) {
        SVTESTRELEASE(self->EPGview);
        self->EPGview = SVTESTRETAIN(current);
        if(self->EPGview) {
           svSettingsPushComponent("tvMiniGuide.settings");
           SvGenericObject overlay = tvEPGOverlayCreate(self->appGlobals, "EPGGrid.StripeHeader", "EPGHeader", "EPGGrid.StripeHeader", (SvGenericObject) self->EPGview);
           svSettingsPopComponent();
           SvEPGGridSetOverlay(self->EPGGrid, overlay, NULL);
           SVRELEASE(overlay);
           if (!self->EPGwindow) {
              SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                   SVSTRING("SvEPGManager"));
              self->EPGwindow = SvEPGManagerCreateDataWindow(epgManager, NULL);
              SvEPGManagerUpdateParams params = SvEPGManagerCreateUpdateParameters(epgManager);
              SvEPGManagerSetUpdatesStrategyForClient(epgManager, (SvObject)self->EPGwindow, params);
              SVRELEASE(params);
           }
           SvEPGDataWindowSetChannelView(self->EPGwindow, (SvGenericObject) self->EPGview, NULL);
           SvEPGGridSetDataWindow(self->EPGGrid, self->EPGwindow, NULL);
       }
    }

    SvEPGGridSetActivePosition(self->EPGGrid, SvTimeNow(), NULL);
    SvTVChannel channel = SvTVContextGetCurrentChannel(self->appGlobals->newTV);
    SvEPGGridSetActiveChannel(self->EPGGrid, SvDBObjectGetID((SvDBObject) channel));
    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);
    self->visible = true;
}

void QBMiniTVGuideHide(QBMiniTVGuide self){
    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow)self);
    self->visible = false;
}

bool QBMiniTVGuideIsVisible(QBMiniTVGuide self){
    return self->visible;
}

