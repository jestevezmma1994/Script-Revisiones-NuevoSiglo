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

#include "confirmationDialog.h"

#include <libintl.h>
#include <settings.h>
#include <QBWidgets/QBDialog.h>
#include <Widgets/QBScrollBar.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBWidgets/QBGrid.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <QBInput/QBInputCodes.h>
#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SWL/viewport.h>

struct QBConfirmationDialog_t {
    SvWidget dialog;
    SvWidget okButton;
    SvWidget cancelButton;
    SvWidget helper;
    SvWidget scrollBar;
    SvWidget viewPort;
    SvWidget grid;

    SvWidget questionLabel;
};
typedef struct QBConfirmationDialog_t* QBConfirmationDialog;

SvLocal bool QBConfirmationDialogNotificationCallback(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    QBConfirmationDialog self = ptr;
    if (self->dialog) {
        if(!buttonTag)
            return false;
        self->dialog = NULL;
        QBDialogBreak(dialog);
        return true;
    }

    return false;
}


SvLocal bool QBConfirmationDialogInputEventHandler(SvWidget w, SvInputEvent ev)
{
    SvWidget viewPort = QBDialogGetWidget(w, SVSTRING("viewport"));
    if (viewPort) {
        if (ev->type == QBInputEventType_keyPressed) {
            switch (ev->fullInfo.u.key.code) {
                case QBKEY_UP:
                    svViewPortStartScrollingVertically(viewPort, false);
                    return true;
                case QBKEY_DOWN:
                    svViewPortStartScrollingVertically(viewPort, true);
                    return true;
            }
        } else if (ev->type == QBInputEventType_keyReleased) {
            switch (ev->fullInfo.u.key.code) {
                case QBKEY_UP:
                case QBKEY_DOWN:
                    svViewPortStopScrollingVertically(viewPort);
                    return true;
            }
        }
    }

    return false;
}

SvWidget QBConfirmationDialogCreate(SvApplication app, const QBConfirmationDialogParams_t *params)
{
    SvWidget dialog = NULL;
    SvWidget okButton = NULL;
    SvWidget cancelButton = NULL;
    SvWidget helper = NULL;

    if (params == NULL)
        return NULL;

    QBConfirmationDialog self = calloc(1, sizeof(struct QBConfirmationDialog_t));

    helper = svWidgetCreateBitmap(app, 0, 0, NULL);

    svSettingsPushComponent("ConfirmationDialog.settings");

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBApplicationController controller =
        (QBApplicationController) QBServiceRegistryGetService(registry, SVSTRING("QBApplicationController"));
    QBDialogParameters dialogParams = {
        .app        = app,
        .controller = controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper),
    };

    if (params->local) {
        dialog = QBDialogLocalNew(&dialogParams, QBDialogLocalTypeFocusable);
    } else {
        dialog = QBDialogGlobalNew(&dialogParams, params->globalName);
    }
    svWidgetAttach(dialog, helper, 0, 0, 0);
    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, params->title);
    QBDialogSetNotificationCallback(dialog, self, QBConfirmationDialogNotificationCallback);

    svSettingsPopComponent();

    if (svSettingsIsWidgetDefined("LongMessageDialog")) {

        const char* dialogName = "LongMessageDialog";

        SvWidget viewPort = NULL;
        SvWidget scrollBar = NULL;
        SvWidget grid = NULL;

        const size_t newFileNameSize = strlen(dialogName) + 15;
        char nameBuff[newFileNameSize];
        snprintf(nameBuff, newFileNameSize, "%s.ScrollBar", dialogName);
        scrollBar = QBScrollBarNew(dialog->app, nameBuff);
        svSettingsWidgetAttach(dialog, scrollBar, nameBuff, 1);

        snprintf(nameBuff, newFileNameSize, "%s.Grid", dialogName);
        grid = QBGridCreate(dialog->app, nameBuff);

        snprintf(nameBuff, newFileNameSize, "%s.AsyncLabel", dialogName);
        QBTextRenderer textRenderer = (QBTextRenderer) QBServiceRegistryGetService(registry, SVSTRING("QBTextRenderer"));
        SvWidget label = QBGridAddAsyncLabel(grid, nameBuff, textRenderer);
        SvString text = SvStringCreate(params->message, NULL);
        QBAsyncLabelSetText(label, text);
        SVRELEASE(text);

        snprintf(nameBuff, newFileNameSize, "%s.ViewPort", dialogName);
        viewPort = svViewPortNew(dialog->app, nameBuff);
        QBDialogAddWidget(dialog, SVSTRING("content"), viewPort, SVSTRING("viewport"), 1, nameBuff);
        svViewPortSetContents(viewPort, grid);
        svViewPortAddRangeListener(viewPort,
                QBScrollBarGetRangeListener(scrollBar),
                SvViewPortRangeOrientation_VERTICAL);
        svWidgetSetFocusable(viewPort, false);

        self->viewPort = viewPort;
        self->grid = grid;
        self->scrollBar = scrollBar;
    }

    if (!self->viewPort)
        QBDialogAddLabel(dialog, SVSTRING("content"), params->message, SVSTRING("message"), 0);

    okButton = QBDialogAddButton(dialog, SVSTRING("OK-button"), params->labelOK ? params->labelOK : gettext("OK"), 1);

    QBDialogSetInputEventHandlerCallback(dialog, QBConfirmationDialogInputEventHandler);

    self->dialog = dialog;
    self->helper = helper;
    self->okButton = okButton;

    if (params->isCancelButtonVisible)
    {
        cancelButton = QBDialogAddButton(dialog, SVSTRING("cancel-button"), params->labelCancel ? params->labelCancel : gettext("Cancel"), 2);
        self->cancelButton = cancelButton;
    }

    helper->prv = self;

    if (params->focusOK || !params->isCancelButtonVisible)
        svWidgetSetFocus(okButton);
    else
        svWidgetSetFocus(cancelButton);

    return dialog;
}
