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

#include "QBXHTMLWindowsService.h"
#include "QBXHTMLWidgets/QBXHTMLWindowsCreator.h"
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <Windows/QBXHTMLContext.h>
#include <main.h>

struct QBXHTMLWindowsService_ {
    struct SvObject_ super;
    AppGlobals appGlobals;
    QBXHTMLWindowsCreator curCreator;
    QBXHTMLWindowsClient xhtmlWindowsClient;
    QBXHTMLWindow curWindow;
    SvURL url;
    QBXHTMLContext xhtmlCtx;
};

SvLocal void
QBXHTMLWindowsServiceDestroy(void *self_)
{
    QBXHTMLWindowsService self = (QBXHTMLWindowsService) self_;
    SVRELEASE(self->xhtmlWindowsClient);
    SVRELEASE(self->curCreator);
}

SvLocal SvType
QBXHTMLWindowsService_getType(void)
{
    static SvType type = NULL;

    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBXHTMLWindowsServiceDestroy
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBXHTMLWindowsService", sizeof(struct QBXHTMLWindowsService_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}


QBXHTMLWindowsService
QBXHTMLWindowsServiceCreate(AppGlobals appGlobals, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    QBXHTMLWindowsService self = (QBXHTMLWindowsService) SvTypeAllocateInstance(QBXHTMLWindowsService_getType(), &error);
    self->appGlobals = appGlobals;
    self->xhtmlWindowsClient = QBXHTMLWindowsClientCreate(appGlobals->sslParams, appGlobals->smartCardMonitor);
    self->curCreator = QBXHTMLWindowsCreatorCreate(self->appGlobals, self->xhtmlWindowsClient);
    SvErrorInfoPropagate(error, errorOut);
    return self;
}

void
QBXHTMLWindowsServiceStart(QBXHTMLWindowsService self)
{
}

void
QBXHTMLWindowsServiceStop(QBXHTMLWindowsService self)
{
    SVTESTRELEASE(self->xhtmlCtx);
    self->xhtmlCtx = NULL;
    SVTESTRELEASE(self->curWindow);
    self->curWindow = NULL;
}

SvLocal void
QBXHTMLWindowsServiceWidgetReadyCallback(void* self_, QBXHTMLWindow window, QBXHTMLWindowsCreatorErrorCode errorCode)
{
    QBXHTMLWindowsService self = (QBXHTMLWindowsService) self_;
    SVTESTRELEASE(self->curWindow);
    self->curWindow = NULL;
    SVTESTRELEASE(self->url);
    self->url = NULL;

    if (errorCode == QBXHTMLWindowsCreatorErrorCode_ok && window) {
        self->curWindow = SVRETAIN(window);
        QBXHTMLContextSetWidget(self->xhtmlCtx, self->curWindow);
    } else {
        SvLogError("Error while processing XHTML widget %d", errorCode);
        QBXHTMLContextSetMessage(self->xhtmlCtx, SVSTRING("Service unavailable"));
    }
}

void
QBXHTMLWindowsServiceDisplay(QBXHTMLWindowsService self, SvURL url, SvString id)
{
    SVTESTRELEASE(self->url);
    self->url = SVRETAIN(url);

    SVTESTRELEASE(self->xhtmlCtx);
    QBWindowContext ctx = QBXHTMLContextCreate(self->appGlobals);
    QBApplicationControllerPushContext(self->appGlobals->controller, ctx);
    self->xhtmlCtx = (QBXHTMLContext) ctx;

    QBXHTMLWindowsCreatorCreateWindowFromURL(self->curCreator, self->url, id, QBXHTMLWindowsServiceWidgetReadyCallback, self);
}

QBXHTMLWindowsClient
QBXHTMLWindowsServiceGetXHTMLWindowsClient(QBXHTMLWindowsService self)
{
    if (!self) {
        SvLogError("%s: null self passed", __func__);
        return NULL;
    }

    return self->xhtmlWindowsClient;
}
