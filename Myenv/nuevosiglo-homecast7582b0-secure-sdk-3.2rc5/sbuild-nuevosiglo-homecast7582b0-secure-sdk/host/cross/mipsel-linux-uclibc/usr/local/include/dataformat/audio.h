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

#ifndef SV_DATA_FORMAT_AUDIO_H
#define SV_DATA_FORMAT_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

enum QBAudioType_e
{
  QBAudioType_unknown = 0,
  QBAudioType_effects = 1,
  QBAudioType_hearing_imp = 2,
  QBAudioType_visually_imp = 3,
};
typedef enum QBAudioType_e  QBAudioType;

const char* QBAudioTypeToString(QBAudioType type);

enum QBAudioCodecType_e
{
  QBAudioCodecType_unknown = 0,
  QBAudioCodecType_decode,
  QBAudioCodecType_passthrough,
  // must be last
  QBAudioCodecType_cnt,
};
typedef enum QBAudioCodecType_e  QBAudioCodecType;

enum QBAudioCodec_e
{
  QBAudioCodec_unknown = 0,
  QBAudioCodec_invalid,
  QBAudioCodec_lpcm,
  QBAudioCodec_mpeg,
  QBAudioCodec_wma,
  QBAudioCodec_wmapro,
  QBAudioCodec_aac,
  QBAudioCodec_heaac,
  QBAudioCodec_ac3,
  QBAudioCodec_eac3,
  QBAudioCodec_dts,
  // must be last
  QBAudioCodec_cnt,
};
typedef enum QBAudioCodec_e  QBAudioCodec;

const char* QBAudioCodecToString(QBAudioCodec codec);


enum QBAudioDualMonoMode_e
{
  QBAudioDualMonoMode_stereo = 0,
  QBAudioDualMonoMode_left,
  QBAudioDualMonoMode_right,
  QBAudioDualMonoMode_mono,
};
typedef enum QBAudioDualMonoMode_e  QBAudioDualMonoMode;

const char* QBAudioDualMonoModeToString(QBAudioDualMonoMode mode);

#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_FORMAT_AUDIO_H
