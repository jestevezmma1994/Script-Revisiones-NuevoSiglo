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

#ifndef QB_VOD_CAROUSEL_H_
#define QB_VOD_CAROUSEL_H_

/**
 * @file QBVoDCarousel.h VOD Carousel
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBWindowContext.h>

/**
 * @defgroup QBVoDCarousel VOD Carousel
 * @ingroup CubiTV
 * @{
 **/

/**
 * VOD Carousel Context
 *
 * @class QBVoDCarouselContext
 **/
typedef struct QBVoDCarouselContext_ *QBVoDCarouselContext;

/**
 * Get runtime type identification object representing QBVoDCarouselContext class.
 *
 * @return QBVoDCarouselContext class
 **/
SvType
QBVoDCarouselContext_getType(void);

/**
 * method that tells if context is protected because of current parental control settings
 * @param[in] self      QBVoDCarouselContext handle
 * @return              true if context is protected and false otherwise
 */
bool
QBVoDCarouselContextIsParentalControlProtected(QBVoDCarouselContext self);

/**
 * Create new QBVoDCarouselContext
 * @param[in] appGlobals               Application global data handle
 * @param[in] path                     path to the node of VOD tree to which this context will be bound
 * @param[in] provider                 provider handle
 * @param[in] serviceId                id of the service
 * @param[in] serviceName              name of the service
 * @param[in] pcProtectedCategory      true if the content is protected by parental control
 * @param[in] invalidateAuthentication flag indicate if while destroying context authentication should by invalidated
 * @return                             new object or NULL in case of error
 */
extern QBWindowContext
QBVoDCarouselContextCreate(AppGlobals appGlobals, SvGenericObject path,
                           SvGenericObject provider, SvString serviceId,
                           SvString serviceName, bool pcProtectedCategory,
                           bool invalidateAuthentication);


/**
 * @}
**/


#endif
