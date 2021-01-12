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

#include "QBBrowserMenu.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <QBApplicationController.h>
#include "menuchoice.h"
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <QBMenu/QBMenu.h>
#include <main.h>


struct QBBrowserMenuHandler_t {
    struct SvObject_ super_;
    QBApplicationController ctr;
    AppGlobals appGlobals;
};

typedef struct QBBrowserMenuHandler_t* QBBrowserMenuHandler;


SvLocal void QBBrowserMenuHandlerDestroy(void *self_)
{
}

SvLocal void QBBrowserMenuChoosen(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_, int position)
{
    if (SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType())) {
        QBActiveTreeNode node = (QBActiveTreeNode) node_;
        SvString globalURL = (SvString) QBActiveTreeNodeGetAttribute(node, SVSTRING("URI"));
        if (!globalURL) {
             SvLogWarning("CubiTV: no URL in browser menu item");
             return;
        }
#if 0
        bool sNav = false, navBar = false;
        SvValue sNavValue = (SvValue)QBActiveTreeNodeGetAttribute(node, SVSTRING("sNav"));
        SvValue navBarValue = (SvValue)QBActiveTreeNodeGetAttribute(node, SVSTRING("navBar"));
        if(sNavValue)
            sNav = SvValueGetBoolean(sNavValue);
        if(navBarValue)
            navBar = SvValueGetBoolean(navBarValue);

        QBWindowContext wwwContext = QBBrowserContextCreate(self->appGlobals);
        QBBrowserSetURI(wwwContext,globalURL);
        QBApplicationControllerPushContext(self->ctr,wwwContext);
        if(sNav)
            QBBrowserShowSNAV(wwwContext, true);
        if(navBar)
            QBBrowserShowNavBar(wwwContext);
        SVRELEASE(wwwContext);
#endif
    }
}


SvLocal SvType QBBrowserMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBrowserMenuHandlerDestroy
    };
    static SvType type = NULL;
    static const struct QBMenuChoice_t menuMethods = {
        .choosen = QBBrowserMenuChoosen
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBBrowserMenuHandler",
                            sizeof(struct QBBrowserMenuHandler_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBMenuEventHandler_getInterface(), &menuMethods,
                            NULL);
    }
    return type;
}

void QBBrowserMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    SvGenericObject path = QBActiveTreeCreateNodePath(appGlobals->menuTree, SVSTRING("WWW"));
    if (!path)
        return;

    QBBrowserMenuHandler handler = (QBBrowserMenuHandler) SvTypeAllocateInstance(QBBrowserMenuHandler_getType(), NULL);
    handler->ctr = appGlobals->controller;
    handler->appGlobals = appGlobals;

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) handler, NULL);

    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) handler, NULL);
    SVRELEASE(handler);

    SVRELEASE(path);
}
