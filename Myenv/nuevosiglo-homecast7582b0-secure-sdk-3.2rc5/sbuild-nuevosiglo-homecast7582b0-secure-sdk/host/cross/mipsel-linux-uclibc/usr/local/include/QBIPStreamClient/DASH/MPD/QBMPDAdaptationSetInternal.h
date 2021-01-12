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

#ifndef QBMPDADAPTATIONSETINTERNAL_H_
#define QBMPDADAPTATIONSETINTERNAL_H_

/**
 * @file QBMPDAdaptationSetInternal.h
 * @brief Adaptation Set element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDAdaptationSet.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDBaseURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDCommonAttributes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDContentComponent.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>
#include <stdbool.h>

/**
 * @addtogroup QBMPDAdaptationSet
 * @{
 **/

/**
 * Create Adaptation Set element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Adaptation Set element, @c NULL in case of error
 **/
extern QBMPDAdaptationSet
QBMPDAdaptationSetCreate(SvErrorInfo *errorOut);

/**
 * Set xlink for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  xlink    Xlink element
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetXlink(QBMPDAdaptationSet self,
                           QBMPDXlink xlink,
                           SvErrorInfo *errorOut);

/**
 * Set identifier for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  id       identifier for Adaptation Set
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetId(QBMPDAdaptationSet self,
                        int id,
                        SvErrorInfo *errorOut);

/**
 * Set group attribute for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  group    identifier for group that is unique in the scope of the contaning Period
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetGroup(QBMPDAdaptationSet self,
                           int group,
                           SvErrorInfo *errorOut);

/**
 * Set language code for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  language language code
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetLang(QBMPDAdaptationSet self,
                          SvString language,
                          SvErrorInfo *errorOut);

/**
 * Set content type for Adaptation Set element.
 *
 * @param[in]  self        Adaptation Set element handle
 * @param[in]  contentType content type
 * @param[out] errorOut    error info
 **/
extern void
QBMPDAdaptationSetSetContentType(QBMPDAdaptationSet self,
                                 SvString contentType,
                                 SvErrorInfo *errorOut);

/**
 * Set picture aspect ratio for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  par      picture aspect ratio of the video media
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetPar(QBMPDAdaptationSet self,
                         SvString par,
                         SvErrorInfo *errorOut);

/**
 * Set minimum and maximum bandwidth attributes for Adaptation Set element.
 *
 * @param[in]  self         Adaptation Set element handle
 * @param[in]  minBandwidth specifies minimum bandwidth in all Representations
 * @param[in]  maxBandwidth specifies maximum bandwidth in all Representations
 * @param[out] errorOut     error info
 **/
extern void
QBMPDAdaptationSetSetBandwidthRange(QBMPDAdaptationSet self,
                                    int minBandwidth,
                                    int maxBandwidth,
                                    SvErrorInfo *errorOut);

/**
 * Set minimum and maximum width attributes for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  minWidth specifies minimum width in all Representations
 * @param[in]  maxWidth specifies maximum width in all Representations
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetWidthRange(QBMPDAdaptationSet self,
                                int minWidth,
                                int maxWidth,
                                SvErrorInfo *errorOut);

/**
 * Set minimum and maximum height attributes for Adaptation Set element.
 *
 * @param[in]  self      Adaptation Set element handle
 * @param[in]  minHeight specifies minimum height in all Representations
 * @param[in]  maxHeight specifies maximum height in all Representations
 * @param[out] errorOut  error info
 **/
extern void
QBMPDAdaptationSetSetHeightRange(QBMPDAdaptationSet self,
                                 int minHeight,
                                 int maxHeight,
                                 SvErrorInfo *errorOut);

/**
 * Set minimum and maximum frame rate attribute for Adaptation Set element.
 *
 * @param[in]  self           Adaptation Set element handle
 * @param[in]  minFrameRate_f specifies numerator of minimum frame rate in all Representations
 * @param[in]  minFrameRate_d specifies denominator of minimum frame rate in all Representations
 * @param[in]  maxFrameRate_f specifies numerator of maximum frame rate in all Representations
 * @param[in]  maxFrameRate_d specifies denominator of maximum frame rate in all Representations
 * @param[out] errorOut       error info
 **/
extern void
QBMPDAdaptationSetSetFrameRateRate(QBMPDAdaptationSet self,
                                   unsigned int minFrameRate_f,
                                   unsigned int minFrameRate_d,
                                   unsigned int maxFrameRate_f,
                                   unsigned int maxFrameRate_d,
                                   SvErrorInfo *errorOut);

/**
 * Set segment alignment attribute for Adaptation Set element.
 *
 * @param[in]  self            Adaptation Set element handle
 * @param[in]  segmentAligment segment alignment
 * @param[out] errorOut        error info
 **/
extern void
QBMPDAdaptationSetSetSegmentAligment(QBMPDAdaptationSet self,
                                     bool segmentAligment,
                                     SvErrorInfo *errorOut);

/**
 * Set subsegment alignment attribute for Adaptation Set element.
 *
 * @param[in]  self               Adaptation Set element handle
 * @param[in]  subSegmentAligment subsegment alignment
 * @param[out] errorOut           error info
 **/
extern void
QBMPDAdaptationSetSetSubSegmentAligment(QBMPDAdaptationSet self,
                                        bool subSegmentAligment,
                                        SvErrorInfo *errorOut);

/**
 * Set subsegment start with SAP attribute for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  sapType  subsegment start with SAP
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetSetSubSegmentStartsWithSAP(QBMPDAdaptationSet self,
                                             QBMPDCommonAttributesSAPType sapType,
                                             SvErrorInfo *errorOut);

/**
 * Set bitstream switching for Adaptation Set element.
 *
 * @param[in]  self               Adaptation Set element
 * @param[in]  bitstreamSwitching bitstream switching
 * @param[out] errorOut           error info
 **/
extern void
QBMPDAdaptationSetSetBitstreamSwitching(QBMPDAdaptationSet self,
                                        bool bitstreamSwitching,
                                        SvErrorInfo *errorOut);

/**
 * Add accessibility descriptor for Adaptation Set element.
 *
 * @param[in]  self          Adaptation Set element handle
 * @param[in]  accessibility information about accessibility scheme
 * @param[out] errorOut      error info
 **/
extern void
QBMPDAdaptationSetAddAccessibility(QBMPDAdaptationSet self,
                                   QBMPDDescriptor accessibility,
                                   SvErrorInfo *errorOut);

/**
 * Add role descriptor for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  role     information about role annotation scheme
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetAddRole(QBMPDAdaptationSet self,
                          QBMPDDescriptor role,
                          SvErrorInfo *errorOut);

/**
 * Add rating descriptor for Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  rating   information about rating scheme
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetAddRating(QBMPDAdaptationSet self,
                            QBMPDDescriptor rating,
                            SvErrorInfo *errorOut);

/**
 * Add viewpoint descriptor for Adaptation Set element.
 *
 * @param[in]  self      Adaptation Set element handle
 * @param[in]  viewpoint information about viewpoint annotation scheme
 * @param[out] errorOut  error info
 **/
extern void
QBMPDAdaptationSetAddViewpoint(QBMPDAdaptationSet self,
                               QBMPDDescriptor viewpoint,
                               SvErrorInfo *errorOut);

/**
 * Add Content Component element to Adaptation Set element.
 *
 * @param[in]  self             Adaptation Set element handle
 * @param[in]  contentComponent Content Component element
 * @param[out] errorOut         error info
 **/
extern void
QBMPDAdaptationSetAddContentComponent(QBMPDAdaptationSet self,
                                      QBMPDContentComponent contentComponent,
                                      SvErrorInfo *errorOut);

/**
 * Add Base URL element to Adaptation Set element handle.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[in]  baseURL  Base URL element
 * @param[out] errorOut error info
 **/
extern void
QBMPDAdaptationSetAddBaseURL(QBMPDAdaptationSet self,
                             QBMPDBaseURL baseURL,
                             SvErrorInfo *errorOut);

/**
 * Add Segment Base element to Adaptation Set element.
 *
 * @param[in]  self        Adaptation Set element handle
 * @param[in]  segmentBase Segment Base element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDAdaptationSetSetSegmentBase(QBMPDAdaptationSet self,
                                 QBMPDSegmentBase segmentBase,
                                 SvErrorInfo *errorOut);

/**
 * Add Segment List element to Adaptation Set element.
 *
 * @param[in]  self        Adaptation Set element handle
 * @param[in]  segmentList Segment List element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDAdaptationSetSetSegmentList(QBMPDAdaptationSet self,
                                 QBMPDSegmentList segmentList,
                                 SvErrorInfo *errorOut);

/**
 * Add Segment Template element to Adaptation Set element.
 *
 * @param[in]  self            Adaptation Set element
 * @param[in]  segmentTemplate Segment Template element
 * @param[out] errorOut        error info
 **/
extern void
QBMPDAdaptationSetSetSegmentTemplate(QBMPDAdaptationSet self,
                                     QBMPDSegmentTemplate segmentTemplate,
                                     SvErrorInfo *errorOut);

/**
 * Add Representation element to Adaptation Set element.
 *
 * @param[in]  self           Adaptation Set element handle
 * @param[in]  representation Representation element
 * @param[out] errorOut       error info
 **/
extern void
QBMPDAdaptationSetAddRepresentation(QBMPDAdaptationSet self,
                                    QBMPDRepresentation representation,
                                    SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDADAPTATIONSETINTERNAL_H_ */
