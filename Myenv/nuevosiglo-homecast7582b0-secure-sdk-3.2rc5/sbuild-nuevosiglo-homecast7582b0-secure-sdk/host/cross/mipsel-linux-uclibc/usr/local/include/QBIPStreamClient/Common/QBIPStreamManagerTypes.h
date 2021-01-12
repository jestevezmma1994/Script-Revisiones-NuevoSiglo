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

#ifndef QBIPSTREAMMANAGERTYPES_H
#define QBIPSTREAMMANAGERTYPES_H

/**
 * @file QBIPStreamManagerTypes.h
 * @brief Common types definitions
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBIPStreamManagerTypes QBIPStreamManager's common types
 * @{
 **/

/**
 * QBIPStreamManagerErrorType
 * Specifies types of QBIPStreamManager's errors
 */
typedef enum {
    QBIPStreamManagerErrorType_sessionInitialization,       //!< error on session initializing
    QBIPStreamManagerErrorType_initializationChunkDownload, //!< error on downloading initialization chunk
    QBIPStreamManagerErrorType_nextChunkDownload,           //!< error on downloading next chunk
    QBIPStreamManagerErrorType_nextChunkChoosing,           //!< error on choosing next chunk to download
    QBIPStreamManagerErrorType_manifestManager,             //!< manifest manager error
    QBIPStreamManagerErrorType_newManifestHandling,         //!< error on handling new manifest
    QBIPStreamManagerErrorType_mp4ParserEngine,             //!< MP4 parser engine error
} QBIPStreamManagerErrorType;

/**
 * QBIPStreamManagerStreamType.
 * Specifies type of an IP stream.
 */
typedef enum {
    QBIPStreamManagerStreamType_audio = 0,      //!< audio stream type
    QBIPStreamManagerStreamType_video,          //!< video stream type
    QBIPStreamManagerStreamType_subtitles,      //!< subtitles stream type
    QBIPStreamManagerStreamType_cnt             //!< count of stream types
} QBIPStreamManagerStreamType;

/**
 * QBIPStreamManagerPlaybackState.
 * Specifies current state of IP stream playback.
 */
typedef enum {
    QBIPStreamManagerPlaybackState_unknown = 0,     //!< unknown playback state
    QBIPStreamManagerPlaybackState_paused,          //!< paused
    QBIPStreamManagerPlaybackState_playing,         //!< playing
    QBIPStreamManagerPlaybackState_discontinuity,   //!< discontinuity detected
    QBIPStreamManagerPlaybackState_EOS,             //!< end of stream detected
} QBIPStreamManagerPlaybackState;

/**
 * @}
 **/

#endif // QBIPSTREAMMANAGERTYPES_H
