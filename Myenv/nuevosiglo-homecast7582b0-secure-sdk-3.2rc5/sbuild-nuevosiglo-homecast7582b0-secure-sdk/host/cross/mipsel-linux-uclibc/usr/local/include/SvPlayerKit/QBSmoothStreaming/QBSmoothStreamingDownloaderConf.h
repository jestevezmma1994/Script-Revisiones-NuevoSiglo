/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_SMOOTH_STREAMING_DOWNLOADER_CONF_H_
#define QB_SMOOTH_STREAMING_DOWNLOADER_CONF_H_

/**
 * @file QBSmoothStreamingDownloaderConf.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvPlayerKit/QBSmoothStreaming/QBVariantCalculator/QBBitrateVariantCalculatorConf.h>
#include <SvPlayerKit/QBSmoothStreaming/QBVariantCalculator/QBBufferDurationVariantCalculatorConf.h>

#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>
/**
 * @defgroup QBSmoothStreamingDownloaderConf QBSmoothStreamingDownloaderConf class
 * @ingroup CubiTV
 * @{
 **/

/**
 * QBSmoothStreamingDownloaderConf class.
 *
 * @class QBSmoothStreamingDownloaderConf
 * @extends SvObject
 **/
typedef struct QBSmoothStreamingDownloaderConf_ *QBSmoothStreamingDownloaderConf;

/**
 * @brief Indicates which variant calculator to use.
 */
typedef enum {
    QBSmoothStreamingDownloaderConfVariantCalculator_bitrate,          ///< Use bitrate variant calculator
    QBSmoothStreamingDownloaderConfVariantCalculator_bufferDuration    ///< Use bufferDuration variant calculator
} QBSmoothStreamingDownloaderConfVariantCalculator;

/**
 * @brief Indicates how player should handle errors occurred while downloading chunks of smooth streaming content.
 **/
typedef enum {
    QBSmoothStreamingDownloaderConfDownloadErrorHandlingPolicy_default,  ///< Default errors handling policy
    QBSmoothStreamingDownloaderConfDownloadErrorHandlingPolicy_skip      ///< Use default errors handling policy then try to download next chunk
} QBSmoothStreamingDownloaderConfDownloadErrorHandlingPolicy;

/**
 * Get runtime type identification object representing the QBSmoothStreamingDownloaderConf class.
 *
 * @relates QBSmoothStreamingDownloaderConf
 *
 * @return QBSmoothStreamingDownloaderConf type identification object
 **/
SvType
QBSmoothStreamingDownloaderConf_getType(void);

/**
 * Create QBSmoothStreamingDownloaderConf.
 *
 * @param[out] errorOut error info
 * @return created QBSmoothStreamingDownloaderConf, @c NULL in case of error
 **/
QBSmoothStreamingDownloaderConf
QBSmoothStreamingDownloaderConfCreate(SvErrorInfo *errorOut);

/**
 * Create QBSmoothStreamingDownloaderConf.
 *
 * @param[in]  settings dictionary with SmoothStreamingDownloader configuration
 * @param[out] errorOut error info
 * @return created QBSmoothStreamingDownloaderConf, @c NULL in case of error
 **/
QBSmoothStreamingDownloaderConf
QBSmoothStreamingDownloaderConfCreateFromSettings(SvHashTable settings, SvErrorInfo *errorOut);

/**
 * Check whether http pipelining is enabled.
 *
 * @param[in] self instance of QBSmoothStreamingDownloaderConf
 * @return if pipelining is enabled
 */
bool
QBSmoothStreamingDownloaderConfGetPipelining(QBSmoothStreamingDownloaderConf self);

/**
 * Get errors handling policy occurred while downloading chunks of smooth streaming content.
 *
 * @param[in] self  instance of QBSmoothStreamingDownloaderConf
 * @return errors handling policy occurred while downloading chunks
 **/
QBSmoothStreamingDownloaderConfDownloadErrorHandlingPolicy
QBSmoothStreamingDownloaderConfGetChunkDownloadErrorHandlingPolicy(QBSmoothStreamingDownloaderConf self);

/**
 * Get type of variant calculator
 *
 * @param[in] self  instance of QBSmoothStreamingDownloaderConf
 * @return variant calculator type
 */
QBSmoothStreamingDownloaderConfVariantCalculator
QBSmoothStreamingDownloaderConfGetVariantCalculator(QBSmoothStreamingDownloaderConf self);

/**
 * Get configuration for bitrate variant calculator
 *
 * @param[in] self  instance of QBSmoothStreamingDownloaderConf
 * @return configuration of bitrate variant calculator
 */
QBBitrateVariantCalculatorConf
QBSmoothStreamingDownloaderConfGetBitrateVariantCalculatorConf(QBSmoothStreamingDownloaderConf self);

/**
 * Get configuration for bufferDuration variant calculator
 *
 * @param[in] self  instance of QBSmoothStreamingDownloaderConf
 * @return configuration of bufferDuration variant calculator
 */
QBBufferDurationVariantCalculatorConf
QBSmoothStreamingDownloaderConfGetBufferDurationVariantCalculatorConf(QBSmoothStreamingDownloaderConf self);

/**
 * @}
 **/

#endif // QB_SMOOTH_STREAMING_DOWNLOADER_CONF_H_
