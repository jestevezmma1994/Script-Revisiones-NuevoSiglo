/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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
#include <Logic/QBParentalControlLogic.h>
#include <Utils/QBEventUtils.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <main.h>

#include <stddef.h>

struct QBParentalControlLogic_ {
    struct SvObject_ super_;
    int timeMs;
    bool userSetTime;

    QBParentalControlHelper helper;
    AppGlobals appGlobals;
};

#define DEFAULT_NO_RATING_CHN_BLOCKING_TIME_MS  2000
/*
 * It's time which block channel after start system.
 * It's required becouse starting system is longer that only set channel.
 */
#define INITIAL_NO_RATING_CHN_BLOCKING_TIME_MS  4000

// QBParentalControlLogicMethods methods

SvLocal void
QBParentalControlLogicSelectedRatingsChanged_(SvObject self_, QBPCList newRatings, SvArray enabledStandards)
{
}

SvLocal bool
QBParentalControlLogicIsAuthNeededForChannel_(SvObject self_, SvTVChannel channel, bool isEventRatingUnknown)
{
    QBParentalControlLogic self = (QBParentalControlLogic) self_;

    if (QBParentalControlLogicAdultIsBlocked(self)) {
        SvValue isAdultVal = channel ? ((SvValue) SvTVChannelGetAttribute(channel, SVSTRING("isAdult"))) : NULL;
        bool isAdult = isAdultVal && SvValueGetBoolean(isAdultVal);
        if (isAdult) {
            return true;
        }
    }

    if (QBParentalControlLogicUnknownRatingIsBlocked(self) && isEventRatingUnknown) {
        return true;
    }

    return false;
}

SvLocal bool
QBParentalControlLogicAdultIsBlocked_(SvObject self_)
{
    QBParentalControlLogic self = (QBParentalControlLogic) self_;

    QBParentalControlAdultContentBlockingState adultContentBlockingState = QBParentalControlGetAdultContentBlockingState(self->appGlobals->pc);
    QBParentalControlState pcState = QBParentalControlGetState(self->appGlobals->pc);

    if ((pcState == QBParentalControlState_On) && (adultContentBlockingState == QBParentalControlAdultContentBlockingState_On)) {
        return true;
    }

    return false;
}

SvLocal SvString
QBParentalControlLogicCreateDescriptionStringFromPCList_(SvObject self_, QBPCList list, SvErrorInfo* errorOut)
{
    return QBPCUtilsCreateDescriptionStringFromPCList(list, true, errorOut);
}

SvLocal SvString
QBParentalControlLogicCreateLockedRatingString_(SvObject self_, SvEPGManager epgManager, SvEPGEvent event)
{
    QBParentalControlLogic self = (QBParentalControlLogic) self_;
    QBPCList ratingList = QBEventUtilsCreateEventRating(self->appGlobals->eventsLogic, epgManager, event);
    SvString rating = QBPCListCreateLockedRatingsString(ratingList, NULL);
    SVTESTRELEASE(ratingList);

    return rating;
}

// SvObject virtual methods

SvType
QBParentalControlLogic_getType(void)
{
    static SvType t = NULL;

    static const struct QBParentalControlLogicMethods_ pcMethods = {
        .selectedRatingsChanged            = QBParentalControlLogicSelectedRatingsChanged_,
        .isAuthNeededForChannel            = QBParentalControlLogicIsAuthNeededForChannel_,
        .adultIsBlocked                    = QBParentalControlLogicAdultIsBlocked_,
        .createDescriptionStringFromPCList = QBParentalControlLogicCreateDescriptionStringFromPCList_,
        .createLockedRatingString          = QBParentalControlLogicCreateLockedRatingString_,
    };

    if (!t) {
        t = SvTypeCreateManaged("QBParentalControlLogic", sizeof(struct QBParentalControlLogic_),
                                SvObject_getType(), &t,
                                QBParentalControlLogicMethods_getInterface(), &pcMethods,
                                NULL);
    }

    return t;
}

// Public API

void
QBParentalControlLogicStart(QBParentalControlLogic self)
{
}

void
QBParentalControlLogicStop(QBParentalControlLogic self)
{
}

void
QBParentalControlLogicStartHelperMonitoring(QBParentalControlLogic self, QBParentalControlHelper helper)
{
    SVTESTRELEASE(self->helper);
    self->helper = SVTESTRETAIN(helper);
}

void
QBParentalControlLogicAddParentalControlHelper(QBParentalControlLogic self, QBParentalControlHelper helper)
{
}

void
QBParentalControlLogicRemoveParentalControlHelper(QBParentalControlLogic self, QBParentalControlHelper helper)
{
}

extern void
QBParentalControlLogicStopHelperMonitoring(QBParentalControlLogic self, QBParentalControlHelper helper)
{
    SVTESTRELEASE(self->helper);
    self->helper = NULL;
}

void
QBParentalControlLogicUpdateState(QBParentalControlLogic self, QBParentalControlHelper helper)
{
}

QBParentalControlLogicAuthState
QBParentalControlLogicGetAuthState(QBParentalControlLogic self, QBParentalControlHelper helper)
{
    if (!self->helper) {
        // It must be Enable becouse when PC mode is off there are no helper.
        // For PC mode on helper existence is guaranteed before any playback
        return QBParentalControlLogicAuthState_EnableAV;
    }

    if (!QBParentalControlHelperIsReady(self->helper)) {
        return QBParentalControlLogicAuthState_DisableAV;
    }

    if (QBParentalControlHelperAuthNeeded(self->helper)) {
        return QBParentalControlLogicAuthState_PINPopup_DisableAV;
    }

    return QBParentalControlLogicAuthState_EnableAV;
}

int
QBParentalControlLogicGetTimeMsToUnblockChannel(QBParentalControlLogic self)
{
    if (!self->userSetTime && self->timeMs == INITIAL_NO_RATING_CHN_BLOCKING_TIME_MS) {
        self->timeMs = DEFAULT_NO_RATING_CHN_BLOCKING_TIME_MS;
        return INITIAL_NO_RATING_CHN_BLOCKING_TIME_MS;
    }
    return self->timeMs;
}

void
QBParentalControlLogicSetTimeMsToUnblockChannel(QBParentalControlLogic self, int timeMs)
{
    self->userSetTime = true;
    self->timeMs = timeMs;
}

QBParentalControlLogic
QBParentalControlLogicCreate(AppGlobals appGlobals)
{
    QBParentalControlLogic self = (QBParentalControlLogic) SvTypeAllocateInstance(QBParentalControlLogic_getType(), NULL);
    self->userSetTime = false;
    self->timeMs = INITIAL_NO_RATING_CHN_BLOCKING_TIME_MS;
    self->appGlobals = appGlobals;
    return self;
}

bool
QBParentalControlLogicAdultIsBlocked(QBParentalControlLogic self)
{
    return QBParentalControlLogicAdultIsBlocked_((SvObject) self);
}

bool
QBParentalControlLogicUnknownRatingIsBlocked(QBParentalControlLogic self)
{
    QBParentalControlUnknownRatingBlockingState unknownRatingBlockingState = QBParentalControlGetUnknownRatingBlockingState(self->appGlobals->pc);
    QBParentalControlState pcState = QBParentalControlGetState(self->appGlobals->pc);

    return (pcState == QBParentalControlState_On) && (unknownRatingBlockingState == QBParentalControlUnknownRatingBlockingState_On);
}

bool QBParentalControlLogicIsAuthenticated(QBParentalControlLogic self)
{
    if (!self->helper) {
        return true;
    }

    return (QBParentalControlHelperGetAuthenticationStatus(self->helper) == QBAuthStatus_OK);
}

bool
QBParentalControlLogicIsAuthNeededForChannel(QBParentalControlLogic self, SvTVChannel channel, bool isEventRatingUnknown)
{
    return QBParentalControlLogicIsAuthNeededForChannel_((SvObject) self, channel, isEventRatingUnknown);
}

bool
QBParentalControlLogicIsContentSourceTypeLocked(QBParentalControlLogic self, QBParentalControlHelper helper)
{
    return QBParentalControlHelperIsContentSourceTypeLocked(helper);
}

bool
QBParentalControlLogicIsAuthNeededForRecord(QBParentalControlLogic self, QBPVRRecording rec)
{
    if (rec->isAdult && QBParentalControlLogicAdultIsBlocked(self)) {
        return true;
    }

    return false;
}

void
QBParentalControlLogicSelectedRatingsChanged(QBParentalControlLogic self, QBPCList newRatings, SvArray enabledStandards)
{
    QBParentalControlLogicSelectedRatingsChanged_((SvObject) self, newRatings, enabledStandards);
}

SvString QBParentalControlLogicCreateDescriptionStringFromPCList(QBParentalControlLogic self, QBPCList list, SvErrorInfo* errorOut)
{
    return QBParentalControlLogicCreateDescriptionStringFromPCList_((SvObject) self, list, errorOut);
}
