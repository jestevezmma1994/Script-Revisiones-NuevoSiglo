/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SvFoundation_SvString_h
#define SvFoundation_SvString_h

/**
 * @file SvString.h String class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <string.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvString String class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * SvString class internals.
 * @private
 **/
struct SvString_ {
    /// super class
    struct SvObject_ super_;
    /**
     * string length in bytes, without terminating NULL character
     * @private
     **/
    size_t length;
    /**
     * actual NULL-terminated string
     * @private
     **/
    const char *text;
};

/**
 * Encoding of a C string to create SvString instance from.
 **/
typedef enum {
    /// ASCII encoding: C string can only contain 7 bit characters
    SvStringEncoding_ASCII,
    /// full UTF-8 encoding
    SvStringEncoding_UTF8
} SvStringEncoding;


/**
 * Get runtime type identification object representing SvString class.
 *
 * @return string class
 **/
extern SvType
SvString_getType(void) __attribute__ ((const));

/**
 * Create an immutable string with contents of existing C string.
 *
 * @memberof SvString
 *
 * @param[in] cstr C string in UTF-8 encoding
 * @param[out] errorOut error info
 * @return created string or @c NULL in case of error
 **/
extern SvString
SvStringCreate(const char *cstr,
               SvErrorInfo *errorOut);

/**
 * Create an immutable empty string.
 *
 * @memberof SvString
 *
 * @param[out] errorOut error info
 * @return created string or @c NULL in case of error
 **/
static inline SvString
SvStringCreateEmpty(SvErrorInfo *errorOut)
{
    return SvStringCreate("", errorOut);
}

/**
 * Create an immutable string based on existing C string and length.
 *
 * @memberof SvString
 *
 * @param[in] cstr NULL-terminated string
 * @param[in] length string length in bytes
 * @param[out] errorOut error info
 * @return created string or @c NULL in case of error
 **/
extern SvString
SvStringCreateWithCStringAndLength(const char *cstr,
                                   size_t length,
                                   SvErrorInfo *errorOut);

/**
 * Create an immutable string with contents of existing C string.
 *
 * @memberof SvString
 *
 * @param[in] cstr NULL-terminated string
 * @param[in] encoding string encoding
 * @param[out] errorOut error info
 * @return created string or @c NULL in case of error
 **/
extern SvString
SvStringCreateWithCStringAndEncoding(const char *cstr,
                                     SvStringEncoding encoding,
                                     SvErrorInfo *errorOut);

/**
 * Create an immutable string based on existing C string and length.
 *
 * @memberof SvString
 *
 * @param[in] cstr NULL-terminated string
 * @param[in] encoding string encoding
 * @param[in] length string length in bytes
 * @param[out] errorOut error info
 * @return created string or @c NULL in case of error
 **/
extern SvString
SvStringCreateWithCStringAndEncodingAndLength(const char *cstr,
                                              SvStringEncoding encoding,
                                              size_t length,
                                              SvErrorInfo *errorOut);

/**
 * Create an immutable string based on format and a variable list of arguments.
 *
 * This method is an equivalent of sprintf() function.
 *
 * @memberof SvString
 *
 * @param[in] format format string
 * @return created string or @c NULL in case of error
 **/
extern SvString
SvStringCreateWithFormat(const char *format,
                         ...) __attribute__ ((format(printf, 1, 2)));

/**
 * Create an immutable string containing a substring of another string.
 *
 * @memberof SvString
 *
 * @param[in] source source string
 * @param[in] startIndex index of a first byte of the substring
 * @param[in] length substring length in bytes, @c -1 to use entire
 *                   remaining substring
 * @param[out] errorOut error info
 * @return created string or @c NULL in case of error
 **/
extern SvString
SvStringCreateSubString(SvString source,
                        size_t startIndex,
                        ssize_t length,
                        SvErrorInfo *errorOut);

/**
 * Get the length of a string.
 *
 * @memberof SvString
 *
 * @param[in] self string handle
 * @return string length in bytes, excluding terminating NULL character
 **/
extern size_t
SvStringGetLength(SvString self);

/** @cond */
static inline size_t
SvStringLength(SvString self)
{
    return SvStringGetLength(self);
}
/** @endcond */

/**
 * Get NULL-terminated C string stored in string object.
 *
 * @memberof SvString
 *
 * @param[in] self string handle
 * @return NULL-terminated C string
 **/
extern const char *
SvStringGetCString(SvString self);

/** @cond */
static inline const char *
SvStringCString(SvString self)
{
    return SvStringGetCString(self);
}
/** @endcond */

/**
 * Check if string is equal to given NULL-terminated C string.
 *
 * @memberof SvString
 *
 * @param[in] self string handle
 * @param[in] cstr NULL-terminated C string
 * @return @c true if @a cstr is equal to string object, @c false otherwise
 **/
extern bool
SvStringEqualToCString(SvString self,
                       const char *cstr);


/**
 * Macro for creating static SvString instances.
 * @hideinitializer
 **/

#ifdef __cplusplus

struct sv_string_expression_wrap__ {
    explicit inline sv_string_expression_wrap__(const char *c)
    {
        this->str = SvStringCreate(c, NULL);
    }
    inline ~sv_string_expression_wrap__()
    {
        SVRELEASE(this->str);
    }
    SvString str;
};
#define SVSTRING(txt) ((sv_string_expression_wrap__((txt))).str)

#else

#define SVSTRING(staticText) \
    ({\
         static struct SvString_ staticString_ = {\
             .super_          = {\
                 .retainCount = -1,\
                 .refList     = NULL,\
                 .magic       = SV_OBJECT_MAGIC_VALUE,\
                 .flags       = 0\
             },\
             .length          = sizeof(staticText) - 1,\
             .text            = staticText\
         };\
         staticString_.super_.type = SvString_getType();\
         &staticString_;\
     })

#endif

/** @cond */
/*
 * Warning! Value of SVAUTOSTRING() cannot be stored in any collection,
 * cannot be retained or released!
 */
#define SVAUTOSTRING(VARNAME, staticText) \
    struct SvString_ autoString_ ## VARNAME ## __LINE__ = {\
        .super_          = {\
            .type        = SvString_getType(),\
            .retainCount = -2,\
            .refList     = NULL,\
            .magic       = SV_OBJECT_MAGIC_VALUE,\
            .flags       = 0\
        },\
        .length          = strlen(staticText),\
        .text            = staticText\
    };\
    SvString VARNAME = &(autoString_ ## VARNAME ## __LINE__);
/** @endcond */

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
