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

#ifndef QBIPSTREAMCLIENT_DASH_QBDASHSTREAM_H_
#define QBIPSTREAMCLIENT_DASH_QBDASHSTREAM_H_

/**
 * @file QBDASHStream.h DASH Stream
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHSegment.h>
#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>

#include <stdbool.h>
#include <stddef.h>

/**
 * @defgroup QBDASHStream DASH stream abstraction
 * @ingroup QBIPStreamDASHSupport
 * @{
 * MPEG-DASH content stream abstraction
 **/

/**
 * QBDASHStream.
 * @class QBDASHStream
 * @extends SvObject
 */
typedef struct QBDASHStream_ *QBDASHStream;

/**
 * Set new DASH Period
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @param period
 */
void
QBDASHStreamSetPeriod(QBDASHStream self, QBDASHPeriod period);

/**
 * Set new DASH AdaptationSet
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @param[in] adaptationSet DASH AdaptationSet to be set
 */
void
QBDASHStreamSetAdaptationSet(QBDASHStream self, QBDASHAdaptationSet adaptationSet);

/**
 * Set new DASH Representation
 *
 * @param[in] self QBDASHStream object
 * @param[in] representation DASH Representation to be set
 */
void
QBDASHStreamSetRepresentation(QBDASHStream self, QBDASHRepresentation representation);

/**
 * Get current DASH AdaptationSet.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return         DASH AdaptationSet, otherwise @c NULL
 **/
QBDASHAdaptationSet
QBDASHStreamGetAdaptationSet(QBDASHStream self);

/**
 * Get current DASH Representation.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return         DASH Representation, otherwise @c NULL
 **/
QBDASHRepresentation
QBDASHStreamGetRepresentation(QBDASHStream self);

/**
 * Get DASH Initialization segment for current content
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return DASH Initialization Segment
 */
QBDASHSegment
QBDASHStreamGetInitSegment(QBDASHStream self);

/**
 * Get next segment to play and set it as current
 *
 * Function will release previously returned segment.
 * To use more than one segment SVRETAIN is needed.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return segment to play
 */
QBDASHSegment
QBDASHStreamGetNextSegment(QBDASHStream self);

/**
 * Get previous segment to play and set it as current
 *
 * Function will release previously returned segment.
 * To use more than one segment SVRETAIN is needed.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return segment to play
 */
QBDASHSegment
QBDASHStreamGetPreviousSegment(QBDASHStream self);

/**
 * Get next segment to play without advancing iterator position.
 *
 * Function will release previously returned segment.
 * To use more than one segment SVRETAIN is needed.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return         segment to play, or @c NULL in case of error
 **/
QBDASHSegment
QBDASHStreamPeekNextSegment(QBDASHStream self);

/**
 * Check if is possible to get next media segment for current stream.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return         @c true if next chunk is available, otherwise @c false
 **/
bool
QBDASHStreamHasNextSegment(QBDASHStream self);

/**
 * Get DASH Segment with given number
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHSegment object
 * @param[in] number number of Segment to get
 * @return DASH Segment with given index
 */
QBDASHSegment
QBDASHStreamGetSegmentByNumber(QBDASHStream self, size_t number);

/**
 * Get DASH Segment to be played at given time
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @param[in] time time when Segment
 * @return Segment to be played at given time
 */
QBDASHSegment
QBDASHStreamGetSegmentByTime(QBDASHStream self, SvTime time);

/**
 * Seek to DASH Segment of given number
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @param[in] number number of segment to be set as current
 */
void
QBDASHStreamSeekToSegmentNumber(QBDASHStream self, size_t number);

/**
 * Seek to DASH Segment corresponding to given time
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @param[in] time time of segment to be set as current
 */
void
QBDASHStreamSeekToSegmentTime(QBDASHStream self, SvTime time);

/**
 * Get duration of stream.
 *
 * @memberof QBDASHStream
 *
 * @param[in] self QBDASHStream object
 * @return         DASH stream duration in seconds
 **/
double
QBDASHStreamGetDuration(QBDASHStream self);

/**
 * Get runtime type identification object representing QBDASHStream class.
 *
 * @relates QBDASHStream
 *
 * @return QBDASHStream runtime type identification object
 **/
SvType
QBDASHStream_getType(void);

/**
 * Create new DASH Stream object.
 *
 * @memberof QBDASHStream
 *
 * @param[out] errorOut error information
 * @return created DASH Stream object, or @c NULL in case of error
 */
QBDASHStream
QBDASHStreamCreate(SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_DASH_QBDASHSTREAM_H_ */
