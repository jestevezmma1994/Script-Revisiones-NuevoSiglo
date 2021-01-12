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

#ifndef STORAGE_INPUT_HTTP_H
#define STORAGE_INPUT_HTTP_H

#include "storage_input.h"

#include <fibers/c/fibers.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/SvMemCounter.h>
#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>

struct storage_input_http_params
{
  SvHTTPClientEngine http_engine;
  SvScheduler  scheduler;
  const char* url;
  SvString proxy;
  int timeout;

  SvBufMan  buf_man;
};

int storage_input_http_create(const struct storage_input_http_params* params,
                              struct storage_input** out, SvErrorInfo* error_out);

void storage_input_http_set_flexible_mem_limit(struct storage_input* input_, uint32_t limit);
int  storage_input_http_get_download_speed(struct storage_input* input_);
void storage_input_http_get_download_stats(struct storage_input* input_, uint64_t* download_size, int* download_ms);
void storage_input_http_set_max_data_size(struct storage_input* input_, uint32_t limit);

/**
 * Set buffer size for http requests performed by internal http client engine.
 *
 * @relates storage_input
 *
 * @param[in]   storage input handle
 **/
void storage_input_http_set_buffer_size(struct storage_input* input_, uint32_t buffer_size);

/**
 * Get URL used for http requests to storage.
 *
 * For alternative @see storage_input_http_get_final_url
 *
 * @relates storage_input
 *
 * @param[in]  input_       storage input handle
 * @param[out] initiallUrl  storage URL
 **/
void storage_input_http_get_initial_url(struct storage_input* input_, char** initiallUrl);

/**
 * Get final URL used for http requests to storage.
 *
 * This method differs from storage_input_http_get_initial_url that it can return already redirected URL, while 'initial' method
 * returns URL with which storage_input was created.
 *
 * @relates storage_input
 *
 * @param[in]  input_       storage input handle
 * @param[out] finalUrl     storage URL
 **/
void storage_input_http_get_final_url(struct storage_input* input_, char** finalUrl);

/**
 * Get HTTP error code returned by server.
 *
 * @param[in] input_    input handle
 * @return              HTTP error code, @c 0 if irrelevant
 **/
int storage_input_http_get_http_error_code(struct storage_input *input_);

/**
 * Get storage input internal http client engine.
 *
 * @relates storage_input
 *
 * Internal HTTP client engine has cookies support enabled and user agent
 * set to "Cubiware Media Platform".
 *
 * @param[in]   storage input handle
 * @return      http client engine
 **/
SvHTTPClientEngine
storage_input_get_http_client_engine(struct storage_input* input_);

#endif // #ifndef STORAGE_INPUT_HTTP_H
