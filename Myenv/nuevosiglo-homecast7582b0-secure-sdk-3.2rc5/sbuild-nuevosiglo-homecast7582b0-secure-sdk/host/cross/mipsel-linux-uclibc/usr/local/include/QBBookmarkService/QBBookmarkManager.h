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

#ifndef QBBOOKMARKMANAGER_H_
#define QBBOOKMARKMANAGER_H_

#include <QBBookmarkService/QBBookmark.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvObject.h>
#include <fibers/c/fibers.h>
#include <main_decl.h>

typedef struct QBBookmarkManager_ *QBBookmarkManager;

QBBookmarkManager
QBBookmarkManagerCreate(AppGlobals appGlobals);

int
QBBookmarkManagerStart(QBBookmarkManager self,
                       SvScheduler scheduler);

int
QBBookmarkManagerStop(QBBookmarkManager self);

SvArray
QBBookmarkManagerCreateBookmarksArrayForContent(QBBookmarkManager self,
                                                SvString contentId);
QBBookmark
QBBookmarkManagerGetBookmarkByID(QBBookmarkManager self,
                                 SvString bookmarkId);

int
QBBookmarkManagerAddBookmark(QBBookmarkManager self,
                             QBBookmark bookmark);
int
QBBookmarkManagerRemoveBookmark(QBBookmarkManager self,
                                QBBookmark bookmark);

int
QBBookmarkManagerUpdateBookmark(QBBookmarkManager self,
                                QBBookmark bookmark);

int
QBBookmarkManagerAddPlugin(QBBookmarkManager self,
                           SvObject plugin,
                           QBBookmarkType *handledTypes,
                           size_t typesCnt);

int
QBBookmarkManagerRemovePlugin(QBBookmarkManager self,
                              SvObject plugin);

int
QBBookmarkManagerRestartPlugin(QBBookmarkManager self,
                               SvObject plugin);

QBBookmark
QBBookmarkManagerGetLastPositionBookmark(QBBookmarkManager self,
                                         SvString contentId);

QBBookmark
QBBookmarkManagerSetLastPositionBookmark(QBBookmarkManager self,
                                         SvString contentId,
                                         double position,
                                         QBBookmarkType type);

#endif /* QBBOOKMARKMANAGER_H_ */
