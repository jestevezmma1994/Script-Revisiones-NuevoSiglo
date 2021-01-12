/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "Innov8onCutvPlugin.h"

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvPlayerKit/SvContent.h>
#include <SvEPGDataLayer/Plugins/Innov8onEPGEvent.h>
#include <CutvManager/CutvPlugin.h>

#define log_debug(fmt, ...)  do { if (0) SvLogNotice(COLBEG() "Innov8on CUTV : " fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)   do { if (1) SvLogNotice(COLBEG() "Innov8on CUTV : " fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { if (1) SvLogError( COLBEG() "Innov8on CUTV : " fmt COLEND_COL(red),  ##__VA_ARGS__); } while (0)


struct Innov8onCutvPlugin_ {
    struct SvObject_ super_;
};

SvLocal
void Innov8onCutvPlugin_start(SvGenericObject self_, SvScheduler sched)
{
}

SvLocal
void Innov8onCutvPlugin_stop(SvGenericObject self_)
{
}

SvLocal
bool Innov8onCutvPlugin_getEventAvailableTimes(SvGenericObject self_, SvEPGEvent event, time_t* startTime, time_t* endTime)
{
    log_debug("%s", __func__);

    if (!SvObjectIsInstanceOf((SvObject) event, Innov8onEPGEvent_getType()) || !((Innov8onEPGEvent) event)->catchup)
        return false;

    if (((Innov8onEPGEvent) event)->catchup->availableFrom == -1 || ((Innov8onEPGEvent) event)->catchup->availableTo == -1)
        return false;

    *startTime = ((Innov8onEPGEvent) event)->catchup->availableFrom;
    *endTime = ((Innov8onEPGEvent) event)->catchup->availableTo;

    return true;
}

SvLocal
SvContent Innov8onCutvPlugin_createContent(SvGenericObject self_, SvEPGEvent event)
{
    SvValue attrV;

    log_debug("%s", __func__);

    if (!SvObjectIsInstanceOf((SvObject) event, Innov8onEPGEvent_getType()) || !((Innov8onEPGEvent) event)->catchup)
        return NULL;

    attrV = (SvValue) SvDBRawObjectGetAttrValue(((Innov8onEPGEvent) event)->catchup->event, "source");
    if (attrV) {
        const char *curl = SvValueGetStringAsCString(attrV, NULL);
        return SvContentCreateFromCString(curl, NULL);
    }

    return NULL;
}

SvLocal
SvDBRawObject Innov8onCutvPlugin_getObject(SvGenericObject self_, SvEPGEvent event)
{
    log_debug("%s", __func__);

    if (!SvObjectIsInstanceOf((SvObject) event, Innov8onEPGEvent_getType()) || !((Innov8onEPGEvent) event)->catchup)
        return NULL;

    return ((Innov8onEPGEvent) event)->catchup->event;
}

SvType
Innov8onCutvPlugin_getType(void)
{
    static const struct CutvPlugin_s pluginMethods = {
        .start = Innov8onCutvPlugin_start,
        .stop = Innov8onCutvPlugin_stop,
        .getEventAvailableTimes = Innov8onCutvPlugin_getEventAvailableTimes,
        .createContent = Innov8onCutvPlugin_createContent,
        .getObject = Innov8onCutvPlugin_getObject,
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("Innov8onCutvPlugin",
                            sizeof(struct Innov8onCutvPlugin_), SvObject_getType(),
                            &type,
                            CutvPlugin_getInterface(), &pluginMethods,
                            NULL);
    };

    return type;
}


Innov8onCutvPlugin
Innov8onCutvPluginInit(Innov8onCutvPlugin plugin, SvErrorInfo *errorOut)
{
    return plugin;
}


