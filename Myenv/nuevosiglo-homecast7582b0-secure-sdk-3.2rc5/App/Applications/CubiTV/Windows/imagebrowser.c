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

#include "imagebrowser.h"

#include <libintl.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <main.h>
#include <Menus/QBFileBrowserMenuChoice.h>
#include <QBFSEntry.h>
#include <QBFSDir.h>
#include <QBFSFile.h>
#include <QBInput/QBInputCodes.h>
#include <CUIT/Core/widget.h>
#include <CUIT/Core/event.h>
#include <CUIT/Core/effect.h>
#include <SWL/icon.h>
#include <SWL/fade.h>
#include <QBDataModel3/QBListProxy.h>
#include <QBDataModel3/QBTreePath.h>
#include <Services/QBAudioBackgroundPlayer.h>

struct QBImageBrowserContext_t {
    struct QBWindowContext_t super_;

    AppGlobals appGlobals;
    SvWidget image;
    SvWidget leftArrow;
    SvWidget rightArrow;
    unsigned int ctxID;
    SvRID waitRID, errorRID, previousRID, currentRID;
    SvGenericObject controller;
    int currentPosition;
    QBListProxy nodeList;

    SvEffectId fadeEffectLeft;
    SvEffectId fadeEffectRight;
    SvTimerId timer;
    int timeToHideArrows;
    double fadeTime;
    double slideshowTime;
    bool slideshow;
    SvTimerId slideshowTimer;

    SvString backgroundMusicPlaylistURI;  //URI of m3u playlist, takes priority over backgroundMusicPlaylist
    SvArray backgroundMusicPlaylist;      //SvContent array
    QBAudioBackgroundPlaybackService backgroundMusicService;
};

SvInterface QBImageBrowserController_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;

    if (!interface) {
        SvInterfaceCreateManaged("QBImageBrowserController", sizeof(struct QBImageBrowserController_t),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }

    return interface;
}

SvLocal void
QBImageBrowserStartTimer(QBImageBrowserContext self)
{
    SvWidget w = self->super_.window;
    if (self->timer)
        svAppTimerStop(w->app, self->timer);
    self->timer = svAppTimerStart(w->app, w, self->timeToHideArrows, true);
}


SvLocal void
QBImageBrowserStopSlideshowTimer(QBImageBrowserContext self)
{
    SvWidget w = self->super_.window;
    if (self->slideshowTimer) {
        svAppTimerStop(w->app, self->slideshowTimer);
        self->slideshowTimer = 0;
    }
}

SvLocal void
QBImageBrowserStartSlideshowTimer(QBImageBrowserContext self)
{
    SvWidget w = self->super_.window;

    QBImageBrowserStopSlideshowTimer(self);
    if (self->slideshow)
        self->slideshowTimer = svAppTimerStart(w->app, w, self->slideshowTime, 1);
}

SvLocal void
QBImageBrowserFadeOutArrows(QBImageBrowserContext self)
{
    SvWidget w = self->super_.window;
    SvEffect eff = NULL;

    if (self->fadeEffectLeft)
        svAppCancelEffect(w->app, self->fadeEffectLeft, SV_EFFECT_ABANDON);

    eff = svEffectFadeNew(self->leftArrow, ALPHA_TRANSPARENT, true,
                          self->fadeTime, SV_EFFECT_FADE_LINEAR);
    svEffectSetNotificationTarget(eff, w);
    self->fadeEffectLeft = svAppRegisterEffect(w->app, eff);

    if (self->fadeEffectRight)
        svAppCancelEffect(w->app, self->fadeEffectRight, SV_EFFECT_ABANDON);

    eff = svEffectFadeNew(self->rightArrow, ALPHA_TRANSPARENT, true,
                          self->fadeTime, SV_EFFECT_FADE_LINEAR);
    svEffectSetNotificationTarget(eff, w);
    self->fadeEffectRight = svAppRegisterEffect(w->app, eff);
}

SvLocal void QBImageBrowserInit(QBImageBrowserContext self, SvApplication app)
{
    typedef enum {
        ImageWait = 0, FallbackImageOnError, ImageOne, ImageTwo
    } IndexfOfImage;

    if (!self || !app) {
        return;
    }

    bool previousImageIsStillLoading = false;
    if (self->image) {
        previousImageIsStillLoading = SvIconIsBitmapLoading(self->image, svIconGetIndex(self->image));
        svWidgetDestroy(self->image);
    }

    svSettingsRestoreContext(self->ctxID);
    self->image = svIconNew(app, "ImageBrowser.Window");
    svSettingsPopComponent();

    svIconSetBitmapFromRID(self->image, (IndexfOfImage) ImageWait, self->waitRID);
    svIconSetBitmapFromRID(self->image, FallbackImageOnError, self->errorRID);

    if (self->currentRID != SV_RID_INVALID) {
        if (self->previousRID == SV_RID_INVALID) {
            svIconSetBitmapFromRID(self->image, ImageOne, self->currentRID);

            if (!svIconIsBitmapAvailable(self->image, ImageOne) ) {
                svIconSwitch(self->image, ImageWait, FallbackImageOnError, 0.0);
            }
            svIconSwitch(self->image, ImageOne, FallbackImageOnError, 0.0);
        } else {
            svIconSetBitmapFromRID(self->image, ImageOne, self->previousRID);
            svIconSetBitmapFromRID(self->image, ImageTwo, self->currentRID);

            if (!svIconIsBitmapAvailable(self->image, ImageOne) ) {
                svIconSwitch(self->image, previousImageIsStillLoading ? ImageWait : FallbackImageOnError, FallbackImageOnError, 0.0);
            } else {
                svIconSwitch(self->image, ImageOne, FallbackImageOnError, 0.0);
            }
            if (!svIconIsBitmapAvailable(self->image, ImageTwo) ) {
                svIconSwitch(self->image, ImageWait, FallbackImageOnError, 0.0);
            }
            svIconSwitch(self->image, ImageTwo, FallbackImageOnError, self->fadeTime);
        }
    } else {
        svIconSwitch(self->image, FallbackImageOnError, FallbackImageOnError, 0.0);
    }


    if (self->image)
        svWidgetAttach(self->super_.window, self->image, 0, 0, 3);

    self->leftArrow->tree_alpha = ALPHA_SOLID;
    self->rightArrow->tree_alpha = ALPHA_SOLID;

    if (self->currentPosition == 0) {
        svIconSwitch(self->leftArrow, 1, 0, 0.1);
    } else {
        svIconSwitch(self->leftArrow, 0, 0, 0.1);
    }

    int len = SvInvokeInterface(QBListModel, self->nodeList, getLength);
    if (self->currentPosition + 1 == len) {
        svIconSwitch(self->rightArrow, 1, 0, 0.1);
    } else {
        svIconSwitch(self->rightArrow, 0, 0, 0.1);
    }

    QBImageBrowserStartTimer(self);
    QBImageBrowserStartSlideshowTimer(self);
}

SvLocal void QBImageBrowserRefresh(QBImageBrowserContext self)
{
    QBImageBrowserInit(self, self->super_.window->app);
}

SvLocal void QBImageBrowserContextChangePicture(QBImageBrowserContext self, int diff, bool manual)
{
    SvGenericObject nodePath = NULL;
    SvGenericObject node = NULL;
    SvRID           nextRID = SV_RID_INVALID;
    int             tmp = self->currentPosition;

    do {
        tmp += diff;
        if (tmp < 0)
            return;
        nodePath = NULL;
        nextRID = SV_RID_INVALID;
        node = QBListProxyGetTreeNode(self->nodeList, tmp, &nodePath);
        if (nodePath && node)
            nextRID = SvInvokeInterface(QBImageBrowserController, self->controller, getRIDFromPath, nodePath);
        if (nextRID != SV_RID_INVALID) {
            self->previousRID = self->currentRID;
            self->currentRID = nextRID;
            self->currentPosition = tmp;
            QBImageBrowserStopSlideshowTimer(self);
            QBImageBrowserRefresh(self);
            break;
        } else if (nodePath && node) {
            continue;
        } else if (!manual) {
            QBApplicationControllerPopContext(self->appGlobals->controller);
            break;
        }
        break;
    } while (true);
}

SvLocal bool QBImageBrowserInputEventHandler(SvWidget w, SvInputEvent e)
{
    QBImageBrowserContext self = w->prv;
    if (e->ch == QBKEY_BACK) {
        QBApplicationControllerPopContext(self->appGlobals->controller);
        return true;
    } else if (e->ch == QBKEY_UP || e->ch == QBKEY_DOWN || e->ch == QBKEY_RIGHT || e->ch == QBKEY_LEFT) {
        if (e->ch == QBKEY_DOWN || e->ch == QBKEY_RIGHT)
            QBImageBrowserContextChangePicture(self, 1, true);
        else
            QBImageBrowserContextChangePicture(self, -1, true);
    }
    return false;
}

SvLocal void
QBImageBrowserEffectEventHandler(SvWidget w, SvEffectEvent e)
{
    QBImageBrowserContext self = w->prv;

    if (e->id == self->fadeEffectLeft) {
        self->fadeEffectLeft = 0;
        return;
    } else if (e->id == self->fadeEffectRight) {
        self->fadeEffectRight = 0;
        return;
    }
}

SvLocal void
QBImageBrowserTimerEventHandler(SvWidget w, SvTimerEvent e)
{
    QBImageBrowserContext self = w->prv;

    if (e->id == self->timer) {
        QBImageBrowserFadeOutArrows(self);
        self->timer = 0;
        return;
    } else if (e->id == self->slideshowTimer) {
        self->slideshowTimer = 0;
        QBImageBrowserContextChangePicture(self, 1, false);
        return;
    }
}

SvLocal void
QBImageBrowserClean(SvApplication app, void *ptr)
{
}

SvLocal void QBImageBrowserContextCreateWindow(QBWindowContext self_, SvApplication app)
{
    QBImageBrowserContext self = (QBImageBrowserContext) self_;

    svSettingsPushComponent("ImageBrowser.settings");
    SvWidget window = svSettingsWidgetCreate(app, "ImageBrowser.Window");
    if (!window) {
        svSettingsPopComponent();
        SvLogError("Could not create ImageBrowser window!");
        return;
    }

    svWidgetSetInputEventHandler(window, QBImageBrowserInputEventHandler);
    svWidgetSetTimerEventHandler(window, QBImageBrowserTimerEventHandler);
    svWidgetSetEffectEventHandler(window, QBImageBrowserEffectEventHandler);
    window->prv = self;
    window->clean = QBImageBrowserClean;
    self->super_.window = window;

    self->waitRID = svSettingsGetResourceID("ImageBrowser.WaitIcon", "bg");
    self->errorRID = svSettingsGetResourceID("ImageBrowser.ErrorIcon", "bg");
    self->ctxID = svSettingsSaveContext();

    self->timeToHideArrows = svSettingsGetInteger("ImageBrowser",
                                                  "timeToHideArrows", 0);
    self->fadeTime = svSettingsGetDouble("ImageBrowser", "fadeTime", 0.0);
    self->fadeEffectLeft = self->fadeEffectRight = 0;
    if (self->slideshowTime < 0)
        self->slideshowTime = svSettingsGetDouble("ImageBrowser", "slideshowDuration", 3.0);

    SvBitmap tmp = NULL;
    self->leftArrow = svIconNew(app, "ImageBrowser.LeftArrow");
    svSettingsWidgetAttach(window, self->leftArrow, svWidgetGetName(self->leftArrow), 5);
    tmp = svSettingsGetBitmap("ImageBrowser.LeftArrow", "activeBg");
    svIconSetBitmap(self->leftArrow, 0, tmp);
    tmp = svSettingsGetBitmap("ImageBrowser.LeftArrow", "inactiveBg");
    svIconSetBitmap(self->leftArrow, 1, tmp);
    self->rightArrow = svIconNew(app, "ImageBrowser.RightArrow");
    svSettingsWidgetAttach(window, self->rightArrow, svWidgetGetName(self->rightArrow), 5);
    tmp = svSettingsGetBitmap("ImageBrowser.RightArrow", "activeBg");
    svIconSetBitmap(self->rightArrow, 0, tmp);
    tmp = svSettingsGetBitmap("ImageBrowser.RightArrow", "inactiveBg");
    svIconSetBitmap(self->rightArrow, 1, tmp);

    if (self->backgroundMusicPlaylistURI || self->backgroundMusicPlaylist) {
        self->backgroundMusicService = QBAudioBackgroundPlaybackServiceCreate(self->appGlobals);
        if (self->backgroundMusicService) {
            if (self->backgroundMusicPlaylistURI)
                QBAudioBackgroundPlaybackServicePlayM3uPlaylist(self->backgroundMusicService, self->backgroundMusicPlaylistURI);
            else
                QBAudioBackgroundPlaybackServicePlayPlaylist(self->backgroundMusicService, self->backgroundMusicPlaylist);
        }
    }

    svSettingsPopComponent();
    QBImageBrowserInit(self, app);
}

SvLocal void QBImageBrowserContextDestroyWindow(QBWindowContext self_)
{
    QBImageBrowserContext self = (QBImageBrowserContext) self_;
    if (self->backgroundMusicService)
        QBAudioBackgroundPlaybackServiceStopPlayback(self->backgroundMusicService);
    if (self->image)
        svWidgetDestroy(self->image);
    self->image = NULL;

    svWidgetDestroy(self->super_.window);
    self->super_.window = NULL;
}

SvLocal void QBImageBrowserContextDestroy(void *self_)
{
    QBImageBrowserContext self = self_;
    SVRELEASE(self->controller);
    SVRELEASE(self->nodeList);
    SVTESTRELEASE(self->backgroundMusicPlaylistURI);
    SVTESTRELEASE(self->backgroundMusicService);
}

SvLocal SvType QBImageBrowserContext_getType(void)
{
    static const struct QBWindowContextVTable_ contextVTable = {
        .super_        = {
            .destroy   = QBImageBrowserContextDestroy
        },
        .createWindow  = QBImageBrowserContextCreateWindow,
        .destroyWindow = QBImageBrowserContextDestroyWindow
    };
    static SvType type = NULL;

    if (!type) {
        SvTypeCreateManaged("QBImageBrowserContext",
                            sizeof(struct QBImageBrowserContext_t),
                            QBWindowContext_getType(),
                            &type,
                            QBWindowContext_getType(), &contextVTable,
                            NULL);
    }

    return type;
}

QBWindowContext QBImageBrowserContextCreate(AppGlobals appGlobals, SvGenericObject parentnodePath_, int nodePosition,
                                            SvGenericObject controller, bool slideshow, double slideshowDuration)
{
    QBImageBrowserContext self = (QBImageBrowserContext) SvTypeAllocateInstance(QBImageBrowserContext_getType(), NULL);
    SvGenericObject nodePath;
    self->appGlobals = appGlobals;
    self->controller = SVRETAIN(controller);
    self->currentPosition = nodePosition;
    self->currentRID = SV_RID_INVALID;
    self->previousRID = SV_RID_INVALID;
    self->slideshow = slideshow;
    self->nodeList = QBListProxyCreate((SvGenericObject) appGlobals->menuTree, parentnodePath_, NULL);
    self->slideshowTime = slideshowDuration;

    QBListProxyGetTreeNode(self->nodeList, nodePosition, &nodePath);
    if (nodePath)
        self->currentRID = SvInvokeInterface(QBImageBrowserController, self->controller, getRIDFromPath, nodePath);
    return (QBWindowContext) self;
}

void QBImageBrowserContextSetBackgroundMusicM3uPlaylist(QBWindowContext self_, SvString URI)
{
    QBImageBrowserContext self = (QBImageBrowserContext) self_;
    assert(self_);

    SVTESTRELEASE(self->backgroundMusicPlaylistURI);
    self->backgroundMusicPlaylistURI = SVTESTRETAIN(URI);
}

void QBImageBrowserContextSetBackgroundMusicPlaylist(QBWindowContext self_, SvArray playlist)
{
    QBImageBrowserContext self = (QBImageBrowserContext) self_;
    assert(self_);

    SVTESTRELEASE(self->backgroundMusicPlaylist);
    self->backgroundMusicPlaylist = SVTESTRETAIN(playlist);
}
