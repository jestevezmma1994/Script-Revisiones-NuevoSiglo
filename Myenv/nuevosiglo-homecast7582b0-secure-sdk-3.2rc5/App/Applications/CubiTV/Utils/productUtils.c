/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "productUtils.h"
#include <QBStringUtils.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <string.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvLog.h>
#include <SvDataBucket2/SvDBObject.h>
#include <QBJSONUtils.h>

SvString
QBProductUtilsGetProductName(SvDBRawObject product, SvErrorInfo *errorOut)
{
    SvString productName = NULL;
    SvErrorInfo error = NULL;
    if (!product) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "product value not defined");
        goto fini;
    }

    SvValue tmp = (SvValue) SvDBRawObjectGetAttrValue(product, "name");
    if (!tmp || !SvObjectIsInstanceOf((SvObject) tmp, SvValue_getType()) || !SvValueIsString(tmp)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid or missing name");
        goto fini;
    } else {
        productName = SvValueGetString(tmp);
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return productName;
}

SvString
QBProductUtilsCreateProductID(SvDBRawObject product, SvErrorInfo *errorOut)
{
    SvString productId = NULL;
    SvErrorInfo error = NULL;
    if (!product) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "product value not defined");
        goto fini;
    }

    SvValue tmp = SvDBObjectGetID((SvDBObject) product);
    if (likely(SvValueIsString(tmp))) {
        productId = SVRETAIN(SvValueGetString(tmp));
    } else if (SvValueIsInteger(tmp)) {
        productId = SvStringCreateWithFormat("%d", SvValueGetInteger(tmp));
    } else {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid object ID (value type %d)", SvValueGetType(tmp));
        goto fini;
    }

fini:
    SvErrorInfoPropagate(error, errorOut);
    return productId;
}

SvString
QBProductUtilsCreateEventData(SvDBRawObject product, SvErrorInfo *errorOut)
{
    SvString data = NULL;
    SvErrorInfo error = NULL;
    if (!product) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "product value not defined");
        goto fini;
    }

    SvString idStr = NULL;
    SvString nameStr = NULL;
    SvString id = QBProductUtilsCreateProductID(product, NULL);
    if (id) {
        char *escapedProductId = QBStringCreateJSONEscapedString(SvStringCString(id));
        idStr = SvStringCreateWithFormat(",\"productId\":\"%s\"", escapedProductId);
        free(escapedProductId);
        SVRELEASE(id);
    }

    SvString name = QBProductUtilsGetProductName(product, NULL);
    if (name) {
        char *escapedTitle = QBStringCreateJSONEscapedString(SvStringCString(name));
        nameStr = SvStringCreateWithFormat(",\"title\":\"%s\"", escapedTitle);
        free(escapedTitle);
    }

    data = SvStringCreateWithFormat("%s%s", idStr ? SvStringCString(idStr) : "", nameStr ? SvStringCString(nameStr) : "");
    SVTESTRELEASE(idStr);
    SVTESTRELEASE(nameStr);
fini:
    SvErrorInfoPropagate(error, errorOut);
    return data;
}

bool
QBProductUtilsCheckIfProductExpired(SvObject product)
{
    bool result = false;
    SvValue expiration = 0;
    const char *expireTime = 0;

    if (SvObjectIsInstanceOf(product, SvTVChannel_getType())) {
        expiration = (SvValue) SvTVChannelGetAttribute((SvTVChannel) product, SVSTRING("expires_at"));
    } else if (SvObjectIsInstanceOf(product, SvDBRawObject_getType())) {
        expiration = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) product, "expires_at");
    }
    if (expiration && SvValueIsString(expiration))
        expireTime = SvValueGetStringAsCString(expiration, 0);
    if (expireTime) {
        SvTime entitlementEnd;
        if (*expireTime) {
            if (QBStringToDateTime(expireTime, QBDateTimeFormat_ISO8601, false, &entitlementEnd) == (ssize_t) strlen(expireTime)) {
                if (SvTimeCmp(entitlementEnd, SvTimeGetCurrentTime()) <= 0) {
                    result = true;
                }
            } else {
                SvLogError("%s(): can't parse ISO 8601 date/time string '%s'", __func__, expireTime);
            }
        } else {
            result = true;
        }
    }

    return result;
}
