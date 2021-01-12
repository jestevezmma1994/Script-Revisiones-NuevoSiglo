/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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
******************************************************************************/

#ifndef QBDRMMANAGER_INTERNAL_H_
#define QBDRMMANAGER_INTERNAL_H_

#include <QBDRMManager/QBDRMManagerListener.h>
#include <QBAppKit/QBAsyncService.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvSet.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>

struct QBDRMManager_ {
    struct SvObject_ super_;
    SvWeakList listeners;
    QBDRMManagerData data;

    int lastSessionId;                              // sessionId counter, every new session has its ID incremented
    SvSet openSessionIds;                           // set of open sessionIds
    SvHashTable sessionIdToDRMInitializationData;   // single initialization data for each session
    SvHashTable sessionIdToDRMSystemType;           // DRM system type for each session
    SvHashTable sessionManagers;                    // registered session managers
    SvHashTable sessionManagersToOpenSessionsCnt;   // session managers with opened sessions

    QBAsyncServiceState state;                      // service state for Service Registry
};

#endif /* QBDRMMANAGER_INTERNAL_H_ */
