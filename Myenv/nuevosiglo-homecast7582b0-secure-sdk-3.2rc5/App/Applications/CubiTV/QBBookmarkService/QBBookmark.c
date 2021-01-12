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

#include "QBBookmark.h"
#include "QBBookmarkInternal.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvValue.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>

#include <stdlib.h>

SvLocal void
QBBookmarkDestroy(void *self_)
{
    QBBookmark self = (QBBookmark) self_;

    SVRELEASE(self->id);
    SVRELEASE(self->contentID);
    SVRELEASE(self->additionalParams);
    SVTESTRELEASE(self->name);
}

SvType
QBBookmark_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBBookmarkDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBBookmark",
                            sizeof(struct QBBookmark_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBBookmark
QBBookmarkInit(struct QBBookmark_* self,
               SvString name,
               SvString contentID,
               QBBookmarkType type,
               double position)
{
    uint64_t randNum = (uint64_t)rand();
    SvTime currentTime = SvTimeGetCurrentTime();

    self->id = SvStringCreateWithFormat("%llu-%lld", (long long unsigned) randNum, (long long) SvTimeToMicroseconds64(currentTime));
    self->name = SVTESTRETAIN(name);
    self->contentID = SVRETAIN(contentID);

    self->position = position;
    self->timestamp = SvTimeNow();
    self->type = type;

    self->additionalParams = SvHashTableCreate(10, NULL);

    return self;
}

QBBookmark
QBBookmarkCreate(SvString name,
                 SvString contentID,
                 double position,
                 QBBookmarkType type)
{
    QBBookmark self = NULL;

    if (!contentID) {
        SvLogError("%s: NULL contentID passed", __func__);
        return NULL;
    } else if (!(self = (QBBookmark) SvTypeAllocateInstance(QBBookmark_getType(), NULL))) {
        SvLogError("%s: can't create QBBookmark", __func__);
        return NULL;
    }

    QBBookmarkInit(self, name, contentID, type, position);

    return self;
}

SvString
QBBookmarkGetBookmarkName(QBBookmark self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    }

    return self->name;
}

SvString
QBBookmarkGetBookmarkId(QBBookmark self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    }

    return self->id;
}

SvString
QBBookmarkGetContentId(QBBookmark self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    }

    return self->contentID;
}

double
QBBookmarkGetPosition(QBBookmark self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1.0;
    }

    return self->position;
}

time_t
QBBookmarkGetTimestamp(QBBookmark self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    }

    return self->timestamp;
}

QBBookmarkType
QBBookmarkGetType(QBBookmark self)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return QBBookmarkType_Unknown;
    }

    return self->type;
}

SvObject
QBBookmarkGetParameter(QBBookmark self,
                       SvObject key)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return NULL;
    } else if (!key) {
        SvLogError("%s: NULL key passed", __func__);
        return NULL;
    }

    return SvHashTableFind(self->additionalParams, key);
}

int
QBBookmarkUpdatePosition(QBBookmark self,
                         double position)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    }

    self->position = position;
    return 0;
}

int
QBBookmarkChangeType(QBBookmark self,
                     QBBookmarkType type)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    }

    self->type = type;
    return 0;
}

int
QBBookmarkAddParameter(QBBookmark self,
                       SvObject key,
                       SvObject value)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!key) {
        SvLogError("%s: NULL key passed", __func__);
        return -1;
    } else if (!value) {
        SvLogError("%s: NULL value passed", __func__);
        return -1;
    }

    SvHashTableInsert(self->additionalParams, key, value);
    return 0;
}

int
QBBookmarkRemoveParameter(QBBookmark self,
                          SvObject key)
{
    if (!self) {
        SvLogError("%s: NULL self passed", __func__);
        return -1;
    } else if (!key) {
        SvLogError("%s: NULL key passed", __func__);
        return -1;
    }

    SvHashTableRemove(self->additionalParams, key);
    return 0;
}

SvLocal SvString
QBBookmarkTypeToString(QBBookmarkType bookmarkType)
{
    if (bookmarkType == QBBookmarkType_Generic)
        return SVSTRING("generic");
    else if (bookmarkType == QBBookmarkType_Recording)
        return SVSTRING("recording");
    else if (bookmarkType == QBBookmarkType_Innov8on)
        return SVSTRING("innov8on");
    else
        return SVSTRING("Unknown");
}

SvLocal QBBookmarkType
QBBookmarkStringToType(SvString type)
{
    assert(type != NULL);
    if (SvStringEqualToCString(type, "generic"))
        return QBBookmarkType_Generic;
    else if (SvStringEqualToCString(type, "recording"))
        return QBBookmarkType_Recording;
    else if (SvStringEqualToCString(type, "innov8on"))
        return QBBookmarkType_Innov8on;
    else
        return QBBookmarkType_Unknown;
}

SvLocal bool QBBookmarkIsValueTypeValid(SvValue val, SvValueType type)
{
    return val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueGetType(val) == type;
}

SvLocal void
QBBookmarkSerialize(SvObject helper_,
                    SvObject bookmark_,
                    SvHashTable desc,
                    SvErrorInfo *errorOut)
{
    QBBookmark self = (QBBookmark) bookmark_;

    SvValue positionV = SvValueCreateWithDouble(self->position, NULL);
    SvValue timestampV = SvValueCreateWithInteger((int) self->timestamp, NULL);

    SvHashTableInsert(desc, (SvObject) SVSTRING("contentId"), (SvObject) self->contentID);
    SvHashTableInsert(desc, (SvObject) SVSTRING("bookmarkType"), (SvObject) QBBookmarkTypeToString(self->type));
    SvHashTableInsert(desc, (SvObject) SVSTRING("position"), (SvObject) positionV);
    SvHashTableInsert(desc, (SvObject) SVSTRING("timestamp"), (SvObject) timestampV);

    if (SvHashTableCount(self->additionalParams) != 0)
        SvHashTableInsert(desc, (SvObject) SVSTRING("additionalParams"), (SvObject) self->additionalParams);
    if (self->name)
        SvHashTableInsert(desc, (SvObject) SVSTRING("name"), (SvObject) self->name);

    SVRELEASE(positionV);
    SVRELEASE(timestampV);
}

SvLocal SvObject
QBBookmarkDeserialize(SvObject helper_,
                      SvString className,
                      SvHashTable desc,
                      SvErrorInfo *errorOut)
{
    SvValue contentIdVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("contentId"));
    SvValue bookmarkTypeVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("bookmarkType"));
    SvValue positionVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("position"));
    SvValue timestampVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("timestamp"));

    SvHashTable additionalParams = (SvHashTable) SvHashTableFind(desc, (SvObject) SVSTRING("additionalParams"));
    SvValue nameVal = (SvValue) SvHashTableFind(desc, (SvObject) SVSTRING("name"));

    if (!QBBookmarkIsValueTypeValid(contentIdVal, SvValueType_string)) {
        SvLogError("%s: bookmark deserializing error, contentIdVal has invalid type", __func__);
        return NULL;
    } else if (!QBBookmarkIsValueTypeValid(bookmarkTypeVal, SvValueType_string)) {
        SvLogError("%s: bookmark deserializing error, bookmarkTypeVal has invalid type", __func__);
        return NULL;
    } else if (!QBBookmarkIsValueTypeValid(positionVal, SvValueType_double)) {
        SvLogError("%s: bookmark deserializing error, positionVal has invalid type", __func__);
        return NULL;
    } else if (!QBBookmarkIsValueTypeValid(timestampVal, SvValueType_integer)) {
        SvLogError("%s: bookmark deserializing error, timestampVal has invalid type", __func__);
        return NULL;
    }

    SvString name = NULL;
    if (nameVal) {
        if (!QBBookmarkIsValueTypeValid(nameVal, SvValueType_string)) {
            SvLogError("%s: bookmark deserializing error: invalid name field", __func__);
            return NULL;
        }
        name = SvValueGetString(nameVal);
    }

    QBBookmark bookmark = QBBookmarkCreate(name, SvValueGetString(contentIdVal), SvValueGetDouble(positionVal), QBBookmarkStringToType(SvValueGetString(bookmarkTypeVal)));
    bookmark->timestamp = (time_t) SvValueGetInteger(timestampVal);

    if (additionalParams && SvObjectIsInstanceOf((SvObject) additionalParams, SvHashTable_getType())) {
        SVTESTRELEASE(bookmark->additionalParams);
        bookmark->additionalParams = SVRETAIN(additionalParams);
    }

    return (SvObject) bookmark;
}

void
QBBookmarkRegisterJSONSerializers(void)
{
    SvJSONClass helper = SvJSONClassCreate(QBBookmarkSerialize, QBBookmarkDeserialize, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBBookmark"), (SvObject) helper, NULL);
    SVRELEASE(helper);
}

