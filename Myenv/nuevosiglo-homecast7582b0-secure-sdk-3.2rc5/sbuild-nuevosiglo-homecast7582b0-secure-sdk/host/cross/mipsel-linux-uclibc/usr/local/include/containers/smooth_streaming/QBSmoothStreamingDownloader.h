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

#ifndef QBSMOOTHSTREAMINGDOWNLOADER_H_
#define QBSMOOTHSTREAMINGDOWNLOADER_H_

/**
 * @file  QBSmoothStreamingDownloader.h
 * @brief Smooth Streaming downloader API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBSmoothStreamingBitrateMonitor.h"
#include "QBSmoothStreamingDownloader_decl.h"

#include <containers/cont_parser.h>
#include <QBMp4Parser/data.h>
#include <SmoothStreaming/stream_index.h>
#include <SmoothStreaming/types.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/QBSmoothStreaming/QBSmoothStreamingDownloaderConf.h>
#include <SvPlayerKit/SvBufMan.h>
#include <SvPlayerKit/SvChbuf.h>
#include <SvPlayerKit/SvPlayerConfig.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvURI.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <stdbool.h>

struct QBSmoothStreamingDownloaderParams {
    bool live;                                      /* @c false for NPvR, PVR and VOD */
    SvHTTPClientEngine httpEngine;                  /* http engine */
    QBSmoothStreamingDownloaderConf config;         /* settings */
    QBSmoothStreamingBitrateMonitor bitrateMonitor; /* bitrate monitor to get current bitrate */
};

/**
 * @defgroup QBSmoothStreamingDownloader Smooth Streaming downloader class
 * @ingroup containers
 * @{
 **/

struct QBSmoothStreamingDownloaderCallbacks_ {
    /**
     * Notify that an error has occurred while data chunk downloading.
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] streamIndex_      stream index
     * @param[in] err               error code
     * @param[in] canSwitchQuality  indicates if the chunk is available in the lower quality and the download can be tried again
     **/
    void (*error)(void *owner, MSSStreamIndex streamIndex_, int err, bool canSwitchQuality);

    /**
     * Notify that downloading has finished.
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] streamIndex_      stream index
     * @param[in] downloadedBytes   total size of downloaded data
     * @param[in] downloadMs        total time of download
     **/
    void (*finished)(void *owner, MSSStreamIndex streamIndex_);

    /**
     * Notify that frame info has been received.
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] streamIndex_      stream index
     * @param[in] info              MP4 fragment info
     **/
    void (*frameInfo)(void *owner, MSSStreamIndex streamIndex_, QBMp4FragmentInfo *info);

    /**
     * Notify that frame has been parsed.
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] streamIndex_      stream index
     * @param[in] chb               frame buffer
     * @param[in] stats             frame statistics
     **/
    void (*frameParsed)(void *owner, MSSStreamIndex streamIndex_, SvChbuf chb, const struct cont_parser_frame_stats *stats);

    /**
     * Notify about new bitrate
     *
     * @param[in] owner             pointer to owner of callbacks
     * @param[in] newBitrate        new bitrate
     **/
    void (*setNewBitrate)(void *owner, int newBitrate);

    /**
     * Ask about player position
     *
     * @param[in] owner             pointer to owner of callbacks
     * @return                      player position in seconds
     **/
    double (*getPlayerPosition)(void *owner);

    /**
     * Get media start time
     *
     * @param[in] owner             pointer to owner of callbacks
     * @return                      start time of first chunk in seconds
     **/
    double (*getMediaStart)(void *owner);
};
typedef struct QBSmoothStreamingDownloaderCallbacks_ QBSmoothStreamingDownloaderCallbacks;

/**
 * Create Smooth Streaming downloader.
 *
 * @param[in] scheduler         scheduler handle
 * @param[in] bufMan            bufMan handle
 * @param[in] baseURI           base URI handle
 * @param[in] streamIndex       stream index
 * @param[in] params            params needed
 * @param[out] errorOut         error info
 * @return                      new created Smooth Streaming downloader instance,
 *                              otherwise @c NULL in case of error
 **/
QBSmoothStreamingDownloader
QBSmoothStreamingDownloaderCreate(SvScheduler scheduler, SvBufMan bufMan, SvURI baseURI, MSSStreamIndex streamIndex, const struct QBSmoothStreamingDownloaderParams *params, SvErrorInfo *errorOut);

/**
 * Set Smooth Streaming downloader callbacks.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[in] callbacks         callbacks
 * @param[in] owner             pointer to owner of callbacks
 **/
void
QBSmoothStreamingDownloaderSetCallbacks(QBSmoothStreamingDownloader self, QBSmoothStreamingDownloaderCallbacks *callbacks, void *owner);

/**
 * Set iterator of stream data.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[in] iterator          stream index iterator
 * @param[out] errorOut         error info
 **/
void
QBSmoothStreamingDownloaderSetStreamDataIterator(QBSmoothStreamingDownloader self, MSSStreamIndexIterator iterator, SvErrorInfo *errorOut);

/**
 * Reset iterator of stream data.
 *
 * This method removes previous iterator of stream data.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[out] errorOut         error info
 **/
void
QBSmoothStreamingDownloaderResetStreamDataIterator(QBSmoothStreamingDownloader self, SvErrorInfo *errorOut);

/**
 * Get chunk for given stream number and quality level of stream index.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[in] streamNumber      stream number
 * @param[out] errorOut         error info
 **/
void
QBSmoothStreamingDownloaderGetChunk(QBSmoothStreamingDownloader self, short streamNumber, SvErrorInfo *errorOut);

/**
 * Check if chunk is downloading.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[out] errorOut         error info
 * @return                      @c true if is chunk is downloading, otherwise @c false
 **/
bool
QBSmoothStreamingDownloaderIsDownloadingChunk(QBSmoothStreamingDownloader self, SvErrorInfo *errorOut);

/**
 * Cancel chunk downloading.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[out] errorOut         error info
 **/
void
QBSmoothStreamingDownloaderCancelChunkDownloading(QBSmoothStreamingDownloader self, SvErrorInfo *errorOut);

/**
 * Resets the data on current chunk.
 * This step is mandatory whenever the downloader iterators are moved outside of downloader,
 * and the downloader is required to know that.
 *
 * @param[in] self      this object
 */
void
QBSmoothStreamingDownloaderResetCurChunk(QBSmoothStreamingDownloader self);

/**
 * Set previous bitrate. It will be helpful to choose first quality variant.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[in] bitrate           bitrate to set
 **/
void
QBSmoothStreamingDownloaderSetBitrate(QBSmoothStreamingDownloader self, int bitrate);

/**
 * Get actual out PTS.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @return                      out PTS
 **/
uint64_t
QBSmoothStreamingDownloaderGetOutPTS(QBSmoothStreamingDownloader self);

/**
 * Check whether downloading is paused.
 *
 * Downloading can be paused because of full buffer.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @return                      @c true if downloading is paused, otherwise @c false
 **/
bool
QBSmoothStreamingDownloaderIsPaused(QBSmoothStreamingDownloader self);

/**
 * Check whether downloading is enabled.
 *
 * Downloading can be active or inactive.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @return                      @c true if downloading is active, otherwise @c false
 **/
bool
QBSmoothStreamingDownloaderIsEnabled(QBSmoothStreamingDownloader self);

/**
 * Enable or disable downloader.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @param[in] enabled           $c true to active downloader, @c false to deactivate downloader
 */
void
QBSmoothStreamingDownloaderSetEnabled(QBSmoothStreamingDownloader self, bool enabled);


/**
 * Get avarage download bitrate.
 *
 * @param[in] self              Smooth Streaming downloader handle
 * @return                      avarage bitrate
 **/
int64_t
QBSmoothStreamingDownloaderGetAvaregeBitrate(QBSmoothStreamingDownloader self);

/**
 * @}
 **/

#endif /* QBSMOOTHSTREAMINGDOWNLOADER_H_ */
