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

#ifndef CAROUSEL_MENU_H_
#define CAROUSEL_MENU_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <CUIT/Core/types.h>
#include <main_decl.h>
#include <stdbool.h>


struct QBCarouselMenuInitParams_s {
    SvString serviceName;
    SvString serviceType;
    SvString serviceID;
    SvString serviceLayout;
    SvGenericObject provider;
    SvGenericObject itemController;
    SvGenericObject itemChoice;
    SvGenericObject contentTree;
};


typedef struct QBCarouselMenuItemService_ *QBCarouselMenuItemService;

SvType
QBCarouselMenuItemService_getType(void);

void
QBCarouselMenuInitWithParams(QBCarouselMenuItemService self, AppGlobals appGlobals, struct QBCarouselMenuInitParams_s *params);

QBCarouselMenuItemService
QBCarouselMenuCreate(AppGlobals appGlobals, SvString serviceName, SvString serviceType,
                     SvString serviceID, SvString serviceLayout,
                     SvGenericObject provider, SvGenericObject itemController,
                     SvGenericObject itemChoice, SvGenericObject skinController, SvGenericObject contentTree);

int QBCarouselMenuMount(QBCarouselMenuItemService info, SvWidget menuBar,
                        SvGenericObject path, QBTreePathMap pathMap, QBActiveTree externalTree, QBTreePathMap skinPathMap);
int QBCarouselMenuUnmount(QBCarouselMenuItemService self, SvWidget menuBar, QBTreePathMap pathMap, QBTreePathMap skinPathMap);

SvGenericObject QBCarouselMenuGetPath(QBCarouselMenuItemService self);

bool QBCarouselMenuIsMounted(QBCarouselMenuItemService self);

SvString QBCarouselMenuGetServiceID(QBCarouselMenuItemService self);

SvString QBCarouselMenuGetServiceName(QBCarouselMenuItemService self);

SvString QBCarouselMenuGetServiceLayout(QBCarouselMenuItemService self);

QBActiveTree QBCarouselMenuGetOwnerTree(QBCarouselMenuItemService self);

SvGenericObject QBCarouselMenuGetProvider(QBCarouselMenuItemService self);

int QBCarouselMenuPushSettingsComponent(SvString serviceName, SvString serviceType);

bool QBCarouselMenuPushContext(QBCarouselMenuItemService self, QBActiveTree externalTree);

#endif /* CAROUSEL_MENU_H_ */
