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

#ifndef QB_TS_TABLE_PARSER_H
#define QB_TS_TABLE_PARSER_H

#include "QBTSSectionParser.h"

#include <SvPlayerKit/SvChbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBTSTableParser_s;
typedef struct QBTSTableParser_s  QBTSTableParser;

typedef enum QBTSTableParserMode_e {
  QBTSTableParser_TSSection,
  QBTSTableParser_TSScteSection,
} QBTSTableParserMode;

struct QBTSTableParserCallbacks_s
{
  void (*error)     (void* target, QBTSTableParser* parser, int err);
  void (*disc)      (void* target, QBTSTableParser* parser);
  void (*data)      (void* target, QBTSTableParser* parser, SvChbuf chb);
  void (*repeated)  (void* target, QBTSTableParser* parser);
};

QBTSTableParser* QBTSTableParserCreate(QBTSSectionParser* sectionParser);

void QBTSTableParserSetCallbacks(QBTSTableParser* parser, const struct QBTSTableParserCallbacks_s* callbacks, void* target);
void QBTSTableParserReportUnique(QBTSTableParser* parser, bool unique);
void QBTSTableParserSetMode(QBTSTableParser* parser, QBTSTableParserMode mode);

void QBTSTableParserClear(QBTSTableParser* parser);

void QBTSTableParserParallelCollect(QBTSTableParser* parser, bool parallelCollect);
QBTSSectionParser *QBTSTableParserGetSectionParser(QBTSTableParser *self);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TS_TABLE_PARSER_H
