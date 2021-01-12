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

#ifndef SV_JSON_CLASS_HELPER_H_
#define SV_JSON_CLASS_HELPER_H_

/**
 * @file SvJSONClassHelper.h JSON class helper interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONClassHelper JSON class helper interface
 * @ingroup SvJSON
 * @{
 **/

/**
 * SvJSONClassHelper interface.
 **/
typedef const struct SvJSONClassHelper_ {
    /**
     * JSON class serializer method.
     *
     * This method prepares JSON representation (a dictionary of named attributes)
     * of a native object. It will be called by JSON serializer for all instances
     * of a class having registered this helper.
     *
     * @param[in] self_     JSON class helper handle
     * @param[in] obj       handle to a native object being serialized
     * @param[in] dict      dictionary to be filled with attributes of @a obj
     * @param[out] errorOut error info
     **/
    void (*serialize)(SvObject self_,
                      SvObject obj,
                      SvHashTable dict,
                      SvErrorInfo *errorOut);

    /**
     * JSON class constructor function.
     *
     * This function creates native object from its JSON representation,
     * i.e. a dictionary of named attributes. It will be called by JSON
     * parser for JSON objects marked with special class tag, as defined
     * by JSON-RPC 2.0 specification.
     *
     * @param[in] self_     JSON class helper handle
     * @param[in] className native class name
     * @param[in] dict      dictionary containing attributes of an object
     * @param[out] errorOut error info
     * @return              newly created native object, @c NULL in case of error
     **/
    SvObject (*createObject)(SvObject self_,
                                    SvString className,
                                    SvHashTable dict,
                                    SvErrorInfo *errorOut);
} *SvJSONClassHelper;


/**
 * Get runtime type identification object representing
 * JSON class helper interface.
 *
 * @return JSON class helper interface
 **/
extern SvInterface
SvJSONClassHelper_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
