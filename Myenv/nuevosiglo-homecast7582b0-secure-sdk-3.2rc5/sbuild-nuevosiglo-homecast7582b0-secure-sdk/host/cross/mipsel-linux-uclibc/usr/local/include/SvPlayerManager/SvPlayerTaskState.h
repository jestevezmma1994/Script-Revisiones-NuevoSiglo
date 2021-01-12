/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_PLAYER_TASK_STATE_H_
#define SV_PLAYER_TASK_STATE_H_

/**
 * @file SvPlayerTaskState.h Definition of the player task state
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerManager/SvPlayerTaskRestrictions.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup SvPlayerTask
 * @{
 **/

/**
 * Description of a player task state.
 **/
typedef struct {
    /// current playback speed
    double currentSpeed;
    /// last playback speed requested by user
    double wantedSpeed;
    /// current position of the played content
    double currentPosition;
    /// current timestamp of the played content, @c -1 if unknown
    int64_t currentStc90k;
    /// playback discontinuity since start of playback
    int64_t playbackDiscontinuityInMs;
    /// playback discontinuity counter
    int64_t playbackDiscontinuityCounter;

    /// content duration (can change over time), @c 0.0 if unknown
    double duration;

    /// end-of-stream reached
    bool eos;

    /// amount of buffered seconds of content
    double bufferedSec;

    /// amount of buffered content in bytes
    int32_t usedBytes;

    /// state of the timeshift recording
    struct {
        /// flag indicating whether timeshift recording is in progress
        short recording;
        /// flag indicating whether timeshift playback have been requested
        short wanted;
        /// flag indicating whether timeshift playback is currently active
        short enabled;
        /// start position of the available recorded part of the content
        double range_start;
        /// end position of the available recorded part of the content
        double range_end;
    } timeshift;

    /// current restrictions
    SvPlayerTaskRestrictions restrictions;
    /// session ID assocatied with playback, @c -1 if unknown
    int8_t playback_session_id;
    /// session ID assocatied with reencryption, @c -1 if unknown
    int8_t reencryption_session_id;
    /// average bitrate of all downloaded data: video, audio, subtitles, etc.
    /// it is measured only for streams which have more than one quality variant
    int64_t avarageBitrate;
    /// last selected bitrate
    int bitrate;
} SvPlayerTaskState;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
