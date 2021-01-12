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

#include "QBChannelMenu.h"
#include "QBChannelMenuInternal.h"

#include <libintl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvType.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <settings.h>
#include <Services/core/playlistManager.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Utils/QBEventUtils.h>
#include <XMB2/XMBItemController.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <main.h>

struct QBXMBChannelItemInfo_t {
    struct QBXMBItemInfo_t super_;
    struct {
        SvWidget w;
        bool rendered;
        int baseLineY, baseLineX;
    } number, name;
    int channelNameSpacing;
    struct {
        SvString name;
        QBTextRenderer renderer;
        SvFont font;
        unsigned int fontSize;
        SvColor textColor;
        unsigned int maxLines;
        SvString textEnd;
        int widthThatLeft;
    } renderingData;
};

typedef struct QBChannelItemConstructor_t *QBChannelItemConstructor;

struct QBChannelItemConstructor_t {
    struct QBXMBItemConstructor_t super_;
    int channelNameSpacing;
};

typedef struct QBXMBChannelItem_t *QBXMBChannelItem;

struct QBXMBChannelItem_t {
    struct QBXMBItem_t super_;
    unsigned int number;
    SvString name;
};

SvLocal void QBXMBChannelItemInfoFillName(void *item_, QBTextRendererTask task);


SvLocal void
QBXMBChannelItemInfoDestroy(void *self_)
{
    QBXMBChannelItemInfo self = self_;

    SVTESTRELEASE(self->renderingData.name);
    SVTESTRELEASE(self->renderingData.font);
    SVTESTRELEASE(self->renderingData.textEnd);
}

SvLocal SvType
QBXMBChannelItemInfo_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXMBChannelItemInfoDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXMBChannelItemInfo",
                            sizeof(struct QBXMBChannelItemInfo_t),
                            QBXMBItemInfo_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}


SvLocal void
QBXMBChannelItemDestroy(void *self_)
{
    QBXMBChannelItem self = self_;

    SVTESTRELEASE(self->name);
}

SvLocal SvType
QBXMBChannelItem_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXMBChannelItemDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXMBChannelItem",
                            sizeof(struct QBXMBChannelItem_t),
                            QBXMBItem_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void
QBXMBChannelItemInfoTextsRendered(QBXMBChannelItemInfo channelInfo)
{
    int height, baseLine;
    int numberDesc, nameDesc, desc;

    channelInfo->number.w->width = channelInfo->number.w->bmp->width;
    channelInfo->number.w->height = channelInfo->number.w->bmp->height;
    channelInfo->name.w->width = channelInfo->name.w->bmp->width;
    channelInfo->name.w->height = channelInfo->name.w->bmp->height;

    channelInfo->name.w->off_x = channelInfo->number.w->width + channelInfo->channelNameSpacing + channelInfo->number.baseLineX;
    channelInfo->number.w->off_x = 0;

    numberDesc = channelInfo->number.w->height - channelInfo->number.baseLineY;
    nameDesc = channelInfo->name.w->height - channelInfo->name.baseLineY;

    desc = (numberDesc >= nameDesc) ? numberDesc: nameDesc;
    if (desc < 0)
        desc = 0;

    baseLine = (channelInfo->number.baseLineY >= channelInfo->name.baseLineY) ? channelInfo->number.baseLineY : channelInfo->name.baseLineY;
    height = baseLine + desc;

    channelInfo->name.w->off_y = baseLine - channelInfo->name.baseLineY;
    channelInfo->number.w->off_y = baseLine - channelInfo->number.baseLineY;

    channelInfo->super_.title.w->width = channelInfo->name.w->width + channelInfo->name.w->off_x;
    channelInfo->super_.title.w->height = height;


    channelInfo->super_.title.baseLineY = baseLine;
    channelInfo->super_.title.baseLineX = channelInfo->number.baseLineX;
    channelInfo->super_.title.rendered = true;

    if (svWidgetIsAttached(channelInfo->super_.title.w))
        channelInfo->super_.positionCallback((QBXMBItemInfo) channelInfo);
}


SvLocal void QBXMBChannelItemInfoFillNumber(void *item_, QBTextRendererTask task)
{
    SvBitmap bmp = task->surface;
    QBXMBChannelItemInfo channelInfo = (QBXMBChannelItemInfo) (item_);

    svWidgetSetBitmap(channelInfo->number.w, bmp);
    channelInfo->number.rendered = true;
    channelInfo->number.baseLineY = task->box.y;
    channelInfo->number.baseLineX = task->box.x;

    QBTextRendererTask channelNameRenderingTask = QBTextRendererTaskNew(channelInfo->renderingData.renderer, channelInfo->renderingData.name, QBXMBChannelItemInfoFillName,
                                                                        channelInfo, channelInfo->renderingData.font, channelInfo->renderingData.fontSize,
                                                                        channelInfo->renderingData.textColor, channelInfo->renderingData.widthThatLeft - bmp->width);
    channelNameRenderingTask->maxLines = channelInfo->renderingData.maxLines;
    channelNameRenderingTask->textEnd = SVTESTRETAIN(channelInfo->renderingData.textEnd);
    SvBitmap nameBitmap = QBTextRendererTaskStart(channelNameRenderingTask);
    svWidgetSetBitmap(channelInfo->name.w, nameBitmap);
    SVRELEASE(channelNameRenderingTask);

    SVTESTRELEASE(channelInfo->renderingData.name);
    channelInfo->renderingData.name = NULL;
    SVTESTRELEASE(channelInfo->renderingData.font);
    channelInfo->renderingData.font = NULL;
    SVTESTRELEASE(channelInfo->renderingData.textEnd);
    channelInfo->renderingData.textEnd = NULL;

    if (channelInfo->number.rendered && channelInfo->name.rendered)
        QBXMBChannelItemInfoTextsRendered(channelInfo);
}


SvLocal void QBXMBChannelItemInfoFillName(void *item_, QBTextRendererTask task)
{
    SvBitmap bmp = task->surface;
    QBXMBChannelItemInfo channelInfo = item_;

    svWidgetSetBitmap(channelInfo->name.w, bmp);
    channelInfo->name.rendered = true;
    channelInfo->name.baseLineY = task->box.y;
    channelInfo->name.baseLineX = task->box.x;

    if (channelInfo->number.rendered && channelInfo->name.rendered)
        QBXMBChannelItemInfoTextsRendered(channelInfo);
}


SvLocal SvWidget
QBChannelItemConstructorCreateCaption(SvObject self_, QBXMBItemInfo itemInfo, QBXMBItem itemData)
{
    QBChannelItemConstructor self = (QBChannelItemConstructor) self_;
    QBXMBChannelItemInfo channelInfo = (QBXMBChannelItemInfo) itemInfo;
    QBXMBChannelItem channelData = (QBXMBChannelItem) itemData;
    QBTextRendererTask channelNoRenderingTask;

    if (!SvObjectIsInstanceOf((SvObject) itemData, QBXMBChannelItem_getType())) {
        long int interfaceID = SvInterfaceGetID(QBXMBItemRenderer_getInterface());
        QBXMBItemRenderer vTable = (QBXMBItemRenderer) SvTypeGetVTable(QBXMBItemConstructor_getType(), interfaceID);
        return (vTable->createCaption) (self_, itemInfo, itemData);
    }

    SvWidget title = svWidgetCreateBitmap(itemInfo->box->app, 0, 0, NULL);
    channelInfo->name.w = svWidgetCreateBitmap(itemInfo->box->app, 0, 0, NULL);
    channelInfo->number.w = svWidgetCreateBitmap(itemInfo->box->app, 0, 0, NULL);

    channelInfo->channelNameSpacing = self->channelNameSpacing;

    svWidgetAttach(title, channelInfo->name.w, 0, 0, 1);
    svWidgetAttach(title, channelInfo->number.w, 0, 0, 1);

    itemInfo->title.rendered = false;
    itemInfo->title.mboxHeight = self->super_.mboxHeight;
    itemInfo->title.w = title;
    SvString numberText = SvStringCreateWithFormat("%03u", channelData->number);

    channelInfo->renderingData.name = SVTESTRETAIN(channelData->name);
    channelInfo->renderingData.renderer = self->super_.renderer;
    channelInfo->renderingData.font = SVTESTRETAIN(self->super_.font);
    channelInfo->renderingData.fontSize = self->super_.fontSize;
    channelInfo->renderingData.textColor = itemInfo->disabled ? self->super_.disabledTextColor : self->super_.textColor;
    channelInfo->renderingData.maxLines = self->super_.titleMultiline ? 0 : 1;
    channelInfo->renderingData.textEnd = self->super_.titleMultiline ? NULL : SVTESTRETAIN(self->super_.textEnd);
    channelInfo->renderingData.widthThatLeft = itemData->focus->data.maxWidth - self->super_.leftPadding - self->super_.rightPadding - 2 * self->super_.spacing
                                               - (itemInfo->icon ? itemInfo->icon->width : 0);

    channelNoRenderingTask = QBTextRendererTaskNew(channelInfo->renderingData.renderer, numberText, QBXMBChannelItemInfoFillNumber,
                                                   itemInfo, channelInfo->renderingData.font, channelInfo->renderingData.fontSize,
                                                   channelInfo->renderingData.textColor, channelInfo->renderingData.widthThatLeft);
    channelNoRenderingTask->maxLines = channelInfo->renderingData.maxLines;
    channelNoRenderingTask->textEnd = SVTESTRETAIN(channelInfo->renderingData.textEnd);

    SvBitmap numberBitmap = QBTextRendererTaskStart(channelNoRenderingTask);
    svWidgetSetBitmap(channelInfo->number.w, numberBitmap);
    SVRELEASE(channelNoRenderingTask);
    SVRELEASE(numberText);

    return title;
}


SvLocal SvType
QBChannelItemConstructor_getType(void)
{
    static SvType type = NULL;
    static const struct QBXMBItemRenderer_t rendererMethods = {
        .createCaption = QBChannelItemConstructorCreateCaption,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelItemConstructor", sizeof(struct QBChannelItemConstructor_t),
                            QBXMBItemConstructor_getType(), &type,
                            QBXMBItemRenderer_getInterface(), &rendererMethods,
                            NULL);
    }

    return type;
}


SvLocal QBChannelItemConstructor
QBChannelItemConstructorCreate(const char *widgetName, QBTextRenderer textRenderer)
{
    QBChannelItemConstructor self = (QBChannelItemConstructor) SvTypeAllocateInstance(QBChannelItemConstructor_getType(), NULL);

    QBXMBItemConstructorInit((QBXMBItemConstructor) self, widgetName, textRenderer);

    self->channelNameSpacing = svSettingsGetInteger(widgetName, "channelNameSpacing", 0);


    return self;
}


SvHidden QBXMBItem QBChannelControllerCreateChannelItemData(QBChannelController self, SvTVChannel channel, SvApplication app)
{
    QBXMBChannelItem itemChannelData = (QBXMBChannelItem) SvTypeAllocateInstance(QBXMBChannelItem_getType(), NULL);
    QBXMBItem itemData = &itemChannelData->super_;
    itemData->loadingRID = itemData->iconRID = SV_RID_INVALID;
    if(channel->logoURL) {
        itemData->loadingRID = self->iconRID;
        //itemData->iconURI.URI = SVRETAIN(SvURIString(channel->logoURL));
        //itemData->iconURI.isStatic = true;
        // AMERELES Logos de Canales en servidor
        itemData->iconURI.URI = SVRETAIN(QBTVLogicGetChannelLogoURL(channel));
        itemData->iconURI.isStatic = false;
        // AMERELES Logos de Canales en servidor
    } else {
        itemData->iconRID = self->iconRID;
    }

    if (self->channelsList)
        itemChannelData->number = QBPlaylistManagerGetNumberOfChannelInPlaylist(self->playlists, (SvGenericObject) self->channelsList, channel);
    else
        itemChannelData->number = -1;
    itemChannelData->name = SVTESTRETAIN(channel->name);
    if(self->showEPG) {
        SvEPGIterator eit = SvEPGManagerCreateIterator(self->manager, SvDBObjectGetID((SvDBObject) channel), SvTimeNow());
        if(eit) {
            SvEPGEvent current = SvEPGIteratorGetNextEvent(eit);
            SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(self->appGlobals->eventsLogic, self->appGlobals->langPreferences, current);
            if(desc && desc->title) {
                itemData->subcaption = SVRETAIN(desc->title);
            }
        }
        SVTESTRELEASE(eit);
    }
    //QBTextRendererAddPersistentText(self->renderer, itemData->caption);
    itemData->disabled = channel->isDisabled;

    return itemData;
}

SvLocal QBXMBItem QBChannelControllerCreateGenericItemData(QBChannelController self, QBActiveTreeNode node, SvApplication app)
{
    QBXMBItem itemData = QBXMBItemCreate();
    SvString caption, subcaption;
    caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    subcaption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption"));
    if (caption) {
        const char *translation, *cCaption = SvStringCString(caption);
        translation = gettext(cCaption);
        if (translation == cCaption) {
            itemData->caption = SVRETAIN(caption);
        } else {
            itemData->caption = SvStringCreate(translation, NULL);
        }
    }

    if (subcaption) {
        const char *translation, *cCaption = SvStringCString(subcaption);
        translation = gettext(cCaption);
        if (translation == cCaption) {
            itemData->subcaption = SVRETAIN(subcaption);
        } else {
            itemData->subcaption = SvStringCreate(translation, NULL);
        }
    }

    SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
    itemData->loadingRID = self->iconRID;
    itemData->iconURI.URI = SVTESTRETAIN(iconPath);
    itemData->iconURI.isStatic = true;

    return itemData;
}

QBXMBItem QBChannelControllerCreateItemData(QBChannelController self, SvGenericObject node_, SvApplication app)
{
    QBXMBItem item = NULL;
    if (SvObjectIsInstanceOf(node_, SvTVChannel_getType())) {
        item = QBChannelControllerCreateChannelItemData(self, (SvTVChannel) node_, app);
    } else {
        item = QBChannelControllerCreateGenericItemData(self, (QBActiveTreeNode) node_, app);
    }

    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    item->disabledFocus = SVTESTRETAIN(self->disabledFocus);

    return item;
}

QBXMBChannelItemInfo QBChannelControllerCreateItemInfo(QBChannelController self, SvGenericObject node_)
{
    QBXMBChannelItemInfo itemChannelData = (QBXMBChannelItemInfo) SvTypeAllocateInstance(QBXMBChannelItemInfo_getType(), NULL);
    return itemChannelData;
}

SvLocal SvWidget QBChannelControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBChannelController self = (QBChannelController) self_;
    if (self->callbacks.createItem) {
        return self->callbacks.createItem(self_, node_, path, app, initialState);
    } else {
        QBXMBItem itemData = QBChannelControllerCreateItemData(self, node_, app);
        QBXMBItemInfo item = (QBXMBItemInfo) QBChannelControllerCreateItemInfo(self, node_);
        SvWidget ret = QBXMBItemConstructorInitItem(self->itemConstructor, itemData, app, initialState, item);
        SVRELEASE(itemData);
        return ret;
    }
}

SvLocal void QBChannelControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBChannelController self = (QBChannelController) self_;

    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBChannelControllerDestroy(void *self_)
{
    QBChannelController self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    if (self->showEPG && self->channelsList && self->epgWatcher) {
        QBEPGWatcherRemoveList(self->epgWatcher, (SvEPGView) self->channelsList);
    }
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->disabledFocus);
    SVTESTRELEASE(self->manager);
    SVTESTRELEASE(self->epgWatcher);
    SVTESTRELEASE(self->channelsList);
}

SvLocal SvType QBChannelController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controller_methods = {
        .createItem = QBChannelControllerCreateItem,
        .setItemState = QBChannelControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelController",
                            sizeof(struct QBChannelController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }

    return type;
}


QBChannelController QBChannelMenuControllerCreate(bool showEPG, AppGlobals appGlobals, SvGenericObject channelsList)
{
    QBChannelController self = (QBChannelController) SvTypeAllocateInstance(QBChannelController_getType(), NULL);

    self->appGlobals = appGlobals;
    self->itemConstructor = (QBXMBItemConstructor) QBChannelItemConstructorCreate("ChannelName", appGlobals->textRenderer);

    self->iconRID = svSettingsGetResourceID("ChannelName", "icon");
    self->focus = QBFrameConstructorFromSM("ChannelName.focus");
    if(svSettingsIsWidgetDefined("ChannelName.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("ChannelName.inactiveFocus");
    if(svSettingsIsWidgetDefined("ChannelName.disabledFocus"))
        self->disabledFocus = QBFrameConstructorFromSM("ChannelName.disabledFocus");

    self->renderer = appGlobals->textRenderer;
    self->channelsList = SVTESTRETAIN(channelsList);
    self->manager = SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    self->epgWatcher = SVTESTRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBEPGWatcher")));

    self->showEPG = showEPG;

    if (self->showEPG && self->channelsList && self->epgWatcher) {
        QBEPGWatcherAddList(self->epgWatcher, (SvEPGView) channelsList);
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->playlists = playlists;

    return self;
}

void
QBChannelMenuControllerSetCallbacks(SvGenericObject self_, QBChannelControllerCallbacks callbacks)
{
    if (!self_)
        return;

    QBChannelController self = (QBChannelController) self_;

    if (callbacks)
        self->callbacks = *callbacks;
}

SvInterface QBChannelMenu_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBChannelMenu", sizeof(struct QBChannelMenu_),
                                 NULL, &interface, NULL);
    }
    return interface;
}
