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

#ifndef FILE_MPEG_COMPONENT_DESC_H
#define FILE_MPEG_COMPONENT_DESC_H

#include <mpeg_psi_parser.h>

#define LANG_CODE_LENGTH 3

// component_descriptor

typedef struct _mpeg_component_desc mpeg_component_desc;

struct _mpeg_component_desc {
    unsigned char component_tag;
    int stream_content;
    int component_data;
    char lang_code[LANG_CODE_LENGTH + 1];
    unsigned char *text_char;
};

/**
 * Initialize component descriptor
 *
 * @param[in] desc handle to component descriptor
 * @param[in] data raw binary descriptor data
 * @return 0 on success, value < 0 on error
 */
int mpeg_component_desc_init(mpeg_component_desc *desc, mpeg_data *data);

/**
 * Get component text file.
 *
 * @param[in] desc handle to component descriptor
 * @return handle to text_char file
 */
static inline mpeg_data mpeg_component_get_text(mpeg_component_desc *desc)
{
    mpeg_data data = {
        .data   = desc->text_char,
        .length = strlen((char *) desc->text_char),
    };
    return data;
}

/**
 * Destroys component descriptor.
 *
 * @param[in] desc handle to component descriptor
 */
void mpeg_component_desc_destroy(mpeg_component_desc *desc);

/**
 * Reports information about component descriptor.
 *
 * @param[in] desc handle to component descriptor
 * @param[in] indent_string string printed before displayed information
 */
void mpeg_component_desc_report(mpeg_component_desc *desc, const char *indent_string);

#endif
/* FILE_MPEG_COMPONENT_DESC_H */
