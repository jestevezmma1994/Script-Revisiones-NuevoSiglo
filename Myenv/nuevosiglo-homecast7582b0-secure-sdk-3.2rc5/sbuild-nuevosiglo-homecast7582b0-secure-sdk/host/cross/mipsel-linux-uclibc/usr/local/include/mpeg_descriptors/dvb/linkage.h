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

#ifndef DVB_LINKAGE_DESC_H
#define DVB_LINKAGE_DESC_H

#include <mpeg_psi_parser.h>

#include <stdint.h>

typedef enum {
  DVB_LINKAGE_DESC_TYPE_INFORMATION_SERVICE = 0x01,
  DVB_LINKAGE_DESC_TYPE_EPG_SERVICE = 0x02,
  DVB_LINKAGE_DESC_TYPE_CA_REPLACEMENT = 0x03,
  DVB_LINKAGE_DESC_TYPE_WHOLE_BOUQUET = 0x04,
  DVB_LINKAGE_DESC_TYPE_SERVICE_REPLACEMENT_SERVICE = 0x05,
  DVB_LINKAGE_DESC_TYPE_DATA_BROADCAST_SERVICE = 0x06,
  DVB_LINKAGE_DESC_TYPE_RCS_MAP = 0x07,
  DVB_LINKAGE_DESC_TYPE_MOBILE_HAND_OVER = 0x08,
  DVB_LINKAGE_DESC_TYPE_SSU_SERVICE = 0x0A,
  DVB_LINKAGE_DESC_TYPE_IP_MAP_SERVICE = 0x0B,
  DVB_LINKAGE_DESC_TYPE_NIT_BAT_SERVICE = 0x0C,
  DVB_LINKAGE_DESC_TYPE_EVENT_LINKAGE = 0x0D,
} dvb_linkage_desc_type;

typedef struct _dvb_linkage_desc  dvb_linkage_desc;

struct _dvb_linkage_desc
{
  uint16_t tsid;
  uint16_t onid;
  uint16_t sid;
  uint8_t type;

  char language_code[4];

  const unsigned char* private_data;
  int private_data_len;
};

int dvb_linkage_desc_init(dvb_linkage_desc* desc, mpeg_data* data);
void dvb_linkage_desc_destroy(dvb_linkage_desc* desc);

void dvb_linkage_desc_report(const dvb_linkage_desc* desc, const char* indent_string);

#endif // DVB_LINKAGE_DESC_H
