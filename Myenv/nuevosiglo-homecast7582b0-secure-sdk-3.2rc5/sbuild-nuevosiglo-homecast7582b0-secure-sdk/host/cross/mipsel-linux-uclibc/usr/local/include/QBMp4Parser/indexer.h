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

#ifndef QB_MP4_PARSER_INDEXER_H
#define QB_MP4_PARSER_INDEXER_H

#include "data.h"

#ifdef __cplusplus
extern "C" {
#endif

struct QBMp4Indexer_s;
typedef struct QBMp4Indexer_s  QBMp4Indexer;


QBMp4Indexer* QBMp4IndexerCreate(const QBMp4Info* info);
void QBMp4IndexerDestroy(QBMp4Indexer* indexer);

/// @returns 1  when is ready
/// @returns 0  when processing is ongoing
/// @returns <0  error
int QBMp4IndexerProcess(QBMp4Indexer* indexer);

/// reset position on all tracks
/// iff @a pts90k is beyond seeking range, @a indexer will go to eos-state
void QBMp4IndexerSeek(QBMp4Indexer* indexer, uint64_t pts90k);

uint64_t QBMp4IndexerGetStartPosition(const QBMp4Indexer* indexer);
uint64_t QBMp4IndexerGetStartOffset(const QBMp4Indexer* indexer);


typedef struct QBMp4IndexPoint_s  QBMp4IndexPoint;
struct QBMp4IndexPoint_s
{
  uint32_t track;
  uint32_t timescale;

  uint64_t dts;
  uint64_t pts;

  uint64_t position;
  uint32_t size;
};

/// Fills @a point_out with properties of next sample to be read.
/// Then, sets internal position after this sample.
/// With this function, you can iterate through all samples (from current position, @see QBMp4IndexerSeek()), ordered by position in the file.
/// @returns true  iff @a point_out has been filled
/// @returns false iff end-of-stream has been reached
bool QBMp4IndexerGetNextSample(QBMp4Indexer* indexer, QBMp4IndexPoint* point_out);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_MP4_PARSER_INDEXER_H
