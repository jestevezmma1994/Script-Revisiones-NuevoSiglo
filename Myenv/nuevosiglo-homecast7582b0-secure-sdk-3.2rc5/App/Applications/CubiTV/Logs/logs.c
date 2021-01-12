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


#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include "smartcardLogs.h"
#include "inputLogs.h"
#include "tunerLogs.h"
#include "networkMonitorLogs.h"
#include "logs.h"
#include <safeFiles.h>
#include <SvJSON/SvJSONParse.h>

struct QBAppLogs_t {
    bool smartcardLogs;
    bool inputLogs;
    bool tunerLogs;
    bool networkMonitorLogs;
};
typedef struct QBAppLogs_t* QBAppLogs;

static QBAppLogs g_appLogs;

SvLocal SvArray QBAppLogsLoadList(SvString confPath)
{
    SvArray confs = NULL;
    SvArray ret = NULL;
    char *buffer = NULL;
    if (confPath) {
        QBFileToBuffer(SvStringCString(confPath), &buffer);
        if (!buffer)
            goto err;
        confs = (SvArray) SvJSONParseString(buffer, false, NULL);
        free(buffer);
        if (!confs || !SvObjectIsInstanceOf((SvObject) confs, SvArray_getType())) {
            goto err;
        }
        ret = SvArrayCreate(NULL);
        SvIterator it = SvArrayIterator(confs);
        SvValue val;
        while ((val = (SvValue) SvIteratorGetNext(&it))) {
            if (!SvObjectIsInstanceOf((SvObject) val, SvValue_getType()) || !SvValueIsString(val)) {
                goto err;
            }
            SvArrayAddObject(ret, (SvGenericObject) SvValueGetString(val));
        }
        SVRELEASE(confs);
    }
    return ret;
err:
    SVTESTRELEASE(confs);
    SVTESTRELEASE(ret);
    return NULL;
}

void QBAppLogsCreate(AppGlobals appGlobals, SvString confPath)
{
    QBAppLogs self = calloc(1, sizeof(struct QBAppLogs_t));
    g_appLogs = self;

    SvArray confs = QBAppLogsLoadList(confPath);
    if (!confs || SvArrayContainsObject(confs, (SvGenericObject) SVSTRING("QBSmartcardLog"))) {
        QBSmartcardLogCreate(SVSTRING("/tmp/syslog_prefix"));
        self->smartcardLogs = true;
    }
    if (!confs || SvArrayContainsObject(confs, (SvGenericObject) SVSTRING("QBInputLog"))) {
        QBInputLogCreate(appGlobals->controller);
        self->inputLogs = true;
    }
    if (!confs || SvArrayContainsObject(confs, (SvGenericObject) SVSTRING("QBTunerLogs"))) {
        QBTunerMonitor tunerMonitor = (QBTunerMonitor) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                   SVSTRING("QBTunerMonitor"));
        if (tunerMonitor) {
            QBTunerLogsCreate(appGlobals->scheduler, tunerMonitor);
            self->tunerLogs = true;
        }
    }
    if (!confs || SvArrayContainsObject(confs, (SvGenericObject) SVSTRING("QBNetworkMonitorLog"))) {
        QBNetworkMonitorLogCreate(appGlobals->networkMonitor);
        self->networkMonitorLogs = true;
    }

    SVTESTRELEASE(confs);
}

void QBAppLogsStart(void)
{
    QBAppLogs self = g_appLogs;
    if (self->smartcardLogs)
        QBSmartcardLogStart();
    if (self->inputLogs)
        QBInputLogStart();
    if (self->tunerLogs)
        QBTunerLogsStart();
    if (self->networkMonitorLogs)
        QBNetworkMonitorLogStart();
}

void QBAppLogsStop(void)
{
    QBAppLogs self = g_appLogs;
    if (self->inputLogs)
        QBInputLogStop();
    if (self->smartcardLogs)
        QBSmartcardLogStop();
    if (self->tunerLogs)
        QBTunerLogsStop();
    if (self->networkMonitorLogs)
        QBNetworkMonitorLogStop();
}

void QBAppLogsDestroy(void)
{
    QBAppLogs self = g_appLogs;
    if (self->inputLogs) {
        QBInputLogDestroy();
        self->inputLogs = false;
    }
    if (self->smartcardLogs) {
        QBSmartcardLogDestroy();
        self->smartcardLogs = false;
    }
    if (self->tunerLogs) {
        QBTunerLogsDestroy();
        self->tunerLogs = false;
    }
    if (self->networkMonitorLogs) {
        QBNetworkMonitorLogDestroy();
        self->networkMonitorLogs = false;
    }
    free(self);
}
