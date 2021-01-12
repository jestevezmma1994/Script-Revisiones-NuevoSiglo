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

#ifndef QBMPDREPRESENTATIONBASE_H_
#define QBMPDREPRESENTATIONBASE_H_

/**
 * @file QBMPDRepresentationBase.h
 * @brief Representation Base element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDCommonAttributes.h>
#include <stdbool.h>

/**
 * @defgroup QBMPDRepresentationBase Representation Base element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Representation Base element class.
 * @class QBMPDRepresentationBase
 * @extends SvObject
 **/
typedef struct QBMPDRepresentationBase_ *QBMPDRepresentationBase;

/**
 * Get runtime type identification object representing
 * type of Representation Base element class.
 *
 * @return Representation Base element class
 **/
extern SvType
QBMPDRepresentationBase_getType(void);

/**
 * Get profiles of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         profiles which associated with Representation(s), @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationBaseGetProfiles(QBMPDRepresentationBase self);

/**
 * Get width of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         width of video media type, @c -1 in case of error
 **/
extern int
QBMPDRepresentationBaseGetWidth(QBMPDRepresentationBase self);

/**
 * Get height of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         height of video media type, @c -1 in case of error
 **/
extern int
QBMPDRepresentationBaseGetHeight(QBMPDRepresentationBase self);

/**
 * Get sample aspect ratio of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         aspect ratio of video media type, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationBaseGetSar(QBMPDRepresentationBase self);

/**
 * Get frame rate of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @param[out] f numerator value of frame rate, 0 if unknown
 * @param[out] d denominator value of frame rate, 0 if unknown
 **/
extern void
QBMPDRepresentationBaseGetFrameRate(QBMPDRepresentationBase self, unsigned int *f, unsigned int *d);

/**
 * Get minimum and maximum audio sampling rate of Representation Base element.
 *
 * @param[in]  self                 Representation Base element handle
 * @param[out] minAudioSamplingRate minimum sampling rate of audio media type, @c -1 in case of error
 * @param[out] maxAudioSamplingRate maximum sampling rate of audio media tyoe, @c -1 in case of error
 **/
extern void
QBMPDRepresentationBaseGetAudioSamplingRate(QBMPDRepresentationBase self,
                                            int *minAudioSamplingRate,
                                            int *maxAudioSamplingRate);

/**
 * Get MIME type of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         MIME type, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationBaseGetMimeType(QBMPDRepresentationBase self);

/**
 * Get segment profiles of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         profiles of Segment, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationBaseGetSegmentProfiles(QBMPDRepresentationBase self);

/**
 * Get codecs of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         codecs present within Representation, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationBaseGetCodecs(QBMPDRepresentationBase self);

/**
 * Get maximum SAP interval of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         maximum SAP interval in seconds
 **/
extern double
QBMPDRepresentationBaseGetMaximumSAPPeriod(QBMPDRepresentationBase self);

/**
 * Get start with SAP attribute of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         start with SAP attribute, @c QBMPDCommonAttributesSAPType_unknown in case of error
 **/
extern QBMPDCommonAttributesSAPType
QBMPDRepresentationBaseGetStartWithSAP(QBMPDRepresentationBase self);

/**
 * Get maximum playout rate of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         maximum playout rate
 **/
extern double
QBMPDRepresentationBaseGetMaxPlayoutRate(QBMPDRepresentationBase self);

/**
 * Get coding dependency of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              coding dependency, @c false in case of error
 **/
extern bool
QBMPDRepresentationBaseGetCodingDependency(QBMPDRepresentationBase self);

/**
 * Get scan type of Representation Base element.
 *
 * @param[in] self Representation Base element handle
 * @return         scan type, @c QBMPDCommonAttributesVideoScanType_unknown in case of error
 **/
extern QBMPDCommonAttributesVideoScanType
QBMPDRepresentationBaseGetScanType(QBMPDRepresentationBase self);

/**
 * Get array of frame packing descriptors of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              array of frame packing descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationBaseGetFramePackings(QBMPDRepresentationBase self);

/**
 * Get array of audio channel configuration descriptors of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              array of audio channel configuration, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationBaseGetAudioChannelConfigurations(QBMPDRepresentationBase self);

/**
 * Get array of content protection descriptors of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              array of content protection descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationBaseGetContentProtections(QBMPDRepresentationBase self);

/**
 * Get array of essential property descriptors of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              array of essential property descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationBaseGetEssentialProperties(QBMPDRepresentationBase self);

/**
 * Get array of supplement property descriptors of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              array of supplement property descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationBaseGetSupplementProperties(QBMPDRepresentationBase self);

/**
 * Get array of inband event stream descriptors of Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @return              array of inband event stream descriptors, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationBaseGetInbandEventStreams(QBMPDRepresentationBase self);

/**
 * @}
 **/

#endif /* QBMPDREPRESENTATIONBASE_H_ */
