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

#ifndef QBMPDREPRESENTATIONINTERNAL_H_
#define QBMPDREPRESENTATIONINTERNAL_H_

/**
 * @file QBMPDRepresentationInternal.h
 * @brief Representation element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDBaseURL.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSubRepresentation.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentation.h>

/**
 * @addtogroup QBMPDRepresentation
 * @{
 **/

/**
 * Create Representation element.
 *
 * @param[in]  id        identifier of Representation
 * @param[in]  bandwidth bandwidth
 * @param[out] errorOut  error info
 * @return               new instance of Representation element, @c NULL in case of error
 **/
extern QBMPDRepresentation
QBMPDRepresentationCreate(SvString id,
                          int bandwidth,
                          SvErrorInfo *errorOut);

/**
 * Set quality ranking attributes for Representation element.
 *
 * @param[in]  self           Representation element handle
 * @param[in]  qualityRanking quality ranking
 * @param[out] errorOut       error info
 **/
extern void
QBMPDRepresentationSetQualityRanking(QBMPDRepresentation self,
                                     int qualityRanking,
                                     SvErrorInfo *errorOut);

/**
 * Set dependency id attribute for Representation element.
 *
 * @param[in]  self         Representation element handle
 * @param[in]  dependencyId dependency id
 * @param[out] errorOut     error info
 **/
extern void
QBMPDRepresentationSetDependencyId(QBMPDRepresentation self,
                                   SvString dependencyId,
                                   SvErrorInfo *errorOut);

/**
 * Set media stream structure id attribute for Representation element.
 *
 * @param[in]  self                   Representation element handle
 * @param[in]  mediaStreamStructureId media stream structure id
 * @param[out] errorOut               error info
 **/
extern void
QBMPDRepresentationSetMediaStreamStructureId(QBMPDRepresentation self,
                                             SvString mediaStreamStructureId,
                                             SvErrorInfo *errorOut);

/**
 * Add Base URL element to Representation element.
 *
 * @param[in]  self     Representation element handle
 * @param[in]  baseURL  Base URL element
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationAddBaseURL(QBMPDRepresentation self,
                              QBMPDBaseURL baseURL,
                              SvErrorInfo *errorOut);

/**
 * Add Sub-Representation element to Representation element.
 *
 * @param[in]  self              Representation element handle
 * @param[in]  subRepresentation Sub-Representation element
 * @param[out] errorOut          error info
 **/
extern void
QBMPDRepresentationAddSubRepresentation(QBMPDRepresentation self,
                                        QBMPDSubRepresentation subRepresentation,
                                        SvErrorInfo *errorOut);

/**
 * Add Segment Base element to Representation element.
 *
 * @param[in]  self        Representation element handle
 * @param[in]  segmentBase Segment Base element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDRepresentationSetSegmentBase(QBMPDRepresentation self,
                                  QBMPDSegmentBase segmentBase,
                                  SvErrorInfo *errorOut);

/**
 * Add Segment List element to Representation element.
 *
 * @param[in]  self        Representation element handle
 * @param[in]  segmentList Segment List element
 * @param[out] errorOut    error info
 **/
extern void
QBMPDRepresentationSetSegmentList(QBMPDRepresentation self,
                                  QBMPDSegmentList segmentList,
                                  SvErrorInfo *errorOut);

/**
 * Add Segment Template to Representation element.
 *
 * @param[in]  self            Representation element handle
 * @param[in]  segmentTemplate Segment Template element
 * @param[out] errorOut        error info
 **/
extern void
QBMPDRepresentationSetSegmentTemplate(QBMPDRepresentation self,
                                      QBMPDSegmentTemplate segmentTemplate,
                                      SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDREPRESENTATIONINTERNAL_H_ */
