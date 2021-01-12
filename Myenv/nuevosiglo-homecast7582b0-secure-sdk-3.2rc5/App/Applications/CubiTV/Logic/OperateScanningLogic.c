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

#include <Logic/OperateScanningLogic.h>
#include <Logic/TVLogic.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <sv_tuner.h>
#include <main.h>
#include <Services/core/QBChannelScanningConfManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <Services/BackgroundScanner.h>
#include <Services/operateScanningManager.h>
#include <Services/QBStandbyAgent.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "OperateScanningLogic"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "LogLevel", "");
    #define log_debug(fmt, ...)     do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_warning(fmt, ...)   do { if (env_log_level() >= 2) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...)     do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#else
    #define log_debug(fmt, ...)
    #define log_warning(fmt, ...)
    #define log_error(fmt, ...)
#endif /* SV_LOG_LEVEL */

// This macro returns name of variable if its value is NULL
#define getNameIfNULL(variableName) (variableName ? "" : #variableName " ")

struct QBOperateScanningLogic_ {
    struct SvObject_ super_;                                      ///< base class
    AppGlobals appGlobals;                                        ///< handle to application globals

    BackgroundScanner scanner;                                    ///< handle to background scanner
    QBOperateScanningBackgroundScanningFinishedCallback callback; ///< handle to callback function - called after scanning is finished
    SvObject callbackOwner;                                       ///< handle to callback owner
};


SvLocal bool QBOperateScanningLogicCheckMux(QBOperateScanningLogic self, unsigned int tunerNum)
{
    struct sv_tuner_state *tuner = sv_tuner_get(tunerNum);
    if (!tuner) {
        return false;
    }

    struct QBTunerStatus status;
    if (sv_tuner_get_status(tuner, &status)) {
        return false;
    }

    if (!status.full_lock) {
        return false;
    }

#if 0
    // check if we want to react on changes detected on this mux
    if (status.params.mux_id.freq < X || Y < status.params.mux_id.freq) {
        return false;
    }
#endif

    return true;
}

SvLocal void QBOperateScanningLogicDestroy(void *self_)
{
    QBOperateScanningLogic self = self_;
    SVTESTRELEASE(self->scanner);
}

SvLocal SvType QBOperateScanningLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBOperateScanningLogicDestroy
    };
    static SvType type = NULL;
    if (!type) {
        SvTypeCreateManaged("QBOperateScanningLogic",
                            sizeof(struct QBOperateScanningLogic_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return type;
}

SvLocal
void QBOperateScannignLogicBackgroundScannerCallback(void *self_, bool success)
{
    QBOperateScanningLogic self = (QBOperateScanningLogic) self_;
    if (self->callback && self->callbackOwner) {
        log_debug("Invoking callback");
        self->callback(self->callbackOwner, success);
        self->callback = NULL;
        self->callbackOwner = NULL;
    }

    if (success) {
        QBOperateScanningManagerChannelScanningFinished(self->appGlobals->operateScanningManager);
    }
}

QBOperateScanningLogic QBOperateScanningLogicCreate(AppGlobals appGlobals)
{
    if (!appGlobals) {
        log_error("%snot defined", getNameIfNULL(appGlobals));
        return NULL;
    }

    QBOperateScanningLogic self = (QBOperateScanningLogic) SvTypeAllocateInstance(QBOperateScanningLogic_getType(), NULL);
    self->appGlobals = appGlobals;
    self->scanner = BackgroundScannerCreate(appGlobals);
    self->callback = NULL;
    BackgroundScannerSetCallback(self->scanner, QBOperateScannignLogicBackgroundScannerCallback, self);
    return self;
}

bool QBOperateScanningLogicScanningNeeded(QBOperateScanningLogic self,
                                          int tunerNum,
                                          bool tableVersionChanged,
                                          QBOperateScanningBackgroundScanningFinishedCallback callback,
                                          SvObject callbackOwner)
{
    if (!callback && callbackOwner) {
        log_error("%snot defined, but callbackOwner is not NULL", getNameIfNULL(callback));
        return false;
    } else if (callback && !callbackOwner) {
        log_error("%snot defined, but callback is not NULL", getNameIfNULL(callbackOwner));
        return false;
    }

    if (!self->appGlobals->epgPlugin.DVB.channelList) {
        return false;
    }

    if (BackgroundScannerIsScanning(self->scanner)) {
        return false;
    }

    // do not scann in standby as background scanner does not have tuner reservation
    // and tuning params may change any time (e.g. epg monitor)
    if (QBStandbyAgentIsStandby(self->appGlobals->standbyAgent)) {
        return false;
    }

    if (!QBOperateScanningLogicCheckMux(self, tunerNum)) {
        return false;
    }

    if (tunerNum == -1) {
        log_debug("Try to find proper tuner number");

        if (!self->appGlobals->tvLogic) {
            log_error("Couldn't find proper tuner because %sis not defined", getNameIfNULL(self->appGlobals->tvLogic));
            return false;
        }

        tunerNum = QBTVLogicGetTunerNum(self->appGlobals->tvLogic);
        if (tunerNum == -1) {
            log_error("Wrong tuner number");
            return false;
        }
    }

    QBChannelScanningConfManager channelScanningConf = (QBChannelScanningConfManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                                                                  SVSTRING("QBChannelScanningConfManager"));
    QBChannelScanningConf conf = QBChannelScanningConfManagerGetConf(channelScanningConf, SVSTRING("background"));
    if (!conf) {
        return false;
    }

    if (tableVersionChanged) {
        self->callback = callback;
        self->callbackOwner = callbackOwner;
        BackgroundScannerStartScanning(self->scanner, tunerNum, conf);
    }

    return false;
}
