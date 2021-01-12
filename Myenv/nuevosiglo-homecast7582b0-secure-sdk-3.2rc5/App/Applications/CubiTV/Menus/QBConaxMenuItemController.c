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

#include "QBConaxMenuItemController.h"

#include <libintl.h>
#include <settings.h>
#include <QBSmartcard2Interface.h>
#include <main.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>
#include <XMB2/XMBMenuController.h>
#include <XMB2/XMBVerticalMenu.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <Services/Conax/QBConaxMailManager.h>

#include <TranslationMerger.h>

SvLocal bool isBooleanAttributeTrue(QBActiveTreeNode node, SvString attr)
{
    SvValue obj = (SvValue) QBActiveTreeNodeGetAttribute(node, attr);
    if (obj) {
        if (SvObjectIsInstanceOf((SvObject) obj, SvValue_getType()) &&
            SvValueIsBoolean(obj)) {
            return SvValueGetBoolean(obj);
        } else {
            SvLogError("%s(): Invalid type (%s, expected bool) for attribute %s in ConaxMenu node.",
                       __func__, SvObjectGetTypeName((SvObject) obj), SvStringCString(attr));
        }
    }
    return false;
}

struct QBConaxMenuItemController_t {
    struct SvObject_ super_;

    AppGlobals appGlobals;

    SvBitmap icon;
    SvBitmap tickMarkOn;
    SvBitmap tickMarkOff;
    QBFrameConstructor *focus;
    QBFrameConstructor *bigFocus;
    QBFrameConstructor *inactiveFocus;

    QBXMBItemConstructor itemConstructor;
    QBXMBItemConstructor bigConstructor;
    QBXMBItemConstructor checkboxConstructor;

    size_t charactersLimit;     // Limit for characters displayed as caption of small item (non-Big).
                                // Limited captions are to be appended with '...'
};
typedef struct QBConaxMenuItemController_t* QBConaxMenuItemController;

SvLocal void QBConaxMenuItemControllerDestroy(void *self_)
{
    QBConaxMenuItemController self = self_;
    SVRELEASE(self->icon);
    SVRELEASE(self->tickMarkOn);
    SVRELEASE(self->tickMarkOff);
    SVRELEASE(self->focus);
    SVRELEASE(self->bigFocus);
    SVTESTRELEASE(self->inactiveFocus);
    QBXMBItemConstructorDestroy(self->itemConstructor);
    QBXMBItemConstructorDestroy(self->bigConstructor);
    QBXMBItemConstructorDestroy(self->checkboxConstructor);
}

SvLocal SvWidget QBConaxMenuItemControllerCreateItem(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
        return NULL;

    QBConaxMenuItemController self = (QBConaxMenuItemController) self_;

    QBActiveTreeNode node = (QBActiveTreeNode) node_;
    SvString id = QBActiveTreeNodeGetID(node);
    QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
    SvString parentId = parent ? QBActiveTreeNodeGetID(parent) : NULL;
    bool bigItems = parent ? isBooleanAttributeTrue(parent, SVSTRING("bigItems")) : false;

    QBXMBItem item = QBXMBItemCreate();
    QBXMBItemConstructor constructor = NULL;

    if (id && SvStringEqualToCString(id, "ConaxMail") && self->appGlobals->conaxMailManager) {
        size_t msgCnt = QBConaxMailManagerGetUnreadMailCount(self->appGlobals->conaxMailManager);
        if (msgCnt > 0) {
            item->subcaption = SvStringCreateWithFormat(gettext("Unread messages! (%zu)"), msgCnt);
        }
    } else if (parentId && SvStringEqualToCString(parentId, "ConaxMail")) {
        if (QBConaxMailManagerIsNodeNoMessages(node)) {
            item->caption = SvStringCreate(gettext("No messages"), NULL);
        } else {
            SvObject mail = QBConaxMailManagerGetMailFromNode(node);
            if (!mail) {
                SvLogError("%s(): invalid message node", __func__);
                SVRELEASE(item);
                return NULL;
            }
            const char* str1 = gettext("Message - NEW!");
            const char* str2 = gettext("Message");
            item->caption = SvStringCreateWithFormat("%s (%d)",
                                                     QBConaxMailManagerIsMailOpened(self->appGlobals->conaxMailManager, mail) ? str2 : str1,
                                                     QBConaxMailManagerGetMailSlot(self->appGlobals->conaxMailManager, mail));
            SvString mailText = QBConaxMailManagerGetMailText(self->appGlobals->conaxMailManager, mail);
            if (bigItems || SvStringLength(mailText) <= self->charactersLimit) {
                item->subcaption = SVTESTRETAIN(mailText);
            } else {
                item->subcaption = SvStringCreateWithFormat("%.*s...", (int) self->charactersLimit, SvStringCString(mailText));
            }
        }
    }

    SvString caption = (SvString)QBActiveTreeNodeGetAttribute(node, SVSTRING("caption"));
    if (!item->caption && caption) {
        const char *translatedCaptionStr = gettext(SvStringCString(caption));
        item->caption = SvStringCreate(translatedCaptionStr, NULL);
    }
    SvString subcaption = (SvString)QBActiveTreeNodeGetAttribute(node, SVSTRING("subcaption"));
    if (!item->subcaption && subcaption) {
        const char *translatedSubcaptionStr = gettext(SvStringCString(subcaption));
        item->subcaption = SvStringCreate(translatedSubcaptionStr, NULL);
    }

    if (parent && item->subcaption &&
        QBActiveTreeNodeGetAttribute(parent, SVSTRING("conaxPurse:debits")) && !bigItems) {
        char *newLineChar = strstr(SvStringCString(item->subcaption), "\n");
        if (newLineChar) {
            unsigned long long firstLineLength = (unsigned long long) (newLineChar - SvStringCString(item->subcaption));
            int minimalSpace = 3;
            int secondLineMinPosition = 20;
            int diff = secondLineMinPosition - firstLineLength;
            int spaces = diff > minimalSpace ? diff : minimalSpace;
            SvString firstLine = SvStringCreateWithCStringAndLength(SvStringCString(item->subcaption),
                                                                    firstLineLength,
                                                                    NULL);
            SvString secondLine = SvStringCreate(newLineChar + 1, NULL);
            SVRELEASE(item->subcaption);
            item->subcaption = SvStringCreateWithFormat("%s%*s%s", SvStringCString(firstLine), spaces, "", SvStringCString(secondLine));
            SVRELEASE(firstLine);
            SVRELEASE(secondLine);
        }
    }

    if (parentId)
    {
        if (SvStringEqualToCString(parentId, "ConaxMaturityRating")) {
            int myRating = 0x0;
            if(SvStringEqualToCString(id, "ConaxRatingG")) {
                myRating = 0x1;
            } else if(SvStringEqualToCString(id, "ConaxRatingPG")) {
                myRating = 0x2;
            } else if(SvStringEqualToCString(id, "ConaxRatingA")) {
                myRating = 0x4;
            } else if(SvStringEqualToCString(id, "ConaxRatingX")) {
                myRating = 0x8;
            }
            int currentRating = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getMaturityRating);

            if(currentRating == myRating)
                item->icon = SVRETAIN(self->tickMarkOn);
            else
                item->icon = SVRETAIN(self->tickMarkOff);

            constructor = self->checkboxConstructor;
            goto do_create;
        }
    }

    if (!constructor && bigItems) {
        constructor = self->bigConstructor;
        item->focus = SVRETAIN(self->bigFocus);
        goto do_create;
    }

do_create:
    if (!item->focus)
        item->focus = SVRETAIN(self->focus);
    if (!item->inactiveFocus)
        item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);
    if (!item->icon)
        item->icon = SVRETAIN(self->icon);
    if (!constructor)
        constructor = self->itemConstructor;

    SvWidget ret = QBXMBItemConstructorCreateItem(constructor, item, app, initialState);

    SVRELEASE(item);
    return ret;
}

SvLocal void QBConaxMenuItemControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBConaxMenuItemController self = (QBConaxMenuItemController) self_;

    /**
     * Hack, we actualy don't know which item controller should be used, but all out constructors
     * are the same class, so it will work. If widget's height is the same as bigConstructor's height
     * then favour bigConstructor as SetItemState function modifies widget's height.
     * The solution is to :
     * 1. Somehow indentify by prv of item_ which constructor to use.
     * 2. Create many different item constrollers
     */
    if (item_->height == self->bigConstructor->height)
        QBXMBItemConstructorSetItemState(self->bigConstructor, item_, state, isFocused);
    else
        QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);}

SvLocal SvWidget QBConaxMenuItemControllerCreateSubMenu(SvGenericObject self_, SvGenericObject node_, SvGenericObject path, SvApplication app)
{
    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()) && QBActiveTreeNodeGetChildNodesCount((QBActiveTreeNode) node_) == 0)
        return NULL;

    QBActiveTreeNode node = (QBActiveTreeNode) node_;

    const char* widget_name = NULL;

    if (isBooleanAttributeTrue(node, SVSTRING("bigItems"))) {
        widget_name = "Big";
    }

    if (widget_name) {
        svSettingsPushComponent("ConaxMenu.settings");
        SvWidget ret = XMBVerticalMenuNew(app, widget_name, NULL);
        svSettingsPopComponent();
        return ret;
    }

    return XMBVerticalMenuNew(app, "menuBar.menu", NULL);
}

SvLocal SvType QBConaxMenuItemController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBConaxMenuItemControllerDestroy
    };
    static SvType type = NULL;

    static const struct XMBMenuController_t menu_controller_methods = {
        .createSubMenu = QBConaxMenuItemControllerCreateSubMenu,
    };

    static const struct XMBItemController_t controller_methods = {
        .createItem = QBConaxMenuItemControllerCreateItem,
        .setItemState = QBConaxMenuItemControllerSetItemState,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBConaxMenuItemController",
                            sizeof(struct QBConaxMenuItemController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBMenuController_getInterface(), &menu_controller_methods,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }

    return type;
}


void QBConaxMenuItemControllerRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvString rootID = SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) == QBCASType_cryptoguard ? SVSTRING("Cryptoguard") : SVSTRING("Conax");
    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, rootID);
    if (!path)
        return;

    QBConaxMenuItemController self = (QBConaxMenuItemController) SvTypeAllocateInstance(QBConaxMenuItemController_getType(), NULL);

    self->appGlobals = appGlobals;

    svSettingsPushComponent("ConaxMenu.settings");

    self->icon = SVRETAIN(svSettingsGetBitmap("MenuItem", "icon"));
    self->tickMarkOn = SVRETAIN(svSettingsGetBitmap("MenuItem", "tickMarkOn"));
    self->tickMarkOff = SVRETAIN(svSettingsGetBitmap("MenuItem", "tickMarkOff"));
    self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    self->bigFocus = QBFrameConstructorFromSM("Big.MenuItem.focus");
    if (svSettingsIsWidgetDefined("MenuItem.inactiveFocus"))
        self->inactiveFocus = QBFrameConstructorFromSM("MenuItem.inactiveFocus");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem", appGlobals->textRenderer);
    self->bigConstructor = QBXMBItemConstructorCreate("Big.MenuItem", appGlobals->textRenderer);
    self->checkboxConstructor = QBXMBItemConstructorCreate("Checkbox.MenuItem", appGlobals->textRenderer);
    self->charactersLimit = svSettingsGetInteger("MenuItem", "charactersLimit", 55);

    svSettingsPopComponent();

    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) self, NULL);

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) self, NULL);
    SVRELEASE(self);

    SVRELEASE(path);
}
