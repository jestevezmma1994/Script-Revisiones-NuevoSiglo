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

#ifndef DVB_LOCAL_TIME_OFFSET_DESC_H
#define DVB_LOCAL_TIME_OFFSET_DESC_H

#include <stdint.h>
#include <mpeg_psi_parser.h>

typedef struct _dvb_local_time_offset_desc  dvb_local_time_offset_desc;

struct _dvb_local_time_offset_desc_item
{
  /// ISO 3166 country code or (if it is a number from '900' to '999') group of countries
  char country_code[4];
  /// 0 means none, 1 to 60 are time zones in the country from east to west
  unsigned int country_region_id;

  /// current local time offset from UTC in this country and region
  struct {
    /// 0 for positive (east from Greenwich), 1 for negative
    int polarity;
    int hour, min;
  } local_time_offset;

  /// UTC time when next local time change will take place
  struct {
    time_t days;
    int hour;
    int min;
    int sec;
  } time_of_change;

  /// local time offset from UTC after next change
  struct {
    int hour, min;
  } next_time_offset;
};

struct _dvb_local_time_offset_desc
{
  struct _dvb_local_time_offset_desc_item *offsets;
  int cnt;
};

int dvb_local_time_offset_desc_init(dvb_local_time_offset_desc* local_time_offset, mpeg_data* data);
void dvb_local_time_offset_desc_destroy(dvb_local_time_offset_desc* local_time_offset);

void dvb_local_time_offset_desc_report(dvb_local_time_offset_desc* local_time_offset, const char* indent_string);

#endif
