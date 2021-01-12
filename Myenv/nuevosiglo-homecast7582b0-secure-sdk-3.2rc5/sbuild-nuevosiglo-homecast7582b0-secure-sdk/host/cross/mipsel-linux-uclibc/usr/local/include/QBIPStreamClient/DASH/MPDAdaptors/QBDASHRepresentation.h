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

#ifndef QBIPSTREAMCLIENT_QBDASHREPRESENTATION_H_
#define QBIPSTREAMCLIENT_QBDASHREPRESENTATION_H_

/**
 * @file QBDASHRepresentation.h MPEG-DASH Representation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHSegment.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvURL.h>
#include <SvCore/SvErrorInfo.h>

#include <stddef.h>

/**
 * @defgroup QBDASHRepresentation MPEG-DASH MPD Representation
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level representation of MPEG-DASH MPD Representation element
 **/

/**
 * QBDASHRepresentation.
 * @class QBDASHRepresentation
 * @extends SvObject
 */
typedef struct QBDASHRepresentation_ *QBDASHRepresentation;

/**
 * DASH Representation type.
 * Type represents type of Segment information inside particular Representation.
 * Section 5.3.9 ISO/IEC 23009-1:2014
 */
typedef enum QBDASHRepresentationType {
    QBDASHRepresentationType_unknown,            //!< unknown type
    QBDASHRepresentationType_singleMediaSegment, //!< SegmentBase (singleMediaSegment)
    QBDASHRepresentationType_segmentList,        //!< SegmentList
    QBDASHRepresentationType_segmentTemplate,    //!< SegmentTemplate
} QBDASHRepresentationType;


/**
 * Get Content Protection information.
 *
 * Function returns array of QBMPDDescriptors each representing one
 * ContentProtection tag. Nodes inside ContentProtection are represented by
 * QBMPDAbstractElement objects.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return protection information, or @c NULL if not present
 */
SvArray
QBDASHRepresentationGetContentProtections(QBDASHRepresentation self);

/**
 * Get codec string.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return string describing codec used to encode/decode representation
 */
SvString
QBDASHRepresentationGetCodecs(QBDASHRepresentation self);

/**
 * Get video representation width.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return representation width
 */
int
QBDASHRepresentationGetWidth(QBDASHRepresentation self);

/**
 * Get video representation height.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return representation height
 */
int
QBDASHRepresentationGetHeight(QBDASHRepresentation self);

/**
 * Get sample aspect ratio of the video media component type.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @param[out] sarx x sar parameter
 * @param[out] sary y sar parameter
 */
void
QBDASHRepresentationGetSar(QBDASHRepresentation self, int *sarx, int *sary);

/**
 * Get maximal and minimal audio sampling rate.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @param[out] min_freq minimum sampling rate or -1 if unknown
 * @param[out] max_freq maximum sampling rate or -1 if unknown
 */
void
QBDASHRepresentationGetAudioSamplingRate(QBDASHRepresentation self, int *min_freq, int *max_freq);

/**
 * Get possible audio channel configurations.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return array of MPD Descriptors with possible audio channel configuration
 */
SvArray
QBDASHRepresentationGetAudioChannelConfigurations(QBDASHRepresentation self);

/**
 * Get representation frame rate
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @param[out] f numerator value of frame rate or 0 if frame rate unknown
 * @param[out] d denominator value of frame rate or 0 if frame rate unknown
 */
void
QBDASHRepresentationGetFrameRate(QBDASHRepresentation self, unsigned int *f, unsigned int *d);

/**
 * Get Initialization segment.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return initialization segment
 */
QBDASHSegment
QBDASHRepresentationGetInitializationSegment(QBDASHRepresentation self);

/**
 * Get Media segment.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @param[in] idx index of media segment
 * @return media segment
 */
QBDASHSegment
QBDASHRepresentationGetMediaSegment(QBDASHRepresentation self, size_t idx);

/**
 * Get Index segment.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @param[in] number number of index segment to get
 * @return index segment
 */
QBDASHSegment
QBDASHRepresentationGetIndexSegment(QBDASHRepresentation self, size_t number);

/**
 * Get Bitstream Switching segment.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return bitstrem switching segment
 */
QBDASHSegment
QBDASHRepresentationGetBitstreamSwitchingSegment(QBDASHRepresentation self);

/**
 * Get type of DASH Representation.
 * It is type of Segment information in DASH Representation.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return DASH Representation type
 */
QBDASHRepresentationType
QBDASHRepresentationGetType(QBDASHRepresentation self);

/**
 * Get number of segments inside DASH Representation.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return number of segments
 */
size_t
QBDASHRepresentationGetSegmentsCount(QBDASHRepresentation self);

/**
 * Get number of first segment.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return number of first segment
 */
size_t
QBDASHRepresentationGetFirstSegmentNumber(QBDASHRepresentation self);

/**
 * Get number of current segment.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return current segment number
 */
size_t
QBDASHRepresentationGetCurrentSegmentNumber(QBDASHRepresentation self);

/**
 * Get number of last segment inside DASH Representation
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return current segment number
 */
size_t
QBDASHRepresentationGetLastSegmentNumber(QBDASHRepresentation self);

/**
 * Get total duration of segments.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return         total duration of segments in timescale units
 **/
uint64_t
QBDASHRepresentationGetTotalSegmentsDuration(QBDASHRepresentation self);

/**
 * Get average segment duration
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return average segment duration in @c timescale units
 */
uint64_t
QBDASHRepresentationGetAverageSegmentDuration(QBDASHRepresentation self);

/**
 * Get first segment number
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @return number of first segment in representation
 */
size_t
QBDASHRepresentationGetSegmentStartNumber(QBDASHRepresentation self);

/**
 * Get time of Segment with given index
 *
 * Function returns start time of Segment with given index.
 * Index is an index of Segment in current Representation.
 *
 * Time will also be computed for indexes above last segment index.
 * It is useful to obtain end time of the last segment.
 *
 * For live stream time is real time of content and for non-live streams it is
 * time from the beginning of recording. Time is in 1/@c timescale unit.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self DASH Representation object
 * @param[in] idx index of Segment
 * @return start time of Segment with given index
 */
uint64_t
QBDASHRepresentationGetSegmentTimeByIndex(QBDASHRepresentation self, size_t idx);

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
size_t
QBDASHRepresentationGetSegmentIndexByTime(QBDASHRepresentation self, uint64_t time);

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
uint64_t
QBDASHRepresentationGetTimescale(QBDASHRepresentation self);

/**
 * Get runtime type identification object representing QBDASHRepresentation class.
 *
 * @relates QBDASHRepresentation
 *
 * @return QBDASHRepresentation runtime type identification object
 **/
SvType
QBDASHRepresentation_getType(void);

/**
 * Initialize QBDASHRepresentation object.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] self uninitialized DASH Representation object
 * @param[in] mpdElement MPEG-DASH MPD Element
 * @param[in] mpdPeriod MPEG-DASH MPD Period
 * @param[in] mpdAdaptationSet MPEG-DASH MPD Adaptation Set
 * @param[in] mpdRepresentation MPEG-DASH MPD Representation
 * @param[in] manifestBaseURL MPEG-DASH manifest base URL
 * @param[out] errorOut error information
 */
void
QBDASHRepresentationInit(QBDASHRepresentation self, QBMPDElement mpdElement,
                         QBMPDPeriod mpdPeriod, QBMPDAdaptationSet mpdAdaptationSet,
                         QBMPDRepresentation mpdRepresentation, SvURL manifestBaseURL, SvErrorInfo *errorOut);

/**
 * Create new QBDASHRepresentation object.
 *
 * @memberof QBDASHRepresentation
 *
 * @param[in] mpdElement MPEG-DASH MPD Element
 * @param[in] mpdPeriod MPEG-DASH MPD Period
 * @param[in] mpdAdaptationSet MPEG-DASH MPD Adaptation Set
 * @param[in] mpdRepresentation MPEG-DASH MPD Representation
 * @param[in] manifestBaseURL MPEG-DASH manifest base URL
 * @param[out] errorOut error information
 * @return created DASH Representation object, or @c NULL in case of error
 */
QBDASHRepresentation
QBDASHRepresentationCreate(QBMPDElement mpdElement, QBMPDPeriod mpdPeriod,
                           QBMPDAdaptationSet mpdAdaptationSet,
                           QBMPDRepresentation mpdRepresentation, SvURL manifestBaseURL, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBDASHREPRESENTATION_H_ */
