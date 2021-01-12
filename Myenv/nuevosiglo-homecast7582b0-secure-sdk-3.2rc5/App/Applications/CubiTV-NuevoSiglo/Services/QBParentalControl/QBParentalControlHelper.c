/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <Services/QBParentalControl/QBParentalControlHelper.h>

#include <Utils/QBEventUtils.h>
#include <Services/QBAccessController/QBAccessManagerListener.h>
#include <SvEPGDataLayer/Views/SvEPGView.h>
#include <SvEPGDataLayer/SvEPGChannelView.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <fibers/c/fibers.h>
#include <QBConf.h>
#include <SvCore/SvLogColors.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvCommonDefs.h>

#include <main.h>

#define QBTimeLimitEnable
#define QBTimeThreshold (50 * 1000)
#include <QBTimeLimit.h>

#define log_debug(fmt, ...)  if (0) SvLogNotice(COLBEG() "QBParentalControl: %s:%d " fmt COLEND_COL(blue), __func__, __LINE__, ## __VA_ARGS__)

struct QBParentalControlHelper_t {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    SvEPGManager epgManager;

    SvFiber fiber;
    SvFiberTimer timer;

    SvFiber authCheckingFiber;
    SvFiberTimer authCheckingTimer;

    SvFiber blockFiber;
    SvFiberTimer blockTimer;

    bool timePassed;
    bool ratingReady;
    bool setupCallbackWasInvoked;

    void *eventSourceData;
    QBParentalControlHelperEventSource eventSource;

    SvTVChannel channel;
    QBPVRRecording rec;
    SvString vodRating;
    bool vodAdult;

    QBPCList rating;

    bool hasUserAuthentication;
    bool waitingForAuthentication;

    QBParentalControlAuthenticationCallback callback;
    void *callbackData;
    SvEPGView tvChannels;

    QBParentalControlHelperContentSourceType contentSourceType; ///< type of source used to deliver content
};

SvLocal void
QBParentalControlHelperInvokeCallback(QBParentalControlHelper self);

SvLocal void
QBParentalControlHelper__dtor__(void *self_)
{
    QBParentalControlHelper self = self_;
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
    }

    if (self->blockFiber) {
        SvFiberDestroy(self->blockFiber);
    }

    if (self->authCheckingFiber) {
        SvFiberDestroy(self->authCheckingFiber);
    }

    SVTESTRELEASE(self->channel);
    SVTESTRELEASE(self->tvChannels);
    SVTESTRELEASE(self->rating);
    SVTESTRELEASE(self->rec);
    SVTESTRELEASE(self->epgManager);
}

QBAuthStatus QBParentalControlHelperGetAuthenticationStatus(QBParentalControlHelper self)
{
    return QBAccessManagerGetAuthenticationStatus(self->appGlobals->accessMgr, SVSTRING("PC"), NULL);
}

SvLocal void QBParentalControlHelperAuthenticationCheckingStep(void *self_)
{
    QBParentalControlHelper self = (QBParentalControlHelper) self_;
    SvFiberDeactivate(self->authCheckingFiber);
    SvFiberEventDeactivate(self->authCheckingTimer);

    QBAuthStatus status = QBParentalControlHelperGetAuthenticationStatus(self);
    switch (status) {
        case QBAuthStatus_OK:
            self->waitingForAuthentication = false;
            QBParentalControlAuthenticationChanged((SvGenericObject) self->appGlobals->pc, self->appGlobals->accessMgr, SVSTRING("PC"));
            return;
        case QBAuthStatus_invalid:
        case QBAuthStatus_unknown:
        case QBAuthStatus_failed:
        case QBAuthStatus_rejected:
        case QBAuthStatus_blocked:
        case QBAuthStatus_notAuthorised:
            self->waitingForAuthentication = false;
            QBParentalControlAuthenticationChanged((SvGenericObject) self->appGlobals->pc, self->appGlobals->accessMgr, SVSTRING("PC"));
            return;
        case QBAuthStatus_inProgress:
            SvFiberTimerActivateAfter(self->authCheckingTimer, SvTimeConstruct(0, 400));
            break;
    }
}

SvLocal
void QBParentalControlHelperAuthenticationChanged(SvGenericObject self_,
                                                  QBAccessManager manager,
                                                  SvString domainName)
{
    QBParentalControlHelper self = (QBParentalControlHelper) self_;

    if (!SvStringEqualToCString(domainName, "PC")) {
        return;
    }
    if (QBAccessManagerGetAuthenticationStatus(manager, SVSTRING("PC"), NULL) != QBAuthStatus_OK) {
        self->hasUserAuthentication = false;
    }

    if (self->authCheckingFiber) {
        SvFiberActivate(self->authCheckingFiber);
    }
}

SvLocal void
QBParentalControlHelperConfPCStateChanged(SvGenericObject self_, const char *key, const char *value)
{
    QBParentalControlHelper self = (QBParentalControlHelper) self_;
    log_debug("PC state changed notification");
    QBParentalControlHelperReset(self);
    QBParentalControlHelperUpdateRating(self);
    QBParentalControlLogicUpdateState(self->appGlobals->parentalControlLogic, self);

    if (QBParentalControlGetState(self->appGlobals->pc) == QBParentalControlState_On) {
        QBParentalControlLogicStartHelperMonitoring(self->appGlobals->parentalControlLogic, self);
    } else {
        QBParentalControlLogicStopHelperMonitoring(self->appGlobals->parentalControlLogic, self);
    }
}

SvLocal SvType
QBParentalControlHelper_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBParentalControlHelper__dtor__
    };
    static SvType type = NULL;

    static const struct QBAccessManagerListener_t accessManagerMethods = {
        .authenticationChanged = QBParentalControlHelperAuthenticationChanged
    };

    static const struct QBConfigListener_t config_methods = {
        .changed = QBParentalControlHelperConfPCStateChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBParentalControlHelper",
                            sizeof(struct QBParentalControlHelper_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBAccessManagerListener_getInterface(), &accessManagerMethods,
                            QBConfigListener_getInterface(), &config_methods,
                            NULL);
    }
    return type;
}

bool
QBParentalControlHelperUpdateRating(QBParentalControlHelper self)
{
    SvErrorInfo error = NULL;
    if (self->vodRating) {
        QBPCList current = QBPCListCreate(&error);
        if (error) {
            goto err;
        }
        QBPCListLockRatingsFromString(current, self->vodRating, &error);
        if (error) {
            SVTESTRELEASE(current);
            goto err;
        }
        SVTESTRELEASE(self->rating);
        self->rating = current;
        self->ratingReady = true;
        return true;
    }

    if (!self->eventSource) {
        return false;
    }

    SvEPGEvent event = NULL;
    self->eventSource(self->eventSourceData, self->channel, SvTimeNow(), &event, NULL);

    if (!event) {
        return false;
    }
    QBPCList current = QBEventUtilsCreateEventRating(self->appGlobals->eventsLogic,
                                                     self->epgManager, event);
    if (current) {
        if (self->rating) {
            SVTESTRELEASE(self->rating);
        }
        self->rating = current;
        self->ratingReady = true;
    }

    return true;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return false;
}

bool
QBParentalControlHelperIsReady(QBParentalControlHelper self)
{
    return self->timePassed || self->ratingReady;
}

SvLocal bool
QBParentalControlHelperIsRatingUnknown(QBParentalControlHelper self)
{
    return (self->timePassed && !self->ratingReady);
}


SvLocal bool QBParentalControlHelperIsRecordingBlocked(QBParentalControlHelper self, QBPVRRecording rec)
{
    if (rec->isAdult) {
        return true;
    }

    if (QBParentalControlLogicIsAuthNeededForRecord(self->appGlobals->parentalControlLogic, rec)) {
        return true;
    }

    return false;
}

SvLocal SvTVChannel QBParentalControlHelperGetRecordingChannel(QBParentalControlHelper self, QBPVRRecording rec)
{
    SvTVChannel channel;
    SvValue val = SvValueCreateWithString(rec->channelId, NULL);
    channel = SvInvokeInterface(SvEPGChannelView, self->tvChannels, getByID, val);
    SVRELEASE(val);
    return channel;
}

SvLocal SvString QBParentalControlHelperGetRatingString(QBParentalControlHelper self)
{
    if (!self->rating) {
        log_debug("rating == NULL");
        return NULL;
    }

    SvErrorInfo error = NULL;
    SvString ratingStr = QBPCListCreateLockedRatingsString(self->rating, &error);
    log_debug("has rating: %s", SvStringCString(ratingStr));
    if (error) {
        SvErrorInfoWriteLogMessage(error);
        SvErrorInfoDestroy(error);
        SVRELEASE(ratingStr);
        return NULL;
    }

    return ratingStr;
}

bool
QBParentalControlHelperAuthNeeded(QBParentalControlHelper self)
{
    if (self->hasUserAuthentication) {
        return false;
    }

    if (QBParentalControlGetState(self->appGlobals->pc) != QBParentalControlState_On) {
        return false;
    }

    SvTVChannel channel = self->channel;

    // BEGIN AMERELES Mensaje de control parental aparece siempre
    if (!self->ratingReady)
    {
        if (!channel || !QBParentalControlChannelIsBlocked(self->appGlobals->pc, channel))
        {
            return false;
        }
    }
    // END AMERELES Mensaje de control parental aparece siempre

    if (self->rec) {
        if (QBParentalControlHelperIsRecordingBlocked(self, self->rec)) {
            return true;
        }
        channel = QBParentalControlHelperGetRecordingChannel(self, self->rec);
    }

    // force time block if neither channel nor recording was set (npvr/catchup/vod)
    if (QBParentalControlIsLockedTimeActive(self->appGlobals->pc)) {
        return true;
    }

    if (self->vodAdult && QBParentalControlLogicAdultIsBlocked(self->appGlobals->parentalControlLogic)) {
        return true;
    }

    bool isRatingUnknown = QBParentalControlHelperIsRatingUnknown(self);

    if (isRatingUnknown && !QBParentalControlLogicIsContentSourceTypeLocked(self->appGlobals->parentalControlLogic, self)) {
        return false;
    }

    SvString ratingStr = QBParentalControlHelperGetRatingString(self);
    bool needed = QBParentalControlChannelWithEventRatingShouldBeBlocked(self->appGlobals->pc, channel, ratingStr, isRatingUnknown);
    SVTESTRELEASE(ratingStr);

    return needed;
}

SvLocal void
QBParentalControlHelperInvokeCallback(QBParentalControlHelper self)
{
    if (self->callback) {
        self->callback(self->callbackData);
    }
}

QBPCList
QBParentalControlHelperGetCurrentRating(QBParentalControlHelper self)
{
    return self->rating;
}

void
QBParentalControlHelperCheck(QBParentalControlHelper self)
{
    if (!self->setupCallbackWasInvoked && QBParentalControlHelperIsReady(self)) {
        self->setupCallbackWasInvoked = true;
        QBParentalControlHelperInvokeCallback(self);
    }

    if (!QBParentalControlHelperAuthNeeded(self)) {
        log_debug("Not needed");
        if (self->waitingForAuthentication) {
            self->waitingForAuthentication = false;
            log_debug("state: auth not needed");
            QBParentalControlHelperInvokeCallback(self);
        }
        return;
    }

    if (!self->waitingForAuthentication) {
        self->waitingForAuthentication = true;
        log_debug("state: auth needed");
        QBParentalControlHelperInvokeCallback(self);
    }
}

void
QBParentalControlHelperForcedNotify(QBParentalControlHelper self)
{
    QBParentalControlHelperInvokeCallback(self);
}

void
QBParentalControlHelperAuthenticated(QBParentalControlHelper self)
{
    self->waitingForAuthentication = false;
    self->hasUserAuthentication = true;
    QBParentalControlHelperInvokeCallback(self);
}

QBParentalControlHelper
QBParentalControlHelperNew(AppGlobals appGlobals)
{
    QBParentalControlHelper self = (QBParentalControlHelper) SvTypeAllocateInstance(QBParentalControlHelper_getType(), NULL);
    self->appGlobals = appGlobals;
    self->epgManager = SVRETAIN(QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("SvEPGManager")));
    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    self->tvChannels = SVRETAIN(QBPlaylistManagerGetById(playlists, SVSTRING("TVChannels")));
    self->vodAdult = false;
    self->contentSourceType = QBParentalControlHelperContentSourceType_Other;
    return self;
}

SvLocal void
QBParentalControlHelperUnblockChannelStep(void *self_)
{
    QBParentalControlHelper self = self_;
    log_debug("Blocking time passed");
    self->timePassed = true;
    SvFiberDeactivate(self->blockFiber);
    SvFiberEventDeactivate(self->blockTimer);
}

SvLocal void
QBParentalControlHelperPoll(void *self_)
{
    QBParentalControlHelper self = self_;
    SvFiberDeactivate(self->fiber);
    SvFiberEventDeactivate(self->timer);
    SvFiberTimerActivateAfter(self->timer, SvTimeConstruct(1, 0));

    QBTimeLimitBegin(timeParentalControl, QBTimeThreshold);

    QBParentalControlHelperUpdateRating(self);
    QBTimeLimitEnd_(timeParentalControl, "ParentalControl-poll.updateRating");

    QBParentalControlHelperCheck(self); // Check can set the timer
    QBTimeLimitEnd_(timeParentalControl, "ParentalControl-poll.check");
}

void
QBParentalControlHelperDeactivate(QBParentalControlHelper self)
{
    if (self->fiber) {
        SvFiberDestroy(self->fiber);
        self->fiber = NULL;
        self->timer = NULL;
    }

    if (self->blockFiber) {
        SvFiberDestroy(self->blockFiber);
        self->blockFiber = NULL;
        self->blockTimer = NULL;
    }

    if (self->authCheckingFiber) {
        SvFiberDestroy(self->authCheckingFiber);
        self->authCheckingFiber = NULL;
        self->authCheckingTimer = NULL;
    }

    QBAccessManagerRemoveListener(self->appGlobals->accessMgr, (SvGenericObject) self, NULL);
    QBConfigRemoveListener((SvGenericObject) self, "PC_STATE");
}

void
QBParentalControlHelperActivate(QBParentalControlHelper self)
{
    if (!self->fiber) {
        self->fiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "ParentalControl-poll",
                                    QBParentalControlHelperPoll, self);
        self->timer = SvFiberTimerCreate(self->fiber);
        SvFiberActivate(self->fiber);
    }

    if (!self->blockFiber) {
        self->blockFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "ParentalControl-block", QBParentalControlHelperUnblockChannelStep, self);
        self->blockTimer = SvFiberTimerCreate(self->blockFiber);
        SvFiberTimerActivateAfter(self->blockTimer, SvTimeConstruct(0, QBParentalControlLogicGetTimeMsToUnblockChannel(self->appGlobals->parentalControlLogic) * 1000));
    }

    if (!self->authCheckingFiber) {
        self->authCheckingFiber = SvFiberCreate(self->appGlobals->scheduler, NULL, "ParentalControl-authChecking",
                                                QBParentalControlHelperAuthenticationCheckingStep, self);
        self->authCheckingTimer = SvFiberTimerCreate(self->authCheckingFiber);
    }

    QBAccessManagerAddListener(self->appGlobals->accessMgr, (SvGenericObject) self, NULL);
    QBConfigAddListener((SvGenericObject) self, "PC_STATE");
}

void QBParentalControlHelperReset(QBParentalControlHelper self)
{
    log_debug("reset");
    self->hasUserAuthentication = false;
    self->waitingForAuthentication = false;
    if (self->rating) {
        SVRELEASE(self->rating);
        self->rating = NULL;
    }
    SVTESTRELEASE(self->vodRating);
    self->vodRating = NULL;
    self->vodAdult = false;
    self->ratingReady = false;
    self->setupCallbackWasInvoked = false;

    QBAccessManagerInvalidateAuthentication(self->appGlobals->accessMgr, SVSTRING("PC"), NULL);
}

void
QBParentalControlHelperSetEventSource(QBParentalControlHelper self, void *eventSourceData, QBParentalControlHelperEventSource eventSource)
{
    self->eventSourceData = eventSourceData;
    self->eventSource = eventSource;

    if (self->rating) {
        SVTESTRELEASE(self->rating);
        SvErrorInfo error = NULL;
        self->rating = QBPCListCreate(&error);
        if (error) {
            SvErrorInfoWriteLogMessage(error);
            SvErrorInfoDestroy(error);
            return;
        }
    }

    QBParentalControlHelperUpdateRating(self);
    QBParentalControlHelperCheck(self);
}

void
QBParentalControlHelperSetChannel(QBParentalControlHelper self, SvTVChannel channel)
{
    self->timePassed = false;
    SVTESTRELEASE(self->channel);
    self->channel = SVTESTRETAIN(channel);
}

void
QBParentalControlHelperSetRecording(QBParentalControlHelper self, QBPVRRecording rec)
{
    SVTESTRELEASE(self->rec);
    self->rec = SVTESTRETAIN(rec);
}

void
QBParentalControlHelperSetVodRating(QBParentalControlHelper self, SvString rating)
{
    SVTESTRELEASE(self->vodRating);
    self->vodRating = SVTESTRETAIN(rating);
}

void
QBParentalControlHelperSetVodAdult(QBParentalControlHelper self, bool flag)
{
    self->vodAdult = flag;
}

void
QBParentalControlHelperSetRatingFromEvent(QBParentalControlHelper self, SvEPGEvent event)
{
    if (!self || !event) {
        return;
    }

    QBPCList current = QBEventUtilsCreateEventRating(self->appGlobals->eventsLogic, self->epgManager, event);
    if (current) {
        SVTESTRELEASE(self->rating);
        self->rating = current;
    }
}

bool
QBParentalControlHelperHasChannel(QBParentalControlHelper self)
{
    return (self->channel != NULL);
}

bool
QBParentalControlHelperHasRecording(QBParentalControlHelper self)
{
    return (self->rec != NULL);
}

bool
QBParentalControlHelperHasUserAuthentication(QBParentalControlHelper self)
{
    return self->hasUserAuthentication;
}

void
QBParentalControlHelperSetCallback(QBParentalControlHelper self, QBParentalControlAuthenticationCallback callback, void *callbackData)
{
    self->callback = callback;
    self->callbackData = callbackData;
}

void
QBParentalControlHelperSetContentSourceType(QBParentalControlHelper self, QBParentalControlHelperContentSourceType type)
{
    self->contentSourceType = type;
}

bool
QBParentalControlHelperIsContentSourceTypeLocked(QBParentalControlHelper self)
{
    switch (self->contentSourceType) {
        case QBParentalControlHelperContentSourceType_Other:
            return true;
        case QBParentalControlHelperContentSourceType_USB:
            return false;
        default:
            assert(0);
            break;
    }
    return true;
}
