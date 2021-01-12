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

#ifndef QBIPSTREAMCLIENT_QBDASHREPRESENTATIONVTABLE_H_
#define QBIPSTREAMCLIENT_QBDASHREPRESENTATIONVTABLE_H_

/**
 * @file QBDASHRepresentationVTable.h MPEG-DASH Representation - vtable
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHRepresentation.h>
#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHSegment.h>
#include <SvFoundation/SvCoreTypes.h>

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup QBDASHRepresentationVTable vtable for QBDASHRepresentation class
 * @ingroup QBIPStreamDASHSupport
 * @{
 * Virtual methods table for High-level representation of MPEG-DASH MPD Representation element
 **/

/**
 * Virtual methods table for DASH Representation class.
 **/
typedef const struct QBDASHRepresentationVTable_ {
    /** virtual methods of the base class */
    struct SvObjectVTable_ super_;

    /**
     * Get Initialization segment.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return initialization segment
     */
    QBDASHSegment (*getInitializationSegment)(QBDASHRepresentation self);

    /**
     * Get Media segment.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @param[in] idx index of media segment
     * @return media segment
     */
    QBDASHSegment (*getMediaSegment)(QBDASHRepresentation self, size_t idx);

    /**
     * Get Index segment.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @param[in] number number of index segment to get
     * @return index segment
     */
    QBDASHSegment (*getIndexSegment)(QBDASHRepresentation self, size_t number);

    /**
     * Get Bitstream Switching segment.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return bitstream switching segment
     */
    QBDASHSegment (*getBitstreamSwitchingSegment)(QBDASHRepresentation self);

    /**
     * Get type of DASH Representation.
     * It is type of Segment information in DASH Representation.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return DASH Representation type
     */
    QBDASHRepresentationType (*getType)(QBDASHRepresentation self);

    /**
     * Get number of segments inside DASH Representation.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return number of segments
     */
    size_t (*getSegmentsCount)(QBDASHRepresentation self);

    /**
     * Get total duration of segments inside DASH Representation.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return         total duration of segments in timescale units
     **/
    uint64_t (*getTotalSegmentsDuration)(QBDASHRepresentation self);

    /**
     * Get average segment duration
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return average segment duration in @c timescale units
     */
    uint64_t (*getAverageSegmentDuration)(QBDASHRepresentation self);

    /**
     * Get first segment number
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return number of first segment in representation
     */
    size_t (*getSegmentStartNumber)(QBDASHRepresentation self);

    /**
     * Get time of Segment with given index
     *
     * Function returns start time of Segment with given index.
     * Index is an index of Segment in current Representation.
     *
     * For live stream time is real time of content and for non-live streams it is
     * time from the beginning of recording. Time is in 1/@c timescale unit.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @param[in] idx index of Segment in Representation
     * @return start time of Segment with given index
     */
    uint64_t (*getSegmentTimeByIndex)(QBDASHRepresentation self, size_t idx);

    /**
     * Get index of Segment containing given time
     *
     * Function returns index of Segment containing given time of content.
     *
     * For live stream time is real time of content and for non-live streams it is
     * time from the beginning of recording. Time is in 1/@c timescale unit.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @param[in] time DASH time (in @c timescale scale) of Segment
     * @return index of Segment with given time
     */
    size_t (*getSegmentIndexByTime)(QBDASHRepresentation self, uint64_t time);

    /**
     * Get Representation time scale.
     *
     * Time scale is in unit 1/s.
     *
     * @memberof QBDASHRepresentation
     *
     * @param[in] self DASH Representation object
     * @return representation time scale
     */
    uint64_t (*getTimescale)(QBDASHRepresentation self);
} *QBDASHRepresentationVTable;

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBDASHREPRESENTATIONVTABLE_H_ */
