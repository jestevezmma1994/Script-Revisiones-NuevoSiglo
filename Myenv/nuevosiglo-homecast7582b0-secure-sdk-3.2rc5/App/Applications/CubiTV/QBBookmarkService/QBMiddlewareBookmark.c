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

#include "QBMiddlewareBookmark.h"
#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkInternal.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvJSON/SvJSONClassRegistry.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

static const char *middlewareBookmarkIdPrefix = "MW";

struct QBMiddlewareBookmark_ {
    struct QBBookmark_ super_;

    SvString productId;
    SvString deviceId;     // Optional?? We don't have it in follow me STB->device
    SvString tag;          // Unique ID for QBBookmark ID
};

SvLocal void
QBMiddlewareBookmarkSerialize(SvObject helper_,
                              SvObject bookmark,
                              SvHashTable desc,
                              SvErrorInfo *errorOut)
{
    // TODO: Implement me!
}

SvLocal SvObject
QBMiddlewareBookmarkDeserialize(SvObject helper_,
                                SvString className,
                                SvHashTable desc,
                                SvErrorInfo *errorOut)
{
    // TODO: Implement me!
    return NULL;
}

SvLocal void
QBMiddlewareBookmarkDestroy(void *self_)
{
    QBMiddlewareBookmark self = (QBMiddlewareBookmark) self_;

    SVRELEASE(self->tag);
    SVRELEASE(self->productId);
    SVTESTRELEASE(self->deviceId);
}

SvType
QBMiddlewareBookmark_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBMiddlewareBookmarkDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBMiddlewareBookmark",
                            sizeof(struct QBMiddlewareBookmark_),
                            QBBookmark_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

QBMiddlewareBookmark
QBMiddlewareBookmarkCreate(SvString name,
                           double timestamp,
                           QBMiddlewareBookmarkParams_t *params,
                           SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBMiddlewareBookmark self = NULL;

    if (!params || !params->tag || !params->productId) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "Invalid parameters");
        goto err;
    }

    self = (QBMiddlewareBookmark) SvTypeAllocateInstance(QBMiddlewareBookmark_getType(), NULL);
    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_noMemory,
                                  "can't create QBMiddlewareBookmark");
        goto err;
    }

    self->productId = SVRETAIN(params->productId);
    self->tag = SVRETAIN(params->tag);

    self->deviceId  = SVTESTRETAIN(params->deviceId);

    SvString tmpBookmarkId = SvStringCreateWithFormat("%s:%s", middlewareBookmarkIdPrefix, SvStringCString(params->tag));
    QBBookmarkInit(&(self->super_), name, tmpBookmarkId, true, timestamp);
    SVRELEASE(tmpBookmarkId);

err:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

SvString
QBMiddlewareBookmarkGetProductId(QBMiddlewareBookmark self, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
    }
    SvErrorInfoPropagate(error, errorOut);
    if (!error && self)
        return self->productId;
    return NULL;
}

SvString
QBMiddlewareBookmarkGetTag(QBMiddlewareBookmark self,
                           SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
    }
    SvErrorInfoPropagate(error, errorOut);
    if (!error && self)
        return self->tag;
    return NULL;
}

SvString
QBMiddlewareBookmarkGetDeviceId(QBMiddlewareBookmark self,
                                SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
    }
    SvErrorInfoPropagate(error, errorOut);
    if (!error && self)
        return self->deviceId;
    return NULL;
}

void
QBMiddlewareBookmarkRegisterJSONSerializers(void)
{
    SvJSONClass helper = SvJSONClassCreate(QBMiddlewareBookmarkSerialize, QBMiddlewareBookmarkDeserialize, NULL);
    SvJSONRegisterClassHelper(SVSTRING("QBMiddlewareBookmark"), (SvObject) helper, NULL);
    SVRELEASE(helper);
}

