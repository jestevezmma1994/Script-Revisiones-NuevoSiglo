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

#ifndef SV_XMLRPC_DATA_READER_H_
#define SV_XMLRPC_DATA_READER_H_

/**
 * @file SvXMLRPCDataReader.h
 * @brief XML-RPC data reader class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCDataReader XML-RPC data reader class
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC data reader class.
 * @class SvXMLRPCDataReader
 * @extends SvObject
 **/
typedef struct SvXMLRPCDataReader_ *SvXMLRPCDataReader;


/**
 * Get runtime type identification object representing
 * type of XML-RPC data reader class.
 *
 * @return XML-RPC data reader class
 **/
extern SvType
SvXMLRPCDataReader_getType(void);

/**
 * Initialize XML-RPC data reader object.
 *
 * @memberof SvXMLRPCDataReader
 *
 * @param[in] self      XML-RPC data reader handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvXMLRPCDataReader
SvXMLRPCDataReaderInit(SvXMLRPCDataReader self,
                       SvErrorInfo *errorOut);

/**
 * Get XML-RPC method call result.
 *
 * This method returns an array containing all values
 * (parameters in XML-RPC terminology) of XML-RPC method call.
 * If you need only the first value (it is the most common
 * case and is the only possibility according to XML-RPC spec),
 * use SvXMLRPCDataReaderGetDataItem() method.
 *
 * @memberof SvXMLRPCDataReader
 *
 * @param[in] self      XML-RPC data reader handle
 * @param[out] errorOut error info
 * @return              result of XML-RPC method call
 **/
extern SvArray
SvXMLRPCDataReaderGetData(SvXMLRPCDataReader self,
                          SvErrorInfo *errorOut);

/**
 * Get one of (possibly) many values returned by XML-RPC method call.
 *
 * @memberof SvXMLRPCDataReader
 *
 * @param[in] self      XML-RPC data reader handle
 * @param[in] index     value (parameter) number, starting from @c 0
 * @param[out] errorOut error info
 * @return              single value returned by XML-RPC method call
 *                      (either SvValue, SvArray or SvHashTable),
 *                      @c NULL in case of error
 **/
extern SvObject
SvXMLRPCDataReaderGetDataItem(SvXMLRPCDataReader self,
                              unsigned int index,
                              SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
