/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_DECODER_BASE_H
#define QB_DECODER_BASE_H

#include <QBDecoder.h>
#include "settings.h"

#include <QBViewport.h>
#include <QBDemux.h>
#include <QBTSSWDescrambler.h>
#include <dataformat/sv_data_format.h>
#include <SvPlayerKit/SvChbuf.h>
#include <fibers/c/fibers.h>
#include <QBDecoder/utils/QBXDSPacketParser.h>

#include <QBCAS.h>

#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBDecoderManager_s;

/**
 * Type of video stream.
 */
typedef enum {
    QBDecoderSourceType_uninitialized = 0, //!< for compatybility with svdatasourcetype
    QBDecoderSourceType_tuner,             //!< video from tuner
    QBDecoderSourceType_udp,               //!< video from udp source (but not live stream)
    QBDecoderSourceType_multicast,         //!< video from multicast source (live stream)
    QBDecoderSourceType_pvr,               //!< video from pvr source
    QBDecoderSourceType_smooth,            //!< smooth streaming
    QBDecoderSourceType_other,             //!< other, unspecified source source type
    QBDecoderSourceType_cnt,               //!< count of possible enum values
} QBDecoderSourceType;

struct QBDecoderBase_s
{
    int id; /// debug only

    struct {
        FormatUpdatedCallback callback;
        void* owner;
    } *formatCallbacks;

    size_t formatCallbacksCount;
    size_t formatCallbacksAllocated;


    QBDecoderCreateSettings  createSettings;
    QBDecoderDataMode  dataMode;
    QBDecoderVideoDecodingSettings videoDecodingSettings;
    QBDecoderSourceType sourceType;
    QBDecoderSTCMode stcMode;

    struct QBDecoderManager_s* man;

    bool isTs;
    bool isAsf;
    bool isEs;
    bool isDirectAudio;
    int  videoOutputCnt;
    int  audioOutputCnt;

    struct {
        struct {
            int nalStartcodeLen; /// 1,2,3,4 - stored here from h264 header
        } h264;

        struct {
            bool addHeaders; /// add adts headers to each audio frame
            int profile;
            int freqIndex;
            int channels;
        } adts;

        struct {
            unsigned char videoStreamId; /// video pes-packets will have this stream-id
            unsigned char audioStreamId; /// audio pes-packets will have this stream-id
        } pes;

        struct {
            uint8_t* format; /// prepared for pushing inside PES
            int formatLen;
            bool addToEveryPes;
        } videoHeader;
    } es;

    struct {
        int aheadMs;
        int aheadMaxMs;
        int delayMaxMs;
        int pushMaxMs;
    } sync;

    ///--------------------------------------------------
    /// Current settings.
    struct {
        int nom;
        int denom;
        bool isPaused;
        bool isTrick;
        bool isStopped;
        bool needsStc;
    } speed;


    ///--------------------------------------------------
    /// Protects wanted settings.
    pthread_mutex_t  cmdMutex;
    bool blockCommands;

    struct QBDecoderOutputSettings  curr;
    struct QBDecoderOutputSettings  pending; /// used only during SetupAsync() and UpdateAsync(), format is the same as in "curr", so not release/retain
    struct QBDecoderWantedSettings  wanted;

    ///--------------------------------------------------
    /// Mutex for the cached stats.
    pthread_mutex_t  statsMutex;

    int64_t  stcOffset;
    int64_t  pcrFirst;
    int64_t  pcrLast;
    int64_t  pcrLastReal; // even when it should be dropped/ignored

    struct {
        bool     ptsUsedInsteadOfPcr;
        int64_t  first;
        int64_t  last;
        bool     hasFirstPtsTime; /// for crude detection if PCRs are missing in the stream
        SvTime   firstPtsTime;
    } pts;

    ///--------------------------------------------------
    /// Async status.
    int asyncError;

    /// CAS status
    struct {
        unsigned short systemId;
        bool systemIdValid;
        /// getInfo command
        QBCASCmd getInfoCmd;
        QBCASCmd startDescCmd;
    } CA;

    ///--------------------------------------------------
    /// Descrambler.
    struct {
        int inProgressCnt;
        bool  lastFailed;
        bool  firstPresent;
        bool  firstHandled;
        bool  needsFlush;
        bool  videoCwReceived;
        int8_t sessionId; /// -1 if uninitialized
    } ecm;

    QBTSSWDescrambler* sw_descrambler;

    ///--------------------------------------------------
    /// Demux for data pids.
    struct {
        QBDemuxChannel* multiChannel;
        SvBuf  sb;
        SvFiber  fiber;
        SvFiberTimer  timer;
        bool needsFlush;
    } demux;

    ///--------------------------------------------------
    /// Closed Captions
    struct {
        SvFiber fiber;
        SvFiberTimer timer;
    } closedCaption;

    ///--------------------------------------------------
    /// Info.
    bool  hasVideoInfo;
    QBViewportVideoInfo  videoInfo;

    ///--------------------------------------------------
    /// XDS Packets Parser
    QBXDSPacketParser xdsPacketParser;

    ///--------------------------------------------------
    /// Debug.
    struct {
        int receivedECMs;
        int processedECMs;
        int receivedPCRs;
        int64_t receivedDataBytes;
        bool eosWanted;
    } debug;

    unsigned int continouslyReceivedNotScrambledVideoTSPacketsWhileCAPresent;

    ///--------------------------------------------------
    /// getInfo command.
    QBCASCmd getInfoCmd;

    /// playback discontinuity since start of playback
    int64_t playbackDiscontinuityInMs;
    /// playback discontinuity counter
    int64_t playbackDiscontinuityCounter;
};
typedef struct QBDecoderBase_s  QBDecoderBase;


/// Call in Create(), after callocating your structure
extern
void QBDecoderBaseInit(QBDecoderBase* base,
                       const QBDecoderCreateSettings* createSettings);

/// Call in Create(), after doing all initialization
extern int QBDecoderBaseInitFinish(QBDecoderBase* base);

/// Call in Close(), before free'ing the structure
extern void QBDecoderBaseClean(QBDecoderBase* base);

/// Call in Setup() or Update(), after initializing decoders
extern int QBDecoderBasePushPrivateData(QBDecoderBase* base, bool isVideo);

extern int QBDecoderBaseFlushDecryption(QBDecoderBase* base);
extern int QBDecoderBaseUpdateDecryptionChannel(QBDecoderBase* base, bool enable, int pid, void* channel);

/// Call asynchronously during playback, to notify about decoded/displayed frames.
/// Base can use this info in its sync heuristicts, if the pcr info is unavailable.
extern void QBDecoderBaseReportPts(QBDecoderBase* base, uint64_t pts);

/// debug only
extern int64_t QBDecoderBaseGetSTCEx(QBDecoderBase* base);

/// Call QBPlatformUpdateVideoViewportInput on all output, based on
/// informartion kept in QBDecoderBase
extern void QBDecoderUpdatePlatformViewports(QBDecoderBase* base);

/// Call QBDecoderDesrambledContentError to notify that
/// status of descrambled content. This should be call only if content
/// status has been changed and descrambler keys are valid
extern void QBDecoderDescrambledContentStatus(QBDecoderBase* base, bool isContentValid);

#ifdef __cplusplus
}
#endif


#endif // #ifndef QB_DECODER_BASE_H
