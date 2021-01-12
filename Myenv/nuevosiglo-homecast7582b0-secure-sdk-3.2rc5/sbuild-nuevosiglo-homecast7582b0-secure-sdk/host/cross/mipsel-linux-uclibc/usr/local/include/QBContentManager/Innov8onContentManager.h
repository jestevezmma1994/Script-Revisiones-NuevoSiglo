/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef INNOV8ON_CONTENT_MANAGER_H_
#define INNOV8ON_CONTENT_MANAGER_H_

#include <SvCore/SvErrorInfo.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvFoundation/SvString.h>

#include <QBContentManager/Innov8onProviderParams.h>

typedef struct Innov8onContentManager_ *Innov8onContentManager;

SvType
Innov8onContentManager_getType(void);

Innov8onContentManager
Innov8onContentManagerInit(Innov8onContentManager self, SvXMLRPCServerInfo serverInfo, SvErrorInfo *errorOut);

int
Innov8onContentManagerSetParams(Innov8onContentManager self, Innov8onProviderParams params, SvErrorInfo *errorOut);

int
Innov8onContentManagerSetDeviceId(Innov8onContentManager self, SvString deviceId, SvErrorInfo *errorOut);

SvXMLRPCServerInfo
Innov8onContentManagerGetServerInfo(Innov8onContentManager self, SvErrorInfo *errorOut);

#endif

