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

#ifndef SV_XMLRPC_DATA_LISTENER_H_
#define SV_XMLRPC_DATA_LISTENER_H_

/**
 * @file SvXMLRPCDataListener.h
 * @brief XML-RPC data listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvXMLRPCDataListener XML-RPC data listener interface
 * @ingroup SvXMLRPCClient
 * @{
 **/

/**
 * XML-RPC data types.
 **/
typedef enum {
    SvXMLRPCType_unknown = 0,  ///< unknown, not set type, in most of cases it means an error in code
    SvXMLRPCType_string,       ///< Unicode string
    SvXMLRPCType_integer,      ///< 32-bit signed integer number
    SvXMLRPCType_double,       ///< double precision floating point number
    SvXMLRPCType_boolean,      ///< boolean value
    SvXMLRPCType_dateTime,     ///< ISO-8601 date/time
    SvXMLRPCType_data,         ///< BASE64-encoded binary data
    SvXMLRPCType_array,        ///< compound array type
    SvXMLRPCType_struct        ///< compound struct type
} SvXMLRPCType;

/**
 * Union for passing values of simple XML-RPC data types.
 **/
typedef union {
    const char *stringV;
    int integerV;
    double doubleV;
    bool booleanV;
    SvTime dateTimeV;
    SvData dataV;
} SvXMLRPCSimpleValue;


/**
 * SvXMLRPCDataListener interface.
 **/
typedef const struct SvXMLRPCDataListener_ {
    /**
     * Method called when new simple value has been parsed from the XML-RPC response.
     *
     * @param[in] self      data listener handle
     * @param[in] type      the type of the
     * @param[out] errorOut error info
     **/
    void (*valueFound)(SvObject self,
                       SvXMLRPCType type,
                       SvXMLRPCSimpleValue v,
                       SvErrorInfo *errorOut);

    /**
     * Method called when opening tags of a new compound value has been
     * parsed from the XML-RPC response.
     *
     * @param[in] self      data listener handle
     * @param[in] type      compound value type (array or struct)
     * @param[out] errorOut error info
     **/
    void (*compoundOpened)(SvObject self,
                           SvXMLRPCType type,
                           SvErrorInfo *errorOut);

    /**
     * Called when closing tags of the compound value have been
     * parsed from the XML-RPC response.
     *
     * @param[in] self      data listener handle
     * @param[in] type      compound value type (array or struct)
     * @param[out] errorOut error info
     **/
    void (*compoundClosed)(SvObject self,
                           SvXMLRPCType type,
                           SvErrorInfo *errorOut);

    /**
     * Method called when new struct member has been found in the
     * XML-RPC response.
     *
     * @param[in] self      data listener handle
     * @param[in] name      member name
     * @param[out] errorOut error info
     **/
    void (*structMemberFound)(SvObject self,
                              const char *name,
                              SvErrorInfo *errorOut);
} *SvXMLRPCDataListener;
#define SvXMLRPCDataListener_t SvXMLRPCDataListener_


/**
 * Get runtime type identification object representing
 * interface of XML-RPC data listener.
 *
 * @return XML-RPC data listener interface
 **/
extern SvInterface
SvXMLRPCDataListener_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
