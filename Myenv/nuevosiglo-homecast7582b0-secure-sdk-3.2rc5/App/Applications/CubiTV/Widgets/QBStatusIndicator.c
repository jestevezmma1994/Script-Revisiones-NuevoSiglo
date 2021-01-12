/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "Widgets/QBStatusIndicator.h"

#include <SvCore/SvCommonDefs.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <settings.h>
#include <CUIT/Core/widget.h>

struct QBStatusIndicator_t {
    struct SvObject_ super_;
    SvWidget stateIcon;
};
typedef struct QBStatusIndicator_t *QBStatusIndicator;

SvLocal void
QBStatusIndicatorClean(SvApplication app, void *ptr)
{
    QBStatusIndicator self = ptr;
    SVRELEASE(self);
}

SvLocal void
QBStatusIndicatorDestroy(void *self_)
{
    /// Nothing to do
}

SvLocal SvType
QBStatusIndicator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBStatusIndicatorDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBStatusIndicator",
                            sizeof(struct QBStatusIndicator_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvWidget
QBStatusIndicatorCreate(SvApplication app, const char* widgetName)
{
    if (!app || !widgetName) {
        SvLogError("%s (%d): Bad arguments", __func__, __LINE__);
        return NULL;
    }

    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    if (!w) {
        SvLogWarning("%s: Can't create widget", __func__);
        return NULL;
    }

    QBStatusIndicator self = (QBStatusIndicator) SvTypeAllocateInstance(QBStatusIndicator_getType(), NULL);
    if (unlikely(!self)) {
        SvLogWarning("%s (%d): Can't allocate QBStatusIndicator", __func__, __LINE__);
        goto error;
    }
    self->stateIcon = NULL;

    w->prv = self;
    w->clean = QBStatusIndicatorClean;
    svWidgetSetFocusable(w, false);

    char buf[128];
    snprintf(buf, sizeof(buf), "%s.Icon", widgetName);
    SvBitmap bmp = svSettingsGetBitmap(buf, "bg");
    svWidgetSetBitmap(w, bmp);

    return w;

error:
    svWidgetDestroy(w);
    return NULL;
}

