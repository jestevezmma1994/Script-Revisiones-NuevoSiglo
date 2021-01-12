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

#ifndef SV_DATA_FORMAT_VIDEO_H
#define SV_DATA_FORMAT_VIDEO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum QBVideoAVCProfile_e
{
  QBVideoAVCProfile_unknown = 0,
  QBVideoAVCProfile_baseline,
  QBVideoAVCProfile_main,
  QBVideoAVCProfile_extended,
  QBVideoAVCProfile_high,
  QBVideoAVCProfile_high_10,
  QBVideoAVCProfile_high_422,
  QBVideoAVCProfile_high_444,
  QBVideoAVCProfile_cnt,
};
typedef enum QBVideoAVCProfile_e  QBVideoAVCProfile;

const char* QBVideoAVCProfileToString(QBVideoAVCProfile profile);

QBVideoAVCProfile QBVideoAVCProfileFromProfileIDC(unsigned char idc);


extern bool QBVideoIsMpeg2(const char* codec);
extern bool QBVideoIsMpeg4Part2(const char* codec);
extern bool QBVideoIsH264(const char* codec);
extern bool QBVideoIsH265(const char* codec);
extern bool QBVideoIsVc1(const char* codec);
extern bool QBVideoIsWmv(const char* codec);


#ifdef __cplusplus
}
#endif

#endif // #ifndef SV_DATA_FORMAT_VIDEO_H
