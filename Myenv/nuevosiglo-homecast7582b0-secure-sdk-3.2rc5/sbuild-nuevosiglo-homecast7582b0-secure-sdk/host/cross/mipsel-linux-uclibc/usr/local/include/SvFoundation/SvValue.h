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

#ifndef SvFoundation_SvValue_h
#define SvFoundation_SvValue_h

/**
 * @file SvValue.h Value class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SvValue Value class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * Type of value stored by value object.
 **/
typedef enum {
    /// string value
    SvValueType_string = 0,
    /// integer value
    SvValueType_integer = 1,
    /// floating point value
    SvValueType_double = 2,
    /// boolean value
    SvValueType_boolean = 3,
    /// date/time value
    SvValueType_dateTime = 4,
    /**
     * NULL value
     * @since 1.12.2
     **/
    SvValueType_null = 5
} SvValueType;


/**
 * Value class.
 * @class SvValue
 * @extends SvObject
 **/
typedef struct SvValue_ *SvValue;

/**
 * SvValue class internals.
 * @private
 **/
struct SvValue_ {
    /// super class
    struct SvObject_ super_;
    /**
     * type of value stored in object
     * @private
     **/
    SvValueType type;
    /**
     * actual stored value
     * @private
     **/
    union {
        /// string value
        SvString s;
        /// integer value
        long long int i;
        /// floating point value
        double d;
        /// boolean value
        bool b;
        /// date/time value
        SvTime dateTime;
    } value;
};


/**
 * Get runtime type identification object representing SvValue class.
 *
 * @return value class
 **/
extern SvType
SvValue_getType(void);

/**
 * Create a string value.
 *
 * @memberof SvValue
 *
 * @param[in] value string to encapsulate in value object
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
extern SvValue
SvValueCreateWithString(SvString value,
                        SvErrorInfo *errorOut);

/**
 * Create a string value.
 *
 * @memberof SvValue
 *
 * @param[in] cstr C string value to encapsulate in value object
 * @param[in] encoding encoding of @a cstr
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
extern SvValue
SvValueCreateWithCStringAndEncoding(const char *cstr,
                                    SvStringEncoding encoding,
                                    SvErrorInfo *errorOut);

/**
 * Create a string value.
 *
 * This method is a convenience wrapper for SvValueCreateWithCStringAndEncoding().
 * It assumes that given C string is in UTF-8 encoding.
 *
 * @memberof SvValue
 *
 * @param[in] cstr C string value to encapsulate in value object
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
static inline SvValue
SvValueCreateWithCString(const char *cstr,
                         SvErrorInfo *errorOut)
{
    return SvValueCreateWithCStringAndEncoding(cstr, SvStringEncoding_UTF8, errorOut);
}

/**
 * Create an integer value.
 *
 * @memberof SvValue
 *
 * @param[in] value the integer value of this value object
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL if case of error
 **/
extern SvValue
SvValueCreateWithInteger(long long int value,
                         SvErrorInfo *errorOut);

/**
 * Create a floating point (double) value.
 *
 * @memberof SvValue
 *
 * @param[in] value the floating point value of this value object
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
extern SvValue
SvValueCreateWithDouble(double value,
                        SvErrorInfo *errorOut);

/**
 * Create a boolean (bool) value.
 *
 * @memberof SvValue
 *
 * @param[in] value boolean value of this value object
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
extern SvValue
SvValueCreateWithBoolean(bool value,
                         SvErrorInfo *errorOut);

/**
 * Create a date/time value from Unix time.
 *
 * @memberof SvValue
 *
 * @param[in] value date/time value of this value object
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
extern SvValue
SvValueCreateWithDateTime(SvTime value,
                          SvErrorInfo *errorOut);

/**
 * Create a NULL value.
 *
 * @memberof SvValue
 * @since 1.12.2
 *
 * @param[out] errorOut error info
 * @return created SvValue or @c NULL in case of error
 **/
extern SvValue
SvValueCreateNULL(SvErrorInfo *errorOut);

/**
 * Get type of value stored in value object.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return value type
 **/
extern SvValueType
SvValueGetType(SvValue self);

/**
 * Checks if value object holds string value.
 *
 * @param[in] self value handle
 * @return @c true if value is of string type
 *
 * @memberof SvValue
 * @since 1.12
 **/
extern bool
SvValueIsString(SvValue self);

/**
 * Get string value from value object.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return string value stored in @a value
 **/
extern SvString
SvValueGetString(SvValue self);

/**
 * Get string value from value object as C string.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @param[out] errorOut error info
 * @return C string or @c NULL on error
 **/
extern const char *
SvValueGetStringAsCString(SvValue self,
                          SvErrorInfo *errorOut);

/**
 * Checks if value object holds integer value.
 *
 * @param[in] self value handle
 * @return @c true if value is of integer type
 *
 * @memberof SvValue
 * @since 1.12
 **/
extern bool
SvValueIsInteger(SvValue self);

/**
 * Get integer value from value object casted to 'int' type.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return integer value stored in @a value
 **/
extern int
SvValueGetInteger(SvValue self);

/**
 * Get full integer value from value object.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return integer value stored in @a value
 **/
extern long long int
SvValueGetLongInteger(SvValue self);

/**
 * Checks if value object holds floating point value.
 *
 * @param[in] self value handle
 * @return @c true if value is of floating point type
 *
 * @memberof SvValue
 * @since 1.12
 **/
extern bool
SvValueIsDouble(SvValue self);

/**
 * Get floating point value from value object.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return floating point value stored in @a value
 **/
extern double
SvValueGetDouble(SvValue self);

/**
 * Checks if value object holds boolean value.
 *
 * @param[in] self value handle
 * @return @c true if value is of boolean type
 *
 * @memberof SvValue
 * @since 1.12
 **/
extern bool
SvValueIsBoolean(SvValue self);

/**
 * Get boolean value from value object.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return boolean value stored in @a value
 **/
extern bool
SvValueGetBoolean(SvValue self);

/**
 * Checks if value object holds date/time value.
 *
 * @param[in] self value handle
 * @return @c true if value is of date/time type
 *
 * @memberof SvValue
 * @since 1.12
 **/
extern bool
SvValueIsDateTime(SvValue self);

/**
 * Get date/time value from value object.
 *
 * @memberof SvValue
 *
 * @param[in] self value handle
 * @return date/time value stored in @a value
 **/
extern SvTime
SvValueGetDateTime(SvValue self);

/**
 * Checks if value object is a NULL value.
 *
 * @param[in] self value handle
 * @return @c true if value is a NULL value
 *
 * @memberof SvValue
 * @since 1.12.2
 **/
extern bool
SvValueIsNULL(SvValue self);


/*
 * Warning! Value of SVAUTOINTVALUE() cannot be stored in any collection,
 * cannot be retained or released!
 */
#define SVAUTOINTVALUE(VARNAME, intValue) \
    struct SvValue_ _tmp_ ## VARNAME ## __LINE__ = {\
        .super_          = {\
            .type        = SvValue_getType(),\
            .retainCount = -2,\
            .refList     = NULL,\
            .magic       = SV_OBJECT_MAGIC_VALUE,\
            .flags       = 0\
        },\
        .type            = SvValueType_integer,\
        .value.i         = (intValue)\
    };\
    SvValue VARNAME = &(_tmp_ ## VARNAME ## __LINE__);

/*
 * Warning! Value of SVAUTOSTRINGVALUE() cannot be stored in any collection,
 * cannot be retained or released!
 */
#define SVAUTOSTRINGVALUE(VARNAME, strExpr) \
    struct SvValue_ _tmp_ ## VARNAME ## __LINE__ = {\
        .super_          = {\
            .type        = SvValue_getType(),\
            .retainCount = -2,\
            .refList     = NULL,\
            .magic       = SV_OBJECT_MAGIC_VALUE,\
            .flags       = 0\
        },\
        .type            = SvValueType_string,\
        .value.s         = (strExpr)\
    };\
    SvValue VARNAME = &(_tmp_ ## VARNAME ## __LINE__);


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
