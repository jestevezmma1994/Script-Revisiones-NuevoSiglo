/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2012-2016 Cubiware Sp. z o.o. All rights reserved.
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

#include "QBReencryptionService.h"
#include <QBReencryptionManager.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvEnv.h>

#if SV_LOG_LEVEL > 0
    #define moduleName "QBReencryptionService"
SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 1, moduleName "LogLevel", "");
    #define log_fun(fmt, ...)   do { if (env_log_level() >= 6) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_debug(fmt, ...) do { if (env_log_level() >= 5) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(green), __func__, ## __VA_ARGS__); } while (0)
    #define log_deep(fmt, ...)  do { if (env_log_level() >= 4) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_info(fmt, ...)  do { if (env_log_level() >= 3) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
    #define log_state(fmt, ...) do { if (env_log_level() >= 2) SvLogNotice(COLBEG() moduleName " :: %s " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); } while (0)
    #define log_error(fmt, ...) do { if (env_log_level() >= 1) SvLogError(COLBEG() moduleName " :: %s " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
 #else
    #define log_fun(fmt, ...)
    #define log_debug(fmt, ...)
    #define log_deep(fmt, ...)
    #define log_info(fmt, ...)
    #define log_state(fmt, ...)
    #define log_error(fmt, ...)
#endif

struct QBReencryptionService_ {
    struct SvObject_ super_;

    QBReencryptionManager manager;

    bool isInStandby;
    bool started;
};

SvLocal void
QBReencryptionServiceDestroy(void *self_)
{
    log_fun();
    QBReencryptionService self = self_;

    if (self->started) {
        QBReencryptionManagerStop(self->manager);
    }
}

SvLocal SvType
QBReencryptionService_getType(void)
{
    log_fun();
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBReencryptionServiceDestroy,
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBReencryptionService",
                            sizeof(struct QBReencryptionService_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

/////////////////////////////// API ///////////////////////////////////////////
int
QBReencryptionServiceStopUsingRecord(QBReencryptionService self, const QBSchedDesc* desc)
{
    log_fun();
    return QBReencryptionManagerStopUsing(self->manager, desc);
}

void
QBReencryptionServiceEnterStandby(QBReencryptionService self)
{
    log_fun();
    self->isInStandby = true;

    if (self->started) {
        QBReencryptionManagerStart(self->manager);
    }
}

void
QBReencryptionServiceLeaveStandby(QBReencryptionService self)
{
    log_fun();
    self->isInStandby = false;

    if (self->started) {
        QBReencryptionManagerStop(self->manager);
    }
}

QBReencryptionService
QBReencryptionServiceCreate(SvScheduler scheduler)
{
    log_fun();
    QBReencryptionService self = (QBReencryptionService) SvTypeAllocateInstance(QBReencryptionService_getType(), NULL);
    self->manager = QBReencryptionManagerCreate(scheduler, NULL);
    return self;
}

void
QBReencryptionServiceStart(QBReencryptionService self)
{
    log_fun();
    if (self->isInStandby) {
        QBReencryptionManagerStart(self->manager);
    }
    self->started = true;
}

void
QBReencryptionServiceStop(QBReencryptionService self)
{
    log_fun();
    QBReencryptionManagerStop(self->manager);
    self->started = false;
}
