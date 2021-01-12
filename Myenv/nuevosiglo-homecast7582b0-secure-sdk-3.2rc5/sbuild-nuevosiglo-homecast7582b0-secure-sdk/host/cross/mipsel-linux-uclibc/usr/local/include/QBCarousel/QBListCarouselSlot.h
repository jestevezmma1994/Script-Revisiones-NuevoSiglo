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

#ifndef QB_LIST_CAROUSEL_SLOT_H_
#define QB_LIST_CAROUSEL_SLOT_H_

/**
 * @file QBListCarouselSlot.h List carousel slot widget
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBCarousel/QBListCarousel.h>

#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBListCarouselSlot QBListCarouselSlot
 * @ingroup QBListCarousel
 * @{
 *
 * QBListCarousel slot widget
 **/

/**
 * List carousel slot's move effect callback.
 *
 * @param[in] prv   callback private data
 * @param[in] slot  slot widget handle
 *
 **/
typedef void (*QBListCarouselSlotMoveCallback)(void *prv, SvWidget slot);

/**
 * List carousel slot widget class
 * @class QBListCarouselSlot
 *
 * This class is wrapped by SvWidget.
 **/
typedef struct QBListCarouselSlot_  * QBListCarouselSlot;

/**
 * Create new list carousel widget.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] app        CUIT application handle
 * @param[in] slotWidth  widget width
 * @param[in] slotHeight widget height
 * @param[in] index      data index
 *
 **/
SvWidget
QBListCarouselSlotCreate(SvApplication app,
                         int slotWidth,
                         int slotHeight,
                         ssize_t index);

/**
 * Get slot data index (the index of corresponding object in data source).
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w     list carousel slot widget handle
 * @return          slot data index
 *
 **/
ssize_t
QBListCarouselSlotGetDataIndex(SvWidget w);

/**
 * Set slot data index (the index of corresponding object in data source).
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w     list carousel slot widget handle
 * @param[in] idx   slot data index
 *
 **/
void
QBListCarouselSlotSetDataIndex(SvWidget w, ssize_t idx);

/**
 * Get slot content (the widget attached to the slot).
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w     list carousel slot widget handle
 * @return          slot content
 *
 **/
SvWidget
QBListCarouselSlotGetContent(SvWidget w);

/**
 * Set slot content (the widget attached to the slot).
 *
 * @protected @memberof QBListCarouselSlot
 *
 * @param[in] w         list carousel slot widget handle
 * @param[in] content   slot content
 *
 **/
void
QBListCarouselSlotSetContent(SvWidget w, SvWidget content);

/**
 * Set slot data handler.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w         list carousel slot widget handle
 * @param[in] prv       caller's private data used as argument for
 *                      set data, set zoom progress and slot size changed callbacks
 * @param[in] setData   slot data handler
 *
 **/
void
QBListCarouselSlotSetDataHandler(SvWidget w,
                                 void *prv,
                                 QBListCarouselSlotContentSetData setData);


/**
 * Set slot callbacks.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w             list carousel slot widget handle
 * @param[in] zoomProgress  zoom progress handler
 * @param[in] sizeChanged   size changed handler
 **/
void
QBListCarouselSlotSetCallbacks(SvWidget w,
                               QBListCarouselSlotContentSetZoomProgress zoomProgress,
                               QBListCarouselSlotSizeChanged sizeChanged);

/**
 * Set slot data object.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w                  list carousel slot widget handle
 * @param[in] data               slot data
 * @param[in] distanceFromFocus  distance from focused widget
 *
 **/
void
QBListCarouselSlotSetData(SvWidget w, SvObject data, int distanceFromFocus);

/**
 * Set slot zoom progress.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w         list carousel slot widget handle
 * @param[in] progress  progress
 **/
void
QBListCarouselSlotSetZoomProgress(SvWidget w, int progress);

/**
 * Set slot height.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w         list carousel slot widget handle
 * @param[in] height    new height
 **/
void
QBListCarouselSetSlotHeight(SvWidget w,
                            unsigned int height);

/**
 * Set slot width.
 *
 * @memberof QBListCarouselSlot
 *
 * @param[in] w         list carousel slot widget handle
 * @param[in] height    new width
 **/
void
QBListCarouselSetSlotWidth(SvWidget w,
                           unsigned int width);

/**
 * @}
 **/

#endif /* QB_LIST_CAROUSEL_SLOT_H_ */
