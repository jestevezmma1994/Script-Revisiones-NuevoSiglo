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

#ifndef INNVO8ONCAROUSELITEMCHOICE_H_
#define INNVO8ONCAROUSELITEMCHOICE_H_

/**
 * @file innov8onCarouselItemChoice.h QBInnov8onCarouselMenuItemChoice class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

/**
 * @defgroup QBInnov8onCarouselMenuItemChoice innov8on carousel menu item choice controller
 * @{
 **/

/**
 * Innov8on carousel menu item choice controller
 * @class QBInnov8onCarouselMenuItemChoice
 * @extends SvObject
 * @implements XMBItemController
 * @implements XMBMenuController
 **/
typedef struct QBInnov8onCarouselMenuChoice_ *QBInnov8onCarouselMenuChoice;

/**
 * Creating QBInnov8onCarouselMenuChoice
 *
 * @param[in] appGlobals        application globals
 * @param[in] vodProvider       provider handle
 * @param[in] tree              tree handle
 * @param[in] serviceId         service id
 * @param[in] serviceName       service name
 * @param[in] serviceType       service type
 * @param[in] serviceLayout     service layout
 * @param[in] call2Action       is call2Action enabled
 * @param[in] serverInfo        call2Action server info
 * @return                      @c item choice controller or @c NULL in case of error
 **/
QBInnov8onCarouselMenuChoice
QBInnov8onCarouselMenuChoiceNew(AppGlobals appGlobals,
                                SvObject vodProvider,
                                SvObject tree,
                                SvString serviceId,
                                SvString serviceName,
                                SvString serviceType,
                                SvString serviceLayout,
                                bool call2Action,
                                SvObject serverInfo);

/**
 * Stop provider
 *
 * @param[in] self_    self_ handle
 **/
void QBInnov8onCarouselMenuChoiceStop(SvObject self_);

/**
 * @}
 **/

#endif /* INNVO8ONCAROUSELITEMCHOICE_H_ */
