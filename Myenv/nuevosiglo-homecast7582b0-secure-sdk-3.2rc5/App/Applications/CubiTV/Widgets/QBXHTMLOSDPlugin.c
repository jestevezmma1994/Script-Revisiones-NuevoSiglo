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

#include "QBXHTMLOSDPlugin.h"
#include <Services/QBXHTMLWindowsService.h>
#include <Services/QBXHTMLWidgets/QBXHTMLWindowsCreator.h>
#include <Logic/TVOSDPluginInterface.h>
#include <SvCore/SvEnv.h>
#include <main.h>

// Minimum refresh delay in seconds
#define XHTML_OSD_MIN_REFRESH_DELAY 0

// Maximum refresh delay in seconds
#define XHTML_OSD_MAX_REFRESH_DELAY 15

// Default refresh time
#define XHTML_OSD_DEFAULT_REFRESH_TIME_S 15 * 60

SV_DECL_INT_ENV_FUN_DEFAULT(env_log_level, 0, "QBXHTMLOSDPlugin", "0:error and warning, 1:state, 2:debug");

#define log_error(fmt, ...)   do { if (env_log_level() >= 0) SvLogError(COLBEG() "[%s] " fmt COLEND_COL(red), __func__, ## __VA_ARGS__); } while (0)
#define log_warning(fmt, ...) do { if (env_log_level() >= 0) SvLogWarning(COLBEG() "[%s] " fmt COLEND_COL(yellow), __func__, ## __VA_ARGS__); } while (0)
#define log_state(fmt, ...)   do { if (env_log_level() >= 1) SvLogNotice(COLBEG() "[%s] " fmt COLEND_COL(cyan), __func__, ## __VA_ARGS__); } while (0)
#define log_debug(fmt, ...)   do { if (env_log_level() >= 2) SvLogNotice("[%s] " fmt, __func__, ## __VA_ARGS__); } while (0)

struct QBXHTMLOSDPlugin_ {
    struct SvObject_ super;
    /** global data handle */
    AppGlobals appGlobals;
    /** widget which works as a OSD */
    SvWidget osd;
    /** flag indicate if plugin has been started */
    bool running;
    /** data of XHTML window displayed within this context */
    QBXHTMLWindow xhtmlWindow;
    /** client provide communication with server */
    QBXHTMLWindowsClient xhtmlClient;
    /** address to xhtmlWidget*/
    SvURL url;
    /** time after which xhtmlWidget will be updated */
    int refreshTime;
    /** creator used to build xhtmlWidget */
    QBXHTMLWindowsCreator curCreator;
    /** fiber to periodically updates of xhtmlWidget */
    SvFiber windowsUpdate;
    /** timer to periodically updates of xhtmlWidget */
    SvFiberTimer windowsUpdateTimer;
};

SvLocal void QBXHTMLOSDPluginStart(SvObject self_, SvWidget osd);
SvLocal void QBXHTMLOSDPluginStop(SvObject self_);

SvLocal void
QBXHTMLOSDPluginDestroy(void *self_)
{
    QBXHTMLOSDPlugin self = (QBXHTMLOSDPlugin) self_;
    QBXHTMLOSDPluginStop((SvObject) self);
    SVRELEASE(self->url);
    SVTESTRELEASE(self->xhtmlClient);
    SVTESTRELEASE(self->curCreator);
    SVTESTRELEASE(self->xhtmlWindow);
}

SvLocal SvType
QBXHTMLOSDPlugin_getType(void)
{
    static struct TVOSDPlugin_ methods = {
        .start = QBXHTMLOSDPluginStart,
        .stop  = QBXHTMLOSDPluginStop
    };

    static SvType type = NULL;

    if (unlikely(!type)) {
        static const struct SvObjectVTable_ objectVTable = {
            .destroy = QBXHTMLOSDPluginDestroy
        };

        SvTypeCreateManaged("QBXHTMLOSDPlugin",
                            sizeof(struct   QBXHTMLOSDPlugin_),
                            SvObject_getType(), &type,
                            SvObject_getType(), &objectVTable,
                            TVOSDPlugin_getInterface(), &methods,
                            NULL);
    }
    return type;
}

SvLocal int64_t
QBXHTMLOSDPluginGetRandomizedDelay(void)
{
    return XHTML_OSD_DEFAULT_REFRESH_TIME_S + (((long long int) rand() | ((long long int) rand() << 32)) %
                                               ((long long int) XHTML_OSD_MAX_REFRESH_DELAY - XHTML_OSD_MIN_REFRESH_DELAY));
}

QBXHTMLOSDPlugin
QBXHTMLOSDPluginCreate(AppGlobals appGlobals, SvString url)
{
    log_debug("");

    if (!appGlobals || !url) {
        log_error("null argument passed (appGlobals: %p, url: %p)", appGlobals, url);
        return NULL;
    }

    SvErrorInfo error;
    QBXHTMLOSDPlugin self = (QBXHTMLOSDPlugin) SvTypeAllocateInstance(QBXHTMLOSDPlugin_getType(), &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        log_error("Can't create QBHTMLOSD plugin");
        return NULL;
    }

    self->appGlobals = appGlobals;
    self->url = SvURLCreateWithString(url, &error);
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        log_error("Can't create QBHTMLOSD plugin.");
        SvErrorInfoDestroy(error);
        SVRELEASE(self); // there is no point to create plugin wthout URL
        return NULL;
    }

    QBXHTMLWindowsClient xhtmlClient = QBXHTMLWindowsServiceGetXHTMLWindowsClient(self->appGlobals->xhtmlWindowsService);
    if (!xhtmlClient) {
        log_error("Can't create QBHTMLOSD plugin. There is no xhtmlClient");
        SVRELEASE(self);
        return NULL;
    }
    self->xhtmlClient = SVRETAIN(xhtmlClient);
    self->refreshTime = QBXHTMLOSDPluginGetRandomizedDelay();

    return self;
}

SvLocal void
QBXHTMLOSDPluginCreateWindowCallback(void* self_, QBXHTMLWindow window, QBXHTMLWindowsCreatorErrorCode errorCode)
{
    log_debug();
    QBXHTMLOSDPlugin self = (QBXHTMLOSDPlugin) self_;

    if (self->running) { // thid callback could be invoked when plugin have been stopped
        if (errorCode == QBXHTMLWindowsCreatorErrorCode_ok && window) {
            if (self->xhtmlWindow) {
                SvWidget oldWidget = QBXHTMLWindowGetWidget(self->xhtmlWindow);
                svWidgetDestroy(oldWidget);
                SVRELEASE(self->xhtmlWindow);
            }
            SvWidget xhtmlWidget = QBXHTMLWindowGetWidget(window);
            svWidgetAttach(self->osd, xhtmlWidget, 0, 0, 6);
            self->xhtmlWindow = SVRETAIN(window);
            int newRefreshTime = QBXHTMLWindowGetRefreshTime(window);
            if (newRefreshTime > 0)
                self->refreshTime = newRefreshTime;
        } else {
            SvLogError("Error while processing XHTML widget %d", errorCode);
        }

        log_state("update xhtmlWindow after: %d [s]", self->refreshTime);
        SvFiberTimerActivateAfter(self->windowsUpdateTimer, SvTimeFromMilliseconds(self->refreshTime * 1000));
    }
}

SvLocal void
QBXHTMLOSDPluginUpdateStep(void *self_)
{
    log_debug();
    QBXHTMLOSDPlugin self = (QBXHTMLOSDPlugin) self_;
    SvFiberDeactivate(self->windowsUpdate);
    SvFiberEventDeactivate(self->windowsUpdateTimer);

    SVTESTRELEASE(self->curCreator);
    self->curCreator = QBXHTMLWindowsCreatorCreate(self->appGlobals, self->xhtmlClient);

    QBXHTMLWindowsCreatorCreateWindowFromURL(self->curCreator, self->url, NULL, QBXHTMLOSDPluginCreateWindowCallback, self);
}

SvLocal void
QBXHTMLOSDPluginStart(SvObject self_, SvWidget osd)
{
    QBXHTMLOSDPlugin self = (QBXHTMLOSDPlugin) self_;
    log_debug("");
    if (self->running)
        return;

    self->running = true;
    self->osd = osd;
    self->windowsUpdate = SvFiberCreate(self->appGlobals->scheduler, NULL, "QBXHTMLOSDPluginUpdate", QBXHTMLOSDPluginUpdateStep, self);
    self->windowsUpdateTimer = SvFiberTimerCreate(self->windowsUpdate);
    SvFiberActivate(self->windowsUpdate);
}

SvLocal void
QBXHTMLOSDPluginStop(SvObject self_)
{
    QBXHTMLOSDPlugin self = (QBXHTMLOSDPlugin) self_;

    if (!self->running)
        return;

    if (self->windowsUpdate) {
        SvFiberDestroy(self->windowsUpdate);
        self->windowsUpdate = NULL;
    }
    self->running = false;
}
