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

#include "DSMCCCarouselItemChoice.h"
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBInnov8onExtendedInfoPane.h>
#include <main.h>
#include <Menus/menuchoice.h>
#include <QBMenu/QBMenuEventHandler.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBDSMCCCarouselMenuChoice"
    #define log_state(fmt, ...) do { SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...) do { SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_state(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif


struct QBDSMCCCarouselMenuChoice_s {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvString serviceId;

    struct QBDSMCCCarouselSideMenu_s {
        QBContextMenu ctx;
    } sidemenu;
};

SvLocal SvType QBDSMCCCarouselMenuChoice_getType(void);

QBDSMCCCarouselMenuChoice QBDSMCCCarouselMenuChoiceCreate(AppGlobals appGlobals, SvString serviceId)
{
    if (!appGlobals) {
        log_error("got NULL appGlobals");
        return NULL;
    }
    if (!serviceId) {
        log_error("got NULL serviceId");
        return NULL;
    }

    QBDSMCCCarouselMenuChoice self = NULL;
    QBContextMenu sidemenuCtx = NULL;

    // create members

    sidemenuCtx = QBContextMenuCreateFromSettings("ContextMenu.settings", appGlobals->controller, appGlobals->res, SVSTRING("SideMenu"));

    // create self

    self = (QBDSMCCCarouselMenuChoice) SvTypeAllocateInstance(QBDSMCCCarouselMenuChoice_getType(), NULL);
    if (unlikely(!self)) {
        log_error("allocation of self failed");
        goto fini;
    }

    // attach members to self

    self->appGlobals = appGlobals;
    self->serviceId = SVRETAIN(serviceId);

    self->sidemenu.ctx = SVRETAIN(sidemenuCtx);

fini:
    SVTESTRELEASE(sidemenuCtx);
    return self;
}

SvLocal void QBDSMCCCarouselMenuChoiceDestroy(void *self_)
{
    QBDSMCCCarouselMenuChoice self = self_;

    SVRELEASE(self->serviceId);

    if (self->sidemenu.ctx) {
        QBContextMenuHide(self->sidemenu.ctx, false);
        SVRELEASE(self->sidemenu.ctx);
    }
}

SvLocal bool QBDSMCCCarouselMenuNodeSelected(SvObject self_, SvObject node, SvObject nodePath);

SvLocal SvType QBDSMCCCarouselMenuChoice_getType(void)
{
    static const struct SvObjectVTable_ vtable = {
        .destroy = QBDSMCCCarouselMenuChoiceDestroy
    };

    static const struct QBMenuEventHandler_ selectedMethods = {
        .nodeSelected = QBDSMCCCarouselMenuNodeSelected
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBDSMCCCarouselMenuChoice",
                            sizeof(struct QBDSMCCCarouselMenuChoice_s),
                            SvObject_getType(), &type,
                            SvObject_getType(), &vtable,
                            QBMenuEventHandler_getInterface(), &selectedMethods,
                            NULL);
    }

    return type;
}

SvLocal bool QBDSMCCCarouselMenuNodeSelected(SvObject self_, SvObject node, SvObject nodePath)
{
    if (!self_) {
        log_error("got NULL self");
        return false;
    }
    if (!node) {
        log_error("got NULL node");
        return false;
    }

    QBDSMCCCarouselMenuChoice self = (QBDSMCCCarouselMenuChoice) self_;

    if (SvObjectIsInstanceOf(node, SvDBRawObject_getType()) && SvDBRawObjectGetAttrValue((SvDBRawObject) node, "description")) {
        QBInnov8onExtendedInfoPane moreInfo =
            QBInnov8onExtendedInfoPaneCreateFromSettings("carouselExtendedInfo.settings",
                                                         self->appGlobals, self->sidemenu.ctx,
                                                         SVSTRING("ExtendedInfoPane"),
                                                         1, (SvObject) node);
        QBContextMenuPushPane(self->sidemenu.ctx, (SvObject) moreInfo);
        QBContextMenuShow(self->sidemenu.ctx);
        SVRELEASE(moreInfo);
        return true;
    }

    return false;
}
