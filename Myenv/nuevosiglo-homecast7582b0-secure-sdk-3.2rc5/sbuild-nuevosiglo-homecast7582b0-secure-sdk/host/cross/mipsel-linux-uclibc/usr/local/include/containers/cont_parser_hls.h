/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CONT_PARSER_M3U_H
#define CONT_PARSER_M3U_H

#include "cont_parser.h"

#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <SvPlayerKit/SvChbuf.h>
#include <storage_input/storage_input.h>

struct cont_parser_hls_params
{
    struct storage_input* input;
    SvChbuf data;

    SvHTTPClientEngine httpEngine;
    SvDRMInfo drmInfo;
};

int cont_parser_hls_create(const struct cont_parser_hls_params* params, struct cont_parser** out);

#endif
