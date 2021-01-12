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

#ifndef QBIPSTREAMCLIENT_QBDASHUTILS_H_
#define QBIPSTREAMCLIENT_QBDASHUTILS_H_

/**
 * @file QBDASHUtils.h MPEG-DASH Adaptation Set
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <stddef.h>

/**
 * @defgroup QBDASHUtils MPEG-DASH MPD Utility functions
 * @ingroup QBIPStreamDASHSupport
 * @{
 *
 **/

/**
 * Parse start of byte range and its length from string.
 * The string should contain byte range in '$first$-$last$' format,
 * where '$first' and '$last' are unsigned integers.
 *
 * @param[in] byteRange         string to be parsed
 * @param[out] byteRangeStart   parsed start of byte range
 * @param[out] byteRangeLength  parsed start of byte range
 * @return   @c 0 if string was parsed correctly, @c -1 in case of error
 **/
int
QBDASHUtilsGetStartByteAndLength(SvString byteRange, size_t *byteRangeStart, size_t *byteRangeLength);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBDASHUTILS_H_ */
