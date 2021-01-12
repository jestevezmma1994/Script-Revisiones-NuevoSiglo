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

#include <Utils/QBContentInfo.h>
#include <SvFoundation/SvType.h>
#include <SvDataBucket2/SvDBRawObject.h>

struct QBContentInfo_t {
    struct SvObject_ super_;

    SvString serviceId;
    SvString serviceName;

    SvObject movie;
    QBContentCategory category;
    SvObject provider;

    SvObject ownerTree;
};

SvLocal void
QBContentInfo__dtor__(void *self_)
{
    QBContentInfo self = (QBContentInfo) self_;

    SVTESTRELEASE(self->serviceId);
    SVTESTRELEASE(self->serviceName);
    SVTESTRELEASE(self->movie);
    SVTESTRELEASE(self->category);
    SVTESTRELEASE(self->provider);
    SVTESTRELEASE(self->ownerTree);
}

SvType
QBContentInfo_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBContentInfo__dtor__
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBContentInfo",
                            sizeof(struct QBContentInfo_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

SvObject
QBContentInfoCreate(AppGlobals appGlobals, SvString serviceId, SvString serviceName,
                    SvObject movie, QBContentCategory category, SvObject provider,
                    SvObject ownerTree)
{
    QBContentInfo self = (QBContentInfo) SvTypeAllocateInstance(QBContentInfo_getType(), NULL);

    self->serviceId = SVTESTRETAIN(serviceId);
    self->serviceName = SVTESTRETAIN(serviceName);
    self->movie = SVTESTRETAIN(movie);
    self->category = SVTESTRETAIN(category);
    self->provider = SVTESTRETAIN(provider);
    self->ownerTree = SVTESTRETAIN(ownerTree);

    return (SvObject) self;
}

SvString
QBContentInfoGetServiceId(QBContentInfo self)
{
    return self->serviceId;
}

SvString
QBContentInfoGetServiceName(QBContentInfo self)
{
    return self->serviceName;
}

SvObject
QBContentInfoGetMovie(QBContentInfo self)
{
    return self->movie;
}

QBContentCategory
QBContentInfoGetCategory(QBContentInfo self)
{
    return self->category;
}

SvObject
QBContentInfoGetProvider(QBContentInfo self)
{
    return self->provider;
}

SvObject
QBContentInfoGetOwnerTree(QBContentInfo self)
{
    return self->ownerTree;
}

void
QBContentInfoSetMovie(QBContentInfo self,
                      SvObject movie)
{
    SVTESTRELEASE(self->movie);
    self->movie = SVTESTRETAIN(movie);
}

bool
QBContentInfoMovieHasPlaylist(QBContentInfo self)
{
    if (self->movie && SvObjectIsInstanceOf(self->movie, SvDBRawObject_getType())) {
        SvDBRawObject movie = (SvDBRawObject) self->movie;
        SvValue hasPlaylistV = (SvValue) SvDBRawObjectGetAttrValue(movie, "has_playlist");
        if (hasPlaylistV && SvObjectIsInstanceOf((SvObject) hasPlaylistV, SvValue_getType()) && SvValueIsInteger(hasPlaylistV)) {
            return SvValueGetInteger(hasPlaylistV) > 0;
        }
    }
    return false;
}
