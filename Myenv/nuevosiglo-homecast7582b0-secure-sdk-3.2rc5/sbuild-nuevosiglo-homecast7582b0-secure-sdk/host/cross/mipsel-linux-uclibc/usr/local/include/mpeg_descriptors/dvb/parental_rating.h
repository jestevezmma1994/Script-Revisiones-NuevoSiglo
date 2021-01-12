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

#ifndef DVB_PARENTAL_RATING_DESC_H
#define DVB_PARENTAL_RATING_DESC_H

#include <mpeg_psi_parser.h>

typedef struct _dvb_parental_rating_desc  dvb_parental_rating_desc;

struct _dvb_parental_rating_desc
{
  mpeg_data* i_data;
  int cnt;
};

int dvb_parental_rating_desc_init(dvb_parental_rating_desc* desc, mpeg_data* data);
void dvb_parental_rating_desc_destroy(dvb_parental_rating_desc* desc);

int dvb_parental_rating_desc_get(const dvb_parental_rating_desc* desc, int idx, char* country_code);

void dvb_parental_rating_desc_report(const dvb_parental_rating_desc* desc, const char* indent_string);

#endif // DVB_PARENTAL_RATING_DESC_H
