/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_HEX_H_
#define QB_HEX_H_

#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Decode hex encoded data.
 *
 * This function decodes data represented in printable form where each byte
 * is represented as a two digit hexadecimal number. You can also use
 * @a ignoreWhiteSpace parameter to ignore white space before, between and after
 * numbers (but not between two digits of a number), often used for pretty
 * printing large data blocks.
 *
 * @param[in] in    hex string
 * @param[in] inlen length of @a in in bytes
 * @param[in] out   output buffer, at least (@a inlen / 2) bytes long
 * @param[in] ignoreWhiteSpace @c true to silently ignore white space characters
 *                  on input (space, horizontal tab, new line, carriage return)
 * @return          number of bytes written to @a out, @c -1 on error
 **/
extern ssize_t QBHexDecode(const char *in, size_t inlen, unsigned char *out, bool ignoreWhiteSpace);

/**
 * Encode data to hex representation.
 *
 * @param[in] in    data to encode
 * @param[in] inlen length of @a in in bytes
 * @param[in] out   output buffer, at least (2 * @a inlen) bytes long
 * @param[in] upperCase @c true to output upper case hexadecimal digits,
 *                  @c false to output only lower case digits
 * @return          number of bytes written to @a out, @c -1 on error
 **/
extern ssize_t QBHexEncode(const unsigned char *in, size_t inlen, char *out, bool upperCase);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_HEX_H_
