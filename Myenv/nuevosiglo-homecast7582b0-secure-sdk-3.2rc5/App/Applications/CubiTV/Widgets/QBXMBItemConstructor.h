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

#ifndef QBXMBITEMCONSTRUCTOR_H
#define QBXMBITEMCONSTRUCTOR_H

#include <CUIT/Core/types.h>
#include <QBResourceManager/SvRBObject.h>
#include <Services/core/QBTextRenderer.h>
#include <CAGE/Text/SvTextLayout.h>
#include <SWL/QBFrame.h>
#include <XMB2/XMBTypes.h>

typedef enum {
    QBXMBItemAlignment_Left = 0,
    QBXMBItemAlignment_Right
} QBXMBItemAlignment;

struct QBXMBItemConstructor_t {
    struct SvObject_ super_;
    SvFont font;
    int leftPadding;
    int rightPadding;
    SvColor textColor, disabledTextColor;
    int textOffset;
    int spacing;
    int normalDim;
    int inactiveDim;
    int fontSize;
    int mboxHeight;
    int subFontSize;
    SvColor subTextColor, disabledSubTextColor;
    int subMBoxHeight;
    int width;
    int height;
    int subcaptionSpacing;
    SvColor descriptionTextColor, disabledDescriptionTextColor;
    int descriptionFontSize;
    int descriptionYOffset;

    int focusXOffset;
    int focusYOffset;
    int focusHeight;
    int focusWidth;
    float focusFadeTime;
    float showTime;

    int iconWidth;
    int iconHeight;

    int minBottomPadding;

    QBTextRenderer renderer;
    bool hideInactiveText;
    bool showTextFocused;
    bool hideInactiveFocus;

    bool titleMultiline;
    bool subtitleMultiline;
    bool isMarkup;
    SvString textEnd;
    int maxDescLines;

    int activeRowHeightFactor;
};


struct QBXMBItem_t {
    struct SvObject_ super_;
    SvString caption;
    SvString subcaption;
    SvString description;

    SvRID loadingRID;

    SvBitmap icon;
    SvRID iconRID;
    struct {
        SvString URI;
        bool isStatic;
    } iconURI;

    SvBitmap iconRight;
    struct {
        SvString URI;
        bool isStatic;
    } iconRightURI;
    bool usesRightIcon;

    bool disabled;
    QBFrameConstructor* focus;
    QBFrameConstructor* inactiveFocus;
    QBFrameConstructor* sortingFocus;
    QBFrameConstructor* disabledFocus;
};

typedef struct QBXMBItem_t* QBXMBItem;

typedef struct QBXMBItemInfo_t* QBXMBItemInfo;


typedef struct QBXMBItemRenderer_t {

    SvWidget (* createCaption) (SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem item);

    SvWidget (* createSubcaption) (SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem item);

    SvWidget (* createDescription) (SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem item);

    SvWidget (* createIcon) (SvGenericObject self_, QBXMBItemInfo itemInfo, QBXMBItem item, QBXMBItemAlignment alignment);

} *QBXMBItemRenderer;

extern SvInterface
QBXMBItemRenderer_getInterface(void);

typedef struct QBXMBItemConstructor_t* QBXMBItemConstructor;

struct QBXMBItemInfo_t {
    struct SvObject_ super_;
    SvWidget box;
    SvWidget icon;
    SvWidget iconRight;
    SvWidget focus, inactiveFocus, sortingFocus, disabledFocus;

    SvEffectId focusFade, inactiveFocusFade, sortingFocusFade, disabledFocusFade;
    float focusFadeTime;

    float showTime;
    SvEffectId show;
    SvEffectId showSubcaption;
    SvEffectId showRightIcon;

    struct {
        SvWidget w;
        short baseLineX, baseLineY;
        bool rendered;
        short mboxHeight;
    } title;

    bool disabled;

    struct {
        SvWidget w;
        short baseLineX, baseLineY;
        bool rendered;
        bool multiLine;
        short mboxHeight;
    } subcaption;

    struct {
        SvWidget w;
        short baseLineX, baseLineY;
        bool rendered;
        bool multiLine;
        short mboxHeight;
    } description;

    int padding;
    int captionsXPos;
    int subcaptionSpacing;
    int descriptionYOffset;

    int minBottomPadding;
    int currentFocusWidth;

    QBXMBItemConstructor constructor;

    void (*positionCallback)(QBXMBItemInfo item);
};

extern SvType
QBXMBItemConstructor_getType(void);

extern QBXMBItemConstructor
QBXMBItemConstructorInit(QBXMBItemConstructor self, const char *widgetName, QBTextRenderer renderer);

extern SvType
QBXMBItem_getType(void);

extern SvType
QBXMBItemInfo_getType(void);

QBXMBItemInfo QBXMBItemInfoCreate(void);

QBXMBItem QBXMBItemCreate(void);

QBXMBItemConstructor QBXMBItemConstructorCreate(const char *widgetName, QBTextRenderer renderer);

void QBXMBItemConstructorDestroy(QBXMBItemConstructor self);

SvWidget QBXMBItemConstructorCreateItem(QBXMBItemConstructor self, QBXMBItem item, SvApplication app, XMBMenuState initialState);

SvWidget QBXMBItemConstructorInitItem(QBXMBItemConstructor self, QBXMBItem itemData,
                                        SvApplication app, XMBMenuState initialState,
                                        QBXMBItemInfo item);

void QBXMBItemConstructorSetItemState(QBXMBItemConstructor self, SvWidget item_, XMBMenuState state, bool isFocused);

//Private
void QBXMBItemInfoPositionItems(QBXMBItemInfo item);
void QBXMBItemInfoSetFocusWidth(QBXMBItemInfo item, int focusWidth);

#endif
