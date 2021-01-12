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

#ifndef ISO_639_TABLE_H
#define ISO_639_TABLE_H

/**
 * Get name of a language.
 * https://en.wikipedia.org/wiki/List_of_ISO_639-3_codes
 *
 * @param code 639-3 language code
 * @return language name or NULL
 */
extern const char *iso639GetLangName(const char *code);

/**
 * Get terminological code related to given bibliographic code.
 * https://en.wikipedia.org/wiki/List_of_ISO_639-2_codes
 *
 * If language has no distinction of bibliographic and terminological codes
 * then NULL is returned.
 *
 * @param code bibliographic code
 * @return terminological code or NULL
 */
extern const char *iso639GetTerminologicalCode(const char *code);

/**
 * Get 639-3 language code corresponding to given 639-1 code
 * https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes
 *
 * @param code 639-1 language code
 * @return 639-3 language code or NULL
 */
extern const char *iso639_1to3(const char *code);

/**
 * Convert language codes to terminological forms if applicable and compare
 * them using strncmp.
 *
 * This function is safe like strncmp(code1, code2, 3);
 *
 * @param code1 639-2 language code
 * @param code2 639-2 language code
 * @return result of strncmp
 */
extern int iso639TerminologicalCompare(const char *code1, const char *code2);

#endif
