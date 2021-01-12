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

#include "QBRoundedLabel.h"
#include <SvCore/SvCommonDefs.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <SWL/label.h>

struct QBRoundedLabel_t {
    SvWidget label;
    SvWidget box;
    int margin;

    SvString text;
};
typedef struct QBRoundedLabel_t *QBRoundedLabel;

SvLocal void
QBRoundedLabelCleanup(SvApplication app, void* ptr)
{
    QBRoundedLabel self = (QBRoundedLabel)ptr;

    SVTESTRELEASE(self->text);
    free(self);
}

SvLocal void QBRoundedLabelSetLabelData(SvWidget label)
{
    QBRoundedLabel self = (QBRoundedLabel)label->prv;
    svLabelSetText(self->label, SvStringCString(self->text));
    int tmpWidth = 2 * self->label->off_x + svLabelGetWidth(self->label) + self->margin;
    QBFrameSetWidth(self->box, tmpWidth);
}

SvWidget QBRoundedLabelNew(SvApplication app, char const *widgetName)
{
    SvWidget label;
    QBRoundedLabel self = calloc(1, sizeof(*self));
    if (!self)
        return NULL;

    label = svSettingsWidgetCreate(app, widgetName);
    label->clean = QBRoundedLabelCleanup;
    label->prv = self;

    self->margin = svSettingsGetInteger(widgetName, "margin", 0);

    char *nameBuf = NULL;
    if (asprintf(&nameBuf, "%s.box", widgetName) < 0)
        goto fini;
    self->box = QBFrameCreateFromSM(app, nameBuf);
    svSettingsWidgetAttach(label, self->box, nameBuf, 1);
    free(nameBuf);

    svWidgetSetFocusable(self->box, false);

    if (asprintf(&nameBuf, "%s.label", widgetName) < 0)
        goto fini;
    self->label = svLabelNewFromSM(app, nameBuf);
    svSettingsWidgetAttach(self->box, self->label, nameBuf, 1);

    free(nameBuf);

    self->text = SvStringCreate("00:00:00", NULL);
    if (!self->text)
        goto fini;

    QBRoundedLabelSetLabelData(label);

    return label;

fini:
    svWidgetDestroy(label);
    return NULL;
}

void QBRoundedLabelSetText(SvWidget label, SvString text)
{
    if (!label || !text)
        return;

    QBRoundedLabel self = (QBRoundedLabel)label->prv;
    if (!self)
        return;

    if (self->text && SvObjectEquals((SvObject) self->text, (SvObject) text))
        return;

    SVTESTRELEASE(self->text);
    self->text = SVRETAIN(text);
    QBRoundedLabelSetLabelData(label);
}

