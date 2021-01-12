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

#ifndef QB_MP4_PARSER_H
#define QB_MP4_PARSER_H

#include <SvFoundation/SvString.h>
#include <storage_input/storage_input_ra.h>

#ifdef __cplusplus
extern "C" {
#endif

struct SvBuf_s;
struct SvChbuf_s;

struct QBMp4Parser_s;
typedef struct QBMp4Parser_s  QBMp4Parser;
struct QBMp4Info_s;
struct QBMp4FragmentInfo_s;


enum QBMp4ParserCode_e
{
  QBMp4ParserCode_ok       =  0,
  QBMp4ParserCode_error    = -1,
  QBMp4ParserCode_no_data  = -2,

  QBMp4ParserCode_internal = -1000,
};
typedef enum QBMp4ParserCode_e  QBMp4ParserCode;



QBMp4Parser* QBMp4ParserCreate(struct storage_input_ra* input,
                               int64_t max_pos,
                               bool fragment_mode);
void QBMp4ParserDestroy(QBMp4Parser* parser);


/** Continue parsing the data.
 *  \returns QBMp4ParserCode
 */
QBMp4ParserCode QBMp4ParserRun(QBMp4Parser* parser);

struct QBMp4Info_s* QBMp4ParserGetInfo(QBMp4Parser* parser);
struct QBMp4FragmentInfo_s* QBMp4ParserGetFragmentInfo(QBMp4Parser* parser);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_MP4_PARSER_H
