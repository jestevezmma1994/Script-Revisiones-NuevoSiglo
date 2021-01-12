/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUTV_MANAGER_H
#define CUTV_MANAGER_H

#include <SvPlayerKit/SvEPGEvent.h>
#include <SvPlayerKit/SvContent.h>
#include <fibers/c/fibers.h>
#include <SvDataBucket2/SvDBRawObject.h>

#include <time.h>
#include <stdbool.h>


struct CutvManager_s;
typedef struct CutvManager_s  CutvManager;


extern
void CutvManagerStart(CutvManager* man, SvScheduler sched);

extern
void CutvManagerStop(CutvManager* man);

extern
bool CutvManagerGetEventAvailableTimes(CutvManager* man, SvEPGEvent event, time_t* startTime, time_t* endTime);

extern
bool CutvManagerIsEventAvailable(CutvManager* man, SvEPGEvent event, time_t now);

extern
SvContent CutvManagerCreateContent(CutvManager* man, SvEPGEvent event);

extern
SvDBRawObject CutvManagerGetObject(CutvManager* man, SvEPGEvent event);


/// TODO: asynchronously ask if we are entitled to watch the content


#endif // #ifndef CUTV_MANAGER_H
