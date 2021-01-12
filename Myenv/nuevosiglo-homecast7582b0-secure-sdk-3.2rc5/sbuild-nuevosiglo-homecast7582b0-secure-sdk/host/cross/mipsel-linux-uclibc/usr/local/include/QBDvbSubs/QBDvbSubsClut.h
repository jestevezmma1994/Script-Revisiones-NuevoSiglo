/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DVB_SUBS_CLUT_H
#define QB_DVB_SUBS_CLUT_H

#include <SvFoundation/SvCoreTypes.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBDvbSubsClut_s {
  struct SvObject_ super_;

  int id;

  int cnt;
  uint32_t* tab;
};

typedef struct QBDvbSubsClut_s* QBDvbSubsClut;

/**
 * Parse Color Lookup Table from \a data.
 * \param id
 * \param data  as seen in EN 300 743, without basic headers, after the byte with version number
 * \param len  length of \a data
 * \returns parsed clut, or null on parse error.
 */
QBDvbSubsClut  QBDvbSubsClutCreate(int id, const unsigned char* data, int len);


QBDvbSubsClut  QBDvbSubsClutCreateDefault2bit(void);
QBDvbSubsClut  QBDvbSubsClutCreateDefault4bit(void);
QBDvbSubsClut  QBDvbSubsClutCreateDefault8bit(void);


// TODO: functions for decoding bitmaps.

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DVB_SUBS_CLUT_H
