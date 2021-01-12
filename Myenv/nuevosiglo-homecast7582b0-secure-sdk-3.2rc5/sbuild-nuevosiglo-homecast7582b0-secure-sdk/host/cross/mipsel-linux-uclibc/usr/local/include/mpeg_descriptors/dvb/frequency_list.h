/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
****************************************************************************/

#ifndef FILE_MPEG_FREQUENCY_LIST_H
#define FILE_MPEG_FREQUENCY_LIST_H

#include <mpeg_psi_parser.h>

typedef enum {
    FrequencyListCodingType_Unknown = 0,
    FrequencyListCodingType_Satellite = 1,
    FrequencyListCodingType_Cable = 2,
    FrequencyListCodingType_Terrestrial = 3
} FrequencyListCodingType;

struct _mpeg_frequency_list_desc {
    FrequencyListCodingType coding_type;
    const unsigned char *data;
    unsigned len;
};

//
typedef struct _mpeg_frequency_list_desc mpeg_frequency_list_desc;

int mpeg_frequency_list_desc_init(mpeg_frequency_list_desc *desc, const unsigned char *data, unsigned len);
void mpeg_frequency_list_desc_destroy(mpeg_frequency_list_desc *desc);

/**
 * Returns frequency in kHz for given index from descriptor.
 *
 * @param[in]    descriptor
 * @patam[in]    frequency index
 * @return       frequency in kHz
 **/
unsigned mpeg_frequency_list_desc_get_frequency(mpeg_frequency_list_desc *desc, int i);
unsigned mpeg_frequency_list_desc_get_length(mpeg_frequency_list_desc *desc);

void mpeg_frequency_list_desc_report(mpeg_frequency_list_desc *desc, const char *indent_string);

#endif
