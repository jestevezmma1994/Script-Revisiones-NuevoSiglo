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

#ifndef QB_VERTICAL_CAROUSEL_SLOT_H_
#define QB_VERTICAL_CAROUSEL_SLOT_H_

#include <unistd.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <CAGE/Core/SvBitmap.h>

/**
 * @defgroup QBVerticalCarouselSlot QBVerticalCarousel slot
 * @ingroup QBVerticalCarousel
 * @{
 *
 * QBVerticalCarouselSlot is a slot of QBVerticalCarousel
 **/

typedef void
(*QBVerticalCarouselSlotContentSetData)(SvWidget content, SvObject data);

typedef void
(*QBVerticalCarouselSlotStateChanged)(SvWidget w, bool isInactive);

/**
 * Create new QBVerticalCarouselSlot.
 *
 * @param[in] app               application handle
 * @param[in] slotWidth         slot width
 * @param[in] slotHeight        slot height
 * @param[in] index             slot index in QBVerticalCarousel
 * @param[in] arrowDown         arrow down bitmap handle
 * @param[in] arrowDownXOffset  arrow down horizontal offset
 * @param[in] arrowDownYOffset  arrow down vertical offset
 * @param[in] arrowDownWidth    arrow down width
 * @param[in] arrowDownHeight   arrow down height
 * @param[in] arrowUp           arrow up bitmap handle
 * @param[in] arrowUpXOffset    arrow up horizontal offset
 * @param[in] arrowUpYOffset    arrow up vertical offset
 * @param[in] arrowUpWidth      arrow up width
 * @param[in] arrowUpHeight     arrow up height
 **/
extern SvWidget
QBVerticalCarouselSlotNew(SvApplication app, int slotWidth, int slotHeight, ssize_t index,
                          SvBitmap arrowDown, int arrowDownXOffset, int arrowDownYOffset,
                          int arrowDownWidth, int arrowDownHeight,
                          SvBitmap arrowUp, int arrowUpXOffset, int arrowUpYOffset,
                          int arrowUpWidth, int arrowUpHeight);

extern void
QBVerticalCarouselSlotSetDataHandler(SvWidget w, QBVerticalCarouselSlotContentSetData s);

extern ssize_t
QBVerticalCarouselSlotSetDataIndex(SvWidget w, ssize_t newIndex);

extern ssize_t
QBVerticalCarouselSlotGetDataIndex(SvWidget w);

extern void
QBVerticalCarouselSlotSetData(SvWidget w, SvObject data);

/**
 * Set content at slot.
 *
 * @param[in] w             slot widget handle
 * @param[in] content       content to be set
 **/
extern void
QBVerticalCarouselSlotSetContent(SvWidget w, SvWidget content);

/**
 * Get content from slot.
 *
 * @param[in] w             slot widget handle
 * @return                  content of slot
 **/
extern SvWidget
QBVerticalCarouselSlotGetContent(SvWidget w);

extern void
QBVerticalCarouselSlotSetFocus(SvWidget slot);

extern void
QBVerticalCarouselSlotSetArrows(SvWidget w, int off);

extern void
QBVerticalCarouselSlotSetStateChangedCallback(SvWidget w, QBVerticalCarouselSlotStateChanged stateChangedCallback);

extern void
QBVerticalCarouselSlotSetState(SvWidget w, bool isInactive);

/**
 * @}
 **/

#endif
