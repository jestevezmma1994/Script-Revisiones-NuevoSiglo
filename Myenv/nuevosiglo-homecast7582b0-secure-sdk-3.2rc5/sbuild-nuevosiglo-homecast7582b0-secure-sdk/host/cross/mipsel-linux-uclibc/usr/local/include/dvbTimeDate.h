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

#ifndef QB_LSTV_DVB_TIMEDATE_H
#define QB_LSTV_DVB_TIMEDATE_H

#include <SvFoundation/SvCoreTypes.h>
#include <mpeg_tables/tdt.h>
#include <mpeg_tables/tot.h>

#ifdef __cplusplus
extern "C" {
#endif

int lstvDVBTimeDateCreate(void);
int  lstvDVBTimeDateStart(int tunerCnt);
void lstvDVBTimeDateStop(void);

void lstvDVBTimeDateUpdateTime(time_t t);
void lstvDVBTimeDateNotify(void);

typedef void (*QBDVBTDTCallback)(SvObject self_, const mpeg_tdt_parser *TDT, int tunerIdx);
typedef void (*QBDVBTOTCallback)(SvObject self_, const mpeg_tot_parser *TOT, int tunerIdx);

typedef void (*QBDVBTimeDateSet)(SvObject self_, bool firstTime);
struct QBDVBTimeDateListener_t {
    QBDVBTimeDateSet timeSet;
};
typedef struct QBDVBTimeDateListener_t *QBDVBTimeDateListener;
SvInterface QBDVBTimeDateListener_getInterface(void);

void QBDVBTimeAddListener(SvObject listener);
void QBDVBTimeSetTablesCallbacks(QBDVBTDTCallback tdtCallback, QBDVBTOTCallback totCallback, SvObject target);

SvTime lstvDVBTimeDateGetLastUpdateTime(int tuner);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_LSTV_DVB_TIMEDATE_H
