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

#ifndef TRAXIS_WEB_CORE_PARSER_H_
#define TRAXIS_WEB_CORE_PARSER_H_

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvValue.h>

#include <stdbool.h>

/// for each key in @a keys, find object under this key in @a root, then, depending on corresponding type in @a types:
/// @c 'H' : object must be a hash-table -> root := ht, continue (or return ht, if no more keys)
/// @c 'a' : object must be an array of hash-tables -> root := a[0], continue (or return a, if no more keys)
/// @c 'A' : object must be an array -> return this array
/// @c 's' : object must be a string -> return this string
extern SvObject
TraxisWebCoreParserFindWithChain(SvHashTable root, const char* const* keys, const char* types, const char* debugKeyPrefix, bool printErrors);


extern void
TraxisWebCoreParserSerializeTime(char* buf, size_t maxlen, time_t t);

extern int
TraxisWebCoreParserDeserializeTime(const char* buf, time_t* tOut);

extern char*
TraxisWebCoreParserReplaceSubstring(const char* original, const char* substring, const char* replacement);

#endif
