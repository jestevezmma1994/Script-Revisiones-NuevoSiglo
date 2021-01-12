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

#ifndef QBMPDREPRESENTATIONBASEINTERNAL_H_
#define QBMPDREPRESENTATIONBASEINTERNAL_H_

/**
 * @file QBMPDRepresentationBaseInternal.h
 * @brief Representation Base element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDCommonAttributes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDRepresentationBase.h>

/**
 * @addtogroup QBMPDRepresentationBase
 * @{
 **/

/**
 * Representation Base element class internals
 **/
struct QBMPDRepresentationBase_ {
    struct SvObject_ super_;                     ///< base class

    /**
     * Representation Base element attributes
     **/
    SvString profiles;                           ///< specifies profiles which the associated Representation(s)
    int width;                                   ///< horizontal visual presentation size of video media type
    int height;                                  ///< vertical visual presentation size of video media type
    SvString sar;                                ///< sample aspect ratio of the video media type
    struct {
        unsigned int f;                          ///< numerator part of frame rate
        unsigned int d;                          ///< denominator part of frame rate
    } frameRate;                                 ///< output frame rate of the video media type
    int minAudioSamplingRate;                    ///< minimum sampling rate of audio media type (sps)
    int maxAudioSamplingRate;                    ///< maximum sampling rate of audio media type (sps)
    SvString mimeType;                           ///< MIME type
    SvString segmentProfiles;                    ///< profiles of Segments
    SvString codecs;                             ///< codecs present within Representation
    double maximumSAPPeriod;                     ///< maximum SAP interval in seconds
    QBMPDCommonAttributesSAPType startWithSAP;   ///< type of SAP with Media Segment starts
    double maxPlayoutRate;                       ///< maximum playout rate
    bool codingDependency;                       ///< specifies if access unit depends on one ore more others access units
    QBMPDCommonAttributesVideoScanType scanType; ///< type of source material of the video media type

    /**
     * Representation Base child elements
     **/
    SvArray framePackings;                       ///< information about frame-packing of the video media type
    SvArray audioChannelConfigurations;          ///< information about audio channel configuration of the audio media type
    SvArray contentProtections;                  ///< information about content protection schemes
    SvArray essentialProperties;                 ///< information about containing element
    SvArray supplementProperties;                ///< supplement information about containing element
    SvArray inbandEventStreams;                  ///< information about presence of an inband even stream in associated Representations
};

/**
 * Initialize Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseInitialize(QBMPDRepresentationBase self,
                                  SvErrorInfo *errorOut);

/**
 * Set MIME type for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  mimeType MIME type
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetMimeType(QBMPDRepresentationBase self,
                                   SvString mimeType,
                                   SvErrorInfo *errorOut);

/**
 * Set profiles for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  profiles profiles which associated with Representation(s)
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetProfiles(QBMPDRepresentationBase self,
                                   SvString profiles,
                                   SvErrorInfo *errorOut);

/**
 * Set width for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  width    width of video media type
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetWidth(QBMPDRepresentationBase self,
                                int width,
                                SvErrorInfo *errorOut);

/**
 * Set height for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  height   height of video media type
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetHeight(QBMPDRepresentationBase self,
                                 int height,
                                 SvErrorInfo *errorOut);

/**
 * Set sample aspect ratio for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  sar      sample aspect ratio of video media type
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetSar(QBMPDRepresentationBase self,
                              SvString sar,
                              SvErrorInfo *errorOut);

/**
 * Set frame rate for Representation Base element.
 *
 * @param[in]  self      Representation Base element handle
 * @param[in]  f         numerator part of output frame rate of video media type
 * @param[in]  d         denominator part of output frame rate of video media type
 * @param[out] errorOut  error info
 **/
extern void
QBMPDRepresentationBaseSetFrameRate(QBMPDRepresentationBase self,
                                    unsigned int f,
                                    unsigned int d,
                                    SvErrorInfo *errorOut);

/**
 * Set minimum and maximum audio sampling rate for Representation Base element.
 *
 * @param[in]  self                 Representation Base element handle
 * @param[in]  minAudioSamplingRate minimum sampling rate of audio media type
 * @param[in]  maxAudioSamplingRate maximum sampling rate of audio media type
 * @param[out] errorOut             error info
 **/
extern void
QBMPDRepresentationBaseSetAudioSamplingRate(QBMPDRepresentationBase self,
                                            int minAudioSamplingRate,
                                            int maxAudioSamplingRate,
                                            SvErrorInfo *errorOut);

/**
 * Set segment profiles for Representation Base element.
 *
 * @param[in]  self            Representation Base element handle
 * @param[in]  segmentProfiles segment profiles of Segments
 * @param[out] errorOut        error info
 **/
extern void
QBMPDRepresentationBaseSetSegmentProfiles(QBMPDRepresentationBase self,
                                          SvString segmentProfiles,
                                          SvErrorInfo *errorOut);

/**
 * Set codecs for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  codecs   codecs present within Representation
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetCodecs(QBMPDRepresentationBase self,
                                 SvString codecs,
                                 SvErrorInfo *errorOut);

/**
 * Set maximum SAP interval for Representation Base element.
 *
 * @param[in]  self             Representation Base element handle
 * @param[in]  maximumSAPPeriod maximum SAP interval in seconds
 * @param[out] errorOut         error info
 **/
extern void
QBMPDRepresentationBaseSetMaximumSAPPeriod(QBMPDRepresentationBase self,
                                           double maximumSAPPeriod,
                                           SvErrorInfo *errorOut);

/**
 * Set start with SAP attribute for Representation Base element.
 *
 * @param[in]  self         Representation Base element handle
 * @param[in]  startWithSAP type of SAP with Media Segment starts
 * @param[out] errorOut     error info
 **/
extern void
QBMPDRepresentationBaseSetStartWithSAP(QBMPDRepresentationBase self,
                                       QBMPDCommonAttributesSAPType startWithSAP,
                                       SvErrorInfo *errorOut);

/**
 * Set maximum playout rate for Representation Base element.
 *
 * @param[in]  self           Representation Base element handle
 * @param[in]  maxPlayoutRate maximum playout rate
 * @param[out] errorOut       error info
 **/
extern void
QBMPDRepresentationBaseSetMaxPlayoutRate(QBMPDRepresentationBase self,
                                         double maxPlayoutRate,
                                         SvErrorInfo *errorOut);

/**
 * Set coding dependency for Representation Base element.
 *
 * @param[in]  self             Representation Base element handle
 * @param[in]  codingDependency coding dependency
 * @param[out] errorOut         error info
 **/
extern void
QBMPDRepresentationBaseSetCodingDependency(QBMPDRepresentationBase self,
                                           bool codingDependency,
                                           SvErrorInfo *errorOut);

/**
 * Set scan type for Representation Base element.
 *
 * @param[in]  self     Representation Base element handle
 * @param[in]  scanType type of source material of video media type
 * @param[out] errorOut error info
 **/
extern void
QBMPDRepresentationBaseSetScanType(QBMPDRepresentationBase self,
                                   QBMPDCommonAttributesVideoScanType scanType,
                                   SvErrorInfo *errorOut);

/**
 * Add frame packing descriptor to Representation Base element.
 *
 * @param[in]  self         Representation Base element handle
 * @param[in]  framePacking frame packing descriptor
 * @param[out] errorOut     error info
 **/
extern void
QBMPDRepresentationBaseAddFramePacking(QBMPDRepresentationBase self,
                                       QBMPDDescriptor framePacking,
                                       SvErrorInfo *errorOut);

/**
 * Add audio channel configuration descriptor to Representation Base element.
 *
 * @param[in]  self                      Representation Base element handle
 * @param[in]  audioChannelConfiguration audio channel configuration descriptor
 * @param[out] errorOut                  error info
 **/
extern void
QBMPDRepresentationBaseAddAudioChannelConfiguration(QBMPDRepresentationBase self,
                                                    QBMPDDescriptor audioChannelConfiguration,
                                                    SvErrorInfo *errorOut);

/**
 * Add content protection descriptor to Representation Base element.
 *
 * @param[in]  self              Representation Base element handle
 * @param[in]  contentProtection content protection descriptor
 * @param[out] errorOut          error info
 **/
extern void
QBMPDRepresentationBaseAddContentProtection(QBMPDRepresentationBase self,
                                            QBMPDDescriptor contentProtection,
                                            SvErrorInfo *errorOut);

/**
 * Add essential property descriptor to Representation Base element.
 *
 * @param[in]  self              Representation Base element handle
 * @param[in]  essentialProperty essential property descriptor
 * @param[out] errorOut          error info
 **/
extern void
QBMPDRepresentationBaseAddEssentialProperty(QBMPDRepresentationBase self,
                                            QBMPDDescriptor essentialProperty,
                                            SvErrorInfo *errorOut);

/**
 * Add supplement property descriptor to Representation Base element.
 *
 * @param[in]  self               Representation Base element handle
 * @param[in]  supplementProperty supplement property descriptor
 * @param[out] errorOut           error info
 **/
extern void
QBMPDRepresentationBaseAddSupplementProperty(QBMPDRepresentationBase self,
                                             QBMPDDescriptor supplementProperty,
                                             SvErrorInfo *errorOut);

/**
 * Add inband event stream descriptor to Representation Base element.
 *
 * @param[in]  self              Representation Base element handle
 * @param[in]  inbandEventStream inband event stream descriptor
 * @param[out] errorOut          error info
 **/
extern void
QBMPDRepresentationBaseAddInbandEventStream(QBMPDRepresentationBase self,
                                            QBMPDDescriptor inbandEventStream,
                                            SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDREPRESENTATIONBASEINTERNAL_H_ */
