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

#include "QBParentalControlLockedTimeEditPane.h"

#include <libintl.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContainerPane.h>
#include <main.h>
#include <QBWidgets/QBComboBox.h>
#include <Widgets/QBTimeEditBox.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Windows/mainmenu.h>
#include <Menus/QBParentalControlMenu.h>

struct QBParentalControlLockedTimeEditPane_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBContainerPane containerPane;
    int settingsCtx;
    SvWidget optionCheckBox;
    SvWidget optionsCombo;
    SvWidget startTime, endTime;
    QBContextMenu ctxMenu;
    QBParentalControlMenuHandler menuHandler;
    QBParentalControlLockedTimeTask task;
    QBActiveTreeNode node;
};

SvLocal void
QBParentalControlLockedTimeEditPane__dtor__(void *ptr)
{
    QBParentalControlLockedTimeEditPane self = ptr;
    SVTESTRELEASE(self->containerPane);
    self->containerPane = NULL;
    SVTESTRELEASE(self->ctxMenu);
    self->ctxMenu = NULL;
    svWidgetDestroy(self->startTime);
    self->startTime = NULL;
    svWidgetDestroy(self->endTime);
    self->endTime = NULL;
    svWidgetDestroy(self->optionsCombo);
    self->optionsCombo = NULL;
    svWidgetDestroy(self->optionCheckBox);
    self->optionCheckBox = NULL;
    self->menuHandler = NULL;
    SVTESTRELEASE(self->task);
    self->task = NULL;
    SVTESTRELEASE(self->node);
    self->node = NULL;
}

SvLocal void
QBParentalControlLockedTimeEditPanetWarningCallback(void *self_, SvString id, QBBasicPane pane, QBBasicPaneItem item)
{
    QBParentalControlLockedTimeEditPane self = self_;
    QBContextMenuPopPane(self->ctxMenu);
}


SvLocal void
QBParentalControlLockedTimeEditPaneShowWarning(QBParentalControlLockedTimeEditPane self, SvString msg)
{
    if (!self || !msg || !self->ctxMenu) {
        SvLogInfo("[%s] Error(self:%p, msg:%p, ctxMenu:%p)", __func__, self, msg,
                  (self && self->ctxMenu ? self->ctxMenu : NULL));
    }
    svSettingsPushComponent("BasicPane.settings");

    QBBasicPane options = (QBBasicPane) SvTypeAllocateInstance(QBBasicPane_getType(), NULL);
    QBBasicPaneInit(options, self->appGlobals->res, self->appGlobals->scheduler, self->appGlobals->textRenderer,
                    self->ctxMenu, 3, SVSTRING("BasicPane"));

    SvString option = SvStringCreate(gettext("Cancel"), NULL);
    QBBasicPaneAddOption(options, SVSTRING("cancel"), option, QBParentalControlLockedTimeEditPanetWarningCallback, self);
    SVRELEASE(option);

    QBBasicPaneSetTitle(options, msg);

    QBContextMenuPushPane(self->ctxMenu, (SvGenericObject) options);

    svSettingsPopComponent();
    SVRELEASE(options);
}

SvLocal bool
QBParentalControlLockedTimeEditPaneUpdateTask(QBParentalControlLockedTimeEditPane self)
{
    QBParentalControlLockedTimeTask task = NULL;

    int option = SvValueGetInteger((SvValue) QBComboBoxGetValue(self->optionsCombo));
    int startTime = QBTimeEditBoxGetMinutes(self->startTime);
    int endTime = QBTimeEditBoxGetMinutes(self->endTime);

    if (startTime == endTime) {
        svSettingsPushComponent("ParentalControl.settings");
        SvString msg = SvStringCreate(gettext(svSettingsGetString("LockedTime.messages", "messageTasksWrongData")), NULL);
        if (msg) {
            QBParentalControlLockedTimeEditPaneShowWarning(self, msg);
            SVRELEASE(msg);
        }
        svSettingsPopComponent();
        return false;
    }

    if (!self->task) {   //< Add
        if (!(task = QBParentalControlLockedTimeTaskNew(self->appGlobals->pc, startTime, endTime, true, option))) {
            SvLogError("[%s] New task creation failed", __func__);
            return false;
        }

        if (QBParentalControlIsLockedTimeTaskExist(self->appGlobals->pc, task)) {
            svSettingsPushComponent("ParentalControl.settings");
            SvString msg = SvStringCreate(gettext(svSettingsGetString("LockedTime.messages", "messageTasksAlreadyExists")), NULL);
            if (msg) {
                QBParentalControlLockedTimeEditPaneShowWarning(self, msg);
                SVRELEASE(msg);
            }
            svSettingsPopComponent();
            SVRELEASE(task);
            return false;
        }

        QBParentalControlAddAndSaveLockedTimeTask(self->appGlobals->pc, task);

        QBActiveTreeNode node = NULL;
        if ((node = (QBActiveTreeNode) QBParentalControlMenuCreateLockedTimeNode(self->appGlobals, task))) {
            QBParentalControlMenuLockedTimeRemoveEmptyNode(self->appGlobals);
            QBParentalControlMenuUpdateLockedTimeTask(self->menuHandler, task, node);
            SVRELEASE(node);
        }

        SVRELEASE(task);
        task = NULL;
    } else { //< Edit
        self->task->startTime = startTime;
        self->task->endTime = endTime;
        self->task->option = option;

        QBParentalControlAddAndSaveLockedTimeTask(self->appGlobals->pc, NULL);
        QBActiveTreePropagateNodeChange(self->appGlobals->menuTree, self->node, NULL);
    }
    return true;
}

SvLocal void
QBParentalControlLockedTimeEditPaneContainerOnShow(void *ptr, QBContainerPane pane,
                                                   SvWidget frame)
{
    QBParentalControlLockedTimeEditPane self = ptr;
    if (self->optionCheckBox) {
        svWidgetAttach(frame, self->optionCheckBox, self->optionCheckBox->off_x,
                       self->optionCheckBox->off_y, 0);
    }
}

SvLocal void
QBParentalControlLockedTimeEditPaneContainerSetActive(void *ptr, QBContainerPane pane,
                                                      SvWidget frame)
{
    QBParentalControlLockedTimeEditPane self = ptr;
    if (self && self->optionsCombo) {
        svWidgetSetFocus(self->optionsCombo);
    }
}

SvLocal void
QBParentalControlLockedTimeEditPaneContainerOnHide(void *ptr, QBContainerPane pane,
                                                   SvWidget frame)
{
    QBParentalControlLockedTimeEditPane self = ptr;
    svWidgetDetach(self->optionCheckBox);
}

SvLocal void
QBParentalControlLockedTimeEditPaneShow(SvGenericObject self_)
{
    QBParentalControlLockedTimeEditPane self = (QBParentalControlLockedTimeEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->containerPane, show);
}

SvLocal void
QBParentalControlLockedTimeEditPaneHide(SvGenericObject self_, bool immediately)
{
    QBParentalControlLockedTimeEditPane self = (QBParentalControlLockedTimeEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->containerPane, hide, immediately);
}

SvLocal void
QBParentalControlLockedTimeEditPaneSetActive(SvGenericObject self_)
{
    QBParentalControlLockedTimeEditPane self = (QBParentalControlLockedTimeEditPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->containerPane, setActive);
}

SvLocal bool
QBParentalControlLockedTimeEditPaneHandleInputEvent(SvObject self_, SvObject src, SvInputEvent e)
{
    return false;
}

SvLocal SvString
QBParentalControlLockedTimeEditPaneOptionPrepare(void *self_, SvWidget combobox, SvGenericObject value)
{
    const char* option = QBParentalControlLimitedTimeDayToString(SvValueGetInteger((SvValue) value));
    if (option) {
        return SvStringCreate(gettext(option), NULL);
    }
    return NULL;
}

SvLocal bool
QBParentalControlLockedTimeEditPaneOptionInput(void *self_, SvWidget combobox, SvInputEvent e)
{
    QBParentalControlLockedTimeEditPane self = (QBParentalControlLockedTimeEditPane) self_;
    if (e->ch != QBKEY_ENTER) {
        return false;
    }
    if (combobox == self->optionsCombo) {
        if (QBParentalControlLockedTimeEditPaneUpdateTask(self)) {
            QBContextMenuPopPane(self->ctxMenu);
        }
        return true;
    }
    return false;
}

SvLocal void
QBParentalControlLockedTimeEditPaneFakeClean(SvApplication app, void *prv)
{
}

SvLocal bool
QBParentalControlLockedTimeEditPaneInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBParentalControlLockedTimeEditPane self = w->prv;
    if (e->ch == QBKEY_UP) {
        if (QBTimeEditBoxIsFocused(self->startTime)) {
            svWidgetSetFocus(self->optionsCombo);
        } else if (QBTimeEditBoxIsFocused(self->endTime)) {
            QBTimeEditBoxSetFocus(self->startTime);
        } else if (svWidgetIsFocused(self->optionsCombo)) {
            QBTimeEditBoxSetFocus(self->endTime);
        }
        return true;
    } else if (e->ch == QBKEY_DOWN) {
        if (svWidgetIsFocused(self->optionsCombo)) {
            QBTimeEditBoxSetFocus(self->startTime);
        } else if (QBTimeEditBoxIsFocused(self->startTime)) {
            QBTimeEditBoxSetFocus(self->endTime);
        } else if (QBTimeEditBoxIsFocused(self->endTime)) {
            svWidgetSetFocus(self->optionsCombo);
        }
        return true;
    } else if (e->ch == QBKEY_ENTER) {
        if (QBTimeEditBoxIsFocused(self->startTime) || QBTimeEditBoxIsFocused(self->endTime)) {
            if (QBParentalControlLockedTimeEditPaneUpdateTask(self)) {
                QBContextMenuPopPane(self->ctxMenu);
            }
        }
        return true;
    }
    return false;
}

SvLocal void
QBParentalControlLockedTimeEditPaneSetValues(QBParentalControlLockedTimeEditPane self)
{
    if (!self || !self->menuHandler) {
        SvLogError("[%s] Error(self:%p, menuHandler:%p)", __func__, self, (self ? self->menuHandler : NULL));
        return;
    }

    QBTimeEditBoxSetMinutes(self->startTime, self->task->startTime);
    QBTimeEditBoxSetMinutes(self->endTime, self->task->endTime);

    SvValue value = SvValueCreateWithInteger(self->task->option, NULL);
    QBComboBoxSetValue(self->optionsCombo, (SvGenericObject) value);
    SVRELEASE(value);
}

SvLocal void
QBParentalControlLockedTimeEditPaneCreate(QBParentalControlLockedTimeEditPane self)
{
    svSettingsRestoreContext(self->settingsCtx);

    if (!self->optionCheckBox && !(self->optionCheckBox = svSettingsWidgetCreate(self->appGlobals->res, "LockedTime"))) {
        svSettingsPopComponent();
        return;
    }
    self->optionCheckBox->off_x = svSettingsGetInteger("LockedTime", "xOffset", 0);
    self->optionCheckBox->off_y = svSettingsGetInteger("LockedTime", "yOffset", 0);
    self->optionCheckBox->prv = self;
    self->optionCheckBox->clean = QBParentalControlLockedTimeEditPaneFakeClean;
    svWidgetSetInputEventHandler(self->optionCheckBox, QBParentalControlLockedTimeEditPaneInputEventHandler);
    svWidgetSetFocusable(self->optionCheckBox, true);
    self->optionsCombo = QBComboBoxNewFromSM(self->appGlobals->res, "LockedTime.OptionsComboBox");
    svWidgetSetFocusable(self->optionsCombo, true);

    QBComboBoxCallbacks cb = { QBParentalControlLockedTimeEditPaneOptionPrepare, QBParentalControlLockedTimeEditPaneOptionInput, NULL, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(self->optionsCombo, self, cb);
    svSettingsWidgetAttach(self->optionCheckBox, self->optionsCombo, svWidgetGetName(self->optionsCombo), 1);

    if ((self->startTime = QBTimeEditBoxNew(self->appGlobals->res, "LockedTime.startTime"))) {
        svSettingsWidgetAttach(self->optionCheckBox, self->startTime, "LockedTime.startTime", 1);
    }
    if ((self->endTime = QBTimeEditBoxNew(self->appGlobals->res, "LockedTime.endTime"))) {
        svSettingsWidgetAttach(self->optionCheckBox, self->endTime, "LockedTime.endTime", 2);
    }

    SvArray values = SvArrayCreate(NULL);
    for (int i = 0; i < QBParentalControlLockedTime_end; ++i) {
        SvGenericObject v = (SvGenericObject) SvValueCreateWithInteger(i, NULL);
        SvArrayAddObject(values, v);
        SVRELEASE(v);
    }
    QBComboBoxSetContent(self->optionsCombo, values);
    SVRELEASE(values);
    QBComboBoxSetPosition(self->optionsCombo, QBParentalControlLockedTime_mondays);

    SvWidget w = QBAsyncLabelNew(self->appGlobals->res, "LockedTime.startTime.desc", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(self->optionCheckBox, w, "LockedTime.startTime.desc", 1);
    QBAsyncLabelSetCText(w, gettext("Start time"));

    w = QBAsyncLabelNew(self->appGlobals->res, "LockedTime.endTime.desc", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(self->optionCheckBox, w, "LockedTime.endTime.desc", 1);
    QBAsyncLabelSetCText(w, gettext("End time"));

    w = QBAsyncLabelNew(self->appGlobals->res, "LockedTime.desc", self->appGlobals->textRenderer);
    svSettingsWidgetAttach(self->optionCheckBox, w, "LockedTime.desc", 1);
    QBAsyncLabelSetCText(w, gettext("Options"));

    svSettingsPopComponent();
}

SvType
QBParentalControlLockedTimeEditPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBParentalControlLockedTimeEditPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show             = QBParentalControlLockedTimeEditPaneShow,
        .hide             = QBParentalControlLockedTimeEditPaneHide,
        .setActive        = QBParentalControlLockedTimeEditPaneSetActive,
        .handleInputEvent = QBParentalControlLockedTimeEditPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBParentalControlLockedTimeEditPane",
                            sizeof(struct QBParentalControlLockedTimeEditPane_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }
    return type;
}

void
QBParentalControlLockedTimeEditPaneInit(QBParentalControlLockedTimeEditPane self, AppGlobals appGlobals,
                                        QBContextMenu ctxMenu, SvString widgetName, int level, void* menuHandler, QBParentalControlLockedTimeTask task,
                                        QBActiveTreeNode node)
{
    SVTESTRELEASE(self->ctxMenu);

    self->settingsCtx = svSettingsSaveContext();
    self->appGlobals = appGlobals;
    self->ctxMenu = SVRETAIN(ctxMenu);
    self->optionCheckBox = NULL;
    self->menuHandler = menuHandler;
    self->task = SVTESTRETAIN(task);
    self->node = SVTESTRETAIN(node);

    self->containerPane = (QBContainerPane) SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
        .onShow    = QBParentalControlLockedTimeEditPaneContainerOnShow,
        .onHide    = QBParentalControlLockedTimeEditPaneContainerOnHide,
        .setActive = QBParentalControlLockedTimeEditPaneContainerSetActive,
    };

    QBParentalControlLockedTimeEditPaneCreate(self);
    QBContainerPaneInit(self->containerPane, appGlobals->res, ctxMenu, level, widgetName, &moreInfoCallbacks, self);

    if (task) {
        QBParentalControlLockedTimeEditPaneSetValues(self);
    }
}
