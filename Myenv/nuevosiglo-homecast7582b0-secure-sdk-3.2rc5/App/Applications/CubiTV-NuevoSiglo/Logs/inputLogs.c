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

#include <QBSecureLogManager.h>
#include <SvCore/SvCommonDefs.h>
#include <QBInput/QBInputCodes.h>
#include <QBInput/QBInputEvent.h>
#include <QBInput/QBInputQueue.h>
#include <QBInput/QBInputFilter.h>
#include <QBInput/QBInputService.h>
#include <QBInput/QBInputUtils.h>
#include <QBApplicationController.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvValue.h>
#include <Services/BoldUsageLog.h> //NR : Adding Bold DVB Usage Logs
#include "inputLogs.h"

struct QBLogFilter_t {
    struct SvObject_ super_;
    QBApplicationController controller;
    QBWindowContext currentContext;
    SvHashTable codes;
};
typedef struct QBLogFilter_t* QBLogFilter;

static QBLogFilter filter = NULL;

SvLocal void QBLogFilterHandleKey(QBLogFilter self, uint32_t key)
{
    SVAUTOINTVALUE(keyVal, key);
    SvString name = (SvString) SvHashTableFind(self->codes, (SvGenericObject) keyVal);
    if (name) {
        QBSecureLogEvent("Input", "Notice.Input.KeyPressed", "JSON:{\"key\":\"%s\"}", SvStringCString(name));
        
        // NR: Log Key Pressing through BoldUsageLog Module
        BoldUsageLog(LOG_INFO, "\"Input\":{\"context\":\"%s\",\"key\":\"%s\"}", self->currentContext ? SvObjectGetTypeName((SvGenericObject) self->currentContext) : "", SvStringCString(name));
    }
}

SvLocal SvString QBLogFilterGetName(SvGenericObject self_)
{
    return SVSTRING("QBLogFilter");
}

SvLocal unsigned QBLogFilterProcessEvents(SvGenericObject self_, QBInputQueue outQueue, QBInputQueue inQueue)
{
    QBLogFilter self = (QBLogFilter) self_;
    QBInputEvent ev;
    for(;;) {
        ev = QBInputQueueGet(inQueue);
        if(ev.type == QBInputEventType_invalid)
            break;
        QBInputQueuePut(outQueue, ev);
        if(ev.type == QBInputEventType_keyTyped && !QBKEY_IS_MOUSE(ev.u.key.code))
            QBLogFilterHandleKey(self, ev.u.key.code);
    }

    return 0;
}

SvLocal void QBLogFilterSwitchStarted(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    QBLogFilter self = (QBLogFilter) self_;
    SVTESTRELEASE(self->currentContext);
    self->currentContext = NULL;
}

SvLocal void QBLogFilterSwitchEnded(SvGenericObject self_, QBWindowContext from, QBWindowContext to)
{
    QBLogFilter self = (QBLogFilter) self_;
    self->currentContext = SVTESTRETAIN(to);
}

SvLocal void QBLogFilter__dtor__(void *self_)
{
    QBLogFilter self = self_;
    SVTESTRELEASE(self->currentContext);
    SVTESTRELEASE(self->codes);
}

SvLocal SvType QBLogFilter_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBLogFilter__dtor__
    };
    static SvType type = NULL;
    static const struct QBInputFilter_ methods = {
        .getName = QBLogFilterGetName,
        .processEvents = QBLogFilterProcessEvents
    };
    static const struct QBContextSwitcherListener_t listenerMethods = {
        .started = QBLogFilterSwitchStarted,
        .ended = QBLogFilterSwitchEnded
    };

    if (!type) {
        SvTypeCreateManaged("QBLogFilter",
                            sizeof(struct QBLogFilter_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBInputFilter_getInterface(), &methods,
                            QBContextSwitcherListener_getInterface(), &listenerMethods,
                            NULL);
    }

    return type;
}

void QBInputLogCreate(QBApplicationController controller)
{
    filter = (QBLogFilter) SvTypeAllocateInstance(QBLogFilter_getType(), NULL);
    filter->codes = QBInputLoadCodesMap(NULL);
    filter->controller = controller;
}

void QBInputLogStart(void)
{
    if (!filter->codes)
        return;

    uint32_t ignored[] = {QBKEY_UP, QBKEY_DOWN, QBKEY_LEFT, QBKEY_RIGHT, QBKEY_ENTER};
    unsigned i;
    for(i = 0; i < sizeof(ignored) / sizeof(uint32_t); i++) {
        SVAUTOINTVALUE(key, ignored[i]);
        SvHashTableRemove(filter->codes, (SvGenericObject) key);
    }

    QBInputServiceAddGlobalFilter((SvGenericObject) filter, NULL, NULL);
    QBApplicationControllerAddListener(filter->controller, (SvGenericObject) filter);
}

void QBInputLogStop(void)
{
    QBInputServiceRemoveFilter((SvGenericObject) filter, NULL);
    QBApplicationControllerRemoveListener(filter->controller, (SvGenericObject) filter);
}

void QBInputLogDestroy(void)
{
    SVRELEASE(filter);
    filter = NULL;
}

// AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
SvString BoldInputLogGetCurrentContext(void)
{
    if (!filter || !filter->currentContext)
    {
        return SvStringCreate("", NULL);
    }
    
    return SvStringCreate(SvObjectGetTypeName((SvGenericObject) filter->currentContext), NULL);
}
