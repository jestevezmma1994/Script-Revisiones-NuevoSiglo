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

#ifndef SV_JSON_STREAM_PARSER_H_
#define SV_JSON_STREAM_PARSER_H_

/**
 * @file SvJSONStreamParser.h JSON stream parser class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONStreamParser JSON stream parser class
 * @ingroup SvJSON
 * @{
 *
 * @image html SvJSONStreamParser.png
 **/

/**
 * JSON stream parser class.
 * @class SvJSONStreamParser
 * @extends SvObject
 **/
typedef struct SvJSONStreamParser_ *SvJSONStreamParser;


/**
 * Get runtime type identification object representing
 * type of JSON stream parser class.
 *
 * @return JSON stream parser class
 **/
extern SvType
SvJSONStreamParser_getType(void);

/**
 * Create JSON stream parser.
 *
 * @memberof SvJSONStreamParser
 *
 * @param[in] handler   handle to an object implementing @ref SvJSONDataHandler,
 *                      @c NULL to use SvJSONDataBuilder with default settings
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvJSONStreamParser
SvJSONStreamParserCreate(SvObject handler,
                         SvErrorInfo *errorOut);

/**
 * Get data handler used by JSON stream parser.
 *
 * @memberof SvJSONStreamParser
 *
 * @param[in] self      JSON stream parser handle
 * @return              data handler used by the parser
 **/
extern SvObject
SvJSONStreamParserGetDataHandler(SvJSONStreamParser self);

/**
 * Parse next part of JSON document.
 *
 * @memberof SvJSONStreamParser
 *
 * @param[in] self      JSON stream parser handle
 * @param[in] data      data buffer
 * @param[in] length    data length in bytes
 * @param[out] errorOut error info
 **/
extern void
SvJSONStreamParserParseChunk(SvJSONStreamParser self,
                             const char *data,
                             size_t length,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
