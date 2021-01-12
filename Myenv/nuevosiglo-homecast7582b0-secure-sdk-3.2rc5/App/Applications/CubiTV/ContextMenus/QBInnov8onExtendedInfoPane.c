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

#include "QBInnov8onExtendedInfoPane.h"

#include <stdio.h>

#include <settings.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <SWL/icon.h>
#include <SWL/viewport.h>
#include <main.h>
#include <Logic/QBParentalControlLogic.h>
#include <ContextMenus/QBContextMenu.h>
#include <ContextMenus/QBContainerPane.h>
#include <QBWidgets/QBGrid.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <Utils/dbobject.h>
#include <Utils/value.h>
#include <QBPCRatings/QBPCUtils.h>
#include <SvCore/SvCoreErrorDomain.h>

struct QBInnov8onExtendedInfo_ {
    SvWidget viewport;
    SvWidget grid;
    SvWidget title;
    SvWidget description;
    SvWidget image;
};
typedef struct QBInnov8onExtendedInfo_ *QBInnov8onExtendedInfo;

struct QBInnov8onExtendedInfoPane_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    QBContainerPane container;
    int settingsCtx;

    SvWidget extendedInfo;

    SvGenericObject product;
};

SvLocal void
QBInnov8onExtendedInfoPaneAddPCRatingsInfoToMovieDescription(SvDBRawObject movie, SvStringBuffer titleBuf)
{
    if (!movie || !titleBuf)
        return;

    SvString ratingsStr = SvValueTryGetString((SvValue) SvDBRawObjectGetAttrValue(movie, "ratings"));
    if (!ratingsStr || SvStringGetLength(ratingsStr) == 0)
        return;

    SvErrorInfo error = NULL;
    QBPCList movieRatings = QBPCListCreate(&error);
    if (error)
        goto err;

    QBPCListLockRatingsFromString(movieRatings, ratingsStr, &error);
    if (error)
        goto err;

    QBParentalControlLogic pcLogic =
            (QBParentalControlLogic) QBGlobalStorageGetItem(QBGlobalStorageGetInstance(), QBParentalControlLogic_getType(), &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "QBGlobalStorageGetItem() failed");
        goto err;
    }

    if (!pcLogic) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidState,
                                  "Could not get instance of QBParentalControlLogic from global storage");
        goto err;
    }

    SvString pcRatingsDescription = QBParentalControlLogicCreateDescriptionStringFromPCList(pcLogic, movieRatings, &error);
    if (error)
        goto err;

    if (pcRatingsDescription) {
        SvStringBufferAppendFormatted(titleBuf, NULL, " (%s)", SvStringCString(pcRatingsDescription));
        SVRELEASE(pcRatingsDescription);
    }

    SVRELEASE(movieRatings);
    return;

err:
    if (error) {
        SVTESTRELEASE(movieRatings);
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
    }
}

SvLocal void
QBInnov8onExtendedInfoSetObject(SvWidget w, SvGenericObject object)
{
    QBInnov8onExtendedInfo prv = w->prv;

    SvDBRawObject movie = (SvDBRawObject) object;
    if (movie) {
        SvString description = NULL;
        SvStringBuffer titleBuf = SvStringBufferCreate(NULL);
        SvString imageURI = NULL;

        const char *description_ = SvDBRawObjectGetStringAttrValue(movie, "extended_description", NULL);
        const char *description2 = SvDBRawObjectGetStringAttrValue(movie, "description", "");
        if (description_)
            description = SvStringCreate(description_, NULL);
        else if (description2)
            description = SvStringCreate(description2, NULL);

        const char *title_ = SvDBRawObjectGetStringAttrValue(movie, "title", NULL);
        const char *title2 = SvDBRawObjectGetStringAttrValue(movie, "name", NULL);
        if (title_)
            SvStringBufferAppendCString(titleBuf, title_, NULL);
        else if (title2)
            SvStringBufferAppendCString(titleBuf, title2, NULL);

        QBInnov8onExtendedInfoPaneAddPCRatingsInfoToMovieDescription(movie, titleBuf);
        SvString title = SvStringBufferCreateContentsString(titleBuf, NULL);
        SVRELEASE(titleBuf);

        const char *img = SvDBRawObjectGetStringAttrValue(movie, "thumbnail", NULL);
        const char *img2 = SvDBRawObjectGetStringAttrValue(movie, "media", NULL);
        if (img)
            imageURI = SvStringCreate(img, NULL);
        else if (img2)
            imageURI = SvStringCreate(img2, NULL);

        QBAsyncLabelSetText(prv->description, description);
        QBAsyncLabelSetText(prv->title, title);

        if(imageURI) {
            svWidgetSetHidden(prv->image, false);
            unsigned int idx = (svIconGetBitmapsCount(prv->image) <= 0) ? 0 : 1;
            svIconSetBitmapFromURI(prv->image, idx, SvStringCString(imageURI));
            svIconSwitch(prv->image, idx, 0, 0.0);
        } else {
            svWidgetSetHidden(prv->image, true);
        }

        SVTESTRELEASE(title);
        SVTESTRELEASE(description);
        SVTESTRELEASE(imageURI);
    } else {
        QBAsyncLabelSetText(prv->description, NULL);
        QBAsyncLabelSetText(prv->title, NULL);
    }
    svViewPortSetContentsPosition(prv->viewport, 0, 0, true);
}

SvLocal void
QBInnov8onExtendedInfoFocusEventHandler(SvWidget w, SvFocusEvent e)
{
    QBInnov8onExtendedInfo prv = w->prv;

    if (e->kind == SvFocusEventKind_GET)
        svWidgetSetFocus(prv->viewport);
}

SvLocal SvWidget
QBInnov8onExtendedInfoCreate(AppGlobals appGlobals , const char *widgetName)
{
    QBInnov8onExtendedInfo prv = calloc(1, sizeof(struct QBInnov8onExtendedInfo_));
    if (!prv) {
        SvLogError("%s() : No memory", __func__);
        return NULL;
    }

    SvApplication app = appGlobals->res;
    QBTextRenderer textRenderer = appGlobals->textRenderer;

    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->prv = prv;
    svWidgetSetFocusable(w, true);
    svWidgetSetFocusEventHandler(w, QBInnov8onExtendedInfoFocusEventHandler);

    char buf[256];

    snprintf(buf, 256, "%s.grid", widgetName);
    prv->grid = QBGridCreate(app, buf);

    snprintf(buf, 256, "%s.viewport", widgetName);
    prv->viewport = svViewPortNew(app, buf);
    svSettingsWidgetAttach(w, prv->viewport, buf, 0);
    svViewPortSetContents(prv->viewport, prv->grid);

    snprintf(buf, 256, "%s.title", widgetName);
    prv->title = QBGridAddAsyncLabel(prv->grid, buf, textRenderer);

    snprintf(buf, 256, "%s.description", widgetName);
    prv->description = QBGridAddAsyncLabel(prv->grid, buf, textRenderer);

    snprintf(buf, 256, "%s.image", widgetName);
    prv->image = QBGridAddIcon(prv->grid, buf);

    return w;
}

// ExtendedInfo pane

SvLocal void
QBInnov8onExtendedInfoPaneContainerOnShow(void *ptr,
                                          QBContainerPane pane,
                                          SvWidget frame)
{
    QBInnov8onExtendedInfoPane self = ptr;
    svSettingsRestoreContext(self->settingsCtx);
    svSettingsWidgetAttach(frame, self->extendedInfo, "ExtendedInfo", 0);
    svSettingsPopComponent();
    QBInnov8onExtendedInfoSetObject(self->extendedInfo, self->product);
}

SvLocal void
QBInnov8onExtendedInfoPaneContainerSetActive(void *ptr,
                                             QBContainerPane pane,
                                             SvWidget frame)
{
    QBInnov8onExtendedInfoPane self = ptr;
    svWidgetSetFocus(self->extendedInfo);
}

SvLocal void
QBInnov8onExtendedInfoPaneContainerOnHide(void *ptr,
                                          QBContainerPane pane,
                                          SvWidget frame)
{
    QBInnov8onExtendedInfoPane self = ptr;
    svWidgetDetach(self->extendedInfo);
}

SvLocal void
QBInnov8onExtendedInfoPaneShow(SvGenericObject self_)
{
    QBInnov8onExtendedInfoPane self = (QBInnov8onExtendedInfoPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, show);
}

SvLocal void
QBInnov8onExtendedInfoPaneHide(SvGenericObject self_, bool immediately)
{
    QBInnov8onExtendedInfoPane self = (QBInnov8onExtendedInfoPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, hide, immediately);
}

SvLocal void
QBInnov8onExtendedInfoPaneSetActive(SvGenericObject self_)
{
    QBInnov8onExtendedInfoPane self = (QBInnov8onExtendedInfoPane) self_;
    SvInvokeInterface(QBContextMenuPane, self->container, setActive);
}

SvLocal bool
QBInnov8onExtendedInfoPaneHandleInputEvent(SvObject self_,
                                           SvObject src,
                                           SvInputEvent e)
{
    return false;
}

SvLocal void
QBInnov8onExtendedInfoPane__dtor__(void *ptr)
{
    QBInnov8onExtendedInfoPane self = ptr;
    SVRELEASE(self->container);
    svWidgetDestroy(self->extendedInfo);
    SVTESTRELEASE(self->product);
}

SvType
QBInnov8onExtendedInfoPane_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBInnov8onExtendedInfoPane__dtor__
    };
    static SvType type = NULL;
    static const struct QBContextMenuPane_ methods = {
        .show             = QBInnov8onExtendedInfoPaneShow,
        .hide             = QBInnov8onExtendedInfoPaneHide,
        .setActive        = QBInnov8onExtendedInfoPaneSetActive,
        .handleInputEvent = QBInnov8onExtendedInfoPaneHandleInputEvent
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBInnov8onExtendedInfoPane",
                            sizeof(struct QBInnov8onExtendedInfoPane_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBContextMenuPane_getInterface(), &methods,
                            NULL);
    }

    return type;
}

void
QBInnov8onExtendedInfoPaneInit(QBInnov8onExtendedInfoPane self,
                               AppGlobals appGlobals,
                               QBContextMenu ctxMenu,
                               SvString widgetName,
                               int level,
                               SvGenericObject product)
{
    self->settingsCtx = svSettingsSaveContext();
    self->appGlobals = appGlobals;
    self->extendedInfo = QBInnov8onExtendedInfoCreate(appGlobals, "ExtendedInfo");
    self->product = SVTESTRETAIN(product);

    self->container = (QBContainerPane)
        SvTypeAllocateInstance(QBContainerPane_getType(), NULL);
    static struct QBContainerPaneCallbacks_t moreInfoCallbacks = {
            .onShow = QBInnov8onExtendedInfoPaneContainerOnShow,
            .onHide = QBInnov8onExtendedInfoPaneContainerOnHide,
            .setActive = QBInnov8onExtendedInfoPaneContainerSetActive,
    };
    QBContainerPaneInit(self->container, appGlobals->res, ctxMenu, level,
                        widgetName, &moreInfoCallbacks, self);
}

QBInnov8onExtendedInfoPane
QBInnov8onExtendedInfoPaneCreateFromSettings(const char *settings,
                                             AppGlobals appGlobals,
                                             QBContextMenu ctxMenu,
                                             SvString widgetName,
                                             int level,
                                             SvGenericObject product)
{
    svSettingsPushComponent(settings);
    QBInnov8onExtendedInfoPane self = (QBInnov8onExtendedInfoPane)
        SvTypeAllocateInstance(QBInnov8onExtendedInfoPane_getType(), NULL);
    QBInnov8onExtendedInfoPaneInit(self, appGlobals, ctxMenu, widgetName,
                                   level, product);
    svSettingsPopComponent();

    return self;
}

