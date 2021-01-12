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

#ifndef QBIPSTREAMCLIENT_DASH_QBDASHSEGMENTINTERNAL_H_
#define QBIPSTREAMCLIENT_DASH_QBDASHSEGMENTINTERNAL_H_

/**
 * @file QBDASHSegmentInternal.h MPEG-DASH Segment internals
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>

#include <stddef.h>

/**
 * @defgroup QBDASHSegmentInternal DASH Segment internals
 * @{
 * High-level MPEG-DASH Segment internals
 **/

/**
 * QBDASHSegment.
 * @class QBDASHSegment
 * @extends SvObject
 */
struct QBDASHSegment_ {
    /** super object */
    struct SvObject_ super_;

    /** URL to content represented by object */
    SvURL resolvedURL;

    /** base part of URL to content represented by object */
    SvURL baseURL;

    /** last part of URL to content represented by object */
    SvString url;

    /** first byte offset in response to content represented by object */
    size_t byteRangeStart;

    /** content data block length in response */
    size_t byteRangeLength;

    /** segment number with included first segment offset */
    size_t segmentNumber;

    /** segment time in timescale units */
    uint64_t segmentTime;

    /** segment duration in timescale units */
    uint64_t segmentDuration;
};

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_DASH_QBDASHSEGMENTINTERNAL_H_ */
