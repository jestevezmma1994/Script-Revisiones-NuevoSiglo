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

#include "miniChannelList.h"
#include "miniChannelListInternal.h"

#include <QBApplicationController.h>
#include <SWL/fade.h>
#include <QBInput/QBInputCodes.h>
#include <main.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <QBConf.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <Menus/QBChannelMenu.h>
#include <Windows/newtv.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBScrollBar.h>
#include <Widgets/extendedInfo.h>
#include <Logic/GUILogic.h>
#include <XMB2/XMBVerticalMenu.h>
#include <QBAppKit/QBServiceRegistry.h>

SvLocal void
QBMiniChannelListDestroy(void *self_)
{
    QBMiniChannelList self = self_;
    SVTESTRELEASE(self->tvChannels);
    svWidgetDestroy(self->super_.window);
}

SvLocal SvType QBMiniChannelList_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBMiniChannelListDestroy
        }
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiniChannelList",
                            sizeof(struct QBMiniChannelList_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

void QBMiniChannelListExtendedInfoDestroy(QBMiniChannelList self)
{
    if (self->extendedInfo) {
        svWidgetDetach(self->extendedInfo);
        svWidgetDestroy(self->extendedInfo);
        self->extendedInfo = NULL;
    }
}

SvLocal void QBMiniChannelListExtendedInfoCreateAndAttach(QBMiniChannelList self)
{
    QBMiniChannelListExtendedInfoDestroy(self);

    SvApplication app = self->appGlobals->res;
    svSettingsPushComponent("tvMiniChannelListExtendedInfo.settings");
    SvWidget extendedInfo = QBExtendedInfoNew(app, "ExtendedInfo", self->appGlobals);
    self->extendedInfo = extendedInfo;

    SvWidget bg = QBFrameCreateFromSM(app, "verticalBar.bg");
    svWidgetAttach(self->extendedInfo, bg, 0, 0, 0);

    SvWidget w = self->super_.window;
    svSettingsWidgetAttach(w, self->extendedInfo, svWidgetGetName(self->extendedInfo), 10);

    svSettingsPopComponent();
}

void QBMiniChannelListExtendedInfoUpdate(QBMiniChannelList self, unsigned int position)
{
    QBMiniChannelListExtendedInfoCreateAndAttach(self);

    SvTVChannel channel = SvInvokeInterface(SvEPGChannelView, self->tvChannels, getByIndex, position);
    if (!channel)
        return;

    SvEPGEvent event = NULL;
    SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                         SVSTRING("SvEPGManager"));
    SvEPGIterator it = SvEPGManagerCreateIterator(epgManager, SvDBObjectGetID((SvDBObject) channel), SvTimeNow());
    if (it) {
        event = SvEPGIteratorGetNextEvent(it);
        SVRELEASE(it);
    }
    if (!event)
        return;

    SvArray events = SvArrayCreate(NULL );
    SvArrayAddObject(events, (SvGenericObject) event);
    QBExtendedInfoSetEvents(self->extendedInfo, events);
    SVRELEASE(events);
}

SvLocal void
QBMiniChannelListOnChangePosition(void *w_, const XMBMenuEvent data)
{
    QBMiniChannelList self = ((SvWidget) w_)->prv;

    if (data->clicked){
        if (QBTVLogicChannelCanPlayByIndex(self->appGlobals->tvLogic, data->position)) {
            QBTVLogicPlayChannelByIndex(self->appGlobals->tvLogic, data->position, SVSTRING("MiniChannelList"));
        }
    } else if (self->extendedInfo && self->position != data->position) {
        QBMiniChannelListExtendedInfoUpdate(self, data->position);
        self->position = data->position;
    }
}

SvLocal bool QBMiniChannelListInputEventHandler(SvWidget w, SvInputEvent ie)
{
    QBMiniChannelList self = w->prv;

    if (self->callbacks.inputEventHandler && self->callbackData) {
        if (self->callbacks.inputEventHandler(self->callbackData, w, ie))
            return true;
    }

    if (ie->type == QBInputEventType_keyPressed) {
        if (ie->fullInfo.u.key.code == QBKEY_LEFT || ie->fullInfo.u.key.code == QBKEY_RIGHT) {
            if (self->extendedInfo) {
                QBExtendedInfoStartScrollingVertically(self->extendedInfo, ie->fullInfo.u.key.code == QBKEY_RIGHT);
                return true;
            }
        }
    } else if (ie->type == QBInputEventType_keyReleased) {
        if (ie->fullInfo.u.key.code == QBKEY_LEFT || ie->fullInfo.u.key.code == QBKEY_RIGHT) {
            if (self->extendedInfo) {
                QBExtendedInfoStopScrollingVertically(self->extendedInfo);
                return true;
            }
        }
    } else if (ie->type == QBInputEventType_keyTyped) {
        switch (ie->fullInfo.u.key.code) {
            case QBKEY_LEFT:
            case QBKEY_RIGHT:
                if (self->extendedInfo)
                    return true;
                break;
            case QBKEY_INFO:
                if(!self->extendedInfo) {
                    int position = XMBVerticalMenuGetPosition(self->xmbMenu);
                    QBMiniChannelListExtendedInfoUpdate(self, position);
                } else {
                    QBMiniChannelListExtendedInfoDestroy(self);
                }
                return true;
        }
    }

    return false;
}

SvLocal void
QBMiniChannelListClean(SvApplication app, void *prv){

}

void QBMiniChannelListSelectChannel(QBMiniChannelList self_, SvTVChannel channel){
    QBMiniChannelList self = (QBMiniChannelList) self_;
    int idx = SvInvokeInterface(SvEPGChannelView, self->tvChannels, getChannelIndex, channel);
    XMBVerticalMenuSetPosition(self->xmbMenu, idx, 1, NULL);
}

QBMiniChannelList QBMiniChannelListNew(AppGlobals appGlobals){
    SvApplication app = appGlobals->res;
    QBMiniChannelList self = (QBMiniChannelList) SvTypeAllocateInstance(QBMiniChannelList_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeFocusable);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBMiniChannelListInputEventHandler);
    window->clean = QBMiniChannelListClean;

    self->appGlobals = appGlobals;
    svSettingsPushComponent("tvMiniChannelList.settings");

    self->fadeTime = svSettingsGetDouble("localWindow", "fadeTime", 0.3);
    SvWidget bg = NULL;
    if (svSettingsIsWidgetDefined("verticalBar.bg"))
        bg = QBFrameCreateFromSM(app, "verticalBar.bg");

    SvWidget scrollBar = NULL;
    if (svSettingsIsWidgetDefined("ScrollBar")) {
        scrollBar = QBScrollBarNew(app, "ScrollBar");
        svSettingsWidgetAttach(bg, scrollBar, svWidgetGetName(scrollBar), 2);
    }
    self->xmbMenu = XMBVerticalMenuNew(app, "verticalBar", 0);

    svSettingsWidgetAttach(window, self->xmbMenu, svWidgetGetName(self->xmbMenu), 10);

    if (scrollBar)
        XMBVerticalMenuAddRangeListener(self->xmbMenu, QBScrollBarGetRangeListener(scrollBar), NULL);

    if (bg)
        XMBVerticalMenuSetBG(self->xmbMenu, bg);

    XMBVerticalMenuSetNotificationTarget(self->xmbMenu, window, QBMiniChannelListOnChangePosition);

    svWidgetSetFocusable(self->xmbMenu, true);
    svWidgetSetFocus(self->xmbMenu);



    svSettingsPopComponent();

    QBGUILogicMiniChannelListRegisterCallbacks(self->appGlobals->guiLogic, (SvGenericObject) self);

    return self;
}

void QBMiniChannelListShow(QBMiniChannelList self)
{
    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvGenericObject current = QBPlaylistManagerGetCurrent(playlists);

    svSettingsPushComponent("tvMiniChannelList.settings");
    SVTESTRELEASE(self->tvChannels);
    self->tvChannels = SVTESTRETAIN(current);
    QBChannelController channelController = QBChannelMenuControllerCreate(true, self->appGlobals, self->tvChannels);
    QBGUILogicChannelMenuControllerRegisterCallbacks(self->appGlobals->guiLogic, (SvGenericObject) channelController);
    if(self->tvChannels)
        XMBVerticalMenuConnectToDataSource(self->xmbMenu, self->tvChannels, (SvGenericObject)channelController, NULL);
    SVRELEASE(channelController);
    svSettingsPopComponent();
    self->visible = true;
    if(self->fadeId)
        svAppCancelEffect(self->appGlobals->res, self->fadeId, SV_EFFECT_ABANDON);
    self->super_.window->tree_alpha = ALPHA_TRANSPARENT;
    SvEffect effect = svEffectFadeNew(self->super_.window, ALPHA_SOLID, 1, self->fadeTime, SV_EFFECT_FADE_LOGARITHM);
    self->fadeId = svAppRegisterEffect(self->appGlobals->res, effect);
}

void QBMiniChannelListHide(QBMiniChannelList self)
{
    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow)self);
    self->visible = false;
    QBMiniChannelListExtendedInfoDestroy(self);
}

bool QBMiniChannelListIsVisible(QBMiniChannelList self)
{
    return self->visible;
}

void QBMiniChannelListReinitialize(QBMiniChannelList self, SvArray itemList) {
    if (!self->tvChannels)
        return;

    if (itemList && SvArrayContainsObject(itemList, (SvGenericObject) SVSTRING("QBParentalControl"))) {
        int cnt = SvInvokeInterface(QBListModel, self->tvChannels, getLength);
        SvInvokeInterface(QBListModelListener, self->xmbMenu->prv, itemsChanged, 0, cnt);
    }
}

void
QBMiniChannelListSetCallbacks(SvGenericObject self_, void *callbackData, QBMiniChannelListCallbacks callbacks)
{
    QBMiniChannelList self = (QBMiniChannelList)self_;

    self->callbackData = callbackData;
    self->callbacks = *callbacks;
}
