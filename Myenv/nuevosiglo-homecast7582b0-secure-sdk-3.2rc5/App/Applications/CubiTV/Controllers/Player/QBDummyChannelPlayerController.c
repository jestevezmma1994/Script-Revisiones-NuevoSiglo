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

#include <Controllers/Player/QBDummyChannelPlayerController.h>
#include <QBPlayerControllers/utils/content.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <QBPlayerControllers/QBChannelPlayer.h>
#include <QBPlayerControllers/QBChannelPlayerController.h>
#include <QBAppKit/QBGlobalStorage.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <stddef.h>
#include <Services/QBCASManager.h>
#include <SvFoundation/SvWeakReference.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvWeakList.h>
#include <SvPlayerKit/SvContent.h>
#include <SvPlayerKit/SvContentMetaData.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>

#define QBTimeLimitEnable
#define QBTimeThreshold (50*1000)
#include <QBTimeLimit.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_DummyChPlayerControllerLogLevel, 0, "DummyChPlayerControllerLogLevel", "")

#define log_debug(fmt, ...)     if (env_DummyChPlayerControllerLogLevel() > 0) { SvLogNotice(COLBEG() "QBDummyChannelPlayerController %s :: " fmt COLEND_COL(blue), __func__, ##__VA_ARGS__); }

struct QBDummyChannelPlayerController_ {
    struct SvObject_ super_;

    SvGenericObject casPopupManager;

    SvWeakReference player;

    SvTVChannel channel;
    SvContent content;

    SvWeakList listeners;

    SvFiber notify;
};

SvLocal void QBDummyChannelPlayerControllerNotifyStep(void *self_)
{
    QBDummyChannelPlayerController* self = self_;
    SvFiberDeactivate(self->notify);

    SvIterator it = SvWeakListIterator(self->listeners);
    SvGenericObject listener;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBChannelPlayerControllerListener, listener, stateChanged, (SvGenericObject) self);
    }
}

SvLocal void QBDummyChannelPlayerControllerFinalizeStart(QBDummyChannelPlayerController* self)
{
    SvObject player = self->player ? SvWeakReferenceTakeReferredObject(self->player) : NULL;
    if (self->content && player)
        SvInvokeInterface(QBChannelPlayer, player, play, self->content, 0.0);
    SVTESTRELEASE(player);
}

SvLocal void QBDummyChannelPlayerControllerEndContentUsage(QBDummyChannelPlayerController* self)
{
    SvObject player = self->player ? SvWeakReferenceTakeReferredObject(self->player) : NULL;
    if (player && self->content)
        SvInvokeInterface(QBChannelPlayer, player, stop);
    SVTESTRELEASE(player);

    SVTESTRELEASE(self->content);
    self->content = NULL;
    SvFiberActivate(self->notify);
}

SvLocal bool QBDummyChannelPlayerControllerSupports(SvGenericObject self_, SvTVChannel channel)
{
    // If URI scheme is other than dvb, it must be MW channel
    if (!SvStringEqualToCString(SvURIScheme(channel->sourceURL), "dvb"))
        return true;

    return false;
}

SvLocal bool QBDummyChannelPlayerControllerCanStart(SvGenericObject self_, SvTVChannel channel)
{
    return true;
}

SvLocal void QBDummyChannelPlayerControllerStart(SvGenericObject self_, SvTVChannel channel)
{
    QBDummyChannelPlayerController* self = (QBDummyChannelPlayerController*) self_;

    SvContent content = createContentFromChannel(channel);
    bool contentChanged = (channel != self->channel) || !compareContentsFromChannel(channel, content, self->content);

    if (!contentChanged) {
        SVRELEASE(content);
        return;
    }

    QBDummyChannelPlayerControllerEndContentUsage(self);

    if (self->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, self->casPopupManager, contentChanged);

    SVTESTRELEASE(self->channel);
    self->channel = SVRETAIN(channel);
    SVTESTRELEASE(self->content);
    self->content = content;

    SvContentMetaData metaData = SvContentGetMetaData(self->content);
    QBGlobalStorage gs = QBGlobalStorageGetInstance();
    SvSSLParams sslParams = (SvSSLParams) QBGlobalStorageGetItemByName(gs, SVSTRING("SSLParams"), NULL);
    if (!SvContentMetaDataSetObjectProperty(metaData, SVSTRING("SSLParams"), (SvObject) sslParams)) {
        SvLogError("Setting SSLParams to SvContentMetaData failed");
    }

    // start playback immediatelly without waiting for the UseProduct answer
    QBDummyChannelPlayerControllerFinalizeStart(self);
    SvFiberActivate(self->notify);
}

SvLocal void QBDummyChannelPlayerControllerStop(SvGenericObject self_)
{
    QBDummyChannelPlayerController* self = (QBDummyChannelPlayerController*) self_;
    QBTimeLimitBegin(playerControllerTime, QBTimeThreshold);

    SVTESTRELEASE(self->channel);
    self->channel = NULL;
    QBDummyChannelPlayerControllerEndContentUsage(self);
    if (self->casPopupManager)
        SvInvokeInterface(QBCASPopupManager, self->casPopupManager, contentChanged);

    QBTimeLimitEnd_(playerControllerTime,"MWChannelPlayerController.stop");
}

SvLocal void QBDummyChannelPlayerControllerSetPlayer(SvGenericObject self_, SvObject newPlayer)
{
    QBDummyChannelPlayerController* self = (QBDummyChannelPlayerController*) self_;
    SvObject player = self->player ? SvWeakReferenceTakeReferredObject(self->player) : NULL;
    if (player == newPlayer) {
        SVTESTRELEASE(player);
        return;
    }
    if (player) {
        if (self->content)
            SvInvokeInterface(QBChannelPlayer, player, stop);
        SVRELEASE(player);
    }

    SVTESTRELEASE(self->player);
    self->player = NULL;

    if (newPlayer) {
        self->player = SvWeakReferenceCreate(newPlayer, NULL);
        if (self->content)
            SvInvokeInterface(QBChannelPlayer, newPlayer, play, self->content, 0.0);
    }
}

SvLocal void QBDummyChannelPlayerControllerAddListener(SvGenericObject self_, SvGenericObject listener)
{
    QBDummyChannelPlayerController* self = (QBDummyChannelPlayerController*) self_;
    SvWeakListPushFront(self->listeners, listener, NULL);
}

SvLocal void QBDummyChannelPlayerControllerRemoveListener(SvGenericObject self_, SvGenericObject listener)
{
    QBDummyChannelPlayerController* self = (QBDummyChannelPlayerController*) self_;
    SvWeakListRemoveObject(self->listeners, listener);
}

SvLocal void QBDummyChannelPlayerController__dtor__(void *self_)
{
    QBDummyChannelPlayerController* self = self_;

    SVTESTRELEASE(self->player);
    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->content);
    SVRELEASE(self->listeners);
    SvFiberDestroy(self->notify);
}

SvType QBDummyChannelPlayerController_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBDummyChannelPlayerController__dtor__
    };
    static SvType type = NULL;
    static const struct QBChannelPlayerController_ playerControllerMethods = {
        .start = QBDummyChannelPlayerControllerStart,
        .canStart = QBDummyChannelPlayerControllerCanStart,
        .supports = QBDummyChannelPlayerControllerSupports,
        .stop = QBDummyChannelPlayerControllerStop,
        .setPlayer = QBDummyChannelPlayerControllerSetPlayer,
        .addListener = QBDummyChannelPlayerControllerAddListener,
        .removeListener = QBDummyChannelPlayerControllerRemoveListener,
    };

    if (!type) {
        SvTypeCreateManaged("QBDummyChannelPlayerController",
                            sizeof(struct QBDummyChannelPlayerController_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBChannelPlayerController_getInterface(), &playerControllerMethods,
                            NULL);
    }

    return type;
}

QBDummyChannelPlayerController* QBDummyChannelPlayerControllerCreate(SvScheduler scheduler, SvGenericObject casPopupManager)
{
    QBDummyChannelPlayerController* self = (QBDummyChannelPlayerController*) SvTypeAllocateInstance(QBDummyChannelPlayerController_getType(), NULL);
    SvLogWarning("Using QBDummyChannelPlayerController");
    self->casPopupManager = casPopupManager;
    self->listeners = SvWeakListCreate(NULL);
    self->notify = SvFiberCreate(scheduler, NULL, "QBDummyChannelPlayerController", QBDummyChannelPlayerControllerNotifyStep, self);
    return self;
}
