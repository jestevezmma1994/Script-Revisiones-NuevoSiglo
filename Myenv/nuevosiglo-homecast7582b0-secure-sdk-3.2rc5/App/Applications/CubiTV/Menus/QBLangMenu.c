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

#include "QBLangMenu.h"

#include <dataformat/sv_data_format.h>
#include <libintl.h>
#include <SvGfxHAL/SvGfxUtils.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <SWL/fade.h>
#include <QBConf.h>

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <SWL/QBFrame.h>
#include <QBLocalWindowManager.h>
#include <Widgets/QBScrollBar.h>
#include <Windows/newtv.h>
#include <Logic/TVLogic.h>
#include <Logic/AudioTrackLogic.h>
#include <QBAppKit/QBServiceRegistry.h>

#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBMenuController.h>
#include <QBMenu/QBMenu.h>

#include <QBTeletextSubtitlesManager.h>
#include <QBScte27Subs.h>
#include <QBClosedCaptionSubsManager.h>
#include <QBExtSubsManager.h>
#include <Widgets/QBXMBItemConstructor.h>

#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePathMap.h>

#include <iso_639_table.h>
#include <main.h>

#define log_error(fmt, ...)  SvLogError(COLBEG() "QBLangMenu : " fmt COLEND_COL(red), ##__VA_ARGS__)
#define log_warning(fmt, ...)  SvLogWarning(COLBEG() "QBLangMenu : " fmt COLEND_COL(yellow), ##__VA_ARGS__)
#define log_fun(fmt, ...)  if(0) SvLogNotice(COLBEG() " %s %d " fmt  COLEND_COL(cyan), __func__,__LINE__,##__VA_ARGS__)

typedef struct QBLangItemController_t* QBLangItemController;

struct QBLangMenu_t {
    struct QBLocalWindow_t super_;

    SvWidget xmbMenu;

    QBActiveTree tree;
    QBLangItemController controller;

    AppGlobals appGlobals;

    struct svdataformat* format;

    SvEffectId fadeId;
    double fadeTime;

    bool visible;

    SvString subsLang;

    void *audioCallbackData;
    QBLangMenuAudioCallback audioCallback;

    void *subsCallbackData;
    QBLangMenuSubsCallback subsCallback;

    QBSubsManager subsManager;
};

SvLocal void QBLangMenuDestroy(void *self_)
{
    QBLangMenu self = (QBLangMenu) self_;
    SVTESTRELEASE(self->subsManager);
    SVTESTRELEASE(self->tree);
    SVTESTRELEASE(self->controller);
    SVTESTRELEASE(self->subsLang);
    if (self->format)
        svdataformat_release(self->format);
    svWidgetDestroy(self->super_.window);
}

SvLocal SvType
QBLangMenu_getType(void)
{
    static const struct QBWindowVTable_ vtable = {
        .super_      = {
            .destroy = QBLangMenuDestroy
        }
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLangMenu",
                            sizeof(struct QBLangMenu_t),
                            QBLocalWindow_getType(),
                            &type,
                            SvObject_getType(), &vtable,
                            NULL);
    }
    return type;
}

// bar item controller

typedef struct QBLangBarController_t *QBLangBarController;
struct QBLangBarController_t {
    struct SvObject_ super_;
};

SvLocal SvWidget
QBLangBarControllerCreateItem(SvGenericObject self_,
        SvGenericObject node_,
        SvGenericObject path,
        SvApplication app,
        XMBMenuState initialState)
{
    return svWidgetCreateBitmap(app, 1, 1, NULL);
}

    SvLocal void
QBLangBarControllerSetItemState(SvGenericObject self_,
        SvWidget item,
        XMBMenuState state,
        bool isFocused)
{
}

    SvLocal SvType
QBLangBarController_getType(void)
{
    static const struct XMBItemController_t methods = {
        .createItem     = QBLangBarControllerCreateItem,
        .setItemState   = QBLangBarControllerSetItemState
    };

    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBLangBarController",
                sizeof(struct QBLangBarController_t),
                SvObject_getType(), &type,
                XMBItemController_getInterface(), &methods,
                NULL);
    }

    return type;
}

    SvLocal QBLangBarController
QBLangBarControllerCreate(void)
{
    return (QBLangBarController) SvTypeAllocateInstance(QBLangBarController_getType(), NULL);
}


struct QBLangItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBXMBItemConstructor itemConstructor;

    SvFont font;
    SvColor textColor;
    int spacing;
    int epgSpacing;
    int normalDim;
    int inactiveDim;
    int fontSize;
    int subFontSize;
    int width;
    int height;

    int focusXOffset;
    int focusYOffset;

    SvBitmap boxBitmap;
    SvBitmap tickMarkOn;
    SvBitmap tickMarkOff;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;

    float focusFadeTime;

    int leftPadding;
    SvPlayerAudioTrack audioTrack;

    QBSubsManager subsManager;
};

SvLocal SvWidget QBLangItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    log_fun("node_ = %p", node_);
    QBLangItemController self = (QBLangItemController) self_;

    SvString caption = NULL;
    SvString subcaption = NULL;


    SvBitmap iconBitmap = NULL;

    QBXMBItem item = QBXMBItemCreate();

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;
        caption = SVTESTRETAIN((SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption")));
        SvString type = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("type"));
        //SvGenericObject langId = QBActiveTreeNodeGetAttribute(node, SVSTRING("langId"));

        log_fun("type=%s", type ? SvStringCString(type) : NULL);
        if (type && SvStringEqualToCString(type, "AUDIO")) {
            SvPlayerAudioTrack audioTrack = (SvPlayerAudioTrack) QBActiveTreeNodeGetAttribute(node, SVSTRING("audioTrack"));
            AudioTrackLogic audioLogic = (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
            SvPlayerTaskControllers taskControllers = AudioTrackLogicGetPlayerTaskControllers(audioLogic);
            SvPlayerTrackController audioTrackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_audio);
            if (audioTrack && audioTrackController) {
                unsigned int currentIdx = SvPlayerTrackControllerGetCurrentTrackIndex(audioTrackController);
                SvArray tracks = SvPlayerTrackControllerGetTracks(audioTrackController);
                SvPlayerAudioTrack currentTrack = (SvPlayerAudioTrack) SvArrayGetObjectAtIndex(tracks, currentIdx);
                if (SvObjectEquals((SvObject) audioTrack, (SvObject) currentTrack))
                    iconBitmap = self->tickMarkOn;
                else
                    iconBitmap = self->tickMarkOff;
            }
        }
    } else if (SvObjectIsInstanceOf(node_, QBSubsTrack_getType())) {
        QBSubsTrack node = (QBSubsTrack) node_;
        QBSubsTrack currentTrack = QBSubsManagerGetCurrentTrack(self->subsManager);

        SvString cap1 = NULL;
        iconBitmap = (node == currentTrack) ? self->tickMarkOn : self->tickMarkOff;

        if (node->langCode) {
            if (SvStringEqualToCString(node->langCode, "off")) {
                cap1 = SvStringCreate(gettext("off"), NULL);
            } else {
                const char *lang = iso639GetLangName(SvStringCString(node->langCode));
                if (lang)
                    cap1 = SvStringCreate(gettext(lang), NULL);
            }
        }

        if (!cap1 && SvObjectIsInstanceOf(node_, QBAnalogCCTrack_getType())) {
            const char *ccName = QBAnalogCCTrackGetChannelName(node_);
            if (ccName) {
                cap1 = SvStringCreate(ccName, NULL);
            }
        }

        if (!cap1)
            cap1 = SvStringCreate(gettext("unknown"), NULL);

        bool hh = SvObjectIsInstanceOf((SvObject) node, QBDvbSubsTrack_getType()) && QBDvbSubsTrackisHardOfHearing((QBDvbSubsTrack) node);
        const char *type = NULL;
        if (SvObjectIsInstanceOf(node_, QBDvbSubsTrack_getType())) {
            type = "[DVB]";
        } else if (SvObjectIsInstanceOf(node_, QBTeletextSubtitlesTrack_getType())) {
            type = "[Teletext]";
        } else if (SvObjectIsInstanceOf(node_, QBScte27SubsTrack_getType())) {
            type = "[DCII]";
        } else if (SvObjectIsInstanceOf(node_, QBExtSubsTrack_getType())) {
            type = SvStringCString(QBExtSubsTrackGetSubtitleType((QBExtSubsTrack) node_));
            subcaption = SVTESTRETAIN(QBExtSubsTrackGetSubtitleFilename((QBExtSubsTrack) node_));
        } else if (SvObjectIsInstanceOf(node_, QBAnalogCCTrack_getType())) {
            type = "[Analog CC]";
        } else if (SvObjectIsInstanceOf(node_, QBDTVCCTrack_getType())) {
            type = "[Digital CC]";
        }

        caption = SvStringCreateWithFormat("%s %s %s", gettext(SvStringCString(cap1)), hh ? "(HoH)" : "", type ? type : "");
        SVTESTRELEASE(cap1);
    } else {
        caption = SVSTRING("???");
    }

    if (!item->focus)
        item->focus = SVRETAIN(self->focus);

    if (!item->inactiveFocus)
        item->inactiveFocus = SVRETAIN(self->inactiveFocus);

    if (!item->icon)
        item->icon = SVTESTRETAIN(iconBitmap);

    item->caption = caption;
    item->subcaption = subcaption;

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void QBLangItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBLangItemController self = (QBLangItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBLangItemControllerDestroy(void *self_)
{
    QBLangItemController self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->font);
    SVRELEASE(self->boxBitmap);
    SVRELEASE(self->focus);
    SVRELEASE(self->inactiveFocus);
    SVRELEASE(self->tickMarkOn);
    SVRELEASE(self->tickMarkOff);
    SVTESTRELEASE(self->audioTrack);
    SVRELEASE(self->subsManager);
}


SvLocal SvWidget QBLangItemControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()) || QBActiveTreeNodeGetChildNodesCount((QBActiveTreeNode) node_) == 0)
        return NULL;

    SvWidget xmbMenu, bg, scrollBar;
    svSettingsPushComponent("LangMenu.settings");

    if(SvInvokeInterface(QBTreePath, path, getLength)<2){
        xmbMenu = XMBVerticalMenuNew(app, "langBar.smallMenu", NULL);
        bg = QBFrameCreateFromSM(app, "langBar.smallBg");
        scrollBar = QBScrollBarNew(app, "ScrollBarSmall");
    }else{
        xmbMenu = XMBVerticalMenuNew(app, "langBar.bigMenu", NULL);
        bg = QBFrameCreateFromSM(app, "langBar.bigBg");
        scrollBar = QBScrollBarNew(app, "ScrollBarBig");
    }

    svSettingsWidgetAttach(bg, scrollBar, svWidgetGetName(scrollBar), 2);
    XMBVerticalMenuAddRangeListener(xmbMenu, QBScrollBarGetRangeListener(scrollBar), NULL);
    XMBVerticalMenuSetBG(xmbMenu, bg);
    svSettingsPopComponent();
    return xmbMenu;
}

SvLocal SvType QBLangItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBLangItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_methods = {
        .createSubMenu = QBLangItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t methods = {
        .createItem = QBLangItemControllerCreateItem,
        .setItemState =  QBLangItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBLangItemController",
                            sizeof(struct QBLangItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBMenuController_getInterface(), &menu_methods,
                XMBItemController_getInterface(), &methods,
                NULL);
    }

    return type;
}

SvLocal void
QBLangMenuSetAudioTrack(QBLangMenu menu, SvPlayerAudioTrack audioTrack)
{
    SVTESTRELEASE(menu->controller->audioTrack);
    menu->controller->audioTrack = SVTESTRETAIN(audioTrack);
}

SvLocal SvGenericObject QBLangItemControllerCreate(AppGlobals appGlobals, QBSubsManager subsManager)
{
    svSettingsPushComponent("LangMenu.settings");

    QBLangItemController self = (QBLangItemController) SvTypeAllocateInstance(QBLangItemController_getType(), NULL);

    self->appGlobals = appGlobals;
    self->itemConstructor = QBXMBItemConstructorCreate("LangItem", appGlobals->textRenderer);

    self->font = svSettingsCreateFont("LangItem", "font");
    self->textColor = svSettingsGetColor("LangItem", "textColor", 0);
    self->spacing = svSettingsGetInteger("LangItem", "spacing", 0);
    self->leftPadding = svSettingsGetInteger("LangItem", "leftPadding", 0);
    self->normalDim = svSettingsGetInteger("LangItem", "normalDim", 0);
    self->inactiveDim = svSettingsGetInteger("LangItem", "inactiveDim", 0);
    self->fontSize = svSettingsGetInteger("LangItem", "fontSize", 0);
    self->subFontSize = svSettingsGetInteger("LangItem", "subFontSize", 0);
    self->width = svSettingsGetInteger("LangItem", "width", 0);
    self->height = svSettingsGetInteger("LangItem", "height", 0);
    self->focusXOffset = svSettingsGetInteger("LangItem", "focusXOffset", 0);
    self->focusYOffset = svSettingsGetInteger("LangItem", "focusYOffset", 0);
    self->boxBitmap = SVRETAIN(svSettingsGetBitmap("LangItem", "boxBitmap"));
    self->tickMarkOn = SVRETAIN(svSettingsGetBitmap("LangItem", "tickMarkOn"));
    self->tickMarkOff = SVRETAIN(svSettingsGetBitmap("LangItem", "tickMarkOff"));
    self->focusFadeTime = svSettingsGetDouble("LangItem", "focusFadeTime", 0.0);
    self->focus = QBFrameConstructorFromSM("LangItem.focus");
    self->inactiveFocus = QBFrameConstructorFromSM("LangItem.inactiveFocus");
    self->audioTrack = NULL;
    self->subsManager = SVRETAIN(subsManager);

    svSettingsPopComponent();

    return (SvGenericObject) self;
}

SvLocal void QBLangMenuCreateAudioActiveTree(QBActiveTreeNode audioMenu)
{
    SvHashTable item;
    SvString attrName;

    AudioTrackLogic audioTrackLogic =
        (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    SvArray items = AudioTrackLogicComputeLangMenuItems(audioTrackLogic);

    SvIterator itemsIter = SvArrayIterator(items);
    while ((item = (SvHashTable) SvIteratorGetNext(&itemsIter))) {
        // create menu node
        QBActiveTreeNode audioNode = QBActiveTreeNodeCreate(NULL, audioMenu, NULL);
        // add 'type' attribute
        SvString nodeType = SvStringCreate("AUDIO", NULL);
        QBActiveTreeNodeSetAttribute(audioNode, SVSTRING("type"), (SvGenericObject) nodeType);
        SVRELEASE(nodeType);
        // add remaining attributes
        SvIterator attrsIter = SvHashTableKeysIterator(item);
        while ((attrName = (SvString) SvIteratorGetNext(&attrsIter))) {
            SvGenericObject attrValue = SvHashTableFind(item, (SvGenericObject) attrName);
            QBActiveTreeNodeSetAttribute(audioNode, attrName, attrValue);
        }
        // add menu node to the tree
        QBActiveTreeNodeAddChildNode(audioMenu, audioNode, NULL);
        SVRELEASE(audioNode);
    }

    SVRELEASE(items);
}

SvLocal void QBLangMenuCreateSubsActiveTree(QBActiveTree tree, QBActiveTreeNode root, QBActiveArray subtitles)
{
    SvString subsMenuId = SVSTRING("X_SUBS");

    QBActiveTreeNode subsMenu = QBActiveTreeNodeCreate(subsMenuId, root, NULL);
    SvString subsCap = SvStringCreate(gettext("Subtitle Language"), NULL);
    QBActiveTreeNodeSetAttribute(subsMenu, SVSTRING("caption"), (SvGenericObject) subsCap);
    SVRELEASE(subsCap);
    QBActiveTreeAddSubTree(tree, root, subsMenu, NULL);

    // why is this necessary?
    if(1){
        QBActiveTreeNode test2 = QBActiveTreeNodeCreate(SVSTRING("TEST2"), subsMenu, NULL);
        QBActiveTreeAddSubTree(tree, subsMenu, test2, NULL);
        SVRELEASE(test2);
    }

    SvGenericObject path = QBActiveTreeCreateNodePath(tree, subsMenuId);
    QBTreeProxy proxy = QBTreeProxyCreate((SvGenericObject) subtitles, NULL, NULL);
    QBActiveTreeMountSubTree(tree, (SvGenericObject) proxy, path, NULL);
    SVRELEASE(proxy);
    SVRELEASE(path);

    SVRELEASE(subsMenu);
}

SvLocal void QBLangMenuCreateCCActiveTree(QBActiveTree tree, QBActiveTreeNode root, QBActiveArray cc)
{
    SvString subsMenuId = SVSTRING("CLOSED_CAPTIONS");

    QBActiveTreeNode subsMenu = QBActiveTreeNodeCreate(subsMenuId, root, NULL);
    SvString subsCap = SvStringCreate(gettext("Closed Captioning"), NULL);
    QBActiveTreeNodeSetAttribute(subsMenu, SVSTRING("caption"), (SvGenericObject) subsCap);
    SVRELEASE(subsCap);
    QBActiveTreeAddSubTree(tree, root, subsMenu, NULL);

    // why is this necessary?
    if(1){
        QBActiveTreeNode test2 = QBActiveTreeNodeCreate(SVSTRING("TEST3"), subsMenu, NULL);
        QBActiveTreeAddSubTree(tree, subsMenu, test2, NULL);
        SVRELEASE(test2);
    }

    SvGenericObject path = QBActiveTreeCreateNodePath(tree, subsMenuId);
    QBTreeProxy proxy = QBTreeProxyCreate((SvGenericObject) cc, NULL, NULL);
    QBActiveTreeMountSubTree(tree, (SvGenericObject) proxy, path, NULL);
    SVRELEASE(proxy);
    SVRELEASE(path);

    SVRELEASE(subsMenu);
}

SvLocal QBActiveTree QBLangMenuCreateTree(QBLangMenu menu, struct svdataformat* format)
{
    QBActiveTree tree;
    QBActiveTreeNode root;
    QBActiveTreeNode fakeLevel1Node;
    QBActiveTreeNode audioMenu;

    tree = (QBActiveTree) SvTypeAllocateInstance(QBActiveTree_getType(), NULL);
    root = QBActiveTreeNodeCreate(NULL, NULL, NULL);

    fakeLevel1Node = QBActiveTreeNodeCreate(SVSTRING("ROOT"), root, NULL);
    QBActiveTreeNodeAddChildNode(root, fakeLevel1Node, NULL);
    SVRELEASE(fakeLevel1Node);

    audioMenu = QBActiveTreeNodeCreate(SVSTRING("X_AUDIO"), root, NULL);
    QBActiveTreeNodeAddChildNode(fakeLevel1Node, audioMenu, NULL);
    SvString audioCap = SvStringCreate(gettext("Audio Track"), NULL);
    QBActiveTreeNodeSetAttribute(audioMenu, SVSTRING("caption"), (SvGenericObject) audioCap);
    SVRELEASE(audioCap);

    QBLangMenuCreateAudioActiveTree(audioMenu);

    QBActiveTreeInit(tree, root, NULL);
    QBActiveArray subsTracks = QBSubsManagerGetAllTracks(menu->subsManager);
    if (QBActiveArrayCount(subsTracks) > 1) {
        QBLangMenuCreateSubsActiveTree(tree, fakeLevel1Node, subsTracks);
    }
    QBActiveArray ccTracks = QBSubsManagerGetClosedCaptionsTracks(menu->subsManager);
    if (QBActiveArrayCount(ccTracks) > 1) { // one NULL tracks is always there
        QBLangMenuCreateCCActiveTree(tree, fakeLevel1Node, ccTracks);
    }
    SVRELEASE(audioMenu);
    SVRELEASE(root);

    return tree;
}

void QBLangMenuSetAudioCallback(QBLangMenu menu, QBLangMenuAudioCallback callback, void *callbackData) {
    if(!menu)
        return;
    menu->audioCallback = callback;
    menu->audioCallbackData = callbackData;
}

void QBLangMenuSetSubsCallback(QBLangMenu menu, QBLangMenuSubsCallback callback, void *callbackData) {
    if(!menu)
        return;
    menu->subsCallback = callback;
    menu->subsCallbackData = callbackData;
}

SvLocal void QBLangMenuSetFormat(QBLangMenu menu, struct svdataformat* format)
{
    if (!menu)
        return;

    if (!menu->tree || menu->format != format) {
        SVTESTRELEASE(menu->tree);

        if (menu->format)
            svdataformat_release(menu->format);

        if (format)
            svdataformat_retain(format);
        menu->format = format;

        //int subsIndex = QBLangPreferencesGetSubtitleIndex(menu->appGlobals->langPreferences, menu->subsLang, format);

        menu->tree = QBLangMenuCreateTree(menu, format);

        QBLangBarController barController = QBLangBarControllerCreate();
        SvInvokeInterface(QBMenu, menu->xmbMenu->prv, connectToDataSource,
                          (SvObject) menu->tree, (SvObject) barController, NULL,
                          (SvObject) menu->controller, NULL);

        SVRELEASE(barController);
    }
}

SvLocal void QBLangMenuUserEventHandler(SvWidget w, SvWidgetId sender, SvUserEvent ue)
{
    QBLangMenu self = w->prv;

    if (ue->code != SV_EVENT_QB_MENU_SELECTED)
        return;

    QBMenuEvent data = ue->prv;
    if (SvInvokeInterface(QBTreePath, data->nodePath, getLength) <= 2)
        return;

    if (!data->clicked)
        return;


    if (SvObjectIsInstanceOf(data->node, QBActiveTreeNode_getType())) {
        SvString type = (SvString) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) data->node, SVSTRING("type"));

    //    log_fun("type=%s langId=%i", type ? SvStringCString(type) : NULL, langIndex);

        if (type && SvStringEqualToCString(type, "AUDIO")) {
            SvPlayerAudioTrack audioTrack = (SvPlayerAudioTrack) QBActiveTreeNodeGetAttribute((QBActiveTreeNode) data->node, SVSTRING("audioTrack"));
            QBLangMenuSetAudioTrack(self, audioTrack);
            if (self->audioCallback)
                self->audioCallback(self->audioCallbackData, audioTrack);
            AudioTrackLogic audioTrackLogic =
                (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
            AudioTrackLogicSetAudioTrack(audioTrackLogic, audioTrack);
        }
        SvGenericObject path = SvInvokeInterface(QBMenu, self->xmbMenu->prv, getPosition);
        QBActiveTree menuTree = self->tree;
        if (path && SvInvokeInterface(QBTreePath, path, getLength) > 0) {
            SvGenericObject parentPath;
            if ((parentPath = SvObjectCopy(data->nodePath, NULL))) {
                SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
                QBTreeIterator iter = SvInvokeInterface(QBTreeModel, menuTree, getIterator, parentPath, 0);
                size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
                QBActiveTreePropagateNodesChange(menuTree, parentPath, 0, nodesCount, NULL);
                SVRELEASE(parentPath);
            }
        }

    } else if (SvObjectIsInstanceOf(data->node, QBSubsTrack_getType())) {
        QBSubsManagerSetCurrentTrack(self->subsManager, (QBSubsTrack)data->node);
        if (self->subsCallback)
            self->subsCallback(self->subsCallbackData, (QBSubsTrack)data->node);
    } else {
        return;
    }

}

SvLocal bool QBLangMenuInputEventHandler(SvWidget w, SvInputEvent ie)
{
    switch(ie->ch){
        case QBKEY_UP:
        case QBKEY_DOWN:
        case QBKEY_LEFT:
        case QBKEY_RIGHT:
            break;
        default:
            return false;
    }
    return true;
}

SvLocal void
QBLangMenuClean(SvApplication app, void *prv) {
}

void QBLangMenuSetSubsLang(QBLangMenu menu, SvString subsLang)
{
    SVTESTRELEASE(menu->subsLang);
    menu->subsLang = SVTESTRETAIN(subsLang);
}

QBLangMenu QBLangMenuNew(AppGlobals appGlobals, QBSubsManager subsManager)
{
    QBLangMenu self = (QBLangMenu) SvTypeAllocateInstance(QBLangMenu_getType(), NULL);
    SvApplication app = appGlobals->res;

    self->subsManager = SVTESTRETAIN(subsManager);
    self->appGlobals = appGlobals;
    SvWidget window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    svWidgetSetName(window, "langMenu");
    QBLocalWindowInit((QBLocalWindow) self, window, QBLocalWindowTypeFocusable);

    window->prv = self;
    svWidgetSetUserEventHandler(window, QBLangMenuUserEventHandler);
    svWidgetSetInputEventHandler(window, QBLangMenuInputEventHandler);
    window->clean = QBLangMenuClean;

    svSettingsPushComponent("LangMenu.settings");

    self->xmbMenu = XMBMenuBarNew(appGlobals->res, "langBar", NULL);
    self->fadeTime = svSettingsGetDouble("langBar", "fadeTime", 0.3);
    svSettingsWidgetAttach(window, self->xmbMenu, svWidgetGetName(self->xmbMenu), 10);

    QBLangBarController barController = QBLangBarControllerCreate();
    SvGenericObject controller = QBLangItemControllerCreate(self->appGlobals, self->subsManager);

    SvInvokeInterface(QBMenu, self->xmbMenu->prv, setNotificationTarget, svWidgetGetId(window));
    self->controller = (QBLangItemController) controller;
    SVRELEASE(barController);

    svSettingsPopComponent();

    return self;
}

void QBLangMenuShow(QBLangMenu self, QBLangMenuSubMenu submenu) {

    QBLangMenuSetFormat(self, QBViewportGetOutputFormat(QBViewportGet()));

    QBApplicationControllerAddLocalWindow(self->appGlobals->controller, (QBLocalWindow) self);
    SvString tag = NULL;
    switch(submenu){
        case QBLangMenuSubMenu_Audio:
            tag = SVSTRING("X_AUDIO");
            break;
        case QBLangMenuSubMenu_Subtitles:
            tag = SVSTRING("X_SUBS");
            break;
    }
    if (self->tree) {
        SvGenericObject tagPath = QBActiveTreeCreateNodePath(self->tree, tag);
        if (tagPath) {
            QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->tree, getIterator, tagPath, 0);
            SvGenericObject subNodePath = QBTreeIteratorGetCurrentNodePath(&iter);

            SvGenericObject path = subNodePath ? subNodePath : tagPath;

            SvInvokeInterface(QBMenu, self->xmbMenu->prv, setPosition, path, NULL);
            SVRELEASE(tagPath);
        }
    }
    if(self->fadeId)
        svAppCancelEffect(self->appGlobals->res, self->fadeId, SV_EFFECT_ABANDON);
    self->super_.window->tree_alpha = ALPHA_TRANSPARENT;
    SvEffect effect = svEffectFadeNew(self->super_.window, ALPHA_SOLID, 1, self->fadeTime, SV_EFFECT_FADE_LOGARITHM);
    self->fadeId = svAppRegisterEffect(self->appGlobals->res, effect);
    self->visible = true;
}

void QBLangMenuHide(QBLangMenu self) {

    QBApplicationControllerRemoveLocalWindow(self->appGlobals->controller, (QBLocalWindow)self);
    self->visible = false;
}

bool QBLangMenuIsVisible(QBLangMenu self)
{
    return self->visible;
}

SvLocal QBActiveTreeNode QBLangMenuFindMatchingAudioMenuNode(QBActiveTreeNode menu, SvPlayerTrackController audioTrackController, SvPlayerAudioTrack audioTrack)
{
    SvArray tracks = SvPlayerTrackControllerGetTracks(audioTrackController);

    for (size_t i = 0; i < SvArrayGetCount(tracks); i++) {
        if (SvObjectEquals((SvObject) audioTrack, (SvObject) SvArrayGetObjectAtIndex(tracks, i)))
            return (QBActiveTreeNode) SvInvokeInterface(QBListModel, menu, getObject, i);
    }

    return NULL;
}

void QBLangMenuUpdateAudioTracks(QBLangMenu self)
{
    QBActiveTreeNode audioMenu, tmpAudioMenu;
    SvString descTag = SVSTRING("audioTrack");
    unsigned int i, cnt, previousCnt;
    SvPlayerAudioTrack audioTrack;

    audioMenu = QBActiveTreeFindNode(self->tree, SVSTRING("X_AUDIO"));
    SvObject path = audioMenu ? QBActiveTreeCreateNodePath(self->tree, QBActiveTreeNodeGetID(audioMenu)) : NULL;
    if (!audioMenu || !path)
        return;

    AudioTrackLogic audioLogic = (AudioTrackLogic) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("AudioTrackLogic"));
    SvPlayerTaskControllers taskControllers = AudioTrackLogicGetPlayerTaskControllers(audioLogic);
    if (!taskControllers) {
        QBActiveTreeRemoveNodes(self->tree, audioMenu, NULL);
        SVRELEASE(path);
        return;
    }

    SvPlayerTrackController audioTrackController = SvPlayerTaskControllersGetTrackController(taskControllers, SvPlayerTrackControllerType_audio);
    if (!audioTrackController) {
        QBActiveTreeRemoveNodes(self->tree, audioMenu, NULL);
        SVRELEASE(path);
        return;
    }

    tmpAudioMenu = QBActiveTreeNodeCreate(SVSTRING("X_AUDIO"), NULL, NULL);
    QBLangMenuCreateAudioActiveTree(tmpAudioMenu);

    // iterate over existing menu nodes, update ones that represent audio tracks that
    // are still present, mark others to be removed
    previousCnt = SvInvokeInterface(QBListModel, audioMenu, getLength);
    for (i = 0; i < previousCnt; i++) {
        QBActiveTreeNode existingNode = (QBActiveTreeNode) SvInvokeInterface(QBListModel, audioMenu, getObject, i);
        audioTrack = (SvPlayerAudioTrack) QBActiveTreeNodeGetAttribute(existingNode, descTag);
        QBActiveTreeNode newNode = QBLangMenuFindMatchingAudioMenuNode(tmpAudioMenu, audioTrackController, audioTrack);
        if (newNode) {
            // copy new stream description to existing node
            audioTrack = (SvPlayerAudioTrack) QBActiveTreeNodeGetAttribute(newNode, descTag);
            QBActiveTreeNodeSetAttribute(existingNode, descTag, (SvGenericObject) audioTrack);
            // remove stream description from new node to mark it as already added
            QBActiveTreeNodeSetAttribute(newNode, descTag, NULL);
        } else {
            // remove stream description from existing node to mark it for removal
            QBActiveTreeNodeSetAttribute(existingNode, descTag, NULL);
        }
    }

    // add new nodes
    cnt = SvInvokeInterface(QBListModel, tmpAudioMenu, getLength);
    for (i = 0; i < cnt; i++) {
        QBActiveTreeNode newNode = (QBActiveTreeNode) SvInvokeInterface(QBListModel, tmpAudioMenu, getObject, i);
        if (!QBActiveTreeNodeGetAttribute(newNode, descTag))
            continue;
        QBActiveTreeAddSubTree(self->tree, audioMenu, newNode, NULL);
    }

    // remove marked existing nodes
    for (i = previousCnt; i > 0; ) {
        --i;
        QBActiveTreeNode existingNode = (QBActiveTreeNode) SvInvokeInterface(QBListModel, audioMenu, getObject, i);
        if (!QBActiveTreeNodeGetAttribute(existingNode, descTag)) {
            QBActiveTreeRemoveSubTree(self->tree, existingNode, NULL);
            previousCnt -= 1;
        }
    }

    SVRELEASE(tmpAudioMenu);

    QBActiveTreePropagateNodesChange(self->tree, path, 0, previousCnt, NULL);

    SVRELEASE(path);
}
