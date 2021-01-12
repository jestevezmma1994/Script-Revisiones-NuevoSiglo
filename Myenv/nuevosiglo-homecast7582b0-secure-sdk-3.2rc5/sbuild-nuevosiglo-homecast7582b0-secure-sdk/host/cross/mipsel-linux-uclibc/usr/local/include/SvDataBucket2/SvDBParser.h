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

#ifndef SV_DB_PARSER_H_
#define SV_DB_PARSER_H_

/**
 * @file SvDBParser.h Data bucket XML parser class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvDataBucket2/SvDataBucket.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvDBParser Data bucket XML parser class
 * @ingroup SvDataBucket2
 * @{
 **/

/**
 * Data bucket parser class.
 * @class SvDBParser
 * @extends SvObject
 **/
typedef struct SvDBParser_ *SvDBParser;


/**
 * Get runtime type identification object representing SvDBParser class.
 *
 * @return data bucket parser class
 **/
extern SvType
SvDBParser_getType(void);

/**
 * Create data bucket parser.
 *
 * @memberof SvDBParser
 *
 * @param[in] storage   data bucket handle
 * @param[out] errorOut error info
 * @return              created parser object, @c NULL in case of error
 **/
extern SvDBParser
SvDBParserCreate(SvDataBucket storage,
                 SvErrorInfo *errorOut);

/**
 * Parse XML representation of data bucket object.
 *
 * @qb_allocator
 * @memberof SvDBParser
 *
 * @param[in] self      data bucket parser handle
 * @param[in] XML       XML representation of data bucket objects
 * @param[in] length    length of @a XML in bytes
 * @param[out] errorOut error info
 * @return              array of objects created from their XML representation,
 *                      @c NULL in case of error
 **/
extern SvArray
SvDBParserParseDB(SvDBParser self,
                  const char *XML,
                  size_t length,
                  SvErrorInfo *errorOut);

/**
 * Prepare data bucket parser for working in streaming mode.
 *
 * @memberof SvDBParser
 *
 * @param[in] self      data bucket parser handle
 * @param[in] fullReset @c true to reset underlying SAX XML parser
 * @param[out] errorOut error info
 **/
extern void
SvDBParserReset(SvDBParser self,
                bool fullReset,
                SvErrorInfo *errorOut);

/**
 * Parse part of XML representation of data bucket object.
 *
 * This method can be used for parsing data in streaming mode.
 *
 * @memberof SvDBParser
 *
 * @param[in] self      data bucket parser handle
 * @param[in] XML       XML representation of data bucket objects
 * @param[in] length    length of @a XML in bytes
 * @param[out] errorOut error info
 **/
extern void
SvDBParserParseChunk(SvDBParser self,
                     const char *XML,
                     size_t length,
                     SvErrorInfo *errorOut);

/**
 * Get data bucket parsed in streaming mode.
 *
 * @qb_allocator
 * @memberof SvDBParser
 *
 * @param[in] self      data bucket parser handle
 * @param[out] errorOut error info
 * @return              array of objects created from their XML representation,
 *                      @c NULL in case of error
 **/
extern SvArray
SvDBParserGetResults(SvDBParser self,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
