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

#include "QBNPvrOptionsSelectPane.h"

#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContainerPane.h>
#include <NPvr/QBnPVRRecording.h>
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

typedef enum {
    QBNPvrOptionsSelectPanePremiere_AllEpisodes,
    QBNPvrOptionsSelectPanePremiere_NewOnly
} QBNPvrOptionsSelectPanePremiere;

struct QBNPvrOptionsSelectPane_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBContextMenu contextMenu;
    int level;
    QBNPvrOptionsSelectPaneType paneType;
    int optionValue;

    QBContainerPane mainPane;

    SvWidget optionsWidget;
    SvWidget optionsCombo;

    int xOffset, yOffset;

    QBNPvrOptionsSelectPaneApproveCallback approveCallback;
    void* approveParam;
};

SvLocal void QBNPvrOptionsSelectPane__dtor__(void *ptr)
{
    QBNPvrOptionsSelectPane self = (QBNPvrOptionsSelectPane) ptr;
    SVRELEASE(self->mainPane);
    svWidgetDestroy(self->optionsWidget);
}

SvLocal SvType QBNPvrOptionsSelectPane_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBNPvrOptionsSelectPane__dtor__
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBNPvrOptionsSelectPane",
                            sizeof(struct QBNPvrOptionsSelectPane_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvLocal void QBNPvrOptionsSelectPaneOnShow(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrOptionsSelectPane self = self_;
    svWidgetAttach(frame, self->optionsWidget, self->xOffset, self->yOffset, 0);
}

SvLocal void QBNPvrOptionsSelectPaneOnHide(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrOptionsSelectPane self = self_;
    svWidgetDetach(self->optionsWidget);
}

SvLocal void QBNPvrOptionsSelectPaneSetActive(void *self_, QBContainerPane pane, SvWidget frame)
{
    QBNPvrOptionsSelectPane self = self_;
    svWidgetSetFocus(self->optionsCombo);

    if (self->paneType == QBNPvrOptionsSelectPaneType_Premiere) {
        if (self->optionValue == (int) QBNPvrOptionsSelectPanePremiere_AllEpisodes) {
            QBComboBoxSetPosition(self->optionsCombo, 0);
        } else if (self->optionValue == (int) QBNPvrOptionsSelectPanePremiere_NewOnly) {
            QBComboBoxSetPosition(self->optionsCombo, 1);
        }
    } else if (self->paneType == QBNPvrOptionsSelectPaneType_SpaceRecovery) {
        if (self->optionValue == (int) QBnPVRRecordingSpaceRecovery_DeleteOldest) {
            QBComboBoxSetPosition(self->optionsCombo, 0);
        } else if (self->optionValue == (int) QBnPVRRecordingSpaceRecovery_Manual) {
            QBComboBoxSetPosition(self->optionsCombo, 1);
        }
    }
}

SvString TranslateOptionValue(QBNPvrOptionsSelectPaneType paneType, int optionValue)
{
    SvString ret = NULL;

    if (paneType == QBNPvrOptionsSelectPaneType_Premiere) {
        if (optionValue == (int) QBNPvrOptionsSelectPanePremiere_AllEpisodes) {
            ret = SvStringCreate(gettext("All Episodes"), NULL);
        } else if (optionValue == (int) QBNPvrOptionsSelectPanePremiere_NewOnly) {
            ret = SvStringCreate(gettext("New Only"), NULL);
        }
    } else if (paneType == QBNPvrOptionsSelectPaneType_SpaceRecovery) {
        if (optionValue == (int) QBnPVRRecordingSpaceRecovery_DeleteOldest) {
            ret = SvStringCreate(gettext("Delete as needed"), NULL);
        } else if (optionValue == (int) QBnPVRRecordingSpaceRecovery_Manual) {
            ret = SvStringCreate(gettext("Delete manually"), NULL);
        }
    } else {
        ret = SvStringCreate(gettext("Unknown"), NULL);
    }

    return ret;
}

SvLocal SvString QBNPvrOptionsSelectPaneComboPrepare(void *self_, SvWidget combobox, SvObject value)
{
    QBNPvrOptionsSelectPane self = self_;
    SvValue valInt = (SvValue) value;
    int val = SvValueGetInteger(valInt);
    SvString ret = TranslateOptionValue(self->paneType, val);
    return ret;
}

SvLocal bool QBNPvrOptionsSelectPaneComboInput(void *self_, SvWidget combobox, SvInputEvent e)
{
    const int key = e->ch;
    QBNPvrOptionsSelectPane self = self_;
    if (key == QBKEY_ENTER) {
        SvValue optionValue = (SvValue) QBComboBoxGetValue(self->optionsCombo);
        int option = SvValueGetInteger(optionValue);
        if (self->approveCallback != NULL) {
            self->approveCallback(self->approveParam, (QBnPVRRecordingSpaceRecovery) option);
        }
        QBContextMenuPopPane(self->contextMenu);
        return true;
    } else if (key == QBKEY_DOWN || key == QBKEY_UP) {
        svWidgetSetFocus(self->optionsCombo);
        return true;
    }
    return false;
}

QBNPvrOptionsSelectPane QBNPvrOptionsSelectPaneCreate(
    AppGlobals appGlobals,
    QBContextMenu contextMenu,
    int level,
    QBNPvrOptionsSelectPaneType paneType
    )
{
    //SAVE DATA
    QBNPvrOptionsSelectPane self = (QBNPvrOptionsSelectPane) SvTypeAllocateInstance(QBNPvrOptionsSelectPane_getType(), NULL);
    self->appGlobals = appGlobals;
    self->contextMenu = contextMenu;
    self->level = level;
    self->paneType = paneType;

    //default value of option depends on pane type
    if (self->paneType == QBNPvrOptionsSelectPaneType_Premiere) {
        self->optionValue = (int) QBNPvrOptionsSelectPanePremiere_AllEpisodes;
    } else if (self->paneType == QBNPvrOptionsSelectPaneType_SpaceRecovery) {
        self->optionValue = (int) QBnPVRRecordingSpaceRecovery_DeleteOldest;
    }

    self->xOffset = 0;
    self->yOffset = 0;

    static struct QBContainerPaneCallbacks_t callbacks = {
        .onHide    = QBNPvrOptionsSelectPaneOnHide,
        .onShow    = QBNPvrOptionsSelectPaneOnShow,
        .setActive = QBNPvrOptionsSelectPaneSetActive,
    };

    //CREATE MAIN PANE
    self->mainPane = QBContainerPaneCreateFromSettings("NpvrOptions.settings", self->appGlobals->res,
                                                       self->contextMenu, self->level, SVSTRING("NpvrOptionsPane"),
                                                       &callbacks, self);

    svSettingsPushComponent("NpvrOptions.settings");

    self->xOffset = svSettingsGetInteger("NpvrOptions", "xOffset", 0);
    self->yOffset = svSettingsGetInteger("NpvrOptions", "yOffset", 0);

    //CREATE START MARGIN WIDGETS
    self->optionsWidget = svSettingsWidgetCreate(self->appGlobals->res, "NpvrOptions");

    SvWidget title = QBAsyncLabelNew(self->appGlobals->res, "NpvrOptions.Title", self->appGlobals->textRenderer);

    svSettingsWidgetAttach(self->optionsWidget, title, svWidgetGetName(title), 1);

    if (self->paneType == QBNPvrOptionsSelectPaneType_Premiere) {
        QBAsyncLabelSetCText(title, gettext("Premiere"));
    } else if (self->paneType == QBNPvrOptionsSelectPaneType_SpaceRecovery) {
        QBAsyncLabelSetCText(title, gettext("Space Recovery"));
    }

    self->optionsCombo = QBComboBoxNewFromSM(self->appGlobals->res, "NpvrOptions.ComboBox");

    svSettingsWidgetAttach(self->optionsWidget, self->optionsCombo, svWidgetGetName(self->optionsCombo), 1);

    svWidgetSetFocusable(self->optionsCombo, true);
    QBComboBoxCallbacks cb = { QBNPvrOptionsSelectPaneComboPrepare, QBNPvrOptionsSelectPaneComboInput, NULL, NULL, NULL, NULL };
    QBComboBoxSetCallbacks(self->optionsCombo, self, cb);

    SvArray values = SvArrayCreate(NULL);
    SvValue value1 = NULL;
    SvValue value2 = NULL;

    if (self->paneType == QBNPvrOptionsSelectPaneType_Premiere) {
        value1 = SvValueCreateWithInteger((int) QBNPvrOptionsSelectPanePremiere_AllEpisodes, NULL);
        SvArrayAddObject(values, (SvObject) value1);

        value2 = SvValueCreateWithInteger((int) QBNPvrOptionsSelectPanePremiere_NewOnly, NULL);
        SvArrayAddObject(values, (SvObject) value2);
    } else if (self->paneType == QBNPvrOptionsSelectPaneType_SpaceRecovery) {
        value1 = SvValueCreateWithInteger((int) QBnPVRRecordingSpaceRecovery_DeleteOldest, NULL);
        SvArrayAddObject(values, (SvObject) value1);

        value2 = SvValueCreateWithInteger((int) QBnPVRRecordingSpaceRecovery_Manual, NULL);
        SvArrayAddObject(values, (SvObject) value2);
    }

    SVTESTRELEASE(value1);
    SVTESTRELEASE(value2);

    QBComboBoxSetContent(self->optionsCombo, values);
    SVRELEASE(values);

    QBComboBoxSetPosition(self->optionsCombo, 0);

    svSettingsPopComponent();

    return self;
}

void QBNPvrOptionsSelectPaneShow(QBNPvrOptionsSelectPane self, int optionValue)
{
    self->optionValue = optionValue;
    QBContextMenuPushPane(self->contextMenu, (SvObject) self->mainPane);
}

void QBNPvrOptionsSelectPaneSetApproveCallback(QBNPvrOptionsSelectPane self, QBNPvrOptionsSelectPaneApproveCallback callback, void* callbackParam)
{
    self->approveCallback = callback;
    self->approveParam = callbackParam;
}
