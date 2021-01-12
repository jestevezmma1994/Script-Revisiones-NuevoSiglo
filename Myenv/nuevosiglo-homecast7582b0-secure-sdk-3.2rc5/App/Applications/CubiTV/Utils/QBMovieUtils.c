/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#include <Utils/QBMovieUtils.h>

#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvJSON/SvJSONParse.h>
#include <SvQuirks/SvRuntimePrefix.h>
#include <settings.h>
#include <SWL/icon.h>
#include <Utils/value.h>
#include <libintl.h>

void
QBMovieUtilsSetThumbnailIntoIcon(SvWidget icon, SvObject object, const char* attrType, const char* attrName, const char* attrNameVal, size_t iconIdx)
{
    const char *thumbUrl = NULL;

    if (!SvObjectIsInstanceOf(object, SvDBRawObject_getType())) {
        goto fini;
    }

    SvDBRawObject movie = (SvDBRawObject) object;

    SvArray attrGroup = (SvArray) SvDBRawObjectGetAttrValue((SvDBRawObject) movie, attrType);
    if (attrGroup && SvObjectIsInstanceOf((SvObject) attrGroup, SvArray_getType())) {
        SvIterator it = SvArrayReverseIterator(attrGroup);
        SvHashTable cover;
        while ((cover = (SvHashTable) SvIteratorGetNext(&it))) {
            if (!SvObjectIsInstanceOf((SvObject) cover, SvHashTable_getType()))
                continue;
            SvString attrNameStr = SvStringCreate(attrName, NULL);
            SvValue name = (SvValue) SvHashTableFind(cover, (SvObject) attrNameStr);
            SVRELEASE(attrNameStr);
            if (!name || !SvObjectIsInstanceOf((SvObject) name, SvValue_getType()) || !SvValueIsString(name) ||
                !SvStringEqualToCString(SvValueGetString(name), attrNameVal))
                continue;
            SvValue url = (SvValue) SvHashTableFind(cover, (SvObject) SVSTRING("url"));
            if (!url || !SvObjectIsInstanceOf((SvObject) url, SvValue_getType()) || !SvValueIsString(url))
                continue;
            thumbUrl = SvStringCString(SvValueGetString(url));
            break;
        }
    }

    if (!thumbUrl) {
        SvString thumbnail = QBMovieUtilsGetAttr(movie, "thumbnail", NULL);
        if (thumbnail)
            thumbUrl = SvStringCString(QBMovieUtilsGetAttr(movie, "thumbnail", NULL));
    }

fini:
    if (thumbUrl) {
        svIconSetBitmapFromURI(icon, iconIdx, thumbUrl);
        if (!svIconIsBitmapAvailable(icon, iconIdx))
            svIconSwitch(icon, 0, 0, 0.0);
        svIconSwitch(icon, iconIdx, 0, -1.0);
    } else {
        svIconSwitch(icon, 0, 0, 0.0);
    }
}

SvString
QBMovieUtilsGetThumbnailUrl(SvArray tab, unsigned width, unsigned height)
{
    SvIterator it = SvArrayReverseIterator(tab);
    SvString thumbUrl = NULL;
    SvHashTable cover;
    while ((cover = (SvHashTable) SvIteratorGetNext(&it))) {
        if (!SvObjectIsInstanceOf((SvObject) cover, SvHashTable_getType()))
            continue;
        SvValue url = (SvValue) SvHashTableFind(cover, (SvObject) SVSTRING("url"));
        if (!url || !SvObjectIsInstanceOf((SvObject) url, SvValue_getType()) || !SvValueIsString(url))
            continue;
        thumbUrl = SvValueGetString(url);
        break;
    }

    return thumbUrl;
}

SvString
QBMovieUtilsGetAttr(SvDBRawObject movie, const char *attr, SvString defaultValue)
{
    SvValue val;
    if (movie && (val = (SvValue) SvDBRawObjectGetAttrValue(movie, attr))) {
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsString(val))
            return SvValueGetString(val);
    }
    return defaultValue;
}

SvLocal SvString
QBMovieUtilsGetStringFormHashTable(SvHashTable table, SvString typeName, SvString type, SvString valueName)
{
    SvString result = NULL;
    SvValue typeVal = (SvValue) SvHashTableFind(table, (SvObject) typeName);
    SvValue valueVal = (SvValue) SvHashTableFind(table, (SvObject) valueName);

    if (typeVal && SvObjectIsInstanceOf((SvObject) typeVal, SvValue_getType()) && SvValueIsString(typeVal)) {
        if (!SvStringEqualToCString(type, SvValueGetStringAsCString(typeVal, NULL))) {
            goto fini;
        }
    } else {
        goto fini;
    }

    if (valueVal && SvObjectIsInstanceOf((SvObject) valueVal, SvValue_getType()) && SvValueIsString(valueVal)) {
        result = SvValueGetString(valueVal);
    }

fini:
    return result;
}

SvArray QBMovieUtilsCreateArrayFromHashTable(SvDBRawObject movie, const char *tableName, SvString typeName, SvString type, SvString value)
{
    SvArray result = SvArrayCreate(NULL);
    SvArray attrGroup = (SvArray) SvDBRawObjectGetAttrValue((SvDBRawObject) movie, tableName);
    if (attrGroup && SvObjectIsInstanceOf((SvObject) attrGroup, SvArray_getType())) {
        SvIterator it = SvArrayIterator(attrGroup);
        SvHashTable category;
        while ((category = (SvHashTable) SvIteratorGetNext(&it))) {
            if (!SvObjectIsInstanceOf((SvObject) category, SvHashTable_getType()))
                continue;
            SvString field = QBMovieUtilsGetStringFormHashTable(category, typeName, type, value);
            SvArrayAddObject(result, (SvObject) field);
        }
    }
    return result;
}

SvString
QBMovieUtilsMapString (SvString text)
{
    if(!svSettingsIsWidgetDefined("StringMap"))
        return SVRETAIN(text);

    const char * mappedString = svSettingsGetString("StringMap", SvStringCString(text));
    if (!mappedString) {
        return SVRETAIN(text);
    }

    SvString outputString = SvStringCreateWithFormat("%s", gettext(mappedString));
    return outputString;
}

int
QBMovieUtilsMapStarRating(SvString rating)
{
    if (!rating)
        return 0;
    if (!svSettingsIsWidgetDefined("StarRatingMap"))
        return 0;
    return svSettingsGetInteger("StarRatingMap", SvStringCString(rating), 0);
}

SvString
QBMovieUtilsCreateStrFromAttrArray(SvDBRawObject movie, const char *attr, SvString defaultValue)
{
    SvArray attrGroup = (SvArray) SvDBRawObjectGetAttrValue((SvDBRawObject) movie, attr);
    SvStringBuffer buffer = SvStringBufferCreate(NULL);
    if (attrGroup && SvObjectIsInstanceOf((SvObject) attrGroup, SvArray_getType())) {
        SvIterator it = SvArrayReverseIterator(attrGroup);
        SvValue vAttr = (SvValue) SvIteratorGetNext(&it);
        while (vAttr) {
            if (SvObjectIsInstanceOf((SvObject) vAttr, SvValue_getType()) && SvValueIsString(vAttr)) {
                SvString mapedString = QBMovieUtilsMapString(SvValueGetString(vAttr));
                SvStringBufferAppendFormatted(buffer, NULL, "%s", SvStringCString(mapedString));
                SVRELEASE(mapedString);
            }
            if ((vAttr = (SvValue) SvIteratorGetNext(&it))) {
                SvStringBufferAppendCString(buffer, ", ", NULL);
            } else
                break;
        }
    }

    SvString result = SvStringBufferCreateContentsString(buffer, NULL);
    SVRELEASE(buffer);
    return result;
}

SvString
QBMovieUtilsCreateFormattedStrFromAttrArray(SvDBRawObject metaData, const char *field, size_t maxLines, const char* separator)
{
    SvString ret = NULL;
    const char* entrySeparator = separator;
    if (!entrySeparator)
        entrySeparator = "\n";
    SvObject castArray = SvDBRawObjectGetAttrValue(metaData, field);
    if (!castArray || !SvObjectIsInstanceOf(castArray, SvArray_getType())) {
        goto fini;
    }
    SvArray cast = (SvArray) castArray;
    char *str = NULL, *tmp;
    size_t cnt = SvArrayCount(cast);
    size_t idx = 0;
    if (cnt > maxLines) {
        cnt = maxLines;
    }
    for (idx = 0; idx < cnt; idx++) {
        SvValue name = (SvValue) SvArrayObjectAtIndex(cast, idx);
        if (name && SvValueIsString(name)) {
            if (str) {
                asprintf(&tmp, "%s%s%s", str, entrySeparator, SvStringCString(SvValueGetString(name)));
                free(str);
                str = tmp;
            } else {
                str = strdup(SvStringCString(SvValueGetString(name)));
            }
        }
    }
    if (str) {
        if (SvArrayCount(cast) > maxLines) {
            ret = SvStringCreateWithFormat("%s, ...", str);
        } else {
            ret = SvStringCreateWithFormat("%s", str);
        }
    }

    free(str);

fini:
    return ret;
}

SvString
QBMovieUtilsCreateStrWithMaxLinesFromAttrArray(SvDBRawObject metaData, const char *field, size_t maxLines)
{
    return QBMovieUtilsCreateFormattedStrFromAttrArray(metaData, field, maxLines, ", ");
}

SvString
QBMovieUtilsCreateStrFromAttrInteger(SvDBRawObject metaData, const char *attr, const char *format, SvString defaultValue)
{
    SvValue val = NULL;
    if (metaData && (val = (SvValue) SvDBRawObjectGetAttrValue(metaData, attr))) {
        if (val && SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) && SvValueIsInteger(val))
            return SvStringCreateWithFormat(format, SvValueGetInteger(val));
    }

    return defaultValue;
}
