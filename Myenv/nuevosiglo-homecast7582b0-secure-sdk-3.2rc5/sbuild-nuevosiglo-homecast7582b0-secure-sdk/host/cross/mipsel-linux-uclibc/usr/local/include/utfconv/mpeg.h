/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef UTFCONV_MPEG_H_
#define UTFCONV_MPEG_H_

/**
 * @file mpeg.h Utilites for decoding text strings from DVB format
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup utfconv utfconv: utilites for decoding text strings from DVB format
 * @ingroup Utils
 * @{
 *
 * utfconv library provides support for converting text strings encoded in DVB-specific
 * format (according to ETSI EN 300 468 Annex A) to UTF-8.
 **/

/**
 * Single element of a map of encodings used by decoding heuristics.
 **/
typedef struct {
    /// ISO 639-3 language code
    const char *lang;
    /// character encoding to be used for this language
    const char *encoding;
} QBLangMapping;


/**
 * Get length of the prefix that specifies encoding of the string.
 *
 * @param[in] src       source string
 * @param[in] len       length of @a src in bytes
 * @return              prefix length in bytes
 **/
extern unsigned int
utfconv_mpeg_get_prefix_len(const unsigned char *src, size_t len);

/**
 * Setup a map of encodings used by decoding heuristics.
 *
 * @see utfconv_mpeg_to_utf8_with_pref()
 *
 * @param[in] mappings  table of mappings
 * @param[in] count     number of mappings
 */
extern void
utfconv_set_mappings(const QBLangMapping *mappings, size_t count);

/**
 * Setup a list of last resort character encordings tried while decoding
 * when all other methods have failed.
 *
 * @see utfconv_mpeg_to_utf8_with_pref()
 *
 * @param[in] encodings table of encoding names
 * @param[in] count     number of encodings
 **/
extern void
utfconv_set_fallbacks(const char *encodings[], size_t count);

/**
 * Decode text string from DVB-specific format to UTF-8.
 *
 * This function decodes text string from the format used in DVB signalling
 * (specified in ETSI EN 300 468 Annex A) to UTF-8 string. The standard
 * specifies the default encoding (ISO 6937, slightly modified by adding
 * an Euro currency symbol) and the format of special prefix that must be
 * used to signal usage of non-default encoding.
 *
 * In DVB networks scattered around the world this part of DVB standard
 * is very often violated. The most frequently encountered problem is usage
 * of one of 8-bit ISO 8859 encodings without the encoding prefix, but the
 * invalid values of the prefix are also quite common. To provide 'best effort'
 * solution this function accepts @a lang parameter that can be used to guess
 * the character encoding used in the source string in the following way:
 *
 *  - if there is no encoding prefix in @a src, @a lang is specified and there
 *    is an encoding for @a lang set up by utfconv_set_mappings(),
 *    assume that source string uses this encoding,
 *  - when source string could not be decoded from the original encoding
 *    (specified by prefix or guessed), try all fallback encodings set up
 *    by utfconv_set_fallbacks(),
 *  - if all other methods have failed, replace all non-ASCII characters
 *    with ASCII dot.
 *
 * @param[in] src       text string in DVB-specific format
 * @param[in] len       length of @a src in bytes
 * @param[in] lang      ISO 639-3 code describing language of @a src string,
 *                      @c NULL if language is unknown
 * @return              newly allocated NULL-terminated string in UTF-8 encoding
 *                      (caller is responsible for freeing it), @c NULL in case of error
 **/
extern unsigned char *
utfconv_mpeg_to_utf8_with_pref(const unsigned char *src, size_t len, const char *lang);

/**
 * Decode text string from DVB-specific format to UTF-8.
 *
 * This function is a convenience wrapper for utfconv_mpeg_to_utf8_with_pref()
 * that doesn't specify the language of the text string.
 *
 * @param[in] src       text string in DVB-specific format
 * @param[in] len       length of @a src in bytes
 * @return              newly allocated NULL-terminated string in UTF-8 encoding
 *                      (caller is responsible for freeing it), @c NULL in case of error
 **/
extern unsigned char *
utfconv_mpeg_to_utf8(const unsigned char *src, size_t len);


/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
