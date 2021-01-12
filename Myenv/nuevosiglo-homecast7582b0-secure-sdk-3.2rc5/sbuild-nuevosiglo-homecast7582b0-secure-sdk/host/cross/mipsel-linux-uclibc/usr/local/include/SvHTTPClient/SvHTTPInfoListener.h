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

#ifndef SV_HTTP_INFO_LISTENER_H_
#define SV_HTTP_INFO_LISTENER_H_

/**
 * @file SvHTTPInfoListener.h
 * @brief HTTP client info listener class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvIterator.h>

/**
 * @defgroup SvHTTPInfoListener HTTP client info listener class
 * @ingroup SvHTTPClient
 * @{
 *
 * A utility class implementing @ref SvHTTPClientListener that stores
 * all received HTTP headers.
 *
 * This class can be used as an HTTP client listener for HTTP HEAD,
 * OPTIONS, DELETE methods. It ignores received data but remembers all
 * received HTTP headers for further inspection.
 *
 * @image html SvHTTPInfoListener.png
 **/


/**
 * HTTP client info listener class.
 * @class SvHTTPInfoListener
 * @extends SvObject
 **/
typedef struct SvHTTPInfoListener_ *SvHTTPInfoListener;


/**
 * Callback function type for HTTP requests.
 *
 * @param[in] prv          opaque data pointer bound to the request
 * @param[in] listener     HTTP client info listener handle
 * @param[in] error        error info (@c NULL on success)
 **/
typedef void (*SvHTTPInfoListenerCallback)(void *prv,
                                           SvHTTPInfoListener listener,
                                           SvErrorInfo error);


/**
 * Get runtime type identification object representing
 * type of HTTP client info listener class.
 *
 * @return HTTP client info listener class
 **/
extern SvType
SvHTTPInfoListener_getType(void);

/**
 * Initialize HTTP client info listener object.
 *
 * @memberof SvHTTPInfoListener
 *
 * @param[in] self         HTTP client info listener handle
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern SvHTTPInfoListener
SvHTTPInfoListenerInit(SvHTTPInfoListener self,
                       SvErrorInfo *errorOut);

/**
 * Set callback function for HTTP client info listener object.
 *
 * This method associates callback function and an opaque data pointer
 * with the listener. This function will be called when the request
 * is completed, either successfully or not.
 *
 * @memberof SvHTTPInfoListener
 *
 * @param[in] self         HTTP client info listener handle
 * @param[in] callback     callback function (@c NULL to remove callback)
 * @param[in] prv          opaque data pointer for callback function
 * @param[out] errorOut    error info
 **/
extern void
SvHTTPInfoListenerSetCallback(SvHTTPInfoListener self,
                              SvHTTPInfoListenerCallback callback,
                              void *prv,
                              SvErrorInfo *errorOut);

/**
 * Get value of one of received HTTP headers.
 *
 * @memberof SvHTTPInfoListener
 *
 * @param[in] self         HTTP client info listener handle
 * @param[in] header       HTTP header name
 * @param[out] errorOut    error info
 * @return                 value of requested HTTP header
 *                         or @c NULL in case of error
 **/
extern SvString
SvHTTPInfoListenerGetHeader(SvHTTPInfoListener self,
                            SvString header,
                            SvErrorInfo *errorOut);

/**
 * Get an iterator over names of all received HTTP headers.
 *
 * @memberof SvHTTPInfoListener
 *
 * @param[in] self         HTTP client info listener handle
 * @param[out] errorOut    error info
 * @return                 an iterator over names of received HTTP headers
 **/
extern SvIterator
SvHTTPInfoListenerGetHeadersIterator(SvHTTPInfoListener self,
                                     SvErrorInfo *errorOut);

// deprecated alias, remove when not needed anymore
/** @cond */
static inline SvIterator
SvHTTPInfoListenerHeadersIterator(SvHTTPInfoListener self,
                                  SvErrorInfo *errorOut)
{
    return SvHTTPInfoListenerGetHeadersIterator(self, errorOut);
}
/** @endcond */

/**
 * @}
 **/


#endif
