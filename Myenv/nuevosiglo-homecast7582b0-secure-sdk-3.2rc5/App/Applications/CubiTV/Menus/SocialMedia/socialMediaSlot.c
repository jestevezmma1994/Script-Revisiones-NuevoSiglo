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

#include "socialMediaSlot.h"

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvHashTable.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <SWL/icon.h>
#include <QBWidgets/QBAsyncLabel.h>

#include "socialMediaInfo.h"

// QBSocialMediaIconWithText

struct QBSocialMediaIconWithText_t {
    SvWidget icon;
    SvWidget text;

    SvString iconKey;
    SvString labelKey;
};
typedef struct QBSocialMediaIconWithText_t *QBSocialMediaIconWithText;

SvLocal void
QBSocialMediaIconWithTextClean(SvApplication app, void *ptr)
{
    QBSocialMediaIconWithText prv = ptr;

    SVRELEASE(prv->iconKey);
    SVRELEASE(prv->labelKey);

    free(ptr);
}

SvLocal SvWidget
QBSocialMediaIconWithTextCreate(SvApplication app, QBTextRenderer textRenderer, const char* widgetName, SvString iconKey, SvString labelKey)
{
    QBSocialMediaIconWithText prv = calloc(1, sizeof(struct QBSocialMediaIconWithText_t));
    if (!prv) {
        SvLogError("%s : out of memory", __func__);
        return NULL;
    }

    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->clean = QBSocialMediaIconWithTextClean;
    w->prv = prv;
    prv->iconKey = SVRETAIN(iconKey);
    prv->labelKey = SVRETAIN(labelKey);

    char *tmp = NULL;

    asprintf(&tmp, "%s.Icon", widgetName);
    prv->icon = svIconNew(app, tmp);
    svSettingsWidgetAttach(w, prv->icon, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.Text", widgetName);
    prv->text = QBAsyncLabelNew(app, tmp, textRenderer);
    svSettingsWidgetAttach(w, prv->text, tmp, 1);
    free(tmp);

    return w;
}

SvLocal void
QBSocialMediaIconWithTextSetObject(SvWidget w, SvGenericObject object)
{
    QBSocialMediaIconWithText prv = w->prv;
    SvHashTable data = (SvHashTable) object;
    SvValue val = NULL;

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) prv->iconKey);
    if (val && SvValueIsString(val)) {
        const char *icon = SvValueGetStringAsCString(val, NULL);
        svIconSetBitmapFromURI(prv->icon, 0, icon);
    }

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) prv->labelKey);
    if (val && SvValueIsString(val)) {
        SvString text = SvValueGetString(val);
        QBAsyncLabelSetText(prv->text, text);
    } else if (val && SvValueIsInteger(val)) {
        int n = SvValueGetInteger(val);
        SvString buf = SvStringCreateWithFormat("%d", n);
        QBAsyncLabelSetText(prv->text, buf);
        SVRELEASE(buf);
    }
}

// QBSocialMediaComment

struct QBSocialMediaComment_t {
    SvWidget author;
    SvWidget date;
    SvWidget text;
    SvWidget likeIt;
};
typedef struct QBSocialMediaComment_t *QBSocialMediaComment;

SvLocal SvWidget
QBSocialMediaCommentCreate(SvApplication app, QBTextRenderer textRenderer, const char* widgetName)
{
    QBSocialMediaComment prv = calloc(1, sizeof(struct QBSocialMediaComment_t));
    if (!prv) {
        SvLogError("%s : out of memory", __func__);
        return NULL;
    }

    SvWidget w = svSettingsWidgetCreate(app, widgetName);
    w->prv = prv;

    char *tmp = NULL;

    asprintf(&tmp, "%s.Author", widgetName);
    prv->author = QBAsyncLabelNew(app, tmp, textRenderer);
    svSettingsWidgetAttach(w, prv->author, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.Text", widgetName);
    prv->text = QBAsyncLabelNew(app, tmp, textRenderer);
    svSettingsWidgetAttach(w, prv->text, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.Date", widgetName);
    prv->date = QBAsyncLabelNew(app, tmp, textRenderer);
    svSettingsWidgetAttach(w, prv->date, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.Counter", widgetName);
    prv->likeIt = QBSocialMediaIconWithTextCreate(app, textRenderer, tmp, SVSTRING("icon"), SVSTRING("value"));
    svSettingsWidgetAttach(w, prv->likeIt, tmp, 1);
    free(tmp);

    return w;
}

SvLocal void
QBSocialMediaCommentSetObject(SvWidget w, SvGenericObject object)
{
    QBSocialMediaComment prv = w->prv;
    SvHashTable data = (SvHashTable) object;
    SvValue val = NULL;
    SvString author = NULL, text = NULL, date = NULL;

    if (object) {
        svWidgetSetHidden(w, false);
    } else {
        svWidgetSetHidden(w, true);
        return;
    }

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("author"));
    if (val && SvValueIsString(val))
        author = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("text"));
    if (val && SvValueIsString(val))
        text = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("date"));
    if (val && SvValueIsString(val))
        date = SvValueGetString(val);

    QBSocialMediaIconWithTextSetObject(prv->likeIt, object);

    QBAsyncLabelSetText(prv->author, author);
    QBAsyncLabelSetText(prv->text, text);
    QBAsyncLabelSetText(prv->date, date);
}

// QBSocialMediaNewSlot

#define ITEM_INFO_COUNTERS_COUNT 2
#define ITEM_INFO_COMMENTS_COUNT 2

struct QBSocialMediaItemDetailsSlotController_t {
    struct SvObject_ super_;

    SvApplication  app;
    QBTextRenderer textRenderer;

    SvString widgetName;
};

struct QBSocialMediaItemDetailsSlot_t {
    SvWidget title;
    SvWidget text;
    SvWidget picture;
    SvWidget footerIcon;
    SvWidget footer;
    SvWidget counters[ITEM_INFO_COUNTERS_COUNT];
    SvWidget comments[ITEM_INFO_COMMENTS_COUNT];
};
typedef struct QBSocialMediaItemDetailsSlot_t *QBSocialMediaItemDetailsSlot;

SvLocal SvWidget
QBSocialMediaItemDetailsSlotCreateWidget(SvGenericObject self_, int width, int height)
{
    QBSocialMediaItemDetailsSlotController self = (QBSocialMediaItemDetailsSlotController) self_;

    QBSocialMediaItemDetailsSlot prv = calloc(1, sizeof(struct QBSocialMediaItemDetailsSlot_t));
    if (!prv) {
        SvLogError("%s : out of memory", __func__);
        return NULL;
    }

    SvWidget w = svWidgetCreateBitmap(self->app, width, height, NULL);
    w->prv = prv;
    const char *widgetName = SvStringCString(self->widgetName);
    char *tmp = NULL;

    svSettingsPushComponent("SocialMediaSlot.settings");

    asprintf(&tmp, "%s.Title", widgetName);
    prv->title = QBAsyncLabelNew(self->app, tmp, self->textRenderer);
    if (prv->title)
        svSettingsWidgetAttach(w, prv->title, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.Text", widgetName);
    prv->text = QBAsyncLabelNew(self->app, tmp, self->textRenderer);
    if (prv->text)
        svSettingsWidgetAttach(w, prv->text, tmp, 1);
    free(tmp);

    asprintf(&tmp, "%s.Picture", widgetName);
    if (svSettingsIsWidgetDefined(tmp)) {
        prv->picture = svIconNew(self->app, tmp);
        if (prv->picture)
            svSettingsWidgetAttach(w, prv->picture, tmp, 1);
    }
    free(tmp);

    for (int i = 0; i < ITEM_INFO_COUNTERS_COUNT; ++i) {
        asprintf(&tmp, "%s.Counter%d", widgetName, i);
        prv->counters[i] = QBSocialMediaIconWithTextCreate(self->app, self->textRenderer, tmp, SVSTRING("icon"), SVSTRING("value"));
        if (prv->counters[i])
            svSettingsWidgetAttach(w, prv->counters[i], tmp, 1);
        free(tmp);
    }

    for (int i = 0; i < ITEM_INFO_COMMENTS_COUNT; ++i) {
        asprintf(&tmp, "%s.Comment%d", widgetName, i);
        prv->comments[i] = QBSocialMediaCommentCreate(self->app, self->textRenderer, tmp);
        if (prv->comments[i])
            svSettingsWidgetAttach(w, prv->comments[i], tmp, 1);
        free(tmp);
    }

    svSettingsPopComponent();

    return w;
}

SvLocal void
QBSocialMediaItemDetailsSlotSetObject(SvGenericObject self_, SvWidget w, SvGenericObject object)
{
    QBSocialMediaItemDetailsSlot prv = w->prv;

    SvHashTable data = (SvHashTable) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("data"));
    SvValue val = NULL;
    SvString text = NULL, title = NULL;
    const char *picture = NULL;

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("title"));
    if (val && SvValueIsString(val) && prv->title)
        title = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("text"));
    if (val && SvValueIsString(val) && prv->text)
        text = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("picture"));
    if (val && SvValueIsString(val) && prv->picture)
        picture = SvValueGetStringAsCString(val, NULL);

    QBAsyncLabelSetText(prv->title, title);
    if (prv->text)
        QBAsyncLabelSetText(prv->text, text);

    if (prv->picture) {
        svIconSetBitmapFromURI(prv->picture, 0, picture);
    }

    SvArray counters = (SvArray) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("counters"));
    if (counters) {
        int size = SvArrayCount(counters);
        for (int i = 0; i < size && i < ITEM_INFO_COUNTERS_COUNT; ++i)
            QBSocialMediaIconWithTextSetObject(prv->counters[i], SvArrayObjectAtIndex(counters, i));
    }

    SvArray comments = (SvArray) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("comments"));
    int size = 0;
    if (comments) {
        size = SvArrayCount(comments);
        for (int i = 0; i < size && i < ITEM_INFO_COMMENTS_COUNT; ++i)
            QBSocialMediaCommentSetObject(prv->comments[i], SvArrayObjectAtIndex(comments, i));
    }
    for (int i = size; i < ITEM_INFO_COMMENTS_COUNT; ++i)
        QBSocialMediaCommentSetObject(prv->comments[i], NULL);
}

SvLocal void
QBSocialMediaItemDetailsSlotDestroy(void *self_)
{
    QBSocialMediaItemDetailsSlotController self = self_;
    SVRELEASE(self->widgetName);
}

SvLocal SvType
QBSocialMediaItemDetailsSlot_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaItemDetailsSlotDestroy
    };
    static SvType type = NULL;

    static const struct QBSocialMediaInfoSlot_t movieInfoSlotMethods = {
        .createWidget = QBSocialMediaItemDetailsSlotCreateWidget,
        .setObject    = QBSocialMediaItemDetailsSlotSetObject
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaItemDetailsSlot",
                            sizeof(struct QBSocialMediaItemDetailsSlot_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSocialMediaInfoSlot_getInterface(), &movieInfoSlotMethods,
                            NULL);
    }
    return type;

}

QBSocialMediaItemDetailsSlotController
QBSocialMediaItemDetailsSlotControllerCreate(SvApplication app, QBTextRenderer textRenderer, SvString widgetName)
{
    QBSocialMediaItemDetailsSlotController self = (QBSocialMediaItemDetailsSlotController) SvTypeAllocateInstance(QBSocialMediaItemDetailsSlot_getType(), NULL);
    self->app = app;
    self->textRenderer = textRenderer;
    self->widgetName = SVRETAIN(widgetName);
    return self;

}

// QBSocialMediaProfileSlot

#define PROFILE_TOP_LIST_SIZE    5
#define PROFILE_BOTTOM_LIST_SIZE 3

struct QBSocialMediaProfileSlotController_t {
    struct SvObject_ super_;

    SvApplication  app;
    QBTextRenderer textRenderer;
};

struct QBSocialMediaProfileSlot_t {

    SvWidget title;

    SvWidget topListHeader;
    SvWidget topList[PROFILE_TOP_LIST_SIZE];

    SvWidget bottomListHeader;
    SvWidget bottomList[PROFILE_BOTTOM_LIST_SIZE];

    SvWidget picture;
};
typedef struct QBSocialMediaProfileSlot_t *QBSocialMediaProfileSlot;

SvLocal SvWidget
QBSocialMediaProfileSlotCreateWidget(SvGenericObject self_, int width, int height)
{
    QBSocialMediaProfileSlotController self = (QBSocialMediaProfileSlotController) self_;

    QBSocialMediaProfileSlot prv = calloc(1, sizeof(struct QBSocialMediaProfileSlot_t));
    if (!prv) {
        SvLogError("%s : out of memory", __func__);
        return NULL;
    }

    SvWidget w = svWidgetCreateBitmap(self->app, width, height, NULL);
    w->prv = prv;

    svSettingsPushComponent("SocialMediaSlot.settings");

    prv->title= QBAsyncLabelNew(self->app, "Profile.Title", self->textRenderer);
    if (prv->title)
        svSettingsWidgetAttach(w, prv->title, "Profile.Title", 1);

    prv->topListHeader = QBAsyncLabelNew(self->app, "Profile.TopListHeader", self->textRenderer);
    if (prv->topListHeader)
        svSettingsWidgetAttach(w, prv->topListHeader, "Profile.TopListHeader", 1);

    prv->bottomListHeader = QBAsyncLabelNew(self->app, "Profile.BottomListHeader", self->textRenderer);
    if (prv->bottomListHeader)
        svSettingsWidgetAttach(w, prv->bottomListHeader, "Profile.BottomListHeader", 1);

    prv->picture = svIconNew(self->app, "Profile.Picture");
    if (prv->picture)
        svSettingsWidgetAttach(w, prv->picture, "Profile.Picture", 1);

    char tmp[32];
    for (int i = 0; i < PROFILE_TOP_LIST_SIZE; ++i) {
        snprintf(tmp, 32, "Profile.TopList%d", i);
        prv->topList[i] = QBSocialMediaIconWithTextCreate(self->app, self->textRenderer, tmp, SVSTRING("icon"), SVSTRING("text"));
        if (prv->topList[i])
            svSettingsWidgetAttach(w, prv->topList[i], tmp, 1);
    }
    for (int i = 0; i < PROFILE_BOTTOM_LIST_SIZE; ++i) {
        snprintf(tmp, 32, "Profile.BottomList%d", i);
        prv->bottomList[i] = QBSocialMediaIconWithTextCreate(self->app, self->textRenderer, tmp, SVSTRING("icon"), SVSTRING("text"));
        if (prv->bottomList[i])
            svSettingsWidgetAttach(w, prv->bottomList[i], tmp, 1);
    }

    svSettingsPopComponent();

    return w;
}

SvLocal void
QBSocialMediaProfileSlotSetObject(SvGenericObject self_, SvWidget w, SvGenericObject object)
{
    //QBSocialMediaProfileSlot self = (QBSocialMediaProfileSlot) self_;
    QBSocialMediaProfileSlot prv = w->prv;

    SvHashTable data = (SvHashTable) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("data"));
    SvValue val = NULL;
    SvString title = NULL, topListHeader = NULL, bottomListHeader = NULL;
    const char *picture = NULL;

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("title"));
    if (val && SvValueIsString(val))
        title = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("picture"));
    if (val && SvValueIsString(val))
        picture = SvValueGetStringAsCString(val, NULL);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("top_list_header"));
    if (val && SvValueIsString(val))
        topListHeader = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("bottom_list_header"));
    if (val && SvValueIsString(val))
        bottomListHeader = SvValueGetString(val);

    QBAsyncLabelSetText(prv->title, title);
    QBAsyncLabelSetText(prv->topListHeader, topListHeader);
    QBAsyncLabelSetText(prv->bottomListHeader, bottomListHeader);

    svIconSetBitmapFromURI(prv->picture, 0, picture);

    SvArray topList = (SvArray) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("top_list"));
    if (topList) {
        int size = SvArrayCount(topList);
        for (int i = 0; i < size && i < PROFILE_TOP_LIST_SIZE; i++) {
            QBSocialMediaIconWithTextSetObject(prv->topList[i], SvArrayObjectAtIndex(topList, i));
        }
    }

    SvArray bottomList = (SvArray) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("bottom_list"));
    if (bottomList) {
        int size = SvArrayCount(bottomList);
        for (int i = 0; i < size && i < PROFILE_BOTTOM_LIST_SIZE; i++) {
            QBSocialMediaIconWithTextSetObject(prv->bottomList[i], SvArrayObjectAtIndex(bottomList, i));
        }
    }
}

SvLocal void
QBSocialMediaProfileSlotControllerDestroy(void *self_)
{
}

SvLocal SvType
QBSocialMediaProfileSlotController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaProfileSlotControllerDestroy
    };
    static SvType type = NULL;

    static const struct QBSocialMediaInfoSlot_t movieInfoSlotMethods = {
        .createWidget = QBSocialMediaProfileSlotCreateWidget,
        .setObject    = QBSocialMediaProfileSlotSetObject
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaProfileSlotController",
                            sizeof(struct QBSocialMediaProfileSlotController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSocialMediaInfoSlot_getInterface(), &movieInfoSlotMethods,
                            NULL);
    }
    return type;

}

QBSocialMediaProfileSlotController
QBSocialMediaProfileSlotControllerCreate(SvApplication app, QBTextRenderer textRenderer)
{
    QBSocialMediaProfileSlotController self = (QBSocialMediaProfileSlotController) SvTypeAllocateInstance(QBSocialMediaProfileSlotController_getType(), NULL);
    self->app = app;
    self->textRenderer = textRenderer;
    return self;

}

// QBSocialMediaPictureSlot

struct QBSocialMediaPictureSlotController_t {
    struct SvObject_ super_;

    SvApplication  app;
    QBTextRenderer textRenderer;
};

struct QBSocialMediaPictureSlot_t {
    SvWidget title;
    SvWidget picture;
};
typedef struct QBSocialMediaPictureSlot_t *QBSocialMediaPictureSlot;

SvLocal SvWidget
QBSocialMediaPictureSlotCreateWidget(SvGenericObject self_, int width, int height)
{
    QBSocialMediaPictureSlotController self = (QBSocialMediaPictureSlotController) self_;

    QBSocialMediaPictureSlot prv = calloc(1, sizeof(struct QBSocialMediaPictureSlot_t));
    if (!prv) {
        SvLogError("%s : out of memory", __func__);
        return NULL;
    }

    SvWidget w = svWidgetCreateBitmap(self->app, width, height, NULL);
    w->prv = prv;

    svSettingsPushComponent("SocialMediaSlot.settings");

    prv->title = QBAsyncLabelNew(self->app, "Picture.Title", self->textRenderer);
    if (prv->title)
        svSettingsWidgetAttach(w, prv->title, "Picture.Title", 1);

    prv->picture = svIconNew(self->app, "Picture.Picture");
    if (prv->picture)
        svSettingsWidgetAttach(w, prv->picture, "Picture.Picture", 1);

    svSettingsPopComponent();

    return w;
}

SvLocal void
QBSocialMediaPictureSlotSetObject(SvGenericObject self_, SvWidget w, SvGenericObject object)
{
    QBSocialMediaPictureSlot prv = w->prv;

    SvHashTable data = (SvHashTable) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("data"));
    SvValue val = NULL;
    SvString title = NULL;
    const char *picture = NULL;

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("title"));
    if (val && SvValueIsString(val))
        title = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("picture"));
    if (val && SvValueIsString(val))
        picture = SvValueGetStringAsCString(val, NULL);

    QBAsyncLabelSetText(prv->title, title);

    svIconSetBitmapFromURI(prv->picture, 0, picture);
}
SvLocal void
QBSocialMediaPictureSlotControllerDestroy(void *self_)
{
}

SvLocal SvType
QBSocialMediaPictureSlotController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaPictureSlotControllerDestroy
    };
    static SvType type = NULL;

    static const struct QBSocialMediaInfoSlot_t movieInfoSlotMethods = {
        .createWidget = QBSocialMediaPictureSlotCreateWidget,
        .setObject    = QBSocialMediaPictureSlotSetObject
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaPictureSlotController",
                            sizeof(struct QBSocialMediaPictureSlotController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSocialMediaInfoSlot_getInterface(), &movieInfoSlotMethods,
                            NULL);
    }
    return type;

}

QBSocialMediaPictureSlotController
QBSocialMediaPictureSlotControllerCreate(SvApplication app, QBTextRenderer textRenderer)
{
    QBSocialMediaPictureSlotController self = (QBSocialMediaPictureSlotController) SvTypeAllocateInstance(QBSocialMediaPictureSlotController_getType(), NULL);
    self->app = app;
    self->textRenderer = textRenderer;
    return self;

}


// QBSocialMediaGallerySlot

struct QBSocialMediaGallerySlotController_t {
    struct SvObject_ super_;

    SvApplication  app;
    QBTextRenderer textRenderer;
};

struct QBSocialMediaGallerySlot_t {
    SvWidget title;
    SvWidget description;
    SvWidget picture;
};
typedef struct QBSocialMediaGallerySlot_t *QBSocialMediaGallerySlot;

SvLocal SvWidget
QBSocialMediaGallerySlotCreateWidget(SvGenericObject self_, int width, int height)
{
    QBSocialMediaGallerySlotController self = (QBSocialMediaGallerySlotController) self_;

    QBSocialMediaGallerySlot prv = calloc(1, sizeof(struct QBSocialMediaGallerySlot_t));
    if (!prv) {
        SvLogError("%s : out of memory", __func__);
        return NULL;
    }

    SvWidget w = svWidgetCreateBitmap(self->app, width, height, NULL);
    w->prv = prv;

    svSettingsPushComponent("SocialMediaSlot.settings");

    prv->title = QBAsyncLabelNew(self->app, "Gallery.Title", self->textRenderer);
    if (prv->title)
        svSettingsWidgetAttach(w, prv->title, "Gallery.Title", 1);

    prv->description = QBAsyncLabelNew(self->app, "Gallery.Description", self->textRenderer);
    if (prv->description)
        svSettingsWidgetAttach(w, prv->description, "Gallery.Description", 1);

    prv->picture = svIconNew(self->app, "Gallery.Picture");
    if (prv->picture)
        svSettingsWidgetAttach(w, prv->picture, "Gallery.Picture", 1);

    svSettingsPopComponent();

    return w;
}

SvLocal void
QBSocialMediaGallerySlotSetObject(SvGenericObject self_, SvWidget w, SvGenericObject object)
{
    QBSocialMediaGallerySlot prv = w->prv;

    SvHashTable data = (SvHashTable) SvHashTableFind((SvHashTable) object, (SvGenericObject) SVSTRING("data"));
    SvValue val = NULL;
    SvString title = NULL, description = NULL;
    const char *picture = NULL;

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("title"));
    if (val && SvValueIsString(val))
        title = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("description"));
    if (val && SvValueIsString(val))
        description = SvValueGetString(val);

    val = (SvValue) SvHashTableFind(data, (SvGenericObject) SVSTRING("picture"));
    if (val && SvValueIsString(val))
        picture = SvValueGetStringAsCString(val, NULL);

    QBAsyncLabelSetText(prv->title, title);
    QBAsyncLabelSetText(prv->description, description);

    svIconSetBitmapFromURI(prv->picture, 0, picture);
}
SvLocal void
QBSocialMediaGallerySlotControllerDestroy(void *self_)
{
}

SvLocal SvType
QBSocialMediaGallerySlotController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSocialMediaGallerySlotControllerDestroy
    };
    static SvType type = NULL;

    static const struct QBSocialMediaInfoSlot_t movieInfoSlotMethods = {
        .createWidget = QBSocialMediaGallerySlotCreateWidget,
        .setObject    = QBSocialMediaGallerySlotSetObject
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSocialMediaGallerySlotController",
                            sizeof(struct QBSocialMediaGallerySlotController_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBSocialMediaInfoSlot_getInterface(), &movieInfoSlotMethods,
                            NULL);
    }
    return type;

}

QBSocialMediaGallerySlotController
QBSocialMediaGallerySlotControllerCreate(SvApplication app, QBTextRenderer textRenderer)
{
    QBSocialMediaGallerySlotController self = (QBSocialMediaGallerySlotController) SvTypeAllocateInstance(QBSocialMediaGallerySlotController_getType(), NULL);
    self->app = app;
    self->textRenderer = textRenderer;
    return self;

}

