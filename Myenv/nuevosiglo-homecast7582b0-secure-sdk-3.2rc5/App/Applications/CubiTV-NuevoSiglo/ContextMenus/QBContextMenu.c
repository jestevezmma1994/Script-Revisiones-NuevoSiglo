/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBContextMenu.h"
#include "QBContextMenuBase.h"
#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEvent.h>
#include <QBAppKit/QBPeerEventReceiver.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <Widgets/SideMenu.h>
#include <QBDialogWindowInterface.h>

SvLocal void QBContextMenuStop(QBContextMenu self);

SvInterface QBContextMenuPane_getInterface(void)
{
    static SvInterface interface = NULL;
    if (unlikely(!interface)) {
        SvInterfaceCreateManaged("QBContextMenuPaneInterface", sizeof(struct QBContextMenuPane_), NULL, &interface, NULL);
    }

    return interface;
}


SvLocal void QBContextMenuDestroy(void *ptr)
{
    QBContextMenu self = ptr;
    while (SvArrayCount(self->stack))
        SvArrayRemoveLastObject(self->stack);

    SVRELEASE(self->stack);
    svWidgetDestroy(self->window);
    self->super_.window = NULL;
}

SvLocal void QBContextMenuForceHide(SvGenericObject self_)
{
    QBContextMenu self = (QBContextMenu) self_;
    QBContextMenuHide(self, true);
    // callback will be executed deeper from QBContextMenuStop (given arg. immediately==true)
}

SvLocal void QBContextMenuHandlePeerEvent(SvObject self_, QBPeerEvent event_, SvObject sender)
{
    QBContextMenu self = (QBContextMenu) self_;

    if (self->sideMenu && sender == self->sideMenu->prv) {
        if (SvObjectIsInstanceOf((SvObject) event_, QBSideMenuLevelChangedEvent_getType())) {
            if (SideMenuGetLevel(self->sideMenu) == 0) {
                QBContextMenuStop(self);
            }
        }
    }
}

void QBContextMenuBaseShow(QBContextMenu self)
{
    if (!self)
        return;
    QBApplicationControllerAddLocalWindow(self->controller, (QBLocalWindow) self);
    self->isShown = true;
}

SvLocal
void QBContextMenuBaseHide(QBContextMenu self, bool immediately)
{
    if (!self || !self->isShown)
        return;

    SvGenericObject pane;
    while ((pane = SvArrayLastObject(self->stack))) {
        SvInvokeInterface(QBContextMenuPane, pane, hide, immediately);
        SvArrayRemoveLastObject(self->stack);
    }
    self->isShown = false;
    if (immediately)
        QBContextMenuStop(self);
}

SvLocal
void QBContextMenuBaseIsShown(QBContextMenu self, bool* result)
{
    *result = self->isShown;
}

SvType QBContextMenu_getType(void)
{
    static const struct QBDialogWindow_ dialogWindowMethods = {
        .forceHide = QBContextMenuForceHide
    };
    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBContextMenuHandlePeerEvent
    };
    static const struct QBContextMenuVTable_ virtualMethods = {
        .super_          = {
            .super_      = {
                .destroy = QBContextMenuDestroy
            },
        },
        .show            = QBContextMenuBaseShow,
        .hide            = QBContextMenuBaseHide,
        .isShown         = QBContextMenuBaseIsShown
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateVirtual("QBContextMenu",
                            sizeof(struct QBContextMenu_t),
                            QBLocalWindow_getType(),
                            sizeof(struct QBContextMenuVTable_),
                            &virtualMethods,
                            &type,
                            QBDialogWindow_getInterface(), &dialogWindowMethods,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }
    return type;
}

SvLocal void QBContextMenuWindowCleanup(SvApplication app, void *self_)
{
    QBContextMenu self = self_;
    self->window = NULL;
}

SvLocal bool QBContextMenuWindowInputEventHandler(SvWidget window, SvInputEvent e)
{
    QBContextMenu self = window->prv;
    // AMERELES Fix en la navegación del menú de la derecha (VOD 1)
    //if (SvArrayGetCount(self->stack) > 1) {
    if (SvArrayGetCount(self->stack) > 0) {
        if (e->ch == QBKEY_LEFT) {
            QBContextMenuPopPane(self);
            return true;
        }
        if (e->ch == QBKEY_BACK) {
            QBContextMenuPopPane(self);
            return true;
        }
    }
    return svWidgetInputEventHandler(window, e);
}

QBContextMenu QBContextMenuCreateFromSettings(const char *settings, QBApplicationController controller, SvApplication app, SvString sideMenuName)
{
    svSettingsPushComponent(settings);
    QBContextMenu self = (QBContextMenu) SvTypeAllocateInstance(QBContextMenu_getType(), NULL);
    QBContextMenuInit(self, controller, app, sideMenuName);
    svSettingsPopComponent();

    return self;
}

void QBContextMenuInit(QBContextMenu self, QBApplicationController controller, SvApplication app, SvString sideMenuName)
{
    self->window = svWidgetCreateBitmap(app, app->width, app->height, NULL);
    self->controller = controller;
    self->sideMenu = SideMenuNew(app, SvStringCString(sideMenuName));
    self->stack = SvArrayCreate(NULL);

    QBLocalWindowInit((QBLocalWindow) self, self->window, QBLocalWindowTypeFocusable);
    self->window->prv = self;
    self->window->clean = QBContextMenuWindowCleanup;
    svWidgetSetInputEventHandler(self->window, QBContextMenuWindowInputEventHandler);
    svWidgetSetName(self->window, SvStringCString(sideMenuName));

    svSettingsWidgetAttach(self->window, self->sideMenu, SvStringCString(sideMenuName), 0);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, QBSideMenuLevelChangedEvent_getType(), self->sideMenu->prv, NULL);
}

void QBContextMenuSetCallbacks(QBContextMenu self, QBContextMenuCallback callback, void *ptr)
{
    self->callback = callback;
    self->callbackData = ptr;
}

void QBContextMenuShow(QBContextMenu self)
{
    SvInvokeVirtual(QBContextMenu, self, show);
}

void QBContextMenuHide(QBContextMenu self, bool immediately)
{
    SvInvokeVirtual(QBContextMenu, self, hide, immediately);
}

bool QBContextMenuIsShown(QBContextMenu self)
{
    bool result = 0;
    SvInvokeVirtual(QBContextMenu, self, isShown, &result);
    return result;
}

void QBContextMenuSetDepth(QBContextMenu self, int level, bool immediately)
{
    SideMenuShow(self->sideMenu, level, immediately);
}

void QBContextMenuPushPane(QBContextMenu self, SvGenericObject pane)
{
    SvArrayAddObject(self->stack, pane);
    SvInvokeInterface(QBContextMenuPane, pane, show);
    SvInvokeInterface(QBContextMenuPane, pane, setActive);
}

void QBContextMenuPopPane(QBContextMenu self)
{
    SvGenericObject oldPane = SvArrayLastObject(self->stack);
    if (oldPane) {
        SvInvokeInterface(QBContextMenuPane, oldPane, hide, false);
    }
    SvArrayRemoveLastObject(self->stack);
    SvGenericObject newPane = SvArrayLastObject(self->stack);
    if (newPane)
        SvInvokeInterface(QBContextMenuPane, newPane, setActive);
    else
        QBContextMenuHide(self, false);
}

void QBContextMenuSwitchPane(QBContextMenu self, SvGenericObject pane)
{
    SvGenericObject oldPane = SvArrayLastObject(self->stack);
    if (oldPane) {
        SvInvokeInterface(QBContextMenuPane, oldPane, hide, false);
        SvArrayRemoveLastObject(self->stack);
        SvArrayAddObject(self->stack, pane);
        SvInvokeInterface(QBContextMenuPane, pane, show);
        SvInvokeInterface(QBContextMenuPane, pane, setActive);
    } else {
        SvLogError("%s: empty stack", __func__);
    }
}

int QBContextMenuGetCurrentPaneLevel(QBContextMenu self)
{
    return SvArrayCount(self->stack);
}

bool QBContextMenuHandleInput(QBContextMenu self, SvObject src, SvInputEvent e)
{
    int idx = SvArrayIndexOfObjectIdenticalTo(self->stack, src);
    bool ret = false;
    int i;
    for (i = 0; i < idx; i++) {
        SvObject pane = SvArrayGetObjectAtIndex(self->stack, i);
        ret = SvInvokeInterface(QBContextMenuPane, pane, handleInputEvent, src, e);
        if (ret) {
            break;
        }
    }

    if (!ret && self->inputEventHandler) {
        ret = self->inputEventHandler(self->inputData, src, e);
    }

    return ret;
}

void QBContextMenuSetInputEventHandler(QBContextMenu self, void *prv, QBContextMenuInputEventHandler handler)
{
    self->inputEventHandler = handler;
    self->inputData = prv;
}

SvLocal void QBContextMenuStop(QBContextMenu self)
{
    if (self->callback) {
        self->callback(self->callbackData, self);
    }
    QBApplicationControllerRemoveLocalWindow(self->controller, (QBLocalWindow) self);
}

