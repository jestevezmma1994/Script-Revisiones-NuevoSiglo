/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TS_EIT_SECTION_H
#define QB_TS_EIT_SECTION_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int
QBTSEITSectionGetSID(const unsigned char* section)
{
    int r = 0;
    r |= section[3] << 8;
    r |= section[4];
    return r;
}

static inline int
QBTSEITSectionGetTSID(const unsigned char* section)
{
    int r = 0;
    r |= section[8] << 8;
    r |= section[9];
    return r;
}

static inline int
QBTSEITSectionGetONID(const unsigned char* section)
{
    int r = 0;
    r |= section[10] << 8;
    r |= section[11];
    return r;
}

static inline int
QBTSEITSectionGetSegmentLastSectionNumber(const unsigned char* section)
{
    return section[12];
}

static inline int
QBTSEITSectionGetLastTableID(const unsigned char* section)
{
    return section[13];
}

typedef struct QBTSEITSectionSniffResults_s  QBTSEITSectionSniffResults;
struct QBTSEITSectionSniffResults_s
{
  int onid;
  int tsid;
  int sid;

  int tableId;
  int sectionNum;
  int version;

  time_t firstEventStartTime;
  time_t lastEventEndTime;

};

int QBTSEITSectionSniff(QBTSEITSectionSniffResults* results, const unsigned char* buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif //QB_TS_EIT_SECTION_H
