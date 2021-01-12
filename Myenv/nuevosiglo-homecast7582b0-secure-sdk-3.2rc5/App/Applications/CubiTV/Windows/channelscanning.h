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


#ifndef QBCHANNELSCANNING_H_
#define QBCHANNELSCANNING_H_

#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>
#include <Widgets/customerLogo.h>
#include <QBChannelScanningConf.h>
#include <QBDVBSatellitesDB.h>

struct QBChannelScanningContext_t {
    struct QBWindowContext_t super_;
    bool force;
    bool withFailureDialog;

    QBChannelScanningConf conf;

    bool forceAutostart;
    bool autostart;

    bool forceAutosave;
    bool autosave;

    bool forcePriority;
    int priority;

    bool forceAutomatic;
    bool automatic;

    AppGlobals appGlobals;
    SvEPGManager epgManager;
    QBDVBSatellitesDB satellitesDB;
};
typedef struct QBChannelScanningContext_t* QBChannelScanningContext;

SvType QBChannelScanningContext_getType(void);

QBWindowContext QBChannelScanningContextCreate(AppGlobals appGlobals, SvString type);
void QBChannelScanningContextInit(QBChannelScanningContext ctx, AppGlobals appGlobals);
void QBChannelScanningContextFailureDialog(QBWindowContext ctx, bool withFailureDialog);
void QBChannelScanningContextForce(QBWindowContext ctx, bool force);
void QBChannelScanningContextForceAutostart(QBWindowContext ctx_, bool autostart);
void QBChannelScanningContextForceAutosave(QBWindowContext ctx_, bool autosave);
void QBChannelScanningContextForceAutomatic(QBWindowContext ctx_, bool automatic);
void QBChannelScanningContextForcePriority(QBWindowContext ctx_, int priority);
void QBChannelScanningContextLoadConf(QBWindowContext ctx_, QBChannelScanningConf conf);


#endif
