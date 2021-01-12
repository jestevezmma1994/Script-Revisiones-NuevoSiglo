/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBConfigurationMenuDefaultItemController.h"

#include <libintl.h>

#include <SvCore/SvCommonDefs.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBTypes.h>
#include <QBDataModel3/QBActiveTreeNode.h>
#include <main.h>
#include <Widgets/QBXMBItemConstructor.h>
#include "QBConfigurationMenuUtils.h"

struct QBConfigurationMenuDefaultItemController_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBActiveTree tree;

    SvRID tickMarkOnRID;
    SvRID tickMarkOffRID;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBXMBItemConstructor itemConstructor;
    unsigned int settingsCtx;

    SvRID iconRID, searchIconRID;
};

SvLocal void
QBConfigurationMenuDefaultItemControllerDestroy(void *self_)
{
    QBConfigurationMenuDefaultItemController self = self_;
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);

    QBXMBItemConstructorDestroy(self->itemConstructor);

    SVRELEASE(self->tree);
}

SvLocal SvWidget
QBConfigurationMenuDefaultItemControllerCreateItem(SvObject self_,
                                                   SvObject node_,
                                                   SvObject path,
                                                   SvApplication app,
                                                   XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBConfigurationMenuDefaultItemController self = (QBConfigurationMenuDefaultItemController) self_;
    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString parentId = parent ? QBActiveTreeNodeGetID(parent) : NULL;

    QBXMBItem item = QBXMBItemCreate();

    QBConfigurationMenuTickState tickState = QBConfigurationMenuCheckTickState(node);
    if (tickState != QBConfigurationMenuTickState__Hidden) {
        if (tickState == QBConfigurationMenuTickState__On)
            item->iconRID = self->tickMarkOnRID;
        else
            item->iconRID = self->tickMarkOffRID;
    }

    item->loadingRID = self->iconRID;

    if (item->iconRID == SV_RID_INVALID) {
        SvString iconPath = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("icon"));
        item->iconURI.URI = SVTESTRETAIN(iconPath);
        item->iconURI.isStatic = true;
    }

    item->caption = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    if (parentId && SvStringEqualToCString(parentId, "MLNG")) {
        SVRETAIN(item->caption);
    } else {
        const char *captionStr, *translatedCaptionStr;
        captionStr = SvStringCString(item->caption);
        translatedCaptionStr = gettext(captionStr);
        if (translatedCaptionStr != captionStr)
            item->caption = SvStringCreate(translatedCaptionStr, NULL);
        else
            SVRETAIN(item->caption);
    }

    const char *subcaptionStr, *translatedSubcaptionStr;
    item->subcaption = (SvString) (QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption")));
    if (item->subcaption) {
        subcaptionStr = SvStringCString(item->subcaption);
        translatedSubcaptionStr = gettext(subcaptionStr);
        if (translatedSubcaptionStr != subcaptionStr)
            item->subcaption = SvStringCreate(translatedSubcaptionStr, NULL);
        else
            SVRETAIN(item->subcaption);
    }

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);
    return ret;
}

SvLocal void
QBConfigurationMenuDefaultItemControllerSetItemState(SvObject self_,
                                                     SvWidget item_,
                                                     XMBMenuState state,
                                                     bool isFocused)
{
    QBConfigurationMenuDefaultItemController self = (QBConfigurationMenuDefaultItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal SvType
QBConfigurationMenuDefaultItemController_getType(void)
{
    static SvType type = NULL;
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConfigurationMenuDefaultItemControllerDestroy
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBConfigurationMenuDefaultItemControllerCreateItem,
        .setItemState = QBConfigurationMenuDefaultItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConfigurationMenuDefaultItemController",
                            sizeof(struct QBConfigurationMenuDefaultItemController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }

    return type;
}

QBConfigurationMenuDefaultItemController
QBConfigurationMenuDefaultItemControllerCreate(AppGlobals appGlobals, QBActiveTree tree, SvErrorInfo *errorOut)
{
    QBConfigurationMenuDefaultItemController self = (QBConfigurationMenuDefaultItemController)
        SvTypeAllocateInstance(QBConfigurationMenuDefaultItemController_getType(), NULL);

    self->appGlobals = appGlobals;
    self->tree = SVRETAIN(tree);
    self->settingsCtx = svSettingsSaveContext();

    svSettingsPushComponent("Configuration.settings");

    self->searchIconRID = svSettingsGetResourceID("MenuItem", "searchIcon");
    self->iconRID = svSettingsGetResourceID("MenuItem", "icon");
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    self->tickMarkOnRID = svSettingsGetResourceID("MenuItem", "tickMarkOn");
    self->tickMarkOffRID = svSettingsGetResourceID("MenuItem", "tickMarkOff");


    svSettingsPopComponent();

    return self;
}
