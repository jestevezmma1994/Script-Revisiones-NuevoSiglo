/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_JSON_DATA_BUILDER_H_
#define SV_JSON_DATA_BUILDER_H_

/**
 * @file SvJSONDataBuilder.h JSON data builder class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONDataBuilder JSON data builder class
 * @ingroup SvJSON
 * @{
 *
 * @image html SvJSONDataBuilder.png
 **/

/**
 * JSON data builder class.
 *
 * SvJSONDataBuilder is a basic implementation of the @ref SvJSONDataHandler.
 *
 * @class SvJSONDataBuilder
 * @extends SvObject
 **/
typedef struct SvJSONDataBuilder_ *SvJSONDataBuilder;


/**
 * Get runtime type identification object representing
 * type of JSON data builder class.
 *
 * @return JSON data builder class
 **/
extern SvType
SvJSONDataBuilder_getType(void);

/**
 * Create JSON data builder.
 *
 * @memberof SvJSONDataBuilder
 *
 * @param[in] maxDepth  max depth of the JSON object hierarchy, @c 0 for no limit
 * @param[in] parseClasses @c true to use class helpers from JSON class
 *                      registry for deserializing native objects
 * @param[out] errorOut error info
 * @return              created data builder or @c NULL in case of error
 **/
extern SvJSONDataBuilder
SvJSONDataBuilderCreate(unsigned int maxDepth,
                        bool parseClasses,
                        SvErrorInfo *errorOut);

/**
 * Configure usage of string cache in data builder.
 *
 * JSON data builder can be configured to remember some number of recently
 * used JSON member names and/or string values and use those @ref SvString
 * objects multiple times in returned object hierarchy. This feature saves
 * some memory when parsed JSON documents contain a lot of repeated strings,
 * at the expense of higher CPU usage during data building process.
 *
 * @memberof SvJSONDataBuilder
 *
 * @param[in] self      JSON data builder handle
 * @param[in] maxCachedNamesCount max number of JSON object member names
 *                      to keep in cache, @c 0 to disable names caching
 * @param[in] maxCachedValuesCount max number of string values to keep
 *                      in cache, @c 0 to disable values caching
 * @param[out] errorOut error info
 **/
extern void
SvJSONDataBuilderSetupStringCache(SvJSONDataBuilder self,
                                  size_t maxCachedNamesCount,
                                  size_t maxCachedValuesCount,
                                  SvErrorInfo *errorOut);

/**
 * Prepare data builder to be used for building another JSON document.
 *
 * This method restores the internal state of the data builder so it can be
 * reused for parsing another JSON document. It won't flush the string cache,
 * so it is especially useful when parsing similar objects.
 *
 * @memberof SvJSONDataBuilder
 *
 * @param[in] self      JSON data builder handle
 * @param[out] errorOut error info
 **/
extern void
SvJSONDataBuilderReset(SvJSONDataBuilder self,
                       SvErrorInfo *errorOut);

/**
 * Get object hierarchy built as a result of parsing data.
 *
 * @memberof SvJSONDataBuilder
 *
 * @param[in] self      JSON data builder handle
 * @param[out] errorOut error info
 * @return              object hierarchy equivalent to parsed JSON document,
 *                      @c NULL in case of error
 **/
extern SvObject
SvJSONDataBuilderGetData(SvJSONDataBuilder self,
                         SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
