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

#ifndef QB_ID3_PARSER_H
#define QB_ID3_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <SvFoundation/SvString.h>

struct SvBuf_s;
struct SvChbuf_s;

struct QBId3Parser_s;
typedef struct QBId3Parser_s  QBId3Parser;

struct QBId3ParserCallbacks_s
{
  /** Handled frames and the names they will be returned with:
   *  TIT1 : Content group
   *  TIT2 : Title
   *  TIT3 : Subtitle
   *  TALB : Album
   *  TRCK : Track (X, or X/Y)
   *  TPE1 : Artist (originally: Lead Artist)
   *  TPE2 : Band
   *  TLAN : Language
   *  TYER : Year
   *
   *  TLEN : Duration (in ms)
   *
   */
  void (*string) (void* target, QBId3Parser* parser, SvString key, SvString value);

  /** Handle PRIV tag
   */
  void (*private_frame) (void* target, QBId3Parser* parser, SvString owner, uint8_t *data, size_t length);
};


QBId3Parser* QBId3ParserCreate(const struct QBId3ParserCallbacks_s* callbacks, void* target);
void QBId3ParserDestroy(QBId3Parser* parser);


/** Push consecutive chunk of data.
 *  The parser becomes the owner of the buffer \a sb.
 *  The data is still read-only.
 */
void QBId3ParserPush(QBId3Parser* parser, struct SvBuf_s* sb);
void QBId3ParserTransfer(QBId3Parser* parser, struct SvChbuf_s* chdata);

/** Parse data gathered from \a QBId3ParserPush calls.
 *  \returns 0 wants more data
 *  \returns 1 finished parsing
 *  \returns -1 parse error
 */
int QBId3ParserRun(QBId3Parser* parser);


/** Get size of whole ID3 tag.
 */
int QBId3ParserGetHeaderSize(QBId3Parser* parser);


/** After parsing is finished, and more data was pushed than the ID3 tag size, return it now.
 */
void QBId3ParserGetLeftoverData(QBId3Parser* parser, struct SvChbuf_s* chbOut);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_ID3_PARSER_H
