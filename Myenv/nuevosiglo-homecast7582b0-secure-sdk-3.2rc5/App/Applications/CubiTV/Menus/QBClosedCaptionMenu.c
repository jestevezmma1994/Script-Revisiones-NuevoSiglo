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

#include "QBClosedCaptionMenu.h"

#include <libintl.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBSortedList.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <QBWidgets/QBDialog.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBOSKPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <main.h>
#include <Menus/menuchoice.h>
#include <Windows/mainmenu.h>
#include <settings.h>
#include <QBConf.h>
#include <SWL/QBFrame.h>
#include <iso_639_table.h>
#include <TranslationMerger.h>

typedef struct QBClosedCaptionMenuHandler_t* QBClosedCaptionMenuHandler;
typedef struct QBClosedCaptionListElement_t* QBClosedCaptionListElement;
typedef enum QBClosedCaptionSetting_e QBClosedCaptionSetting;

enum QBClosedCaptionSetting_e {
    QBClosedCaptionSetting_PriChannel,
    QBClosedCaptionSetting_SecChannel,
    QBClosedCaptionSetting_PriLang,
    QBClosedCaptionSetting_SecLang
};

struct QBClosedCaptionListElement_t {
    struct SvObject_ super_;
    SvString caption;
    unsigned int pos;
    QBClosedCaptionSetting setting;
};

struct QBClosedCaptionMenuHandler_t {
    struct SvObject_ super_;
    QBActiveTreeNode mainNode;
    QBXMBItemConstructor itemConstructor;
    AppGlobals appGlobals;
    QBActiveTreeNode ccMode;
    QBActiveTreeNode analogCCChanel, analogCCChanelPrimary, analogCCChanelSecondary;
    QBActiveTreeNode digitalCCLang, digitalCCLangPrimary, digitalCCLangSecondary;

    QBTreeProxy analogCCChannelsProxyPri;
    QBTreeProxy analogCCChannelsProxySec;
    QBTreeProxy digitalCCLangsProxyPri;
    QBTreeProxy digitalCCLangsProxySec;
    QBSortedList analogCCListPri;
    QBSortedList analogCCListSec;
    QBSortedList digitalCCListPri;
    QBSortedList digitalCCListSec;

    SvRID tickMarkOnRID;
    SvRID tickMarkOffRID;
    SvRID iconRID;
    QBFrameConstructor* focus;
    QBFrameConstructor* disabledFocus;
    bool isManualEnabled;

    struct QBClosedCaptionSideMenu_t {
        QBContextMenu ctx;
    } sidemenu;
};

SvLocal void QBClosedCaptionListElementDestroy(void *self_)
{
    QBClosedCaptionListElement self = (QBClosedCaptionListElement) self_;
    SVTESTRELEASE(self->caption);
}

SvLocal SvType QBClosedCaptionListElement_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBClosedCaptionListElementDestroy
    };
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBClosedCaptionListElement",
                            sizeof(struct QBClosedCaptionListElement_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal QBClosedCaptionListElement QBClosedCaptionListElementCreate(unsigned int pos, QBClosedCaptionSetting setting, const char * caption)
{
    QBClosedCaptionListElement self = (QBClosedCaptionListElement) SvTypeAllocateInstance(QBClosedCaptionListElement_getType(), 0);
    self->caption = SvStringCreateWithFormat(caption, NULL);
    self->pos = pos;
    self->setting = setting;

    return self;
}

SvLocal void QBClosedCaptionMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position);
SvLocal void QBClosedCaptionMenuHandlerDestroy(void *self_);
SvLocal bool QBClosedCaptionMenuNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_);
SvLocal void QBClosedCaptionMenuCreateElementFromConfig(SvGenericObject self_,QBActiveTreeNode node_, QBXMBItem item, SvString configurationName);
SvLocal SvWidget QBClosedCaptionMenuCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState);
SvLocal void QBClosedCaptionMenuSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused);
SvLocal void QBClosedCaptionSideMenuClose(void *self_, QBContextMenu menu);
SvLocal void QBClosedCaptionAddAllNodes(QBClosedCaptionMenuHandler handler);
SvLocal void QBClosedCaptionMenuNodeUpdateSubcaption(QBActiveTreeNode node, SvGenericObject subcaption);
SvLocal void QBClosedCaptionSideMenuChoosen(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item);
SvLocal SvGenericObject QBClosedCaptionCreateSideMenu(QBClosedCaptionMenuHandler self);
SvLocal void QBClosedCaptionAddPaneOption(QBBasicPane options, const char *opt, QBBasicPaneItemCallback cbk, void *cbckData);
SvLocal void QBClosedCaptionPropagateNodesChange(QBClosedCaptionMenuHandler handler);
SvLocal int QBClosedCaptionListElementCompareFn(void *prv, SvGenericObject objectA, SvGenericObject objectB);
SvLocal void QBClosedCaptionUnmountManualNodes(QBClosedCaptionMenuHandler handler);
SvLocal void QBClosedCaptionMountManualNodes(QBClosedCaptionMenuHandler handler);
SvLocal void QBClosedCaptionUpdateSubcaptions(QBClosedCaptionMenuHandler self);
SvLocal void QBClosedCaptionRemoveAllNodes(QBClosedCaptionMenuHandler handler);
SvLocal const char * QBClosedCaptionMenuGetSetting(QBClosedCaptionSetting setting);

SvLocal SvType QBClosedCaptionMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBClosedCaptionMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
            .choosen = QBClosedCaptionMenuChoosen
    };

    static const struct QBMenuEventHandler_ selectMethods = {
            .nodeSelected = QBClosedCaptionMenuNodeSelected
    };

    static const struct XMBItemController_t methods = {
            .createItem = QBClosedCaptionMenuCreateItem,
            .setItemState = QBClosedCaptionMenuSetItemState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBClosedCaptionMenuHandler",
                            sizeof(struct QBClosedCaptionMenuHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                QBMenuEventHandler_getInterface(), &selectMethods,
                QBMenuChoice_getInterface(), &menuMethods,
                NULL);
    }

    return type;
}

static const char* const analogChanelOptions[] = {
        gettext_noop("off"),
        gettext_noop("CC1"),
        gettext_noop("CC3")
};
static const char* const digitalChanelOptions[] = {
        gettext_noop("off"),
        gettext_noop("eng"),
        gettext_noop("spa"),
        gettext_noop("dut") };

void QBClosedCaptionMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    QBActiveTreeNode mainNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("ClosedCaptions"));
    SvObject path = mainNode ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!path) {
        return;
    }

    QBClosedCaptionMenuHandler handler = (QBClosedCaptionMenuHandler)
            SvTypeAllocateInstance(QBClosedCaptionMenuHandler_getType(), NULL);
    handler->mainNode = mainNode;
    handler->appGlobals = appGlobals;

    svSettingsPushComponent("ClosedCaptioning.settings");
    handler->tickMarkOnRID = svSettingsGetResourceID("MenuItem", "tickMarkOn");
    handler->tickMarkOffRID = svSettingsGetResourceID("MenuItem", "tickMarkOff");
    handler->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    handler->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    handler->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.disabledFocus"))
        handler->disabledFocus = QBFrameConstructorFromSM("MenuItem.disabledFocus");
    svSettingsPopComponent();

    handler->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));
    const char *val = QBConfigGet("CLOSED_CAPTION_CHANNEL_MODE");
    handler->isManualEnabled = val && !strcmp(val, "Manual");

    handler->analogCCListPri = QBSortedListCreateWithCompareFn(QBClosedCaptionListElementCompareFn, NULL, NULL);
    handler->analogCCListSec = QBSortedListCreateWithCompareFn(QBClosedCaptionListElementCompareFn, NULL, NULL);
    handler->digitalCCListPri = QBSortedListCreateWithCompareFn(QBClosedCaptionListElementCompareFn, NULL, NULL);
    handler->digitalCCListSec = QBSortedListCreateWithCompareFn(QBClosedCaptionListElementCompareFn, NULL, NULL);
    for (unsigned int i = 0; i < sizeof(analogChanelOptions) / sizeof(*analogChanelOptions); i++) {
        SvGenericObject priElement = (SvGenericObject) QBClosedCaptionListElementCreate(i, QBClosedCaptionSetting_PriChannel, analogChanelOptions[i]);
        SvGenericObject secElement = (SvGenericObject) QBClosedCaptionListElementCreate(i, QBClosedCaptionSetting_SecChannel, analogChanelOptions[i]);
        QBSortedListInsert(handler->analogCCListPri, priElement);
        QBSortedListInsert(handler->analogCCListSec, secElement);
        SVRELEASE(priElement);
        SVRELEASE(secElement);
    }

    for (unsigned int i = 0; i < sizeof(digitalChanelOptions) / sizeof(*digitalChanelOptions); i++) {
        SvGenericObject priElement = (SvGenericObject) QBClosedCaptionListElementCreate(i, QBClosedCaptionSetting_PriLang, digitalChanelOptions[i]);
        SvGenericObject secElement = (SvGenericObject) QBClosedCaptionListElementCreate(i, QBClosedCaptionSetting_SecLang, digitalChanelOptions[i]);
        QBSortedListInsert(handler->digitalCCListPri, priElement);
        QBSortedListInsert(handler->digitalCCListSec, secElement);
        SVRELEASE(priElement);
        SVRELEASE(secElement);
    }
    QBContextMenuSetCallbacks(handler->sidemenu.ctx, QBClosedCaptionSideMenuClose, handler);
    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) handler, NULL);
    QBClosedCaptionAddAllNodes(handler);
    if (!handler->isManualEnabled)
        QBClosedCaptionUnmountManualNodes(handler);
    QBClosedCaptionUpdateSubcaptions(handler);
    SVRELEASE(handler);

    SVRELEASE(path);
}

void QBClosedCaptionMenuHandlerDestroy(void *self_)
{
    QBClosedCaptionMenuHandler self = (QBClosedCaptionMenuHandler) self_;
    QBClosedCaptionRemoveAllNodes(self);
    SVTESTRELEASE(self->sidemenu.ctx);
    SVRELEASE(self->itemConstructor);
    SVRELEASE(self->analogCCListPri);
    SVRELEASE(self->analogCCListSec);
    SVRELEASE(self->digitalCCListPri);
    SVRELEASE(self->digitalCCListSec);

    SVTESTRELEASE(self->analogCCChanel);
    SVTESTRELEASE(self->analogCCChanelPrimary);
    SVTESTRELEASE(self->analogCCChanelSecondary);

    SVTESTRELEASE(self->digitalCCLang);
    SVTESTRELEASE(self->digitalCCLangPrimary);
    SVTESTRELEASE(self->digitalCCLangSecondary);
    SVTESTRELEASE(self->ccMode);

    SVRELEASE(self->analogCCChannelsProxyPri);
    SVRELEASE(self->analogCCChannelsProxySec);
    SVRELEASE(self->digitalCCLangsProxyPri);
    SVRELEASE(self->digitalCCLangsProxySec);

    SVRELEASE(self->focus);
    SVTESTRELEASE(self->disabledFocus);
}

void QBClosedCaptionMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBClosedCaptionMenuHandler self = (QBClosedCaptionMenuHandler) self_;
    SvGenericObject options = NULL;

    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {

        QBActiveTreeNode node = (QBActiveTreeNode) node_;
        QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
        SvString configurationName = (SvString) QBActiveTreeNodeGetAttribute(parent,SVSTRING("configurationName"));

        if (configurationName) {
            SvString configurationValue = (SvString) QBActiveTreeNodeGetAttribute(node,SVSTRING("configurationValue"));
            const char * setting = SvStringCString(configurationValue);
            const char * settingParam = SvStringCString(configurationName);
            if (settingParam && setting) {
                QBConfigSet(settingParam, setting);
                QBConfigSave();
            }
        } else {
            SvString id = QBActiveTreeNodeGetID(node);

            //update closed captionong mode subcaption when menu language is changed
            QBClosedCaptionUpdateSubcaptions(self);
            if(id && SvStringEqualToCString(id, "Closed Captioning Mode")) {
                options = QBClosedCaptionCreateSideMenu(self);
            }
        }
    } else if (SvObjectIsInstanceOf(node_, QBClosedCaptionListElement_getType())) {
        QBClosedCaptionListElement node = (QBClosedCaptionListElement) node_;
        const char * setting = SvStringCString(node->caption);
        const char * settingParam = QBClosedCaptionMenuGetSetting(node->setting);
        if (settingParam && setting) {
            QBConfigSet(settingParam, setting);
            QBConfigSave();
        }
    }

    QBClosedCaptionPropagateNodesChange(self);

    if (options) {
        QBContextMenuShow(self->sidemenu.ctx);
        QBContextMenuPushPane(self->sidemenu.ctx, options);
        SVRELEASE(options);
    }
}

bool QBClosedCaptionMenuNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return false;
    return true;
}

SvLocal void QBClosedCaptionMenuCreateElementFromConfig(SvGenericObject self_,QBActiveTreeNode node_, QBXMBItem item, SvString configurationName)
{

    QBClosedCaptionMenuHandler self = (QBClosedCaptionMenuHandler) self_;
    QBActiveTreeNode node = node_;

    SvString configurationValue = (SvString)QBActiveTreeNodeGetAttribute((QBActiveTreeNode)node,SVSTRING("configurationValue"));

    const char * setting = NULL;
    const char * settingParam = SvStringCString(configurationName);
    if (settingParam)
       setting = QBConfigGet(settingParam);

    SVTESTRELEASE(item->icon);
    item->caption = (SvString)QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    item->focus = SVRETAIN(self->focus);
    item->disabledFocus = SVTESTRETAIN(self->disabledFocus);

    if (setting && SvStringEqualToCString(configurationValue, setting)) {
        item->iconRID = self->tickMarkOnRID;
    } else {
        item->iconRID = self->tickMarkOffRID;
    }

    const char *captionStr = SvStringCString(item->caption);
    if (!strcmp(captionStr,"off")) {
        item->caption = SvStringCreate(gettext(captionStr), NULL);
    } else {
        const char *lang = iso639GetLangName(SvStringCString(item->caption));
        if (lang) {
            item->caption = SvStringCreate(gettext(lang), NULL);
        } else {
            SVRETAIN(item->caption);
        }
    }
}

SvWidget QBClosedCaptionMenuCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBXMBItem item = 0;
    QBClosedCaptionMenuHandler self = (QBClosedCaptionMenuHandler) self_;
    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;
        QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
        SvString configurationName = (SvString) QBActiveTreeNodeGetAttribute(parent, SVSTRING("configurationName"));
        item = QBXMBItemCreate();
        if (configurationName) {
            QBClosedCaptionMenuCreateElementFromConfig(self_, node, item, configurationName);
        } else {
            item->loadingRID = self->iconRID;

            SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
            item->iconURI.URI = SVTESTRETAIN(iconPath);
            item->iconURI.isStatic = true;

            item->caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
            item->subcaption = SVTESTRETAIN(QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));
            item->focus = SVRETAIN(self->focus);
            item->disabledFocus = SVTESTRETAIN(self->disabledFocus);
            const char *captionStr, *translatedCaptionStr;
            captionStr = SvStringCString(item->caption);
            translatedCaptionStr = gettext(captionStr);
            if (translatedCaptionStr != captionStr)
                item->caption = SvStringCreate(translatedCaptionStr, NULL);
            else
                SVRETAIN(item->caption);

            const char *nodeID = SvStringCString(QBActiveTreeNodeGetID(node));
            if ((strcmp(nodeID, "Analog Closed Captioning") == 0)
                    || (strcmp(nodeID, "Digital Closed Captioning") == 0))
                item->disabled = !self->isManualEnabled;
        }
    } else if (SvObjectIsInstanceOf(node_, QBClosedCaptionListElement_getType())) {
        QBClosedCaptionListElement node = (QBClosedCaptionListElement) node_;
        item = QBXMBItemCreate();

        const char * setting = NULL;
        const char * settingParam = QBClosedCaptionMenuGetSetting(node->setting);
        if (settingParam)
           setting = QBConfigGet(settingParam);

        if (setting && SvStringEqualToCString(node->caption, setting)) {
            item->iconRID = self->tickMarkOnRID;
        } else {
            item->iconRID = self->tickMarkOffRID;
        }

        item->caption = node->caption;
        item->focus = SVRETAIN(self->focus);
        item->disabledFocus = SVTESTRETAIN(self->disabledFocus);
        const char *captionStr = SvStringCString(item->caption);
        if (!strcmp(captionStr,"off")) {
            item->caption = SvStringCreate(gettext(captionStr), NULL);
        } else {
            const char *lang = iso639GetLangName(SvStringCString(item->caption));
            if (lang) {
                item->caption = SvStringCreate(gettext(lang), NULL);
            } else {
                SVRETAIN(item->caption);
            }
        }
    }

    if (!item)
        return NULL;
    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void QBClosedCaptionMenuSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBClosedCaptionMenuHandler self = (QBClosedCaptionMenuHandler) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBClosedCaptionSideMenuClose(void *self_, QBContextMenu menu)
{
    QBClosedCaptionMenuHandler self = self_;
    SVTESTRELEASE(self->sidemenu.ctx);
    self->sidemenu.ctx = QBContextMenuCreateFromSettings("ContextMenu.settings", self->appGlobals->controller, self->appGlobals->res, SVSTRING("SideMenu"));
    QBContextMenuSetCallbacks(self->sidemenu.ctx, QBClosedCaptionSideMenuClose, self);
}

SvLocal void
QBClosedCaptionAddNode(QBClosedCaptionMenuHandler handler, QBActiveTreeNode *node_, const char *caption, QBActiveTreeNode parentNode)
{
    if (!caption || !parentNode)
        return;
    QBActiveTreeNode node = 0;
    if (node_ && !*node_) {
        SvString id = SvStringCreate(caption, NULL);
        node = QBActiveTreeNodeCreate(id, NULL, NULL);
        QBActiveTreeNodeSetAttribute(node, SVSTRING("caption"), (SvGenericObject) id);
        SVRELEASE(id);
    } else if (node_ && *node_)
        node = *node_;

    QBActiveTreeAddSubTree(handler->appGlobals->menuTree, parentNode, node, NULL);
    QBClosedCaptionMenuNodeUpdateSubcaption(node, NULL);

    if (node_)
        *node_ = node;
}

SvLocal void
QBClosedCaptionAddAllNodes( QBClosedCaptionMenuHandler handler )
{
    QBClosedCaptionAddNode( handler, &handler->ccMode, gettext_noop("Closed Captioning Mode"), handler->mainNode);
    QBClosedCaptionAddNode( handler, &handler->analogCCChanel, gettext_noop("Analog Closed Captioning"), handler->mainNode);
    QBClosedCaptionAddNode( handler, &handler->digitalCCLang, gettext_noop("Digital Closed Captioning"), handler->mainNode);

    QBClosedCaptionAddNode( handler, &handler->analogCCChanelPrimary, gettext_noop("CC Primary Channel"), handler->analogCCChanel);
    QBClosedCaptionAddNode( handler, &handler->analogCCChanelSecondary, gettext_noop("CC Secondary Channel"), handler->analogCCChanel);
    QBClosedCaptionAddNode( handler, &handler->digitalCCLangPrimary, gettext_noop("CC Primary Language"), handler->digitalCCLang);
    QBClosedCaptionAddNode( handler, &handler->digitalCCLangSecondary, gettext_noop("CC Secondary  Language"), handler->digitalCCLang);

    handler->analogCCChannelsProxyPri = QBTreeProxyCreate((SvGenericObject) handler->analogCCListPri, (SvGenericObject) handler->analogCCChanelPrimary, NULL);
    handler->analogCCChannelsProxySec = QBTreeProxyCreate((SvGenericObject) handler->analogCCListSec, (SvGenericObject) handler->analogCCChanelSecondary, NULL);

    handler->digitalCCLangsProxyPri = QBTreeProxyCreate((SvGenericObject) handler->digitalCCListPri, (SvGenericObject) handler->digitalCCLangPrimary, NULL);
    handler->digitalCCLangsProxySec = QBTreeProxyCreate((SvGenericObject) handler->digitalCCListSec, (SvGenericObject) handler->digitalCCLangSecondary, NULL);

    SvObject path = NULL;

    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Primary Channel")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->analogCCChannelsProxyPri, path, NULL);
        SVRELEASE(path);
    }
    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Secondary Channel")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->analogCCChannelsProxySec, path, NULL);
        SVRELEASE(path);
    }

    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Primary Language")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->digitalCCLangsProxyPri, path, NULL);
        SVRELEASE(path);
    }
    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Secondary Language")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->digitalCCLangsProxySec, path, NULL);
        SVRELEASE(path);
    }

    QBClosedCaptionMenuNodeUpdateSubcaption(handler->ccMode, (SvGenericObject)SVSTRING(""));

    QBClosedCaptionMenuNodeUpdateSubcaption(handler->analogCCChanelPrimary, (SvGenericObject)SVSTRING(""));
    QBClosedCaptionMenuNodeUpdateSubcaption(handler->analogCCChanelSecondary, (SvGenericObject)SVSTRING(""));
    QBClosedCaptionMenuNodeUpdateSubcaption(handler->digitalCCLangPrimary, (SvGenericObject)SVSTRING(""));
    QBClosedCaptionMenuNodeUpdateSubcaption(handler->digitalCCLangSecondary, (SvGenericObject)SVSTRING(""));

}

SvLocal void
QBClosedCaptionUnmountManualNodes(QBClosedCaptionMenuHandler handler)
{
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->analogCCChanelPrimary, NULL);
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->analogCCChanelSecondary, NULL);
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->digitalCCLangPrimary, NULL);
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->digitalCCLangSecondary, NULL);
}

SvLocal void
QBClosedCaptionMountManualNodes(QBClosedCaptionMenuHandler handler)
{
    QBClosedCaptionAddNode(handler, &handler->analogCCChanelPrimary, gettext_noop("CC Primary Channel"), handler->analogCCChanel);
    QBClosedCaptionAddNode(handler, &handler->analogCCChanelSecondary, gettext_noop("CC Secondary Channel"), handler->analogCCChanel);
    QBClosedCaptionAddNode(handler, &handler->digitalCCLangPrimary, gettext_noop("CC Primary Language"), handler->digitalCCLang);
    QBClosedCaptionAddNode(handler, &handler->digitalCCLangSecondary, gettext_noop("CC Secondary  Language"), handler->digitalCCLang);

    SvObject path = NULL;

    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Primary Channel")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->analogCCChannelsProxyPri, path, NULL);
        SVRELEASE(path);
    }
    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Secondary Channel")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->analogCCChannelsProxySec, path, NULL);
        SVRELEASE(path);
    }

    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Primary Language")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->digitalCCLangsProxyPri, path, NULL);
        SVRELEASE(path);
    }
    if ((path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("CC Secondary  Language")))) {
        QBActiveTreeMountSubTree(handler->appGlobals->menuTree, (SvObject) handler->digitalCCLangsProxySec, path, NULL);
        SVRELEASE(path);
    }

    QBClosedCaptionMenuNodeUpdateSubcaption(handler->ccMode, (SvGenericObject) SVSTRING(""));

    QBClosedCaptionMenuNodeUpdateSubcaption(handler->analogCCChanelPrimary, (SvGenericObject) SVSTRING(""));
    QBClosedCaptionMenuNodeUpdateSubcaption(handler->analogCCChanelSecondary, (SvGenericObject) SVSTRING(""));
    QBClosedCaptionMenuNodeUpdateSubcaption(handler->digitalCCLangPrimary, (SvGenericObject) SVSTRING(""));
    QBClosedCaptionMenuNodeUpdateSubcaption(handler->digitalCCLangSecondary, (SvGenericObject) SVSTRING(""));
}


SvLocal void
QBClosedCaptionRemoveAllNodes(QBClosedCaptionMenuHandler handler)
{
    if (handler->isManualEnabled)
    {
        QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->analogCCChanelPrimary, NULL);
        QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->analogCCChanelSecondary, NULL);
        QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->digitalCCLangPrimary, NULL);
        QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->digitalCCLangSecondary, NULL);
    }
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->analogCCChanel, NULL);
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->digitalCCLang, NULL);
    QBActiveTreeRemoveSubTree(handler->appGlobals->menuTree, handler->ccMode, NULL);
}

SvLocal SvGenericObject QBClosedCaptionCreateSideMenu(QBClosedCaptionMenuHandler self)
{
    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    svSettingsPushComponent("BasicPane.settings");
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer, self->sidemenu.ctx, 1, SVSTRING("BasicPane"));
    svSettingsPopComponent();

    QBClosedCaptionAddPaneOption(options, "Off", QBClosedCaptionSideMenuChoosen, self);
    QBClosedCaptionAddPaneOption(options, "Automatic", QBClosedCaptionSideMenuChoosen, self);
    QBClosedCaptionAddPaneOption(options, "Manual", QBClosedCaptionSideMenuChoosen, self);

    return (SvGenericObject) options;
}

SvLocal void QBClosedCaptionUpdateSubcaptions(QBClosedCaptionMenuHandler self)
{
    const char * mode = QBConfigGet("CLOSED_CAPTION_CHANNEL_MODE");
    if (!mode)
        mode = "";
    SvString str = SvStringCreate(gettext(mode), NULL);
    QBClosedCaptionMenuNodeUpdateSubcaption(self->ccMode, (SvGenericObject) str);
    SVRELEASE(str);
}


SvLocal void QBClosedCaptionSideMenuChoosen(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBClosedCaptionMenuHandler self = self_;
    const char *optionID = SvStringCString(id);
    if (!optionID) {
        SvLogError("%s: optionID == NULL", __func__);
        goto fini;
    }
    bool refreshNodes = false;
    QBConfigSet("CLOSED_CAPTION_CHANNEL_MODE", optionID);
    if (!strcmp(optionID, "Off") || !strcmp(optionID, "Automatic")) {
        refreshNodes = self->isManualEnabled;
        self->isManualEnabled = false;
    } else if (strcmp(optionID, "Manual") == 0) {
        refreshNodes = !self->isManualEnabled;
        self->isManualEnabled = true;
    }

    QBConfigSave();
    if (refreshNodes) {
        if (self->isManualEnabled)
            QBClosedCaptionMountManualNodes(self);
        else
            QBClosedCaptionUnmountManualNodes(self);
    }
fini:
    QBClosedCaptionUpdateSubcaptions(self);
    QBClosedCaptionPropagateNodesChange(self);
    QBContextMenuHide(self->sidemenu.ctx, false);
}

SvLocal void
QBClosedCaptionMenuNodeUpdateSubcaption(QBActiveTreeNode node, SvGenericObject subcaption)
{
    SvString stringVal = NULL;

    if (subcaption && SvObjectIsInstanceOf(subcaption, SvString_getType())) {
        stringVal = (SvString) subcaption;
        SVRETAIN(stringVal);
    } else {
        stringVal = SvStringCreate("", NULL);
    }
    QBActiveTreeNodeSetAttribute(node, SVSTRING("subcaption"), (SvGenericObject) stringVal);

    SVRELEASE(stringVal);
}

SvLocal void QBClosedCaptionPropagateNodesChange(QBClosedCaptionMenuHandler handler)
{
    SvObject path = QBActiveTreeCreateNodePath(handler->appGlobals->menuTree, SVSTRING("ClosedCaptions"));
    if (!path)
        return;

    QBTreeIterator iter = SvInvokeInterface(QBTreeModel, handler->appGlobals->menuTree, getIterator, path, 0);
    size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
    QBActiveTreePropagateNodesChange(handler->appGlobals->menuTree, path, 0, nodesCount, NULL);

    if (handler->isManualEnabled) {
        SvInvokeInterface(QBListModelListener, handler->analogCCChannelsProxyPri, itemsChanged, 0, QBSortedListCount(handler->analogCCListPri));
        SvInvokeInterface(QBListModelListener, handler->analogCCChannelsProxySec, itemsChanged, 0, QBSortedListCount(handler->analogCCListSec));
        SvInvokeInterface(QBListModelListener, handler->digitalCCLangsProxyPri, itemsChanged, 0, QBSortedListCount(handler->digitalCCListPri));
        SvInvokeInterface(QBListModelListener, handler->digitalCCLangsProxySec, itemsChanged, 0, QBSortedListCount(handler->digitalCCListSec));
    }

    SVRELEASE(path);
}

SvLocal void QBClosedCaptionAddPaneOption(QBBasicPane options, const char *opt, QBBasicPaneItemCallback cbk, void *cbckData)
{
    if (!options || !opt)
        return;
    SvString option = SvStringCreate(gettext(opt), NULL);
    SvString id = SvStringCreate(opt, NULL);
    QBBasicPaneAddOption(options, id, option, cbk, cbckData);
    SVRELEASE(option);
    SVRELEASE(id);
}

SvLocal int QBClosedCaptionListElementCompareFn(void *prv, SvGenericObject objectA, SvGenericObject objectB)
{
    QBClosedCaptionListElement first = (QBClosedCaptionListElement) objectA, second = (QBClosedCaptionListElement) objectB;
    return first->pos >= second->pos;
}

SvLocal const char * QBClosedCaptionMenuGetSetting(QBClosedCaptionSetting setting)
{
    switch(setting)
    {
    case QBClosedCaptionSetting_PriChannel:
        return "CLOSED_CAPTION_CHANNEL_PRI";
    case QBClosedCaptionSetting_SecChannel:
        return "CLOSED_CAPTION_CHANNEL_SEC";
    case QBClosedCaptionSetting_PriLang:
        return "CLOSED_CAPTION_LANG_PRI";
    case QBClosedCaptionSetting_SecLang:
        return "CLOSED_CAPTION_LANG_SEC";
    default:
        SvLogWarning("%s Unsupported value",__func__);
        return NULL;
    }
}

