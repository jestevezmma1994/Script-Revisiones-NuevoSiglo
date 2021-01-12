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

#ifndef QBVOD_MATRIX_H_
#define QBVOD_MATRIX_H_

/**
 * @file QBVoDMatrix.h VoD matrix widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBVoDMatrix QBVoDMatrix modules
 * @ingroup QBTileGrid_widgets
 * @{
 *
 * Default CubiConnect VoD content browser
 **/

/**
 * VoD Matrix notification user event type.
 *
 * Use this value in widget's user event handler to check the type of incoming event.
 *
 * @see svUserEventCreate()
 **/
#define SV_EVENT_VOD_MATRIX_NOTIFICATION 358

/**
 * QBVoDMatrix notification type.
 **/
typedef enum {
    QBVoDMatrixNotificationType_matrixShown,     ///< matrix shown
    QBVoDMatrixNotificationType_matrixHidden,    ///< matrix hidden
    QBVoDMatrixNotificationType_activeElement,   ///< element activated (focused)
    QBVoDMatrixNotificationType_selectedElement, ///< element selected
} QBVoDMatrixNotificationType;

/**
 * QBVoDMatrix scroll direction.
 **/
typedef enum QBVoDMatrixScrollDirection_ {
    QBVoDMatrixScrollDirection_IDLE = 0,    ///< idle
    QBVoDMatrixScrollDirection_UP,          ///< up (acceleration phase)
    QBVoDMatrixScrollDirection_UP_STOP,     ///< up (deacceleration phase)
    QBVoDMatrixScrollDirection_DOWN,        ///< down (acceleration phase)
    QBVoDMatrixScrollDirection_DOWN_STOP,   ///< down (deacceleration phase)
} QBVoDMatrixScrollDirection;

/**
 * VoD matrix slot data handler.
 *
 * Called when object associated to slot is changed.
 *
 * @param[in] content                           slot content widget handle
 * @param[in] prv                               caller's private data from set data handler method
 * @param[in] data                              data
 * @param[in] verticalDistanceFromFocus         vertical distance from focus in slots
 * @param[in] horizontalDistanceFromFocus       horizontal distance from focus in slots
 *
 * @see QBVoDMatrixSetDataHandler
 **/
typedef void
(*QBVoDMatrixSlotContentSetData)(SvWidget content, void *prv, SvObject data,
                                 int verticalDistanceFromFocus,
                                 int horizontalDistanceFromFocus);

/**
 * QBVoDMatrix notification.
 **/
typedef struct QBVoDMatrixNotification_ {
    QBVoDMatrixNotificationType type; ///< notification type
    ssize_t dataIndex;                ///< index of object for which notification was created
    SvObject dataObject;              ///< object handle for which notification was created
    int xOff;                         ///< focused slot x offset
    int yOff;                         ///< focused slot y offset
    SvObject notificationData;        ///< notification data from set active element method
} *QBVoDMatrixNotification;

/**
 * QBVoDMatrix initialization parameters.
 **/
typedef struct QBVoDMatrixParams_ {
    int width;                  ///< widget width
    int height;                 ///< widget height

    int rowsCnt;                ///< matrix rows count (visible)
    int columnsCnt;             ///< matrix columns count
    int verticalSpacing;        ///< vertical spacing between slots
    int horizontalSpacing;      ///< horizontal spacing between slots
    int slotWidth;              ///< slot width
    int slotHeight;             ///< slot height

    double scrollAcceleration;  ///< scroll effect acceleration
    double scrollMaxSpeed;      ///< scroll effect maximum speed

    double scatterDuration;     ///< scatter effect duration

    bool showEmptySlots;        ///< should empty slots be visible
} QBVoDMatrixParams;

/**
 * VoD matrix class
 *
 * @class QBVoDMatrix QBVoDMatrix.h <QBCarousel/QBVoDMatrix.h>
 * @extends SvObject
 * @implements QBListModelListener
 *
 * This class is wrapped by SvWidget and in most cases, direct access to
 * QBVoDMatrix object instance is not needed. However it is available at
 * SvWidget::prv field.
 *
 **/
typedef struct QBVoDMatrix_ *QBVoDMatrix;

/**
 * Create new VoD matrix widget.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  app      CUIT application handle
 * @param[in]  name     widget name
 * @param[in]  params   VoD matrix parameters
 * @param[out] errorOut error info
 * @return              new QBVoDMatrix instance
 *
 **/
SvWidget
QBVoDMatrixNew(SvApplication app,
               const char *name,
               QBVoDMatrixParams *params,
               SvErrorInfo *errorOut);

/**
 * Create new VoD matrix widget using settings from Settings Manager.
 *
 * @public @memberof QBVoDMatrix
 *
 * This method creates VoD matrix widget. Appearance of this widget is
 * controlled by the Settings Manager. It will use the same parameters like
 * defined in @ref QBVoDMatrixParams.
 *
 * | Name               | Type    | Description
 * | ------------------ | ------- | ----------------
 * | width              | integer | widget width
 * | height             | integer | widget height
 * | rowsCount          | integer | visible rows count
 * | columnsCount       | integer | columns count
 * | offscreenRows      | integer | count of the rows rendered above the screen
 * | verticalSpacing    | integer | vertical slot spacing
 * | horizontalSpacing  | integer | horizontal slot spacing
 * | slotWidth          | integer | slot widget width
 * | slotHeight         | integer | slot widget height
 * | scrollAcceleration | double  | scroll efect acceleration
 * | scrollMaxSpeed     | double  | scroll effect max speed
 * | scatterDuration    | double  | scatter effect duration
 * | showEmptySlots     | bool    | should empty slots be visible
 *
 * @param[in]  app       CUIT application handle
 * @param[in]  name      widget name
 * @param[out] errorOut  error info
 * @return               new QBVoDMatrix instance
 *
 * @see QBVoDMatrixParams
 *
 **/
SvWidget
QBVoDMatrixNewFromSM(SvApplication app,
                     const char *name,
                     SvErrorInfo *errorOut);

/**
 * Connect VoD matrix to data source.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w          VoD matrix widget handle
 * @param[in]  dataSource object implementing QBListModel interface
 * @param[out] errorOut   error info
 *
 **/
void
QBVoDMatrixConnectToDataSource(SvWidget w,
                               SvObject dataSource,
                               SvErrorInfo *errorOut);

/**
 * Set slot's data handler.
 *
 * @public @memberof QBVoDMatrix
 *
 * Slot's data handler will be called every time, the object associated to slot
 * will change. Use it to modify appearance of the content widget attached to
 * the slot.
 *
 * @param[in]  w         VoD matrix widget handle
 * @param[in]  slotIndex slot index
 * @param[in]  prv       caller's private data used as argument for set data callback
 * @param[in]  setData   data handler
 * @param[out] errorOut  error info
 *
 **/
void
QBVoDMatrixSetDataHandler(SvWidget w,
                          size_t slotIndex,
                          void *prv,
                          QBVoDMatrixSlotContentSetData setData,
                          SvErrorInfo *errorOut);

/**
 * Attach content widget to specified slot.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w         VoD matrix widget handle
 * @param[in]  slotIndex slot index
 * @param[in]  content   new slot content
 * @param[out] errorOut  error info
 *
 **/
void
QBVoDMatrixSetContentAtIndex(SvWidget w,
                             size_t slotIndex,
                             SvWidget content,
                             SvErrorInfo *errorOut);

/**
 * Get content widget attached to specified slot.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w         VoD matrix widget handle
 * @param[in]  slotIndex slot index
 * @param[out] errorOut  error info
 * @return               slot content
 *
 **/
SvWidget
QBVoDMatrixGetContentAtIndex(SvWidget w,
                             size_t slotIndex,
                             SvErrorInfo *errorOut);

/**
 * Get matrix total slots count.
 *
 * @public @memberof QBVoDMatrix
 *
 * @note Slots count is greater than number of slots visible on screen.
 * There is one additional row above and below the screen used to render next
 * elements.
 *
 * @param[in] w VoD matrix widget handle
 * @return      slots count
 *
 **/
size_t
QBVoDMatrixGetSlotCount(SvWidget w);

/**
 * Get content widget attached to slot at specified position.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w         VoD matrix widget handle
 * @param[in]  row       row
 * @param[in]  column    column
 * @param[out] errorOut  error info
 * @return               slot content
 *
 **/
SvWidget
QBVoDMatrixGetContentAtPosition(SvWidget w,
                                size_t row,
                                size_t column,
                                SvErrorInfo *errorOut);

/**
 * Get rows count.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w        VoD matrix widget handle
 * @return              rows count
 **/
size_t
QBVoDMatrixGetRowsCount(SvWidget w);

/**
 * Get columns count.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w        VoD matrix widget handle
 * @return              columns count
 **/
size_t
QBVoDMatrixGetColumnsCount(SvWidget w);

/**
 * Show VoD matrix.
 *
 * @public @memberof QBVoDMatrix
 *
 * This function will start slots scatter effect. All slots will be moved to
 * their destination position. When finished, @ref QBVoDMatrixNotificationType_matrixShown
 * notification will be sent.
 *
 * @param[in] w             VoD matrix widget handle
 * @param[in] immediately   if @c true action will be immediate, otherwise
 *                          effect will last \ref QBVoDMatrixParams::scatterDuration
 *
 **/
void
QBVoDMatrixShow(SvWidget w, bool immediately);

/**
 * Hide VoD matrix.
 *
 * @public @memberof QBVoDMatrix
 *
 * This function will start slots scatter effect. All slots will be moved to
 * their initial position (just before the matrix left border). When finished,
 * @ref QBVoDMatrixNotificationType_matrixHidden notification will be sent.
 *
 * @param[in] w             VoD matrix widget handle
 * @param[in] immediately   if @c true action will be immediate, otherwise
 *                          effect will last \ref QBVoDMatrixParams::scatterDuration
 *
 **/
void
QBVoDMatrixHide(SvWidget w, bool immediately);

/**
 * Set active element.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w                        VoD matrix widget handle
 * @param[in]  dataIndex                data index
 * @param[in]  notificationData         caller notification data that should be added to QBVoDMatrixNotification
 * @param[out] errorOut                 error info
 *
 **/
void
QBVoDMatrixSetActiveElement(SvWidget w,
                            size_t dataIndex,
                            SvObject notificationData,
                            SvErrorInfo *errorOut);

/**
 * VoD matrix realign callback.
 *
 * Called after matrix realignment. Use this callback to synchronize other
 * matrices by calling @ref QBVoDMatrixRealign inside callback.
 *
 * @param[in] prv    callback's private data
 * @param[in] w      VoD matrix widget handle
 * @param[in] scroll scroll direction
 * @param[in] offset number of pixels to scroll
 *
 * @see QBVoDMatrixRealign
 **/
typedef void
(*QBVoDMatrixRealignCallback)(void *prv,
                              SvWidget w,
                              QBVoDMatrixScrollDirection scroll,
                              int offset);

/**
 * VoD matrix focused column callback.
 *
 * Called when user switches focused column. Use this callback to synchronize other
 * matrices by calling @ref QBVoDMatrixSetFocusedColumn inside callback.
 *
 * @param[in] prv               callback's private data
 * @param[in] w                 VoD matrix widget handle
 * @param[in] focusedColumn     new focused column
 *
 * @see QBVoDMatrixSetFocusedColumn
 **/
typedef void
(*QBVoDMatrixColumnFocusedCallback)(void *prv,
                                    SvWidget w,
                                    int focusedColumn);

/**
 * Realign slots position.
 *
 * @public @memberof QBVoDMatrix
 *
 * Use this function to synchronize position with other matrices.
 *
 * @warning Possible infinite recursion.
 * This function will call @ref QBVoDMatrixRealignCallback.
 * Make sure, that there is no cycle between @ref QBVoDMatrixRealign and
 * @ref QBVoDMatrixRealignCallback.
 *
 * @param [in] w         VoD matrix widet handle
 * @param [in] direction scroll direction
 * @param [in] offset    the number of pixels to scroll
 *
 * @see QBVoDMatrixRealignCallback
 * @see QBVoDMatrixSetRealignCallback
 *
 **/
void
QBVoDMatrixRealign(SvWidget w,
                   QBVoDMatrixScrollDirection direction,
                   int offset);

/**
 * Set realign callback.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in] w         VoD matrix widget handle
 * @param[in] callback  realign callback
 * @param[in] prv       realign callback private data
 *
 * @see QBVoDMatrixRealign
 * @see QBVoDMatrixRealignCallback
 *
 **/
void
QBVoDMatrixSetRealignCallback(SvWidget w,
                              QBVoDMatrixRealignCallback callback,
                              void *prv);

/**
 * Set focused column.
 *
 * @public @memberof QBVoDMatrix
 *
 * Use this function to synchronize focused column with other matrices.
 *
 * @param [in] w                VoD matrix widget handle
 * @param [in] focusedColumn    focused column to set
 *
 * @see QBVoDMatrixColumnFocusedCallback
 * @see QBVoDMatrixSetColumnFocusedCallback
 *
 **/
void
QBVoDMatrixSetFocusedColumn(SvWidget w,
                            int focusedColumn);

/**
 * Get focused column.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w        VoD matrix widget handle
 * @return              focused column
 **/
size_t
QBVoDMatrixGetFocusedColumn(SvWidget w);

/**
 * Get focused row.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in]  w        VoD matrix widget handle
 * @return              focused row
 **/
size_t
QBVoDMatrixGetFocusedRow(SvWidget w);

/**
 * Set notification target.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in] w         VoD matrix widget handle
 * @param[in] target    id of widget to be notified
 **/
void
QBVoDMatrixSetNotificationTarget(SvWidget w,
                                 SvWidgetId target);

/**
 * Set focused column callback.
 *
 * @public @memberof QBVoDMatrix
 *
 * @param[in] w         VoD matrix widget handle
 * @param[in] callback  focused column callback
 * @param[in] prv       focused column callback private data
 *
 * @see QBVoDMatrixSetFocusedColumn
 * @see QBVoDMatrixColumnFocusedCallback
 *
 **/
void
QBVoDMatrixSetColumnFocusedCallback(SvWidget w,
                                    QBVoDMatrixColumnFocusedCallback callback,
                                    void *prv);

/**
 * Use VoDMatrix to handle input event.
 *
 * @param[in] w       VoD matrix handle
 * @param[in] e       event to be handled
 *
 * @return            flag indicating if the event was handled
 **/
bool
QBVoDMatrixHandleInputEvent(SvWidget w, SvInputEvent e);

/**
 * @}
 **/

#endif /* QBVOD_MATRIX_H_ */
