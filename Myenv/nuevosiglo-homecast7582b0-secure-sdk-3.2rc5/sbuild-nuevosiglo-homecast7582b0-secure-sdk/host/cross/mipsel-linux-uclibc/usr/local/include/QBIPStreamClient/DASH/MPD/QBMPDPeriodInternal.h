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

#ifndef QBMPDPERIODINTERNAL_H_
#define QBMPDPERIODINTERNAL_H_

/**
 * @file QBMPDPeriodInternal.h
 * @brief Period element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDBaseURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDEventStream.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSubset.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDPeriod.h>
#include <stdbool.h>

/**
 * @addtogroup QBMPDPeriod
 * @{
 **/

/**
 * Create Period element.
 *
 * @param[out] errorOut error info
 * @return             new instance of Period element, @c NULL in case of error
 **/
extern QBMPDPeriod
QBMPDPeriodCreate(SvErrorInfo *errorOut);

/**
 * Set xlink for Period element.
 *
 * @param[in]  self     Period element handle
 * @param[in]  xlink    Xlink element
 * @param[out] errorOut error info
 **/
extern void
QBMPDPeriodSetXlink(QBMPDPeriod self,
                    QBMPDXlink xlink,
                    SvErrorInfo *errorOut);

/**
 * Set identifier for Period element.
 *
 * @param[in]  self     Period element handle
 * @param[in]  id       identifier for Period
 * @param[out] errorOut error info
 **/
extern void
QBMPDPeriodSetId(QBMPDPeriod self,
                 SvString id,
                 SvErrorInfo *errorOut);

/**
 * Set start time for Period element.
 *
 * @param[in]  self     Period element handle
 * @param[in]  start    start time of Period
 * @param[out] errorOut error info
 **/
extern void
QBMPDPeriodSetStart(QBMPDPeriod self,
                    SvTime start,
                    SvErrorInfo *errorOut);

/**
 * Set duration for Period element.
 *
 * @param[in]  self     Period element handle
 * @param[in]  duration duration of Period
 * @param[out] errorOut error info
 **/
extern void
QBMPDPeriodSetDuration(QBMPDPeriod self,
                       SvTime duration,
                       SvErrorInfo *errorOut);

/**
 * Set bitstream switching attribute for Period element.
 *
 * @param[in]  self               Period element handle
 * @param[in]  bitstreamSwitching bitstream switching
 * @param[out] errorOut           error info
 **/
extern void
QBMPDPeriodSetBitstreamSwitching(QBMPDPeriod self,
                                 bool bitstreamSwitching,
                                 SvErrorInfo *errorOut);
/**
 * Add Base URL element to Period element.
 *
 * @param[in]  self     Period element handle
 * @param[in]  baseURL  Base URL element
 * @param[out] errorOut error info
 **/
extern void
QBMPDPeriodAddBaseURL(QBMPDPeriod self,
                      QBMPDBaseURL baseURL,
                      SvErrorInfo *errorOut);

/**
 * Set Segment Base element for Period element.
 *
 * @param[in]  self        Period element handle
 * @param[in]  segmentBase Segment Base element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDPeriodSetSegmentBase(QBMPDPeriod self,
                          QBMPDSegmentBase segmentBase,
                          SvErrorInfo *errorOut);

/**
 * Set Segment List element for Period element.
 *
 * @param[in]  self        Period element handle
 * @param[in]  segmentList Segment List element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDPeriodSetSegmentList(QBMPDPeriod self,
                          QBMPDSegmentList segmentList,
                          SvErrorInfo *errorOut);

/**
 * Set Segment Template element for Period element.
 *
 * @param[in]  self            Period element handle
 * @param[in]  segmentTemplate Segment Template element
 * @param[out] errorOut        error info
 **/
extern void
QBMPDPeriodSetSegmentTemplate(QBMPDPeriod self,
                              QBMPDSegmentTemplate segmentTemplate,
                              SvErrorInfo *errorOut);

/**
 * Set asset identifier descriptor for Period element.
 *
 * @param[in]  self            Period element handle
 * @param[in]  assetIdentifier asset identifier descriptor
 * @param[out] errorOut        error info
 **/
extern void
QBMPDPeriodSetAssetIdentifier(QBMPDPeriod self,
                              QBMPDDescriptor assetIdentifier,
                              SvErrorInfo *errorOut);

/**
 * Add Event Stream element to Period element.
 *
 * @param[in]  self        Period element handle
 * @param[in]  eventStream Event Stream element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDPeriodAddEventStream(QBMPDPeriod self,
                          QBMPDEventStream eventStream,
                          SvErrorInfo *errorOut);

/**
 * Add Adaptation Set element to Period element.
 *
 * @param[in]  self          Period element handle
 * @param[in]  adaptationSet Adaptation Set element
 * @param[out] errorOut      error info
 **/
extern void
QBMPDPeriodAddAdaptationSet(QBMPDPeriod self,
                            QBMPDAdaptationSet adaptationSet,
                            SvErrorInfo *errorOut);

/**
 * Add Subset element to Period element.
 *
 * @param[in]  self     Period element handle
 * @param[in]  subset   Subset element
 * @param[out] errorOut error info
 **/
extern void
QBMPDPeriodAddSubset(QBMPDPeriod self,
                     QBMPDSubset subset,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDPERIODINTERNAL_H_ */
