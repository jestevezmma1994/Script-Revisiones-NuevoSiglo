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

#ifndef QBMPDMULTIPLESEGMENTBASEINTERNAL_H_
#define QBMPDMULTIPLESEGMENTBASEINTERNAL_H_

/**
 * @file QBMPDMultipleSegmentBaseInternal.h
 * @brief Multiple Segment Base element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBMPDSegmentBaseInternal.h"
#include <SvCore/SvErrorInfo.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTimeline.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDMultipleSegmentBase.h>
#include <stdint.h>

/**
 * @addtogroup QBMPDMultipleSegmentBase
 * @{
 **/

/**
 * Multiple Segment Base element class
 **/
struct QBMPDMultipleSegmentBase_ {
    struct QBMPDSegmentBase_ super_;        ///< base class

    /**
     * Multiple Segment Base element attributes
     **/
    uint64_t duration;                      ///< constant approximate Segment duration (in @c timescale units)
    unsigned int startNumber;               ///< number of first Media Segment in this Representation in the Period

    /**
     * Multiple Segment Base child elements
     **/
    QBMPDSegmentTimeline segmentTimeline;   ///< timeline of arbitrary Segment durations
    QBMPDURL bitstreamSwitching;            ///< URL including possible byte range for Bitstream Switching Segment
};

/**
 * Initialize Multiple Segment Base element.
 *
 * @param[in]  self     Multiple Segment Base element handle
 * @param[out] errorOut error info
 **/
extern void
QBMPDMultipleSegmentBaseInitialize(QBMPDMultipleSegmentBase self,
                                   SvErrorInfo *errorOut);

/**
 * Set duration for Multiple Segment Base element.
 *
 * @param[in]  self     Multiple Segment Base element handle
 * @param[in]  duration constant approximate Segment duration in @c timescale units
 * @param[out] errorOut error info
 **/
extern void
QBMPDMultipleSegmentBaseSetDuration(QBMPDMultipleSegmentBase self,
                                    unsigned int duration,
                                    SvErrorInfo *errorOut);

/**
 * Set start number for Multiple Segment Base element.
 *
 * @param[in]  self        Multiple Segment Base element handle
 * @param[in]  startNumber number of the first Media Segment in this Representation in the Period
 * @param[out] errorOut    error info
 **/
extern void
QBMPDMultipleSegmentBaseSetStartNumber(QBMPDMultipleSegmentBase self,
                                       unsigned int startNumber,
                                       SvErrorInfo *errorOut);

/**
 * Set Segment Timeline element for Multiple Segment Base element.
 *
 * @param[in]  self            Multiple Segment Base element handle
 * @param[in]  segmentTimeline Segment Timeline element
 * @param[out] errorOut        error info
 **/
extern void
QBMPDMultipleSegmentBaseSetSegmentTimeline(QBMPDMultipleSegmentBase self,
                                           QBMPDSegmentTimeline segmentTimeline,
                                           SvErrorInfo *errorOut);

/**
 * Set URL including possible byte range for Bitstream Switching Segment for Multiple Segment Base element.
 *
 * @param[in]  self               Multiple Segment Base element handle
 * @param[in]  bitstreamSwitching URL including possible byte range for Bitstream Switching Segment
 * @param[out] errorOut           error info
 **/
extern void
QBMPDMultipleSegmentBaseSetBitstreamSwitching(QBMPDMultipleSegmentBase self,
                                              QBMPDURL bitstreamSwitching,
                                              SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDMULTIPLESEGMENTBASEINTERNAL_H_ */
