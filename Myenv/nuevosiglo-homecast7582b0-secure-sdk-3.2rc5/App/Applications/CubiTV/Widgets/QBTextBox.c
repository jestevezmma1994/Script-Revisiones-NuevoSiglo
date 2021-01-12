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

#include <Widgets/QBTextBox.h>
#include <Widgets/QBTitle.h>
#include <Widgets/QBScrollView.h>

#include <CUIT/Core/event.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/widget.h>
#include <settings.h>

#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBActiveArray.h>

#include <SWL/label.h>

#define log_debug(fmt, ...)  if (0) SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(cyan), __func__, __LINE__, ## __VA_ARGS__)

typedef struct QBTextBox_ *QBTextBox;

struct QBTextBox_ {
    struct SvObject_ super_;

    SvApplication app;

    SvWidget scrollView;
    QBActiveArray text;
    SvString textWidgetName;
    int paragraphLimit;
};

SvLocal void
QBTextBoxDestroy(void *self_)
{
    QBTextBox self = self_;

    SVTESTRELEASE(self->text);
    self->text = NULL;
    SVTESTRELEASE(self->textWidgetName);
    self->textWidgetName = NULL;
}

SvLocal void
QBTextBoxClean(SvApplication app, void *ptr)
{
    QBTextBox self = ptr;

    SVRELEASE(self);
}

SvLocal SvWidget
QBTextBoxItemControllerCreateItem(SvObject self_,
                                  SvApplication app,
                                  int width, int minHeight)
{
    QBTextBox self = (QBTextBox) self_;
    // every svLabel is created during QBTextBoxCreate() and it uses settings context from there
    return svLabelNewFromSM(app, SvStringGetCString(self->textWidgetName));
}

SvLocal void
QBTextBoxItemControllerSetObject(SvObject self_,
                                 SvWidget item_,
                                 SvObject object)
{
    if (SvObjectIsInstanceOf(object, SvString_getType())) {
        SvString text = (SvString) object;
        svLabelSetText(item_, SvStringGetCString(text));
    }
}

SvLocal SvType
QBTextBox_getType(void)
{
    static const struct QBScrollViewItemController_t controllerMethods = {
        .createItem = QBTextBoxItemControllerCreateItem,
        .setObject  = QBTextBoxItemControllerSetObject,
    };

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTextBoxDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTextBox",
                            sizeof(struct QBTextBox_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBScrollViewItemController_getInterface(), &controllerMethods,
                            NULL);
    }

    return type;
}

SvWidget
QBTextBoxCreate(SvApplication app, const char* widgetName, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBTextBox self = NULL;

    self = (QBTextBox) SvTypeAllocateInstance(QBTextBox_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "Can't allocate QBTextBox");
        goto err;
    }

    if (!svSettingsIsWidgetDefined(widgetName)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Widget [%s] not found", widgetName);
        goto err;
    }

    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->prv = self;
    w->clean = QBTextBoxClean;

    self->paragraphLimit = svSettingsGetInteger(widgetName, "paragraphLimit", 250);
    self->text = QBActiveArrayCreate(1, NULL);

    self->textWidgetName = SvStringCreateWithFormat("%s.text", widgetName);
    if (!svSettingsIsWidgetDefined(SvStringGetCString(self->textWidgetName))) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Widget [%s] not found", SvStringGetCString(self->textWidgetName));
        goto err;
    }

    self->scrollView = QBScrollViewCreate(app, widgetName, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "Couldn't create widget [%s]", widgetName);
        goto err;
    }

    QBScrollViewSetItemController(self->scrollView, (SvObject) self, NULL);
    QBScrollViewConnectToSource(self->scrollView, (SvObject) self->text);
    svSettingsWidgetAttach(w, self->scrollView, widgetName, 1);

    return w;

err:
    SVTESTRELEASE(self);
    SvErrorInfoPropagate(error, errorOut);
    return NULL;
}

void
QBTextBoxAddText(SvWidget w, SvString text)
{
    QBTextBox self = w->prv;
    QBTextBoxAddTextWithLimit(w, text, self->paragraphLimit);
}

void
QBTextBoxAddTextWithLimit(SvWidget w, SvString text, int paragraphLimit)
{
    QBTextBox self = w->prv;

    const char* rawStr = SvStringGetCString(text);
    size_t begining = 0, end = 0;
    ssize_t currentParagraphLength = 0;

    while (rawStr[end]) {
        currentParagraphLength++;
        if (rawStr[end] == '\n' || rawStr[end] == '\r') {
            SvString lineToAdd = SvStringCreateSubString(text, begining, currentParagraphLength - 1, NULL);
            QBActiveArrayAddObject(self->text, (SvObject) lineToAdd, NULL);
            SVRELEASE(lineToAdd);
            currentParagraphLength = 0;
            if (rawStr[end] == '\r' && rawStr[end + 1] == '\n') {
                end = end + 2; // Windows OS newline
            } else {
                end++; // Unix newline
            }
            begining = end;
        } else if (currentParagraphLength >= paragraphLimit) {
            SvString lineToAdd = SvStringCreateSubString(text, begining, currentParagraphLength, NULL);
            QBActiveArrayAddObject(self->text, (SvObject) lineToAdd, NULL);
            SVRELEASE(lineToAdd);
            currentParagraphLength = 0;
            end++;
            begining = end;
        } else {
            end++;
        }
    }

    if (currentParagraphLength > 0) {
        SvString lineToAdd = SvStringCreateSubString(text, begining, currentParagraphLength, NULL);
        QBActiveArrayAddObject(self->text, (SvObject) lineToAdd, NULL);
        SVRELEASE(lineToAdd);
    }
}

void
QBTextBoxSetHeight(SvWidget w, int height)
{
    QBTextBox self = w->prv;
    w->height = height;
    self->scrollView->height = height;
    SvWidget viewport = QBScrollViewGetViewport(self->scrollView);
    viewport->height = height;
}
