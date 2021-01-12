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

#ifndef QB_UTF8_UTILS_H_
#define QB_UTF8_UTILS_H_

/**
 * @file QBUTF8Utils.h
 * @brief UTF-8 utilities
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h> // for ssize_t
#include <SvCore/SvCommonDefs.h>

#define UTF8_NON_BREAKING_SPACE "\xC2\xA0"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBUTF8Utils UTF-8 string processing utilities
 * @ingroup QBStringUtils
 * @{
 *
 * Functions operating on NULL-terminated C strings in UTF-8 encoding.
 **/


/** @cond */
extern unsigned int
QBUTF8StringReadChar_(const unsigned char *src,
                      const unsigned char **next);
/** @endcond */

/**
 * Read single Unicode character from UTF-8 string.
 *
 * @param[in] src       source string
 * @param[out] next     position in the @a src string after character
 * @return              character from string, @c -1 when UTF-8 sequence
 *                      is invalid
 **/
static inline unsigned int
QBUTF8StringReadChar(const char *src,
                     const char **next)
{
    const unsigned char *src_ = (const unsigned char *) src;
    if (likely(*src_ < 0x80)) {
        *next = (const char *) (src_ + 1);
        return *src_;
    }
    return QBUTF8StringReadChar_(src_, (const unsigned char **) next);
}

/** @cond */
extern unsigned char *
QBUTF8StringWriteChar_(unsigned char *dest,
                       unsigned int charCode);
/** @endcond */

/**
 * Write single Unicode character to UTF-8 string.
 *
 * @param[in] dest      destination string
 * @param[in] charCode  character to be written
 * @return              position in the @a dest string after character,
 *                      @c NULL if @a charCode is invalid
 **/
static inline char *
QBUTF8StringWriteChar(char *dest,
                      unsigned int charCode)
{
    if (likely(charCode < 0x80)) {
        *dest = (char) charCode;
        return dest + 1;
    }
    return (char *) QBUTF8StringWriteChar_((unsigned char *) dest, charCode);
}

/** @cond */
extern int
QBUTF8StringGetCharLength_(const unsigned char *str);
/** @endcond */

/**
 * Get length in bytes of single character in UTF-8 string.
 *
 * @param[in] str       UTF-8 string
 * @return              length in bytes of the first character in @a str,
 *                      @c -1 in case of error
 **/
static inline int
QBUTF8StringGetCharLength(const char *str)
{
    const unsigned char *c = (const unsigned char *) str;
    if (likely(*c < 0x80))
        return 1;
    return QBUTF8StringGetCharLength_(c);
}

/**
 * Get length in bytes of multiple characters in UTF-8 string.
 *
 * @param[in] str       UTF-8 string
 * @param[in] maxCount  max number of characters to count
 * @return              length in bytes of characters in @a str,
 *                      @c -1 in case of error
 **/
ssize_t
QBUTF8StringGetCharsLength(const char *str,
                           size_t maxCount);

/**
 * Get number of characters in UTF-8 string.
 *
 * @param[in] str       UTF-8 string
 * @return              number of characters character in @a str,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringGetLength(const char *str);

/**
 * Verify UTF-8 string.
 *
 * This function checks if all UTF-8 sequences in a string
 * are correct.
 *
 * @param[in] str       UTF-8 string
 * @param[in] len       max number of bytes in @a str to verify,
 *                      @c -1 to check entire NULL-terminated string
 * @return              number of valid characters found,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringVerify(const char *str,
                   ssize_t len);

/**
 * Convert UTF-8 string to little-endian UTF-16 string.
 *
 * @param[in] src       source UTF-8 string
 * @param[in] srcLen    max number of bytes to read from @a src,
 *                      @c -1 to convert entire NULL-terminated string
 * @param[in] dest      destination little-endian UTF-16 string
 * @param[in] destLen   max number of characters that can be written to @a dest
 * @param[in] emitBOM   @c true to start dest with byte-order mark
 * @return              number of converted characters not including terminating
 *                      NULL character (but including BOM), @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringConvertToUTF16LE(const char *src,
                             ssize_t srcLen,
                             uint16_t *dest,
                             size_t destLen,
                             bool emitBOM);

/**
 * Convert UTF-8 string to big-endian UTF-16 string.
 *
 * @param[in] src       source UTF-8 string
 * @param[in] srcLen    max number of bytes to read from @a src,
 *                      @c -1 to convert entire NULL-terminated string
 * @param[in] dest      destination big-endian UTF-16 string
 * @param[in] destLen   max number of characters that can be written to @a dest
 * @param[in] emitBOM   @c true to start dest with byte-order mark
 * @return              number of converted characters not including terminating
 *                      NULL character (but including BOM), @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringConvertToUTF16BE(const char *src,
                             ssize_t srcLen,
                             uint16_t *dest,
                             size_t destLen,
                             bool emitBOM);

/**
 * Convert UTF-8 string to UTF-16 string in native endiannes.
 *
 * @param[in] src       source UTF-8 string
 * @param[in] srcLen    max number of bytes to read from @a src,
 *                      @c -1 to convert entire NULL-terminated string
 * @param[in] dest      destination UTF-16 string
 * @param[in] destLen   max number of characters that can be written to @a dest
 * @param[in] emitBOM   @c true to start dest with byte-order mark
 * @return              number of converted characters not including terminating
 *                      NULL character (but including BOM), @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringConvertToUTF16(const char *src,
                           ssize_t srcLen,
                           uint16_t *dest,
                           size_t destLen,
                           bool emitBOM);

/**
 * Convert UTF-32 string to UTF-8 string.
 *
 * @param[in] dest      destination UTF-8 string
 * @param[in] destLen   max number of bytes that can be written to @a dest
 * @param[in] src       source UTF-32 string
 * @param[in] srcLen    max number of characters to read from @a src,
 *                      @c -1 to convert entire NULL-terminated string
 * @return              number of converted characters not including terminating
 *                      NULL character, @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringConvertFromUTF32(char *dest,
                             size_t destLen,
                             const uint32_t *src,
                             ssize_t srcLen);

/**
 * Convert UTF-8 string to UTF-32 string.
 *
 * @param[in] src       source UTF-8 string
 * @param[in] srcLen    max number of bytes to read from @a src,
 *                      @c -1 to convert entire NULL-terminated string
 * @param[in] dest      destination UTF-32 string
 * @param[in] destLen   max number of characters that can be written to @a dest
 * @return              number of converted characters not including terminating
 *                      NULL character, @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringConvertToUTF32(const char *src,
                           ssize_t srcLen,
                           uint32_t *dest,
                           size_t destLen);
/**
 * Decode escaped C string to UTF-8.
 *
 * Supported escape sequences: backslash "\\", 2 digit hex numbers "\xhh"
 *
 * @param[in]  src      source escaped C string
 * @param[in]  srcLen   length of @a src in bytes
 * @param[out] dest     destination UTF-8 string
 * @param[in]  destLen  max number of bytes that can be written to @a dest
 * @return              number of bytes written to @a dest, not including
 *                      terminating NULL character, @c -1 on error
 */
extern ssize_t
QBUTF8StringCUnescape(const char *src,
                      size_t srcLen,
                      char *dest,
                      size_t destLen);

/**
 * Verify if Unicode character code is valid.
 *
 * @param[in] charCode  character code to verify
 * @return              @c true if @a charCode is valid, @c false otherwise
 **/
static inline bool
QBUTF8StringIsValidChar(unsigned int charCode)
{
    return (likely(charCode < 0xd800u) || (charCode >= 0xe000u && charCode <= 0x10fffdu));
}

/**
 * Filter string replacing all byte sequences that are not valid UTF-8
 * characters with given replacement string.
 *
 * @param[in] src       source string to sanitize
 * @param[in] dest      destination string
 * @param[in] destLen   max number of characters that can be written to @a dest
 * @param[in] replacement string to replace invalid byte sequences with,
 *                      can be empty to just remove invalid data
 * @return              number of bytes needed to fully sanitize @a src not including
 *                      terminating NULL character (can be greater than @a destLen),
 *                      @c -1 in case of error
 **/
extern ssize_t
QBUTF8StringSanitize(const char *src,
                     char *dest,
                     size_t destLen,
                     const char *replacement);


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
