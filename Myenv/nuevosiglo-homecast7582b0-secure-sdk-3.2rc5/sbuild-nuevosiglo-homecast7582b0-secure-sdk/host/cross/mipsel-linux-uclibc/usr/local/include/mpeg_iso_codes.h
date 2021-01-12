/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef MPEG_ISO_CODES_H_
#define MPEG_ISO_CODES_H_

/**
 * @file mpeg_iso_codes.h Utilities for verifying format of language and country codes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>


/**
 * @defgroup mpeg_iso_codes Utilities for verifying format of language and country codes
 * @ingroup mpeg
 * @{
 **/

/**
 * Check if supplied string is in ISO 3166-1 alpha-3 country code format.
 *
 * This function verifies format of an ISO 3166-1 alpha-3 country code:
 * It must consist of exactly 3 ASCII letters. It doesn't check if the
 * code actually represents any country recognized by ISO 3166 standard.
 *
 * @param[in] code      ISO 3166-1 alpha-3 code to be verified
 * @return              @c true if code has correct format, @c false otherwise
 **/
extern bool mpeg_is_valid_ISO3166_country_code(const char *code);

/**
 * Check if supplied string is in DVB country code format.
 *
 * This function verifies format of a country code field present in various
 * DVB descriptors such as country availability descriptor, local time offset
 * descriptor and parental rating descriptor. Such country code can be:
 *  - an ISO 3166-1 alpha-3 country code: three ASCII letters,
 *  - a group of countries defined by ETSI TS 101 162: three ASCII digits.
 *
 * @param[in] code      DVB country code to be verified
 * @return              @c true if code has correct format, @c false otherwise
 **/
extern bool mpeg_is_valid_DVB_country_code(const char *code);

/**
 * Check if supplied string is in ISO 639-2 language code format.
 *
 * This function verifies format of an ISO 639-2 language code:
 * It must consist of exactly 3 ASCII letters. It doesn't check if the
 * code actually represents any language recognized by ISO 639 standard.
 *
 * @param[in] code      ISO 639-2 language code to be verified
 * @return              @c true if code has correct format, @c false otherwise
 **/
extern bool mpeg_is_valid_ISO639_language_code(const char *code);

/**
 * @}
 **/


#endif
