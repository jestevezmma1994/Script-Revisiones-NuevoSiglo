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

#ifndef QBPARENTALCONTROLLOGIC_H
#define QBPARENTALCONTROLLOGIC_H

/**
 * @file QBParentalControlLogic.h
 * @brief Customizations of the parental control services.
 **/

/**
 * @defgroup QBParentalControlLogic Parental Control Logic
 * @ingroup CubiTV_logic
 * @{
 **/

#include <main_decl.h>
#include <Services/QBParentalControl/QBParentalControlHelper.h>
#include <QBPVRRecording.h>
#include <QBPCRatings/QBPCList.h>
#include <QBPCRatings/QBPCUtils.h>

typedef enum QBParentalControlLogicAuthState_ {
    QBParentalControlLogicAuthState_DisableAV,
    QBParentalControlLogicAuthState_PINPopup_DisableAV,
    QBParentalControlLogicAuthState_EnableAV
} QBParentalControlLogicAuthState;

/**
 * Content sources type.
 **/
typedef enum QBParentalControlLogicContentSource_ {
    /// Other content sources
    QBParentalControlLogicContentSource_Other,
    /// USB content source
    QBParentalControlLogicContentSource_USB,
} QBParentalControlLogicContentSource;

typedef struct QBParentalControlLogic_ *QBParentalControlLogic;

/**
 * Get runtime type identification object of QBParentalControlLogic.
 *
 * @return runtime type identification object
 **/
extern SvType
QBParentalControlLogic_getType(void) __attribute__((weak));

extern QBParentalControlLogic
QBParentalControlLogicCreate(AppGlobals appGlobals) __attribute__((weak));

extern void
QBParentalControlLogicStart(QBParentalControlLogic self) __attribute__((weak));

extern void
QBParentalControlLogicStop(QBParentalControlLogic self) __attribute__((weak));

/**
 * Add QBParentelControlHelper to QBParentalControlLogic.
 * QBParentalControlLogic uses helpers to determine if assets should be blocked by PC.
 *
 * @param[in] self      QBParentalControlLogic handler
 * @param[in] helper    QBParentelControlHelper handler
 **/
extern void
QBParentalControlLogicAddParentalControlHelper(QBParentalControlLogic self, QBParentalControlHelper helper) __attribute__((weak));

/**
 * Remove QBParentelControlHelper from QBParentalControlLogic.
 *
 * @param[in] self      QBParentalControlLogic handler
 * @param[in] helper    QBParentelControlHelper handler
 **/
extern void
QBParentalControlLogicRemoveParentalControlHelper(QBParentalControlLogic self, QBParentalControlHelper helper) __attribute__((weak));

extern void
QBParentalControlLogicStartHelperMonitoring(QBParentalControlLogic self, QBParentalControlHelper helper)  __attribute__((weak));

extern void
QBParentalControlLogicStopHelperMonitoring(QBParentalControlLogic self, QBParentalControlHelper helper)  __attribute__((weak));

extern void
QBParentalControlLogicUpdateState(QBParentalControlLogic self, QBParentalControlHelper helper)  __attribute__((weak));

extern QBParentalControlLogicAuthState
QBParentalControlLogicGetAuthState(QBParentalControlLogic self, QBParentalControlHelper helper) __attribute__((weak));

/**
 * Get time to unblock program without rating info
 * @param[in] self QBParentalControlLogic handler
 * @return time in [ms]
 **/
extern int
QBParentalControlLogicGetTimeMsToUnblockChannel(QBParentalControlLogic self) __attribute__((weak));

/**
 * Set time to unblock program without rating info
 *
 * @param[in] self      QBParentalControlLogic handler
 * @param[in] timeMs    blocking time in [ms] for a channels without rating info
 **/
extern void
QBParentalControlLogicSetTimeMsToUnblockChannel(QBParentalControlLogic self, int timeMs) __attribute__((weak));
/**
 * Function that decides if adult titles and content is blocked in the application
 *
 * @param[in] self  QBParentalControlLogic handler
 * @return          true if adult content is blocked, false if not.
 **/
extern bool
QBParentalControlLogicAdultIsBlocked(QBParentalControlLogic self) __attribute__((weak));

/**
 * Function that decides if content with unknown rating is blocked in the application
 *
 * @param[in] self  QBParentalControlLogic handler
 * @return          true if unknown rating content is blocked, false if not.
 **/
extern bool
QBParentalControlLogicUnknownRatingIsBlocked(QBParentalControlLogic self) __attribute__((weak));

/**
 * Function that gives the result of PC logic that tells if user is authenticated
 * Actually it is a wrapper of QBParentalControlHelperGetAuthenticationStatus.
 *
 * @param[in] self  QBParentalControlLogic handler
 * @return          true if AV should be currently enabled, false if not.
 **/
extern bool
QBParentalControlLogicIsAuthenticated(QBParentalControlLogic self) __attribute__((weak));

/**
 * Checks if parental authorization for specific channel is required.
 *
 * @param[in] self                  QBParentalControlLogic handler
 * @param[in] channel               channel for which parental control is checked
 * @param[in] isEventRatingUnknown  true when we suppose we may not have complete information about the event rating yet (especially when is missing)
 * @return                          true when QBParentalControlLogic demands parental authorization.
 **/
extern bool

QBParentalControlLogicIsAuthNeededForChannel(QBParentalControlLogic self, SvTVChannel channel, bool isEventRatingUnknown) __attribute__((weak));

/**
 * Checks if parental authorization for PVR record is required.
 *
 * @param[in] self          QBParentalControlLogic handler
 * @param[in] rec           recording for which parental control is checked
 * @return                  true when QBParentalControlLogic demands parental authorization.
 **/
extern bool
QBParentalControlLogicIsAuthNeededForRecord(QBParentalControlLogic self, QBPVRRecording rec) __attribute__((weak));

/**
 * Called when ratings configuration was changed in Parental Control menu. We get information about new ratings configuration
 * and set of enabled PC standards
 *
 * @param[in] self               QBParentalControlLogic handler
 * @param[in,out] newRatings     new ratings configuration to be used
 * @param[in] enabledStandards   enabled PC standards (can be empty)
 */
extern void
QBParentalControlLogicSelectedRatingsChanged(QBParentalControlLogic self, QBPCList newRatings, SvArray enabledStandards) __attribute__((weak));

/**
 * Wrapper for QBPCUtilsCreateDescriptionStringFromPCList.
 *
 * @param[in] self QBParentalControlLogic handle
 * @param[in] list QBPCList handle
 * @param[out] errorOut error handle
 * @return output string containing information about ratings
 **/
SvString QBParentalControlLogicCreateDescriptionStringFromPCList(QBParentalControlLogic self, QBPCList list, SvErrorInfo* errorOut) __attribute__((weak));

/**
 * Function that checks if content source type should be locked for playing when ratings is unknown.
 *
 * @param[in] self   QBParentalControlLogic handle
 * @param[in] helper QBParentalControlHelper handle
 * @return          @c true if type of given Helper's content source is locked
 **/
bool
QBParentalControlLogicIsContentSourceTypeLocked(QBParentalControlLogic self, QBParentalControlHelper helper) __attribute__((weak));

/**
 * @}
 **/

#endif // QBPARENTALCONTROLLOGIC_H
