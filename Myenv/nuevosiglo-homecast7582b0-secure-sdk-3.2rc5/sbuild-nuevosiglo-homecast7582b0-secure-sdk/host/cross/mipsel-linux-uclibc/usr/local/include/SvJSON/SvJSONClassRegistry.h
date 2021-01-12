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

#ifndef SV_JSON_CLASS_REGISTRY_H_
#define SV_JSON_CLASS_REGISTRY_H_

/**
 * @file SvJSONClassRegistry.h JSON class registry API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvJSON/SvJSONClassHelper.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvJSONClass JSON class description class
 * @ingroup SvJSON
 * @{
 **/

/**
 * JSON class description class.
 *
 * SvJSONClass is a basic implementation of the @ref SvJSONClassHelper.
 *
 * @class SvJSONClass
 * @extends SvObject
 **/
typedef struct SvJSONClass_ *SvJSONClass;


/**
 * JSON class serializer method.
 *
 * This method prepares JSON representation (a dictionary of named attributes)
 * of a native object. It will be called by JSON serializer for all instances
 * of a class having registered @a helper.
 *
 * @param[in] helper    SvJSONClass handle
 * @param[in] obj       handle to a native object being serialized
 * @param[in] dict      dictionary to be filled with attributes of @a obj
 * @param[out] errorOut error info
 **/
typedef void
(*SvJSONClassSerialize)(SvObject helper,
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
 * @param[in] helper    SvJSONClass handle
 * @param[in] className native class name
 * @param[in] dict      dictionary containing attributes of an object
 * @param[out] errorOut error info
 * @return              newly created native object, @c NULL in case of error
 **/
typedef SvObject
(*SvJSONClassCreateObject)(SvObject helper,
                           SvString className,
                           SvHashTable dict,
                           SvErrorInfo *errorOut);


/**
 * Get runtime type identification object representing
 * JSON class description class.
 *
 * @return JSON class description class
 **/
extern SvType
SvJSONClass_getType(void);

/**
 * Create SvJSONClass object.
 *
 * This method creates an instance of SvJSONClass.
 * At least one of the methods must be provided.
 *
 * @memberof SvJSONClass
 *
 * @param[in] serialize    serializer method or @c NULL
 * @param[in] createObject constructor method or @c NULL
 * @param[out] errorOut    error info
 * @return                 newly created SvJSONClass object
 *                         or @c NULL in case of error
 **/
extern SvJSONClass
SvJSONClassCreate(SvJSONClassSerialize serialize,
                  SvJSONClassCreateObject createObject,
                  SvErrorInfo *errorOut);

/**
 * Check if JSON class object has serializer method.
 *
 * @param[in] self         JSON class handle
 * @return                 @c true if serializer method was specified when
 *                         calling SvJSONClassCreate(), otherwise @c false
 **/
extern bool
SvJSONClassHasSerializer(SvJSONClass self);

/**
 * @}
 **/


/**
 * @defgroup SvJSONClassRegistry JSON class registry
 * @ingroup SvJSON
 * @{
 **/

/**
 * Register JSON helper for a named class.
 *
 * @param[in] className    name of the class that can be serialized
 *                         and/or deserialized using @a helper
 * @param[in] helper       handle to a class helper (an object
 *                         implementing @ref SvJSONClassHelper)
 * @param[out] errorOut    error info
 **/
extern void
SvJSONRegisterClassHelper(SvString className,
                          SvObject helper,
                          SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
