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

#include <Utils/appType.h>

#include <limits.h>
#include <string.h>

#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBRecordFS/root.h>
#include <sv_tuner.h>
#include <QBConf.h>

bool QBAppTypeIsDVB(void)
{
    const char* mode = QBConfigGet("HYBRIDMODE");
    return !mode || !strcmp(mode, "DVB");
}

bool QBAppTypeIsIP(void)
{
    const char* mode = QBConfigGet("HYBRIDMODE");
    return mode && !strcmp(mode, "IP");
}

bool QBAppTypeIsSAT(void)
{
    bool isIP = QBAppTypeIsIP();
    if (isIP)
        return false;

    QBTunerType currentType = QBTunerType_unknown;
    if (!sv_tuner_get_type(0, &currentType) && currentType == QBTunerType_sat)
        return true;
    return false;
}

bool QBAppTypeIsHybrid(void)
{
    const char* mode = QBConfigGet("HYBRIDMODE");
    return mode && !strcmp(mode, "HYBRID");
}

bool QBAppTypeIsPVR(void)
{
    static bool is_pvr = false;
    static bool is_known = false;
    if (!is_known) {
        const char *dataRoot = "/tmp/qb_internal_storage";
        is_pvr = (QBRecordFSUtilsIsDeviceInternal(dataRoot) == 1);
        is_known = true;
    }
    return is_pvr;
}
