/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef XHTML_UTILS_H
#define XHTML_UTILS_H_

#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>

/**
 * @file QBXHTMLUtils.h
 * @brief Utils for QBXHTML Widgets.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * Find value in SvHashTable based on key.
 * Returned value always contains string.
 *
 * @param[in] values    SvHashTable with values to search
 * @param[in] key       key to search
 * @return              founded value or @c NULL in other case
 **/
SvValue
QBXHTMLUtilsGetStringValue(SvHashTable values, SvString key);

/**
 * Find value in SvHashTable based on key.
 * Founded value must contain SvString.
 *
 * @param[in] values    SvHashTable with values to search
 * @param[in] key       key to search
 * @return              founded string or @c NULL in other case
 **/
SvString
QBXHTMLUtilsGetString(SvHashTable values, SvString key);
#endif
