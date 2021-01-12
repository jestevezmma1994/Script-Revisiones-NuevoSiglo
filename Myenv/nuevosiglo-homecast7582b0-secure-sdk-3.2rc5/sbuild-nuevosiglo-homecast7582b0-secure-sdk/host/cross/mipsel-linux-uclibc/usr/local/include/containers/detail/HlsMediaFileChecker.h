/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef HLSMEDIAFILECHECKER_H_
#define HLSMEDIAFILECHECKER_H_

/**
 * @file HlsMediaFileChecker.h
 * @brief Utility module used for checking file size given by URL
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvURL.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup HlsMediaFileChecker Checking file size given by URL
 * @ingroup containers
 *
 * Utility module used for checking file size given by URL.
 **/


/**
 * HLS media file checker.
 **/
typedef struct HlsMediaFileChecker_t* HlsMediaFileChecker;


/**
 * HlsMediaFileChecker callback.
 *
 * This callback returns remote file data that can be used by
 * adaptive bitrate algorithms.
 *
 * @param[out] prv              pointer to the module calling FileChecker
 * @param[out] nextFileSize     size of the tested file
 * @param[out] reqDurationMs    time taken by the request used to get file size
 * @param[out] httpErrorCode    error code returned by the request (can be 0)
 * @param[out] error            error info
 **/
typedef void (*HlsMediaFileCheckerCallbackFun)(void *prv,
                                               int64_t nextFileSize,
                                               unsigned int reqDurationMs,
                                               int httpErrorCode,
                                               SvErrorInfo error);

/**
 * Create HlsMediaFileChecker instance.
 *
 * @param[in] httpEngine                HTTP Client Engine
 * @param[in] restartTimeMs             delay in [ms] between retries in case of the request failure
 * @param[in] restartCyclesTimeoutMs    maximum retries time in [ms]
 * @return                              a pointer to new instance of HlsMediaFileChecker or @c NULL if
 *                                      the creation could not be performed
 **/
HlsMediaFileChecker
HlsMediaFileCheckerCreate(SvHTTPClientEngine httpEngine, int restartTimeMs, int restartCyclesTimeoutMs);


/**
 * Set HlsMediaFileChecker callback.
 *
 * @param[in] self          reference to a HlsMediaFileChecker
 * @param[in] callback      pointer to a callback function
 * @param[in] prv           pointer to a callback's private data
 **/
void
HlsMediaFileCheckerSetCallback(HlsMediaFileChecker self,
                               HlsMediaFileCheckerCallbackFun callback,
                               void *prv);

/**
 * Get size of the file pointed by URL.
 *
 * Warning: This function should be called only when the prevoius call has finished.
 *
 * @param[in] self          reference to a HlsMediaFileChecker
 * @param[in] file          url pointing the remote file
 **/
void
HlsMediaFileCheckerGetFileSize(HlsMediaFileChecker self, SvURL file);


/**
 * Function tells if the server reports file sizes.
 *
 * @param[in] self          reference to a HlsMediaFileChecker
 * @return                  true if the server reports file sizes, false if not
 **/
bool
HlsMediaFileCheckerIsFileSizeHandled(HlsMediaFileChecker self);

#ifdef __cplusplus
}
#endif

#endif /* HLSMEDIAFILECHECKER_H_ */
