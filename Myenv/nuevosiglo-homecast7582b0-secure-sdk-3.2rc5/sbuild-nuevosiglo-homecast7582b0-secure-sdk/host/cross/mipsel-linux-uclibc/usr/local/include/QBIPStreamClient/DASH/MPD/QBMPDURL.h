/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBMPDURL_H_
#define QBMPDURL_H_

/**
 * @file QBMPDURL.h
 * @brief URL element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>

/**
 * @defgroup QBMPDURL URL element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * URL element class.
 * @class QBMPDURL
 * @extends SvObject
 **/
typedef struct QBMPDURL_ *QBMPDURL;

/**
 * Get runtime type identification object representing
 * type of URL element class.
 *
 * @return URL element class
 **/
extern SvType
QBMPDURL_getType(void);

/**
 * Get source URL of URL element.
 *
 * @param[in] self URL element handle
 * @return         source URL (it shall be formatted as absolute URI or relative reference), @c NULL in case of error
 **/
extern SvURI
QBMPDURLGetSourceURL(QBMPDURL self);

/**
 * Get bytes range of URL element.
 *
 * @param[in] self URL element handle
 * @return         bytes range, @c NULL in case of error
 **/
extern SvString
QBMPDURLGetRange(QBMPDURL self);

/**
 * @}
 **/

#endif /* QBMPDURL_H_ */
