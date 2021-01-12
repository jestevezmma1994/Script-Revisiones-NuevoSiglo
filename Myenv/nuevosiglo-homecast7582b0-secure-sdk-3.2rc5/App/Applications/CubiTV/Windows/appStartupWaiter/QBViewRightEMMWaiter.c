/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2017 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBViewRightEMMWaiter.h"

#include <Windows/appStartupWaiter.h>

#include <QBViewRightOSM.h>
#include <QBViewRight.h>

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>

#include <stdbool.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBViewRightEMMWaiter"
    SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 2, moduleName "LogLevel", "0:error, 1:warning, 2:state, 3:info, 4:deep, 5:debug, 6:function");
    #define log_fun(fmt, ...)   do { if(env_log_level() >= 6) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(green),  __func__, ##__VA_ARGS__); } while(0)
    #define log_debug(fmt, ...) do { if(env_log_level() >= 5) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(green),  __func__, ##__VA_ARGS__); } while(0)
    #define log_deep(fmt, ...)  do { if(env_log_level() >= 4) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(cyan),   __func__, ##__VA_ARGS__); } while(0)
    #define log_info(fmt, ...)  do { if(env_log_level() >= 3) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(cyan),   __func__, ##__VA_ARGS__); } while(0)
    #define log_state(fmt, ...) do { if(env_log_level() >= 2) SvLogNotice(COLBEG()  moduleName " :: %s " fmt COLEND_COL(blue),   __func__, ##__VA_ARGS__); } while(0)
    #define log_warn(fmt, ...)  do { if(env_log_level() >= 1) SvLogWarning(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ##__VA_ARGS__); } while(0)
    #define log_error(fmt, ...) do { if(env_log_level() >= 0) SvLogError(COLBEG()   moduleName " :: %s " fmt COLEND_COL(red),    __func__, ##__VA_ARGS__); } while(0)
#else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_warn(fmt, ...)
    #define log_error(fmt, ...)
#endif

/**
 * After startup - ViewRightDVB library needs to be fed with some amount of EMM packets before it is able to descramble content.
 * This plugin waits until the library would see as many EMMs as it needs to start descrabling process, but no longer then
 * a specified maximum timeout.
 **/

struct QBViewRightEMMWaiter_s {
    struct SvObject_ super_;

    bool finished;
    bool infiniteTimeout;
    SvTime finishAfter;
    QBTVLogic tvLogic;
};

// ----------------- OSMListenerMethods methods -----------------------
SvLocal void QBViewRightEMMWaiterShow(SvGenericObject self_, QBViewRightOSM osm)
{
}

SvLocal void QBViewRightEMMWaiterPinCheck(SvObject self_, int sessionId, QBViewRightCheckPinSource pinSource,
                                          QBViewRightPinType pinType, QBViewRightPinTextSelector textSelector)
{
}

SvLocal void QBViewRightEMMWaiterPinOrder(SvObject self_, QBViewRightCheckPinSource pinSource, QBViewRightPinType pinType,
                                          SvString availableCredit, SvString costOfEvent)
{
}

SvLocal void QBViewRightEMMWaiterShowSmartcardMMINotification(SvObject self_, QBViewRightSmartcardMMINotificationType type)
{
}

SvLocal void QBViewRightEMMWaiterShowMMINotification(SvObject self_, QBViewRightDescramblingMMINotificationType type)
{
    log_fun();

    QBViewRightEMMWaiter self = (QBViewRightEMMWaiter) self_;

    if (unlikely(self->finished)) {
        return;
    }

    if (unlikely(type == QBViewRightDescramblingMMINotificationType_OK)) {
        self->finished = true;
    }
}

// ------------------- QBAppStartupWaiterPlugin methods ---------------------
SvLocal bool QBViewRightEMMWaiterHasFinished(SvObject self_)
{
    log_fun();

    QBViewRightEMMWaiter self = (QBViewRightEMMWaiter) self_;

    if (unlikely(self->finished)) {
        log_state("plugin has finished normally");
        return true;
    }

    if (unlikely(!self->infiniteTimeout && SvTimeCmp(SvTimeGet(), self->finishAfter) > 0)) {
        self->finished = true;
        log_state("plugin has finished due to timeout");
        return true;
    }

    return false;
}

SvLocal void QBViewRightEMMWaiterStart(SvObject self)
{
    log_fun();

    QBTVLogicResumePlaying(((QBViewRightEMMWaiter) self)->tvLogic, SVSTRING("QBViewRightEMMWaiter"));
}

SvLocal void QBViewRightEMMWaiterStop(SvObject self_)
{
}

SvLocal void QBViewRightEMMWaiterDestroy(void *self)
{
    SVRELEASE(((QBViewRightEMMWaiter) self)->tvLogic);
}

SvLocal SvType QBViewRightEMMWaiter_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBViewRightEMMWaiterDestroy
    };

    static const struct QBViewRightOSMListener_s OSMListenerMethods = {
        .show                            = QBViewRightEMMWaiterShow,
        .checkPin                        = QBViewRightEMMWaiterPinCheck,
        .orderPin                        = QBViewRightEMMWaiterPinOrder,
        .showSmartcardMMINotification    = QBViewRightEMMWaiterShowSmartcardMMINotification,
        .showDescramblingMMINotification = QBViewRightEMMWaiterShowMMINotification
    };

    static const struct QBAppStartupWaiterPlugin_ appStartupMethods = {
        .start           = QBViewRightEMMWaiterStart,
        .stop            = QBViewRightEMMWaiterStop,
        .hasFinished     = QBViewRightEMMWaiterHasFinished,
    };

    if (!type) {
        SvTypeCreateManaged("QBViewRightEMMWaiter",
                            sizeof(struct QBViewRightEMMWaiter_s),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            QBViewRightOSMListener_getInterface(), &OSMListenerMethods,
                            QBAppStartupWaiterPlugin_getInterface(), &appStartupMethods,
                            NULL);
    }

    return type;
}

QBViewRightEMMWaiter QBViewRightEMMWaiterCreate(unsigned timeout, QBTVLogic tvLogic, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBViewRightEMMWaiter self = NULL;

    if (!QBViewRightGetInstance()) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "This project doesn't use ViewRightDVB CAS. QBViewRightEMMWaiter won't be created");
        goto fini;
    }

    self = (QBViewRightEMMWaiter) SvTypeAllocateInstance(QBViewRightEMMWaiter_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error, "can't allocate QBViewRightEMMWaiter");
        goto fini;
    }

    if (timeout) {
        self->finishAfter = SvTimeAdd(SvTimeGet(), SvTimeFromMilliseconds(timeout * 1000));
    } else {
        self->infiniteTimeout = true;
    }
    self->tvLogic = SVRETAIN(tvLogic);

    QBViewRightOsmListenerAdd(QBViewRightGetInstance(), (SvGenericObject) self);

fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

