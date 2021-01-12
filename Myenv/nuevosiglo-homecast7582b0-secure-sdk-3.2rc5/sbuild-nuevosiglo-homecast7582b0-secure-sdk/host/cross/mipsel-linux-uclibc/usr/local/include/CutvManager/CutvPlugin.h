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

#ifndef CUTV_PLUGIN_H
#define CUTV_PLUGIN_H

#include "CutvManager.h"
#include <stdbool.h>
#include <time.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvEPGEvent.h>


typedef const struct CutvPlugin_s {
    void (*start)(SvObject self_, SvScheduler sched);
    void (*stop)(SvObject self_);
    bool (*getEventAvailableTimes)(SvObject self_, SvEPGEvent event, time_t *startTime, time_t *endTime);
    SvContent (*createContent)(SvObject self_, SvEPGEvent event);
    SvDBRawObject (*getObject)(SvObject self_, SvEPGEvent event);
} *CutvPlugin;


extern SvInterface
CutvPlugin_getInterface(void);


#endif // #ifndef CUTV_PLUGIN_H
