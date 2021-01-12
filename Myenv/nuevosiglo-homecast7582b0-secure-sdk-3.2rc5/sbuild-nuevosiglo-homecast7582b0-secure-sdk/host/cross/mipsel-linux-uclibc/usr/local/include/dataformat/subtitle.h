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

#ifndef SV_DATA_FORMAT_SUBTITLE_H
#define SV_DATA_FORMAT_SUBTITLE_H

#ifdef __cplusplus
extern "C" {
#endif

enum QBSubtitleType_e
{
  QBSubtitleType_unknown        = 0x00,

  QBSubtitleType_txt            = 0x01,
  QBSubtitleType_txt_assoc      = 0x02,
  QBSubtitleType_vbi            = 0x03,

  QBSubtitleType_dvb            = 0x10,
  QBSubtitleType_dvb_4x3        = 0x11,
  QBSubtitleType_dvb_16x9       = 0x12,
  QBSubtitleType_dvb_221x1      = 0x13,
  QBSubtitleType_dvb_hd         = 0x14,

  QBSubtitleType_dvb_imp        = 0x20,
  QBSubtitleType_dvb_imp_4x3    = 0x21,
  QBSubtitleType_dvb_imp_16x9   = 0x22,
  QBSubtitleType_dvb_imp_221x1  = 0x23,
  QBSubtitleType_dvb_imp_hd     = 0x24,

  QBSubtitleType_sign_open      = 0x30,
  QBSubtitleType_sign_closed    = 0x31,

  QBSubtitleType_video          = 0x40,

  QBSubtitleType_cc_cea608      = 0x101,
  QBSubtitleType_cc_cea708      = 0x102,
};
typedef enum QBSubtitleType_e QBSubtitleType;

const char* QBSubtitleTypeToString(QBSubtitleType type);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_FORMAT_SUBTITLE_H
