/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef CONT_PARSER_PROXY_H
#define CONT_PARSER_PROXY_H

#include "cont_parser.h"

#include <storage_input/storage_input.h>
#include <SvPlayerKit/SvBufMan.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <SvPlayerKit/SvPlayerConfig.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>

#include <stdbool.h>

/**
 * The proxy parser will act as below:
 * 1. read \a min_detection_size bytes from the input (or less iff eos)
 * 2. use internal detection functions to recognize the format
 * 3. data used in the detection phase will be pushed back to the input (with use of storage_input proxy)
 * 4. create final container parser, based on the received format
 * The proxy parser will then transparently behave as the final parser.
 * This includes waiting for the final parser to perform full format detection if it's needed.
 */

struct cont_parser_proxy_params
{
  struct storage_input* input;
  struct svdataformat* format;
  SvDRMInfo drmInfo;
  SvPlayerConfig playerConfig;
  SvHTTPClientEngine httpEngine;

  bool prefer_simple_subparsers;
  int alignment;
};

int cont_parser_proxy_create(const struct cont_parser_proxy_params* params, struct cont_parser** out);

#endif // #ifndef CONT_PARSER_PROXY_H
