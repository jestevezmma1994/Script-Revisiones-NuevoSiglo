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

#ifndef SV_HTTP_CLIENT_LISTENER_H_
#define SV_HTTP_CLIENT_LISTENER_H_

/**
 * @file SvHTTPClientListener.h
 * @brief HTTP client listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvData.h>
#include <SvHTTPClient/SvHTTPRequest.h>


/**
 * @defgroup SvHTTPClientListener HTTP client listener interface
 * @ingroup SvHTTPClient
 * @{
 *
 * An interface for a class that handles notifications from HTTP request.
 *
 * @image html SvHTTPClientListener.png
 **/

/**
 * SvHTTPClientListener interface.
 **/
typedef const struct SvHTTPClientListener_ {
    /**
     * Method called when HTTP header has been received.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     * @param[in] headerName HTTP header name
     * @param[in] value     HTTP header value
     **/
    void (*headerReceived)(SvGenericObject self_,
                           SvHTTPRequest request,
                           const char *headerName,
                           const char *value);

    /**
     * Method called when HTTP data chunk has been received.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     * @param[in] offset    offset of the data chunk
     * @param[in] data      received data chunk
     * @param[in] length    size of data chunk in bytes
     **/
    void (*dataChunkReceived)(SvGenericObject self_,
                              SvHTTPRequest request,
                              off_t offset,
                              const uint8_t *data,
                              size_t length);

    /**
     * Method called when HTTP request is finished succesfully.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     * @param[in] total     total number of bytes fetched
     **/
    void (*transferFinished)(SvGenericObject self_,
                             SvHTTPRequest request,
                             off_t total);

    /**
     * Method called when HTTP request has been cancelled.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     **/
    void (*transferCancelled)(SvGenericObject self_,
                              SvHTTPRequest request);

    /**
     * Method called when HTTP request fails.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     * @param[in] httpError HTTP error code, @c 0 if irrelevant
     **/
    void (*errorOccured)(SvGenericObject self_,
                         SvHTTPRequest request,
                         int httpError);
} *SvHTTPClientListener;

/**
 * Get runtime type identification object representing
 * HTTP client listener interface.
 *
 * @return HTTP client listener interface
 **/
extern SvInterface
SvHTTPClientListener_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup SvHTTPClientListener2 HTTP client listener (version 2) interface
 * @ingroup SvHTTPClient
 * @{
 *
 * An interface adding zero-copy download functionality
 * to the @ref SvHTTPClientListener.
 *
 * @image html SvHTTPClientListener2.png
 **/

/**
 * SvHTTPClientListener2 interface.
 **/
typedef struct SvHTTPClientListener2_ {
    /// super interface
    struct SvHTTPClientListener_ super_;

    /**
     * Method called when new download buffer is needed.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     * @param[in] minSize   minimum buffer size in bytes
     * @return              a handle to an empty buffer
     **/
    SvData (*getBuffer)(SvGenericObject self_,
                        SvHTTPRequest request,
                        size_t minSize);

    /**
     * Method called when download buffer is ready to be processed.
     *
     * This method notifies listener that buffer has been filled
     * with data and can be processed. It is used instead of
     * SvHTTPClientListener_::dataChunkReceived() method.
     *
     * @note In all cases except for the last buffer (when the data transfer
     * ends) buffer will be filled to its full capacity.
     *
     * @param[in] self_     listener handle
     * @param[in] request   HTTP client request handle
     * @param[in] offset    offset of the data in @a buffer
     * @param[in] buffer    handle to a buffer received from
     *                      SvHTTPClientListener2_::getBuffer()
     * @param[in] length    number of bytes in @a buffer
     **/
    void (*handleBuffer)(SvGenericObject self_,
                         SvHTTPRequest request,
                         off_t offset,
                         SvData buffer,
                         size_t length);
} *SvHTTPClientListener2;

/**
 * Get runtime type identification object representing
 * HTTP client listener (version 2) interface.
 *
 * @return HTTP client listener (version 2) interface
 **/
extern SvInterface
SvHTTPClientListener2_getInterface(void);

/**
 * @}
 **/


#endif
