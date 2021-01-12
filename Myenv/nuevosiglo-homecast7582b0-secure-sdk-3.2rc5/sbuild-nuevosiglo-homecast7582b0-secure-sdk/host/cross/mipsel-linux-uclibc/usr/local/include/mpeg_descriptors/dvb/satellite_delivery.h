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

#ifndef DVB_SATELLITE_DELIVERY_DESC_H
#define DVB_SATELLITE_DELIVERY_DESC_H

#include <mpeg_psi_parser.h>

#include <stdint.h>

typedef struct _dvb_satellite_delivery_desc  dvb_satellite_delivery_desc;

struct _dvb_satellite_delivery_desc
{
  /// kHz
  int32_t freq;

  /// degrees * 10
  int32_t orbital_position;

  /// else: east
  bool is_west;

  /// 0: linear H
  /// 1: linear V
  /// 2: circular left
  /// 3: circular right
  uint8_t polarization;

  /// 20, 25, 35, or 0 iff unknown
  uint8_t roll_off;

  /// true: DVB-S2
  /// false: DVB-S
  bool modulation_system_s2;

  /// 0: auto
  /// 1: QPSK
  /// 2: 8PSK
  /// 3: QAM-16
  uint8_t modulation_type;

  int32_t symbol_rate;

  int fec_inner; // not parsed
};

int dvb_satellite_delivery_desc_init(dvb_satellite_delivery_desc* desc, mpeg_data* data);
void dvb_satellite_delivery_desc_destroy(dvb_satellite_delivery_desc* desc);

void dvb_satellite_delivery_desc_report(const dvb_satellite_delivery_desc* desc, const char* indent_string);

#endif // DVB_SATELLITE_DELIVERY_DESC_H
