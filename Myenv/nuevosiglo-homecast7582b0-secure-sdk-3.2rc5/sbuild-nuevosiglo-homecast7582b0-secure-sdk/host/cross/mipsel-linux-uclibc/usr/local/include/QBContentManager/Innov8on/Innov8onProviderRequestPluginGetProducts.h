/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/QBVoDManager/Headers/QBContentManager/Innov8on/Innov8onProviderRequestPluginGetProducts.h */

#ifndef INNOV8ONPROVIDERREQUESTPLUGINGETPRODUCTS_H_
#define INNOV8ONPROVIDERREQUESTPLUGINGETPRODUCTS_H_

/**
 * @file Innov8onProviderRequestPluginGetProducts.h
 * @brief Innov8on Provider Request Plugin GetProducts class.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup Innov8onProviderRequestPlugin innov8on provider request plugin Get Products
 * @ingroup Innov8on
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * Create Innov8on provider request plugin.
 * This plugin sends "Get Products" request.
 *
 * @return Newly created plugin.
 **/
SvObject Innov8onProviderRequestPluginGetProductsCreate(void);

/**
 * @}
 **/

#endif
