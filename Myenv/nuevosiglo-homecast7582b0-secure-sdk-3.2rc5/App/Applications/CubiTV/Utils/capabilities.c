/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "capabilities.h"
#include <SvCore/SvLog.h>
#include <QBUtils/credentials.h>

#ifdef __linux__

#include <sys/capability.h>


int QBDropMkNodPrivileges(void)
{
    static const cap_value_t capsToKeep[] = {
        CAP_SYS_BOOT,
        CAP_SYS_TIME,
        CAP_SYS_NICE
    };
    static const unsigned int count = sizeof(capsToKeep) / sizeof(capsToKeep[0]);

    cap_t currentCaps = cap_get_proc();
    cap_t emptyCaps = cap_init();
    int diff = cap_compare(currentCaps, emptyCaps);
    cap_free(emptyCaps);
    cap_free(currentCaps);
    if (diff == 0) {
        // nothing to do: we have no capabilities
        return 0;
    }

    if (QBUtilsSetThreadCredentials("CubiTV", -1, -1, NULL, 0, capsToKeep, count) < 0) {
        SvLogError("%s(): QBUtilsSetThreadCredentials() failed", __func__);
        return -1;
    }

    QBUtilsPrintCredentials("CubiTV");

    return 0;
}

int QBDropRootPrivileges(uid_t uid, gid_t gid)
{
    static const cap_value_t capsToKeep[] = {
        CAP_SYS_BOOT,
        CAP_SYS_TIME,
        CAP_SYS_NICE,
        CAP_MKNOD
    };
    static const unsigned int count = sizeof(capsToKeep) / sizeof(capsToKeep[0]);

    if (geteuid() != 0) {
        SvLogNotice("%s(): already not a root, nothing to do", __func__);
        return 0;
    }

    if (QBUtilsSetThreadCredentials("CubiTV", uid, gid, NULL, 0, capsToKeep, count) < 0) {
        SvLogError("%s(): QBUtilsSetThreadCredentials() failed", __func__);
        return -1;
    }

    QBUtilsPrintCredentials("CubiTV");
    return 0;
}

#else // __linux__

int QBDropMkNodPrivileges(void)
{
    return 0;
}

int QBDropRootPrivileges(uid_t uid, gid_t gid)
{
    QBUtilsPrintCredentials("CubiTV");
    return 0;
}

#endif // __linux__
