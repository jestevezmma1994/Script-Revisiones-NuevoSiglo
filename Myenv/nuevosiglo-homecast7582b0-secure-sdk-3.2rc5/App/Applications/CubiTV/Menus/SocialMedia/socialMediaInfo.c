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

#include "socialMediaInfo.h"
#include "socialMediaSlot.h"

#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SWL/QBFrame.h>
#include <SWL/icon.h>
#include <QBStringUtils.h>
#include <ContextMenus/QBBasicPane.h>
#include <ContextMenus/QBContextMenu.h>
#include <QBContentManager/QBContentCategory.h>
#include <Menus/menuchoice.h>
#include <Widgets/QBXMBItemConstructor.h>
#include <Widgets/XMBCarousel.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Windows/mainmenu.h>
#include <Windows/pvrplayer.h>
#include <Utils/QBMovieUtils.h>

struct  QBSocialMediaInfo_t {
    SvWidget        owner;
    SvWidget        content;
    SvWidget        contentImage;

    SvWidget        slot;

    SvApplication   app;

    unsigned int    ctxID;
    QBTextRenderer  textRenderer;
    SvRID           tmpBmpRID;
    SvRID           tmpCategoryBmpRID;

    SvGenericObject object;
};
typedef struct QBSocialMediaInfo_t *QBSocialMediaInfo;

static SvHashTable controllers = NULL;

SvLocal void
QBSocialMediaInfoRegisterControllers(QBSocialMediaInfo self)
{
    controllers = SvHashTableCreate(20, NULL);
    SvGenericObject controller = NULL;

    controller = (SvGenericObject) QBSocialMediaItemDetailsSlotControllerCreate(self->app, self->textRenderer, SVSTRING("ItemDetails"));
    SvHashTableInsert(controllers, (SvGenericObject) SVSTRING("ITEM_DETAILS"), controller);
    SVRELEASE(controller);

    controller = (SvGenericObject) QBSocialMediaItemDetailsSlotControllerCreate(self->app, self->textRenderer, SVSTRING("ItemDetailsNoPic"));
    SvHashTableInsert(controllers, (SvGenericObject) SVSTRING("ITEM_DETAILS_NO_PIC"), controller);
    SVRELEASE(controller);

    controller = (SvGenericObject) QBSocialMediaItemDetailsSlotControllerCreate(self->app, self->textRenderer, SVSTRING("ItemDetailsNoText"));
    SvHashTableInsert(controllers, (SvGenericObject) SVSTRING("ITEM_DETAILS_NO_TEXT"), controller);
    SVRELEASE(controller);

    controller = (SvGenericObject) QBSocialMediaProfileSlotControllerCreate(self->app, self->textRenderer);
    SvHashTableInsert(controllers, (SvGenericObject) SVSTRING("PROFILE"), controller);
    SVRELEASE(controller);

    controller = (SvGenericObject) QBSocialMediaGallerySlotControllerCreate(self->app, self->textRenderer);
    SvHashTableInsert(controllers, (SvGenericObject) SVSTRING("GALLERY"), controller);
    SVRELEASE(controller);

    controller = (SvGenericObject) QBSocialMediaPictureSlotControllerCreate(self->app, self->textRenderer);
    SvHashTableInsert(controllers, (SvGenericObject) SVSTRING("PICTURE"), controller);
    SVRELEASE(controller);
}

SvLocal void
QBSocialMediaInfoCleanup( SvApplication app, void* ptr )
{
    QBSocialMediaInfo prv = (QBSocialMediaInfo) ptr;
    SVTESTRELEASE(prv->object);
    free(ptr);
}

SvWidget
SocialMediaInfoCreate(SvApplication app, QBTextRenderer textRenderer)
{
    QBSocialMediaInfo prv = calloc(1, sizeof(struct QBSocialMediaInfo_t));

    prv->app                = app;
    prv->textRenderer       = textRenderer;

    {
        prv->tmpBmpRID          = svSettingsGetResourceID("MoviePage.ContentImage", "bg");
        prv->tmpCategoryBmpRID  = svSettingsGetResourceID("MoviePage.CategoryImage", "bg");
    }

    if (!controllers)
        QBSocialMediaInfoRegisterControllers(prv);

    SvWidget w              = svSettingsWidgetCreate(app, "MoviePage.Background");
    w->prv                  = prv;
    prv->owner              = w;
    w->clean                = QBSocialMediaInfoCleanup;

    prv->content            = svSettingsWidgetCreate(prv->app, "MoviePage.Background");
    svWidgetAttach(prv->owner, prv->content, 0, 0, 3);

    prv->ctxID              = svSettingsSaveContext();

    return w;
}

SvLocal void
QBSocialMediaInfoSetupWidget(QBSocialMediaInfo self, SvString presentationScheme, SvGenericObject object)
{
    if (presentationScheme) {
        SvGenericObject controller = SvHashTableFind(controllers, (SvGenericObject) presentationScheme);
        if (controller) {
            if (!self->slot) {
                self->slot = SvInvokeInterface(QBSocialMediaInfoSlot, controller, createWidget, self->content->width, self->content->height);
                svWidgetAttach(self->content, self->slot, 0, 0, 1);
            }
            SvInvokeInterface(QBSocialMediaInfoSlot, controller, setObject, self->slot, object);
        } else {
            SvLogError("%s unrecognized presentationScheme: '%s'", __func__, SvStringCString(presentationScheme));
        }
    }
}

SvLocal void
SocialMediaInfoClearWidgets(QBSocialMediaInfo self)
{
    svWidgetDestroy(self->content);
    self->content = NULL;
    svWidgetDestroy(self->contentImage);
    self->contentImage = NULL;
    self->slot = NULL;

    self->content = svSettingsWidgetCreate(self->app, "MoviePage.Background");
    svWidgetAttach(self->owner, self->content, 0, 0, 3);
}

SvLocal bool
ObjectsCompatible(SvGenericObject objA, SvGenericObject objB)
{
    if (!objA && !objB)
        return true;
    if (!objA || !objB)
        return false;
    if (SvObjectGetType(objA) == SvObjectGetType(objB)) {
        if (SvObjectIsInstanceOf(objA, QBContentCategory_getType())) {
            return true;
        } else if (SvObjectIsInstanceOf((SvObject) objA, SvDBRawObject_getType())) {
            SvString typeA = QBMovieUtilsGetAttr((SvDBRawObject) objA, "type", NULL);
            SvString typeB = QBMovieUtilsGetAttr((SvDBRawObject) objB, "type", NULL);
            if (typeA && typeB && SvObjectEquals((SvObject) typeA, (SvObject) typeB)) {
                SvString schemeA = QBMovieUtilsGetAttr((SvDBRawObject) objA, "presentation_scheme", NULL);
                SvString schemeB = QBMovieUtilsGetAttr((SvDBRawObject) objB, "presentation_scheme", NULL);
                if (SvObjectEquals((SvObject) schemeA, (SvObject) schemeB))
                    return true;
            }
        }
    }

    return false;
}

SvLocal void
QBSocialMediaInfoCreateContentImage(QBSocialMediaInfo self, const char* settings)
{
    if (!self->contentImage) {
        self->contentImage  = svIconNew(self->app, settings);
        self->tmpBmpRID = svSettingsGetResourceID(settings, "bg");
        svSettingsWidgetAttach(self->owner, self->contentImage, settings, 5);
    }

    svIconSetBitmapFromRID(self->contentImage, 0, self->tmpBmpRID);
}

SvLocal void
QBSocialMediaInfoCreateSetupCategoryWidget(QBSocialMediaInfo self, QBContentCategory category)
{
    svSettingsPushComponent("Carousel_WEBTV.settings");
    QBSocialMediaInfoCreateContentImage(self, "MoviePage.ContentImage");

    svIconSetBitmapFromRID(self->contentImage, 0, self->tmpCategoryBmpRID);

    SvValue uriV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("thumbnail"));
    if (unlikely(!uriV)) {
        uriV = (SvValue) QBContentCategoryGetAttribute(category, SVSTRING("contentURI"));
    }
    if (uriV && SvObjectIsInstanceOf((SvObject) uriV, SvValue_getType()) && SvValueIsString(uriV)) {
        SvString uri = SvValueGetString(uriV);
        svIconSetBitmapFromURI(self->contentImage, 0, SvStringCString(uri));
        svIconSwitch(self->contentImage, 0, 0, -1.0);
    }

    svSettingsPopComponent();
}

void
SocialMediaInfoSetObject(SvWidget w, SvGenericObject object)
{
    QBSocialMediaInfo prv = (QBSocialMediaInfo) w->prv;

    svSettingsRestoreContext(prv->ctxID);

    if (!ObjectsCompatible(prv->object, object)) {
        SocialMediaInfoClearWidgets(prv);
    }

    SVTESTRETAIN(object);
    SVTESTRELEASE(prv->object);
    prv->object = object;

    if(!object) {
        svSettingsPopComponent();
        return;
    } else {
        SvValue presentationScheme = NULL;
        SvGenericObject presentationDetails = NULL;
        if (SvObjectIsInstanceOf((SvObject) object, SvDBRawObject_getType())) {
            presentationScheme = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) object, "presentation_scheme");
            if (presentationScheme && SvValueIsString(presentationScheme)) {
                presentationDetails = SvDBRawObjectGetAttrValue((SvDBRawObject) object, "presentation_details");
                QBSocialMediaInfoSetupWidget(prv, SvValueGetString(presentationScheme), presentationDetails);
            }
        } else if (SvObjectIsInstanceOf(object, QBContentCategory_getType())) {
            presentationScheme = (SvValue) QBContentCategoryGetAttribute((QBContentCategory) object , SVSTRING("presentation_scheme"));
            if (presentationScheme && SvValueIsString(presentationScheme)) {
                presentationDetails = QBContentCategoryGetAttribute((QBContentCategory) object, SVSTRING("presentation_details"));
                QBSocialMediaInfoSetupWidget(prv, SvValueGetString(presentationScheme), presentationDetails);
            } else {
                QBSocialMediaInfoCreateSetupCategoryWidget(prv, (QBContentCategory) object);
            }
        } else {
            //any other type received
        }
    }

    svSettingsPopComponent();
}

SvInterface
QBSocialMediaInfoSlot_getInterface(void)
{
    static SvInterface interface = NULL;
    if(!interface) {
        SvInterfaceCreateManaged("SocialMediaInfoSlot",
                                 sizeof(struct QBSocialMediaInfoSlot_t),
                                 NULL, &interface, NULL);
    }
    return interface;
}

