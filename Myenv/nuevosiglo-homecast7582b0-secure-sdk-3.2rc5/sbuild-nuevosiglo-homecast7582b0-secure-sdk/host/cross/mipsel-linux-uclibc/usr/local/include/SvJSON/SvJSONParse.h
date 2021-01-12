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

#ifndef SV_JSON_PARSE_H_
#define SV_JSON_PARSE_H_

/**
 * @file SvJSONParse.h JSON parsing API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONParsing JSON parsing
 * @ingroup SvJSON
 * @{
 **/

/**
 * Parse JSON string to SvFoundation objects.
 *
 * This method parses JSON data passed as @a jsonString and returns
 * an equivalent hierarchy of SvFoundation objects.
 *
 * JSON data types are mapped to objects as follows:
 *  - object is represented as SvHashTable,
 *  - array is represented as SvArray,
 *  - string is represented as SvValue of string kind,
 *  - number is represented as SvValue of integer or double kind,
 *  - boolean are represented as SvValue of boolean kind,
 *  - null value is represented as SvValue of pointer kind with @c NULL value.
 *
 * Additionally, if @a parseClasses is @c true, JSON objects with special
 * '__jsonclass__' tag, as defined by JSON-RPC 2.0 specification, are mapped
 * to native objects with the help of registered JSON class helpers.
 *
 * According to RFC4627, the toplevel value in JSON documents is either
 * an object or an array, so the returned object is an SvHashTable, SvArray
 * or @c NULL in case of parsing error.
 *
 * @param[in] jsonString  JSON document
 * @param[in] parseClasses @c true to use class helpers from JSON class
 *                        registry for deserializing native objects
 * @param[out] errorOut   error info
 * @return                object hierarchy equivalent to JSON document,
 *                        @c NULL in case of error
 **/
extern SvObject
SvJSONParseString(const char *jsonString,
                  bool parseClasses,
                  SvErrorInfo *errorOut);

/**
 * Parse JSON file to SvFoundation objects.
 *
 * This function reads file contents into memory and parses it
 * just like SvJSONParseString().
 *
 * @param[in] filePath    path to a file in JSON format
 * @param[in] parseClasses @c true to use class helpers from JSON class
 *                        registry for deserializing native objects
 * @param[out] errorOut   error info
 * @return                object hierarchy equivalent to JSON document,
 *                        @c NULL in case of error
 **/
extern SvObject
SvJSONParseFile(const char *filePath,
                bool parseClasses,
                SvErrorInfo *errorOut);

/**
 * Parse JSON document to SvFoundation objects.
 *
 * This method works in similar manner to SvJSONParseString(). It uses
 * SvJSONStreamParser internally, so it does not require NULL-terminated
 * C string.
 *
 * @param[in] jsonData    JSON document
 * @param[in] length      length of @a jsonData
 * @param[in] parseClasses @c true to use class helpers from JSON class
 *                        registry for deserializing native objects
 * @param[out] errorOut   error info
 * @return                object hierarchy equivalent to JSON document,
 *                        @c NULL in case of error
 **/
extern SvObject
SvJSONParseData(const char *jsonData,
                size_t length,
                bool parseClasses,
                SvErrorInfo *errorOut);

/**
 * Parse JSON string template in printf()-like format.
 * @deprecated This method is deprecated and you should not to use it in new code.
 *
 * This method parses JSON data passed as @a jsonTemplate, that contains
 * formatting tokens, just like formatting string in printf() function.
 * Formatting tokens can be used in place of any JSON value. They are
 * interpreted as follows:
 *  - '\%d', '\%ld', '\%lld', '\%u', '\%lu', '\%llu' and '\%f' tokens can
 *    be used to represent JSON numbers,
 *  - '\%s' token can be used to represent JSON strings,
 *  - '\%b' (not present in C) to represent JSON booleans,
 *  - '\%@' representing any objects (including JSON null value).
 *
 * @param[in] jsonTemplate JSON document template
 * @param[out] errorOut   error info
 * @return                object hierarchy equivalent to JSON document,
 *                        @c NULL in case of error
 **/
extern SvObject
SvJSONParseTemplate(const char *jsonTemplate,
                    SvErrorInfo *errorOut,
                    ...);

/**
 * Parse JSON string template in printf()-like format.
 * @deprecated This method is deprecated and you should not to use it in new code.
 *
 * This function is equivalent to SvJSONParseTemplate().
 *
 * @param[in] jsonTemplate JSON document template
 * @param[in] args        list of arguments
 * @param[out] errorOut   error info
 * @return                object hierarchy equivalent to JSON document,
 *                        @c NULL in case of error
 **/
extern SvObject
SvJSONParseTemplateV(const char *jsonTemplate,
                     va_list args,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
