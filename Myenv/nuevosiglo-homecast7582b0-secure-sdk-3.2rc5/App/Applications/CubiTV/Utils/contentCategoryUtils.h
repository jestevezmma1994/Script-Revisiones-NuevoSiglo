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

#ifndef _CONTENTCATEGORY_UTILS_H_
#define _CONTENTCATEGORY_UTILS_H_

/**
 * @file contentCategoryUtils.h Utils methods related to QBContentCategory
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>
#include <QBContentManager/QBContentCategory.h>

/**
 * @defgroup contentCategoryUtils QBContentCategory utils functions
 * @ingroup CubiTV_utils
 * @{
 **/

/**
 * Returns index of given object in QBContentCategory. If there is no such object function returns -1 value.
 *
 * @param[in] category    category handle
 * @param[in] product     object handle
 * @return                @c larger or equal 0, if object was found, @c -1, otherwise
**/
int
QBContentCategoryUtilsGetProductIdxInCategory(QBContentCategory category, SvObject product);

/**
 * Finds category in the tree by path.
 *
 * @param[in] tree    tree handle
 * @param[in] path    category path
 * @return            @c QBContentCategory, if category was found with success, @c NULL, otherwise
**/
QBContentCategory
QBContentCategoryUtilsFindCategory(SvObject tree, SvObject path);

/**
 * @}
 **/

#endif /*_CONTENTCATEGORY_UTILS_H_*/
