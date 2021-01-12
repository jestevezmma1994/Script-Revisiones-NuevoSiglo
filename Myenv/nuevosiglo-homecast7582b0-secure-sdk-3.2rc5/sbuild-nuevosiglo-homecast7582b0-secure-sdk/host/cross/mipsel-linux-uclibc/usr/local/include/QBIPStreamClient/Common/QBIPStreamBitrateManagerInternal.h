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

#ifndef QB_IP_STREAM_BITRATE_MANAGER_INTERNAL_H
#define QB_IP_STREAM_BITRATE_MANAGER_INTERNAL_H

#include <QBIPStreamClient/Common/QBIPStreamBitrateKeeper.h>
#include <QBIPStreamClient/Common/QBIPStreamDownloader.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakReference.h>

struct QBIPStreamBitrateManager_ {
    struct SvObject_ super_;

    /// stores previous bitrate in file, used for setting up first bitrate
    QBIPStreamBitrateKeeper bitrateKeeper;

    /// implementation of QBIPStreamPlaybackInfoProviderIface
    SvWeakReference playbackInfoProvider;

    /// true when anyone of downloaders is active (is not paused), otherwise false
    bool activeTimer;
    /// time when unpaused
    SvTime startTimeOfLastSample;
    /// total operating time (when not paused).
    int totalDownloadTime;
    /// total bytes downloaded
    size_t totalDownloadBytes;

    /// number of past chunks to include in history
    int bitrateHistoryDepth;
    /// number of variants to keep history of
    int variantsCount;
    /// number of bitrates included in history per variant
    int *bitrateHistorySize;
    /// bitrate history
    int *bitrateHistory;
    /// success count
    unsigned *successCount;
    /// try count
    unsigned *tryCount;

    /// selected quality variant
    int qualityVariant;
    /// selected quality variant bitrate
    int qualityVariantBitrate;

    bool challengeMode;
    bool challengeResult;

    QBIPStreamDownloaderState downloaderState;
};

#endif /* QB_IP_STREAM_BITRATE_MANAGER_INTERNAL_H */
