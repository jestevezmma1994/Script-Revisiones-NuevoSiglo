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


#ifndef QBPARENTALCONTROL_H
#define QBPARENTALCONTROL_H

/**
 * @file QBParentalControl.h
 * @brief Parental Control API
 **/

/**
 * @defgroup QBParentalControl Parental Control
 * @ingroup CubiTV_services
 * @{
 **/

#include <Services/QBParentalControl/QBPCLockedTimes.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvStringBuffer.h>
#include <QBPVRRecording.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <SvPlayerKit/SvEPGEvent.h>

#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <player_opts/epg_event.h>
#include <stdbool.h>
#include <Services/QBAccessController/QBAccessManager.h>

#include <QBPCRatings/QBPCList.h>
#include <QBPCRatings/QBPCRating.h>

typedef enum {
    QBParentalControlState_On,
    QBParentalControlState_Off_AutoRelock,
    QBParentalControlState_Off_RelockManually,
    QBParentalControlState_Max,
} QBParentalControlState;

typedef enum {
    QBParentalControlAdultContentBlockingState_On,
    QBParentalControlAdultContentBlockingState_Off,
    QBParentalControlAdultContentBlockingState_Max
} QBParentalControlAdultContentBlockingState;

typedef enum {
    QBParentalControlUnknownRatingBlockingState_On,
    QBParentalControlUnknownRatingBlockingState_Off,
    QBParentalControlUnknownRatingBlockingState_Max
} QBParentalControlUnknownRatingBlockingState;

typedef enum {
    QBParentalControlRatingStatus_InactiveUnlocked,
    QBParentalControlRatingStatus_ActiveUnlocked,
    QBParentalControlRatingStatus_InactiveLocked,
    QBParentalControlRatingStatus_ActiveLocked
} QBParentalControlRatingStatus;

typedef enum {
    QBParentalControlChannelLockingStatus_NotLocked,
    QBParentalControlChannelLockingStatus_InactiveLocked,
    QBParentalControlChannelLockingStatus_ActiveLocked
} QBParentalControlChannelLockingStatus;

/**
 * Get runtime type identification object representing QBParentalControlLogicMethods interface.
 *
 * @return object representing runtime type of QBParentalControlLogicMethods
 **/
SvInterface
QBParentalControlLogicMethods_getInterface(void);

/**
 * QBParentalControlLogicMethods interface.
 * This interface is aimed to provide access to QBParentalControlLogic
 * public API without including it in QBParentalControl service.
 **/
typedef const struct QBParentalControlLogicMethods_ {
    /**
     * Called when ratings configuration was changed in Parental Control menu.
     *
     * @param[in] self               object implementing QBParentalControlLogicMethods
     * @param[in,out] newRatings     new ratings configuration to be used
     * @param[in] enabledStandards   enabled PC standards (can be empty)
     **/
    void (*selectedRatingsChanged)(SvObject self_,
                                   QBPCList newRatings,
                                   SvArray enabledStandards);

    /**
     * Checks if parental authorization for specific channel is required.
     *
     * @param[in] self                    object implementing QBParentalControlLogicMethods
     * @param[in] channel                 channel for which parental control is checked
     * @param[in] isEventRatingUnknown    true when might not have complete information about the event rating yet (especially when is missing)
     *
     * @return                            true when QBParentalControlLogic demands parental authorization.
     **/
    bool (*isAuthNeededForChannel)(SvObject self_,
                                   SvTVChannel channel,
                                   bool isEventRatingUnknown);

    /**
     * Check whether adult titles and content is blocked in the application.
     *
     * @param[in] self    object implementing QBParentalControlLogicMethods
     *
     * @return            true if adult content is blocked, false if not.
     **/
    bool (*adultIsBlocked)(SvObject self_);

    /**
     * Creates description from PClist.
     *
     * @param[in] self_        object implementing QBParentalControlLogicMethods
     * @param[in] list         QBPCList handle
     * @param[out] errorOut    error information handle
     *
     * @return output string containing information about ratings
     **/
    SvString (*createDescriptionStringFromPCList)(SvObject self_,
                                                  QBPCList list,
                                                  SvErrorInfo *errorOut);

    /**
     * Creates locked rating string from EPG event.
     *
     * @param[in] self_         object implementing QBParentalControlLogicMethods
     * @param[in] epgManager    epgManager handle
     * @param[in] event         epgEvent handle
     *
     * @return created rating string
     **/
    SvString (*createLockedRatingString)(SvObject self_,
                                         SvEPGManager epgManager,
                                         SvEPGEvent event);
} *QBParentalControlLogicMethods;

/**
 * QBParentalControl service.
 *
 * @class QBParentalControl QBParentalControl.h  <Services/QBParentalControl.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 **/
typedef struct QBParentalControl_ *QBParentalControl;

/**
 * Get runtime type identification object representing QBParentalControl.
 *
 * @return object representing runtime type of QBParentalControl
 **/
SvType QBParentalControl_getType(void);

/**
 * Check if given rating is allowed (ie not blocked) by current PC rating settings
 *
 * @param[in] pc        parental control service handle
 * @param[in] rating    rating string (in standard serialized to string form)
 * @return              true if given ratings are allowed (not blocked)
 **/
bool QBParentalControlRatingAllowed(QBParentalControl pc, SvString rating);

/**
 * Check if EPG event should be blocked by parental control.
 *
 * @param[in] self      parental control service handle
 * @param[in] event     EPG event
 * @return              true if EPG event should be blocked by parental control
 **/
bool QBParentalControlEventShouldBeBlocked(QBParentalControl self, SvEPGEvent event);

/**
 * Check if EPG event description should be blocked by parental control.
 *
 * @param[in] self      parental control service handle
 * @param[in] event     EPG event
 * @return              true if EPG event description should be blocked by parental control
 **/
bool QBParentalControlEventDescShouldBeBlocked(QBParentalControl self, SvEPGEvent event);

SvEPGEventDesc QBParentalControlGetBlockedDescription(QBParentalControl self);

SvString
QBParentalControlGetBlockedMovieTitle(QBParentalControl self);

SvString
QBParentalControlGetBlockedMovieDescription(QBParentalControl self);

void QBParentalControlAddRating(QBParentalControl self, QBPCRating rating);
void QBParentalControlAddRatingAsString(QBParentalControl self, SvString ratingString);
void QBParentalControlAddRatings(QBParentalControl self, SvArray ratings);

void QBParentalControlRemoveRating(QBParentalControl self, QBPCRating rating);
void QBParentalControlRemoveRatingAsString(QBParentalControl self, SvString ratingString);

void QBParentalControlDisable(QBParentalControl self);
bool QBParentalControlIsDisabled(QBParentalControl self);

bool QBParentalControlIsStandardCleared(QBParentalControl self, QBPCRatingStandard standard);

void QBParentalControlAddListener(QBParentalControl self, SvObject listener);

void QBParentalControlRemoveListener(QBParentalControl self, SvObject listener);

bool QBParentalControlIsConfEnabled(QBParentalControl pc, SvString rating);

void QBParentalControlSetState(QBParentalControl pc, const SvString state);
QBParentalControlState QBParentalControlGetState(QBParentalControl pc);

/**
 * Set adult content blocking state and update GUI.
 *
 * @param[in] self pointer to QBParentalControl
 * @param[in] value string accordingly to conf parameters ("ON"/"OFF")
 **/
void QBParentalControlSetAdultContentBlockingState(QBParentalControl self, const char *value);

/**
 * Get current adultContentBlockingState.
 *
 * @param[in] self pointer to QBParentalControl
 * @return    current adultContentBlockingState
 **/
QBParentalControlAdultContentBlockingState QBParentalControlGetAdultContentBlockingState(QBParentalControl self);

/**
 * Get current unknownRatingBlockingState.
 *
 * @param[in] self pointer to QBParentalControl
 * @return    current unknownRatingBlockingState
 **/
QBParentalControlUnknownRatingBlockingState QBParentalControlGetUnknownRatingBlockingState(QBParentalControl self);

int QBParentalControlGetAutoRelockTime(QBParentalControl pc);
void QBParentalControlSetAutoRelockTime(QBParentalControl pc, int time);

QBParentalControlRatingStatus
QBParentalControlGetRatingStatus(QBParentalControl self, SvString rating);

/**
 * Check if QBParentalControl has an active locked time.
 * If locked time is active all content is forbidden until time ends.
 *
 * @param[in] self QBParentalControl pointer
 * @return @c true if blocked, @c false otherwise
 */
bool
QBParentalControlIsLockedTimeActive(QBParentalControl self);

QBParentalControlChannelLockingStatus
QBParentalControlGetChannelLockingStatus(QBParentalControl pc, SvGenericObject channel_);

bool
QBParentalControlChannelIsBlocked(QBParentalControl pc, SvTVChannel channel);

QBPCList
QBParentalControlGetRating(QBParentalControl self);

void QBParentalControlEnableRatingStandard(QBParentalControl self, QBPCRating rating);
void QBParentalControlPrintAgeLimits(QBParentalControl self, SvString rating, SvStringBuffer buf);

/**
 * Check if given channel with rating for its current event should be blocked by current PC policy
 *
 * @param[in] self              parental control service handle
 * @param[in] channel           corresponding tv channel structure handle
 * @param[in] eventRatingStr    rating string for event (in standard serialized to string form)
 * @param[in] isRatingUnknown   flag is true when we suppose we may not have complete information about the rating yet (especially when is missing)
 * @return                      true if current PC policy blocks the channel and/or the event
 **/
bool
QBParentalControlChannelWithEventRatingShouldBeBlocked(QBParentalControl self, SvTVChannel channel, SvString eventRatingStr, bool isRatingUnknown);

/**
 * Notify parental control service that the authentication state has changed.
 *
 * Warning: This method will not be exported.
 *
 * @param[in] self_             parental control service handle
 * @param[in] manager           QBAccessManager handle
 * @param[in] domainName        authentication domain
 **/
void QBParentalControlAuthenticationChanged(SvGenericObject self_,
                                            QBAccessManager manager,
                                            SvString domainName);

/**
 * @}
 **/

#endif
