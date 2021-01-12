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

#ifndef QBDASHSEGMENTTIMELINE_H_
#define QBDASHSEGMENTTIMELINE_H_

/**
 * @file QBDASHSegmentTimeline.h
 * @brief MPEG-DASH Segment Timeline element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTimeline.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBDASHSegmentTimeline MPEG-DASH Segment Timeline element
 * @{
 * High-level representation of MPEG-DASH Segment Timeline element
 **/

/**
 * MPEG-DASH Segment Timeline element class
 * @class QBDASHSegmentTimeline
 * @extends SvObject
 **/
typedef struct QBDASHSegmentTimeline_ *QBDASHSegmentTimeline;

/**
 * Get runtime type identification object representing
 * type of Segment Timeline element class.
 *
 * @return Segment Timeline element class
 **/
SvType
QBDASHSegmentTimeline_getType(void);

/**
 * Create MPEG-DASH Segment Timeline element.
 *
 * @param[in]  timeline               MPD Segment Timeline
 * @param[in]  timescale              timescale in units per seconds
 * @param[in]  presentationTimeOffset presentation time offset of representation in @c timescale units
 * @param[in]  periodDuration         duration of period in @c timescale units
 * @param[out] errorOut               error info
 * @return                            new instance of MPEG-DASH Segment Timeline element, @c NULL in case of error
 **/
QBDASHSegmentTimeline
QBDASHSegmentTimelineCreate(QBMPDSegmentTimeline timeline,
                            uint64_t timescale,
                            uint64_t presentationTimeOffset,
                            uint64_t periodDuration,
                            SvErrorInfo *errorOut);

/**
 * Get average segment duration.
 *
 * @param[in] self MPEG-DASH Segment Timeline element handle
 * @return         average segment duration in @c timescale units
 **/
uint64_t
QBDASHSegmentTimelineGetAverageSegmentDuration(QBDASHSegmentTimeline self);

/**
 * Get parameters (index, duration) of segment containing given time.
 *
 * @param[in]  self        MPEG-DASH Segment Timeline element handle
 * @param[in]  time        time of segment in @c timescale units
 * @param[out] outIndex    index of segment containing given time
 * @param[out] outDuration duration of segment containing given time in @c timescale units
 **/
void
QBDASHSegmentTimelineGetSegmentParametersByTime(QBDASHSegmentTimeline self,
                                                uint64_t time,
                                                size_t *outIndex,
                                                uint64_t *outDuration);

/**
 * Get parameters (time, duration) of segment with given index.
 *
 * @param[in]  self        MPEG-DASH Segment Timeline element handle
 * @param[in]  index       index of segment
 * @param[out] outTime     start time of segment with given index in @c timescale units
 * @param[out] outDuration duration of segment with given index in @c timescale units
 **/
void
QBDASHSegmentTimelineGetSegmentParametersByIndex(QBDASHSegmentTimeline self,
                                                 size_t index,
                                                 uint64_t *outTime,
                                                 uint64_t *outDuration);

/**
 * Get number of segments.
 *
 * @param[in] self MPEG-DASH Segment Timeline element handle
 * @return         number of segments
 **/
size_t
QBDASHSegmentTimelineGetSegmentsCount(QBDASHSegmentTimeline self);

/**
 * Get total duration of segments.
 *
 * @param[in] self MPEG-DASH Segment Timeline element handle
 * @return         total duration of segments in @c timescale units
 **/
uint64_t
QBDASHSegmentTimelineGetTotalSegmentsDuration(QBDASHSegmentTimeline self);
/**
 * @}
 **/

#endif /* QBDASHSEGMENTTIMELINE_H_ */
