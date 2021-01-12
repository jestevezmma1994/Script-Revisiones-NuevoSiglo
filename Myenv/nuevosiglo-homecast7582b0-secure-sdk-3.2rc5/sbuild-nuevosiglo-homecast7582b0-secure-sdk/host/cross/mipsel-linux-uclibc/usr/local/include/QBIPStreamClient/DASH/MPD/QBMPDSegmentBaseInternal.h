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

#ifndef QBMPDSEGMENTBASEINTERNAL_H_
#define QBMPDSEGMENTBASEINTERNAL_H_

/**
 * @file QBMPDSegmentBaseInternal.h
 * @brief Segment Base element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <stdbool.h>

/**
 * @addtogroup QBMPDSegmentBase
 * @{
 **/

/**
 * Segment Base element class internals
 **/
struct QBMPDSegmentBase_ {
    struct SvObject_ super_;         ///< base class

    /**
     * Segment Base element attributes
     **/
    uint64_t timescale;              ///< timescale in units per seconds
    uint64_t presentationTimeOffset; ///< presentation time offset of the Representation relative to the start of the Period
    SvTime timeShiftBufferDepth;     ///< duration of the time shifting buffer for Representation that is guaranteed to be available for a Media Presentation
    SvString indexRange;             ///< byte range that contains the Segment Index in all Media Segments of the Representation
    bool indexRangeExact;            ///< specifies if data outside the prefix defined by @c indexRange contains the data needed to access all access units of all media streams syntactically
    double availabilityTimeOffset;   ///< offset to define the adjusted segment availability time
    bool availabilityTimeComplete;   ///< specifies if all Segments of all associated Representation are complete at the adjusted availability start time

    /**
     * Segment Base child elements
     **/
    QBMPDURL initialization;         ///< URL including a possible byte range for the Initialization Segment
    QBMPDURL representationIndex;    ///< URL including a possible byte range for the Representation Index Segment
};

/**
 * Initialize Segment Base element.
 *
 * @param[in]  self     Segment Base element handle
 * @param[out] errorOut error info
 **/
extern void
QBMPDSegmentBaseInitialize(QBMPDSegmentBase self,
                           SvErrorInfo *errorOut);

/**
 * Create Segment Base element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Segment Base element, @c NULL in case of error
 **/
extern QBMPDSegmentBase
QBMPDSegmentBaseCreate(SvErrorInfo *errorOut);

/**
 * Set timescale for Segment Base element.
 *
 * @param[in]  self      Segment Base element handle
 * @param[in]  timescale timescale in units per seconds
 * @param[out] errorOut  error info
 **/
extern void
QBMPDSegmentBaseSetTimescale(QBMPDSegmentBase self,
                             uint64_t timescale,
                             SvErrorInfo *errorOut);

/**
 * Set presentation time offset for Segment Base element.
 *
 * @param[in]  self                   Segment Base element handle
 * @param[in]  presentationTimeOffset presentation time offset
 * @param[out] errorOut               error info
 *
 **/
extern void
QBMPDSegmentBaseSetPresentationTimeOffset(QBMPDSegmentBase self,
                                          uint64_t presentationTimeOffset,
                                          SvErrorInfo *errorOut);

/**
 * Set time shifting buffer for Segment Base element.
 *
 * @param[in]  self                 Segment Base element handle
 * @param[in]  timeShiftBufferDepth time shifting buffer
 * @param[out] errorOut             error info
 **/
extern void
QBMPDSegmentBaseSetTimeShiftBufferDepth(QBMPDSegmentBase self,
                                        SvTime timeShiftBufferDepth,
                                        SvErrorInfo *errorOut);

/**
 * Set bytes range that contains Segment Index for Segment Base element.
 *
 * @param[in]  self       Segment Base element handle
 * @param[in]  indexRange byte range
 * @param[out] errorOut   error info
 **/
extern void
QBMPDSegmentBaseSetIndexRange(QBMPDSegmentBase self,
                              SvString indexRange,
                              SvErrorInfo *errorOut);

/**
 * Set index range excat for Segment Base element.
 *
 * @param[in]  self            Segment Base element handle
 * @param[in]  indexRangeExact index range exact
 * @param[out] errorOut        error info
 **/
extern void
QBMPDSegmentBaseSetIndexRangeExact(QBMPDSegmentBase self,
                                   bool indexRangeExact,
                                   SvErrorInfo *errorOut);

/**
 * Set availability time offset for Segment Base element.
 *
 * @param[in]  self                   Segment Base element handle
 * @param[in]  availabilityTimeOffset availability time offset in seconds
 * @param[out] errorOut               error info
 **/
extern void
QBMPDSegmentBaseSetAvailabilityTimeOffset(QBMPDSegmentBase self,
                                          double availabilityTimeOffset,
                                          SvErrorInfo *errorOut);

/**
 * Set availability time complete for Segment Base element.
 *
 * @param[in]  self                     Segment Base element handle
 * @param[in]  availabilityTimeComplete availability time complete
 * @param[out] errorOut                 error info
 **/
extern void
QBMPDSegmentBaseSetAvailabilityTimeComplete(QBMPDSegmentBase self,
                                            bool availabilityTimeComplete,
                                            SvErrorInfo *errorOut);

/**
 * Set initialization for Segment Base element.
 *
 * @param[in]  self           Segment Base element handle
 * @param[in]  initialization URL including possible range for Initialization Segment
 * @param[out] errorOut       error info
 **/
extern void
QBMPDSegmentBaseSetInitialization(QBMPDSegmentBase self,
                                  QBMPDURL initialization,
                                  SvErrorInfo *errorOut);

/**
 * Set representation index for Segment Base element.
 *
 * @param[in]  self                Segment Base element handle
 * @param[in]  representationIndex URL including possible byte range for Representation Index Segment
 * @param[out] errorOut            error info
 **/
extern void
QBMPDSegmentBaseSetRepresentationIndex(QBMPDSegmentBase self,
                                       QBMPDURL representationIndex,
                                       SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDSEGMENTBASEINTERNAL_H_ */
