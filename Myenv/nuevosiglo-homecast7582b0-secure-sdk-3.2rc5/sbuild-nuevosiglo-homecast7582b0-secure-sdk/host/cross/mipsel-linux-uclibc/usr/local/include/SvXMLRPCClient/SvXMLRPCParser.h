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

#ifndef SV_XMLRPC_PARSER_H_
#define SV_XMLRPC_PARSER_H_

/**
 * @file SvXMLRPCParser.h
 * @brief XML-RPC response parser class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvXMLRPCClient/SvXMLRPCDataListener.h>


/**
 * @defgroup SvXMLRPCParser XML-RPC response parser class
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC response parser class.
 * @class SvXMLRPCParser
 * @extends SvObject
 **/
typedef struct SvXMLRPCParser_ *SvXMLRPCParser;


/**
 * Get runtime type identification object representing
 * type of XML-RPC response parser class.
 *
 * @return XML-RPC response parser class
 **/
extern SvType
SvXMLRPCParser_getType(void);

/**
 * Initialize XML-RPC response parser object.
 *
 * @memberof SvXMLRPCParser
 *
 * @param[in] self      XML-RPC response parser object handle
 * @param[in] dataListener data listener object handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvXMLRPCParser
SvXMLRPCParserInit(SvXMLRPCParser self,
                   SvObject dataListener,
                   SvErrorInfo *errorOut);

/**
 * Parse next chunk of XML-RPC response.
 *
 * @memberof SvXMLRPCParser
 *
 * @param[in] self      XML-RPC response parser object handle
 * @param[in] data      data buffer
 * @param[in] length    data length in bytes
 * @param[out] errorOut error info
 **/
extern void
SvXMLRPCParserParseChunk(SvXMLRPCParser self,
                         const char *data,
                         size_t length,
                         SvErrorInfo *errorOut);

/**
 * Finish parsing of XML-RPC response.
 *
 * @memberof SvXMLRPCParser
 *
 * @param[in] self      XML-RPC response parser object handle
 * @param[out] errorOut error info
 **/
extern void
SvXMLRPCParserFinish(SvXMLRPCParser self,
                     SvErrorInfo *errorOut);

/**
 * Get exception.
 *
 * This method returns the XML-RPC exception code and description.
 *
 * @memberof SvXMLRPCParser
 *
 * @param[in] self         XML-RPC response parser object handle
 * @param[out] faultCode   XML-RPC fault code
 * @param[out] faultString XML-RPC fault string
 * @param[out] errorOut    error info
 **/
extern void
SvXMLRPCParserGetException(SvXMLRPCParser self,
                           int *faultCode,
                           SvString *faultString,
                           SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
