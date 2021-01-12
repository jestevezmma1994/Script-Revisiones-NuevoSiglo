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

#ifndef QBCONTENTINFO_H_
#define QBCONTENTINFO_H_

#include <QBContentManager/QBContentCategory.h>
#include <SvFoundation/SvCoreTypes.h>
#include <main_decl.h>

typedef struct QBContentInfo_t* QBContentInfo;

/** Get runtime type identification object representing QBContentInfo class
 * @return object representing QBContentInfo class
 */
SvType
QBContentInfo_getType(void);

SvObject
QBContentInfoCreate(AppGlobals appGlobals, SvString serviceId, SvString serviceName,
                    SvObject movie, QBContentCategory category, SvObject provider,
                    SvObject ownerTree);

SvString
QBContentInfoGetServiceId(QBContentInfo self);

SvString
QBContentInfoGetServiceName(QBContentInfo self);

SvObject
QBContentInfoGetMovie(QBContentInfo self);

QBContentCategory
QBContentInfoGetCategory(QBContentInfo self);

SvObject
QBContentInfoGetProvider(QBContentInfo self);

SvObject
QBContentInfoGetOwnerTree(QBContentInfo self);

void
QBContentInfoSetMovie(QBContentInfo self,
                      SvObject movie);

/**
 * Check if product has playlist.
 *
 * @param[in] self self object
 * @return true if movie has playlist or false in other case
 */
bool
QBContentInfoMovieHasPlaylist(QBContentInfo self);

#endif /* QBCONTENTINFO_H_ */
