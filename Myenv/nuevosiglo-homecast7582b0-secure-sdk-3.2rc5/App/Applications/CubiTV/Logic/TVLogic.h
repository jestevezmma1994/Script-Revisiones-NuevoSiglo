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

#ifndef TVLOGIC_H
#define TVLOGIC_H

/**
 * @file  TVLogic.h
 * @brief TV logic API
 **/

#include <CUIT/Core/types.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>
#include <Services/core/playlistCursor.h>
#include <QBWindowContext.h>
#include <stdbool.h>

#include <QBDvbSubsManager.h>
#include <QBTextSubsManager.h>
#include <QBTeletextSubtitlesManager.h>
#include <QBClosedCaptionSubsManager.h>
#include <QBTeletextLineReceiver.h>
#include <QBVBITeletext.h>
#include <QBTeletextReceiver.h>
#include <QBScte27Subs.h>
#include <QBOSD/QBOSDMain.h>

#include <SvPlayerManager/SvPlayerTask.h>
#include <QBPVRRecording.h>


/**
 * @defgroup TVLogic QBTV logic
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Container for Meta Data Managers
 */
struct QBTVLogicMetaDataManagers_ {
    QBTeletextLineReceiver teletextLineReceiver;
    QBVBITeletext teletextVBIDisplayer;
    QBTeletextReceiver teletextReceiver;
    QBTeletextSubtitlesManager teletextSubsManager;
    QBDvbSubsManager dvbSubsManager;
    QBTextSubsManager textSubsManager;
    QBScte27SubsManager scte27SubsManager;
    QBClosedCaptionSubsManager closedCaptionSubsManager;
};

typedef struct QBTVLogicMetaDataManagers_ *QBTVLogicMetaDataManagers;

typedef struct QBTVLogic_t *QBTVLogic;

/**
 * Get runtime type identification object representing QBTVLogic class.
 *
 * @return QBTVLogic runtime type identification object
 **/
SvType
QBTVLogic_getType(void);

bool QBTVLogicHandleInputEvent(QBTVLogic self, SvInputEvent ev) __attribute__((weak));

QBTVLogic QBTVLogicNew(AppGlobals appGlobals) __attribute__((weak));
void QBTVLogicStart(QBTVLogic self) __attribute__((weak));
void QBTVLogicStop(QBTVLogic self) __attribute__((weak));

void QBTVLogicPlayChannel(QBTVLogic self, SvTVChannel channel, SvString source) __attribute__((weak));

/**
 * Switches 'offset' number of channels up (for positive offset), or down (for negative offset).
 *
 * @param[in] self this object
 * @param[in] offset number of channels to skip - positive or negative number
 * @param[in] source name of the source that originated the channel change request
 **/
void QBTVLogicPlayChannelByOffset(QBTVLogic self, int offset, SvString source) __attribute__((weak));
void QBTVLogicPlayChannelFromPlaylist(QBTVLogic self, SvString playlistID, SvTVChannel channel, SvString source) __attribute__((weak));
void QBTVLogicPlayChannelByIndexFromPlaylist(QBTVLogic self, SvString playlistID, int index, SvString source) __attribute__((weak));
void QBTVLogicPlayChannelByCursor(QBTVLogic self, QBPlaylistCursor cursor, SvString source) __attribute__((weak));
void QBTVLogicPlayChannelByIndex(QBTVLogic self, int index, SvString source) __attribute__((weak));
void QBTVLogicPlayChannelByNumber(QBTVLogic self, int selection, SvString source) __attribute__((weak));
void QBTVLogicResumePlaying(QBTVLogic self, SvString source) __attribute__((weak));
void QBTVLogicStopPlaying(QBTVLogic self, SvString source) __attribute__((weak));
void QBTVLogicPlayPreviousChannel(QBTVLogic self) __attribute__((weak));

/**
 * Set current player task.
 * @param[in] self          TVLogic handle
 * @param[in] playerTask    player task to be set (pass NULL to reset)
 **/
void QBTVLogicSetPlayerTask(QBTVLogic self, SvPlayerTask playerTask) __attribute__((weak));

/**
 * Get current player task.
 * @param[in] self          TVLogic handle
 * @return  currently played player task
 **/
SvPlayerTask QBTVLogicGetPlayerTask(QBTVLogic self) __attribute__((weak));

void QBTVLogicRestartPlayback(QBTVLogic self) __attribute__((weak));
bool QBTVLogicHandleError(QBTVLogic self, SvErrorInfo errorInfo, int* channelErrorCode) __attribute__((weak));

void QBTVLogicGetSpeedBoundary(QBTVLogic self, float *backwardMin, float *backwardMax, float *forwardMin, float *forwardMax) __attribute__((weak));
int QBTVLogicGetTunerNum(QBTVLogic self) __attribute__((weak));

void QBTVLogicPostWidgetsCreate(QBTVLogic self) __attribute__((weak));
void QBTVLogicPreWidgetsDestroy(QBTVLogic self) __attribute__((weak));

void QBTVLogicEnableAudioDelaySetupMode(QBTVLogic self) __attribute__((weak));

SvObject QBTVLogicGetPlaylist(QBTVLogic self) __attribute__((weak));
SvTVChannel QBTVLogicGetChannel(QBTVLogic self) __attribute__((weak));

/**
 * Get channel by given number.
 *
 * @param[in] self      TVLogic handle
 * @param[in] number    channel number
 * @param[in] playlist  channel playlist
 * @return              selected channel, otherwise @c NULL
 **/
SvTVChannel QBTVLogicGetChannelByNumber(QBTVLogic self, int number, SvObject playlist) __attribute__((weak));

/**
 * Show options for ongoing recording.
 *
 * @param[in] self              tv logic handle
 * @param[in] rec               ongoing recording handle
 **/
void QBTVLogicShowRecordingOptions(QBTVLogic self, QBPVRRecording rec) __attribute__((weak));

typedef enum {
    DisplayLockMode_Full = 0,
    DisplayLockMode_EnableUpDown
} DisplayLockMode;

void QBTVLogicLockDisplay(QBTVLogic self, DisplayLockMode displayLockMode) __attribute__((weak));
void QBTVLogicUnlockDisplay(QBTVLogic self) __attribute__((weak));

bool QBTVLogicChannelCanPlay(QBTVLogic self, SvTVChannel channel) __attribute__((weak));
bool QBTVLogicChannelCanPlayByIndex(QBTVLogic self, int index) __attribute__((weak));

/**
 * Create main OSD component.
 *
 * @param[in] self      TVLogic handle
 * @param[in] app       CUIT application handle
 * @param[in] tvContext TV window context
 * @return              new main OSD component or @c NULL in case of error
 */
QBOSDMain QBTVLogicOSDCreate(QBTVLogic self, SvApplication app, QBWindowContext tvContext) __attribute__((weak));

/**
 * If it is necessary shows animated background.
 *
 * @param[in] self TVLogic handle
 * @param[in] ctx Window Context handle
 * @param[in] channel actually playing channel
 **/
void QBTVLogicCheckBackground(QBTVLogic self, QBWindowContext ctx, SvTVChannel channel) __attribute__((weak));

/**
 * Creates Subs Manager instance
 *
 * @param[in] self TVLogic instance
 * @return created Subs Manager
 */
QBSubsManager QBTVLogicCreateSubsManager(QBTVLogic self);

/**
 * Check if PVR timeshift is allowed for given channel.
 *
 * @param[in] self      TVLogic handle
 * @param[in] channel   actually playing channel
 * @return              @c true if PVR timeshift is allowed, otherwise @c false
 **/
bool QBTVLogicIsPVRTimeshiftAllowedForChannel(QBTVLogic self, SvTVChannel channel) __attribute__((weak));

/**
 * Creates Meta Data Managers and starts them
 *
 * @param[in] self TVLogic instance
 * @param[in] subsManager QBSubsManager instance
 * @param[in] format data format
 * @param[in] pid teletext pid
 * @param[in] hintedLang hinted language (can be NULL)
 */
void QBTVLogicStartMetaDataManagers(QBTVLogic self, QBSubsManager subsManager,
                                    struct svdataformat* format, int pid, const char *hintedLang);

/**
 * Stops Meta Data Managers
 *
 * @param[in] self TVLogic instance
 */
void QBTVLogicStopMetaDataManagers(QBTVLogic self);

/**
 * Returns struct containing all Meta Data Managers
 *
 * @param[in] self TVLogic instance
 * @return Meta Data managers
 */
QBTVLogicMetaDataManagers QBTVLogicGetMetaDataManagers(QBTVLogic self);


/**
 * Create context for TV
 *
 * @param[in] self TVLogic instance
 * @return TV window context
 */
QBWindowContext
QBTVLogicCreateTVContext(QBTVLogic self);

/**
 * Get Channel logo url from a channel
 * @param[in] channel SvTVChannel Canal del cual se desea conocer la url del logo
 * @return SvString url del logo
 **/
SvString QBTVLogicGetChannelLogoURL(SvTVChannel channel); //NR: Logos de Canales en servidor

/**
 * @}
 **/

#endif /* TVLOGIC_H */
