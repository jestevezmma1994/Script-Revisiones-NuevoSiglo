/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
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

#ifndef SvFoundation_SvStringBuffer_h
#define SvFoundation_SvStringBuffer_h

/**
 * @file SvStringBuffer.h String buffer class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvStringBuffer String buffer class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * String buffer class.
 * @class SvStringBuffer
 * @extends SvObject
 **/
typedef struct SvStringBuffer_ *SvStringBuffer;


/**
 * Get runtime type identification object representing SvStringBuffer class.
 *
 * @since 1.9
 *
 * @return string buffer class
 **/
extern SvType
SvStringBuffer_getType(void);

/**
 * Create new string buffer.
 *
 * @memberof SvStringBuffer
 *
 * @param[out] errorOut error info
 * @return              created object, @c NULL in case of error
 **/
extern SvStringBuffer
SvStringBufferCreate(SvErrorInfo *errorOut);

/**
 * Create new string buffer with given initial capacity.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] capacity  initial capacity
 * @param[out] errorOut error info
 * @return              created object, @c NULL in case of error
 **/
extern SvStringBuffer
SvStringBufferCreateWithCapacity(size_t capacity,
                                 SvErrorInfo *errorOut);

/**
 * Get current length of a string buffer contents.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @return              contents length in bytes, @c -1 on error
 **/
extern ssize_t
SvStringBufferGetLength(SvStringBuffer self);

/** @cond */
static inline ssize_t
SvStringBufferLength(SvStringBuffer self)
{
    return SvStringBufferGetLength(self);
}
/** @endcond */

/**
 * Get underlying storage of the string buffer.
 *
 * @note Returned value can be used only until the next call to any
 * of string buffer methods.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @return              buffer storage, @c NULL in case of error
 **/
extern const char *
SvStringBufferGetStorage(SvStringBuffer self);

/**
 * Get contents of the string buffer as a string.
 *
 * @memberof SvStringBuffer
 * @qb_allocator
 *
 * @param[in] self      string buffer handle
 * @param[out] errorOut error info
 * @return              buffer contents, @c NULL in case of error
 **/
extern SvString
SvStringBufferCreateContentsString(SvStringBuffer self,
                                   SvErrorInfo *errorOut);

/**
 * Append single Unicode character to the string buffer.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @param[in] c         Unicode character
 * @param[out] errorOut error info
 **/
extern void
SvStringBufferAppendUnicodeChar(SvStringBuffer self,
                                unsigned int c,
                                SvErrorInfo *errorOut);

/**
 * Append string to the string buffer.
 *
 * @memberof SvStringBuffer
 * @since 1.12.1
 *
 * @param[in] self      string buffer handle
 * @param[in] str       string handle
 * @param[out] errorOut error info
 **/
extern void
SvStringBufferAppendString(SvStringBuffer self,
                           SvString str,
                           SvErrorInfo *errorOut);

/**
 * Append an UTF-8 string to the string buffer.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @param[in] str       NULL-terminated UTF-8 string
 * @param[out] errorOut error info
 **/
extern void
SvStringBufferAppendCString(SvStringBuffer self,
                            const char *str,
                            SvErrorInfo *errorOut);

/**
 * Append leading part of an UTF-8 string to the string buffer.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @param[in] str       UTF-8 string
 * @param[in] numBytes  number of bytes from @a str to append
 * @param[out] errorOut error info
 **/
extern void
SvStringBufferAppendCStringWithLength(SvStringBuffer self,
                                      const char *str,
                                      size_t numBytes,
                                      SvErrorInfo *errorOut);

/**
 * Reset the string buffer to prepare it to build new string.
 *
 * @note The buffer retains current capacity.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 **/
extern void
SvStringBufferReset(SvStringBuffer self);

/**
 * Append a formatted string to the string buffer.
 *
 * @note Formatting is performed using asprintf() so any format
 *       specifiers allowed in printf() are also allowed here.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @param[out] errorOut error info
 * @param[in] fmt       printf-style formatting string
 **/
extern void
SvStringBufferAppendFormatted(SvStringBuffer self,
                              SvErrorInfo *errorOut,
                              const char *fmt,
                              ...) __attribute__ ((format(printf, 3, 4)));

/**
 * Increase the capacity of the string buffer.
 *
 * @memberof SvStringBuffer
 *
 * @param[in] self      string buffer handle
 * @param[in] neededBytes requested capacity (in bytes)
 * @param[out] errorOut error info
 **/
extern void
SvStringBufferGrow(SvStringBuffer self,
                   size_t neededBytes,
                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
