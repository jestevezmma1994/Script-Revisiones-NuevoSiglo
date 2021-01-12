/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBCallerIDMenu.h"

#include <stdbool.h>
#include <libintl.h>

#include <SvFoundation/SvType.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBEventBus.h>
#include <QBAppKit/QBPeerEventReceiver.h>

#include <Services/QBCallerIdManager.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContainerPane.h>
#include <ContextMenus/QBBasicPane.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <QBWidgets/QBAsyncLabel.h>

#include <Menus/menuchoice.h>
#include <Windows/mainmenu.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBTreeProxy.h>
#include <XMB2/XMBItemController.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <Logic/timeFormat.h>
#include <QBStringUtils.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/button.h>
#include <SWL/QBFrame.h>
#include <SWL/events.h>
#include <QBMenu/QBMenu.h>

#include <main.h>


typedef struct QBCallerIDMenuHandler_ *QBCallerIDMenuHandler;

struct QBCallerIDMenuHandler_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    QBActiveTreeNode mainNode;

    struct QBCallerIDHistorySideMenu_t {
        QBContextMenu ctx;
        SvHashTable callDataHT;
        SvWidget okButton;
    } sidemenu;

    QBActiveArray callsList;

    SvWidget callDetailsWidget;
    SvBitmap phoneIcon;

    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
    QBFrameConstructor* disabledFocus;
};

SvLocal void
QBCallerIDMenuHandlerDestroy(void *self_)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;

    if (self->appGlobals->callerIdManager)
        QBCallerIdManagerRemoveListener(self->appGlobals->callerIdManager, (SvGenericObject) self);

    SVRELEASE(self->callsList);
    SVRELEASE(self->phoneIcon);

    QBXMBItemConstructorDestroy(self->itemConstructor);
    SVTESTRELEASE(self->focus);
    SVTESTRELEASE(self->disabledFocus);
    SVTESTRELEASE(self->sidemenu.ctx);
}

SvLocal SvWidget
QBCallerIDHistoryCreateItem(SvGenericObject self_, SvGenericObject nodeData_, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;

    if (!SvObjectIsInstanceOf(nodeData_, SvHashTable_getType()))
        return NULL;

    SvHashTable callData = (SvHashTable) nodeData_;
    QBXMBItem item = QBXMBItemCreate();
    item->focus = SVTESTRETAIN(self->focus);
    item->disabledFocus = SVTESTRETAIN(self->disabledFocus);
    item->icon = SVRETAIN(self->phoneIcon);
    SvValue nameVal = (SvValue) SvHashTableFind(callData, (SvGenericObject) SVSTRING("callerName"));
    if (nameVal) {
        item->caption = SVRETAIN(SvValueGetString(nameVal));
    } else {
        SvValue numberVal = (SvValue) SvHashTableFind(callData, (SvGenericObject) SVSTRING("number"));
        item->caption = SVRETAIN(SvValueGetString(numberVal));
    }

    SvWidget ret = NULL;
    ret = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);
    SVRELEASE(item);

    return ret;
}

SvLocal void
QBCallerIDHistorySetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void
QBCallerIDHistoryMenuCallDetailsWidgetClean(SvApplication app, void *self_)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;
    self->callDetailsWidget = NULL;
}

SvLocal void
QBCallerIDHistoryMenuCallDetailsWidgetCreate(QBCallerIDMenuHandler self)
{
    self->callDetailsWidget = svSettingsWidgetCreate(self->appGlobals->res, "CallDetails");
    self->callDetailsWidget->prv = self;
    self->callDetailsWidget->clean = QBCallerIDHistoryMenuCallDetailsWidgetClean;

    // name
    SVAUTOSTRING(callerNameKeyVal, "callerName");
    SvValue callerNameVal = (SvValue) SvHashTableFind(self->sidemenu.callDataHT, (SvGenericObject)callerNameKeyVal);
    if (callerNameVal) {
        SvWidget callerNameWidget = QBAsyncLabelNew(self->appGlobals->res, "CallDetails.CallerName", self->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->callDetailsWidget, callerNameWidget, svWidgetGetName(callerNameWidget), 1);
        SvString callerNameStr = SvValueGetString(callerNameVal);
        QBAsyncLabelSetCText(callerNameWidget, SvStringCString(callerNameStr));
    }

    // number
    SvWidget numberWidget = QBAsyncLabelNew(self->appGlobals->res, "CallDetails.Number", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(self->callDetailsWidget, numberWidget, svWidgetGetName(numberWidget), 1);
    SVAUTOSTRING(numberKeyVal, "number");
    SvValue numberVal = (SvValue) SvHashTableFind(self->sidemenu.callDataHT, (SvGenericObject)numberKeyVal);
    SvString numberStr = SvValueGetString(numberVal);
    QBAsyncLabelSetCText(numberWidget, SvStringCString(numberStr));

    // line
    SVAUTOSTRING(lineKeyVal, "line");
    SvValue lineVal = (SvValue) SvHashTableFind(self->sidemenu.callDataHT, (SvGenericObject)lineKeyVal);
    if (lineVal) {
        SvWidget lineWidget = QBAsyncLabelNew(self->appGlobals->res, "CallDetails.Line", self->appGlobals->textRenderer);
        svSettingsWidgetAttach(self->callDetailsWidget, lineWidget, svWidgetGetName(lineWidget), 1);
        SvString lineStr = SvValueGetString(lineVal);
        SvString lineNumberStr = SvStringCreateWithFormat("%s %s", gettext("Line"), SvStringCString(lineStr));
        QBAsyncLabelSetCText(lineWidget, SvStringCString(lineNumberStr));
        SVRELEASE(lineNumberStr);
    }

    // date & time
    SVAUTOSTRING(dateTimeKeyVal, "dateTime");
    SvValue dateTimeVal = (SvValue) SvHashTableFind(self->sidemenu.callDataHT, (SvGenericObject) dateTimeKeyVal);
    if (!dateTimeVal) {
        return;
    }

    SvString dateTimeStr = SvValueGetString(dateTimeVal);
    SvTime dateTimeV;
    size_t len = QBStringToDateTime(SvStringCString(dateTimeStr), QBDateTimeFormat_ISO8601, true, &dateTimeV);
    if (len == 0) {
        return;
    }

    struct tm localTime;
    if (SvTimeBreakDown(dateTimeV, true, &localTime) < 0) {
        //Ups, we are out of t_time type range -> sizeof(time_t) == 4
        const char* endOfTheTime = "2038-01-01T00:00:00Z";
        QBStringToDateTime(endOfTheTime, QBDateTimeFormat_ISO8601, false, &dateTimeV);
        SvTimeBreakDown(dateTimeV, true, &localTime);
    }
    char dateFormatedCStr[100] = {'\0'};
    char timeFormatedCStr[100] = {'\0'};
    strftime(dateFormatedCStr, sizeof(dateFormatedCStr), QBTimeFormatGetCurrent()->callDate, &localTime);
    strftime(timeFormatedCStr, sizeof(timeFormatedCStr), QBTimeFormatGetCurrent()->callTime, &localTime);

    SvWidget dateWidget = QBAsyncLabelNew(self->appGlobals->res, "CallDetails.Date", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(self->callDetailsWidget, dateWidget, svWidgetGetName(dateWidget), 1);
    QBAsyncLabelSetCText(dateWidget, dateFormatedCStr);

    SvWidget timeWidget = QBAsyncLabelNew(self->appGlobals->res, "CallDetails.Time", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(self->callDetailsWidget, timeWidget, svWidgetGetName(timeWidget), 1);
    QBAsyncLabelSetCText(timeWidget, timeFormatedCStr);

    self->sidemenu.okButton = svButtonNewFromSM(self->appGlobals->res, "CallDetails.OKButton", gettext("OK"), 1, svWidgetGetId(self->callDetailsWidget));
    svSettingsWidgetAttach(self->callDetailsWidget, self->sidemenu.okButton , svWidgetGetName(self->sidemenu.okButton), 1);
    svWidgetSetFocusable(self->sidemenu.okButton, true);

    QBEventBus eventBus = (QBEventBus) QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("EventBus"));
    QBEventBusRegisterReceiverForSender(eventBus, (SvObject) self, SvButtonPushedEvent_getType(), self->sidemenu.okButton->prv, NULL);

    SVRELEASE(self->sidemenu.callDataHT);
}

SvLocal void
QBCallerIDHistoryMenuCallDetailsOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
}

SvLocal void
QBCallerIDMenuHandlerHandlePeerEvent(SvObject self_, QBPeerEvent event, SvObject sender)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;

    if (SvObjectIsInstanceOf((SvObject) event, SvButtonPushedEvent_getType())) {
        if (self->callDetailsWidget && sender == (SvObject) self->sidemenu.okButton->prv) {
            QBContextMenuHide(self->sidemenu.ctx, false);
            return;
        }
    }
}

SvLocal void
QBCallerIDHistoryMenuCallDetailsOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBCallerIDMenuHandler self = self_;

    svSettingsPushComponent("CallerID.settings");

    QBCallerIDHistoryMenuCallDetailsWidgetCreate(self);
    svSettingsWidgetAttach(frame, self->callDetailsWidget, svWidgetGetName(self->callDetailsWidget), 0);

    svSettingsPopComponent();
}

SvLocal void
QBCallerIDHistoryMenuCallDetailsSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBCallerIDMenuHandler self = self_;
    svWidgetSetFocus(self->sidemenu.okButton);
}

SvLocal void
QBCallerIDHistoryMenuPrepareSidemenu(QBCallerIDMenuHandler self, SvGenericObject node_)
{
    const char *componentName = "CallerID.settings";
    svSettingsPushComponent(componentName);

    self->sidemenu.callDataHT = (SvHashTable) SVRETAIN(node_);
    SVTESTRELEASE(self->sidemenu.ctx);

    self->sidemenu.ctx = QBContextMenuCreateFromSettings(componentName, self->appGlobals->controller, self->appGlobals->res, SVSTRING("CallDetailsSideMenu"));

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBCallerIDHistoryMenuCallDetailsOnHide,
        .onShow    = QBCallerIDHistoryMenuCallDetailsOnShow,
        .setActive = QBCallerIDHistoryMenuCallDetailsSetActive
    };

    QBContainerPane callDetails = (QBContainerPane)SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    QBContainerPaneInit(callDetails, self->appGlobals->res, self->sidemenu.ctx, 1, SVSTRING("BasicPane"), &callbacks, self);

    QBContextMenuPushPane(self->sidemenu.ctx, (SvGenericObject) callDetails);
    QBContextMenuShow(self->sidemenu.ctx);
    SVRELEASE(callDetails);

    svSettingsPopComponent();
}

SvLocal void
QBCallerIDHistoryMenuPrepare(QBCallerIDMenuHandler self)
{
    SvArray callsData = QBCallerIdManagerGetCallsHistory(self->appGlobals->callerIdManager);
    if (!callsData) {
        return;
    }

    QBActiveArrayRemoveAllObjects(self->callsList, NULL);

    SvIterator it = SvArrayIterator(callsData);
    SvGenericObject data = NULL;
    while ((data = SvIteratorGetNext(&it))) {
        QBActiveArrayAddObject(self->callsList, data, NULL);
    }
}

SvLocal void
QBCallerIDMenuCallsDataChanged(SvGenericObject self_)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;

    SvArray callsData = QBCallerIdManagerGetCallsHistory(self->appGlobals->callerIdManager);

    if (SvArrayCount(callsData) == 0) {
        return;
    }

    QBActiveArrayInsertObjectAtIndex(self->callsList, 0, (SvGenericObject) SvArrayAt(callsData, 0), NULL);
    int count = QBActiveArrayCount(self->callsList);
    if (count > QBCallerIdManagerGetHistorySize(self->appGlobals->callerIdManager)) {
        QBActiveArrayRemoveObjectAtIndex(self->callsList, count - 1, NULL);
    }
}

SvLocal bool
QBCallerIDHistoryNodeSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath)
{
    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) self_;
    if (!SvObjectIsInstanceOf(node_, SvHashTable_getType())) {
        return false;
    }

    QBCallerIDHistoryMenuPrepareSidemenu(self, node_);
    return true;
}

SvLocal SvType
QBCallerIDMenuHandler_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBCallerIDMenuHandlerDestroy
    };
    static SvType type = NULL;

    static const struct XMBItemController_t itemControllerMethods = {
        .createItem = QBCallerIDHistoryCreateItem,
        .setItemState = QBCallerIDHistorySetItemState
    };

    static const struct QBMenuEventHandler_ selectMethods = {
        .nodeSelected = QBCallerIDHistoryNodeSelected
    };

    static const struct QBCallerIdManagerListener_t callerIdManagerListenerMethods = {
        .callsDataChanged = QBCallerIDMenuCallsDataChanged
    };

    static const struct QBPeerEventReceiver_ eventReceiverMethods = {
        .handleEvent = QBCallerIDMenuHandlerHandlePeerEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBCallerIDMenuHandler",
                            sizeof(struct QBCallerIDMenuHandler_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &itemControllerMethods,
                            QBMenuEventHandler_getInterface(), &selectMethods,
                            QBCallerIdManagerListener_getInterface(), &callerIdManagerListenerMethods,
                            QBPeerEventReceiver_getInterface(), &eventReceiverMethods,
                            NULL);
    }

    return type;
}

void QBCallerIDMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
    if (!appGlobals->callerIdManager) {
        return;
    }

    QBActiveTreeNode mainNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("CallerIdHistory"));
    SvObject path = mainNode ? QBActiveTreeCreateNodePath(appGlobals->menuTree, QBActiveTreeNodeGetID(mainNode)) : NULL;
    if (!mainNode || !path) {
        return;
    }

    QBCallerIDMenuHandler self = (QBCallerIDMenuHandler) SvTypeAllocateInstance(QBCallerIDMenuHandler_getType(), NULL);
    self->appGlobals = appGlobals;
    self->mainNode = mainNode;

    //---------------------------------------------------------------------
    svSettingsPushComponent("CallerID.settings");
    self->itemConstructor = QBXMBItemConstructorCreate("MenuItem.Item", self->appGlobals->textRenderer);
    if (svSettingsIsWidgetDefined("MenuItem.focus")) {
        self->focus = QBFrameConstructorFromSM("MenuItem.focus");
    }
    if (svSettingsIsWidgetDefined("MenuItem.disabledFocus")) {
        self->disabledFocus = QBFrameConstructorFromSM("MenuItem.disabledFocus");
    }

    self->phoneIcon = SVRETAIN(svSettingsGetBitmap("MenuItem.Item", "phoneIcon"));
    svSettingsPopComponent();

    //---------------------------------------------------------------------
    self->callsList = QBActiveArrayCreate(QBCallerIdManagerGetHistorySize(self->appGlobals->callerIdManager), NULL);
    QBTreeProxy callsListProxy = QBTreeProxyCreate((SvGenericObject) self->callsList, (SvGenericObject) self->mainNode, NULL);
    QBActiveTreeMountSubTree(self->appGlobals->menuTree, (SvGenericObject)callsListProxy, path, NULL);
    SVRELEASE(callsListProxy);

    QBTreePathMapInsert(pathMap, path, (SvGenericObject) self, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setItemControllerForPath, path, (SvObject) self, NULL);
    SvInvokeInterface(QBMenu, menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);

    QBCallerIDHistoryMenuPrepare(self);

    QBCallerIdManagerAddListener(self->appGlobals->callerIdManager, (SvGenericObject) self);

    SVRELEASE(path);
    SVRELEASE(self);
}
