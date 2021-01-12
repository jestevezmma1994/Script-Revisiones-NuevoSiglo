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

#ifndef QB_DECODER_H
#define QB_DECODER_H

#include <stdbool.h>
#include <stdint.h>

#include <SvPlayerKit/SvClosedCaptionSubtitles.h>
#include <dataformat/QBContentSource.h>

struct SvBuf_s;
struct SvChbuf_s;
struct svdataformat;
struct QBViewport_s;

struct QBDecoder_s;
typedef struct QBDecoder_s  QBDecoder;


#ifdef __cplusplus
extern "C" {
#endif

struct QBDecoderCallbacks_s
{
  /// content cannot be decrypted (whatever the reason)
  void (*decryption_failed) (void* target, QBDecoder* dec, bool externalReason, int errorCode);
  /// content can be decrypted now (only called if "decryption_failed" was called previously)
  void (*decryption_ok)     (void* target, QBDecoder* dec);
  /// content needs to be decrypted iff \a needed is true
  void (*decryption_needed) (void* target, QBDecoder* dec, bool needed);

  /// passes filtered and decrypted packets from input TS (data packets are: teletext and various subtitles)
  void (*data_packets)      (void* target, QBDecoder* dec, const uint8_t* packets, uint32_t packetCnt);

  void (*closed_caption_packet)(void *target, QBDecoder* dec, const QBClosedCaptionData *data);

  void (*set_session_id) (void *target, QBDecoder* dec, int8_t sessionId);

  /// desrambling status
  void (*descrambling_status) (void* target, QBDecoder* dec, bool scramblingOk);
  /// no valid CA descriptor found in PMT
  void (*none_supported_caid_found) (void* target, QBDecoder* dec);

  /**
   * Callback returning decoding error details
   *
   * @param[in] target  listener handle
   * @param[in] dec     QBDecoder handle
   * @param[in] buf     string containing decoding error description along with stream status
   */
  void (*decoding_error) (void* target, QBDecoder* dec, const char* buf);

};
typedef struct QBDecoderCallbacks_s  QBDecoderCallbacks;


extern void QBDecoderCleanup(void);

/**
 * Indicates how video decoder should handle errors in video stream.
 */
typedef enum QBDecoderVideoErrorHandlingMode_e {
    QBDecoderVideoErrorHandlingMode_none,             //!< Show everything, no error handling.
    QBDecoderVideoErrorHandlingMode_partial,          //!< Show every frame, but try to fix it if it's broken
    QBDecoderVideoErrorHandlingMode_dropBrokenFrames, //!< If the broken frame has been detected do not show it.
    QBDecoderVideoErrorHandlingMode_heuristic,        //!< Use heuristic platform specific algorithm to detect broken fragments.
    QBDecoderVideoErrorHandlingMode_cnt               //!< count of possible enum values
} QBDecoderVideoErrorHandlingMode;

/**
 * STC updated mode.
 */
typedef enum {
    QBDecoderSTCMode_PCR = 0,   //!< let autoplayer set STC based on parsed PCRs
    QBDecoderSTCMode_autoPCR,   //!< let video decoder set STC based on parsed PCRs
    QBDecoderSTCMode_autoPTS,   //!< let video decoder set STC based on parsed PTSs
    QBDecoderSTCMode_cnt        //!< count of possible enum values
} QBDecoderSTCMode;

/**
 * Video decoding settings
 */
typedef struct QBDecoderVideoDecodingSettings_s {
    QBDecoderVideoErrorHandlingMode errorHandlingMode;  //!< Decoder error handling mode
    bool waitForCorrectIFrame;                          //!< specify if decoder should wait for correct IFrame before starting playback
} QBDecoderVideoDecodingSettings;

typedef struct QBDecoderCreateSettings_s  QBDecoderCreateSettings;
struct QBDecoderCreateSettings_s
{
    const struct svdataformat* format;
    QBContentSource source;
    struct QBViewport_s* viewport;

    QBDecoderVideoDecodingSettings videoDecodingSettings;
    QBDecoderSTCMode syncMode;

    const struct QBDecoderCallbacks_s* callbackFunctor;
    void* callbackTarget;
    int8_t playbackSessionId;
};

/** Creates handle. Starts initializing decoders.
 *  @returns non-null on success
 */
extern QBDecoder* QBDecoderCreate(const QBDecoderCreateSettings* createSettings);

/** closes decoders, frees associated structures.
 */
extern void QBDecoderDestroy(QBDecoder* dec);


/** check if the decoder is ready to be used
 *  \returns 0 when ready
 *  \returns WOULDBLOCK when still preparing
 *  \returns negative on fatal error
 */
extern int QBDecoderCheck(QBDecoder* dec);


/** Notify that speed of the stream is about to change.
 *  (negative slow playback, or slow playback on audio_only is not allowed.)
 *  MUST be called before any frames are pushed.
 *  Timestamps of frames being pushed are still to be expressed in presentation time.
 *  In case of slow-playback, timestamps are to be expressed as in speed=1
 *  (it is the job of the decoder to display with desired speed, then).
 *  It is the job of the caller to push frames in proper amount.
 *  Decoder should detect if changes of speed are making the stream display
 *  discontinuous and act accordingly.
 *  Decoder should detect if audio should be played/dropped/muted for each speed.
 *  speed = nom / denom
 *  denom > 0
 *  @returns negative on failure, else success.
 */
extern int QBDecoderPlay(QBDecoder* dec, int nom, int denom);

typedef struct QBDecoderDataMode_s  QBDecoderDataMode;
struct QBDecoderDataMode_s
{
    /// E.g. in live mode, where we can safely drop data, when waiting for an ECM to give us CW.
    /// As opposed to VOD, where we should not be dropping data.
    bool droppableData;
    /// E.g. in partially encrypted content before first ECM is present, we want to erase data to avoid partially decoded stream
    bool erasableData;
};

extern void QBDecoderGetDataMode(QBDecoder* dec, QBDecoderDataMode* dataMode);

extern void QBDecoderSetDataMode(QBDecoder* dec, const QBDecoderDataMode* dataMode);

/** Has to be used when expecting dicontinuity in the stream.
 *  Callers need to call proper vd_setspeed() next (even if it's the same speed).
 *  @returns negative on failure, else success.
 */
extern int QBDecoderStop(QBDecoder* dec);


typedef struct QBDecoderPacket_s  QBDecoderPacket;
struct QBDecoderPacket_s
{
  struct SvChbuf_s* cb; /// buffer ownership can be taken
  bool isVideo;   /// video or audio
  uint8_t streamId; /// e.g. audio/video track number
  int64_t pts;    /// timestamp (90kHz)
};

/** Push data packet to the decoder.
 *  Proper QBDecoderPlay() MUST be called before any packets are pushed.
 *  Proper QBDecoderCanPush() MUST be called first.
 *  @returns negative on failure, else success.
 */
extern int QBDecoderPushData(QBDecoder* dec, QBDecoderPacket packet);

/** Push meta packet.
 *  E.g. PMT update, PCR, ECM (or control word).
 *  Proper QBDecoderPlay() MUST be called before any meta is pushed.
 *  Proper QBDecoderCanPush() MUST be called first.
 *  @returns negative on failure, else success.
 */
extern int QBDecoderPushMeta(QBDecoder* dec, const struct SvBuf_s* sb);


/** @returns zero, if PushMeta() would block.
 *           negative on error, else success.
 *  @param sb the meta that wants to be pushed
 */
extern int QBDecoderCanPushMeta(QBDecoder* dec, const struct SvBuf_s* sb);

/** @returns zero, if PushData() would block.
 *           negative on error, else success.
 *  @param sb the data packet that wants to be pushed
 *  @param pts timestamp of the data frame in ES mode, or @c -1 otherwise
 */
extern int QBDecoderCanPushData(QBDecoder* dec, const struct SvBuf_s* sb, int64_t pts);

/** Check if decoder has already outputed all frames it had.
 *  This function changes internal state to "eos", so no more data can be pushed after this call.
 *  @returns non-zero if playback of stream has been finished.
 */
extern int QBDecoderIsEos(QBDecoder* dec);

#if 0
/** Check if decoder has already outputed all frames it had.
 *  This function does not change decoder's state in any way.
 *  @returns @c 0 iff still has some buffered data. @c 1 iff no more data is to be processed. negative on error.
 */
extern int QBDecoderIsEmpty(QBDecoder* dec);
#endif

typedef struct QBDecoderStats_s  QBDecoderStats;
struct QBDecoderStats_s
{
    /// pts of last frame outputed by the decoder (should be used to synchronize with external sources, like subtitles), -1 iff unknown yet
    int64_t  stc90k;
    /// how much of the content was played, using frame pts, pcr, frame count, or other means, with continuity fixups (should be used to display position)
    int64_t  durationPlayed;

    /// estimated duration of buffered data (still not played), -1 iff unknown yet
    int64_t  durationBuffered;

    /// playback discontinuity since start of playback
    int64_t  playbackDiscontinuityInMs;
    /// playback discontinuity counter
    int64_t  playbackDiscontinuityCounter;
};

/** @param[out]stats  to be filled by this function
 *  @returns zero on success, SV_ERR_WOULDBLOCK if unknown yet, else fatal error.
 */
extern int QBDecoderGetStats(QBDecoder *dec, QBDecoderStats *stats);

/** Format update callback type definition.
 *  @param[in] target pointer registered by addFormatUpdatesListener function
 *  @param[in] format new format
 */
typedef void (*FormatUpdatedCallback)(void* target, const struct svdataformat* format);

/** Decoder track controller interface. */
struct QBDecoderTrackControllerIface
{
  /** Set audio track by number.
   *  @param[in] self_ pointer to Decoder
   *  @param[in] track number of track to be set
   */
  void (*setAudioTrack)(void* self_, int track);

  /** Set video track by number.
   *  @param[in] self_ pointer to Decoder
   *  @param[in] track number of track to be set
   */
  void (*setVideoTrack)(void* self_, int track);

  /** Enable/disable PID in demux/descrambler.
   *  @param[in] self_ pointer to Decoder
   *  @param[in] pid PID to enable/disable
   *  @param[in] enable set to true if PID is about to be enabled, false otherwise
   */
  void (*setupPID)(void *self_, int pid, bool enable);

  /** Add format update listener
   *  @param[in] self_ pointer to Decoder
   *  @param[in] callback function called on format update
   *  @param[in] target pointer passed to callback function
   */
  void (*addFormatUpdatesListener)(void* self_, FormatUpdatedCallback callback, void* target);

  /** Remove format update listener
   *  @param[in] self_ pointer to Decoder
   *  @param[in] callback function called on format update
   *  @param[in] target pointer passed to callback function
   */
  void (*removeFormatUpdatesListener)(void* self_, FormatUpdatedCallback callback, void* target);
};

/** Get functions of track controller interface implemented by decoder
 *  @param[in] base pointer to Decoder
 *  @param[out] iface table of interface functions
 */
extern void
QBDecoderGetTrackController(QBDecoder* base,
                            struct QBDecoderTrackControllerIface* iface);

#ifdef __cplusplus
}
#endif

#endif // QB_DECODER_H
