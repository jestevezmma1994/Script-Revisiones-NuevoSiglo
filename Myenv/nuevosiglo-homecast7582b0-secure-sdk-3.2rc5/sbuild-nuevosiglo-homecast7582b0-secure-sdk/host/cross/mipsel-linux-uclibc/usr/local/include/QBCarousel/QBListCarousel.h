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

#ifndef QBLIST_CAROUSEL_H_
#define QBLIST_CAROUSEL_H_

/**
 * @file QBListCarousel.h List carousel widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBListCarousel QBListCarousel modules
 * @ingroup QBTileGrid_widgets
 * @{
 *
 * Default CubiConnect channel list.
 **/

/**
 * List carousel notification user event type.
 *
 * Use this value in widget's user event handler to check the type of incoming event.
 *
 * @see svUserEventCreate()
 **/
#define QB_LIST_CAROUSEL_NOTIFICATION 91

/**
 * QBListCarousel scroll animation type.
 **/
typedef enum {
    QBListCarouselScrollType_separate,  ///< top, middle and bottom parts are moving separately,
                                        ///< horizontal scrolling is not supported
    QBListCarouselScrollType_simultan,  ///< whole carousel is moving simultaneously
} QBListCarouselScrollType;

/**
 * QBListCarousel notification type.
 **/
typedef enum {
    QBListCarouselNotificationType_activeElement,   ///< element activated (focused)
    QBListCarouselNotificationType_selectedElement, ///< element selected
} QBListCarouselNotificationType;

/**
 * QBListCarousel scroll direction.
 **/
typedef enum QBListCarouselScrollDirection_ {
    QBListCarouselScrollDirection_IDLE = 0,    ///< idle
    QBListCarouselScrollDirection_BACKWARD,       ///< up (vertical) or left (horizontal)
    QBListCarouselScrollDirection_BACKWARD_STOP,  ///< up (stop stage)
    QBListCarouselScrollDirection_FORWARD,        ///< down (vertical) or right (horizontal)
    QBListCarouselScrollDirection_FORWARD_STOP,   ///< down (stop stage)
} QBListCarouselScrollDirection;

/**
 * List carousel slot data handler.
 *
 * Called when object associated to slot is changed.
 *
 * @param[in] content            slot content widget handle
 * @param[in] prv                callers private data from set data handler method
 * @param[in] data               data
 * @param[in] distanceFromFocus  distance from focused widget
 *
 * @see QBListCarouselSetDataHandler
 **/
typedef void
(*QBListCarouselSlotContentSetData)(SvWidget content, void *prv, SvObject data, int distanceFromFocus);

/**
 * List carousel slot zoom progress handler.
 *
 * Called when object associated to slot is switching between sizes.
 *
 * @param[in] content   slot content widget handle
 * @param[in] prv       callers private data from set data handler method
 * @param[in] progress  progress in percentages
 *
 * @see QBListCarouselSetDataHandler
 **/
typedef void
(*QBListCarouselSlotContentSetZoomProgress)(SvWidget content, void *prv, int progress);

/**
 * List carousel slot size changed handler.
 *
 * Called when slot size changes.
 *
 * @param[in] content   slot content widget handle
 * @param[in] prv       callers private data from set data handler method
 * @param[in] width     new slot width
 * @param[in] height    new slot height
 *
 * @see QBListCarouselSetDataHandler
 **/
typedef void
(*QBListCarouselSlotSizeChanged)(SvWidget content,
                                 void *prv,
                                 unsigned int width,
                                 unsigned int height);

/**
 * QBListCarousel notification.
 **/
typedef struct QBListCarouselNotification_ {
    QBListCarouselNotificationType type;       ///< notification type
    ssize_t dataIndex;                         ///< index of object for which notification was created
    SvObject dataObject;                       ///< object handle for which notification was created
    SvObject notificationData;                 ///< notification data from set active element method
} *QBListCarouselNotification;

/**
 * QBListCarousel orientation.
 **/
typedef enum {
    QBListCarouselOrientation_vertical,         ///< vertical orientation
    QBListCarouselOrientation_horizontal        ///< horizontal orientation, QBListCarouselScrollType_separate
                                                ///< is not supported
} QBListCarouselOrientation;

/**
 * QBListCarousel initialization parameters.
 **/
typedef struct QBListCarouselParams_ {
    int width;                              ///< widget width
    int height;                             ///< widget height
    QBListCarouselOrientation orientation;  ///< list orientation

    int elementsCnt;                        ///< list elements count (visible)
    int focusedElemPosition;                ///< focused element position on the screen (first = 0)
    int verticalSpacing;                    ///< vertical spacing between slots
    int horizontalSpacing;                  ///< horizontal spacing between slots
    int slotWidth;                          ///< slot width
    int slotHeight;                         ///< slot height
    int smallSlotHeight;                    ///< small slot height (in horizontal orientation is not used)
    int smallSlotWidth;                     ///< small slot width (in vertical orientation is not used)

    bool loopedList;                        ///< is list looped
    double scrollAcceleration;              ///< scroll effect acceleration
    double scrollMaxSpeed;                  ///< scroll effect maximum speed

    bool showEmptySlots;                    ///< should empty slots be visible

    QBListCarouselScrollType scrollType;    ///< scroll animation type
} QBListCarouselParams;

/**
 * List carousel class
 *
 * @class QBListCarousel QBListCarousel.h <QBCarousel/QBListCarousel.h>
 * @extends SvObject
 * @implements QBListModelListener
 *
 * This class is wrapped by SvWidget.
 **/
typedef struct QBListCarousel_ *QBListCarousel;

/**
 * Create new list carousel widget.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in]  app      CUIT application handle
 * @param[in]  name     widget name
 * @param[in]  params   list carousel parameters
 * @param[out] errorOut error info
 * @return              new QBListCarousel instance
 **/
SvWidget
QBListCarouselNew(SvApplication app,
                  const char *name,
                  QBListCarouselParams *params,
                  SvErrorInfo *errorOut);

/**
 * Create new list carousel widget using settings from Settings Manager.
 *
 * @public @memberof QBListCarousel
 *
 * This method creates list carousel widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use the same parameters like
 * as defined in @ref QBListCarouselParams.
 *
 * | Name               | Type    | Description
 * | ------------------ | ------- | ----------------
 * | width              | integer | widget width
 * | height             | integer | widget height
 * | elemsCount         | integer | visible elems count
 * | verticalSpacing    | integer | vertical slot spacing
 * | horizontalSpacing  | integer | horizontal slot spacing
 * | slotWidth          | integer | slot widget width
 * | slotHeight         | integer | slot widget height
 * | smallSlotHeight    | integer | small slot widget height
 * | loopedList         | bool    | should list be looped
 * | scrollAcceleration | double  | scroll efect acceleration
 * | scrollMaxSpeed     | double  | scroll effect max speed
 * | showEmptySlots     | bool    | should empty slots be visible
 *
 * @param[in]  app       CUIT application handle
 * @param[in]  name      widget name
 * @param[out] errorOut  error info
 * @return               new QBListCarousel instance
 *
 * @see QBListCarouselParams
 **/
SvWidget
QBListCarouselNewFromSM(SvApplication app,
                        const char *name,
                        SvErrorInfo *errorOut);

/**
 * Connect list carousel to data source.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in]  w          list carousel widget handle
 * @param[in]  dataSource object implementing QBListModel interface
 * @param[out] errorOut   error info
 **/
void
QBListCarouselConnectToDataSource(SvWidget w,
                                  SvObject dataSource,
                                  SvErrorInfo *errorOut);

/**
 * Set slot's data handler and slot's size handler.
 *
 * @public @memberof QBListCarousel
 *
 * Slot's data handler will be called every time, the object associated to slot
 * will change. Use it to modify appearance of the content widget attached to
 * the slot.
 * Slot's size handler will be called when size of slot will change.
 *
 * @param[in]  w                        VoD matrix widget handle
 * @param[in]  slotIndex                slot index
 * @param[in]  prv                      callers private data used as argument for
 *                                      set data, set zoom progress and slot size changed callbacks
 * @param[in]  setData                  data handler
 * @param[in]  zoomProgess              zoom progress handler
 * @param[in]  slotSizeChanged          slot size changed handler
 * @param[out] errorOut                 error info
 **/
void
QBListCarouselSetDataHandler(SvWidget w,
                             size_t slotIndex,
                             void *prv,
                             QBListCarouselSlotContentSetData setData,
                             QBListCarouselSlotContentSetZoomProgress zoomProgress,
                             QBListCarouselSlotSizeChanged slotSizeChanged,
                             SvErrorInfo *errorOut);

/**
 * Attach content widget to specified slot.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in]  w         list carousel widget handle
 * @param[in]  slotIndex slot index
 * @param[in]  content   new slot content
 * @param[out] errorOut  error info
 **/
void
QBListCarouselSetContentAtIndex(SvWidget w,
                                size_t slotIndex,
                                SvWidget content,
                                SvErrorInfo *errorOut);

/**
 * Get content widget attached to specified slot.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in]  w         list carousel widget handle
 * @param[in]  slotIndex slot index
 * @param[out] errorOut  error info
 * @return               slot content
 **/
SvWidget
QBListCarouselGetContentAtIndex(SvWidget w,
                                size_t slotIndex,
                                SvErrorInfo *errorOut);

/**
 * Get focused slot index.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in]  w         list carousel widget handle
 * @param[out] errorOut  error info
 * @return               focused slot index
 **/
int
QBListCarouselGetFocusedIndex(SvWidget w,
                              SvErrorInfo *errorOut);

/**
 * Get list total slots count.
 *
 * @public @memberof QBListCarousel
 *
 * @note When list is not looped, slots count is greater than number of slots
 * visible on screen. There is one additional elem before and after the screen
 * used to render next elements.
 *
 * @param[in] w list carousel widget handle
 * @return      slots count
 **/
size_t
QBListCarouselGetSlotsCount(SvWidget w);

/**
 * Set active element.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in]  w                        list carousel widget handle
 * @param[in]  dataIndex                data index
 * @param[in]  notificationData         caller notificationData that should be added to QBListCarouselNotification
 * @param[out] errorOut                 error info
 **/
void
QBListCarouselSetActiveElement(SvWidget w,
                               size_t dataIndex,
                               SvObject notificationData,
                               SvErrorInfo *errorOut);

/**
 * List carousel realign callback.
 *
 * Called after list realignment. Use this callback to synchronize other
 * matrices by calling @ref QBListCarouselRealign inside callback.
 *
 * @param[in] prv    callback's private data
 * @param[in] w      list carousel widget handle
 * @param[in] scroll scroll direction
 * @param[in] offset number of pixels to scroll
 *
 * @see QBListCarouselRealign
 **/
typedef void
(*QBListCarouselRealignCallback)(void *prv,
                                 SvWidget w,
                                 QBListCarouselScrollDirection scroll,
                                 int offset);

/**
 * Realign slots position.
 *
 * @public @memberof QBListCarousel
 *
 * Use this function to synchronize position with other matrices.
 *
 * This function will call @ref QBListCarouselRealignCallback.
 * Make sure, that there is no cycle between @ref QBListCarouselRealign and
 * @ref QBListCarouselRealignCallback.
 *
 * @param [in] w         list carousel widget handle
 * @param [in] direction scroll direction
 * @param [in] offset    the number of pixels to scroll
 *
 * @see QBListCarouselRealignCallback
 * @see QBListCarouselSetRealignCallback
 **/
void
QBListCarouselRealign(SvWidget w,
                      QBListCarouselScrollDirection direction,
                      int offset);

/**
 * Set realign callback.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in] w         list carousel widget handle
 * @param[in] callback  realign callback
 * @param[in] prv       realign callback private data
 *
 * @see QBListCarouselRealign
 * @see QBListCarouselRealignCallback
 **/
void
QBListCarouselSetRealignCallback(SvWidget w,
                                 QBListCarouselRealignCallback callback,
                                 void *prv);

/**
 * Set notification target.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in] w         list carousel widget handle
 * @param[in] target    id of widget to be notified
 **/
void
QBListCarouselSetNotificationTarget(SvWidget w,
                                    SvWidgetId target);

/**
 * Use the list carousel widget to handle input event.
 *
 * @public @memberof QBListCarousel
 *
 * @param[in] w       list carousel widget handle
 * @param[in] e       event to be handled
 *
 * @return            flag indicating if the event was handled
 **/
bool
QBListCarouselHandleInputEvent(SvWidget w, SvInputEvent e);

/**
 * @}
 **/

#endif /* QBLIST_CAROUSEL_H_ */
