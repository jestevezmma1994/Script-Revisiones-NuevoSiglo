/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef HLSSEEK_H_
#define HLSSEEK_H_

/**
 * @file HlsSeek.h HlsSeek interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvFoundation/SvURL.h>
#include "../cont_parser.h"
#include <storage_input/storage_input_http.h>

#include "HlsVariant.h"
#include "HlsMediaProvider.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup HlsSeek HlsSeek handler interface
 * @{
 **/
typedef struct HlsSeek_* HlsSeek;

struct HlsSeekParams
{
    struct storage_input* input;
    HlsMediaProvider mediaProvider;
    const struct cont_parser_callbacks* callbacks;
    void* owner;

    struct cont_parser_play_stats* stats;
    SvURL baseUrl;
    SvHTTPClientEngine httpEngine;
    HlsVariant variant;
    int64_t totalDuration90k;
    unsigned int downloadSpeed;

    HlsKeyProvider hlsKeyProvider;
    HlsEncMethod encMethod;
};

/**
 * HlsSeek interface.
 **/
extern HlsSeek
HlsSeekCreate(const struct HlsSeekParams* params,
              void* owner);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* HLSSEEK_H_ */
