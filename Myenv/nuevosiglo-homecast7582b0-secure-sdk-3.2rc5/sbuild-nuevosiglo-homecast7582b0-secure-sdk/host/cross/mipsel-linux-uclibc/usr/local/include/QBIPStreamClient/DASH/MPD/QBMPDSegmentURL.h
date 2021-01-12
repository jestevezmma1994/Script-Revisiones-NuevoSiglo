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

#ifndef QBMPDSEGMENTURL_H_
#define QBMPDSEGMENTURL_H_

/**
 * @file QBMPDSegmentURL.h
 * @brief Segment URL element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>

/**
 * @defgroup QBMPDSegmentURL Segment URL element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Segment URL element class.
 * @class QBMPDSegmentURL
 * @extends SvObject
 **/
typedef struct QBMPDSegmentURL_ *QBMPDSegmentURL;

/**
 * Get runtime type identification object representing
 * type of Segment URL element class.
 *
 * @return Segment URL element class
 **/
extern SvType
QBMPDSegmentURL_getType(void);

/**
 * Get URL to Media Segment of Segment URL element.
 *
 * @param[in] self Segment URL element handle
 * @return         URL to Media Segment (it shall be formatted as absolute URI or relative reference), @c NULL in case of error
 **/
extern SvURI
QBMPDSegmentURLGetMedia(QBMPDSegmentURL self);

/**
 * Get byte range corresponding to Media Segment of Segment URL element.
 *
 * @param[in] self Segment URL element handle
 * @return         byte range, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentURLGetMediaRange(QBMPDSegmentURL self);

/**
 * Get URL to Index Segment of Segment URL element.
 *
 * @param[in] self Segment URL element handle
 * @return         URL to Index Segment (it shall be formatted as absolute URI or relative reference), @c NULL in case of error
 **/
extern SvURI
QBMPDSegmentURLGetIndex(QBMPDSegmentURL self);

/**
 * Get byte range corresponding to Index Segment of Segment URL element.
 *
 * @param[in] self Segment URL element handle
 * @return         byte range, @c NULL in case of error
 **/
extern SvString
QBMPDSegmentURLGetIndexRange(QBMPDSegmentURL self);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTURL_H_ */
