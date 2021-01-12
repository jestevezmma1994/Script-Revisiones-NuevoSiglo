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

#ifndef QBIPSTREAMCLIENT_QBIPSTREAMDOWNLOADER_H_
#define QBIPSTREAMCLIENT_QBIPSTREAMDOWNLOADER_H_

/**
 * @file QBIPStreamDownloader.h Stream downloader adapter
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/SvBufMan.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <unistd.h>
#include <QBIPStreamClient/Common/QBIPStreamBitrateManager.h>

/**
 * @defgroup QBIPStreamDownloader Stream downloader
 * @{
 * Stream downloader
 **/

/**
 * QBIPStreamDownloader.
 * @class QBIPStreamDownloader
 * @extends SvObject
 */
typedef struct QBIPStreamDownloader_ *QBIPStreamDownloader;

/**
 * QBIPStreamDownloader constructor parameters
 * @struct QBIPStreamDownloaderParams
 */
typedef struct QBIPStreamDownloaderParams {
    SvBufMan bufMan;                     //!< buffer manager
    SvScheduler scheduler;               //!< scheduler
    SvHTTPClientEngine httpClientEngine; //!< HTTP client engine
    int curlBufferSize;                  //!< curl buffer size
} QBIPStreamDownloaderParams;

/**
 * Possible statuses returned from QBIPStreamDownloader functions.
 * @enum QBIPStreamDownloaderStatus
 */
typedef enum QBIPStreamDownloaderStatus {
    QBIPStreamDownloaderStatus_reserved,    //!< reserved status
    QBIPStreamDownloaderStatus_ok,          //!< success
    QBIPStreamDownloaderStatus_error,       //!< error
} QBIPStreamDownloaderStatus;

/**
 * State of QBIPStreamDownloader
 * @enum QBIPStreamDownloaderState
 */
typedef enum QBIPStreamDownloaderState {
    QBIPStreamDownloaderState_unknown,     //!< unknown state
    QBIPStreamDownloaderState_initial,     //!< initial state
    QBIPStreamDownloaderState_downloading, //!< downloading
    QBIPStreamDownloaderState_paused,      //!< download is paused
    QBIPStreamDownloaderState_finished,    //!< downloading is finished
    QBIPStreamDownloaderState_rollback,    //!< bandwidth is too low, lower quality is required
} QBIPStreamDownloaderState;

/**
 * Possible error types in QBIPStreamDownloader
 */
typedef enum QBIPStreamDownloaderError {
    QBIPStreamDownloaderError_fatal,    //!< Fatal error
} QBIPStreamDownloaderError;

/**
 * Set URL to download.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[in] url URL to be set
 * @param[out] errorOut error information
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderSetURL(QBIPStreamDownloader self, SvURL url, SvErrorInfo *errorOut);

/**
 * Get buffered bytes range
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[out] curr current position
 * @param[out] start buffer start position
 * @param[out] end buffer end position
 * @param[out] errorOut error information
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderGetByteRange(QBIPStreamDownloader self, int64_t *curr, int64_t *start,
                                 int64_t *end, SvErrorInfo *errorOut);

/**
 * Start downloading.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[out] errorOut error information
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderStart(QBIPStreamDownloader self, SvErrorInfo *errorOut);

/**
 * Suspend downloading.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[out] errorOut error information
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderSuspend(QBIPStreamDownloader self, SvErrorInfo *errorOut);

/**
 * Resume downloading.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[out] errorOut error information
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderResume(QBIPStreamDownloader self, SvErrorInfo *errorOut);

/**
 * Cancel downloading.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[out] errorOut error information
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderCancel(QBIPStreamDownloader self, SvErrorInfo *errorOut);

/**
 * Get HTTP error code returned by server.
 *
 * @param[in]  self     handle to an instance of QBIPStreamDownloader
 * @param[out] errorOut error info
 * @return              HTTP error code, @c 0 if irrelevant
 **/
int
QBIPStreamDownloaderGetHTTPErrorCode(QBIPStreamDownloader self,
                                     SvErrorInfo *errorOut);

/**
 * Get downloader state.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @return downloader state
 */
QBIPStreamDownloaderState
QBIPStreamDownloaderGetState(QBIPStreamDownloader self);

/**
 * Add downloader listener.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[in] listener listener to be added
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderAddListener(QBIPStreamDownloader self, SvObject listener);

/**
 * Remove downloader listener.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[in] listener listener to be removed
 * @return call status
 */
QBIPStreamDownloaderStatus
QBIPStreamDownloaderRemoveListener(QBIPStreamDownloader self, SvObject listener);

/**
 * Create final URL (with resolved redirections) of resource being downloaded.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[out] errorOut error information
 * @return final url object or @c NULL on error
 */
SvURL
QBIPStreamDownloaderCreateFinalURL(QBIPStreamDownloader self, SvErrorInfo *errorOut);

/**
 * Get assigned bitrate manager.
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @return bitrate manager, may be @c NULL
 */
QBIPStreamBitrateManager
QBIPStreamDownloaderGetBitrateManager(QBIPStreamDownloader self);

/**
 * Assign bitrate manager.
 *
 * @param[in] self handle to an instance of QBIPStreamDownloader
 * @param[in] bitrateManager bitrate manager handle
 */
void
QBIPStreamDownloaderSetBitrateManager(QBIPStreamDownloader self, QBIPStreamBitrateManager bitrateManager);

/**
 * Get runtime type identification object representing QBIPStreamDownloader class.
 *
 * @relates QBIPStreamDownloader
 *
 * @return QBIPStreamDownloder runtime type identification object
 **/
SvType
QBIPStreamDownloader_getType(void);

/**
 * Create downloader.
 *
 * @memberof QBIPStreamDownloader
 *
 * @param[in] params downloader constructor parameters
 * @param[out] errorOut error information
 * @return created downloader instance or @c NULL in case of error
 */
QBIPStreamDownloader
QBIPStreamDownloaderCreate(QBIPStreamDownloaderParams *params, SvErrorInfo *errorOut);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBIPSTREAMDOWNLOADER_H_ */
