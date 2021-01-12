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

#ifndef QBIPSTREAMMANIFESTPLAYBACKINFO_H
#define QBIPSTREAMMANIFESTPLAYBACKINFO_H

/**
 * @file QBIPStreamManifestPlaybackInfo.h IPStream Manager Manifest Playback Info
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <dataformat/sv_data_format.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>

/**
 * @defgroup QBIPStreamManifestPlaybackInfo IPStream Manager Manifest Playback Info
 * @{
 * High-level representation of IPStream Manager Manifest Playback Info
 *
 **/

/**
 * Specifies type of media presentation
 **/
typedef enum {
    QBIPStreamMediaPresentationType_unknown = -1, ///< special marker when type is unknown
    QBIPStreamMediaPresentationType_static,       ///< for static media presentation e.g. VOD, NPVR
    QBIPStreamMediaPresentationType_dynamic       ///< for live media presentation
} QBIPStreamMediaPresentationType;

/**
 * QBIPStreamManifestPlaybackInfo.
 * @class QBIPStreamManifestPlaybackInfo
 * @extends SvObject
 */
typedef struct QBIPStreamManifestPlaybackInfo_ *QBIPStreamManifestPlaybackInfo;

/**
 * Create an instance of QBIPStreamManifestPlaybackInfo class.
 *
 * @param[in]  format   data format handle
 * @param[out] errorOut error info
 *
 * @return              an instance of QBIPStreamManifestPlaybackInfo class
 **/
QBIPStreamManifestPlaybackInfo
QBIPStreamManifestPlaybackInfoCreate(struct svdataformat *format,
                                     SvErrorInfo *errorOut);

/**
 * Set data format for manifest playback info.
 *
 * @param[in]  self     IPStream Manager manifest playback info handle
 * @param[in]  format   data format
 * @param[out] errorOut error info
 **/
void
QBIPStreamManifestPlaybackInfoSetFormat(QBIPStreamManifestPlaybackInfo self,
                                        struct svdataformat *format,
                                        SvErrorInfo *errorOut);

/**
 * Set media presentation type.
 *
 * @param[in]  self                      IPStream Manager manifest playback info handle
 * @param[in]  mediaPresentationType     type of media presentation
 * @param[out] errorOut                  error info
 **/
void
QBIPStreamManifestPlaybackInfoSetMediaPresentationType(QBIPStreamManifestPlaybackInfo self,
                                                       QBIPStreamMediaPresentationType mediaPresentationType,
                                                       SvErrorInfo *errorOut);

/**
 * Set media presentation duration.
 *
 * @param[in]  self                      IPStream Manager manifest playback info handle
 * @param[in]  mediaPresentationDuration duration of media presentation
 * @param[out] errorOut                  error info
 **/
void
QBIPStreamManifestPlaybackInfoSetMediaPresentationDuration(QBIPStreamManifestPlaybackInfo self,
                                                           SvTime mediaPresentationDuration,
                                                           SvErrorInfo *errorOut);
/**
 * Set maximum segment duration.
 *
 * @param[in]  self               IPStream Manager manifest playback info handle
 * @param[in]  maxSegmentDuration maximum duration of segment in media presentation
 * @param[out] errorOut           error info
 **/
void
QBIPStreamManifestPlaybackInfoSetMaxSegmentDuration(QBIPStreamManifestPlaybackInfo self,
                                                    SvTime maxSegmentDuration,
                                                    SvErrorInfo *errorOut);

/**
 * Set availability start time of media presentation.
 *
 * @param[in]  self                  IPStream Manager manifest playback info handle
 * @param[in]  availabilityStartTime availability start time (wall-clock-time)
 * @param[out] errorOut              error info
 **/
void
QBIPStreamManifestPlaybackInfoSetAvailabilityStartTime(QBIPStreamManifestPlaybackInfo self,
                                                       SvTime availabilityStartTime,
                                                       SvErrorInfo *errorOut);

/**
 * Set availability end time of media presentation.
 *
 * @param[in]  self                IPStream Manager manifest playback info handle
 * @param[in]  availabilityEndTime availability end time (wall-clock-time)
 * @param[out] errorOut            error info
 **/
void
QBIPStreamManifestPlaybackInfoSetAvailabilityEndTime(QBIPStreamManifestPlaybackInfo self,
                                                     SvTime availabilityEndTime,
                                                     SvErrorInfo *errorOut);

/**
 * Set minimum buffer time.
 *
 * Parameter describes how much buffer a client should have under ideal network
 * conditions.
 *
 * @param[in]  self          IPStream Manager manifest playback info handle
 * @param[in]  minBufferTime minimum buffer time
 * @param[out] errorOut      error info
 **/
void
QBIPStreamManifestPlaybackInfoSetMinBufferTime(QBIPStreamManifestPlaybackInfo self,
                                               SvTime minBufferTime,
                                               SvErrorInfo *errorOut);

/**
 * Get data format from manifest playback info.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         data format, otherwise @c NULL in case of error
 **/
struct svdataformat *
QBIPStreamManifestPlaybackInfoGetFormat(QBIPStreamManifestPlaybackInfo self);

/**
 * Get media presentation type.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         type of media presentation, otherwise @c QBIPStreamMediaPresentationType_unknown in case of error
 **/
QBIPStreamMediaPresentationType
QBIPStreamManifestPlaybackInfoGetMediaPresentationType(QBIPStreamManifestPlaybackInfo self);

/**
 * Get media presentation duration.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         duration of media presentation
 **/
SvTime
QBIPStreamManifestPlaybackInfoGetMediaPresentationDuration(QBIPStreamManifestPlaybackInfo self);

/**
 * Get maximum segment duration.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         maximum duration of segment in media presentation
 **/
SvTime
QBIPStreamManifestPlaybackInfoGetMaxSegmentDuration(QBIPStreamManifestPlaybackInfo self);

/**
 * Get availability start time of media presentation.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         availability start time of media presentation (wall-clock-time)
 **/
SvTime
QBIPStreamManifestPlaybackInfoGetAvailabilityStartTime(QBIPStreamManifestPlaybackInfo self);

/**
 * Get availability end time of media presentation.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         availability end time of media presentation (wall-clock-time)
 **/
SvTime
QBIPStreamManifestPlaybackInfoGetAvailabilityEndTime(QBIPStreamManifestPlaybackInfo self);

/**
 * Get minimum buffer time.
 *
 * @param[in] self IPStream Manager manifest playback info handle
 * @return         minimum buffer time
 **/
SvTime
QBIPStreamManifestPlaybackInfoGetMinBufferTime(QBIPStreamManifestPlaybackInfo self);

/**
 * @}
 */

#endif // QBIPSTREAMMANIFESTPLAYBACKINFO_H
