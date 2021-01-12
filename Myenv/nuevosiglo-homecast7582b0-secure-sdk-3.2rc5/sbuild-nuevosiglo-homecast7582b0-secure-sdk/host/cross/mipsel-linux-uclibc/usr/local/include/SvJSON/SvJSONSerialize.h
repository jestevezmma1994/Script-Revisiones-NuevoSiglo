/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_JSON_SERIALIZE_H_
#define SV_JSON_SERIALIZE_H_

/**
 * @file SvJSONSerialize.h JSON serialization API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvStringBuffer.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONSerialization JSON serialization
 * @ingroup SvJSON
 * @{
 **/

/**
 * Serialize object hierarchy to a JSON document.
 *
 * @param[in] rootObject root of the object hierarchy (an object implementing
 *                       @ref SvSequence or @ref SvDictionary, usually an
 *                       SvImmutableArray or SvHashTable)
 * @param[in] buffer     string buffer for JSON document
 * @param[in] serializeClasses @c true to use class helpers from JSON class
 *                       registry for serializing native objects
 * @param[out] errorOut  error info
 **/
extern void
SvJSONSerialize(SvObject rootObject,
                SvStringBuffer buffer,
                bool serializeClasses,
                SvErrorInfo *errorOut);

/**
 * Serialize object hierarchy to a JSON document with optional indentation.
 *
 * @since 1.2.7
 *
 * @param[in] rootObject root of the object hierarchy (an object implementing
 *                       @ref SvSequence or @ref SvDictionary, usually an
 *                       SvImmutableArray or SvHashTable)
 * @param[in] buffer     string buffer for JSON document
 * @param[in] serializeClasses @c true to use class helpers from JSON class
 *                       registry for serializing native objects
 * @param[in] indent     @c true to visually indent output string
 * @param[out] errorOut  error info
 **/
extern void
SvJSONSerializeExt(SvObject rootObject,
                   SvStringBuffer buffer,
                   bool serializeClasses,
                   bool indent,
                   SvErrorInfo *errorOut);

/**
 * Serialize object hierarchy to a JSON document.
 *
 * @param[in] rootObject root of the object hierarchy (an object implementing
 *                       @ref SvSequence or @ref SvDictionary, usually an
 *                       SvImmutableArray or SvHashTable)
 * @param[in] indent     @c true to visually indent output string
 * @param[out] errorOut  error info
 * @return               new JSON document as string (caller is responsible
 *                       for releasing it), @c NULL in case of error
 **/
extern SvString
SvJSONDataToString(SvObject rootObject,
                   bool indent,
                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
