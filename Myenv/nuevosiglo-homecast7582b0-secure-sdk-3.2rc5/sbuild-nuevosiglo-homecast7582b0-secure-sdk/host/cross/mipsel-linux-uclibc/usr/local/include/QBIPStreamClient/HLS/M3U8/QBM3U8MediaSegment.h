/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBM3U8MEDIASEGMENT_H_
#define QBM3U8MEDIASEGMENT_H_

/**
 * @file QBM3U8MediaSegment.h
 * @brief HLS Media segment API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURL.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup QBM3U8MediaSegment HLS Media Segment class
 * @ingroup QBM3U8Parser
 * @{
 **/

/**
 * Segment class.
 * @class QBM3U8MediaSegment.
 **/
typedef struct QBM3U8MediaSegment_ *QBM3U8MediaSegment;

/**
 * Get runtime type identification object representing
 * type of Media Segment class.
 *
 * @return Media Segment class
 **/
extern SvType
QBM3U8MediaSegment_getType(void);

/**
 * Get Media Segment URI.
 *
 * @param[in] self - segment to get from
 * @return         - URI of specified segment
 */
extern SvURL
QBM3U8MediaSegmentGetURL(QBM3U8MediaSegment self);

/**
 * Get duration of Media Segment.
 *
 * @param[in] self - segment to get from
 * @return         - duration of specified segment
 */
extern double
QBM3U8MediaSegmentGetDuration(QBM3U8MediaSegment self);

/**
 * Get byte range length of Media Segment.
 *
 * @param[in] self - segment to get from
 * @return         - byte range length
 */
extern uint64_t
QBM3U8MediaSegmentGetByteRangeLength(QBM3U8MediaSegment self);

/**
 * Get byte range offset of Media Segment.
 *
 * @param[in] self - segment to get from
 * @return         bSpec- byte offset
 */
extern uint64_t
QBM3U8MediaSegmentGetByteRangeOffset(QBM3U8MediaSegment self);

/**
 * Get discontinuity of Media Segment.
 *
 * @param[in] self - segment to get from
 * @return         - discontinuity value
 */
extern bool
QBM3U8MediaSegmentGetDiscontinuity(QBM3U8MediaSegment self);

/**
 * Get date and time of the first sample of Media Segment.
 *
 * @param[in] self - segment to get from
 * @return         - date and time of the first sample
 */
extern SvTime
QBM3U8MediaSegmentGetDateTime(QBM3U8MediaSegment self);

/**
 * @}
 **/

#endif // QBM3U8MEDIASEGMENT_H_
