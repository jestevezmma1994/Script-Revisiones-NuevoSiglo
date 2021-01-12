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

#ifndef INNOV8ON_CAROUSEL_H_
#define INNOV8ON_CAROUSEL_H_

/**
 * @file innov8onCarouselItemController.h QBInnov8onCarouselMenuItemController class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "Menus/carouselMenu.h"
#include <main_decl.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>
#include <SvDataBucket2/SvDBRawObject.h>

/**
 * @defgroup QBInnov8onCarouselMenuItemController innov8on carousel menu item controller
 * @{
 **/

/**
 * Innov8on carousel menu item controller
 * @class QBInnov8onCarouselMenuItemController
 * @extends SvObject
 * @implements XMBItemController
 * @implements XMBMenuController
 **/
typedef struct QBInnov8onCarouselMenuItemController_t* QBInnov8onCarouselMenuItemController;

/**
 * Creating QBCarouselMenuItemService
 *
 * @param[in] appGlobals                    application globals
 * @param[in] serviceType                   service type
 * @param[in] serviceID                     service id
 * @param[in] serviceName                   service name
 * @param[in] serviceLayout                 service layout
 * @param[in] service                       object representing service
 * @param[in] call2Action                   is call2Action enabled
 * @param[in] externalUrl                   call2Action external url
 * @return                                  @c item service or @c NULL in case of error
 **/
QBCarouselMenuItemService
QBInnov8onCarouselMenuCreate(AppGlobals appGlobals,
                             SvString serviceType,
                             SvString serviceID,
                             SvString serviceName,
                             SvString serviceLayout,
                             SvDBRawObject service,
                             bool call2Action,
                             SvString externalUrl);

/**
 * Create item controller
 *
 * @param[in] appGlobals    application globals
 * @param[in] provider      provider
 * @return                   @c item controller or @c NULL in case of error
 **/
QBInnov8onCarouselMenuItemController
QBInnov8onCarouselMenuItemControllerCreate(AppGlobals appGlobals, SvObject provider);



/**
 * @}
 **/

#endif /* INNOV8ON_CAROUSEL_H_ */
