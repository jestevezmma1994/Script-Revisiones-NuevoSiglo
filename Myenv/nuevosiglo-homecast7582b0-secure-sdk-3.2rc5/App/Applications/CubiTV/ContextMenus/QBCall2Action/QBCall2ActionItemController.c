/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <ContextMenus/QBCall2Action/QBCall2ActionItemController.h>
#include <ContextMenus/QBCall2Action/log.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <main.h>
#include <QBCall2ActionClient/QBCall2ActionTreeNode.h>
#include <SWL/QBFrame.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvType.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <libintl.h>

struct QBCall2ActionItemController_ {
    struct SvObject_ super;
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focusConstructor;
    QBFrameConstructor* disabledFocusConstructor;
    SvRID iconRID;
    QBCall2ActionTree tree;
};

typedef struct QBCall2ActionItemController_ *QBCall2ActionItemController;

SvLocal void
QBCall2ActionItemControllerDestroy(void *self_)
{
    QBCall2ActionItemController self = self_;
    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->focusConstructor);
    SVRELEASE(self->tree);
    SVTESTRELEASE(self->disabledFocusConstructor);
}

SvLocal SvWidget
QBCall2ActionItemControllerCreateItem(SvObject self_, SvObject node_, SvObject path_, SvApplication app,
                                      XMBMenuState initialState)
{
    log_info();
    QBCall2ActionItemController self = (QBCall2ActionItemController) self_;
    if (!SvObjectIsInstanceOf(node_, QBBasicPaneItem_getType()))
        return NULL;

    QBBasicPaneItem item = (QBBasicPaneItem) node_;
    QBXMBItem itemCtor = QBXMBItemCreate();
    itemCtor->caption = SVRETAIN(item->caption);

    SVAUTOSTRINGVALUE(idVal, item->id);
    QBCall2ActionTreeNode action = (QBCall2ActionTreeNode) QBCall2ActionTreeFindNodeById(self->tree, idVal, NULL, NULL);

    SvValue value = NULL;
    SvString subcaption = NULL;
    if (item->subcaption) {
        itemCtor->subcaption = SVTESTRETAIN(item->subcaption);
    } else {
        value = QBCall2ActionTreeNodeGetAttrValue(action, "subcaption", NULL);
        if (value) {
            subcaption = SvValueGetString(value);
            itemCtor->subcaption = SvStringCreate(gettext(SvStringCString(subcaption)), NULL);
        }
    }
    itemCtor->focus = SVRETAIN(self->focusConstructor);
    itemCtor->disabledFocus = SVTESTRETAIN(self->disabledFocusConstructor);
    itemCtor->disabled = item->itemDisabled;

    value = QBCall2ActionTreeNodeGetAttrValue(action, "icon", NULL);
    if (value && SvValueIsString(value)) {
        itemCtor->loadingRID = self->iconRID;
        itemCtor->iconURI.URI = SVRETAIN(SvValueGetString(value));
    }

    SvWidget ret = QBXMBItemConstructorCreateItem(self->itemConstructor, itemCtor, app, initialState);
    SVRELEASE(itemCtor);

    return ret;
}

SvLocal void
QBCall2ActionItemControllerSetItemState(SvObject self_, SvWidget item, XMBMenuState menuState, bool isFocused)
{
    log_info();
    QBCall2ActionItemController self = (QBCall2ActionItemController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item, menuState, isFocused);
}


SvLocal SvType
QBCall2ActionItemController_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ vTable = {
        .destroy = QBCall2ActionItemControllerDestroy
    };

    static const struct XMBItemController_t methods = {
        .createItem   = QBCall2ActionItemControllerCreateItem,
        .setItemState = QBCall2ActionItemControllerSetItemState
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCall2ActionItemController",
                            sizeof(struct QBCall2ActionItemController_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vTable,
                            XMBItemController_getInterface(), &methods,
                            NULL);
    }

    return type;
}

SvObject
QBCall2ActionItemControllerCreate(QBCall2ActionTree tree, AppGlobals appGlobals, SvString widgetName)
{
    log_info();
    QBCall2ActionItemController self = (QBCall2ActionItemController)
                                       SvTypeAllocateInstance(QBCall2ActionItemController_getType(), NULL);
    char *itemName, *focusName, *disabledFocusName;
    asprintf(&itemName, "%s.Item", SvStringCString(widgetName));
    asprintf(&focusName, "%s.Focus", SvStringCString(widgetName));
    asprintf(&disabledFocusName, "%s.DisabledFocus", SvStringCString(widgetName));
    self->tree = SVRETAIN(tree);
    self->itemConstructor = QBXMBItemConstructorCreate(itemName, appGlobals->textRenderer);
    self->focusConstructor = QBFrameConstructorFromSM(focusName);
    self->iconRID = svSettingsGetResourceID(itemName, "icon");
    if (svSettingsIsWidgetDefined(disabledFocusName))
        self->disabledFocusConstructor = QBFrameConstructorFromSM(disabledFocusName);
    free(itemName);
    free(focusName);
    free(disabledFocusName);
    return (SvObject) self;
}
