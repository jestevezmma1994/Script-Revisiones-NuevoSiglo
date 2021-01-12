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

#ifndef QB_VERTICAL_CAROUSEL_2_H_
#define QB_VERTICAL_CAROUSEL_2_H_

/**
 * @file QBVerticalCarousel2.h Vertical carousel 2 widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBVerticalCarousel2 QBVerticalCarousel2 modules
 * @ingroup QBTileGrid_widgets
 * @{
 *
 * Vertical carousel version 2
 **/

/**
 * Vertical carousel notification user event type.
 *
 * Use this value in widget's user event handler to check the type of incoming event.
 *
 * @see svUserEventCreate()
 **/
#define QB_VERTICAL_CAROUSEL_2_NOTIFICATION 92

/**
 * QBVerticalCarousel2 notification type.
 **/
typedef enum {
    QBVerticalCarousel2NotificationType_activeElement,   ///< element activated (focused)
    QBVerticalCarousel2NotificationType_selectedElement, ///< element selected
} QBVerticalCarousel2NotificationType;

/**
 * QBVerticalCarousel2 scroll direction.
 **/
typedef enum QBVerticalCarousel2ScrollDirection_ {
    QBVerticalCarousel2ScrollDirection_IDLE,        ///< idle
    QBVerticalCarousel2ScrollDirection_UP,          ///< up
    QBVerticalCarousel2ScrollDirection_UP_STOP,     ///< up (stop stage)
    QBVerticalCarousel2ScrollDirection_DOWN,        ///< down
    QBVerticalCarousel2ScrollDirection_DOWN_STOP,   ///< down (stop stage)
} QBVerticalCarousel2ScrollDirection;

/**
 * Vertical carousel slot data handler.
 *
 * Called when object associated to slot is changed.
 *
 * @param[in] content            slot content widget handle
 * @param[in] prv                caller's private data from set data handler method
 * @param[in] data               data
 * @param[in] distanceFromFocus  distance from focused widget
 *
 * @see QBVerticalCarousel2SetDataHandler
 **/
typedef void
(*QBVerticalCarousel2SlotContentSetData)(SvWidget content, void *prv, SvObject data, int distanceFromFocus);

/**
 * QBVerticalCarousel2 notification.
 **/
typedef struct QBVerticalCarousel2Notification_ {
    QBVerticalCarousel2NotificationType type;  ///< notification type
    ssize_t dataIndex;                         ///< index of object for which notification was created
    SvObject dataObject;                       ///< object handle for which notification was created
    SvObject notificationData;                 ///< notification data from set active element method
} *QBVerticalCarousel2Notification;

/**
 * QBVerticalCarousel2 initialization parameters.
 **/
typedef struct QBVerticalCarousel2Params_ {
    int width;                  ///< widget width
    int height;                 ///< widget height

    int rowsCnt;                ///< matrix rows count (visible)
    int verticalSpacing;        ///< vertical spacing between slots
    int horizontalSpacing;      ///< horizontal spacing between slots
    int slotWidth;              ///< slot width
    int slotHeight;             ///< slot height

    bool isLooped;            ///< is list looped
    double scrollAcceleration;  ///< scroll effect acceleration
    double scrollMaxSpeed;      ///< scroll effect maximum speed

    bool showEmptySlots;        ///< should empty slots be visible
} QBVerticalCarousel2Params;

/**
 * Vertical carousel class
 *
 * @class QBVerticalCarousel2 QBVerticalCarousel2.h <QBCarousel/QBVerticalCarousel2.h>
 * @extends SvObject
 * @implements QBListModelListener
 *
 * This class is wrapped by SvWidget.
 **/
typedef struct QBVerticalCarousel2_ *QBVerticalCarousel2;

/**
 * Create new vertical carousel widget.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in]  app      CUIT application handle
 * @param[in]  name     widget name
 * @param[in]  params   vertical carousel parameters
 * @param[out] errorOut error info
 * @return              new vertical carousel widget handle
 **/
SvWidget
QBVerticalCarousel2New(SvApplication app,
                       const char *name,
                       QBVerticalCarousel2Params *params,
                       SvErrorInfo *errorOut);

/**
 * Create new vertical carousel widget using settings from Settings Manager.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * This method creates vertical carousel widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use the same parameters
 * as defined in @ref QBVerticalCarousel2Params.
 *
 * | Name               | Type    | Description
 * | ------------------ | ------- | ----------------
 * | width              | integer | widget width
 * | height             | integer | widget height
 * | rowsCount          | integer | visible rows count
 * | verticalSpacing    | integer | vertical slot spacing
 * | horizontalSpacing  | integer | horizontal slot spacing
 * | slotWidth          | integer | slot widget width
 * | slotHeight         | integer | slot widget height
 * | isLooped           | bool    | should list be looped
 * | scrollAcceleration | double  | scroll efect acceleration
 * | scrollMaxSpeed     | double  | scroll effect max speed
 * | showEmptySlots     | bool    | should empty slots be visible
 *
 * @param[in]  app       CUIT application handle
 * @param[in]  name      widget name
 * @param[out] errorOut  error info
 * @return               new vertical carousel widget handle
 *
 * @see QBVerticalCarousel2Params
 **/
SvWidget
QBVerticalCarousel2NewFromSM(SvApplication app,
                             const char *name,
                             SvErrorInfo *errorOut);

/**
 * Connect vertical carousel to data source.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in]  w          vertical carousel widget handle
 * @param[in]  dataSource object implementing QBListModel interface
 * @param[out] errorOut   error info
 **/
void
QBVerticalCarousel2ConnectToDataSource(SvWidget w,
                                       SvObject dataSource,
                                       SvErrorInfo *errorOut);

/**
 * Set slot's data handler.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * Slot's data handler will be called every time, the object associated to slot
 * will change. Use it to modify appearance of the content widget attached to
 * the slot.
 * Slot's size handler will be called when size of slot will change.
 *
 * @param[in]  w                        vertical carousel widget handle
 * @param[in]  slotIndex                slot index
 * @param[in]  prv                      caller's private data used as argument for set data callback
 * @param[in]  setData                  data handler
 * @param[in]  zoomProgess              zoom progress handler
 * @param[in]  slotSizeChanged          slot size changed handler
 * @param[out] errorOut                 error info
 **/
void
QBVerticalCarousel2SetDataHandler(SvWidget w,
                                  size_t slotIndex,
                                  void *prv,
                                  QBVerticalCarousel2SlotContentSetData setData,
                                  SvErrorInfo *errorOut);

/**
 * Attach content widget to specified slot.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in]  w         vertical carousel widget handle
 * @param[in]  slotIndex slot index
 * @param[in]  content   new slot content
 * @param[out] errorOut  error info
 **/
void
QBVerticalCarousel2SetContentAtIndex(SvWidget w,
                                     size_t slotIndex,
                                     SvWidget content,
                                     SvErrorInfo *errorOut);

/**
 * Get content widget attached to specified slot.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in]  w         vertical carousel widget handle
 * @param[in]  slotIndex slot index
 * @param[out] errorOut  error info
 * @return               slot content
 **/
SvWidget
QBVerticalCarousel2GetContentAtIndex(SvWidget w,
                                     size_t slotIndex,
                                     SvErrorInfo *errorOut);

/**
 * Get focused slot index.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in]  w         vertical carousel widget handle
 * @param[out] errorOut  error info
 * @return               focused slot index
 **/
int
QBVerticalCarousel2GetFocusedIndex(SvWidget w,
                                   SvErrorInfo *errorOut);

/**
 * Get list total slots count.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @note When carousel is not looped, slots count is greater than number of slots
 * visible on screen. There is one additional row above and below the screen
 * used to render next elements.
 *
 * @param[in] w vertical carousel widget handle
 * @return      slots count
 **/
size_t
QBVerticalCarousel2GetSlotsCount(SvWidget w);

/**
 * Set active element.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in]  w                        vertical carousel widget handle
 * @param[in]  dataIndex                data index
 * @param[in]  notificationData         caller notificationData that should be added to QBVerticalCarousel2Notification
 * @param[out] errorOut                 error info
 **/
void
QBVerticalCarousel2SetActiveElement(SvWidget w,
                                    size_t dataIndex,
                                    SvObject notificationData,
                                    SvErrorInfo *errorOut);

/**
 * Vertical carousel realign callback.
 *
 * Called after carousel realignment. Use this callback to synchronize other
 * matrices by calling @ref QBVerticalCarousel2Realign inside callback.
 *
 * @param[in] prv    callback's private data
 * @param[in] w      vertical carousel widget handle
 * @param[in] scroll scroll direction
 * @param[in] offset number of pixels to scroll
 *
 * @see QBVerticalCarousel2Realign
 **/
typedef void
(*QBVerticalCarousel2RealignCallback)(void *prv,
                                      SvWidget w,
                                      QBVerticalCarousel2ScrollDirection scroll,
                                      int offset);

/**
 * Realign slots position.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * Use this function to synchronize position with other matrices.
 *
 * This function will call @ref QBVerticalCarousel2RealignCallback.
 * Make sure, that there is no cycle between @ref QBVerticalCarousel2Realign and
 * @ref QBVerticalCarousel2RealignCallback.
 *
 * @param [in] w         vertical carousel widget handle
 * @param [in] direction scroll direction
 * @param [in] offset    the number of pixels to scroll
 *
 * @see QBVerticalCarousel2RealignCallback
 * @see QBVerticalCarousel2SetRealignCallback
 **/
void
QBVerticalCarousel2Realign(SvWidget w,
                           QBVerticalCarousel2ScrollDirection direction,
                           int offset);

/**
 * Set realign callback.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in] w         vertical carousel widget handle
 * @param[in] callback  realign callback
 * @param[in] prv       realign callback private data
 *
 * @see QBVerticalCarousel2Realign
 * @see QBVerticalCarousel2RealignCallback
 **/
void
QBVerticalCarousel2SetRealignCallback(SvWidget w,
                                      QBVerticalCarousel2RealignCallback callback,
                                      void *prv);

/**
 * Set notification target.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in] w         vertical carousel widget handle
 * @param[in] target    id of widget to be notified
 **/
void
QBVerticalCarousel2SetNotificationTarget(SvWidget w,
                                         SvWidgetId target);

/**
 * Use the vertical carousel widget to handle input event.
 *
 * @public @memberof QBVerticalCarousel2
 *
 * @param[in] w       vertical carousel widget handle
 * @param[in] e       event to be handled
 *
 * @return            flag indicating if the event was handled
 **/
bool
QBVerticalCarousel2HandleInputEvent(SvWidget w, SvInputEvent e);

/**
 * @}
 **/

#endif /* QB_VERTICAL_CAROUSEL_2_H_ */
