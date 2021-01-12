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

#include "QBAutoTest.h"
#include <SvCore/SvLog.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <QBInput/QBInputFilter.h>
#include <QBInput/QBInputService.h>
#include <QBInput/QBInputCodes.h>
#include <Utils/autoTest.h>
#include <Utils/magicCode.h>
#include <unistd.h>
#include <QBShellCmds.h>


#if SV_LOG_LEVEL == 0
// ----------- SECURE SECTION PART -----------------

void QBAutoTestCreate(void)
{
}

void QBAutoTestDestroy(void)
{
}

#else
// ---------- DEBUG SECTION PART -------------------

typedef struct QBAutoTestFilter_t* QBAutoTestFilter;

struct QBAutoTestFilter_t {
    struct SvObject_ super_;  ///< super class
    bool started;                     ///< is auto test started
    QBMagicCode *magicCode;           ///< magic code used to start/stop auto test
};

// PRIVATE FUNCTIONS

SvLocal void QBAutoTestFilter__dtor__(void *self_)
{
    QBAutoTestFilter self = (QBAutoTestFilter) self_;
    QBMagicCodeDestroy(self->magicCode);
}

SvLocal SvString QBAutoTestFilterGetName(SvGenericObject self_)
{
        return SVSTRING("QBAutoTestFilter");
}

SvLocal const char* QBAutoTestGetDaemonPath(void)
{
    static const char* daemonPath = "/etc/init.d/auto_testd";
    return daemonPath;
}

SvLocal void QBAutoTestFilterStartAutoTest(QBAutoTestFilter self)
{
    SvLogNotice("QBAutoTest - START");
    char *cmd = NULL;
    asprintf(&cmd, "%s start", QBAutoTestGetDaemonPath());
    QBShellExec(cmd);
    free(cmd);
    self->started = true;
}

SvLocal void QBAutoTestFilterStopAutoTest(QBAutoTestFilter self)
{
    SvLogNotice("QBAutoTest - STOP");
    char *cmd = NULL;
    asprintf(&cmd, "%s stop", QBAutoTestGetDaemonPath());
    QBShellExec(cmd);
    free(cmd);
    self->started = false;
}

SvLocal void QBAutoTestFilterCheckMagicCode(QBAutoTestFilter self, QBInputEvent event)
{
    if (!QBIsAutoTestEvent(&event)) {
        QBMagicCodeAdd(self->magicCode, event.u.key.code);
        if (QBMagicCodeMatches(self->magicCode)) {
            if (!self->started)
                QBAutoTestFilterStartAutoTest(self);
            else
                QBAutoTestFilterStopAutoTest(self);
        }
    }
}

SvLocal unsigned QBAutoTestFilterProcessEvents(SvGenericObject self, QBInputQueue outQueue, QBInputQueue inQueue)
{
    QBInputEvent ev;
    for(;;) {
        ev = QBInputQueueGet(inQueue);
        if(ev.type == QBInputEventType_invalid)
            break;
        QBInputQueuePut(outQueue, ev);
        if (ev.type == QBInputEventType_keyTyped)
            QBAutoTestFilterCheckMagicCode((QBAutoTestFilter) self, ev);
    }

    return 0;
}

SvLocal SvType QBAutoTestFilter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBAutoTestFilter__dtor__
    };
    static SvType type = NULL;
    static const struct QBInputFilter_ methods = {
        .getName = QBAutoTestFilterGetName,
        .processEvents = QBAutoTestFilterProcessEvents
    };

    if (!type) {
        SvTypeCreateManaged("QBAutoTestFilter",
                            sizeof(struct QBAutoTestFilter_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBInputFilter_getInterface(), &methods,
                NULL);
    }

    return type;
}

SvLocal QBAutoTestFilter
QBAutoTestFilterCreate(void)
{
    QBAutoTestFilter filter = (QBAutoTestFilter) SvTypeAllocateInstance(QBAutoTestFilter_getType(), NULL);

    /**
    * @brief Code used to enable/disable auto tests. You can type it from remote
    * control, in any place - tv, menu, etc.
    */
    uint16_t magicCode[] = {
        QBKEY_MUTE,
        QBKEY_RED,
        QBKEY_MUTE,
        QBKEY_GREEN,
        QBKEY_MUTE,
        QBKEY_YELLOW
    };

    filter->magicCode = QBMagicCodeCreate(magicCode, sizeof(magicCode) / sizeof(magicCode[0]));
    return filter;
}

SvLocal QBAutoTestFilter* QBAutoTestFilterGetInstance(void)
{
    static QBAutoTestFilter singleton = NULL;
    if (!singleton)
        singleton = QBAutoTestFilterCreate();

    return &singleton;
}

// PUBLIC FUNCTIONS

void QBAutoTestCreate(void)
{
    if (!access(QBAutoTestGetDaemonPath(), F_OK)) {
        QBAutoTestFilter *autoTestFilter = QBAutoTestFilterGetInstance();
        QBInputServiceAddGlobalFilter((SvGenericObject) *autoTestFilter, NULL, NULL);
    }
}

void QBAutoTestDestroy(void)
{
    if (!access(QBAutoTestGetDaemonPath(), F_OK)) {
        QBAutoTestFilter *filter = QBAutoTestFilterGetInstance();
        QBInputServiceRemoveFilter((SvGenericObject) *filter, NULL);
        SVRELEASE(*filter);
        *filter = NULL;
    }
}


#endif // SV_LOG_LEVEL == 0

