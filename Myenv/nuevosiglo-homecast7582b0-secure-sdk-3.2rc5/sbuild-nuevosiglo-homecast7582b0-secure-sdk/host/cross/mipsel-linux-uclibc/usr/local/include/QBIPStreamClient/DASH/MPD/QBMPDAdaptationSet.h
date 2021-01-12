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

#ifndef QBMPADAPTATIONSET_H_
#define QBMPADAPTATIONSET_H_

/**
 * @file QBMPDAdaptationSet.h
 * @brief Adaptation Set API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDCommonAttributes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDXlink.h>
#include <stdbool.h>

/**
 * @defgroup QBMPDAdaptationSet Adaptation Set element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Adaptation Set element class.
 * @class QBMPDAdaptationSet
 * @extends QBMPDRepresentationBase
 **/
typedef struct QBMPDAdaptationSet_ *QBMPDAdaptationSet;

/**
 * Get runtime type identification object representing
 * type of Adaptation Set element class.
 *
 * @return Adaptation Set element class
 **/
extern SvType
QBMPDAdaptationSet_getType(void);

/**
 * Get xlink of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         Xlink element, @c NULL in case of error
 **/
extern QBMPDXlink
QBMPDAdaptationSetGetXlink(QBMPDAdaptationSet self);

/**
 * Get identifier of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         identifier, @c -1 in case of error
 **/
extern int
QBMPDAdaptationSetGetId(QBMPDAdaptationSet self);

/**
 * Get group of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         group, @c -1 in case of error
 **/
extern int
QBMPDAdaptationSetGetGroup(QBMPDAdaptationSet self);

/**
 * Get language code of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         language code, @c NULL in case of error
 **/
extern SvString
QBMPDAdaptationSetGetLang(QBMPDAdaptationSet self);

/**
 * Get content type of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         content type, @c NULL in case of error
 **/
extern SvString
QBMPDAdaptationSetGetContentType(QBMPDAdaptationSet self);

/**
 * Get picture aspect ratio of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element.
 * @return         picture aspect ratio, @c NULL in case of error
 **/
extern SvString
QBMPDAdaptationSetGetPar(QBMPDAdaptationSet self);

/**
 * Get minimum and maximum bandwidth attributes of Adaptation Set element.
 *
 * @param[in]  self         Adaptation Set element handle
 * @param[out] minBandwidth minimum bandwidth, @c -1 in case of error
 * @param[out] maxBandwidth maximum bandwidth, @c -1 in case of error
 **/
extern void
QBMPDAdaptationSetGetBandwidthRange(QBMPDAdaptationSet self,
                                    int *minBandwidth,
                                    int *maxBandwidth);

/**
 * Get minimum and maximum width attributes of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @param[out] minWidth minimum width, @c -1 in case of error
 * @param[out] maxWidth maximum width, @c -1 in case of error
 **/
extern void
QBMPDAdaptationSetGetWidthRange(QBMPDAdaptationSet self,
                                int *minWidth,
                                int *maxWidth);

/**
 * Get minimum and maximum height attributes of Adaptation Set element.
 *
 * @param[in]  self      Adaptation Set element handle
 * @param[out] minHeight minimum height, @c -1 in case of error
 * @param[out] maxHeight maximum height, @c -1 in case of error
 **/
extern void
QBMPDAdaptationSetGetHeightRange(QBMPDAdaptationSet self,
                                 int *minHeight,
                                 int *maxHeight);

/**
 * Get minimum and maximum attributes of Adaptation Set element.
 *
 * @param[in]  self           Adaptation Set element handle
 * @param[out] minFrameRate_f numerator of minimum frame rate
 * @param[out] minFrameRate_d denominator of minimum frame rate
 * @param[out] maxFrameRate_f numerator of  maximum frame rate
 * @param[out] maxFrameRate_d denominator of  maximum frame rate
 **/
extern void
QBMPDAdaptationSetGetFrameRateRange(QBMPDAdaptationSet self,
                                    unsigned int *minFrameRate_f,
                                    unsigned int *minFrameRate_d,
                                    unsigned int *maxFrameRate_f,
                                    unsigned int *maxFrameRate_d);

/**
 * Get segment alignment attribute of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              segment alignment
 **/
extern bool
QBMPDAdaptationSetGetSegmentAligment(QBMPDAdaptationSet self);

/**
 * Get subsegment alignment attribute of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              subsegment alignment
 **/
extern bool
QBMPDAdaptationSetGetSubSegmentAligment(QBMPDAdaptationSet self);

/**
 * Get subsegment start with SAP attribute of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         subsegment start with SAP, @c QBMPDCommonAttributesSAPType_unknown in case of error
 **/
extern QBMPDCommonAttributesSAPType
QBMPDAdaptationSetGetSubSegmentStartsWithSAP(QBMPDAdaptationSet self);

/**
 * Get bitstream switching of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              bitstream switching
 **/
extern bool
QBMPDAdaptationSetGetBitstreamSwitching(QBMPDAdaptationSet self);

/**
 * Get array of accessibility descriptors of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of accessibility descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetAccessibilities(QBMPDAdaptationSet self);

/**
 * Get array of role descriptors of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of role descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetRoles(QBMPDAdaptationSet self);

/**
 * Get array of rating descriptors of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of rating descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetRatings(QBMPDAdaptationSet self);

/**
 * Get array of viewpoint descriptors of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of viewpoint descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetViewpints(QBMPDAdaptationSet self);

/**
 * Get array of Content Component elements of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of Content Component elements, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetContentComponents(QBMPDAdaptationSet self);

/**
 * Get array of Base URL elements of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of Base URL elements, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetBaseURLs(QBMPDAdaptationSet self);

/**
 * Get Segment Base element of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         Segment Base element, @c NULL in case of error
 **/
extern QBMPDSegmentBase
QBMPDAdaptationSetGetSegmentBase(QBMPDAdaptationSet self);

/**
 * Get Segment List element of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         Segment List element, @c NULL in case of error
 **/
extern QBMPDSegmentList
QBMPDAdaptationSetGetSegmentList(QBMPDAdaptationSet self);

/**
 * Get Segment Template element of Adaptation Set element.
 *
 * @param[in] self Adaptation Set element handle
 * @return         Segment Template element, @c NULL in case of error
 **/
extern QBMPDSegmentTemplate
QBMPDAdaptationSetGetSegmentTemplate(QBMPDAdaptationSet self);

/**
 * Get array of Representation element of Adaptation Set element.
 *
 * @param[in]  self     Adaptation Set element handle
 * @return              array of Representation elements, @c NULL in case of error
 **/
extern SvArray
QBMPDAdaptationSetGetRepresentations(QBMPDAdaptationSet self);

/**
 * @}
 **/

#endif /* QBMPDADAPTATIONESET_H_ */
