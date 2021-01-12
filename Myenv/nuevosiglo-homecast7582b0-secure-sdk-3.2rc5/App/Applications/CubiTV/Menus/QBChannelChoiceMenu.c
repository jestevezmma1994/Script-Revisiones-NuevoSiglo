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

#include "QBChannelChoiceMenu.h"
#include <SvCore/SvCommonDefs.h>
#include <CAGE/Core/SvSurface.h>
#include <settings.h>
#include <SWL/QBFrame.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <XMB2/XMBItemController.h>

struct QBChannelChoiceController_t {
    struct SvObject_ super_;

    SvBitmap tickMarkOn;
    SvBitmap tickMarkOff;
    SvBitmap noTickMark;
    SvBitmap focusBitmap;

    void *callbackData;
    QBChannelChoiceControllerCallbacks callbacks;
    SvGenericObject list;

    // applying standard XMB interface
    QBXMBItemConstructor itemConstructor;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
};

SvLocal SvWidget QBChannelChoiceControllerCreateItem(SvGenericObject self_, SvGenericObject node, SvGenericObject path, SvApplication app, XMBMenuState initialState)
{
    QBChannelChoiceController self = (QBChannelChoiceController) self_;

    SvString caption = NULL;
    ChannelChoiceTickState state = self->callbacks->isTicked(self->callbackData, node);

    if (self->callbacks->createNodeCaption) {
        caption = self->callbacks->createNodeCaption(self->callbackData, node);
    }

    // if previous attempt failed (or createNodeCaption is false), try with node as SvString
    if (!caption) {
        if (SvObjectIsInstanceOf(node, SvString_getType())) {
            caption = SVRETAIN((SvString) node);
        } else
            caption = SVSTRING("???");
    }

    SvWidget box = NULL;
    SvBitmap icon;

    if (state == ChannelChoiceTickState_On) {
        icon = SVRETAIN(self->tickMarkOn);
    } else if (state == ChannelChoiceTickState_Off) {
        icon = SVRETAIN(self->tickMarkOff);
    } else {
        icon = SVRETAIN(self->noTickMark);
    }

    QBXMBItem item = QBXMBItemCreate();
    item->caption = caption;
    item->icon = icon;
    item->focus = SVRETAIN(self->focus);
    item->inactiveFocus = SVTESTRETAIN(self->inactiveFocus);

    box = QBXMBItemConstructorCreateItem(self->itemConstructor, item, app, initialState);

    SVRELEASE(item);
    return box;
}

SvLocal void QBChannelChoiceControllerSetItemState(SvGenericObject self_, SvWidget item_, XMBMenuState state, bool isFocused)
{
    QBChannelChoiceController self = (QBChannelChoiceController) self_;
    QBXMBItemConstructorSetItemState(self->itemConstructor, item_, state, isFocused);
}

SvLocal void QBChannelChoiceControllerDestroy(void *self_)
{
    QBChannelChoiceController self = self_;

    if (self->itemConstructor)
        QBXMBItemConstructorDestroy(self->itemConstructor);
    SVRELEASE(self->tickMarkOn);
    SVRELEASE(self->tickMarkOff);
    SVRELEASE(self->noTickMark);
    SVRELEASE(self->list);
    SVRELEASE(self->focus);
    SVTESTRELEASE(self->inactiveFocus);
    SVTESTRELEASE(self->focusBitmap);
}

SvLocal SvType QBChannelChoiceController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelChoiceControllerDestroy
    };
    static SvType type = NULL;
    static const struct XMBItemController_t controller_methods = {
        .createItem = QBChannelChoiceControllerCreateItem,
        .setItemState = QBChannelChoiceControllerSetItemState,
    };
    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelChoiceController",
                            sizeof(struct QBChannelChoiceController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            XMBItemController_getInterface(), &controller_methods,
                            NULL);
    }
    return type;
}

QBChannelChoiceController QBChannelChoiceControllerCreateFromSettings(const char *settings, QBTextRenderer renderer, SvGenericObject list, void* callbackData, QBChannelChoiceControllerCallbacks callbacks)
{
    QBChannelChoiceController self = (QBChannelChoiceController) SvTypeAllocateInstance(QBChannelChoiceController_getType(), NULL);

    svSettingsPushComponent(settings);

    self->tickMarkOn = SVRETAIN(svSettingsGetBitmap("ChannelChoiceName", "tickMarkOn"));
    self->tickMarkOff = SVRETAIN(svSettingsGetBitmap("ChannelChoiceName", "tickMarkOff"));
    self->noTickMark = SVRETAIN(svSettingsGetBitmap("ChannelChoiceName", "noTickMark"));
    self->focusBitmap = SVRETAIN(svSettingsGetBitmap("ChannelChoiceName", "bg"));

    self->callbackData = callbackData;
    self->callbacks = callbacks;
    self->list = SVRETAIN(list);

    // standard XMB interface
    self->itemConstructor = QBXMBItemConstructorCreate("ChannelChoiceName", renderer);
    self->focus = QBFrameConstructorFromSM("ChannelChoiceName.focus");

    const char *inactiveFocusWidgetName = "ChannelChoiceName.inactiveFocus";
    if (svSettingsIsWidgetDefined(inactiveFocusWidgetName)) {
        self->inactiveFocus = QBFrameConstructorFromSM(inactiveFocusWidgetName);
    }

    svSettingsPopComponent();

    return self;
}
