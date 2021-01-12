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

#ifndef CAROUSEL_MENU_PRIVATE_H_
#define CAROUSEL_MENU_PRIVATE_H_

#include "Menus/carouselMenu.h"
#include <SvFoundation/SvObject.h>


/**
 * QBCarouselMenuItemService class internals.
 **/
struct QBCarouselMenuItemService_ {
    struct SvObject_ super_;

    AppGlobals appGlobals;
    SvString service, serviceType, serviceName;
    SvString serviceLayout;

    SvGenericObject provider;
    SvGenericObject itemController;
    SvGenericObject itemChoice;
    SvGenericObject skinController;
    SvGenericObject contentTree;

    SvGenericObject path;

    QBActiveTree ownerTree;

    SvWidget errorDialog;
    int settingsCtx;

    bool topLevel, clearOnFocusLost, focused;
};

/**
 * QBCarouselMenuItemService virtual methods.
 **/
typedef const struct QBCarouselMenuItemServiceVTable_ {
    /// virtual methods of the base class
    struct SvObjectVTable_ super_;

    /**
     * Method called when service is mounted.
     *
     * @param[in] self carousel menu item service handle
     **/
    void (*mounted)(QBCarouselMenuItemService self);

    /**
     * Method called when service is unmounted.
     *
     * @param[in] self carousel menu item service handle
     **/
    void (*unmounted)(QBCarouselMenuItemService self);
} *QBCarouselMenuItemServiceVTable;


static inline void
QBCarouselMenuItemServiceMounted(QBCarouselMenuItemService self)
{
    SvInvokeVirtual(QBCarouselMenuItemService, self, mounted);
}

static inline void
QBCarouselMenuItemServiceUnmounted(QBCarouselMenuItemService self)
{
    SvInvokeVirtual(QBCarouselMenuItemService, self, unmounted);
}

#endif
