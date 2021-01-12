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

#ifndef QBVIEWPORT_H
#define QBVIEWPORT_H

/**
 * @file QBViewport.h A/V decoder viewport library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvCore/SvErrorInfo.h>
#include <CAGE/Core/Sv2DRect.h>
#include <QBPlatformHAL/QBPlatformViewport.h>
#include <dataformat/audio.h>
#include <SvFoundation/SvHashTable.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBViewport QBViewport: viewport class
 * @ingroup MediaPlayer
 * @{
 *
 * QBViewport object represents a control point for media output from A/V decoder engine instance.
 * It preserves and applies permanent and per-content playback settings.
 *
 * Underlying decoder can connect to QBViewport object and grant control to:
 * - A/V tracks,
 * - audio volume,
 * - video viewport mode, dimensions and position,
 * - etc.
 *
 * All requested changes are delivered and applied to decoder asynchronously.
 * Output-dependent options (volume, video window, etc.) are kept permanently between content-changes.
 * Content-dependent options (a/v tracks) are only kept until current content playback is shutdown.
 */

// forward only
struct svdataformat;

// opaque
struct QBViewport_s;

/**
 * Viewport class.
 *
 * @note QBViewport is @b not an SvObject.
 */
typedef struct QBViewport_s* QBViewport;

/**
 * Status of underlying video decoder.
 */
typedef struct QBViewportVideoInfo_s
{
  /// width of the video content (in pixels)
  int width;
  /// height of the video content (in pixels)
  int height;
  /// aspect ratio of the video content
  QBAspectRatio aspectRatio;
  /// @c true if the video is interlaced
  bool interlaced;
  /// number of frames per seconds multiplied by @c 1000 (FPS * 1000)
  int frameRate;
  //more ?
} QBViewportVideoInfo;

/**
 * QBViewport application callback methods.
 *
 * Callbacks will always be called from some internal fiber, never during any QBViewport API call.
 * All callbacks here are reentrant (during a callback, you can call any method on the QBViewport object, including destruction).
 */
struct qb_viewport_callbacks
{
  /**
   * After receiving this callback, you can call QBViewportGetOutputFormat(), which will
   * return non-null format when something is played, and @c NULL when nothing is played
   * (disconnected from data stream.)
   *
   * @param[in] owner opaque pointer to callbacks' owner
   * @param[in] qbv  viewport handle
   */
  void (*output_format_changed) (void* owner, QBViewport qbv);

  /**
   * Called when video info has changed, so QBViewportGetVideoInfo() will return
   * different information than previously.
   *
   * @param[in] owner opaque pointer to callbacks' owner
   * @param[in] qbv viewport handle
   * @param[in] videoInfo information about video content from decoder
   */
  void (*video_info_changed) (void* owner, QBViewport qbv, const QBViewportVideoInfo *videoInfo);

  /**
   * Called when number of entities blocking video was changed.
   * To get an information which entities block video call QBViewportBlockVideoGetBlockers()
   *
   * @param[in] owner opaque pointer to callbacks' owner
   * @param[in] qbv viewport handle
   */
  void (*video_blocking_changed) (void* owner, QBViewport qbv);
};


/**
 * Get viewport instance.
 *
 * @return viewport instance
 */
QBViewport QBViewportGet(void);

/**
 * Register callbacks.
 *
 * @param[in] qbv  viewport handle
 * @param[in] callbacks  callbacks
 * @param[in] owner opaque pointer to @a callbacks' owner
 */
void QBViewportAddCallbacks(QBViewport qbv, const struct qb_viewport_callbacks* callbacks, void* owner);

/**
 * Unregister callbacks that were registered before.
 *
 * @param[in] qbv  viewport handle
 * @param[in] callbacks  callbacks
 * @param[in] owner opaque pointer to @a callbacks' owner
 */
void QBViewportRemoveCallbacks(QBViewport qbv, const struct qb_viewport_callbacks* callbacks, void* owner);


/**
 * Return format of content that is currently playing on this viewport.
 *
 * @param[in] qbv  viewport handle
 * @return non-retained format or @c NULL iff not playing or unknown yet
 */
struct svdataformat* QBViewportGetOutputFormat(QBViewport qbv);

/**
 * Check if the viewport is busy still processing given commands, and applying some settings.
 *
 * To be used when we want to make sure some important settings have to be applied before proceeding with some other things.
 *
 * Example is when we have to disable video decoding before we can change video resolution on some architectures.
 *
 * @param[in] qbv  viewport handle
 * @return @c true if the viewport object is still applying given commands
 */
bool QBViewportHasPendingCommands(QBViewport qbv);

// -------- video -----------

/**
 * Change video track of the underlying content being played.
 *
 * Valid only when QBViewportGetOutputFormat() returns proper format.
 * This function is asynchronous w.r.t. underlying decoder.
 * This setting is lost when playback of current content is shut down.
 *
 * @param[in] qbv  viewport handle
 * @param[in] track  index of new video track, as seen on dataformat, or @c -1 to disable video
 * @param[out] errorOut error info
 * @return @c 0 on success, negative on error.
 */
int QBViewportSetVideoTrack(QBViewport qbv, int track, SvErrorInfo *errorOut);

/**
 * Get video track of the underlying content being played.
 *
 * @param[in] qbv  viewport handle
 * @return current track number or @c -1 when no track is present/chosen (or when not playing any content)
 */
int QBViewportGetVideoTrack(QBViewport qbv);

/**
 * Reason why video is blocked
 */
typedef enum QBViewportBlockVideoReason_e {
  QBViewportBlockVideoReason_unknown, /**< unknown reason of blocking */
  QBViewportBlockVideoReason_contentExpired, /**< content cannot be shown because is not longer valid */
  QBViewportBlockVideoReason_contentExpirationPossible, /**< content cannot be shown because is expiration time is set but current time is unknown */
  QBViewportBlockVideoReason_maturityRating, /**< content cannot be shown because of maturity rating */
} QBViewportBlockVideoReason;

/**
 * Block/unblock video
 *
 * This setting is permanent between content switches.
 * This function is asynchronous w.r.t. underlying decoder.
 *
 * @param[in] qbv viewport handle
 * @param[in] blockerName name of instance which want to block video
 * @param[in] reason why video is blocked, @see QBViewportBlockVideoReason
 * @param[in] block @c true to forbid video decoder, @c false to allow it
 */
void QBViewportBlockVideo(QBViewport qbv, const SvString blockerName, const QBViewportBlockVideoReason reason, const bool block);

/**
 * Get table with all entities which block video
 *
 * @param[in] qbv viewport handle
 * @return table with entities which blocked video
 */
SvHashTable QBViewportBlockVideoGetBlockers(QBViewport qbv);

/**
 * Change video position on the screen.
 *
 * This setting is permanent between content switches.
 * This function is asynchronous w.r.t. underlying decoder.
 *
 * @param[in] qbv  viewport handle
 * @param[in] mode  off/fullscreen/windowed/etc.
 * @param[in] rect  position on the scree, needed only in "windowed" mode, given in GFX coordinates
 * @return @c 0 on sucess, negative when parameters are invalid
 */
int QBViewportSetVideoWindow(QBViewport qbv, QBViewportMode mode, const Sv2DRect* rect);

/**
 * Get video position on the screen.
 *
 * @param[in] qbv  viewport handle
 * @param[in] mode  off/fullscreen/windowed/etc.
 * @param[in] rect  position on the scree, needed only in "windowed" mode, given in GFX coordinates
 */
void QBViewportGetVideoWindow(QBViewport qbv, QBViewportMode* mode, Sv2DRect* rect);

/**
 * Change video content mode for given output.
 *
 * This setting is permanent between content switches.
 * This function is asynchronous w.r.t. underlying decoder.
 *
 * @param[in] qbv  viewport handle
 * @param[in] outputID  QBPlatformHAL outputID, non-master outputs are ignored
 * @param[in] mode  content mode (e.g. letterbox, panscan, fullscreen)
 */
void QBViewportSetContentMode(QBViewport qbv, unsigned int outputID, QBContentDisplayMode mode);

/**
 * Get video content mode for given output.
 *
 * This setting is permanent between content switches.
 *
 * @param[in] qbv  viewport handle
 * @param[in] outputID  QBPlatformHAL outputID, non-master outputs are ignored
 * @return  content mode (e.g. letterbox, panscan, fullscreen)
 */
QBContentDisplayMode QBViewportGetContentMode(QBViewport qbv, unsigned int outputID);


// -------- audio -----------


/**
 * Change audio track of the underlying content being played.
 *
 * Valid only when QBViewportGetOutputFormat() returns proper format.
 * This function is asynchronous w.r.t. underlying decoder.
 * This setting is lost when playback of current content is shutdown.
 *
 * @param[in] qbv  viewport handle
 * @param[in] track  index of new audio track, as seen on dataformat
 * @param[out] errorOut error info
 * @return @c 0 on success, negative on error
 */
int QBViewportSetAudioTrack(QBViewport qbv, int track, SvErrorInfo *errorOut);

/**
 * Get audio track of the underlying content being played.
 *
 * @param[in] qbv  viewport handle
 * @return current track number or @c -1 when no track is present/chosen (or when not playing any content)
 */
int QBViewportGetAudioTrack(QBViewport qbv);

/**
 * Forbid/allow creating audio decoder.
 *
 * This setting is permanent between content switches.
 * This function is asynchronous w.r.t. underlying decoder.
 *
 * @param[in] qbv  viewport handle
 * @param[in] block @c true to forbid audio decoder, @c false to allow it
 */
void QBViewportBlockAudio(QBViewport qbv, bool block);

/// Max value for volume settings in QBViewport API.
#define QBVIEWPORT_VOL_MAX  256

/**
 * Change audio volume on this viewport instance, also chaning "mute" status to @c false.
 * This setting is permanent between content switches.
 *
 * This function is asynchronous w.r.t. underlying decoder.
 *
 * @param[in] qbv  viewport handle
 * @param[in] volume  audio volume value, from @c 0 (muted) to @c QBVIEWPORT_VOL_MAX (max volume)
 */
void QBViewportSetVolume(QBViewport qbv, int volume);

/**
 * Return current audio volume setting.
 *
 * This setting is permanent, even when nothing is playing.
 *
 * @param[in] qbv  viewport handle
 * @return audio volume, from @c 0 (muted) to @c QBVIEWPORT_VOL_MAX (max volume)
 */
int QBViewportGetVolume(QBViewport qbv);

/**
 * Mute/unmute audio on this viewport instance.
 *
 * This setting is permanent, even when nothing is playing.
 * This function is asynchronous w.r.t. underlying decoder.
 *
 * @param[in] qbv  viewport handle
 * @param[in] mute  @c true for muted, @c false for unmuted
 */
void QBViewportMuteVolume(QBViewport qbv, bool mute);

/**
 * Get 'mute' status of this viewport instance.
 *
 * @param[in] qbv  viewport handle
 * @return  @c true for muted, @c false for unmuted
 */
bool QBViewportGetVolumeMuted(QBViewport qbv);


// -------- stats -----------

/**
 * Get information about current video track.
 *
 * @param[in] qbv  viewport handle
 * @param[out] videoInfo  video info struct to be filled
 * @return @c 0 on success, negative when not available
 */
int QBViewportGetVideoInfo(QBViewport qbv, QBViewportVideoInfo* videoInfo);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // #ifndef QBVIEWPORT_H
