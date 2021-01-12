/*****************************************************************************
** Cubiware K.K. Software License Version 1.1
**
** Copyright (C) 2002-2007 Cubiware K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware K.K.
**
** Any User wishing to make use of this Software must contact Cubiware K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_CUSTOMER_LOGO_H_
#define QB_CUSTOMER_LOGO_H_

/**
 * @file customerLogo.h Widget showing customer logo that changes upon request
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CUIT/Core/types.h>
#include <QBApplicationController.h>

/**
 * @defgroup CustomerLogo Customer Logo
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Create logo widget and attach it to the window.
 *
 * @param[in] window        window handle
 * @param[in] widgetName    name of the widget in currently pushed settings describing the logo
 * @param[in] level         z-order at which to attach the widget
 * @return                  logo widget handle
 */
SvWidget QBCustomerLogoAttach(SvWidget window, SvString widgetName, int level);

/**
 * Replace the logo widget if needed.
 *
 * @param[in] logo          logo widget handle
 * @param[in] window        window handle
 * @param[in] widgetName    name of the widget in currently pushed settings describing the logo
 * @param[in] level         z-order at which to attach the widget
 * @return                  new logo widget handle or @a logo if not changed
 **/
SvWidget QBCustomerLogoReplace(SvWidget logo, SvWidget window, SvString widgetName, int level);

/**
 * @}
 **/


#endif
