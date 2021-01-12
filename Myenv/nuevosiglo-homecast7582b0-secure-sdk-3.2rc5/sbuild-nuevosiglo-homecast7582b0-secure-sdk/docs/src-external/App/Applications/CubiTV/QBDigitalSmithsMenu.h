/*****************************************************************************
 * ** Cubiware Sp. z o.o. Software License Version 1.0
 * **
 * ** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
 * **
 * ** Any rights which are not expressly granted in this License are entirely and
 * ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 * ** modify, translate, reverse engineer, decompile, disassemble, or create
 * ** derivative works based on this Software. You may not make access to this
 * ** Software available to others in connection with a service bureau,
 * ** application service provider, or similar business, or make any other use of
 * ** this Software without express written permission from Cubiware Sp. z o.o.
 * **
 * ** Any User wishing to make use of this Software must contact
 * ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 * ** includes, but is not limited to:
 * ** (1) integrating or incorporating all or part of the code into a product for
 * **     sale or license by, or on behalf of, User to third parties;
 * ** (2) distribution of the binary or source code to third parties for use with
 * **     a commercial product sold or licensed by, or on behalf of, User.
 * ******************************************************************************/

#ifndef QB_DIGITAL_SMITHS_MENU_H_
#define QB_DIGITAL_SMITHS_MENU_H_

/**
 * @file QBDigitalSmithsMenu.h Digitalsmith submenu factory class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "Menus/QBMWClientMenuParams.h"
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvObject.h>

/**
 * @defgroup QBDigitalSmithsMenu QBDigitalSmithsMenu: Digitalsmith menu factory
 * @ingroup CubiTV_menus
 * @{
 *
 * CubiTV Digitalsmith menu factory
 **/

/**
 * Create new QBDigitalSmithsMenuFactory object.
 * QBDigitalSmithsMenuFactory construct
 *
 * @public
 * @implements QBSubmenuFactory
 * @extends SvObject
 *
 * @param[in]  rootCategoryName name of root category to use as root for the content tree
 * @param[in]  mwVoDParams      params from MW VoD service (used for name and layout). If @c NULL the service will be independent from MW settings
 * @param[out] errorOut         error info
 * @return     New QBDigitalSmithsMenuFactory instance or @c NULL in case of error
 */
SvObject
QBDigitalSmithsMenuFactoryCreate(SvString rootCategoryName, QBMWClientMenuParams mwVoDParams, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QB_DIGITAL_SMITHS_MENU_H_
