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

#ifndef QB_VERTICAL_CAROUSEL_H_
#define QB_VERTICAL_CAROUSEL_H_

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <CUIT/Core/types.h>

#include "QBVerticalCarouselSlot.h"

/**
 * @defgroup QBVerticalCarousel QBVerticalCarousel
 * @{
 *
 * QBVerticalCarousel is a carousel which might be oriented vertically or horizontally
 * depending on its parameters.
 **/

/// carousel notification type
#define SV_EVENT_QB_CAROUSEL_NOTIFICATION 0x214fa11

/**
 * QBVerticalCarousel scroll state.
 **/
typedef enum {
  QB_CAROUSEL_SCROLL_LEFT,
  QB_CAROUSEL_SCROLL_RIGHT,
  QB_CAROUSEL_SCROLL_LEFT_STOP,
  QB_CAROUSEL_SCROLL_RIGHT_STOP,
  QB_CAROUSEL_IDLE
} QBVerticalCarouselScrollState;

/**
 * QBVerticalCarousel notification type.
 **/
typedef enum {
    QB_CAROUSEL_NOTIFICATION_ACTIVE_ELEMENT,
    QB_CAROUSEL_NOTIFICATION_SELECTED_ELEMENT,
    QB_CAROUSEL_NOTIFICATION_EXIT
} QBVerticalCarouselNotificationType;

/**
 * QBVerticalCarousel state.
 **/
typedef enum {
    QBVerticalCarouselState_active,
    QBVerticalCarouselState_inactive
} QBVerticalCarouselState;

/**
 * QBVerticalCarousel notification.
 **/
typedef struct QBVerticalCarouselNotification_ {
    QBVerticalCarouselNotificationType type; ///< notification type
    int dataIndex;       ///< index of object for which notification was created
    SvObject dataObject; ///< object handle for which notification was created
    bool immediately;    ///< should it be handled immediately
} *QBVerticalCarouselNotification;

/**
 * QBVerticalCarousel orientation.
 **/
typedef enum {
    QBCarouselOrientation_horizontal,
    QBCarouselOrientation_vertical
} QBCarouselOrientation;

/**
 * QBVerticalCarousel key action.
 **/
typedef enum {
    QBVerticalCarouselKeyAction_none,
    QBVerticalCarouselKeyAction_enter,
    QBVerticalCarouselKeyAction_prev,
    QBVerticalCarouselKeyAction_next
} QBVerticalCarouselKeyAction;

/**
 * QBVerticalCarousel parameters
 * @class QBVerticalCarouselParams
 * @extends SvObject
 **/
typedef struct QBVerticalCarouselParams_ {
    ///< super class
    struct SvObject_ super_;

    QBCarouselOrientation orientation;  ///< carousel orientation

    unsigned int width;                 ///< carousel width
    unsigned int height;                ///< carousel height

    int rows;                           ///< number of rows
    int columns;                        ///< number of columns (note: when vertical carousel is horizontal number of columns is really number of rows)

    int firstFocusableColumn;           ///< first column that should be focusable
    int lastFocusableColumn;            ///< last column that should be focusable
    int defaultFocusColumn;             ///< column that should be focused by default (should be between firstFocusableColumn and lastFocusableColumn)

    int firstVisibleColumn;             ///< first visible column
    int lastVisibleColumn;              ///< last visible column

    int slotWidth;                      ///< carousel slot width
    int slotHeight;                     ///< carousel slot height
    int verticalSpacing;                ///< vertical spacing between slots
    int horizontalSpacing;              ///< horizontal spacing between slots

    double scrollMaxSpeed;              ///< scrolling effect max speed in px/s
    double scrollAcceleration;          ///< scrolling effect max acceleration px/s^2

    bool loopEnabled;                   ///< is carousel loop enabled
    bool autoWrap;                      ///< is auto wrapping enabled
    bool skipFirstElement;              ///< should it skip first element
    bool alignOnScrollStart;            ///< should slots get alligned on scrolling start
    bool showEmptySlots;                ///< should empty slots be shown

    SvBitmap arrowDown;                 ///< arrow down bitmap handle
    int arrowDownXOffset;               ///< arrow down horizontal offset
    int arrowDownYOffset;               ///< arrow down vertical offset
    int arrowDownWidth;                 ///< arrow down width
    int arrowDownHeight;                ///< arrow down height

    SvBitmap arrowUp;                   ///< arrow up bitmap handle
    int arrowUpXOffset;                 ///< arrow up horizontal offset
    int arrowUpYOffset;                 ///< arrow up vertical offset
    int arrowUpWidth;                   ///< arrow up width
    int arrowUpHeight;                  ///< arrow up height

    SvHashTable keymap;                 ///< key map for overriding default key map of vertical carousel,
                                        ///< elements key is SvValue with key code integer,
                                        ///< value is SvValue with QBVerticalCarouselKeyAction casted to integer
} *QBVerticalCarouselParams;

/**
 * Create default QBVerticalCarouselParams.
 *
 * @memberof QBVerticalCarouselParams
 *
 * This is a method that allocates and initializes QBVerticalCarouselParams with default values.
 *
 * @param[out] *errorOut    SvErrorInfo
 * @return                  created parameters, @c NULL in case of error
 **/
extern QBVerticalCarouselParams
QBVerticalCarouselParamsCreateDefault(SvErrorInfo *errorOut);

typedef void
(*QBVerticalCarouselFocusContentSetData)(SvWidget focus, SvObject data);

typedef void
(*QBVerticalCarouselRealignColumnCallback)(void* owner, SvWidget w, QBVerticalCarouselScrollState scrollState, int i, int offset);

/**
 * Create QBVerticalCarousel from parameters.
 *
 * This is a method that allocates and initializes QBVerticalCarouselParams with default values.
 *
 * @param[in] app           application handle
 * @param[in] params        parameters handle
 * @param[in] widgetName    name of the carousel widget
 * @param[out] *errorOut    SvErrorInfo
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBVerticalCarouselNew(SvApplication app, const QBVerticalCarouselParams params, const char *widgetName, SvErrorInfo *errorOut);

/**
 * Create QBVerticalCarousel from settings manager.
 *
 * This is a method that allocates and initializes QBVerticalCarouselParams with default values.
 *
 * @param[in] app           application handle
 * @param[in] widgetName    name of the widget
 * @return                  created widget, @c NULL in case of error
 **/
extern SvWidget
QBVerticalCarouselNewFromSM(SvApplication app, const char *widgetName);

/**
 * Set content at QBVerticalCarousel slot with index.
 *
 * @param[in] carousel      carousel widget handle
 * @param[in] slotIndex     slot index
 * @param[in] content       widget to be set as content
 **/
extern void
QBVerticalCarouselSetContentAtIndex(SvWidget carousel, int slotIndex, SvWidget content);

/**
 * Get content from QBVerticalCarousel slot with index.
 *
 * @param[in] carousel      carousel widget handle
 * @param[in] slotIndex     slot index
 * @return                  content of slot
 **/
extern SvWidget
QBVerticalCarouselGetContentAtIndex(SvWidget carousel, int slotIndex);

/**
 * Set notification target for QBVerticalCarousel.
 *
 * @param[in] w         list carousel widget handle
 * @param[in] target    id of widget to be notified
 **/
void
QBVerticalCarouselSetNotificationTarget(SvWidget w, SvWidgetId target);

extern void
QBVerticalCarouselSetDataHandler(SvWidget carousel, int slotIndex, QBVerticalCarouselSlotContentSetData s);

extern int
QBVerticalCarouselGetSlotCount(SvWidget carousel);

#define QBVerticalCarouselSetSlotHandlers(carousel, setter, constructor, ...) \
{\
    int i;\
    for (i = 0; i < QBVerticalCarouselGetSlotCount(carousel); i++) {\
        QBVerticalCarouselSetContentAtIndex(carousel, i, constructor(carousel->app, ##__VA_ARGS__));\
        QBVerticalCarouselSetDataHandler(carousel, i, setter);\
    }\
}

extern void
QBVerticalCarouselConnectToSource(SvWidget w, SvObject dataSource, SvErrorInfo *error);

extern void
QBVerticalCarouselSetActive(SvWidget w, int index);

extern void
QBVerticalCarouselSlideToIndex(SvWidget w, int index);

extern void
QBVerticalCarouselSetFocusWidget(SvWidget carousel, SvWidget focus);

extern void
QBVerticalCarouselSetFocusDataHandler(SvWidget carousel, QBVerticalCarouselFocusContentSetData fs);

#define QBVerticalCarouselSetFocusHandlers(carousel, setter, constructor, ...) \
{\
    QBVerticalCarouselSetFocusWidget(carousel, constructor(carousel->app, ##__VA_ARGS__));\
    QBVerticalCarouselSetFocusDataHandler(carousel, setter);\
}

extern void
QBVerticalCarouselFocusSetData(SvWidget w, int x);

extern void
QBVerticalCarouselDropFocus(SvWidget w);

extern bool
QBVerticalCarouselIsScrolling(SvWidget carousel);

extern void
QBVerticalCarouselStopScrolling(SvWidget w);

extern bool
QBVerticalCarouselIsFocused(SvWidget carousel);

extern void
QBVerticalCarouselRealignColumn(SvWidget w, QBVerticalCarouselScrollState scrollState, int i, int offset);

extern void
QBVerticalCarouselSetRealignColumnCallback(SvWidget w, void* owner, QBVerticalCarouselRealignColumnCallback);

extern void
QBVerticalCarouselUpdateItems(SvWidget carousel);

extern void
QBVerticalCarouselSetSlotStateChangeHandler(SvWidget w, QBVerticalCarouselSlotStateChanged slotStateChanged);

extern void
QBVerticalCarouselSetState(SvWidget w, QBVerticalCarouselState state);

extern bool
QBVerticalCarouselHandleInputEvent(SvWidget w, SvInputEvent e);

extern int
QBVerticalCarouselGetCurrentSlot(SvWidget w);

/**
 * @}
 **/

#endif
