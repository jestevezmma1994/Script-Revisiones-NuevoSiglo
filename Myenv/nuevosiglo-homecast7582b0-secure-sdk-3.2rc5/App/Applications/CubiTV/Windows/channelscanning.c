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

#include <sv_tuner.h>
#include <main.h>
#include <Windows/channelscanning/manual.h>
#include <Windows/channelscanning/simple.h>
#include <Logic/ChannelScanningLogic.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <QBAppKit/QBGlobalStorage.h>
#include "channelscanning.h"

SvLocal void QBChannelScanningContextDestroy(void *self_)
{
    QBChannelScanningContext self = self_;
    SVTESTRELEASE(self->conf);
    SVTESTRELEASE(self->epgManager);
    SVTESTRELEASE(self->satellitesDB);
}

SvType QBChannelScanningContext_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBChannelScanningContextDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBChannelScanningContext",
                            sizeof(struct QBChannelScanningContext_t),
                            QBWindowContext_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

QBWindowContext QBChannelScanningContextCreate(AppGlobals appGlobals, SvString type)
{
    if(!type)
        type = QBChannelScanningLogicGetDefaultScanningMethod(appGlobals->channelScanningLogic);
    if(SvStringEqualToCString(type, "manual")) {
        return QBChannelScanningManualContextCreate(appGlobals);
    } else if(SvStringEqualToCString(type, "simple")) {
        return QBChannelScanningSimpleContextCreate(appGlobals);
    } else {
        SvLogEmergency("Unknown channel scanning screen requested");
    }

    return NULL;
}

void QBChannelScanningContextInit(QBChannelScanningContext ctx, AppGlobals appGlobals)
{
    ctx->appGlobals = appGlobals;
    QBGlobalStorage globalStorage = QBGlobalStorageGetInstance();
    QBDVBSatellitesDB satellitesDB = (QBDVBSatellitesDB)
        QBGlobalStorageGetItem(globalStorage, QBDVBSatellitesDB_getType(), NULL);

    ctx->satellitesDB = SVTESTRETAIN(satellitesDB);
    ctx->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
}

void QBChannelScanningContextFailureDialog(QBWindowContext ctx_, bool withFailureDialog)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    ctx->withFailureDialog = withFailureDialog;
}

void QBChannelScanningContextForce(QBWindowContext ctx_, bool force)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    ctx->force = force;
}

void QBChannelScanningContextForceAutostart(QBWindowContext ctx_, bool autostart)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    ctx->forceAutostart = true;
    ctx->autostart = autostart;
}

void QBChannelScanningContextForceAutosave(QBWindowContext ctx_, bool autosave)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    ctx->forceAutosave = true;
    ctx->autosave = autosave;
}

void QBChannelScanningContextForceAutomatic(QBWindowContext ctx_, bool automatic)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    ctx->forceAutomatic = true;
    ctx->automatic = automatic;
}

void QBChannelScanningContextForcePriority(QBWindowContext ctx_, int priority)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    ctx->forcePriority = true;
    ctx->priority = priority;
}

void QBChannelScanningContextLoadConf(QBWindowContext ctx_, QBChannelScanningConf conf)
{
    QBChannelScanningContext ctx = (QBChannelScanningContext) ctx_;
    SVTESTRELEASE(ctx->conf);
    ctx->conf = SVTESTRETAIN(conf);
}
