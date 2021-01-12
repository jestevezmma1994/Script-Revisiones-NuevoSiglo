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

#ifndef QBSMOOTHSTREAMINGBITRATEMONITOR_H_
#define QBSMOOTHSTREAMINGBITRATEMONITOR_H_

/**
 * @file  QBSmoothStreamingBitrateMonitor.h
 * @brief Smooth Streaming bitrate monitor. Used to calculate real bitrate which
 * comes through smooth streaming.
 **/

#include <stdbool.h>
#include <SvFoundation/SvObject.h>
#include "QBSmoothStreamingDownloader_decl.h"

typedef struct QBSmoothStreamingBitrateMonitor_ *QBSmoothStreamingBitrateMonitor;

/**
 * Create Smooth Streaming bitrate monitor.
 *
 * @return                      new created Smooth Streaming bitrate monitor,
 *                              otherwise @c NULL in case of error
 **/
QBSmoothStreamingBitrateMonitor
QBSmoothStreamingBitrateMonitorCreate(void);

/**
 * Get current bitrate (bits per second) which is calculated from incoming bytes.
 *
 * @param[in] self              Smooth Streaming bitrate monitor handle
 * @return                      actual weighted bitrate (bits per second)
 **/
int
QBSmoothStreamingBitrateMonitorGetBitrate(QBSmoothStreamingBitrateMonitor self);

/**
 * Inform bitrate monitor about completing of downloading bytes.
 *
 * @param[in] self              Smooth Streaming bitrate monitor handle
 * @param[in] newBytes          number of just income bytes
 **/
void
QBSmoothStreamingBitrateMonitorAddBytes(QBSmoothStreamingBitrateMonitor self, unsigned newBytes);

/**
 * Add downloader to notify about pausing of downloading.
 *
 * @param[in] self              Smooth Streaming bitrate monitor handle
 * @param[in] downloader        QBSmoothStreamingDownloader handle
 **/
void
QBSmoothStreamingBitrateMonitorAddDownloader(QBSmoothStreamingBitrateMonitor self, QBSmoothStreamingDownloader downloader);

/**
 * Remove downloader to stop notify about pausing of downloading.
 *
 * @param[in] self              Smooth Streaming bitrate monitor handle
 * @param[in] downloader        QBSmoothStreamingDownloader handle
 **/
void
QBSmoothStreamingBitrateMonitorRemoveDownloader(QBSmoothStreamingBitrateMonitor self, QBSmoothStreamingDownloader downloader);

/**
 * Set pause or activate to bitrate monitor.
 *
 * If all downloaders will pause, bitrate monitor will pause counting the time.
 * This method can be used to activate counting the time.
 *
 * @param[in] self              Smooth Streaming bitrate monitor handle
 * @param[in] paused            @c true for pause, @c false for activate
 **/
void
QBSmoothStreamingBitrateMonitorSetPaused(QBSmoothStreamingBitrateMonitor self, bool paused);

#endif /* QBSMOOTHSTREAMINGBITRATEMONITOR_H_ */
