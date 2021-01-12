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


#ifndef QB_PVR_PLAYER_H_
#define QB_PVR_PLAYER_H_

/**
 * @file pvrplayer.h
 * @brief PVR Player API
 **/

#include <QBPVRRecording.h>
#include <QBApplicationController.h>
#include <main_decl.h>
#include <Menus/QBLangMenu.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <NPvr/QBnPVRTypes.h>
#include <Logic/AnyPlayerLogic.h>
#include <Logic/QBParentalControlLogic.h>

/**
 * @defgroup QBPVRPlayerContext PVR player context class
 * @ingroup CubiTV_windows
 * @{
 **/

/**
 * type of playback associated with the pvr player.
 * Default value is a recording, and other possibilities are
 * chatchup and start over.
 **/
typedef enum {
    QBPVRPlayerContextPlaybackType_default,
    QBPVRPlayerContextPlaybackType_startOver,
    QBPVRPlayerContextPlaybackType_catchup
} QBPVRPlayerContextPlaybackType;


/**
 * Get runtime type identification object representing QBPVRPlayerContext class.
 *
 * @return QBPVRPlayerContext class
 **/
SvType QBPVRPlayerContext_getType(void);

QBWindowContext QBPVRPlayerContextCreate(AppGlobals appGlobals, QBAnyPlayerLogic anyPlayerLogic, time_t loadTime);
void QBPVRPlayerContextStopPlaying(QBWindowContext ctx_);

/**
 * Set PVR recording for player
 *
 * @param[in] ctx_       Player Context handle
 * @param[in] recording  recording handle
 **/
void QBPVRPlayerContextSetRecording(QBWindowContext ctx_, QBPVRRecording recording);
void QBPVRPlayerContextSetContent(QBWindowContext self_, SvContent content);
void QBPVRPlayerContextSetMusicPlaylist(QBWindowContext self_, SvString playlistURI);
void QBPVRPlayerContextSetMusic(QBWindowContext self_, SvContent content);

void QBPVRPlayerContextFinishPlayback(QBWindowContext ctx_);
void QBPVRPlayerContextStartPlayback(QBWindowContext ctx_, double position);

void QBPVRPlayerContextSetTitle(QBWindowContext self_, SvString title);
void QBPVRPlayerContextSetCoverURI(QBWindowContext self_, SvString coverURI);

void QBPVRPlayerContextShowContentSideMenu(QBWindowContext self_);
void QBPVRPlayerContextHideContentSideMenu(QBWindowContext self_);
bool QBPVRPlayerContextIsContentSideMenuShown(QBWindowContext self_);
void QBPVRPlayerContextSetContentData(QBWindowContext self_, SvObject contentData);

void QBPVRPlayerOSDShow(QBWindowContext ctx_, bool immediate, bool force);
void QBPVRPlayerOSDLock(QBWindowContext ctx_, bool lock, bool force);
void QBPVRPlayerOSDHide(QBWindowContext ctx_, bool immediate, bool force);
bool QBPVRPlayerOSDIsVisible(QBWindowContext ctx_);

void QBPVRPlayerPlay(QBWindowContext ctx_, double position);
void QBPVRPlayerPause(QBWindowContext ctx_);
void QBPVRPlayerREW(QBWindowContext ctx_);
void QBPVRPlayerFFW(QBWindowContext ctx_);
double QBPVRPlayerContextGetLastPositon(QBWindowContext ctx_);
double QBPVRPlayerGetSpeed(QBWindowContext ctx);
int QBPVRPlayerGetPlayerState(QBWindowContext ctx, SvPlayerTaskState *state);
void QBPVRPlayerNextContent(QBWindowContext ctx_);
void QBPVRPlayerPrevContent(QBWindowContext ctx_);

bool QBPVRPlayerExtendedInfoShow(QBWindowContext ctx_);
void QBPVRPlayerExtendedInfoHide(QBWindowContext ctx_);
bool QBPVRPlayerExtendedInfoIsVisible(QBWindowContext ctx_);

void QBPVRPlayerLangMenuShow(QBWindowContext ctx_, QBLangMenuSubMenu submenu);
void QBPVRPlayerLangMenuHide(QBWindowContext ctx_);
bool QBPVRPlayerLangMenuIsVisible(QBWindowContext ctx_);

/**
 * Check if the teletext is avaliable for a given Player Context.
 *
 * @param[in] ctx_ Player Context handle
 * @return @c true if teletext is available, otherwise @c false.
 **/
bool QBPVRPlayerTeletextIsAvailable(QBWindowContext ctx_);
void QBPVRPlayerTeletextShow(QBWindowContext ctx_);
void QBPVRPlayerTeletextHide(QBWindowContext ctx_);
bool QBPVRPlayerTeletextIsVisible(QBWindowContext ctx_);
void QBPVRPlayerTeletextSetTransparent(QBWindowContext ctx_, bool transparent);
bool QBPVRPlayerTeletextIsTransparent(QBWindowContext ctx_);

void QBPVRPlayerContextResetContentPosition(QBWindowContext ctx_);

const char *QBPVRPlayerContextGetTitle(QBWindowContext ctx_);

void QBPVRPlayerContextUpdateFormat(QBWindowContext ctx_);
void QBPVRPlayerContextUseBookmarks(QBWindowContext ctx_, bool status);
void QBPVRPlayerContextSetVodMetaData(QBWindowContext ctx_, SvDBRawObject meta);
void QBPVRPlayerContextSetMetaDataFromNPvrRecording(QBWindowContext ctx_, QBnPVRRecording obj);
/**
 * Set the metadata for PVR player from epg event and indicate what type of playback it is.
 *
 * @param[in] ctx_          Player context handle
 * @param[in] event         Epg event to get the metadata from
 * @param[in] playbackType  type of playbach associated with epg event (catchup or start over)
 **/
void QBPVRPlayerContextSetMetaDataFromEvent(QBWindowContext ctx_, SvEPGEvent event, QBPVRPlayerContextPlaybackType playbackType);
void QBPVRPlayerContextSetStartPosition(QBWindowContext ctx_, double pos);

SvObject QBPVRPlayerContextGetAdBitrate(QBWindowContext ctx_);
SvObject QBPVRPlayerContextGetRecording(QBWindowContext ctx_);
void QBPVRPlayerContextSetAuthenticated(QBWindowContext ctx_);

/**
 * Returns logic associated with this player context
 *
 * @param[in] ctx_       Player Context handle
 * @return logic associated with this player context
 **/
QBAnyPlayerLogic QBPVRPlayerContextGetAnyPlayerLogic(QBWindowContext ctx_);

/**
 * Set access domain. This domain will be used during authentication.
 *
 * @param[in]   ctx_            Player Context handle
 * @param[in]   accessDomain    Domain for access manager.
 **/
void QBPVRPlayerContextSetAccessDomain(QBWindowContext ctx_, SvString accessDomain);

/**
 * Set content source type for PVR player context
 *
 * @param[in] ctx_   Player Context handle
 * @param[in] type   content source type
 **/
void QBPVRPlayerSetContentSourceType(QBWindowContext ctx_, QBParentalControlHelperContentSourceType type);


/** This function creates JSON-like formated string which contains information about played content.
 *  Example of returned string:
 *          "url":"http://bs.vod.columbuscdn.com/vod/previews/TVNX0038601303011380/stb.ism/Manifest","productId":"3528","title":"Cars (HD)","errorCode":"10"
 *          "url":"http://bs.npvr.columbuscdn.com/npvr2/5/content/hgtvhd/17119-27565604-201603011300/stb.ism/Manifest?vbegin=0&vend=2400&devplybck=1","channel":"HGTV HD","channelNumber":"118","epgID":"1648696","epgTitle":"Flip or Flop Follow-Up","epgTimeRange":"8:00-8:30","errorCode":"10"
 *
 * @param[in] ctx_          Player Context handle
 * @param[in] errorInfo     handler to information about last error
 *
 * @return JSON-like formated string or @c NULL in case of error
 **/
SvString QBPVRPlayerContextCreateEventData(QBWindowContext ctx_, const SvErrorInfo errorInfo);

bool QBPVRPlayerCanPlay(QBWindowContext ctx_); // AMERELES BOLD TRACKING

/**
 * @}
 **/

#endif
