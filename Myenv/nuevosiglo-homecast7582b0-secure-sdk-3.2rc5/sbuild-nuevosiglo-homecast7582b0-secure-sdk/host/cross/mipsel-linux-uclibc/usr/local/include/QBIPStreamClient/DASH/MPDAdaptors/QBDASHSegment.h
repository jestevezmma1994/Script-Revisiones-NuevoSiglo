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

#ifndef QBIPSTREAMCLIENT_DASH_QBDASHSEGMENT_H_
#define QBIPSTREAMCLIENT_DASH_QBDASHSEGMENT_H_

/**
 * @file QBDASHSegment.h MPEG-DASH Segment
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvURL.h>

#include <stddef.h>

/**
 * @defgroup QBDASHSegment MPEG-DASH MPD Segment element
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level representation of MPEG-DASH MPD Segment element
 **/

/**
 * QBDASHSegment.
 * @class QBDASHSegment
 * @extends SvObject
 */
typedef struct QBDASHSegment_ *QBDASHSegment;

/**
 * QBDASHSegmentr constructor parameters
 * @struct QBDASHSegmentParams
 */
typedef struct QBDASHSegmentParams {
    SvURL baseURL;              ///< base part of Segment URL
    SvString url;               ///< last part of Segment URL
    size_t byteRangeStart;      ///< offset to first byte of content in response stream
    size_t byteRangeLength;     ///< size of data block with content in response stream
    size_t segmentNumber;       ///< segment number
    uint64_t segmentTime;       ///< segment time
    uint64_t segmentDuration;   ///< segment duration
} QBDASHSegmentParams;

/**
 * Get Segment URL.
 *
 * @memberof QBDASHSegment
 *
 * @param[in] self DASH Segment object
 * @return URL of segment
 */
SvURL
QBDASHSegmentGetURL(QBDASHSegment self);

/**
 * Get byte offset of Segment URL range.
 *
 * Function returns offset to first byte (in URL response) containing media represented by this Segment object.
 *
 * @memberof QBDASHSegment
 *
 * @param[in] self DASH Segment object
 * @return byte offset of Segment content
 */
size_t
QBDASHSegmentGetByteRangeStart(QBDASHSegment self);

/**
 * Get Segment URL range length.
 *
 * Function returns size of data block (in URL response) containing media represented by this Segment object.
 *
 * @memberof QBDASHSegment
 *
 * @param[in] self DASH Segment object
 * @return length of Segment URL range
 */
size_t
QBDASHSegmentGetByteRangeLength(QBDASHSegment self);

/**
 * Get Segment number
 *
 * @memberof QBDASHSegment
 *
 * @param[in] self DASH Segment object
 * @return         segment number
 */
size_t
QBDASHSegmentGetNumber(QBDASHSegment self);

/**
 * Get Segment time
 *
 * @memberof QBDASHSegment
 *
 * @param[in] self DASH Segment object
 * @return         segment time in timescale units
 */
uint64_t
QBDASHSegmentGetTime(QBDASHSegment self);

/**
 * Get Segment duration
 *
 * @memberof QBDASHSegment
 *
 * @param[in] self DASH Segment object
 * @return         segment duration in timescale units
 */
uint64_t
QBDASHSegmentGetDuration(QBDASHSegment self);

/**
 * Get runtime type identification object representing QBDASHSegment class.
 *
 * @relates QBDASHSegment
 *
 * @return QBDASHSegment runtime type identification object
 **/
SvType
QBDASHSegment_getType(void);

/**
 * Create new DASH Segment object.
 *
 * @param[in]  params   DASH segment constructor params
 * @param[out] errorOut error information
 * @return created DASH Segment object, or @c NULL in case of error
 */
QBDASHSegment
QBDASHSegmentCreate(QBDASHSegmentParams *params,
                    SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_DASH_QBDASHSEGMENT_H_ */
