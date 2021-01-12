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

#ifndef SV_HTTP_BASIC_LISTENER_H_
#define SV_HTTP_BASIC_LISTENER_H_

/**
 * @file SvHTTPBasicListener.h
 * @brief HTTP client basic listener class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <sys/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBOutputStream.h>

/**
 * @defgroup SvHTTPBasicListener HTTP client basic listener class
 * @ingroup SvHTTPClient
 * @{
 *
 * A utility class implementing the @ref SvHTTPClientListener, that writes
 * all received data to an output stream.
 *
 * This class can be used as an HTTP client listener for typical
 * HTTP GET file downloads.
 *
 * @image html SvHTTPBasicListener.png
 **/


/**
 * HTTP client basic listener class.
 * @class SvHTTPBasicListener
 * @extends SvObject
 **/
typedef struct SvHTTPBasicListener_ *SvHTTPBasicListener;

/**
 * Callback function type for HTTP requests.
 *
 * @param[in] prv          opaque data pointer bound to the request
 * @param[in] filePath     path to the downloaded file
 * @param[in] error        error info (@c NULL on success)
 **/
typedef void (*SvHTTPBasicListenerCallback)(void *prv,
                                            SvString filePath,
                                            SvErrorInfo error);


/**
 * Get runtime type identification object representing
 * type of HTTP client basic listener class.
 *
 * @return HTTP client basic listener class
 **/
extern SvType
SvHTTPBasicListener_getType(void);

/**
 * Initialize HTTP client basic listener object.
 *
 * @memberof SvHTTPBasicListener
 *
 * @param[in] self         HTTP client basic listener handle
 * @param[in] filePath     destination file path
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvHTTPBasicListener
SvHTTPBasicListenerInit(SvHTTPBasicListener self,
                        const char *filePath,
                        SvErrorInfo *errorOut);

/**
 * Initialize HTTP client basic listener object.
 *
 * @memberof SvHTTPBasicListener
 *
 * @param[in] self         HTTP client basic listener handle
 * @param[in] outputStream output stream to write received data to
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvHTTPBasicListener
SvHTTPBasicListenerInitWithOutputStream(SvHTTPBasicListener self,
                                        QBOutputStream outputStream,
                                        SvErrorInfo *errorOut);


/**
 * Set max destination file size.
 *
 * @memberof SvHTTPBasicListener
 *
 * @param[in] self         HTTP client basic listener handle
 * @param[in] maxSize      max file size in bytes, @c -1 to disable limit
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPBasicListenerSetMaxFileSize(SvHTTPBasicListener self,
                                  off_t maxSize,
                                  SvErrorInfo *errorOut);

/**
 * Set callback function for HTTP client basic listener object.
 *
 * This method associates callback function and an opaque data pointer
 * with the listener. This function will be called when the request
 * is completed, either successfully or not.
 *
 * @memberof SvHTTPBasicListener
 *
 * @param[in] self         HTTP client basic listener handle
 * @param[in] callback     callback function (@c NULL to remove callback)
 * @param[in] prv          opaque data pointer for callback function
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPBasicListenerSetCallback(SvHTTPBasicListener self,
                               SvHTTPBasicListenerCallback callback,
                               void *prv,
                               SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
