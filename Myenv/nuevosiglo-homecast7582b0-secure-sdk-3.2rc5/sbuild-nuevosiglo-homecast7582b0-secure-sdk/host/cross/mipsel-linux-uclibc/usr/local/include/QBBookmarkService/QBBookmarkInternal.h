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

#ifndef QBBOOKMARKINTERNAL_H_
#define QBBOOKMARKINTERNAL_H_

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvHashTable.h>

struct QBBookmark_ {
    struct SvObject_ super_;

    SvString id;                  // Unique id
    SvString contentID;           // It may be different for different bookmarks, eg. meta-id for recordings, productID for VoD

    QBBookmarkType type;          // Bookmark type/origin
    double position;              // Position
    time_t timestamp;             // Time of last bookmark update

    SvString name;                // Optional, given by user
    SvHashTable additionalParams; // Parameters specific for one bookmark type
};

struct QBBookmark_*
QBBookmarkInit(struct QBBookmark_* self,
               SvString name,
               SvString contentID,
               QBBookmarkType type,
               double position);

#endif /* QBBOOKMARKINTERNAL_H_ */
