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

#include "stringUtils.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h> // isspace

size_t
strip_whitespace(char *out, size_t outLen, const char *str)
{
    if (outLen == 0) {
        return 0;
    }

    // Trim leading space
    while (isspace(*str)) {
        str++;
    }

    // all spaces?
    if (*str == 0) {
        *out = 0;
        return 1;
    }

    // Trim trailing space
    const char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
        end--;
    }
    end++;

    // Set output size to minimum of trimmed string length and buffer size minus 1
    size_t out_size = (unsigned) (end - str) < (outLen - 1) ? (unsigned) (end - str) : (outLen - 1);

    // Copy trimmed string and add null terminator
    memcpy(out, str, out_size);
    out[out_size] = 0;

    return out_size;
}

void removeSpaces(char *str)
{
    // To keep track of non-space character count
    int count = 0;

    // Traverse the given string. If current character
    // is not space, then place it at index 'count++'
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i]; // here count is
                                   // incremented
    str[count] = '\0';
}

// BEGIN AMERELES
char * str_replace ( const char *string, const char *substr, const char *replacement)
{
    char *tok = NULL;
    char *newstr = NULL;
    char *oldstr = NULL;
    char *head = NULL;

    /* if either substr or replacement is NULL, duplicate string a let caller handle it */
    if ( substr == NULL || replacement == NULL ) return strdup (string);
    newstr = strdup (string);
    head = newstr;
    while ((tok = strstr ( head, substr )))
    {
        oldstr = newstr;
        newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
        /*failed to alloc mem, free old string and return NULL */
        if ( newstr == NULL )
        {
            free (oldstr);
            return NULL;
        }
        memcpy ( newstr, oldstr, tok - oldstr );
        memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
        memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
        memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
        /* move back head right after the last replacement */
        head = newstr + (tok - oldstr) + strlen( replacement );
        free (oldstr);
    }
    
    return newstr;
}
// END AMERELES

