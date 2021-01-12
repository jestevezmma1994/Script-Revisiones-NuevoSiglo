/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSMOOTHSTREAMINGUTILS_H_
#define QBSMOOTHSTREAMINGUTILS_H_

/**
 * @file  QBSmoothStreamingUtils.h
 * @brief Utility methods operating on Smooth Streaming Media object
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SmoothStreaming/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvBuf.h>
#include <dataformat/sv_data_format.h>
#include <stdbool.h>

/**
 * @defgroup QBSmoothStreamingUtils Smooth Streaming utils class
 * @ingroup containers
 * @{
 **/

/**
 * Create data format for Smooth Streaming Clip with given index in Smooth Streaming Media object.
 * This method creates format for QualityLevel with lowest bitrate of each type (audio, video and text) of StreamIndex.
 *
 * @param[in] media                     Smooth Streaming Media object
 * @param[in] clipIndex                 index of Smooth Streaming Clip
 * @param[in] isAudioVideoSyncNeeded    @c true if audio and video synchronization needed
 * @param[in,out] streamIdentificators  array of stream identificators
 * @param[out] errorOut                 error info
 * @return                              new created data format, otherwise @c NULL in case of error
 **/
struct svdataformat*
QBSmoothStreamingUtilsCreateDataFormat(MSSMedia media, size_t clipIndex, bool isAudioVideoSyncNeeded, short **streamIdentificators, SvErrorInfo *errorOut);

/**
 * Create H264 private data for QualityLevel with given NAL unit length.
 *
 * @param[in] inputData                 base16-encoded string data with H264 private data
 * @param[in] inputDataLength           length in bytes of base16-encoded H264 private data
 * @param[in] nalUnitLength             number of bytes used for the NAL
 * @param[out] errorOut                 error info
 * @return                              new created H264
 **/
SvBuf
QBSmoothStreamingUtilsCreateH264QualityLevelPrivateData(const char *inputData, unsigned int inputDataLength, unsigned int nalUnitLength, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBSMOOTHSTREAMINGUTILS_H_ */
