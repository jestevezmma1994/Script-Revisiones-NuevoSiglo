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

#ifndef QBMPDBASEURL_H_
#define QBMPDBASEURL_H_

/**
 * @file QBMPDBaseURL.h
 * @brief Base URL API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <stdbool.h>

/**
 * @defgroup QBMPDBaseURL Base URL element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Base URL element class.
 * @class QBMPDBaseURL
 * @extends SvObject
 **/
typedef struct QBMPDBaseURL_ *QBMPDBaseURL;

/**
 * Get runtime type identification object representing
 * type of Base URL element class.
 *
 * @return Base URL element class
 **/
extern SvType
QBMPDBaseURL_getType(void);

/**
 * Get URI from Base URL element.
 *
 * @param[in] self Base URL element handle
 * @return         URI, @c NULL in case of error
 **/
extern SvURI
QBMPDBaseURLGetURI(QBMPDBaseURL self);

/**
 * Get service location of Base URL element.
 *
 * @param[in] self Base URL element handle
 * @return         service location, @c NULL in case of error
 **/
extern SvString
QBMPDBaseURLGetServiceLocation(QBMPDBaseURL self);

/**
 * Get bytes range of Base URL element.
 *
 * @param[in] self Base URL element handle
 * @return         bytes range, @c NULL in case of error
 **/
extern SvString
QBMPDBaseURLGetBytesRange(QBMPDBaseURL self);

/**
 * Get availability time offset of Base URL element.
 *
 * @param[in]  self     Base URL element handle
 * @return              availability time offset
 **/
extern double
QBMPDBaseURLGetAvailabilityTimeOffset(QBMPDBaseURL self);

/**
 * Get availability time complete of Base URL element.
 *
 * @param[in]  self     Base URL element handle
 * @return              availability time complete
 **/
extern bool
QBMPDBaseURLGetAvailabilityTimeComplete(QBMPDBaseURL self);

/**
 * @}
 **/

#endif /* QBMPDBASEURL_H_ */
