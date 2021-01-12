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

#include "QBSkinsCarouselItemChoice.h"

#include <libintl.h>
#include <QBConf.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <QBCarousel/QBCarousel.h>
#include <init.h>
#include <Services/QBSkinManager.h>
#include <Menus/menuchoice.h>
#include <Widgets/confirmationDialog.h>
#include <QBWidgets/QBDialog.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <Utils/value.h>
#include <main.h>


struct QBSkinsCarouselMenuChoice_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBSkinManager skinManager;

    SvWidget confirmationDialog;

    SvGenericObject path;
    SvGenericObject skin;

    const char *skinName;

    int serviceId;
};

SvLocal void
QBSkinsCarousel_dialogCallback(void *self_,
                               SvWidget dialog,
                               SvString buttonTag,
                               unsigned keyCode)
{
    QBSkinsCarouselMenuChoice self = self_;

    if (SvStringEqualToCString(buttonTag, "OK-button"))
        QBSkinManagerSetSkin(self->skinManager, self->skin);

    SVRELEASE(self->skin);
    self->skin = NULL;
    self->confirmationDialog = NULL;
}

SvLocal void
QBSkinsCarouselMenuCreateDialog(QBSkinsCarouselMenuChoice self)
{
    QBConfirmationDialogParams_t params = {
        .title = gettext("Warning"),
        .message = gettext("System reboot is required to apply new application layout."),
        .local = true,
        .focusOK = false,
        .isCancelButtonVisible = true
    };

    self->confirmationDialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
    if (self->confirmationDialog) {
        QBDialogRun(self->confirmationDialog, self, QBSkinsCarousel_dialogCallback);
    }
}

SvLocal bool
QBSkinsCarouselMenuNodeSelected(SvGenericObject self_,
                                SvGenericObject node,
                                SvGenericObject nodePath_)
{
    QBSkinsCarouselMenuChoice self = (QBSkinsCarouselMenuChoice) self_;
    SVTESTRETAIN(nodePath_);
    SVTESTRELEASE(self->path);
    self->path = nodePath_;

    SVRETAIN(node);
    SVTESTRELEASE(self->skin);
    self->skin = node;

    if (!self->skinManager)
        return true;

    if (SvObjectIsInstanceOf(node, SvDBRawObject_getType())) {
        SvString skinName = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) node, "name"));
        if (!self->skinName || (skinName && !SvStringEqualToCString(skinName, self->skinName)))
            QBSkinsCarouselMenuCreateDialog(self);
        return true;
    } else if (SvObjectIsInstanceOf(node, SvString_getType())) {
        if (!self->skinName || (node && !SvStringEqualToCString((SvString) node, self->skinName)))
            QBSkinsCarouselMenuCreateDialog(self);
        return true;
    }
    return false;
}

SvLocal void
QBSkinsCarouselMenuChoiceDestroy(void *self_)
{
    QBSkinsCarouselMenuChoice self = self_;

    if (self->confirmationDialog)
        QBDialogBreak(self->confirmationDialog);

    SVTESTRELEASE(self->path);
    SVTESTRELEASE(self->skin);

    SVTESTRELEASE(self->skinManager);
}

SvLocal SvType
QBSkinsCarouselMenuChoice_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSkinsCarouselMenuChoiceDestroy
    };
    static SvType type = NULL;

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBSkinsCarouselMenuNodeSelected,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSkinsCarouselMenuChoice",
                            sizeof(struct QBSkinsCarouselMenuChoice_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            NULL);
    }

    return type;
}

QBSkinsCarouselMenuChoice
QBSkinsCarouselMenuChoiceNew(AppGlobals appGlobals, QBSkinManager skinManager)
{
    QBSkinsCarouselMenuChoice self = (QBSkinsCarouselMenuChoice) SvTypeAllocateInstance(QBSkinsCarouselMenuChoice_getType(), NULL);

    self->appGlobals = appGlobals;
    self->skinManager = SVTESTRETAIN(skinManager);

    self->skin = NULL;
    self->skinName = QBConfigGet("SKIN.NAME");

    return self;
}
