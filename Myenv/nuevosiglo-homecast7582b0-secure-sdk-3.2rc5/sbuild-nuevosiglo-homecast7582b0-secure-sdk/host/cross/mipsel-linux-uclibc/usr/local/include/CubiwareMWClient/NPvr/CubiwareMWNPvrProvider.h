/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBIWAREMWNPVRPROVIDER_H_
#define CUBIWAREMWNPVRPROVIDER_H_

/**
 * @file CubiwareMWNPvrProvider.h
 * @brief Cubiware MW NPvr provider class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CubiwareMWClient/Core/CubiwareMWCustomerInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <Services/core/QBMiddlewareManager.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup CubiwareMWNPvrProvider NPvr provider
 * @ingroup CubiwareMWClient
 **/

/**
 * CubiwareMWNPvrProvider type.
 * @class CubiwareMWNPvrProvider
 **/
typedef struct CubiwareMWNPvrProvider_s *CubiwareMWNPvrProvider;

/**
 * Get runtime type identification object
 * representing CubiwareMWNPvrProvider class.
 *
 * @return CubiwareMWNPvrProvider provider class
 **/
extern SvType
CubiwareMWNPvrProvider_getType(void);

/**
 * Initialize Cubiware MW NPVR provider instance.
 *
 * @memberof CubiwareMWNPvrProvider
 *
 * @param[in] self                 CubiwareMW NPVR provider handle
 * @param[in] customerInfoMonitor  CustomerInfoMonitor service
 * @param[in] customerId           customer id
 * @param[in] middlewareManager    middlewareManager handle
 * @param[out] errorOut            error info
 **/
extern void
CubiwareMWNPvrProviderInit(CubiwareMWNPvrProvider self,
                           CubiwareMWCustomerInfo customerInfoMonitor,
                           SvString customerId,
                           QBMiddlewareManager middlewareManager,
                           SvErrorInfo* errorOut);

/** @} */

#endif /* CUBIWAREMWNPVRPROVIDER_H_ */
