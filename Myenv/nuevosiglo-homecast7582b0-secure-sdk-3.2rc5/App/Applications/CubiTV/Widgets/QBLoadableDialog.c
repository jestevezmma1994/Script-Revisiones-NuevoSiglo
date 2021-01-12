/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "Widgets/QBLoadableDialog.h"
#include <main.h>
#include <CUIT/Core/widget.h>
#include <QBWidgets/QBWaitAnimation.h>
#include <QBWidgets/QBDialog.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <stdbool.h>
#include <libintl.h>

void
QBLoadableDialogSetData(SvWidget dialog, SvString title, SvString text)
{
    const char* titleStr = NULL;
    const char* textStr = NULL;
    if (text)
        textStr = gettext(SvStringCString(text));
    if (title)
        titleStr = gettext(SvStringCString(title));
    QBDialogSetTitle(dialog, titleStr);
    QBDialogRemoveItem(dialog, SVSTRING("animation"));
    QBDialogAddLabel(dialog, SVSTRING("content"), textStr, SVSTRING("message"), 0);
    SvWidget button = QBDialogAddButton(dialog, SVSTRING("OK-button"), gettext("Hide"), 1);

    svWidgetSetFocus(button);
}

SvLocal bool
QBLoadableDialogNotificationCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    if (keyCode == QBKEY_BACK || keyCode == QBKEY_FUNCTION || (buttonTag && SvStringEqualToCString(buttonTag, "OK-button"))) {
        QBDialogBreak(dialog);
        return true;
    }

    return false;
}

SvWidget
QBLoadableDialogCreate(AppGlobals appGlobals, SvString title, SvErrorInfo *errorOut)
{
    if (!appGlobals) {
        SvErrorInfo error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                              "NULL argument passed");
        SvErrorInfoPropagate(error, errorOut);
        return NULL;
    }
    SvWidget helper = svWidgetCreateBitmap(appGlobals->res, 0, 0, NULL);
    QBDialogParameters params = {
        .app        = appGlobals->res,
        .controller = appGlobals->controller,
        .widgetName = "Dialog",
        .ownerId    = svWidgetGetId(helper)
    };
    SvWidget dialog = QBDialogLocalNew(&params, QBDialogLocalTypeFocusable);
    QBDialogSetNotificationCallback(dialog, helper, QBLoadableDialogNotificationCallback);
    unsigned int globalKeys[] = { QBKEY_BACK, QBKEY_FUNCTION, 0 };
    QBDialogSetGlobalKeys(dialog, globalKeys);
    svWidgetAttach(dialog, helper, 0, 0, 0);

    const char* titleStr = NULL;
    if (title)
        titleStr = gettext(SvStringCString(title));
    else
        titleStr = gettext("Please wait for result");

    QBDialogAddPanel(dialog, SVSTRING("content"), NULL, 1);
    QBDialogSetTitle(dialog, titleStr);

    SvWidget animation = QBWaitAnimationCreate(appGlobals->res, "Popup.Animation");

    QBDialogAddWidget(dialog, SVSTRING("content"), animation, SVSTRING("animation"), 3, "Dialog.Animation");

    return dialog;
}
