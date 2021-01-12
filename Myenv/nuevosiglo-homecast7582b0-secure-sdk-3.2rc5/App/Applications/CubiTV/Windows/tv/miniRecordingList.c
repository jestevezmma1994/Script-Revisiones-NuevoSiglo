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

#include "miniRecordingList.h"

#include <libintl.h>
#include <Services/pvrAgent.h>
#include <Services/diskPVRProvider/QBDiskPVRProvider.h>
#include <QBApplicationController.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/fade.h>
#include <QBConf.h>
#include <Menus/QBChannelMenu.h>
#include <Windows/newtv.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBScrollBar.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Utils/QBEventUtils.h>
#include <QBTuner.h>
#include <Logic/TVLogic.h>
#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

struct QBMiniRecordingItemController_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvEPGManager epgManager;
    SvBitmap iconTuner1, iconTuner2, iconIP;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
};
typedef struct QBMiniRecordingItemController_t* QBMiniRecordingItemController;

SvLocal void QBMiniRecordingItemController__dtor__(void *ptr)
{
    QBMiniRecordingItemController self = ptr;
    SVRELEASE(self->iconTuner1);
    SVRELEASE(self->iconTuner2);
    SVRELEASE(self->iconIP);
    SVRELEASE(self->focus);
    SVRELEASE(self->inactiveFocus);
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVTESTRELEASE(self->epgManager);
}

SvLocal int QBMiniRecordingItemControllerGetTuner(QBMiniRecordingItemController self, QBPVRRecording rec)
{
    if (QBPVRProviderGetCapabilities(self->appGlobals->pvrProvider)->type != QBPVRProviderType_disk) {
        return -1;
    }

    QBPVRRecording firstRec = (QBPVRRecording) QBActiveArrayObjectAtIndex(QBPVRAgentGetOnGoingRecordingsList(self->appGlobals->pvrAgent), 0);
    if (!firstRec)
        return -1;

    const struct QBTunerMuxId *firstMuxId = QBDiskPVRProviderGetRecordingsMuxId((QBDiskPVRProvider) self->appGlobals->pvrProvider, firstRec);
    const struct QBTunerMuxId *recMuxId = QBDiskPVRProviderGetRecordingsMuxId((QBDiskPVRProvider) self->appGlobals->pvrProvider, rec);
    if (!firstMuxId || !recMuxId)
        return -1;

    return QBTunerMuxIdEqual(recMuxId, firstMuxId);
}

SvLocal SvWidget QBMiniRecordingItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBMiniRecordingItemController self = (QBMiniRecordingItemController) self_;

    QBXMBItem item = QBXMBItemCreate();

    QBPVRRecording recording = (QBPVRRecording) node_;
    SvWidget ret = NULL;
    int tuner = QBMiniRecordingItemControllerGetTuner(self, recording);

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVRETAIN(self->inactiveFocus);
    SvValue valId = SvValueCreateWithString(recording->channelId, NULL);
    SvTVChannel channel = SvEPGManagerFindTVChannel(self->epgManager, valId, NULL);
    if(channel) {
        QBServiceRegistry registry = QBServiceRegistryGetInstance();
        QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
        int number = QBPlaylistManagerGetGlobalNumberOfChannel(playlists, channel);
        item->caption = SvStringCreateWithFormat("%03i %s", number, SvStringCString(channel->name));

        if (SvObjectIsInstanceOf((SvObject) channel, DVBEPGChannel_getType())) {
            item->icon = SVRETAIN(tuner > 0 ? self->iconTuner2 : self->iconTuner1);
        } else {
            item->icon = SVRETAIN(self->iconIP);
        }
    } else {
        SvString channelName = recording->channelName;
        if (channelName)
            item->caption = SVRETAIN(channelName);
        else
            item->caption = SvStringCreate(gettext("000 unknown"), NULL);
    }
    SvEPGIterator eit = SvEPGManagerCreateIterator(self->epgManager, valId, SvTimeNow());
    SVRELEASE(valId);
    if(eit) {
        SvEPGEvent current = SvEPGIteratorGetNextEvent(eit);
        SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, current);
        if(desc && desc->title) {
            item->subcaption = SVRETAIN(desc->title);
        }
    }
    SVTESTRELEASE(eit);

    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void QBMiniRecordingItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBMiniRecordingItemController self = (QBMiniRecordingItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal SvType QBMiniRecordingItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMiniRecordingItemController__dtor__
    };
    static SvType type = NULL;

    static const struct XMBItemController_t controllerMethods = {
        .createItem = QBMiniRecordingItemControllerCreateItem,
        .setItemState = QBMiniRecordingItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiniRecordingItemController",
                            sizeof(struct QBMiniRecordingItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

struct QBMiniRecordingList_t {
    struct QBLocalWindow_t super_;
    SvWidget xmbMenu;
    SvWidget title;
    AppGlobals appGlobals;

    SvEffectId fadeId;
    double fadeTime;
    bool visible;
};

SvLocal void
QBMiniRecordingList__dtor__(void *self_)
{
    QBMiniRecordingList self = self_;
    svWidgetDestroy(self->super_.window);
}

SvLocal SvType QBMiniRecordingList_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBMiniRecordingList__dtor__
        }
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiniRecordingList",
                            sizeof(struct QBMiniRecordingList_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }

    return type;
}

SvLocal bool QBMiniRecordingListInputEventHandler(SvWidget w, SvInputEvent ie)
{
    switch(ie->ch){
        case QBKEY_UP:
        case QBKEY_PGUP:
        case QBKEY_DOWN:
        case QBKEY_PGDN:
        case QBKEY_CHUP:
        case QBKEY_CHDN:
            break;
        default:
            return false;
    }
    return true;
}

SvLocal void
QBMiniRecordingListOnChangePosition(void *w_, const XMBMenuEvent data)
{
    QBMiniRecordingList self = ((SvWidget) w_)->prv;

    if (data->clicked){
        QBPVRRecording recording = (QBPVRRecording) QBActiveArrayObjectAtIndex(QBPVRAgentGetOnGoingRecordingsList(self->appGlobals->pvrAgent), data->position);
        QBTVLogicShowRecordingOptions(self->appGlobals->tvLogic, recording);
    }
}

SvLocal void
QBMiniRecordingListClean(SvApplication app, void *prv){

}

QBMiniRecordingList QBMiniRecordingListNew(AppGlobals appGlobals){
    SvApplication app = appGlobals->res;
    QBMiniRecordingList self = (QBMiniRecordingList) SvTypeAllocateInstance(QBMiniRecordingList_getType(), NULL);
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeFocusable);
    window->prv = self;
    svWidgetSetInputEventHandler(window, QBMiniRecordingListInputEventHandler);
    window->clean = QBMiniRecordingListClean;

    self->appGlobals = appGlobals;
    svSettingsPushComponent("tvMiniRecordingList.settings");

    self->fadeTime = svSettingsGetDouble("localWindow", "fadeTime", 0.3);
    SvWidget bg = QBFrameCreateFromSM(app, "verticalBar.bg");
    SvWidget scrollBar = QBScrollBarNew(app, "ScrollBar");
    svSettingsWidgetAttach(bg, scrollBar, svWidgetGetName(scrollBar), 2);

    self->xmbMenu = XMBVerticalMenuNew(app, "verticalBar", 0);
    svSettingsWidgetAttach(window, self->xmbMenu, svWidgetGetName(self->xmbMenu), 10);

    self->title = QBAsyncLabelNew(app, "title", appGlobals->textRenderer);
    svSettingsWidgetAttach(self->xmbMenu, self->title, svWidgetGetName(self->title), 20);

    XMBVerticalMenuAddRangeListener(self->xmbMenu, QBScrollBarGetRangeListener(scrollBar), NULL);
    XMBVerticalMenuSetBG(self->xmbMenu, bg);

    XMBVerticalMenuSetNotificationTarget(self->xmbMenu, window, QBMiniRecordingListOnChangePosition);

    svWidgetSetFocusable(self->xmbMenu, true);
    svWidgetSetFocus(self->xmbMenu);

    svSettingsPopComponent();

    return self;
}

void QBMiniRecordingListShow(QBMiniRecordingList self){

    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);

    svSettingsPushComponent("tvMiniRecordingList.settings");

    QBMiniRecordingItemController itemController = (QBMiniRecordingItemController)
            SvTypeAllocateInstance(QBMiniRecordingItemController_getType(), NULL);
    AppGlobals appGlobals = self->appGlobals;
    itemController->appGlobals = appGlobals;
    itemController->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    itemController->iconTuner1 = SVRETAIN(svSettingsGetBitmap("MenuItem", "iconTuner1"));
    itemController->iconTuner2 = SVRETAIN(svSettingsGetBitmap("MenuItem", "iconTuner2"));
    itemController->iconIP = SVRETAIN(svSettingsGetBitmap("MenuItem", "iconIP"));
    itemController->focus = QBFrameConstructorFromSM("MenuItem.focus");
    itemController->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    itemController->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);

    XMBVerticalMenuConnectToDataSource(self->xmbMenu,
            (SvGenericObject) QBPVRAgentGetOnGoingRecordingsList(self->appGlobals->pvrAgent),
            (SvGenericObject) itemController, NULL);
    SVRELEASE(itemController);
    svSettingsPopComponent();
    self->visible = true;
    if(self->fadeId)
        svAppCancelEffect(self->appGlobals->res, self->fadeId, SV_EFFECT_ABANDON);
    self->super_.window->tree_alpha = ALPHA_TRANSPARENT;
    SvEffect effect = svEffectFadeNew(self->super_.window, ALPHA_SOLID, 1, self->fadeTime, SV_EFFECT_FADE_LOGARITHM);
    self->fadeId = svAppRegisterEffect(self->appGlobals->res, effect);
}

void QBMiniRecordingListHide(QBMiniRecordingList self){
    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow)self);
    self->visible = false;
}

bool QBMiniRecordingListIsVisible(QBMiniRecordingList self){
    return self->visible;
}
