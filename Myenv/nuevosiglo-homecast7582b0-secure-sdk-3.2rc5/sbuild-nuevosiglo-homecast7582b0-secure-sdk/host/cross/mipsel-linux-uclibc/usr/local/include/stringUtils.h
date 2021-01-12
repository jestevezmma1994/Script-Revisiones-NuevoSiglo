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

#ifndef QB_STRING_UTILS_H
#define QB_STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

/**
 * Strip trailing and leading whitespace
 *
 * @param[out] out      stripped string
 * @param[in] outLen    out buffer length
 * @param[in] str       string to be stripped
 * @return              length of the stripped string
 **/
size_t
QBStringUtilsStripWhitespace(char *out, size_t outLen, const char *str);

/**
 * Replace substring
 *
 * @param[out] dst      destination buffer
 * @param[in] src       source string
 * @param[in] rep       substring to replace
 * @param[in] repWith   replacement string
 * @param[in] maxLen    max lenght of the result string
 * @return              length of the result string or -1 in case of error
 **/
ssize_t
QBStringUtilsReplaceStr(char* dst, const char* src, const char* rep, const char* repWith, size_t maxLen);

/**
 * Replace all character occurences with the given char
 *
 * @param[out] dst      destination buffer
 * @param[in] src       source string
 * @param[in] rep       char to replace
 * @param[in] repWith   replacement char (can be '\0' - remove rep chars)
 * @param[in] maxLen    max lenght of the result string
 * @return              length of the result string or -1 in case of error
 **/
ssize_t
QBStringUtilsReplaceChar(char* dst, const char* src, char rep, char repWith, size_t maxLen);

/**
 * Test if source string contains only whitespace characters.
 *
 * @param[in] str    source string
 * @return           true if source string contains only whitespace characters or false in other case
 **/
bool
QBStringUtilsIsWhitespace(const char *str);

/**
 * Escape all occurrences of a character with a backslash.
 * Returned string must be freed by a caller of the function.
 *
 * @param[in] str    source string
 * @param[in] c      character to be escaped
 * @param[in] e      escape character
 * @return           escaped string
 **/
char *
QBStringUtilsEscapeCharacter(const char *str, char c, char e);

#endif // QB_STRING_UTILS_H
