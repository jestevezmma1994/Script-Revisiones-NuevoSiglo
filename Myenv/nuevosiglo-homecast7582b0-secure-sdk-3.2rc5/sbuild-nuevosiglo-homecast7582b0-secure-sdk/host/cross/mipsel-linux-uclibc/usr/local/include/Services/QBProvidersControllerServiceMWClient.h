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

#ifndef QB_PROVIDERS_CONTROLLER_SERVICE_MWCLIENT_H_
#define QB_PROVIDERS_CONTROLLER_SERVICE_MWCLIENT_H_

/**
 * @file QBProvidersControllerServiceMWClient.h MWClient Providers Controller Service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBProvidersControllerServiceMWClient MWClient Providers Controller Service
 * @ingroup QBProvidersControllerService
 * @{
 **/

#include <SvFoundation/SvObject.h>
#include <main_decl.h>

/**
 * Create instance of MWClient services controller.
 *
 * @param appGlobals  AppGlobals handle
 * @return            MWClient service controller handle, or NULL on error.
 */
SvObject
QBProvidersControllerServiceMWClientCreate(AppGlobals appGlobals);

/**
 * @}
 **/

#endif /* QB_PROVIDERS_CONTROLLER_SERVICE_MWCLIENT_H_ */
