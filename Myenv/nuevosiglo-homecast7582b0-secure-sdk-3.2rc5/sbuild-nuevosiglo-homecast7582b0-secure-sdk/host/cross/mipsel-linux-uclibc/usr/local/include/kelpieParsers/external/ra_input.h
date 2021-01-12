/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef KPPARSERS_RA_INPUT_H
#define KPPARSERS_RA_INPUT_H

#include "common_types.h"

#include <storage_input/storage_input_ra.h>

typedef struct storage_input_ra  kp_ra_input;

#ifdef __cplusplus
extern "C" {
#endif

#define kp_ra_input_seek          storage_input_ra_seek
#define kp_ra_input_skip          storage_input_ra_skip
#define kp_ra_input_peek          storage_input_ra_peek
#define kp_ra_input_try_read      storage_input_ra_try_read
#define kp_ra_input_get_position  storage_input_ra_get_position

#ifdef __cplusplus
}
#endif

#endif // #ifndef KPPARSERS_RA_INPUT_H

