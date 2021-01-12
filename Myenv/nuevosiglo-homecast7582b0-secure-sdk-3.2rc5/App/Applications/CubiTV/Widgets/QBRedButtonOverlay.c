/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBRedButtonOverlay.h"
#include <CUIT/Core/event.h>
#include <CUIT/Core/widget.h>
#include <QBInput/QBInputCodes.h>
#include <settings.h>
#include <SvFoundation/SvInterface.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <SWL/icon.h>
#include <SWL/label.h>
#include <SWL/QBFrame.h>

#define log_error(fmt, ...) do { SvLogError(COLBEG() "QBRedButtonOverlay :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { SvLogWarning(COLBEG() "QBRedButtonOverlay :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)

typedef enum {
    BUTTON_RED = 0,
    BUTTON_GREEN,
    BUTTON_YELLOW,
    BUTTON_BLUE,
    BUTTONS_COUNT
} QBButtonColor;

SvLocal const char *const QBButtonColorNames[BUTTONS_COUNT] = {
    [BUTTON_RED] = "red",
    [BUTTON_GREEN] = "green",
    [BUTTON_YELLOW] = "yellow",
    [BUTTON_BLUE] = "blue"
};

SvLocal QBButtonColor QBButtonColorFromInputCode(uint16_t ch)
{
    switch (ch) {
        case QBKEY_RED:    return BUTTON_RED;
        case QBKEY_GREEN:  return BUTTON_GREEN;
        case QBKEY_YELLOW: return BUTTON_YELLOW;
        case QBKEY_BLUE:   return BUTTON_BLUE;
        default:           return -1;
    }
}

struct QBRedButtonOverlay_ {
    SvWidget w;

    SvString titles[BUTTONS_COUNT];
    SvRBBitmap images[BUTTONS_COUNT];

    SvWidget content;
    QBButtonColor currentColor;
    QBFrameDescription *backgroundFocus, *backgroundNonFocus;
    SvWidget buttonFrames[BUTTONS_COUNT];
    SvWidget buttonTitles[BUTTONS_COUNT];
};
typedef struct QBRedButtonOverlay_* QBRedButtonOverlay;


SvLocal void QBRedButtonOverlaySelectColor(QBRedButtonOverlay self, QBButtonColor color);

SvLocal void QBRedButtonOverlayCleanup(SvApplication app, void *self_);
SvLocal bool QBRedButtonOverlayInputEventHandler(SvWidget w, SvInputEvent e);

SvLocal void QBRedButtonOverlaySetContentImage(QBRedButtonOverlay self, int idx, SvRBBitmap overlay);

SvLocal bool QBRedButtonOverlayLoadData(QBRedButtonOverlay self, SvHashTable data)
{
    SvHashTable titlesHashTable = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("titles"));
    if (!titlesHashTable) {
        log_error("no titles in data");
        return false;
    }

    SvHashTable imagesHashTable = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("images"));
    if (!imagesHashTable) {
        log_error("no images in data");
        return false;
    }

    for (int i = 0; i < BUTTONS_COUNT; ++i) {
        SVTESTRELEASE(self->titles[i]);
        self->titles[i] = NULL;
        SVTESTRELEASE(self->images[i]);
        self->images[i] = NULL;
    }

    SvHashTable titles = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("titles"));
    SvHashTable images = (SvHashTable) SvHashTableFind(data, (SvObject) SVSTRING("images"));

    for (int i = 0; i < BUTTONS_COUNT; ++i) {
        SVAUTOSTRING(colorName, QBButtonColorNames[i]);
        SvString title = (SvString) SvHashTableFind(titles, (SvObject) colorName);
        SvRBBitmap imageRB = (SvRBBitmap) SvHashTableFind(images, (SvObject) colorName);

        if (!title || !imageRB)
            continue;

        self->titles[i] = SVRETAIN(title);
        self->images[i] = SVRETAIN(imageRB);
    }

    return true;
}

SvWidget QBRedButtonOverlayNew(SvApplication app, const char *redButtonOverlayName, SvHashTable data)
{
    if (!app) {
        log_error("got NULL SvApplication");
        return NULL;
    }
    if (!redButtonOverlayName) {
        log_error("got NULL redButtonOverlayName");
        return NULL;
    }
    if (!data) {
        log_error("got NULL data");
        return NULL;
    }

    QBRedButtonOverlay self = calloc(1, sizeof(*self));
    if (!self) {
        log_error("failed allocating self");
        return NULL;
    }

    if (!QBRedButtonOverlayLoadData(self, data)) {
        free(self);
        return NULL;
    }

    SvWidget w = svSettingsWidgetCreate(app, redButtonOverlayName);
    self->w = w;

    svWidgetSetInputEventHandler(w, QBRedButtonOverlayInputEventHandler);
    w->prv = self;
    w->clean = QBRedButtonOverlayCleanup;

    { // create content widget
        char *contentName;
        asprintf(&contentName, "%s.Content", redButtonOverlayName);
        self->content = svIconNew(app, contentName);

        for (int i = 0; i < BUTTONS_COUNT; ++i)
            QBRedButtonOverlaySetContentImage(self, i, self->images[i]);

        svSettingsWidgetAttach(self->w, self->content, contentName, 1);
        free(contentName);
    }
    { // create button frames
        char *buttonFrameName;
        asprintf(&buttonFrameName, "%s.ButtonFrame", redButtonOverlayName);
        { // create backgrounds
            char *backgroundName;
            asprintf(&backgroundName, "%s.Background", buttonFrameName);
            {
                char *focusBackgroundDescName;
                asprintf(&focusBackgroundDescName, "%s.Focus", backgroundName);
                self->backgroundFocus = QBFrameDescriptionCreateFromSM(focusBackgroundDescName);
                if (!self->backgroundFocus)
                    log_error("Failed loading focus frame");
                free(focusBackgroundDescName);
            }
            {
                char *nonFocusBackgroundDescName;
                asprintf(&nonFocusBackgroundDescName, "%s.NonFocus", backgroundName);
                self->backgroundNonFocus = QBFrameDescriptionCreateFromSM(nonFocusBackgroundDescName);
                if (!self->backgroundNonFocus)
                    log_error("Failed loading nonfocus frame");
                free(nonFocusBackgroundDescName);
            }
            for (int i = 0; i < BUTTONS_COUNT; ++i) {
                self->buttonFrames[i] = QBFrameCreateWithDescription(app, self->backgroundNonFocus);
                int yOffset = svSettingsGetInteger(backgroundName, "yOffset", 0);
                svWidgetAttach(self->w, self->buttonFrames[i], self->buttonFrames[i]->width * i, yOffset, 1);
            }
            free(backgroundName);
        }
        { // create color dots
            for (int i = 0; i < BUTTONS_COUNT; ++i) {
                char *colorDotName;
                asprintf(&colorDotName, "%s.ColorDot.%s", buttonFrameName, QBButtonColorNames[i]);
                SvWidget colorDot = svSettingsWidgetCreate(app, colorDotName);
                if (!colorDot)
                    SvLogError("%s(): Failed loading %s color dot", __func__, QBButtonColorNames[i]);
                svSettingsWidgetAttach(self->buttonFrames[i], colorDot, colorDotName, 2);
                free(colorDotName);
            }
        }
        { // create text labels
            char *textLabelName;
            asprintf(&textLabelName, "%s.TextLabel", buttonFrameName);
            for (int i = 0; i < BUTTONS_COUNT; ++i) {
                self->buttonTitles[i] = svLabelNewFromSM(app, textLabelName);
                if (self->titles[i])
                    svLabelSetText(self->buttonTitles[i], SvStringCString(self->titles[i]));
                svSettingsWidgetAttach(self->buttonFrames[i], self->buttonTitles[i], textLabelName, 2);
            }
            free(textLabelName);
        }
        free(buttonFrameName);
    }

    QBRedButtonOverlaySelectColor(self, BUTTON_RED);

    return w;
}

SvLocal void QBRedButtonOverlayCleanup(SvApplication app, void *self_)
{
    QBRedButtonOverlay self = self_;
    for (int i = 0; i < BUTTONS_COUNT; ++i) {
        SVTESTRELEASE(self->titles[i]);
        SVTESTRELEASE(self->images[i]);
    }
    SVRELEASE(self->backgroundFocus);
    SVRELEASE(self->backgroundNonFocus);
    free(self);
}

SvLocal bool QBRedButtonOverlayInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBRedButtonOverlay self = w->prv;
    if (e->ch == QBKEY_RED || e->ch == QBKEY_GREEN || e->ch == QBKEY_YELLOW || e->ch == QBKEY_BLUE) {
        QBButtonColor color = QBButtonColorFromInputCode(e->ch);
        if (self->currentColor != color)
            QBRedButtonOverlaySelectColor(self, color);
        return true;
    }
    return false;
}

void QBRedButtonOverlayReplaceContent(SvWidget self_, SvHashTable data)
{
    if (!self_) {
        log_error("got NULL self");
        return;
    }
    if (!data) {
        log_error("got NULL data");
        return;
    }

    QBRedButtonOverlay self = self_->prv;

    if (!QBRedButtonOverlayLoadData(self, data))
        return;

    for (int i = 0; i < BUTTONS_COUNT; ++i) {
        if (!self->titles[i] || !self->images[i])
            continue;
        svLabelSetText(self->buttonTitles[i], SvStringCString(self->titles[i]));
        QBRedButtonOverlaySetContentImage(self, i, self->images[i]);
    }
}

SvLocal void QBRedButtonOverlaySetContentImage(QBRedButtonOverlay self, int idx, SvRBBitmap image)
{
    if (image)
        svIconSetBitmapFromRID(self->content, idx, SvRBObjectGetID((SvRBObject) image));
    else {
        SvBitmap placeholder = SvBitmapCreateAndFill(1, 1, SV_CS_MONO, ALPHA_TRANSPARENT);
        svIconSetBitmap(self->content, idx, placeholder);
        SVRELEASE(placeholder);
    }
}


void QBRedButtonOverlayReset(SvWidget self_)
{
    QBRedButtonOverlay self = self_->prv;
    QBRedButtonOverlaySelectColor(self, BUTTON_RED);
}

SvLocal void QBRedButtonOverlaySelectColor(QBRedButtonOverlay self, const QBButtonColor color)
{
    QBFrameSetBitmapFromExtendedParams(self->buttonFrames[self->currentColor], &(self->backgroundNonFocus->data));

    self->currentColor = color;
    svIconSwitch(self->content, color, 0, -1);
    QBFrameSetBitmapFromExtendedParams(self->buttonFrames[color], &(self->backgroundFocus->data));
}
