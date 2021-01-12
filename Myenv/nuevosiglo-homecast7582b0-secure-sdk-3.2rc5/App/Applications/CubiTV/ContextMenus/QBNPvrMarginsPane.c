/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBNPvrMarginsPane.h"

#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContainerPane.h>
#include <QBConf.h>
#include <QBInput/QBInputCodes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvValue.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBComboBox.h>
#include <libintl.h>
#include <main.h>
#include <settings.h>

#define MARGIN_INTERVAL 5
#define MAX_MARGIN 15

struct QBNPvrMarginsPane_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBContextMenu contextMenu;
    int level;

    QBContainerPane mainPane;

    SvWidget startMarginWidget;
    SvWidget startMarginCombo;

    int startXOffset, startYOffset;

    SvWidget endMarginWidget;
    SvWidget endMarginCombo;

    int endXOffset, endYOffset;

    QBNPvrMarginsPaneApproveCallback hideCallback;
    void* hideParam;
};

SvLocal void QBNPvrMarginsPane__dtor__(void *ptr)
{
    QBNPvrMarginsPane self = (QBNPvrMarginsPane) ptr;
    SVRELEASE(self->mainPane);

    svWidgetDestroy(self->startMarginWidget);
    svWidgetDestroy(self->endMarginWidget);
}

SvLocal SvType QBNPvrMarginsPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNPvrMarginsPane__dtor__
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrMarginsPane",
                            sizeof(struct QBNPvrMarginsPane_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBNPvrMarginsPaneOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrMarginsPane self = self_;
    svWidgetAttach(frame, self->startMarginWidget, self->startXOffset, self->startYOffset, 0);
    svWidgetAttach(frame, self->endMarginWidget, self->endXOffset, self->endYOffset, 0);
}

SvLocal void QBNPvrMarginsPaneOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrMarginsPane self = self_;
    svWidgetDetach(self->startMarginWidget);
    svWidgetDetach(self->endMarginWidget);

    if (self->hideCallback != NULL) {
        self->hideCallback(self->hideParam);
    }
}

SvLocal void QBNPvrMarginsPaneSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrMarginsPane self = self_;
    svWidgetSetFocus(self->startMarginCombo);

    const char* startMargin = QBConfigGet("NPVRSTARTMARGIN");
    long startMins = strtol(startMargin, NULL, 10);
    int startIndex = startMins / MARGIN_INTERVAL;
    QBComboBoxSetPosition(self->startMarginCombo, startIndex);

    const char* endMargin = QBConfigGet("NPVRENDMARGIN");
    long endMins = strtol(endMargin, NULL, 10);
    int endIndex = endMins / MARGIN_INTERVAL;
    QBComboBoxSetPosition(self->endMarginCombo, endIndex);
}

SvLocal SvString QBNPvrMarginsPaneComboPrepare(void *self_, SvWidget combobox, SvObject value)
{
    SvValue valInt = (SvValue) value;
    int val = SvValueGetInteger(valInt);
    if (val == 0) {
        return SvStringCreate(gettext("Disabled"), NULL);
    }
    return SvStringCreateWithFormat(gettext("%d min"), val);
}

SvLocal void QBNPvrMarginsPaneSave(QBNPvrMarginsPane self)
{
    SvValue startMarginVal = (SvValue) QBComboBoxGetValue(self->startMarginCombo);
    int startMargin = SvValueGetInteger(startMarginVal);
    char *buf = NULL;
    asprintf(&buf, "%d", startMargin);
    QBConfigSet("NPVRSTARTMARGIN", buf);
    free(buf);

    SvValue endMarginVal = (SvValue) QBComboBoxGetValue(self->endMarginCombo);
    int endMargin = SvValueGetInteger(endMarginVal);
    buf = NULL;
    asprintf(&buf, "%d", endMargin);
    QBConfigSet("NPVRENDMARGIN", buf);
    free(buf);
}

SvLocal bool QBNPvrMarginsPaneStartComboInput(void *self_, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBNPvrMarginsPane self = self_;
    if (key == QBKEY_DOWN || key == QBKEY_UP) {
        svWidgetSetFocus(self->endMarginCombo);
        return true;
    }
    if (key == QBKEY_ENTER) {
        QBNPvrMarginsPaneSave(self);
        QBContextMenuPopPane(self->contextMenu);
        return true;
    }
    return false;
}

SvLocal bool QBNPvrMarginsPaneEndComboInput(void *self_, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBNPvrMarginsPane self = self_;
    if (key == QBKEY_DOWN || key == QBKEY_UP) {
        svWidgetSetFocus(self->startMarginCombo);
        return true;
    }
    if (key == QBKEY_ENTER) {
        QBNPvrMarginsPaneSave(self);
        QBContextMenuPopPane(self->contextMenu);
        return true;
    }
    return false;
}

QBNPvrMarginsPane QBNPvrMarginsPaneCreate(
    AppGlobals appGlobals,
    QBContextMenu contextMenu,
    int level
    )
{
    //SAVE DATA
    QBNPvrMarginsPane self = (QBNPvrMarginsPane) SvTypeAllocateInstance(QBNPvrMarginsPane_getType(), NULL);
    self->appGlobals = appGlobals;
    self->contextMenu = contextMenu;
    self->level = level;
    self->startXOffset = 0;
    self->startYOffset = 0;
    self->endXOffset = 0;
    self->endYOffset = 0;

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBNPvrMarginsPaneOnHide,
        .onShow    = QBNPvrMarginsPaneOnShow,
        .setActive = QBNPvrMarginsPaneSetActive,
    };

    //CREATE MAIN PANE
    self->mainPane = QBContainerPaneCreateFromSettings("NpvrMargins.settings", self->appGlobals->res,
                                                       self->contextMenu, self->level, SVSTRING("NpvrMarginsPane"),
                                                       &callbacks, self);

    svSettingsPushComponent("NpvrMargins.settings");

    self->startXOffset = svSettingsGetInteger("NpvrStartMargin", "xOffset", 0);
    self->startYOffset = svSettingsGetInteger("NpvrStartMargin", "yOffset", 0);
    self->endXOffset = svSettingsGetInteger("NpvrEndMargin", "xOffset", 0);
    self->endYOffset = svSettingsGetInteger("NpvrEndMargin", "yOffset", 0);

    //CREATE START MARGIN WIDGETS
    self->startMarginWidget = svSettingsWidgetCreate(self->appGlobals->res, "NpvrStartMargin");

    SvWidget title = QBAsyncLabelNew(self->appGlobals->res, "NpvrStartMargin.Title", self->appGlobals->textRenderer);

    svSettingsWidgetAttach(self->startMarginWidget, title, svWidgetGetName(title), 1);
    QBAsyncLabelSetCText(title, gettext("Start Margin"));

    self->startMarginCombo = QBComboBoxNewFromSM(self->appGlobals->res, "NpvrStartMargin.ComboBox");

    svSettingsWidgetAttach(self->startMarginWidget, self->startMarginCombo, svWidgetGetName(self->startMarginCombo), 1);

    svWidgetSetFocusable(self->startMarginCombo, true);
    {
        QBComboBoxCallbacks cb = { QBNPvrMarginsPaneComboPrepare, QBNPvrMarginsPaneStartComboInput, NULL, NULL, NULL, NULL };
        QBComboBoxSetCallbacks(self->startMarginCombo, self, cb);
    }

    SvArray values = SvArrayCreate(NULL);

    for (int i = 0; i <= MAX_MARGIN; i += MARGIN_INTERVAL) {
        SvValue val = SvValueCreateWithInteger(i, NULL);
        SvArrayAddObject(values, (SvObject) val);
        SVRELEASE(val);
    }

    QBComboBoxSetContent(self->startMarginCombo, values);
    QBComboBoxSetPosition(self->startMarginCombo, 0);
    SVRELEASE(values);

    //CREATE END MARGIN WIDGETS
    self->endMarginWidget = svSettingsWidgetCreate(self->appGlobals->res, "NpvrEndMargin");

    title = QBAsyncLabelNew(self->appGlobals->res, "NpvrEndMargin.Title", self->appGlobals->textRenderer);

    svSettingsWidgetAttach(self->endMarginWidget, title, svWidgetGetName(title), 1);
    QBAsyncLabelSetCText(title, gettext("End Margin"));

    self->endMarginCombo = QBComboBoxNewFromSM(self->appGlobals->res, "NpvrEndMargin.ComboBox");

    svSettingsWidgetAttach(self->endMarginWidget, self->endMarginCombo, svWidgetGetName(self->endMarginCombo), 1);

    svWidgetSetFocusable(self->endMarginCombo, true);
    {
        QBComboBoxCallbacks cb = {QBNPvrMarginsPaneComboPrepare, QBNPvrMarginsPaneEndComboInput, NULL, NULL, NULL, NULL};
        QBComboBoxSetCallbacks(self->startMarginCombo, self, cb);
    }

    values = SvArrayCreate(NULL);

    for (int i = 0; i <= MAX_MARGIN; i += MARGIN_INTERVAL) {
        SvValue val = SvValueCreateWithInteger(i, NULL);
        SvArrayAddObject(values, (SvObject) val);
        SVRELEASE(val);
    }

    QBComboBoxSetContent(self->endMarginCombo, values);
    QBComboBoxSetPosition(self->endMarginCombo, 0);
    SVRELEASE(values);

    svSettingsPopComponent();

    return self;
}

void QBNPvrMarginsPaneShow(QBNPvrMarginsPane self)
{
    QBContextMenuPushPane(self->contextMenu, (SvObject) self->mainPane);
}

void QBNPvrMarginsPaneSetApproveCallback(QBNPvrMarginsPane self, QBNPvrMarginsPaneApproveCallback callback, void* callbackParam)
{
    self->hideCallback = callback;
    self->hideParam = callbackParam;
}
