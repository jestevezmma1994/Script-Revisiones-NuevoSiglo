/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_STRING_UTILS_H_
#define QB_STRING_UTILS_H_

/**
 * @file QBStringUtils.h
 * @brief Various operations on strings
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <SvCore/QBDate.h>
#include <SvCore/SvTime.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBStringUtils QBStringUtils: string handling library
 * @ingroup Utils
 *
 * QBStringUtils library defines various operations on character strings.
 **/

/**
 * @defgroup StringTestings Testing if string matches certain conditions
 * @ingroup QBStringUtils
 * @{
 *
 * Testing if string belongs to some defined subset of all possible strings.
 **/

/**
 * Test if string contains only whitespace characters.
 *
 * @param[in] str       tested string
 * @param[in] asciiMode @c true if only ASCII whitespace characters are used,
 *                      @c false for other Unicode whitespace characters
 * @return              @c 1 if @a str contains only whitespace (also for an empty string),
 *                      @c 0 otherwise,
 *                      @c -1 in case of error: when @a str is @c NULL
 *                      or is not a valid UTF-8 string
 **/
extern int
QBStringIsWhitespace(const char *str, bool asciiMode);

/**
 * @}
 **/

/**
 * @defgroup NumberFormatting Number formatting
 * @ingroup QBStringUtils
 * @{
 *
 * Printing numbers to strings and parsing numbers from strings.
 **/

/**
 * Interprets @a str C-string content as an integral number and stores it in @a res as an unsigned long long type.
 * Representation of an integral number in @a str can be formatted as a decimal, octal (prefixed with "0")
 * or hexadecimal (prefixed with "0x" or "0X") based number. An optional sign character "+" in the beginning is allowed.
 *
 * @param[in] str   source string
 * @param[out] res  reference to a variable where the converted integral number is stored
 * @return          on success returns number of characters read until the first invalid character is found,
 *                  @c -1 in case of error. If the value read is out of the range of representable values by an unsigned long long type, the function also returns -1.
 */
extern int
QBStringToULongLong(const char *str, unsigned long long *res);

/**
 * Interprets @a str C-string content as an integral number and stores it in @a res as a long long type.
 * Representation of an integral number in @a str can be formatted as a decimal, octal (prefixed with "0")
 * or hexadecimal (prefixed with "0x" or "0X") based number. An optional sign character ("+" or "-") in the beginning is allowed.
 *
 * @param[in] str   source string
 * @param[out] res  reference to a variable where the converted integral number is stored
 * @return          on success returns number of characters read until the first invalid character is found,
 *                  @c -1 in case of error. If the value read is out of the range of representable values by a long long type, the function also returns -1.
 */
extern int
QBStringToLongLong(const char *str, long long *res);

/**
 * Interprets @a str C-string content as an integral number and stores it in @a res as an unsigned int type.
 * Representation of an integral number in @a str can be formatted as a decimal, octal (prefixed with "0")
 * or hexadecimal (prefixed with "0x" or "0X") based number. An optional sign character "+" in the beginning is allowed.
 *
 * @param[in] str   source string
 * @param[out] res  reference to a variable where the converted integral number is stored
 * @return          on success returns number of characters read until the first invalid character is found,
 *                  @c -1 in case of error. If the value read is out of the range of representable values by an unsigned int type, the function also returns -1.
 */
extern int
QBStringToUInteger(const char *str, unsigned int *res);

/**
 * Interprets @a str C-string content as an integral number and stores it in @a res as a int type.
 * Representation of an integral number in @a str can be formatted as a decimal, octal (prefixed with "0")
 * or hexadecimal (prefixed with "0x" or "0X") based number. An optional sign character ("+" or "-") in the beginning is allowed.
 *
 * @param[in] str   source string
 * @param[out] res  reference to a variable where the converted integral number is stored
 * @return          on success returns number of characters read until the first invalid character is found,
 *                  @c -1 in case of error. If the value read is out of the range of representable values by a int type, the function also returns -1.
 **/
extern int
QBStringToInteger(const char *str, int *res);

/**
 * Read floating point number in locale-independent standard
 * C format from string.
 *
 * @param[in] src       source string
 * @param[out] value    result
 * @return              number of characters processed,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringToDouble(const char *src,
                 double *value);

/**
 * Print floating point number to string in locale-independent
 * standard C format the same as '%g' conversion in printf().
 *
 * @param[in] dest      output buffer
 * @param[in] value     floating point value to print
 * @param[in] maxLength size of @a dest in bytes
 * @param[in] precision number of significant digits to output
 * @return              number of characters written to @a dest,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringFromDouble(char *dest,
                   double value,
                   size_t maxLength,
                   unsigned int precision);

/**
 * @}
 **/

/**
 * @defgroup DateTimeFormatting Date/time formatting
 * @ingroup QBStringUtils
 * @{
 *
 * Printing date/time to strings and parsing date/time from strings.
 **/

/**
 * Date/time string formats.
 **/
typedef enum {
    /// unknown, reserved format. If is set it means that somebody made mistake in code.
    QBDateTimeFormat_unknown = 0,
    /// "%Y%m%d" in strptime() format
    QBDateTimeFormat_YYYYMMDD,
    /// "%Y-%m-%d" in strptime() format
    QBDateTimeFormat_YYYY_MM_DD,
    /// "%d%m%Y" in strptime() format
    QBDateTimeFormat_DDMMYYYY,
    /// "%d-%m-%Y" in strptime() format
    QBDateTimeFormat_DD_MM_YYYY,
    /// full ISO 8601 format: "%Y-%m-%dT%H:%M:%S±TZ"
    QBDateTimeFormat_ISO8601,
    /// subset of ISO 8601 (basic format) described in XML-RPC spec: "%Y%m%dT%H:%M:%S"
    QBDateTimeFormat_XMLRPC,
    /// ISO 8601 time format: "%H:%M:%S±TZ"
    QBDateTimeFormat_ISO8601TIME,
    /// "%Y-%m-%d %H:%M:%S" in strptime() format
    QBDateTimeFormat_YYYY_MM_DD_HH_MM_SS,
} QBDateTimeFormat;

/**
 * Read time from string in given format
 * (only ISO8601 and XML_RPC have valid time format specification so only those formats are supported).
 * If string happens to be whole date/time (like 1999-12-25T16:30:00+02:00)
 * only the time part will be interpreted (16:30:00+02:00)
 * Result of this function will be a time point of January the first 1970 plus a time parsed from a string
 *
 *
 * @param[in] src       source string
 * @param[in] format    time format (QBDateTimeFormat_ISO8601 or QBDateTimeFormat_XMLRPC)
 * @param[in] inLocalTime @c true if @a src represents local time, @c false
 *                      if @a src represents UTC time;  if @a src provides
 *                      such information this parameter is ignored
 * @param[out] value    result (1970-01-01 + time parsed as string)
 * @return              number of characters processed,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringToTime(const char *src,
               QBDateTimeFormat format,
               bool inLocalTime,
               SvTime *value);

/**
 * Read date from string in given format.
 *
 * @param[in] src       source string
 * @param[in] format    date format
 * @param[in] inLocalTime @c true if @a src represents local time, @c false
 *                      if @a src represents UTC time;  if @a src provides
 *                      such information this parameter is ignored
 * @param[out] value    result
 * @return              number of characters processed,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringToDate(const char *src,
               QBDateTimeFormat format,
               bool inLocalTime,
               QBDate *value);

/**
 * Read date/time from string in given format.
 *
 * @param[in] src       source string
 * @param[in] format    date/time format
 * @param[in] inLocalTime @c true if @a src represents local time, @c false
 *                      if @a src represents UTC time;  if @a src provides
 *                      such information this parameter is ignored
 * @param[out] value    result
 * @return              number of characters processed,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringToDateTime(const char *src,
                   QBDateTimeFormat format,
                   bool inLocalTime,
                   SvTime *value);

/**
 * Print date/time to string according to given format.
 *
 * @param[in] dest      output buffer
 * @param[in] value     date/time value to print
 * @param[in] format    date/time format
 * @param[in] maxLength size of @a dest in bytes
 * @param[in] inLocalTime @c true to print date/time in local time,
 *                      @c false to print UTC date/time
 * @return              number of characters written to @a dest,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringFromDateTime(char *dest,
                     SvTime value,
                     QBDateTimeFormat format,
                     size_t maxLength,
                     bool inLocalTime);

/**
 * @}
 **/

/**
 * @defgroup TimeDurationFormatting Time duration formatting
 * @ingroup QBStringUtils
 * @{
 *
 * Printing time duration to strings and parsing time duration from strings.
 **/

/**
 * Time duration string formats.
 **/
typedef enum {
    /// unknown, reserved format. If is set it means that somebody made mistake in code.
    QBTimeDurationFormat_unknown = 0,
    /// partial ISO 8601 time duration format: "PT%H%M%S"
    QBTimeDurationFormat_ISO8601_TIME
} QBTimeDurationFormat;

/**
 * Read time duration from string in given format.
 *
 * @param[in]  src      source string
 * @param[in]  format   time duration format
 * @param[out] value    result
 * @return              number of characters processed,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringToTimeDuration(const char *src,
                       QBTimeDurationFormat format,
                       SvTime *value);

/**
 * Print time duration to string according to given format.
 *
 * @param[in] dest      output buffer
 * @param[in] value     time duration value to print
 * @param[in] format    time duration format
 * @param[in] maxLength size of @a dest in bytes
 * @return              number of characters written to @a dest,
 *                      @c -1 in case of error
 **/
extern ssize_t
QBStringFromTimeDuration(char *dest,
                         SvTime value,
                         QBTimeDurationFormat format,
                         size_t maxLength);

/**
 * @}
 **/

/**
 * @defgroup PathProcessing UNIX path processing
 * @ingroup QBStringUtils
 * @{
 *
 * Parsing and processing UNIX file paths.
 **/

/**
 * UNIX file path parsed into components.
 **/
typedef struct QBPathComponents_ {
    /// number of elements in @a components array
    size_t count;
    /// array of UNIX path components
    const char **components;
} *QBPathComponents;

/**
 * Normalize UNIX file path and divide it into directory/file name components.
 *
 * This function parses UNIX file path and returns it as an array of path
 * components. If @a path is relative, it is converted to absolute before
 * parsing. Returned array of components does not contain '.' nor '..'.
 *
 * @param[in] path      UNIX file path
 * @return              parsed path (caller is responsible for freeing it),
 *                      @c NULL in case of error
 **/
extern QBPathComponents
QBStringParsePath(const char *path);

/**
 * @}
 **/

/**
 * @defgroup StringTransforming String transformations
 * @ingroup QBStringUtils
 * @{
 *
 * Operations to convert one string into another.
 **/

/**
 * Replace occurences of a substring with another string.
 *
 * @param[in] dest      output buffer
 * @param[in] src       source string. If @c NULL error is returned
 * @param[in] substr    substring to be replaced. If @c NULL or empty string, error is returned
 * @param[in] replacement replacement string. If @c NULL error is returned
 * @param[in] maxCount  max number of substitutions, pass @c 0 for no limit
 * @param[in] maxLength size of @a dest in bytes. If @c 0, error is returned
 * @return              number of bytes written to @a dest or @c -1 in case of error
 **/
extern ssize_t
QBStringReplaceSubstring(char *dest,
                         const char *src,
                         const char *substr,
                         const char *replacement,
                         size_t maxCount,
                         size_t maxLength);

/**
 * Remove occurences of a substring.
 *
 * @param[in] dest      output buffer
 * @param[in] src       source string. If @c NULL error is returned
 * @param[in] substr    substring to be removed. If @c NULL or empty string, error is returned
 * @param[in] maxCount  max number of removes, pass @c 0 for no limit
 * @param[in] maxLength size of @a dest in bytes. If @c 0, error is returned
 * @return              number of bytes written to @a dest or @c -1 in case of error
 **/
static inline ssize_t
QBStringRemoveSubstring(char *dest,
                        const char *src,
                        const char *substr,
                        size_t maxCount,
                        size_t maxLength)
{
    return QBStringReplaceSubstring(dest, src, substr, "", maxCount, maxLength);
}

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
