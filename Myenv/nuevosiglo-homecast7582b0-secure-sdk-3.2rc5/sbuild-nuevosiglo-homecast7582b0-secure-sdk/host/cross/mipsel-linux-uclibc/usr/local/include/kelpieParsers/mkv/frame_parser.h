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

#ifndef KPPARSERS_MKV_FRAME_PARSER_H
#define KPPARSERS_MKV_FRAME_PARSER_H

#include "parser_codes.h"
#include "parser_data.h"
#include "frame_output.h"
#include "../external/ra_input.h"

#ifdef __cplusplus
extern "C" {
#endif

struct kpebml_parser;
struct kpmkv_frame_parser;

SvExport
struct kpmkv_frame_parser*
kpmkv_frame_parser_create(const struct kpmkv_parser_data* parser_data,
                          struct kpmkv_frame_output* frame_output,
                          kp_ra_input* input);
SvExport
void kpmkv_frame_parser_destroy(struct kpmkv_frame_parser* parser);

SvExport
int kpmkv_frame_parser_run(struct kpmkv_frame_parser* parser);
SvExport
void kpmkv_frame_parser_reset(struct kpmkv_frame_parser* parser);


SvExport
bool kpmkv_frame_parser_seek(struct kpmkv_frame_parser* parser, uint64_t* pts);

SvExport
void kpmkv_frame_parser_set_trick_mode(struct kpmkv_frame_parser* parser, bool enabled, bool forward, double speed);


#ifdef __cplusplus
}
#endif

#endif // #ifndef KPPARSERS_MKV_FRAME_PARSER_H
