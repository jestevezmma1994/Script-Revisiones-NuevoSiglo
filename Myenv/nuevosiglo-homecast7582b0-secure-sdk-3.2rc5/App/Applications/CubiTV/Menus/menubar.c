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

#include "menubar.h"
#include "menubar_internal.h"

#include <libintl.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvData.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvJSON/SvJSONParse.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvFont.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBLocator.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <Utils/loadFile.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBMenuBar.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBIcon.h>
#include <Logic/GUILogic.h>
#include <QBMenu/QBMenu.h>
#include <main.h>
#include <SvQuirks/SvRuntimePrefix.h>

struct QBMenuBarItemController_t {
    struct SvObject_ super_;
    int iconWidth;
    int iconHeight;
    SvColor textColor;
    float iconScale;
    double transitionTime;
    double xCenterRatioSmall;
    double yCenterRatioSmall;
    double xCenterRatioBig;
    double yCenterRatioBig;
    int orgSizeSmallWidth, orgSizeSmallHeight;
    int orgSizeBigWidth, orgSizeBigHeight;
    int clippedSmallWidth, clippedSmallHeight;
    int clippedBigWidth, clippedBigHeight;
    int offsetXSmall, offsetYSmall;
    int offsetXBig, offsetYBig;
    int bigFontSize;
    int smallFontSize;
    int descWidth;
    int descHeight;
    SvBitmap orgBigIcon;
    SvFont font;
    struct QBMenuBarItemControllerCallbacks_s callbacks;
    void *callbackData;
    SvHashTable iconCropInfo;
};
typedef struct QBMenuBarItemController_t* QBMenuBarItemController;

struct QBMenuItemController_t {
    struct SvObject_ super_;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    SvRID iconRID;

    QBXMBItemConstructor itemConstructor;
    QBTextRenderer textRenderer;

};
typedef struct QBMenuItemController_t* QBMenuItemController;

struct QBIconCropInfo_t {
    struct SvObject_ super_;
    int orginalWidth;
    int orginalHeight;
    int clippedWidth;
    int clippedHeight;
    int offsetX;
    int offsetY;
};
typedef struct QBIconCropInfo_t* QBIconCropInfo;

SvLocal SvType QBIconCropInfo_getType(void)
{
    static SvType type = NULL;
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBIconCropInfo",
                            sizeof(struct QBIconCropInfo_t),
                            SvObject_getType(),
                            &type,
                            NULL);
    }
    return type;
}

SvLocal SvObject QBIconCropInfoCreateFromHashTable(SvObject helper_, SvString className, SvHashTable desc, SvErrorInfo *errorOut)
{
    QBIconCropInfo cropInfo = (QBIconCropInfo) SvTypeAllocateInstance(QBIconCropInfo_getType(), NULL);

    SvValue orginalWidth = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("orginalWidth"));
    SvValue orginalHeight = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("orginalHeight"));
    SvValue clippedWidth = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("clippedWidth"));
    SvValue clippedHeight = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("clippedHeight"));
    SvValue offsetX = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("offsetX"));
    SvValue offsetY = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("offsetY"));

    if (!SvValueIsInteger(orginalWidth) || !SvValueIsInteger(orginalHeight) ||
        !SvValueIsInteger(clippedWidth) || !SvValueIsInteger(clippedHeight) ||
        !SvValueIsInteger(offsetX) || !SvValueIsInteger(offsetY)) {
        SvLogError("iconcropinfo object cannot be parsed");
        return NULL;
    }

    cropInfo->orginalWidth = SvValueGetInteger(orginalWidth);
    cropInfo->orginalHeight = SvValueGetInteger(orginalHeight);
    cropInfo->clippedWidth = SvValueGetInteger(clippedWidth);
    cropInfo->clippedHeight = SvValueGetInteger(clippedHeight);
    cropInfo->offsetX = SvValueGetInteger(offsetX);
    cropInfo->offsetY = SvValueGetInteger(offsetY);

    return (SvObject) cropInfo;
}

SvLocal SvHashTable QBMenuBarLoadCropInfoTable(SvApplication app)
{
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBResourceManager resourceManager = (QBResourceManager) QBServiceRegistryGetService(registry, SVSTRING("ResourceManager"));
    SvRBLocator locator = QBResourceManagerGetResourceLocator(resourceManager);

    char *cropInfoPath = SvRBLocatorFindFile(locator, "icon/iconcropinfo.json");
    if (!cropInfoPath)
        return NULL;
    SvData data = SvDataCreateFromFile(cropInfoPath, -1, NULL);
    free(cropInfoPath);
    if (!data) {
        SvLogError("unable to open icon crop info file");
        return NULL;
    }

    SvHashTable info = (SvHashTable) SvJSONParseData(SvDataGetBytes(data), SvDataGetLength(data), true, NULL);
    SVRELEASE(data);
    if (!info) {
        SvLogError("unable to parse icon crop info file");
    }
    return info;
}

SvLocal SvWidget QBMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBMenuItemController self = (QBMenuItemController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    QBXMBItem item = QBXMBItemCreate();
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    item->loadingRID = self->iconRID;

    SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
    item->iconURI.URI = SVTESTRETAIN(iconPath);
    item->iconURI.isStatic = true;

    SvString caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    const char* captionStr = SvStringCString(caption);
    const char* translatedCaptionStr = *captionStr ? gettext(captionStr) : "";
    if (translatedCaptionStr != captionStr)
        caption = SvStringCreate(translatedCaptionStr, NULL);
    else
        SVRETAIN(caption);
    item->caption = caption;

    caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption"));
    if (caption && SvStringLength(caption) != 0) {
        captionStr = SvStringCString(caption);
        translatedCaptionStr = gettext(captionStr);
        if (translatedCaptionStr != captionStr)
            caption = SvStringCreate(translatedCaptionStr, NULL);
        else
            SVRETAIN(caption);
        item->subcaption = caption;
    }
    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void QBMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBMenuItemController self = (QBMenuItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBMenuItemControllerDestroy(void *self_)
{
    QBMenuItemController self = self_;

    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    QBXMBItemConstructorDestroy(self->itemConstructor);
}

SvLocal SvType QBMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMenuItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t methods = {
        .createItem = QBMenuItemControllerCreateItem,
        .setItemState = QBMenuItemControllerSetItemState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMenuItemController",
                            sizeof(struct QBMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvLocal SvGenericObject QBMenuItemControllerCreate(QBTextRenderer renderer)
{
    QBMenuItemController self = (QBMenuItemController) SvTypeAllocateInstance(QBMenuItemController_getType(), NULL);

    self->iconRID = svSettingsGetResourceID("MenuItem", "icon");

    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", renderer);
    self->textRenderer = renderer;

    return (SvGenericObject) self;
}

SvWidget QBMenuBarItemControllerCreateIcon(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBMenuBarItemController self = (QBMenuBarItemController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    SvString iconBigPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
    SvString iconSmallPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("iconSmall"));
    struct XMBIconBitmap_ bmps[2] = {
        [0] = { .bmp = NULL },
        [1] = { .bmp = NULL }
    };

    SvBitmap clippedBig = NULL;
    if (iconBigPath) {
        Sv2DRect clippingRect;
        QBIconCropInfo crop = (QBIconCropInfo)SvHashTableFind(self->iconCropInfo, (SvObject)iconBigPath);
        if (crop) {
            clippingRect = Sv2DRectCreate(crop->offsetX, crop->offsetY,
                                          crop->clippedWidth, crop->clippedHeight);
            bmps[1].offsetX = crop->offsetX;
            bmps[1].offsetY = crop->offsetY;
            bmps[1].orgWidth = crop->orginalWidth;
            bmps[1].orgHeight = crop->orginalHeight;
        } else {
            SvLogWarning("No clipping rectangle defined for: %s", SvStringGetCString(iconBigPath));
            clippingRect = Sv2DRectCreate(self->offsetXBig, self->offsetYBig, self->clippedBigWidth, self->clippedBigHeight);
            bmps[1].offsetX = self->offsetXBig;
            bmps[1].offsetY = self->offsetYBig;
            bmps[1].orgWidth = self->orgSizeBigWidth;
            bmps[1].orgHeight = self->orgSizeBigHeight;
        }
        bmps[1].bmp = clippedBig = createClippedPNG(SvStringCString(iconBigPath), SvBitmapType_static, &clippingRect);
    }
    if (!bmps[1].bmp)
        bmps[1].bmp = self->orgBigIcon;
    bmps[1].inactiveScale = self->iconScale;
    bmps[1].activeScale = 1.0f;
    bmps[1].xCenterRatio = self->xCenterRatioBig;
    bmps[1].yCenterRatio = self->yCenterRatioBig;

    SvBitmap clippedSmall = NULL;
    if (iconSmallPath) {
        Sv2DRect clippingRect;
        QBIconCropInfo crop = (QBIconCropInfo)SvHashTableFind(self->iconCropInfo, (SvObject)iconSmallPath);
        if (crop) {
            clippingRect = Sv2DRectCreate(crop->offsetX, crop->offsetY,
                                          crop->clippedWidth, crop->clippedHeight);
            bmps[0].offsetX = crop->offsetX;
            bmps[0].offsetY = crop->offsetY;
            bmps[0].orgWidth = crop->orginalWidth;
            bmps[0].orgHeight = crop->orginalHeight;
        } else {
            SvLogWarning("No clipping rectangle defined for: %s", SvStringGetCString(iconSmallPath));
            clippingRect = Sv2DRectCreate(self->offsetXSmall, self->offsetYSmall, self->clippedSmallWidth, self->clippedSmallHeight);
            bmps[0].offsetX = self->offsetXSmall;
            bmps[0].offsetY = self->offsetYSmall;
            bmps[0].orgWidth = self->orgSizeSmallWidth;
            bmps[0].orgHeight = self->orgSizeSmallHeight;
        }
        bmps[0].bmp = clippedSmall = createClippedPNG(SvStringCString(iconSmallPath), SvBitmapType_static, &clippingRect);
    }
    if (bmps[0].bmp) {
        bmps[0].inactiveScale = 1.0f / self->iconScale;
        bmps[0].activeScale = 1.0f;
    } else {
        bmps[0].bmp = bmps[1].bmp;
        bmps[0].inactiveScale = 1.0f;
        bmps[0].activeScale = self->iconScale;
    }

    bmps[0].xCenterRatio = self->xCenterRatioSmall;
    bmps[0].yCenterRatio = self->yCenterRatioSmall;

    SvWidget icon = XMBIconNew(app, self->iconWidth, self->iconHeight, bmps, NULL);
    XMBIconSelectBitmap(icon, 0, 0.0, NULL);

    SVTESTRELEASE(clippedBig);
    SVTESTRELEASE(clippedSmall);

    return icon;
}

SvLocal SvWidget QBMenuBarItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBMenuBarItemController self = (QBMenuBarItemController) self_;

    if (self->callbacks.createItem) {
        return self->callbacks.createItem(self->callbackData, self_, node_, path, app, initialState);
    } else {
        return QBMenuBarItemControllerCreateIcon(self_, node_, path, app, initialState);
    }
}

void QBMenuBarItemControllerSetIconState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBMenuBarItemController self = (QBMenuBarItemController) self_;
    int currentBitmap = XMBIconGetSelectedBitmap(item_);
    int wantedBitmap = (state == XMBMenuState_normal) ? (isFocused ? 2 : 1) : 0;
    if (currentBitmap != wantedBitmap) {
        XMBIconSelectBitmap(item_, wantedBitmap, self->transitionTime, NULL);
    }
}

SvLocal void QBMenuBarItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{

    QBMenuBarItemController self = (QBMenuBarItemController) self_;

    if (self->callbacks.setItemState) {
        self->callbacks.setItemState(self->callbackData, self_, item_, state, isFocused);
    } else {
        QBMenuBarItemControllerSetIconState(self_, item_, state, isFocused);
    }
}

SvLocal void QBMenuBarItemControllerDestroy(void *self_)
{
    QBMenuBarItemController self = self_;
    SVRELEASE(self->orgBigIcon);
    SVRELEASE(self->font);
    SVRELEASE(self->iconCropInfo);
}

SvLocal SvType QBMenuBarItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMenuBarItemControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t methods = {
        .createItem = QBMenuBarItemControllerCreateItem,
        .setItemState = QBMenuBarItemControllerSetItemState,
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMenuBarItemController",
                            sizeof(struct QBMenuBarItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvLocal SvGenericObject QBMenuBarItemControllerCreate(SvApplication app)
{
    QBMenuBarItemController self = (QBMenuBarItemController) SvTypeAllocateInstance(QBMenuBarItemController_getType(), NULL);

    self->iconWidth = svSettingsGetInteger("MenuBarItem", "width", 0);
    self->iconHeight = svSettingsGetInteger("MenuBarItem", "height", 0);
    self->textColor = svSettingsGetColor("MenuBarItem", "textColor", 0);
    self->iconScale = svSettingsGetDouble("MenuBarItem", "scale", 1.0);
    self->orgBigIcon = SVRETAIN(svSettingsGetBitmap("MenuBarItem", "icon"));
    self->bigFontSize = svSettingsGetInteger("MenuBarItem", "bigFontSize", 0);
    self->smallFontSize = svSettingsGetInteger("MenuBarItem", "smallFontSize", 0);
    self->descWidth = svSettingsGetInteger("MenuBarItem", "descWidth", 0);
    self->descHeight = svSettingsGetInteger("MenuBarItem", "descHeight", 0);
    self->font = svSettingsCreateFont("MenuBarItem", "font");
    self->xCenterRatioSmall = svSettingsGetDouble("MenuBarItem", "xCenterRatioSmall", 0.5);
    self->yCenterRatioSmall = svSettingsGetDouble("MenuBarItem", "yCenterRatioSmall", 0.5);
    self->xCenterRatioBig = svSettingsGetDouble("MenuBarItem", "xCenterRatioBig", self->xCenterRatioSmall);
    self->yCenterRatioBig = svSettingsGetDouble("MenuBarItem", "yCenterRatioBig", self->yCenterRatioSmall);
    self->orgSizeSmallWidth = svSettingsGetInteger("MenuBarItem", "orgSizeSmallWidth", 192);
    self->orgSizeSmallHeight= svSettingsGetInteger("MenuBarItem", "orgSizeSmallHeight", 192);
    self->orgSizeBigWidth = svSettingsGetInteger("MenuBarItem", "orgSizeBigWidth", 192);
    self->orgSizeBigHeight = svSettingsGetInteger("MenuBarItem", "orgSizeBigHeight", 192);
    self->clippedSmallWidth = svSettingsGetInteger("MenuBarItem", "clippedSmallWidth", 192);
    self->clippedSmallHeight = svSettingsGetInteger("MenuBarItem", "clippedSmallHeight", 192);
    self->clippedBigWidth = svSettingsGetInteger("MenuBarItem", "clippedBigWidth", 192);
    self->clippedBigHeight = svSettingsGetInteger("MenuBarItem", "clippedBigHeight", 192);
    self->offsetXSmall = svSettingsGetInteger("MenuBarItem", "offsetXSmall", 0);
    self->offsetYSmall = svSettingsGetInteger("MenuBarItem", "offsetYSmall", 0);
    self->offsetXBig = svSettingsGetInteger("MenuBarItem", "offsetXBig", 0);
    self->offsetYBig = svSettingsGetInteger("MenuBarItem", "offsetYBig", 0);
    self->transitionTime = svSettingsGetDouble("MenuBarItem", "transitionTime", 0.25);
    self->iconCropInfo = QBMenuBarLoadCropInfoTable(app);

    return (SvGenericObject) self;
}

SvWidget QBMenuBarCreate(SvApplication app, AppGlobals appGlobals)
{
    SvWidget menuBar = XMBMenuBarNew(app, "menuBar", NULL);

    SvJSONClass helper = SvJSONClassCreate(NULL, QBIconCropInfoCreateFromHashTable, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBIconCropInfo"), (SvObject) helper, NULL);
    SVRELEASE(helper);

    SvGenericObject barController = QBMenuBarItemControllerCreate(appGlobals->res);
    QBGUILogicMenuBarItemControllerRegisterCallbacks(appGlobals->guiLogic, barController);
    SvGenericObject controller = QBMenuItemControllerCreate(appGlobals->textRenderer);
    SvInvokeInterface(QBMenu, menuBar->prv, connectToDataSource,
                      (SvObject) appGlobals->menuTree, (SvObject) barController, NULL,
                      (SvObject) controller, NULL);
    SVRELEASE(barController);
    SVRELEASE(controller);

    svWidgetSetFocusable(menuBar, true);

    return menuBar;
}

void QBMenuBarItemControllerSetCallbacks(SvGenericObject self_, void *callbackData, QBMenuBarItemControllerCallbacks callbacks)
{
    QBMenuBarItemController self = (QBMenuBarItemController) self_;

    self->callbackData = callbackData;
    self->callbacks = *callbacks;
}
