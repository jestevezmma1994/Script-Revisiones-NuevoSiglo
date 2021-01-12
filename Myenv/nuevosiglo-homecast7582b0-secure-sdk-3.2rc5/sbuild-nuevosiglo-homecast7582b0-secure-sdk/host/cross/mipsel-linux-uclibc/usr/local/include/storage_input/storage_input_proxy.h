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

#ifndef STORAGE_INPUT_PROXY_H
#define STORAGE_INPUT_PROXY_H

#include "storage_input.h"
#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvChbuf.h>

int storage_input_proxy_create(struct storage_input* sub_input,
                               struct storage_input** out,
                               SvErrorInfo* error_out);

/** Push back some of the data that was read from the input (or sub_input).
 *  It will be returned with next data() or eos() callback.
 */
void storage_input_proxy_push_back(struct storage_input* input, SvChbuf chdata);

#endif // #ifndef STORAGE_INPUT_PROXY_H
