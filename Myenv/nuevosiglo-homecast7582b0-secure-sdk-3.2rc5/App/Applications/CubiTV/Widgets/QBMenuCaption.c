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

#include "QBMenuCaption.h"

#include <libintl.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/effect.h>
#include <CUIT/Core/event.h>
#include <SWL/icon.h>
#include <SWL/fade.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Menus/menuchoice.h>
#include <Windows/mainmenu.h>
#include <QBContentManager/QBContentSearch.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentCategoryListener.h>
#include <Widgets/QBMenuCaptionGetter.h>
#include <XMB2/XMBMenuBar.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBDataModel.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <QBDataModel3/QBTreeModelListener.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <QBMenu/QBMainMenuInterface.h>
#include <QBMenu/QBMenu.h>
#include <main.h>


struct QBMenuCaption_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvGenericObject parentPath;
    SvGenericObject node;

    SvWidget widget;
    SvWidget label;
    SvWidget icon;
    SvRID emptyRID;

    double iconFadeDuration;
    SvEffectId iconFadeId;

    SvWeakList listeners;
};
typedef struct QBMenuCaption_t *QBMenuCaption;

SvLocal void
QBMenuCaptionChanged(void *self_, SvWidget label)
{
    QBMenuCaption self = self_;
    if (!self->listeners) {
        return;
    }

    SvIterator iter = SvWeakListIterator(self->listeners);
    SvGenericObject listener;
    while ((listener = SvIteratorGetNext(&iter))) {
        SvInvokeInterface(QBMenuCaptionListener, listener, changed);
    }
}

SvLocal void
QBMenuCaptionSetIcon(QBMenuCaption self, SvString iconURI)
{
    if (self->iconFadeId) {
        svAppCancelEffect(self->widget->app, self->iconFadeId, SV_EFFECT_ABANDON);
        self->iconFadeId = 0;
    }

    if (!iconURI) {
        svIconSwitch(self->icon, 0, 0, self->iconFadeDuration);
        if (self->icon->tree_alpha != ALPHA_TRANSPARENT) {
            if (self->iconFadeDuration <= 0.0) {
                self->icon->tree_alpha = ALPHA_TRANSPARENT;
            } else {
                SvEffect fade = svEffectFadeNew(self->icon, ALPHA_TRANSPARENT, true, self->iconFadeDuration, SV_EFFECT_FADE_LOGARITHM);
                svEffectSetNotificationTarget(fade, self->widget);
                self->iconFadeId = svAppRegisterEffect(self->widget->app, fade);
            }
        }
    } else {
        svIconSetBitmapFromURI(self->icon, 1, SvStringCString(iconURI));
        svIconSwitch(self->icon, 1, 0, self->iconFadeDuration);
        if (self->icon->tree_alpha != ALPHA_SOLID) {
            if (self->iconFadeDuration <= 0.0) {
                self->icon->tree_alpha = ALPHA_SOLID;
            } else {
                SvEffect fade = svEffectFadeNew(self->icon, ALPHA_SOLID, true, self->iconFadeDuration, SV_EFFECT_FADE_LOGARITHM);
                svEffectSetNotificationTarget(fade, self->widget);
                self->iconFadeId = svAppRegisterEffect(self->widget->app, fade);
            }
        }
    }

    QBMenuCaptionChanged(self, self->label);
}

SvLocal SvString QBMenuCaptionCreateCaptionFromQBContentSearch(QBContentSearch node)
{
    QBContentCategoryLoadingState loadingState = QBContentCategoryGetLoadingState((QBContentCategory) node);
    int results = QBContentCategoryGetLength((QBContentCategory) node) - QBContentCategoryGetStaticCount((QBContentCategory) node);
    if ((loadingState == QBContentCategoryLoadingState_active) && !results) {
        return SvStringCreate(gettext("Search in progress..."), NULL);
    }

    return SvStringCreateWithFormat(ngettext("Search: %d result", "Search: %d results", results), results);
}

SvLocal SvString QBMenuCaptionCreateTranslatedCaptionFromSource(SvString srcCaption)
{
    const char* captionStr = SvStringCString(srcCaption);
    const char* translatedCaptionStr = gettext(captionStr);

    if (translatedCaptionStr != captionStr) {
        return SvStringCreate(translatedCaptionStr, NULL);
    }

    return SVRETAIN(srcCaption);
}

SvLocal SvString QBMenuCaptionGetCaptionFromPath(QBMenuCaption self, SvObject node, SvObject parentPath)
{
    if (!node)
        return NULL;

    SvObject controllerPath = SvObjectCopy(parentPath, NULL);
    if (SvInvokeInterface(QBTreePath, controllerPath, getLength) > 2)
        SvInvokeInterface(QBTreePath, controllerPath, truncate, -1);
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvObject handler = SvInvokeInterface(QBMenu, menuBar->prv, getItemControllerForPath, controllerPath, NULL);
    SVRELEASE(controllerPath);
    if (handler && SvObjectIsImplementationOf(handler, QBMenuCaptionGetter_getInterface()))
        return SvInvokeInterface(QBMenuCaptionGetter, handler, getMenuCaptionForPath, parentPath, node);

    return NULL;
}

SvLocal void QBMenuCaptionUpdate(QBMenuCaption self, SvObject node_, SvObject parentPath)
{
    SvString iconURI = NULL, srcCaption = NULL;
    SvString caption = NULL; // be careful when you set this value:
                             // if value was set by get method you should retain it
                             // in value was created - you should not retain it
    QBActiveTreeNode treeNode;

    caption = QBMenuCaptionGetCaptionFromPath(self, node_, parentPath);
    SVTESTRETAIN(caption);

    if (!caption) {
        if (node_ && SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
            treeNode = (QBActiveTreeNode) node_;
            caption = (SvString) QBActiveTreeNodeGetAttribute(treeNode, SVSTRING("windowCaption"));
            SVTESTRETAIN(caption);
            if (!caption)
                srcCaption = (SvString) QBActiveTreeNodeGetAttribute(treeNode, SVSTRING("caption"));
            iconURI = (SvString) QBActiveTreeNodeGetAttribute(treeNode, SVSTRING("serviceIcon"));
        } else if (node_ && SvObjectIsInstanceOf(node_, QBContentSearch_getType())) {
            caption = QBMenuCaptionCreateCaptionFromQBContentSearch((QBContentSearch) node_);
        } else if (node_ && SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
            QBContentCategory node = (QBContentCategory) node_;
            srcCaption = QBContentCategoryGetTitle(node);
            SvValue v = (SvValue) QBContentCategoryGetAttribute(node, SVSTRING("iconURI"));
            iconURI = (v && SvValueIsString(v)) ? SvValueGetString(v) : NULL;
        } else if ((treeNode = QBActiveTreeFindNodeByPath(self->appGlobals->menuTree, parentPath))) {
            srcCaption = (SvString) QBActiveTreeNodeGetAttribute(treeNode, SVSTRING("caption"));
        }
    } else if (node_ && SvObjectIsInstanceOf(node_, QBContentCategory_getType())) {
        QBContentCategory node = (QBContentCategory) node_;
        srcCaption = QBContentCategoryGetTitle(node);
        SvValue v = (SvValue) QBContentCategoryGetAttribute(node, SVSTRING("iconURI"));
        iconURI = (v && SvValueIsString(v)) ? SvValueGetString(v) : NULL;
    } else if ((treeNode = QBActiveTreeFindNodeByPath(self->appGlobals->menuTree, parentPath))) {
        srcCaption = (SvString) QBActiveTreeNodeGetAttribute(treeNode, SVSTRING("caption"));
    }

    if (!caption && srcCaption) {
        caption = QBMenuCaptionCreateTranslatedCaptionFromSource(srcCaption);
    }

    QBAsyncLabelSetText(self->label, caption ? caption : SVSTRING(""));
    QBMenuCaptionSetIcon(self, iconURI);

    SVTESTRELEASE(caption);
}

SvLocal void
QBMenuCaptionSearchStateChanged(SvGenericObject self_, QBContentCategory category, QBContentCategoryLoadingState prevState, QBContentCategoryLoadingState currState)
{
    QBMenuCaption self = (QBMenuCaption) self_;
    QBMenuCaptionUpdate(self, self->node, self->parentPath);
}

SvLocal void
QBMenuCaptionMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBMenuCaption self = (QBMenuCaption) self_;
    SvGenericObject path = NULL;

    if (SvInvokeInterface(QBTreePath, nodePath_, getLength) > 1) {
        path = SvObjectCopy(nodePath_, NULL);
        SvInvokeInterface(QBTreePath, path, truncate, -1);
        node_ = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getNode, path);
    } else {
        path = SVRETAIN(nodePath_);
    }

    QBMenuCaptionUpdate(self, node_, path);

    if (SvObjectIsInstanceOf(self->node, QBContentSearch_getType())) {
        QBContentCategoryRemoveListener((QBContentCategory) self->node, self_, NULL);
    }

    SVTESTRELEASE(self->parentPath);
    SVTESTRELEASE(self->node);
    self->parentPath = path;
    self->node = SVTESTRETAIN(node_);


    if (SvObjectIsInstanceOf(self->node, QBContentSearch_getType())) {
        QBContentCategoryAddListener((QBContentCategory) self->node, self_, NULL);
    }
}

SvLocal void QBMenuCaptionTreeIgnore(SvGenericObject self_, SvGenericObject path, size_t first, size_t count)
{
}

SvLocal void QBMenuCaptionTreeChanged(SvGenericObject self_, SvGenericObject path, size_t first, size_t count)
{
    QBMenuCaption self = (QBMenuCaption) self_;
    if (!self->parentPath)
        return;

    if (SvObjectEquals(path, self->parentPath))
        QBMenuCaptionUpdate(self, self->node, self->parentPath);
}

SvLocal void QBMenuCaption__dtor__(void *self_)
{
    QBMenuCaption self = self_;
    SVTESTRELEASE(self->parentPath);
    SVTESTRELEASE(self->node);
    SVRELEASE(self->listeners);
}

SvLocal SvType
QBMenuCaption_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMenuCaption__dtor__
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBMenuCaptionMenuChoosen
    };

    static const struct QBTreeModelListener_t treeMethods = {
        .nodesAdded = QBMenuCaptionTreeIgnore,
        .nodesRemoved = QBMenuCaptionTreeIgnore,
        .nodesChanged = QBMenuCaptionTreeChanged
    };

    static const struct QBContentCategoryListener_ searchMethods = {
        .loadingStateChanged = QBMenuCaptionSearchStateChanged
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMenuCaption",
                            sizeof(struct QBMenuCaption_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            QBTreeModelListener_getInterface(), &treeMethods,
                            QBContentCategoryListener_getInterface(), &searchMethods,
                            NULL);
    }

    return type;
}

SvLocal void
QBMenuCaptionEffectEventHandler(SvWidget w, SvEffectEvent ev)
{
    QBMenuCaption self = w->prv;
    if (ev->id == self->iconFadeId) {
        self->iconFadeId = 0;
        return;
    }
}

SvLocal void
QBMenuCaptionCleanup(SvApplication app, void *self_)
{
    SVRELEASE(self_);
}

void QBMenuCaptionRegister(SvWidget caption, AppGlobals appGlobals)
{
    QBMenuCaption self = caption->prv;
    self->appGlobals = appGlobals;
    SvInvokeInterface(QBMainMenu, self->appGlobals->main, addGlobalHandler, (SvObject) self);
    SvInvokeInterface(QBDataModel, appGlobals->menuTree, addListener, (SvGenericObject) self, NULL);

    // update caption
    SvWidget menuBar = SvInvokeInterface(QBMainMenu, self->appGlobals->main, getMenu);
    SvGenericObject path = SvInvokeInterface(QBMenu, menuBar->prv, getPosition);

    SvGenericObject parentPath = NULL;
    if (SvInvokeInterface(QBTreePath, path, getLength) > 1) {
        parentPath = SvObjectCopy(path, NULL );
        SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
    } else {
        parentPath = SVRETAIN(path);
    }

    SvGenericObject node = SvInvokeInterface(QBTreeModel, appGlobals->menuTree, getNode, parentPath);
    QBMenuCaptionUpdate(self, node, parentPath);
    SVRELEASE(parentPath);

    self->listeners = SvWeakListCreate(NULL);
}

SvWidget
QBMenuCaptionNew(SvApplication app, char const *widgetName, QBTextRenderer renderer)
{
    const size_t nameBufferSize = strlen(widgetName) +7;
    char nameBuffer[nameBufferSize];
    QBMenuCaption self;

    self = (QBMenuCaption) SvTypeAllocateInstance(QBMenuCaption_getType(), NULL);
    self->widget = svSettingsWidgetCreate(app, widgetName);
    self->widget->prv = self;
    svWidgetSetEffectEventHandler(self->widget, QBMenuCaptionEffectEventHandler);
    self->widget->clean = QBMenuCaptionCleanup;

    snprintf(nameBuffer, nameBufferSize, "%s.label", widgetName);
    self->label = QBAsyncLabelNew(app, nameBuffer, renderer);
    svSettingsWidgetAttach(self->widget, self->label, nameBuffer, 1);

    QBAsyncLabelSetCallback(self->label, QBMenuCaptionChanged, self);

    snprintf(nameBuffer, nameBufferSize, "%s.icon", widgetName);
    if (svSettingsIsWidgetDefined(nameBuffer)) {
        self->icon = svIconNew(app, nameBuffer);
        svSettingsWidgetAttach(self->widget, self->icon, nameBuffer, 1);
        self->iconFadeDuration = svSettingsGetDouble(nameBuffer, "fadeDuration", 0.0);
        self->emptyRID = svSettingsGetResourceID(nameBuffer, "defaultBitmap");
        if (self->emptyRID != SV_RID_INVALID) {
            svIconSetBitmapFromRID(self->icon, 0, self->emptyRID);
        }
    }

    return self->widget;
}

unsigned int QBMenuCaptionGetLabelWidth(SvWidget w)
{
    QBMenuCaption self = w->prv;
    unsigned int labelWidth = QBAsyncLabelGetWidth(self->label);

    return labelWidth;
}

unsigned int QBMenuCaptionGetIconWidth(SvWidget w)
{
    QBMenuCaption self = w->prv;
    unsigned int iconWidth = 0;

    if (self->icon)
        iconWidth = self->icon->width;

    return iconWidth;
}

unsigned int QBMenuCaptionGetWidth(SvWidget w)
{
    unsigned int width = QBMenuCaptionGetLabelWidth(w) +
                         QBMenuCaptionGetIconWidth(w);

    return width;
}

void QBMenuCaptionAddListener(SvWidget caption, SvGenericObject listener)
{
    QBMenuCaption self = caption->prv;
    SvWeakListPushFront(self->listeners, listener, NULL);
}
void QBMenuCaptionRemoveListener(SvWidget caption, SvGenericObject listener)
{
    QBMenuCaption self = caption->prv;
    SvWeakListRemoveObject(self->listeners, listener);
}

SvInterface QBMenuCaptionListener_getInterface(void)
{
    static SvInterface interface = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBMenuCaptionListener", sizeof(struct QBMenuCaptionListener_),
                                 NULL, &interface, NULL);
    }
    return interface;
}
