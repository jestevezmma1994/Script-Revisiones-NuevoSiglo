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

#ifndef QBPARENTALCONTROLHELPER_H
#define QBPARENTALCONTROLHELPER_H

/**
 * @file QBParentalControlHelper.h
 * @brief Service monitoring of the parental control access during TV/VOD playback.
 **/

#include <SvFoundation/SvObject.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <Services/QBAccessController/QBAccessControllerTypes.h>
#include <SvEPGDataLayer/Views/SvEPGView.h>
#include <QBPCRatings/QBPCList.h>
#include <QBPVRRecording.h>
#include <time.h>
#include <stdbool.h>

#include <main_decl.h>

// QBParentalControlHelper
typedef struct QBParentalControlHelper_t *QBParentalControlHelper;
typedef void (*QBParentalControlAuthenticationCallback)(void *ptr);
typedef void (*QBParentalControlHelperEventSource)(void *ptr, SvTVChannel channel, time_t now, SvEPGEvent* current, SvEPGEvent *next);

/**
 * Type of source used to deliver content.
 **/
typedef enum QBParentalControlHelperContentSourceType_ {
    /// Other content sources
    QBParentalControlHelperContentSourceType_Other,
    /// USB content source
    QBParentalControlHelperContentSourceType_USB,
} QBParentalControlHelperContentSourceType;

QBParentalControlHelper QBParentalControlHelperNew(AppGlobals appGlobals);
void
QBParentalControlHelperActivate(QBParentalControlHelper self);
void
QBParentalControlHelperReset(QBParentalControlHelper self);
void
QBParentalControlHelperSetEventSource(QBParentalControlHelper self, void *eventSourceData, QBParentalControlHelperEventSource eventSource);
void
QBParentalControlHelperSetCallback(QBParentalControlHelper self, QBParentalControlAuthenticationCallback callback, void *callbackData);
void
QBParentalControlHelperAuthenticated(QBParentalControlHelper self);
/**
 * Check if can unblock program
 *
 * @param[in] self QBParentalControlHelper handler
 * @return @c true if can unblock, @c false if program must be blocked
 */
bool
QBParentalControlHelperIsReady(QBParentalControlHelper self);
bool
QBParentalControlHelperAuthNeeded(QBParentalControlHelper self);
void
QBParentalControlHelperCheck(QBParentalControlHelper self);
void
QBParentalControlHelperDeactivate(QBParentalControlHelper self);
void
QBParentalControlHelperSetChannel(QBParentalControlHelper self, SvTVChannel channel);
bool
QBParentalControlHelperHasChannel(QBParentalControlHelper self);

/**
 * Set recording to parental control helper
 *
 * @param[in] self      QBParentalControlHelper handler
 * @param[in] rec       recording handle
 */
void
QBParentalControlHelperSetRecording(QBParentalControlHelper self, QBPVRRecording rec);
bool
QBParentalControlHelperHasRecording(QBParentalControlHelper self);

/**
 * Set vod adult flag. True when vod product is for adult.
 *
 * @param[in] self  QBParentalControlHelper handler
 * @param[in] flag  New value of vodAdult flag.
 */
void
QBParentalControlHelperSetVodAdult(QBParentalControlHelper self, bool flag);

void
QBParentalControlHelperSetVodRating(QBParentalControlHelper self, SvString rating);
void
QBParentalControlHelperSetRatingFromEvent(QBParentalControlHelper self, SvEPGEvent event);
bool
QBParentalControlHelperHasUserAuthentication(QBParentalControlHelper self);
QBPCList
QBParentalControlHelperGetCurrentRating(QBParentalControlHelper self);
bool
QBParentalControlHelperUpdateRating(QBParentalControlHelper self);
void
QBParentalControlHelperForcedNotify(QBParentalControlHelper self);

QBAuthStatus
QBParentalControlHelperGetAuthenticationStatus(QBParentalControlHelper self);

/**
 * Function that sets content source type.
 * Content source type is QBParentalControlHelperContentSourceType_Other when not set.
 *
 * @param[in] self   QBParentalControlHelper handler
 * @param[in] type   content source type
 **/
void
QBParentalControlHelperSetContentSourceType(QBParentalControlHelper self, QBParentalControlHelperContentSourceType type);

/**
 * Function that checks if content source type should be locked for playing when ratings is unknown.
 *
 * @param[in] self  QBParentalControlHelper handler
 * @return          @c true if type of this Helper's content source is locked
 **/
bool
QBParentalControlHelperIsContentSourceTypeLocked(QBParentalControlHelper self);

#endif // QBPARENTALCONTROLHELPER_H
