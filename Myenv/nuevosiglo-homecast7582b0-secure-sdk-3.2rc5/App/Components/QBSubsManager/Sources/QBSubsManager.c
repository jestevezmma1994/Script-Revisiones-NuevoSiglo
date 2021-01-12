/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
****************************************************************************/

#include <QBSubsManager.h>
#include <stdint.h>
#include <limits.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvWeakList.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvBinaryTree.h>
#include <SvFoundation/SvDeque.h>
#include <QBAppKit/QBObserver.h>
#include <QBAppKit/QBObservable.h>
#include <QBAppKit/QBAsyncService.h>
#include <SvPlayerControllers/SvPlayerTrackController.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <QBConf.h>
#include <QBTextUtils/hex.h>
#include <QBStringUtils.h>
#include <CUIT/Core/QBCUITWidget.h>
#include <CUIT/Core/app.h>
#include <CUIT/Core/widget.h>


#define log_error(fmt, ...)  SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(red), __func__, __LINE__, ## __VA_ARGS__)
#define log_debug(fmt, ...)  if (0) SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(blue), __func__, __LINE__, ## __VA_ARGS__)
#define log_stats(fmt, ...)  if (0) SvLogNotice(COLBEG() "%s:%d " fmt COLEND_COL(blue), __func__, __LINE__, ## __VA_ARGS__)

#define QB_SUBS_MANAGER_MAX_SUBS 30

//all in 90k
#define SUBTITLE_MAX_LAG 90000LL

#define SUBTITLE_DECODE_TIME 45000LL
#define SUBTITLE_SHOW_TIME 3000LL
#define SUBTITLE_MAX_UNSYNC (90000LL * 30LL)

SvLocal inline int64_t pts_diff(int64_t pts1, int64_t pts2, int bits)
{
    int64_t mask = (1LL << bits) - 1;
    int64_t diff = (pts1 & mask) - (pts2 & mask);
    int64_t max = (1LL << (bits - 1));
    if (diff < -max) {
        diff += (1LL << bits);
    } else if (diff > max) {
        diff -= (1LL << bits);
    }
    return diff;
}

SvLocal void QBSubsTrackDestroy(void *self_)
{
    QBSubsTrack self = (QBSubsTrack) self_;
    SVTESTRELEASE(self->langCode);
    self->langCode = NULL;
}

SvType QBSubsTrack_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSubsTrackDestroy
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSubsTrack",
                            sizeof(struct QBSubsTrack_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }

    return type;
}

struct QBSubsManager_ {
    struct QBObservable_ super_;
    SvWeakList listeners;

    QBActiveArray tracks;
    QBActiveArray ccTracks; // Closed Captions

    QBSubsTrack currentTrack;
    SvHashTable queues;

    SvPlayerTask playerTask;

    SvFiber fiber;
    SvFiberTimer timer;

    SvWidget w;

    QBSubsTrack nullTrack;

    SvApplication app;

    SvBinaryTree displayedSubsByRemoveTime;
    SvDeque displayedSubsQueue;
    SvHashTable decodedSubs;

    SvArray plugins;

    struct {
        void (*fun)(void *target, QBSubsManager manager);
        void *target;
    } prefFun;

    struct {
        void (*callback)(void *callbackData, QBSubsTrack track);
        void *callbackData;
    } currTrackChanged;

    QBAsyncServiceState serviceState;
};

SvLocal SvString QBSubsManagerGetName(SvObject self_)
{
    return SVSTRING("QBSubsManager");
}

SvLocal SvImmutableArray QBSubsManagerGetDependencies(SvObject self_)
{
    return NULL;
}

SvLocal QBAsyncServiceState QBSubsManagerGetState(SvObject self_)
{
    QBSubsManager self = (QBSubsManager) self_;
    return self->serviceState;
}

SvLocal void QBSubsManagerStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    QBSubsManager self = (QBSubsManager) self_;
    self->serviceState = QBAsyncServiceState_running;
    self->w = svWidgetCreateBitmap(self->app, INT_MAX / 2, INT_MAX / 2, NULL);

    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBSubsManagerStop(SvObject self_, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    QBSubsManager self = (QBSubsManager) self_;
    if (svWidgetIsAttached(self->w)) {
        svWidgetDetach(self->w);
    }
    svWidgetDestroy(self->w);
    self->serviceState = QBAsyncServiceState_idle;

    SvErrorInfoPropagate(error, errorOut);
}

SvLocal void QBSubsManagerDestroy(void *self_)
{
    log_debug("");
    QBSubsManager self = (QBSubsManager) self_;
    SVRELEASE(self->tracks);
    SVRELEASE(self->ccTracks);
    SVRELEASE(self->queues);
    SVTESTRELEASE(self->currentTrack);
    SVTESTRELEASE(self->playerTask);
    SVRELEASE(self->nullTrack);
    SVRELEASE(self->displayedSubsByRemoveTime);
    SVRELEASE(self->displayedSubsQueue);
    SVRELEASE(self->decodedSubs);
    SVRELEASE(self->listeners);

    SvFiberEventDeactivate(self->timer);
    SvFiberDestroy(self->fiber);
}

SvLocal const char *QBSubsManagerGetConfigValue(const char *key, const char *defaultValue)
{
    const char *value = QBConfigGet(key);
    if (value && strcmp(value, "") != 0) {
        return value;
    }
    return defaultValue;
}

SvLocal void QBSubsManagerReadSettings(QBSubsManager self, QBSubsManualFontParams params)
{
    const char *value = NULL;

    params->useColor = false;
    params->color = COLOR(0xFF, 0xFF, 0xFF, 0xFF);
    value = QBSubsManagerGetConfigValue("UI.SUBTITLES.COLOR", "#FFFFFFFF");

    if (*value == '#') {
        value = &value[1];
        size_t len = 8; // RGBA
        unsigned char outputBuffer[len / 2];
        size_t res = QBHexDecode(value, len, outputBuffer, false);
        log_debug("hexdecode R:%d G:%d B:%d A:%d", outputBuffer[0],
                  outputBuffer[1], outputBuffer[2], outputBuffer[3]);
        if (res == len / 2) {
            params->color = COLOR(outputBuffer[0], outputBuffer[1],
                                  outputBuffer[2], outputBuffer[3]);
            params->useColor = true;
        } else {
            SvLogWarning("key: UI.SUBTITLES.COLOR value: %s - wrong format! #RRGGBBAA hex format expected.",
                         value);
        }
    } else {
        SvLogWarning("key: UI.SUBTITLES.COLOR value: %s - wrong format! #RRGGBBAA hex format expected.",
                     value);
    }

    params->useSize = false;
    value = QBSubsManagerGetConfigValue("UI.SUBTITLES.FONTSIZE", NULL);
    if (value) {
        int res = QBStringToUInteger(value, &params->size);
        if (res != -1) {
            params->useSize = true;
        } else {
            SvLogWarning("key: UI.SUBTITLES.FONTSIZE value: %s - unsupported value.",
                         value);
        }
    }

    value = QBSubsManagerGetConfigValue("UI.SUBTITLES.STYLE", "normal");
    if (strcasestr(value, "normal")) {
        params->style = SvFontStyle_normal;
    } else if (strcasestr(value, "bold")) {
        params->style = SvFontStyle_oblique;
    } else {
        SvLogWarning("key: UI.SUBTITLES.STYLE value: %s - unsupported value.",
                     value);
        params->style = SvFontStyle_normal;
    }
}

SvLocal void QBSubsManagerConfigChanged(SvObject self_, const char *key,
                                        const char *value)
{
    QBSubsManager self = (QBSubsManager) self_;
    log_debug("%s got config change, key: %s value: %s", __func__, key, value);

    if (self->plugins) {
        struct QBSubsManualFontParams_ params_;
        QBSubsManagerReadSettings(self, &params_);
        SvIterator it = SvArrayIterator(self->plugins);
        SvObject plugin;
        while ((plugin = SvIteratorGetNext(&it))) {
            SvInvokeInterface(QBSubsManagerPluginInterface, plugin, setup, &params_);
        }
    }
}

SvLocal SvType QBSubsManager_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBSubsManagerDestroy
    };
    static const struct QBAsyncService_ asyncServiceMethods = {
        .getName         = QBSubsManagerGetName,
        .getDependencies = QBSubsManagerGetDependencies,
        .getState        = QBSubsManagerGetState,
        .start           = QBSubsManagerStart,
        .stop            = QBSubsManagerStop
    };
    static const struct QBConfigListener_t configMethods = {
        .changed = QBSubsManagerConfigChanged
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBSubsManager",
                            sizeof(struct QBSubsManager_),
                            QBObservable_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBConfigListener_getInterface(), &configMethods,
                            QBAsyncService_getInterface(), &asyncServiceMethods,
                            NULL);
    }

    return type;
}

SvInterface
QBSubsManagerPluginInterface_getInterface(void)
{
    static SvInterface interface = NULL;
    SvErrorInfo error = NULL;
    if (!interface) {
        SvInterfaceCreateManaged("QBSubsManagerInterface", sizeof(struct QBSubsManagerPluginInterface_),
                                 NULL, &interface, &error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            abort();
        }
    }
    return interface;
}

int QBSubsManagerAddPlugin(QBSubsManager self, SvObject plugin, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed");
        goto err;
    } else if (!plugin || !SvObjectIsImplementationOf(plugin, QBSubsManagerPluginInterface_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "invalid plugin passed");
        goto err;
    }
    struct QBSubsManualFontParams_ params_;
    QBSubsManagerReadSettings(self, &params_);
    SvInvokeInterface(QBSubsManagerPluginInterface, plugin, setup, &params_);
    SvArrayAddObject(self->plugins, plugin);

err:
    SvErrorInfoPropagate(error, errorOut);
    return error ? -1 : 0;
}

int QBSubsManagerRemovePlugin(QBSubsManager self, SvObject plugin, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!self) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL self passed.");
        goto err;
    } else if (!plugin) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL plugin passed.");
        goto err;
    } else if (!SvArrayContainsObject(self->plugins, (SvObject) plugin)) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_notFound,
                                  "Given plugin does is not exist in array.");
        goto err;
    }

    SvArrayRemoveObject(self->plugins, (SvObject) plugin);

err:
    SvErrorInfoPropagate(error, errorOut);
    return error ? -1 : 0;
}

SvLocal void QBSubsManagerSubtitleDestroy(void *self_)
{
}

SvLocal void QBSubsManagerSubtitleDecode_(QBSubsManagerSubtitle self)
{
}

SvLocal void QBSubsManagerSubtitleRemove_(QBSubsManagerSubtitle self)
{
}

SvType QBSubsManagerSubtitle_getType(void)
{
    static const struct QBSubsManagerSubtitleVTable_ subtitleVTable = {
        .super_      = {
            .destroy = QBSubsManagerSubtitleDestroy
        },
        .decode      = QBSubsManagerSubtitleDecode_,
        .remove      = QBSubsManagerSubtitleRemove_
    };
    static SvType type = NULL;

    if (unlikely(!type)) {
        SvTypeCreateVirtual("QBSubsManagerSubtitle",
                            sizeof(struct QBSubsManagerSubtitle_),
                            SvObject_getType(),
                            sizeof(subtitleVTable), &subtitleVTable,
                            &type,
                            NULL);
    }

    return type;
}

SvLocal int
QBSubsManagerDisplayEndCompare(void *prv, SvObject subA_, SvObject subB_)
{
    QBSubsManagerSubtitle subA = (QBSubsManagerSubtitle) subA_;
    QBSubsManagerSubtitle subB = (QBSubsManagerSubtitle) subB_;

    assert(subA->ptsBits == subB->ptsBits);
    int64_t diff = pts_diff(subA->pts, subB->pts, subA->ptsBits);
    diff += subA->duration - subB->duration;
    if (diff < 0) {
        return -1;
    } else if (diff > 0) {
        return 1;
    } else {
        if (subA == subB) {
            return 0;
        } else if (subA < subB) {
            return -1;
        } else {
            return 1;
        }
    }
}

SvLocal void QBSubsManagerShowStats(QBSubsManager self)
{
    SvIterator it = SvHashTableValuesIterator(self->queues);
    SvDeque queue = NULL;
    SvDeque current = self->currentTrack ? (SvDeque) SvHashTableFind(self->queues, (SvObject) self->currentTrack) : NULL;
    QBSubsManagerSubtitle subtitle = NULL;
    log_stats("Total subtitles Ids: %zd", SvHashTableCount(self->queues));
    while ((queue = (SvDeque) SvIteratorGetNext(&it))) {
        char c = queue == current ? '*' : ' ';
        if ((subtitle = (QBSubsManagerSubtitle) SvDequeFront(queue))) {
            log_stats(" [%c]  pts=%llx [count=%zd]", c, (long long int) subtitle->pts, SvDequeCount(queue));
        } else {
            log_stats(" [%c]  [empty]", c);
        }
    }
}

SvLocal void QBSubsManagerClearSubtitles(QBSubsManager self)
{
    svWidgetDestroySubwidgets(self->w);

    SvBinaryTreeRemoveAllObjects(self->displayedSubsByRemoveTime);
    SvDequeRemoveAllObjects(self->displayedSubsQueue);
    SvHashTableRemoveAllObjects(self->decodedSubs);
}

SvLocal bool
QBSubsManagerSubtitlesInSync(int64_t nextShowTime90k)
{
    if (nextShowTime90k >= 0) {
        return true;
    }

    if (llabs(nextShowTime90k) > SUBTITLE_MAX_UNSYNC) {
        //display subtitle instant when unsync detected
        SvLogWarning("Subtitles are out of sync, difference bigger than %lld!!!", SUBTITLE_MAX_UNSYNC);
    }

    return false;
}

SvLocal void
QBSubsManagerShowSubtitles(QBSubsManager self, int64_t currentStc90k, int64_t *nextShowAfterUs, int64_t *nextRemoveAfterUs)
{
    SvDeque queue = (SvDeque) SvHashTableFind(self->queues, (SvObject) self->currentTrack);
    if (!queue) {
        log_error("!queue");
        return;
    }

    QBSubsManagerSubtitle subtitle;
    int64_t lastRemoveAfter = *nextRemoveAfterUs * 9 / 100;
    while ((subtitle = (QBSubsManagerSubtitle) SvDequeFront(queue))) {
        int64_t diff = pts_diff(subtitle->pts, currentStc90k, subtitle->ptsBits);
        if (diff > SUBTITLE_SHOW_TIME) {
            *nextShowAfterUs = (diff - SUBTITLE_SHOW_TIME) * 100 / 9.0;
            break;
        } else if (!QBSubsManagerSubtitlesInSync(diff)) {
            *nextShowAfterUs = 0;
        }

        if (subtitle->preClear) {
            QBSubsManagerClearSubtitles(self);
        }

        log_debug("Show subtitle, pts=[%lld] duration=[%lld]", (long long) subtitle->pts, (long long) subtitle->duration);
        QBSubsManagerSubtitleShow(subtitle, self->w, self->displayedSubsQueue);
        subtitle->pts = currentStc90k & ((1LL << subtitle->ptsBits) - 1);
        SvBinaryTreeInsert(self->displayedSubsByRemoveTime, (SvObject) subtitle);
        SvHashTableRemove(self->decodedSubs, (SvObject) subtitle);
        SVTESTRELEASE(SvDequeTakeFront(queue));

        if (subtitle->duration > 0 && subtitle->duration < lastRemoveAfter) {
            *nextRemoveAfterUs = subtitle->duration * 100 / 9.0;
            lastRemoveAfter = subtitle->duration;
        }
    }
}

SvLocal void
QBSubsManagerDecodeSubtitles(QBSubsManager self, int64_t currentStc90k, int64_t *nextDecodeAfterUs)
{
    SvDeque queue = (SvDeque) SvHashTableFind(self->queues, (SvObject) self->currentTrack);
    if (!queue) {
        log_error("No subtitles for current track");
        return;
    }

    SvIterator it = SvDequeIterator(queue);
    QBSubsManagerSubtitle subtitle;
    while ((subtitle = (QBSubsManagerSubtitle) SvIteratorGetNext(&it))) {
        if (SvHashTableFind(self->decodedSubs, (SvObject) subtitle)) {
            continue;
        }

        int64_t diff = pts_diff(subtitle->pts, currentStc90k, subtitle->ptsBits);
        if (diff > SUBTITLE_DECODE_TIME) {
            *nextDecodeAfterUs = (diff - SUBTITLE_DECODE_TIME) * 100 / 9.0;
            log_debug("Trigger next decode in [%lld us]", (long long) *nextDecodeAfterUs);
            break;
        }

        log_debug("Start decoding, pts=[%lld] duration=[%lld]", (long long) subtitle->pts, (long long) subtitle->duration);
        QBSubsManagerSubtitleDecode(subtitle);
        SvHashTableInsert(self->decodedSubs, (SvObject) subtitle, (SvObject) subtitle);
    }
}

SvLocal void
QBSubsManagerRemoveDisplayed(QBSubsManager self, int64_t currentStc90k, int64_t *nextRemoveAfterUs)
{
    size_t i = 0;
    size_t cnt = SvBinaryTreeNodesCount(self->displayedSubsByRemoveTime);

    QBSubsManagerSubtitle subtitle;
    while (i < cnt) {
        subtitle = (QBSubsManagerSubtitle) SvBinaryTreeObjectAtIndex(self->displayedSubsByRemoveTime, 0);
        int64_t diff = pts_diff(subtitle->pts, currentStc90k, subtitle->ptsBits) + subtitle->duration;
        if (diff >= 0) {
            *nextRemoveAfterUs = llabs(diff) * 100 / 9.0;
            break;
        }

        QBSubsManagerSubtitleRemove(subtitle);
        SvBinaryTreeRemove(self->displayedSubsByRemoveTime, (SvObject) subtitle);
        ++i;
    }
}

SvLocal void
QBSubsManagerRemoveExpired(QBSubsManager self, QBSubsTrack track, int64_t currentStc90k)
{
    SvDeque queue = (SvDeque) SvHashTableFind(self->queues, (SvObject) track);
    if (!queue) {
        log_error("No subtitles for current track");
        return;
    }

    QBSubsManagerSubtitle subtitle;
    while ((subtitle = (QBSubsManagerSubtitle) SvDequeFront(queue))) {
        int64_t diff = pts_diff(subtitle->pts, currentStc90k, subtitle->ptsBits) + subtitle->duration;
        if (diff >= 0) {
            break;
        }

        if (subtitle->duration == 0) {
            //If subtitle duration equals zero it means that subtitles have infinite duration. So they shouldn't expire before being shown.
            break;
        }

        if (llabs(diff) < SUBTITLE_MAX_LAG) {
            break;
        }

        QBSubsManagerSubtitleRemove(subtitle);
        SvHashTableRemove(self->decodedSubs, (SvObject) subtitle);
        SVTESTRELEASE(SvDequeTakeFront(queue));
    }
}

SvLocal void QBSubsManagerUpdateTimer(QBSubsManager self)
{
    if (self->currentTrack == QBSubsManagerGetNullTrack(self)) {
        SvFiberDeactivate(self->fiber);
        SvFiberEventDeactivate(self->timer);
        return;
    }

    if (!SvFiberEventIsActive(self->timer) || !SvFiberEventIsArmed(self->timer)) {
        SvFiberActivate(self->fiber);
    }
}

SvLocal void QBSubsManagerStep(void *self_)
{
    log_debug();
    QBSubsManager self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);

    if (!self->currentTrack) {
        log_debug("Current track is not set");
        return;
    }

    log_debug("lang=%s", SvStringCString(self->currentTrack->langCode));

    int64_t nextRemoveAfterUs = LLONG_MAX, nextDecodeAfterUs = LLONG_MAX, nextShowAfterUs = LLONG_MAX;

    SvPlayerTaskState state = SvPlayerTaskGetState(self->playerTask);
    QBSubsManagerRemoveExpired(self, self->currentTrack, state.currentStc90k);
    QBSubsManagerRemoveDisplayed(self, state.currentStc90k, &nextRemoveAfterUs);

    state = SvPlayerTaskGetState(self->playerTask);
    QBSubsManagerDecodeSubtitles(self, state.currentStc90k, &nextDecodeAfterUs);

    state = SvPlayerTaskGetState(self->playerTask);
    QBSubsManagerShowSubtitles(self, state.currentStc90k, &nextShowAfterUs, &nextRemoveAfterUs);

    int64_t wakeUpAfterUs = nextRemoveAfterUs < nextDecodeAfterUs ? nextRemoveAfterUs : nextDecodeAfterUs;
    if (nextShowAfterUs < wakeUpAfterUs) {
        wakeUpAfterUs = nextShowAfterUs;
    }

    if (wakeUpAfterUs != LLONG_MAX) {
        log_debug("Wake up fiber after [%lld ms]", (long long) wakeUpAfterUs / 1000);
        SvFiberTimerActivateAfter(self->timer, SvTimeFromMicroseconds(wakeUpAfterUs));
    }

    QBSubsManagerShowStats(self);
}

QBSubsManager QBSubsManagerNew(SvApplication app)
{
    log_debug();
    QBSubsManager self = (QBSubsManager) SvTypeAllocateInstance(QBSubsManager_getType(), NULL);

    self->nullTrack = (QBSubsTrack) SvTypeAllocateInstance(QBSubsTrack_getType(), NULL);
    self->nullTrack->langCode = SVSTRING("off");
    self->listeners = SvWeakListCreate(NULL);

    self->tracks = QBActiveArrayCreate(13, NULL);
    self->ccTracks = QBActiveArrayCreate(13, NULL);
    self->queues = SvHashTableCreate(13, NULL);

    self->fiber = SvFiberCreate(NULL, NULL, "QBSubsManager", &QBSubsManagerStep, self);
    self->timer = SvFiberTimerCreate(self->fiber);

    self->app = app;

    self->displayedSubsByRemoveTime = SvBinaryTreeCreateWithCompareFn(QBSubsManagerDisplayEndCompare, NULL, NULL);
    self->displayedSubsQueue = SvDequeCreate(NULL);

    self->decodedSubs = SvHashTableCreate(30, NULL);
    self->plugins = SvArrayCreate(NULL);

    QBSubsManagerRegisterTrack(self, QBSubsManagerGetNullTrack(self), DISABLE_SUBS_TRACK);
    QBSubsManagerRegisterTrackExt(self, QBSubsManagerGetNullTrack(self), QBSubsTrackType_closedCaptions, DISABLE_SUBS_TRACK);
    QBSubsManagerSetCurrentTrack(self, QBSubsManagerGetNullTrack(self));

    QBConfigAddListener((SvObject) self, "UI.SUBTITLES.COLOR");
    QBConfigAddListener((SvObject) self, "UI.SUBTITLES.FONTSIZE");
    QBConfigAddListener((SvObject) self, "UI.SUBTITLES.STYLE");

    return self;
}

QBSubsTrack QBSubsManagerRegisterTrack(QBSubsManager self, QBSubsTrack track, unsigned int id)
{
    QBSubsTrack subsTrack = QBSubsManagerRegisterTrackExt(self, track, QBSubsTrackType_subtitles, id);
    QBObservableSetChanged((QBObservable) self, NULL);
    QBObservableNotifyObservers((QBObservable) self, NULL, NULL);
    return subsTrack;
}

QBSubsTrack QBSubsManagerRegisterTrackExt(QBSubsManager self, QBSubsTrack track, QBSubsTrackType type, unsigned int id)
{
    log_debug("track=%p type=%d", track, type);
    QBActiveArray array = self->tracks;
    if (type == QBSubsTrackType_closedCaptions) {
        array = self->ccTracks;
    }
    if (QBActiveArrayIndexOfObject(array, (SvObject) track, NULL) >= 0) {
        log_error("%p already register", track);
        return NULL;
    }
    QBActiveArrayAddObject(array, (SvObject) track, NULL);

    SvDeque queue = SvDequeCreate(NULL);
    SvHashTableInsert(self->queues, (SvObject) track, (SvObject) queue);
    SVRELEASE(queue);

    SvImmutableArray listeners = (SvImmutableArray) SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvImmutableArrayGetIterator(listeners);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBSubsManagerListener, listener, trackAdded, track);
    }
    SVRELEASE(listeners);

    if (self->prefFun.fun) {
        self->prefFun.fun(self->prefFun.target, self);
    }

    track->id = id;

    return track;
}

void QBSubsManagerUnregisterTrack(QBSubsManager self, QBSubsTrack track)
{
    int idx = QBActiveArrayIndexOfObject(self->tracks, (SvObject) track, NULL);
    int ccIdx = QBActiveArrayIndexOfObject(self->ccTracks, (SvObject) track, NULL);
    if (idx < 0 && ccIdx < 0) {
        log_error("%p isn't registered", track);
        return;
    }
    if (track == QBSubsManagerGetNullTrack(self)) {
        log_error("Unable to unregister nullTrack");
        return;
    }
    if (idx >= 0) {
        QBActiveArrayRemoveObjectAtIndex(self->tracks, idx, NULL);
    }
    if (ccIdx >= 0) {
        QBActiveArrayRemoveObjectAtIndex(self->ccTracks, ccIdx, NULL);
    }

    SvHashTableRemove(self->queues, (SvObject) track);

    SvImmutableArray listeners = (SvImmutableArray) SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvImmutableArrayGetIterator(listeners);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBSubsManagerListener, listener, trackRemoved, track);
    }
    SVRELEASE(listeners);

    if (self->prefFun.fun) {
        self->prefFun.fun(self->prefFun.target, self);
    } else if (self->currentTrack == track) {
        QBSubsManagerSetCurrentTrack(self, QBSubsManagerGetNullTrack(self));
    }
}

void QBSubsManagerFlushSubtitleForTrack(QBSubsManager self, QBSubsTrack track)
{
    if (!self || !track) {
        log_error("NULL argument passed");
        return;
    }

    SvDeque queue = (SvDeque) SvHashTableFind(self->queues, (SvObject) track);
    if (!queue) {
        log_error("%p isn't registered", track);
        return;
    }
    SvDequeRemoveAllObjects(queue);

    if (track == self->currentTrack) {
        QBSubsManagerClearSubtitles(self);
    }

    QBObservableSetChanged((QBObservable) self, NULL);
    QBObservableNotifyObservers((QBObservable) self, NULL, NULL);
}

void QBSubsManagerAddSubtitleForTrack(QBSubsManager self, QBSubsTrack track, QBSubsManagerSubtitle subtitle)
{
    SvDeque queue = (SvDeque) SvHashTableFind(self->queues, (SvObject) track);
    if (!queue) {
        log_error("%p isn't registered", track);
        return;
    }

    QBSubsManagerSubtitle lastSubtitle;
    if ((lastSubtitle = (QBSubsManagerSubtitle) SvDequeBack(queue))) {
        if (subtitle->pts - lastSubtitle->pts < 0) { // Check if subtitles are in order, may be wrong i.e after rewinding
            QBSubsManagerFlushSubtitleForTrack(self, track);
        }
    }

    SvDequePushBack(queue, (SvObject) subtitle, NULL);
    if (SvDequeGetCount(queue) >= QB_SUBS_MANAGER_MAX_SUBS) {
        SVTESTRELEASE(SvDequeTakeFront(queue));
    }

    if (subtitle->immediate) {
        SvPlayerTaskState state = SvPlayerTaskGetState(self->playerTask);
        int64_t currentStc90k = state.currentStc90k;
        if (currentStc90k != -1) {
            subtitle->pts = currentStc90k & ((1LL << subtitle->ptsBits) - 1);
        }
    }

    QBSubsManagerUpdateTimer(self);
    QBObservableSetChanged((QBObservable) self, NULL);
    QBObservableNotifyObservers((QBObservable) self, NULL, NULL);
}

QBActiveArray QBSubsManagerGetAllTracks(QBSubsManager self)
{
    return self->tracks;
}

QBActiveArray QBSubsManagerGetClosedCaptionsTracks(QBSubsManager self)
{
    return self->ccTracks;
}

void QBSubsManagerSetCurrentTrack(QBSubsManager self, QBSubsTrack track)
{
    if (!self || !track) {
        log_error("NULL argument passed");
        return;
    }

    log_debug("track=%s currentTrack=%s [%s]", track ? SvStringCString(track->langCode) : "(nil)", self->currentTrack ? SvStringCString(self->currentTrack->langCode) : "(nil)", SvObjectGetTypeName((SvObject) self));
    if ((QBActiveArrayIndexOfObject(self->tracks, (SvObject) track, NULL) < 0) &&
        (QBActiveArrayIndexOfObject(self->ccTracks, (SvObject) track, NULL) < 0)) {
        log_error("%p isn't registered", track);
        return;
    }
    if (track == self->currentTrack) {
        return;
    }

    QBSubsManagerClearSubtitles(self);

    QBSubsTrack oldTrack = self->currentTrack;

    SVTESTRELEASE(self->currentTrack);
    self->currentTrack = SVRETAIN(track);

    if (oldTrack) {
        int idx = QBActiveArrayIndexOfObject(self->tracks, (SvObject) oldTrack, NULL);
        if (idx >= 0) {
            QBActiveArrayPropagateObjectsChange(self->tracks, idx, 1, NULL);
        }
        idx = QBActiveArrayIndexOfObject(self->ccTracks, (SvObject) oldTrack, NULL);
        if (idx >= 0) {
            QBActiveArrayPropagateObjectsChange(self->ccTracks, idx, 1, NULL);
        }
    }

    if (self->currentTrack) {
        int idx = QBActiveArrayIndexOfObject(self->tracks, (SvObject) self->currentTrack, NULL);
        if (idx >= 0) {
            QBActiveArrayPropagateObjectsChange(self->tracks, idx, 1, NULL);
        }
        idx = QBActiveArrayIndexOfObject(self->ccTracks, (SvObject) self->currentTrack, NULL);
        if (idx >= 0) {
            QBActiveArrayPropagateObjectsChange(self->ccTracks, idx, 1, NULL);
        }
    }

    QBSubsManagerUpdateTimer(self);

    QBObservableSetChanged((QBObservable) self, NULL);
    QBObservableNotifyObservers((QBObservable) self, NULL, NULL);

    QBSubsManagerShowStats(self);

    if (self->currTrackChanged.callbackData) {
        self->currTrackChanged.callback(self->currTrackChanged.callbackData, self->currentTrack);
    }

    SvImmutableArray listeners = (SvImmutableArray) SvWeakListCreateElementsList(self->listeners, NULL);
    SvIterator it = SvImmutableArrayGetIterator(listeners);
    SvObject listener = NULL;
    while ((listener = SvIteratorGetNext(&it))) {
        SvInvokeInterface(QBSubsManagerListener, listener, trackSet, self->currentTrack);
    }
    SVRELEASE(listeners);
}

QBSubsTrack QBSubsManagerGetCurrentTrack(QBSubsManager self)
{
    if (!self) {
        log_error("NULL self passed");
        return NULL;
    }

    return self->currentTrack;
}

void QBSubsManagerSetPlayerTask(QBSubsManager self, SvPlayerTask playerTask)
{
    SVTESTRELEASE(self->playerTask);
    self->playerTask = SVTESTRETAIN(playerTask);
}

SvPlayerTask QBSubsManagerGetPlayerTask(QBSubsManager self)
{
    return self->playerTask;
}

void QBSubsManagerAttach(QBSubsManager self, SvWidget parent, int x, int y, int w, int h, int level)
{
    QBSubsManagerClearSubtitles(self);
    svWidgetDetach(self->w);
    self->w->width = w;
    self->w->height = h;
    svWidgetAttach(parent, self->w, x, y, level);
}

void
QBSubsManagerAttachToQBCUITWidget(QBSubsManager self,
                                  QBCUITWidget parent,
                                  int xOffset, int yOffset,
                                  unsigned int width, unsigned int height,
                                  unsigned short int level)
{
    QBSubsManagerClearSubtitles(self);
    svWidgetDetach(self->w);
    self->w->width = width;
    self->w->height = height;
    QBCUITWidgetAttachSvWidget(parent, self->w, xOffset, yOffset, level);
}

void QBSubsManagerDetach(QBSubsManager self)
{
    svWidgetDetach(self->w);
}

void QBSubsManagerSetMute(QBSubsManager self, bool mute)
{
    //TODO don't render subtitles in mute mode
    svWidgetSetHidden(self->w, mute);
}

QBSubsTrack QBSubsManagerGetNullTrack(QBSubsManager self)
{
    return self->nullTrack;
}

void QBSubsManagerSetPreferenceFunction(QBSubsManager self, void (*fun)(void *target, QBSubsManager manager), void *target)
{
    self->prefFun.fun = fun;
    self->prefFun.target = target;
}

void QBSubsManagerSetCurrentTrackChangedCallback(QBSubsManager self, void (*callback)(void *callbackData, QBSubsTrack track), void *callbackData)
{
    self->currTrackChanged.callback = callback;
    self->currTrackChanged.callbackData = callbackData;
}

void
QBSubsManagerAddListener(QBSubsManager self, SvObject listener, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!listener || !SvObjectIsImplementationOf(listener, QBSubsManagerListener_getInterface())) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL or invalid argument passed: %p", listener);
        goto out;
    }

    SvWeakListPushBack(self->listeners, listener, &error);
    if (error) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_invalidState, error,
                                           "SvWeakListPushBack() failed");
        goto out;
    }

out:
    SvErrorInfoPropagate(error, errorOut);
}

void
QBSubsManagerRemoveListener(QBSubsManager self, SvObject listener, SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;

    if (!listener) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed: %p", listener);
        SvErrorInfoPropagate(error, errorOut);
        return;
    }

    SvWeakListRemoveObject(self->listeners, listener);
}

SvInterface
QBSubsManagerListener_getInterface(void)
{
    static SvInterface interface = NULL;

    if (unlikely(!interface)) {
        SvInterfaceCreateManaged("QBSubsManagerListener",
                                 sizeof(struct QBSubsManagerListener_),
                                 NULL, &interface, NULL);
    }
    return interface;
}
