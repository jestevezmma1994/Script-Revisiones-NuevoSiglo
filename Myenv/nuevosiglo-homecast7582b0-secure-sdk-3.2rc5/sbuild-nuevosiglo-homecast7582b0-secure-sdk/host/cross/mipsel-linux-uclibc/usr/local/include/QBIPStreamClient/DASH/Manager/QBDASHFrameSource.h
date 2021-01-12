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

#ifndef QBDASHFRAMESOURCE_H
#define QBDASHFRAMESOURCE_H

/**
 * @file QBDASHFrameSource.h MPEG-DASH Frame Source
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/DASH/MPDAdaptors/QBDASHStream.h>
#include <QBIPStreamClient/Common/QBIPStreamBitrateManager.h>
#include <QBIPStreamClient/DASH/Logic/QBDASHHTTPErrorsLogic.h>

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/SvBufMan.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

#include <unistd.h>
#include <stdbool.h>

/**
 * @defgroup QBDASHFrameSource MPEG-DASH Frame Source
 * @ingroup QBIPStreamDASHSupport
 * @{
 * High-level representation of MPEG-DASH Frame Source
 *
 **/

/**
 * QBDASHFrameSource.
 * @class QBDASHFrameSource
 * @extends SvObject
 */
typedef struct QBDASHFrameSource_* QBDASHFrameSource;

/**
 * QBDASHFrameSourceErrorType
 * Specifies types of QBDASHFrameSource's errors
 */
typedef enum {
    QBDASHFrameSourceErrorType_initSegmentDownload,   //!< error on downloading initialization segment
    QBDASHFrameSourceErrorType_segmentDownload,       //!< error on downloading segment
    QBDASHFrameSourceErrorType_mp4Parser              //!< mp4 parser error
} QBDASHFrameSourceErrorType;

/**
 * QBDASHFrameSourceState.
 * Specifies current state of MPEG-DASH frame source.
 */
typedef enum {
    QBDASHFrameSourceState_unknown = 0,     //!< unknown state
    QBDASHFrameSourceState_initialized,     //!< initialized
    QBDASHFrameSourceState_working,         //!< working
    QBDASHFrameSourceState_paused,          //!< paused
    QBDASHFrameSourceState_finished,        //!< segment finished
    QBDASHFrameSourceState_stopped,         //!< stopped
    QBDASHFrameSourceState_restart,         //!< need to be restarted
} QBDASHFrameSourceState;

/**
 * QBDASHFrameSource constructor parameters
 * @struct QBDASHFrameSourceParams
 */
typedef struct QBDASHFrameSourceParams {
    SvBufMan bufMan;                            //!< buffer manager
    SvScheduler scheduler;                      //!< scheduler
    SvHTTPClientEngine httpClientEngine;        //!< HTTP client engine
    int curlBufferSize;                         //!< curl buffer size
    QBDASHHTTPErrorsLogic httpErrorLogic;       ///< optional: DASH HTTP error logic
    bool useBitrateManager;                     //!< bitrate manager control
} QBDASHFrameSourceParams;

/**
 * Create an instance of QBDASHFrameSource class.
 *
 * @param[in] params     parameters needed to construct QBDASHFrameSource
 * @param[out] errorOut  errorInfo
 * @return created QBDASHFrameSource object
 */
QBDASHFrameSource
QBDASHFrameSourceCreate(QBDASHFrameSourceParams* params, SvErrorInfo* errorOut);

/**
 * Setup the FrameSource object for the next operations.
 *
 * @param[in] self                  frame source handle
 * @param[in] stream                QBDASHStream handle
 * @param[out] errorOut             errorInfo
 * @return @c true if the operation was successful @c false if failed
 */
bool
QBDASHFrameSourceSetup(QBDASHFrameSource self, QBDASHStream stream, SvErrorInfo* errorOut);

/**
 * Return type of the stream handled by the FrameSource.
 *
 * @param[in] self frame source handle
 * @return Adaptation set type of the handled stream
 */
QBDASHAdaptationSetType
QBDASHFrameSourceGetStreamType(QBDASHFrameSource self);

/**
 * Return current state of the FrameSource.
 *
 * @param[in] self                  frame source handle
 * @return current state
 */
QBDASHFrameSourceState
QBDASHFrameSourceGetState(QBDASHFrameSource self);

/**
 * Add listener of FrameSource.
 *
 * @param[in] self       frame source handle
 * @param[in] listener   listener to be added
 * @param[out] errorOut  errorInfo
 */
void
QBDASHFrameSourceAddListener(QBDASHFrameSource self, SvObject listener, SvErrorInfo* errorOut);

/**
 * Remove listener of FrameSource.
 *
 * @param[in] self      frame source handle
 * @param[in] listener  listener to be added
 */
void
QBDASHFrameSourceRemoveListener(QBDASHFrameSource self, SvObject listener);

/**
 * Start downloading and parsing next segment.
 *
 * @param[in] self          frame source handle
 * @param[out] segmentOut   started DASH segment, in case of init segment reports @c NULL
 * @param[out] errorOut     errorInfo
 * @return @c true if the operation was successful @c false if failed
 */
bool
QBDASHFrameSourceStart(QBDASHFrameSource self, QBDASHSegment* segmentOut, SvErrorInfo* errorOut);

/**
 * Finish work.
 *
 * @param[in] self frame source handle
 * @param[out] errorOut  errorInfo
 * @return @c true if the operation was successful @c false if failed
 */
bool
QBDASHFrameSourceStop(QBDASHFrameSource self, SvErrorInfo* errorOut);

/**
 * Get internal bitrate manager used by corresponding downloader.
 *
 * @param[in] self          frame source handle
 * @return                  internal bitrate manager
 */
QBIPStreamBitrateManager
QBDASHFrameSourceGetBitrateManager(QBDASHFrameSource self);

/**
 * @}
 */

#endif // QBDASHFRAMESOURCE_H
