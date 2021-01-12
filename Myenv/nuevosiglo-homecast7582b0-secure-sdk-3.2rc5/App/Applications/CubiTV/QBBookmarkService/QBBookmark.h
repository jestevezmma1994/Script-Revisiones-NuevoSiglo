/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBBOOKMARK_H_
#define QBBOOKMARK_H_

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>

#include <time.h>

typedef struct QBBookmark_ *QBBookmark;

typedef enum QBBookmarkType_e {
    QBBookmarkType_Unknown,
    QBBookmarkType_Generic,
    QBBookmarkType_Recording,
    QBBookmarkType_Innov8on,
} QBBookmarkType;

SvType
QBBookmark_getType(void);

QBBookmark
QBBookmarkCreate(SvString name,
                 SvString contentID,
                 double position,
                 QBBookmarkType type);

SvString
QBBookmarkGetBookmarkId(QBBookmark self);

SvString
QBBookmarkGetBookmarkName(QBBookmark self);

SvString
QBBookmarkGetContentId(QBBookmark self);

double
QBBookmarkGetPosition(QBBookmark self);

QBBookmarkType
QBBookmarkGetType(QBBookmark self);

time_t
QBBookmarkGetTimestamp(QBBookmark self);

SvObject
QBBookmarkGetParameter(QBBookmark self,
                       SvObject key);

int
QBBookmarkUpdatePosition(QBBookmark self,
                         double position);

int
QBBookmarkChangeType(QBBookmark self,
                     QBBookmarkType type);

int
QBBookmarkAddParameter(QBBookmark self,
                       SvObject key,
                       SvObject value);

int
QBBookmarkRemoveParameter(QBBookmark self,
                          SvObject key);

void
QBBookmarkRegisterJSONSerializers(void);

#endif /* QBBOOKMARK_H_ */
