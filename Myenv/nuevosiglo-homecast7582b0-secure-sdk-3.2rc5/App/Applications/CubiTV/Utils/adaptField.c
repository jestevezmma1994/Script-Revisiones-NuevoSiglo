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

#include "adaptField.h"

#include <stdlib.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvURL.h>
#include <SvDataBucket2/SvDBObject.h>
#include <Utils/value.h>
#include <stringUtils.h>

bool
adaptField(SvDBRawObject obj, const char *field, SvString prefix)
{
    bool res = false;
    SvString uri = NULL, original = NULL;
    SvURL url = NULL, prefixURL = NULL, adaptedURL = NULL;
    if (unlikely(!obj || !field || !prefix)) {
        goto fini;
    }
    SvValue value = (SvValue) SvDBRawObjectGetAttrValue(obj, field);
    if ((original = SvValueTryGetString(value))) {
        // strip leading and trailing whitespace
        const char *originalStr = SvStringCString(original);
        size_t len = SvStringLength(original);
        char *stripped = malloc(sizeof(char) * (len  + 1));
        size_t newLen = QBStringUtilsStripWhitespace(stripped, len + 1, originalStr);
        uri = SvStringCreate(stripped, NULL);
        free(stripped);

        // set stripped attribute if any whitespace stripped
        if (unlikely(newLen < len)) {
            SvValue val = SvValueCreateWithString(uri, NULL);
            SvDBRawObjectSetAttrValue(obj, field, (SvObject) val);
            SVTESTRELEASE(val);
        }

        url = SvURLCreateWithString(uri, NULL);
        if (!url) {
            goto fini;
        }

        prefixURL = SvURLCreateWithString(prefix, NULL);
        if (!prefixURL) {
            goto fini;
        }
        adaptedURL = NULL;
        if (prefixURL && !SvURIIsAbsolute(url))
            adaptedURL = SvURLCreateWithStringRelativeToURL(uri, prefixURL, NULL);
        else
            adaptedURL = SVRETAIN(url);
        if (!adaptedURL) {
            goto fini;
        }
        SvValue val = SvValueCreateWithString(SvURLString(adaptedURL), NULL);
        SvDBRawObjectSetAttrValue(obj, field, (SvObject) val);
        SVRELEASE(val);
        res = true;
    }

fini:
    SVTESTRELEASE(adaptedURL);
    SVTESTRELEASE(prefixURL);
    SVTESTRELEASE(url);
    SVTESTRELEASE(uri);
    if (!res)
        SvLogWarning("%s() :: can't adapt field [%s] for '%s' object,"
                     " original value: [%s], prefix: [%s]", __func__, field,
                     SvDBObjectGetTypeAsCString((SvDBObject) obj),
                     original ? SvStringCString(original) : NULL,
                     prefix ? SvStringCString(prefix) : NULL);
    return res;
}

