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

#ifndef QBMPDELEMENTINTERNAL_H_
#define QBMPDELEMENTINTERNAL_H_

/**
 * @file QBMPDElementInternal.h
 * @brief Media Presentation Description element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDBaseURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDMetrics.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDProgramInformation.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDElement.h>

/**
 * @addtogroup QBMPDElement
 * @{
 **/

/**
 * Create MPD element object.
 *
 * @param[in]  profiles      MPD profiles
 * @param[in]  minBufferTime minimum size of buffer
 * @param[out] errorOut      error info
 * @return                   new instance of MPD element, @c NULL in case of error
 **/
extern QBMPDElement
QBMPDElementCreate(SvString profiles,
                   SvTime minBufferTime,
                   SvErrorInfo *errorOut);

/**
 * Set identifier for MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  id       MPD identifier
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementSetId(QBMPDElement self,
                  SvString id,
                  SvErrorInfo *errorOut);

/**
 * Set profiles for MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  profiles MPD profiles
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementSetProfiles(QBMPDElement self,
                        SvString profiles,
                        SvErrorInfo *errorOut);

/**
 * Set type for MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  type     type of Media Presentation
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementSetType(QBMPDElement self,
                    QBMPDElementPresentationType type,
                    SvErrorInfo *errorOut);

/**
 * Set availability start time for MPD element.
 *
 * @param[in]  self                  MPD element handle
 * @param[in]  availabilityStartTime availability start time in UTC
 * @param[out] errorOut              error info
 **/
extern void
QBMPDElementSetAvailabilityStartTime(QBMPDElement self,
                                     SvTime availabilityStartTime,
                                     SvErrorInfo *errorOut);

/**
 * Set availability end time for MPD element.
 *
 * @param[in]  self                MPD element handle
 * @param[in]  availabilityEndTime availability end time in UTC
 * @param[out] errorOut            error info
 **/
extern void
QBMPDElementSetAvailabilityEndTime(QBMPDElement self,
                                   SvTime availabilityEndTime,
                                   SvErrorInfo *errorOut);

/**
 * Set publish time for MPD element.
 *
 * @param[in]  self        MPD element handle
 * @param[in]  publishTime MPD publish time in wall-clock time
 * @param[out] errorOut    error info
 **/
extern void
QBMPDElementSetPublishTime(QBMPDElement self,
                           SvTime publishTime,
                           SvErrorInfo *errorOut);

/**
 * Set Media Presentation duration for MPD element.
 *
 * @param[in]  self                      MPD element handle
 * @param[in]  mediaPresentationDuration media presentation duration
 * @param[out] errorOut                  error info
 **/
extern void
QBMPDElementSetMediaPresentationDuration(QBMPDElement self,
                                         SvTime mediaPresentationDuration,
                                         SvErrorInfo *errorOut);

/**
 * Set minimum update period for MPD element.
 *
 * @param[in]  self                MPD element handle
 * @param[in]  minimumUpdatePeriod minimum update period in seconds
 * @param[out] errorOut            error info
 **/
extern void
QBMPDElementSetMinimumUpdatePeriod(QBMPDElement self,
                                   SvTime minimumUpdatePeriod,
                                   SvErrorInfo *errorOut);

/**
 * Set minimum buffer time for MPD element.
 *
 * @param[in]  self          MPD element handle
 * @param[in]  minBufferTime minimum buffer time in seconds
 * @param[out] errorOut      error info
 **/
extern void
QBMPDElementSetMinBufferTime(QBMPDElement self,
                             SvTime minBufferTime,
                             SvErrorInfo *errorOut);

/**
 * Set time shift buffer depth for MPD element.
 *
 * @param[in]  self                 MPD element handle
 * @param[in]  timeShiftBufferDepth time shift buffer depth in seconds
 * @param[out] errorOut             error info
 **/
extern void
QBMPDElementSetTimeShiftBufferDepth(QBMPDElement self,
                                    SvTime timeShiftBufferDepth,
                                    SvErrorInfo *errorOut);

/**
 * Set suggest presentation delay for MPD element.
 *
 * @param[in]  self                     MPD element handle
 * @param[in]  suggestPresentationDelay suggest presentation delay in seconds
 * @param[out] errorOut                 error info
 **/
extern void
QBMPDElementSetSuggestPresentationDelay(QBMPDElement self,
                                        SvTime suggestPresentationDelay,
                                        SvErrorInfo *errorOut);

/**
 * Set max segment duration for MPD element.
 *
 * @param[in]  self               MPD element handle
 * @param[in]  maxSegmentDuration max segment duration in seconds
 * @param[out] errorOut           error info
 **/
extern void
QBMPDElementSetMaxSegmentDuration(QBMPDElement self,
                                  SvTime maxSegmentDuration,
                                  SvErrorInfo *errorOut);

/**
 * Set max subsegment duration for MPD element.
 *
 * @param[in]  self                  MPD element handle
 * @param[in]  maxSubsegmentDuration max subsegment duration in seconds
 * @param[out] errorOut              error info
 **/
extern void
QBMPDElementSetMaxSubsegmentDuration(QBMPDElement self,
                                     SvTime maxSubsegmentDuration,
                                     SvErrorInfo *errorOut);

/**
 * Add program information element to MPD element.
 *
 * @param[in]  self               MPD element handle
 * @param[in]  programInformation program information element
 * @param[out] errorOut           error info
 **/
extern void
QBMPDElementAddProgramInformation(QBMPDElement self,
                                  QBMPDProgramInformation programInformation,
                                  SvErrorInfo *errorOut);
/**
 * Add base URL element to MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  baseURL  base URL element
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementAddBaseURL(QBMPDElement self,
                       QBMPDBaseURL baseURL,
                       SvErrorInfo *errorOut);

/**
 * Add location element to MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  location location element
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementAddLocation(QBMPDElement self,
                        SvURI location,
                        SvErrorInfo *errorOut);

/**
 * Add period element to MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  period   period element
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementAddPeriod(QBMPDElement self,
                      QBMPDPeriod period,
                      SvErrorInfo *errorOut);

/**
 * Add metrics element to MPD element.
 *
 * @param[in]  self     MPD element handle
 * @param[in]  metrics  metrics element
 * @param[out] errorOut error info
 **/
extern void
QBMPDElementAddMetrics(QBMPDElement self,
                       QBMPDMetrics metrics,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDELEMENTINTERNAL_H_ */
