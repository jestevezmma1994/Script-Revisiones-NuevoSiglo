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

#ifndef ANYPLAYER_LOGIC_H
#define ANYPLAYER_LOGIC_H

/**
 * @file AnyPlayerLogic.h
 **/

/**
 * @defgroup QBAnyPlayerLogic Any Player Logic
 * @ingroup CubiTV_logic
 * @{
 **/

#include <main_decl.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <QBApplicationController.h>
#include <ContextMenus/QBContentSideMenu.h>
#include <QBBookmarkService/QBBookmark.h>
#include <Utils/QBContentInfo.h>
#include <QBOSD/QBOSDMain.h>
#include <SvHTTPClient/SvSSLParams.h>

typedef struct QBAnyPlayerLogic_t *QBAnyPlayerLogic;

/**
 * Create player controller for given product info.
 *
 * \note Created controller is not set to the logic - for that use QBAnyPlayerLogicSetController().
 *
 * @param[in] self self object
 * @param[in] contentInfo information about content to handle
 * @return created player controller or null in case of error
 */
SvObject QBAnyPlayerLogicCreateController(QBAnyPlayerLogic self, QBContentInfo contentInfo) __attribute__((weak));

/**
 * Create and set playback state controller if necessary.
 *
 * @param[in] self self object
 */
void QBAnyPlayerLogicSetupPlaybackStateController(QBAnyPlayerLogic self) __attribute__((weak));

void QBAnyPlayerLogicToggleAll(QBAnyPlayerLogic self, bool enabled) __attribute__((weak));
void QBAnyPlayerLogicToggleWidgets(QBAnyPlayerLogic self, bool enabled) __attribute__((weak));
void QBAnyPlayerLogicToggleLoop(QBAnyPlayerLogic self, bool enabled) __attribute__((weak));
bool QBAnyPlayerLogicIsLoopEnabled(QBAnyPlayerLogic self) __attribute__((weak));
bool QBAnyPlayerLogicHasPlaylist(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicToggleSpeedControl(QBAnyPlayerLogic self, bool enabled) __attribute__((weak));
void QBAnyPlayerLogicTogglePause(QBAnyPlayerLogic self, bool enabled) __attribute__((weak));
void QBAnyPlayerLogicToggleChannelChange(QBAnyPlayerLogic self, bool enabled) __attribute__((weak));

bool QBAnyPlayerLogicHandleInputEvent(QBAnyPlayerLogic self, SvInputEvent ev) __attribute__((weak));
QBAnyPlayerLogic QBAnyPlayerLogicNew(AppGlobals appGlobals) __attribute__((weak));
void QBAnyPlayerLogicSetContext(QBAnyPlayerLogic self, QBWindowContext pvrPlayer) __attribute__((weak));

void QBAnyPlayerLogicStartPlayback(QBAnyPlayerLogic self, SvString source) __attribute__((weak));
void QBAnyPlayerLogicEndPlayback(QBAnyPlayerLogic self, SvString source) __attribute__((weak));
void QBAnyPlayerLogicHandlePlayerError(QBAnyPlayerLogic self, SvErrorInfo errorInfo) __attribute__((weak));

void QBAnyPlayerLogicGetSpeedBoundary(QBAnyPlayerLogic, float *backwardMin, float *backwardMax, float *forwardMin, float *forwardMax, bool *loop) __attribute__((weak));

void QBAnyPlayerLogicPlaybackFinished(QBAnyPlayerLogic self) __attribute__((weak));

void QBAnyPlayerLogicSetVodId(QBAnyPlayerLogic self, SvString id) __attribute__((weak));
SvString QBAnyPlayerLogicGetVodId(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicSetController(QBAnyPlayerLogic self, SvObject controller);
void QBAnyPlayerLogicSetPlaybackStateController(QBAnyPlayerLogic self, SvObject controller);

void QBAnyPlayerLogicLockDisplay(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicUnlockDisplay(QBAnyPlayerLogic self) __attribute__((weak));

void QBAnyPlayerLogicGotPlayerEvent(QBAnyPlayerLogic self, SvString name, void *arg) __attribute__((weak));
void QBAnyPlayerLogicPostWidgetsCreate(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicPreWidgetsDestroy(QBAnyPlayerLogic self) __attribute__((weak));

void QBAnyPlayerLogicSetLastPositionBookmark(QBAnyPlayerLogic self, QBBookmark bookmark) __attribute__((weak));
QBBookmark QBAnyPlayerLogicGetLastPositionBookmark(QBAnyPlayerLogic self) __attribute__((weak));

/**
 * Set start position.
 *
 * @param[in] self     handle of the QBAnyPlayerLogic object
 * @param[in] position start postion
 */
void QBAnyPlayerLogicSetStartPosition(QBAnyPlayerLogic self, double position);

/**
 * Get start position.
 *
 * If returned value is not -1 then it should take precedence over value stored in bookmark.
 *
 * @param[in] self handle of the QBAnyPlayerLogic object
 * @return         start position, -1 if not set
 */
double QBAnyPlayerLogicGetStartPosition(QBAnyPlayerLogic self);

/**
 * Send information about current player satate.
 *
 * @param[in] self handle of the QBAnyPlayerLogic object
 */
void QBAnyPlayerLogicReportState(QBAnyPlayerLogic self);

void QBAnyPlayerLogicSetProperty(QBAnyPlayerLogic self, SvValue propertyKey, SvValue propertyValue) __attribute__((weak));

void QBAnyPlayerLogicVolumeUp(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicVolumeDown(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicFFW(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicREW(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicPause(QBAnyPlayerLogic self) __attribute__((weak));
void QBAnyPlayerLogicPlay(QBAnyPlayerLogic self, double position) __attribute__((weak));
void QBAnyPlayerLogicShowOnFrontPanel(QBAnyPlayerLogic self) __attribute__((weak));

void QBAnyPlayerLogicContentSideMenuRegisterCallbacks(QBAnyPlayerLogic self, QBContentSideMenu contentSideMenu) __attribute__((weak));

/**
 * Create main OSD component.
 *
 * @param[in] self     handle of the QBAnyPlayerLogic object
 * @param[in] app      CUIT application handle
 * @param[in] userdata optional data to OSD create
 * @return             new main OSD component or @c NULL in case of error
 */
QBOSDMain QBAnyPlayerLogicOSDCreate(QBAnyPlayerLogic self, SvApplication app, void *userdata) __attribute__((weak));

void QBAnyPlayerLogicSetBookmarkId(QBAnyPlayerLogic self, SvString bookmarkId) __attribute__((weak));
SvString QBAnyPlayerLogicGetBookmarkId(QBAnyPlayerLogic self) __attribute__((weak));

/**
 * Set additional get parameter(s) that will be appended to PlayReady licence request url.
 *
 * @param[in] self          logic handle
 * @param[in] licenceParams get parameters
 */
void QBAnyPlayerLogicSetPlayReadyLicenceRequestParams(QBAnyPlayerLogic self, SvString licenceParams) __attribute__((weak));

/**
 * Replace custom data used during PlayReady licence acquisition.
 *
 * @param[in] self       logic handle
 * @param[in] customData new custom data
 */
void QBAnyPlayerLogicSetPlayReadyLicenceRequestCustomData(QBAnyPlayerLogic self, SvString customData) __attribute__((weak));

/**
 * Set SSL parameters used during PlayReady licence acquisition.
 *
 * @param[in] self      logic handle
 * @param[in] sslParams SSL parameters
 */
void QBAnyPlayerLogicSetPlayReadyLicenceRequestSSLParams(QBAnyPlayerLogic self, SvSSLParams sslParams) __attribute__((weak));

/**
 * @}
 **/

#endif
