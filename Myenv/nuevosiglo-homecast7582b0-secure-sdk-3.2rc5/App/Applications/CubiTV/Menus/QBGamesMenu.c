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

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <settings.h>
#include <main.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContainerPane.h>
#include <Widgets/calc.h>
#include <Menus/menuchoice.h>
#include "QBGamesMenu.h"


struct QBGamesMenuHandler_ {
    struct SvObject_ super_;
    QBContextMenu ctxMenu;
    QBContainerPane calculatorPane;
    SvWidget calculator;
    AppGlobals appGlobals;

    int calcXOffset;
    int calcYOffset;
};
typedef struct QBGamesMenuHandler_ *QBGamesMenuHandler;

SvLocal void QBGamesMenuHandlerDestroy(void *self_)
{
    QBGamesMenuHandler self = self_;
    svWidgetDetach(self->calculator);
    SVTESTRELEASE(self->calculatorPane);
    SVTESTRELEASE(self->ctxMenu);
    svWidgetDestroy(self->calculator);
}

SvLocal void QBGamesMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    QBGamesMenuHandler self = (QBGamesMenuHandler) self_;
    SvString id = QBActiveTreeNodeGetID((QBActiveTreeNode) node_);
    if(SvStringEqualToCString(id, "Calculator")) {
        QBContextMenuPushPane(self->ctxMenu, (SvGenericObject) self->calculatorPane);
        QBContextMenuShow(self->ctxMenu);
    }
}

SvLocal SvType QBGamesMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBGamesMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBGamesMenuChoosen
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBGamesMenuHandler",
                            sizeof(struct QBGamesMenuHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuChoice_getInterface(), &menuMethods,
                            NULL);
    }

    return type;
}

SvLocal void QBGamesCalculatorOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBGamesMenuHandler self = self_;
    svWidgetDetach(self->calculator);
}

SvLocal void QBGamesCalculatorOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBGamesMenuHandler self = self_;
    svWidgetAttach(frame, self->calculator, self->calcXOffset, self->calcYOffset, 0);
}

SvLocal void QBGamesCalculatorSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBGamesMenuHandler self = self_;
    svWidgetSetFocus(self->calculator);
}

void QBGamesMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("GAMES"));
    if (!path)
        return;

    QBGamesMenuHandler self = (QBGamesMenuHandler) SvTypeAllocateInstance(QBGamesMenuHandler_getType(), NULL);
    self->appGlobals = appGlobals;

    QBTreePathMapInsert(pathMap, path, (SvObject) self, NULL);
    SVRELEASE(path);
    SVRELEASE(self);


    self->ctxMenu = QBContextMenuCreateFromSettings("SideMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide = QBGamesCalculatorOnHide,
        .onShow = QBGamesCalculatorOnShow,
        .setActive = QBGamesCalculatorSetActive,
    };

    self->calculatorPane = QBContainerPaneCreateFromSettings("CalcPane.settings", appGlobals->res, self->ctxMenu, 1, SVSTRING("CalcPane"), &callbacks, self);

    svSettingsPushComponent("Calc.settings");
    self->calculator = QBCalcNew(appGlobals->res, appGlobals->scheduler, "Calc");
    self->calcXOffset = svSettingsGetInteger("Calc", "xOffset", 0);
    self->calcYOffset = svSettingsGetInteger("Calc", "yOffset", 0);
    svSettingsPopComponent();
}
