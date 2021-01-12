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

#ifndef QB_RANGE_TREE_H_
#define QB_RANGE_TREE_H_

#include <stddef.h>
#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>

// FIXME: documentation!!!

typedef struct QBContentRange_ *QBContentRange;
typedef struct QBRangeTree_ *QBRangeTree;

struct QBContentRange_ {
    struct SvObject_ super_;
    size_t start;
    size_t count;
    SvObject *objects;
};

extern QBRangeTree QBRangeTreeCreate(void);

extern void QBRangeTreeSetRangeSize(QBRangeTree self, size_t rangeSize);

extern void QBRangeTreeSetSingleRangeMode(QBRangeTree self);

extern void QBRangeTreeAddRange(QBRangeTree self, size_t index, SvArray objects);

extern QBContentRange QBRangeTreeGetRange(QBRangeTree self, size_t idx);

extern void QBRangeTreeItemsChanged(QBRangeTree self, size_t start, SvIterator *it, size_t count);

extern void QBRangeTreeItemChanged(QBRangeTree self, size_t index, SvObject object);

extern void QBRangeTreeMergeItems(QBRangeTree self, bool *didChange, size_t first, SvArray objects);

extern void QBRangeTreeSetLength(QBRangeTree self, size_t length);

extern ssize_t QBRangeTreeFindObject(QBRangeTree self, SvObject obj);

extern void QBRangeTreeRemoveRange(QBRangeTree self, QBContentRange range);

#endif
