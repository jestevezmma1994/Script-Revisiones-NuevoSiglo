/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2020 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSMOOTHSTREAMINGDOWNLOADERINTERNAL_H_
#define QBSMOOTHSTREAMINGDOWNLOADERINTERNAL_H_

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/QBSmoothStreaming/QBSmoothStreamingDownloaderConf.h>
#include <SvPlayerKit/SvBuf.h>
#include <storage_input/storage_input.h>
#include <storage_input/storage_input_http.h>
#include <error_codes.h>
#include "mp4_fragment_parser.h"
#include <SmoothStreaming/util.h>

#include "log.h"
#include "QBSmoothStreamingDownloader.h"
#include "QBSmoothStreamingUtils.h"
#include "QBVariantCalculator/QBVariantCalculator.h"

#define LAST_CHUNK_BITRATE_FACTOR 0.8
#define JUMP_UP_THRESHOLD 0.7
#define JUMP_DOWN_THRESHOLD 0.9

#if SV_LOG_LEVEL > 0
SV_DECL_INT_ENV_FUN_DEFAULT(env_fixed_quality, -1, "QBSSUseFixedQuality", "random qual");
SV_DECL_INT_ENV_FUN_DEFAULT(env_random_quality, 0, "QBSSUseRandomQuality", "fixed qual");
#else
int env_fixed_quality(void)
{
    return -1;
}

int env_random_quality(void)
{
    return 0;
}
#endif

/**
 * Smooth Streaming downloader stream data class
 **/
struct QBSmoothStreamingDownloaderStreamData_ {
    struct SvObject_ super_;

    short streamNumber;
    MSSStreamIndex streamIndex;
    MSSStreamIndexIterator streamIterator;
    struct MSSChunk_ currentChunk;
    SvBuf streamQualityLevelPrivateData;
    struct storage_input *streamInput;
    struct mp4_fragment_parser *streamFragmentParser;
};

typedef struct QBSmoothStreamingDownloaderStreamData_ *QBSmoothStreamingDownloaderStreamData;

/**
 * Smooth Streaming downloader class
 **/
struct QBSmoothStreamingDownloader_ {
    struct SvObject_ super;

    SvHTTPClientEngine httpEngine;
    SvScheduler scheduler;
    SvBufMan bufMan;
    QBSmoothStreamingDownloaderStreamData streamData;
    SvURI baseURI;

    void *owner;
    QBSmoothStreamingDownloaderCallbacks callbacks;

    QBSmoothStreamingDownloaderConf config;
    int qualityVariants;
    int selectedVariant;
    int estimatedBitrate;
    int previousBitrate;
    /// current average bitrate of downloaded data
    /// it is measured only for streams which have more than one quality variant
    int64_t avarageBitrate;
    /// number of measurements of current bitrate
    /// helping value used to count incremental average bitrate
    int64_t bitrateSamplesCount;
    uint64_t outPTS;
    QBSmoothStreamingBitrateMonitor bitrateMonitor;
    bool pauseDownloading;
    bool enabled;

    QBVariantCalculator variantCalculator;
};

SvType
QBSmoothStreamingDownloader_getType(void);

#endif /* QBSMOOTHSTREAMINGDOWNLOADERINTERNAL_H_ */
