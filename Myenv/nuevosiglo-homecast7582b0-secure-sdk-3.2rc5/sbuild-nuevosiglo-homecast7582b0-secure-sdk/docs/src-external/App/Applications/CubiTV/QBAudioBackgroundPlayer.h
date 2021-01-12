/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_AUDIO_BACKGROUND_PLAYER_H_
#define QB_AUDIO_BACKGROUND_PLAYER_H_

/**
 * @file QBAudioBackgroundPlayer.h Audio background playback service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <main_decl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBAudioBackgroundPlayback_service Audio background playback service
 * @ingroup CubiTV_services
 * @{
 **/

typedef struct QBAudioBackgroundPlaybackService_ *QBAudioBackgroundPlaybackService;

/**
 * Creates new instance of Audio background playback service
 *
 * @param[in] appGlobals AppGlobals handle
 * @return               Handle of Audio background playback service
 */
QBAudioBackgroundPlaybackService
QBAudioBackgroundPlaybackServiceCreate(AppGlobals appGlobals);

/**
 * Play m3u playlist in background
 *
 * @param[in] self  Handle of Audio background playback service
 * @param[in] URI   Handle of playlist's URI
 */
void
QBAudioBackgroundPlaybackServicePlayM3uPlaylist(QBAudioBackgroundPlaybackService self, SvString URI);

/**
 * Play playlist in background
 *
 * @param[in] self     Handle of Audio background playback service
 * @param[in] playlist Handle of the playlist. Playlist must contain SvContent objects.
 */
void
QBAudioBackgroundPlaybackServicePlayPlaylist(QBAudioBackgroundPlaybackService self, SvArray playlist);

/**
 * Play single audio file in background
 *
 * @param[in] self  Handle of Audio background playback service
 * @param[in] URI   Handle of file's URI
 */
void
QBAudioBackgroundPlaybackServicePlayAudio(QBAudioBackgroundPlaybackService self, SvString URI);

/**
 * Stop background playback. No effect when nothing is played
 *
 * @param[in] self  Handle of Audio background playback service
 */
void
QBAudioBackgroundPlaybackServiceStopPlayback(QBAudioBackgroundPlaybackService self);

/**
 * Get status of the service
 *
 * @param[in] self  Handle of Audio background playback service
 * @return          true when music is played in background, false otherwise
 */
bool
QBAudioBackgroundPlaybackServiceIsPlaying(QBAudioBackgroundPlaybackService self);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
