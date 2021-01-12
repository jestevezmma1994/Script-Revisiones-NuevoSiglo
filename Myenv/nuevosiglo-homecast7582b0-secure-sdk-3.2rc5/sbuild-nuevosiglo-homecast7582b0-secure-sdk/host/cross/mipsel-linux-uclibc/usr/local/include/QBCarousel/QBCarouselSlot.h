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

#ifndef QB_CAROUSEL_SLOT_H_
#define QB_CAROUSEL_SLOT_H_

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>

#define QB_CAROUSEL_SLOT_NOTIFICATION 0x214fa11

typedef enum {
    QB_CAROUSEL_SLOT_ACTIVE,
    QB_CAROUSEL_SLOT_SELECTED,
} QBCarouselSlotNotificationType;

typedef struct QBCarouselSlotNotification_ {
    QBCarouselSlotNotificationType type;
    SvWidget carousel;
    int dataIndex;
    SvObject dataObject;
} *QBCarouselSlotNotification;

extern void
QBCarouselSlotNotify(SvWidget w, QBCarouselSlotNotificationType type, SvWidget carousel, int dataIndex, SvObject data);


typedef struct {
    int lowestDataIndex;
    int highestDataIndex;
    int activeDataIndex;
} QBCarouselSlotContentDataRange;

typedef struct {
    int xOffset;
    int yOffset;
    unsigned int width;
    unsigned int height;
} QBCarouselSlotContentSizeAndOffset;

typedef const struct QBCarouselSlotContent_ {
    bool (*attach)(SvObject self_, SvWidget slotParent, SvWidget carousel);
    bool (*detach)(SvObject self_);
    bool (*addDataObject)(SvObject self_, SvObject data, int dataIndex);
    bool (*clearData)(SvObject self_);
    void (*active)(SvObject self_);
    QBCarouselSlotContentDataRange (*getDataRange)(SvObject self_);
    QBCarouselSlotContentSizeAndOffset (*getSizeAndOffset)(SvObject self_);
    void (*setReplaceMode)(SvObject self_, bool replace);
} *QBCarouselSlotContent;

extern SvInterface
QBCarouselSlotContent_getInterface(void);

// Constructor
extern SvWidget
QBCarouselSlotNew(SvApplication app, int slotWidth, int slotHeight, SvWidget carousel);

extern void
QBCarouselSlotSetContentListener(SvWidget w, SvObject contentListener);

extern void
QBCarouselSlotSetFocus(SvWidget w);

#endif
