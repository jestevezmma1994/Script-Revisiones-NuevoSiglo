/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_SETTINGS_PARAMS_ARRAY_H_
#define SV_SETTINGS_PARAMS_ARRAY_H_

/**
 * @file SvSettingsParamsArray.h Settings Manager parameters array class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvFont.h>
#include <QBResourceManager/SvRBObject.h>
#include <SvSettingsManager/SvSettingsManagerTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvSettingsParamsArray Settings Manager parameters array class
 * @ingroup SvSettingsManager
 * @{
 *
 * Settings Manager parameters array provides access to parameter values
 * with specialized per-type accessor methods.
 **/

/**
 * Type of parameters kept in the array.
 **/
typedef enum {
    /// special value marking unknown type
    SvSettingsParamType_unknown = 0,
    /// integer parameter type
    SvSettingsParamType_integer,
    /// boolean parameter type
    SvSettingsParamType_boolean,
    /// color parameter type
    SvSettingsParamType_color,
    /// string parameter type
    SvSettingsParamType_string,
    /// double parameter type
    SvSettingsParamType_double,
    /// bitmap parameter type
    SvSettingsParamType_bitmap,
    /// font parameter type
    SvSettingsParamType_font
} SvSettingsParamType;


/**
 * Get runtime type identification object representing SvSettingsParamsArray class.
 *
 * @return Settings Manager parameters array class
 **/
extern SvType
SvSettingsParamsArray_getType(void);

/**
 * Get type of parameters in the array.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @return              type of parameters, ::SvSettingsParamType_unknown in case of error
 **/
extern SvSettingsParamType
SvSettingsParamsArrayGetElementType(SvSettingsParamsArray self);

/**
 * Get number of parameters in the array.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @return              number of parameters, @c -1 in case of error
 **/
extern ssize_t
SvSettingsParamsArrayGetCount(SvSettingsParamsArray self);

/**
 * Get (optional) key of a parameter at given index.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter key, @c NULL if not present
 **/
extern SvString
SvSettingsParamsArrayGetKey(SvSettingsParamsArray self,
                            size_t i,
                            SvErrorInfo *errorOut);

/**
 * Get parameter at given index.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value, @c NULL in case of error
 **/
extern SvObject
SvSettingsParamsArrayGetParameter(SvSettingsParamsArray self,
                                  size_t i,
                                  SvErrorInfo *errorOut);

/**
 * Find parameter with given key.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] key       parameter key
 * @param[out] errorOut error info
 * @return              index of the parameter with @a key, @c -1 if not found
 **/
extern ssize_t
SvSettingsParamsArrayFindParameter(SvSettingsParamsArray self,
                                   SvString key,
                                   SvErrorInfo *errorOut);

/**
 * Get value of integer parameter.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value
 **/
extern int
SvSettingsParamsArrayGetInteger(SvSettingsParamsArray self,
                                size_t i,
                                SvErrorInfo *errorOut);

/**
 * Get value of boolean parameter.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value
 **/
extern bool
SvSettingsParamsArrayGetBoolean(SvSettingsParamsArray self,
                                size_t i,
                                SvErrorInfo *errorOut);

/**
 * Get value of color type parameter.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value
 **/
extern SvColor
SvSettingsParamsArrayGetColor(SvSettingsParamsArray self,
                              size_t i,
                              SvErrorInfo *errorOut);

/**
 * Get value of double type parameter.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value
 **/
extern double
SvSettingsParamsArrayGetDouble(SvSettingsParamsArray self,
                               size_t i,
                               SvErrorInfo *errorOut);

/**
 * Get value of string parameter.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value, @c NULL in case of error
 **/
extern SvString
SvSettingsParamsArrayGetString(SvSettingsParamsArray self,
                               size_t i,
                               SvErrorInfo *errorOut);

/**
 * Get resource ID of a bitmap type or font type parameter.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value, @c SV_RID_INVALID in case of error
 **/
extern SvRID
SvSettingsParamsArrayGetResourceID(SvSettingsParamsArray self,
                                   size_t i,
                                   SvErrorInfo *errorOut);

/**
 * Get value of bitmap type parameter.
 *
 * @note This method will work only for bitmaps, that can be loaded
 * synchronously, i.e. the ones that are available in local file system.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value, @c NULL in case of error
 **/
extern SvBitmap
SvSettingsParamsArrayGetBitmap(SvSettingsParamsArray self,
                               size_t i,
                               SvErrorInfo *errorOut);

/**
 * Get value of font type parameter.
 *
 * @note This method will work only for fonts, that can be loaded
 * synchronously, i.e. the ones that are available in local file system.
 *
 * @note Returned value is autoreleased, so it should be retained by
 * the caller if it needs to keep the reference.
 *
 * @memberof SvSettingsParamsArray
 *
 * @param[in] self      Settings Manager parameters array handle
 * @param[in] i         parameter index
 * @param[out] errorOut error info
 * @return              parameter value, @c NULL in case of error
 **/
extern SvFont
SvSettingsParamsArrayGetFont(SvSettingsParamsArray self,
                             size_t i,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
