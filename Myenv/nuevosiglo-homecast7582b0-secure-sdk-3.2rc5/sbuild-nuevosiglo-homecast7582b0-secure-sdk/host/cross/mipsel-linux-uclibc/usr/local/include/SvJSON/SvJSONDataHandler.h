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

#ifndef SV_JSON_DATA_HANDLER_H_
#define SV_JSON_DATA_HANDLER_H_

/**
 * @file SvJSONDataHandler.h JSON data handler interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONDataHandler JSON data handler interface
 * @ingroup SvJSON
 * @{
 **/

/**
 * JSON data types.
 **/
typedef enum {
    /// reserved value
    SvJSONDataType_unknown = 0,
    /// JSON object type
    SvJSONDataType_object,
    /// JSON array type
    SvJSONDataType_array,
    /// string type
    SvJSONDataType_string,
    /// number type expressed as integer value
    SvJSONDataType_integer,
    /// number type expressed as floating point value
    SvJSONDataType_double,
    /// boolean type
    SvJSONDataType_boolean,
    /// null type
    SvJSONDataType_null
} SvJSONDataType;

/**
 * Union for passing values of simple JSON data types.
 **/
typedef union {
    /// string value
    struct {
        /// UTF-8 string, can be non-terminated!
        const char *str;
        /// length of string in bytes
        size_t length;
    } stringV;
    /// integer value
    long long int integerV;
    /// floating point value
    double doubleV;
    /// boolean value
    bool booleanV;
} SvJSONValue;


/**
 * SvJSONDataHandler interface.
 **/
typedef const struct SvJSONDataHandler_ {
    /**
     * Method called when new value has been found in JSON document
     *
     * @param[in] self_     data handler
     * @param[in] type      type of the JSON value
     * @param[in] v         JSON value, valid when @a type is not
     *                      an object, an array or null
     * @param[out] errorOut error info
     **/
    void (*valueFound)(SvObject self_,
                       SvJSONDataType type,
                       SvJSONValue v,
                       SvErrorInfo *errorOut);

    /**
     * Method called when the name of a new object member
     * is found in JSON document.
     *
     * @param[in] self_     data handler
     * @param[in] name      name of the object member
     * @param[out] errorOut error info
     **/
    void (*objectMemberFound)(SvObject self_,
                              const char *name,
                              SvErrorInfo *errorOut);

    /**
     * Method called when the termination of a compound value (an object
     * or an array) is found in JSON document.
     *
     * @param[in] self      data handler
     * @param[in] type      type of JSON compound value
     *                      (::SvJSONDataType_object or ::SvJSONDataType_array)
     * @param[out] errorOut error info
     * @return              @c true iff top-level compound was closed (parsing finished)
     **/
    bool (*compoundClosed)(SvObject self_,
                           SvJSONDataType type,
                           SvErrorInfo *errorOut);
} *SvJSONDataHandler;


/**
 * Get runtime type identification object representing
 * interface of JSON data handler.
 *
 * @return JSON data handler interface
 **/
extern SvInterface
SvJSONDataHandler_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
