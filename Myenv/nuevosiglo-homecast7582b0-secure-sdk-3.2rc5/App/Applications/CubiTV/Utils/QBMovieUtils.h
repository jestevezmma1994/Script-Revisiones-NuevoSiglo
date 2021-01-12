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

#ifndef QBMOVIEUTILS_H_
#define QBMOVIEUTILS_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <CUIT/Core/types.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvDataBucket2/SvDBObject.h>


void
QBMovieUtilsSetThumbnailIntoIcon(SvWidget icon, SvObject object, const char* attrType, const char* attrName, const char* attrNameVal, size_t iconIdx);

SvString
QBMovieUtilsGetThumbnailUrl(SvArray tab, unsigned width, unsigned height);

SvString
QBMovieUtilsGetAttr(SvDBRawObject movie, const char *attr, SvString defaultValue);
SvString
QBMovieUtilsCreateStrFromAttrArray(SvDBRawObject movie, const char *attr, SvString defaultValue);
SvString
QBMovieUtilsCreateStrWithMaxLinesFromAttrArray(SvDBRawObject metaData, const char *field, size_t maxLines);
SvString
QBMovieUtilsCreateFormattedStrFromAttrArray(SvDBRawObject metaData, const char *field, size_t maxLines, const char* separator);
SvArray
QBMovieUtilsCreateArrayFromHashTable(SvDBRawObject movie, const char *tableName, SvString typeName, SvString type, SvString value);
SvString
QBMovieUtilsCreateStrFromAttrInteger(SvDBRawObject metaData, const char *attr, const char *format, SvString defaultValue);

SvString
QBMovieUtilsMapString (SvString text);

int
QBMovieUtilsMapStarRating(SvString rating);

#endif /* QBMOVIEUTILS_H_ */
