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

#ifndef QB_CAROUSEL_H_
#define QB_CAROUSEL_H_

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

#include "QBCarouselSlot.h"

typedef void
(*QBCarouselFocusSetData)(SvWidget focus, SvObject data);
typedef void
(*QBCarouselFocusTransform)(SvWidget focus, int offx, int offy, int width, int height);

extern SvWidget QBCarouselNewFromSM(SvApplication app, const char *widgetName);

extern void QBCarouselConnectToSource(SvWidget w, SvObject dataSource, SvErrorInfo *error);

extern void QBCarouselSetActive(SvWidget w, int index);

extern void QBCarouselSetFocusWidget(SvWidget carousel, SvWidget focus);
extern void QBCarouselSetFocusDataHandler(SvWidget carousel, QBCarouselFocusSetData fs);
extern void QBCarouselSetFocusTransformHandler(SvWidget carousel, QBCarouselFocusTransform ts);
extern void QBCarouselDropFocus(SvWidget w);
extern void QBCarouselSetPrefetchRange(SvWidget w, int prefetch);
extern int QBCarouselGetSlotCount(SvWidget w);
extern void QBCarouselSetSlotContentListener(SvWidget w, int slotIndex, SvObject contentListener);


#endif
