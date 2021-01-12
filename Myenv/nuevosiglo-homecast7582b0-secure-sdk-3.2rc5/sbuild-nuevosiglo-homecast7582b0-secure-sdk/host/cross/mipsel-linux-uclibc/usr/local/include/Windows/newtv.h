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

#ifndef NEWTV_H_
#define NEWTV_H_

/**
 * @file newtv.h
 * @brief CubiTV Television player
 **/

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <CUIT/Core/types.h>
#include <QBViewport.h>
#include <player_events/decryption.h>
#include <Menus/QBLangMenu.h>
#include <Windows/tv/miniChannelList.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <CAGE/Core/Sv2DRect.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBPVRRecording.h>
#include <QBWindowContext.h>
#include "main_decl.h"

/**
 * @defgroup TV CubiTV Television player
 * @ingroup CubiTV_windows
 *
 * This context implements the TV playback, manages the player and a variety of associated widgets.
 * It integrates closely with TVLogic in order to provide a interface that can be used by all
 * clients, but using TVLogic capabilities to redefine behaviour based on client requests.
 *
 * TV Context allows to manage which widgets will be shown, but does not by itself show or hide them.
 * Similarly it does not manage channel changes, it expects TVLogic to handle that.
 *
 * TV Context implements the QBChannelPlayer context in order to manage the player.
 *
 * TV Context will continue playback, even if it is not shown on screen, if some other
 * context wants to show TV playback, it may request it from this context.
 **/
/**
 *  Source priority for EPG events in new tv context.
 **/
typedef enum {
    SvTVContextCurrentEventSourcesPriority_PlayerThenManager, /**< get EPG from player in the first place, then from manager */
    SvTVContextCurrentEventSourcesPriority_ManagerThenPlayer, /**< get EPG from manager in the first place, then from player*/
    SvTVContextCurrentEventSourcesPriority_ManagerOnly /**< get EPG only from manager, this state avoids getting present-following events from player*/
} SvTVContextCurrentEventSourcesPriority;

typedef enum {
    QBTVSideMenuType_Full,
    QBTVSideMenuType_NPvrKeyword,
    QBTVSideMenuType_NPvrSeries
} QBTVSideMenuType;

/**
 * Get runtime type identification object representing SvTVContext class.
 *
 * @return TV Context class
 **/
SvType SvTVContext_getType(void);

/**
 * Updates everything, except of the player, with information about selected channel.
 * Player can be updated by using QBChannelPlayer::play function.
 *
 * This separation is done in order to allow the user to see that he is trying to play
 * a channel, but due to some reason it is impossible to even try starting playback
 * (for example there is no free tuner). This function is called to show the intent,
 * when playback may be started QBChannelPlayer::play will be called.
 * Additionaly this behaviour may be used to show the user that he is playing one channel
 * but actualy play an another one.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] channel   channel to update upon
 * @param[in] listID    id of playlist from which the channel is
 **/
void SvTVContextSetChannel(QBWindowContext ctx_, SvTVChannel channel, SvString listID);

/**
 * Sets where the TV playback should be shown. May be called only by an another context
 * that is currently on screen.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] rect      if NULL it means that playback should be full screen
 *                      otherwise it describes where on screen the playback should be.
 **/
void SvTVContextSetOverlay(QBWindowContext ctx_, const Sv2DRect* rect);

/**
 * Hides the TV playback from screen. May be called only by an another context
 * that is currently on screen.
 *
 * @param[in] ctx       TV Context handle
 **/
void SvTVContextDisableOverlay(QBWindowContext ctx);

/**
 * Used to obtain channel previously set by SvTVContextSetChannel().
 *
 * @param[in] ctx       TV Context handle
 * @return current channel if present, otherwise NULL.
 **/
SvTVChannel SvTVContextGetCurrentChannel(QBWindowContext ctx);

/**
 * Used to obtain data format of currently played channel.
 *
 * @param[in] ctx       TV Context handle
 * @return data format of current channel, NULL if player is not started.
 **/
struct svdataformat* SvTVContextGetCurrentFormat(QBWindowContext ctx);

/**
 * Creates the TV Context.
 *
 * @param[in] appGlobals CubiTV application
 * @return newly created TV Context handle
 **/
QBWindowContext SvTVContextCreate(AppGlobals appGlobals);

/**
 * Requests that the player rewinds if possible. Uses QBTVLogicGetSpeedBoundary()
 * to determine at exactly what speed should it rewind.
 * Has any effect only on contents that support timeshift.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextREW(QBWindowContext ctx_);

/**
 * Requests that the player fast forwards if possible. Uses QBTVLogicGetSpeedBoundary()
 * to determine at exactly what speed should it fast forward.
 * Has any effect only on contents that support timeshift.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextFFW(QBWindowContext ctx_);

/**
 * Requests that the player position changes if possible.
 * Position is modified by value of delta (forward for positive delta
 * or backward for negative delta)
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] delta     Time step in seconds (positive or negative)
 **/
void SvTVContextChangePosition(QBWindowContext ctx_, double delta);

/**
 * Attaches subtitles to specified widget, usually it should be one
 * that is positioned and of size identical (or similar) to overlay
 * set by SvTVContextSetOverlay. Subtitles will be rescaled to fit.
 *
 * @param[in] ctx       TV Context handle
 * @param[in] window    widget to which subtitles will be attached to
 **/
void SvTVContextAttachSubtitle(QBWindowContext ctx, SvWidget window);

/**
 * Detaches subtitles widget.
 *
 * @param[in] ctx      TV Context Handle
 **/
void SvTVContextDetachSubtitle(QBWindowContext ctx);

/**
 * TV Context will attach subtitles by itself
 * (@see SvTVContextAttachSubtitle()). Works only if
 * TV Context is currently on screen.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextShowSubtitles(QBWindowContext ctx_);

/**
 * Requests that the player starts live playback, but
 * does not drop timeshift data.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextEnterLive(QBWindowContext ctx_);

/**
 * Requests that the player enables timeshift.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextEnableTimeshift(QBWindowContext ctx_);

/**
 * Requests that the player disables timeshift, all timeshift
 * data will be forgotten and player will enter live playback.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextDisableTimeshift(QBWindowContext ctx_);

/**
 * Used to obtain current playback speed, that can be modified by
 * SvTVContextREW()/SvTVContextFFW().
 *
 * @param[in] ctx_      TV Context handle
 * @return current playback speed
 **/
double SvTVContextGetSpeed(QBWindowContext ctx_);

/**
 * Checks if player does live playback.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff player is in live point
 **/
bool SvTVContextIsLive(QBWindowContext ctx_);

/**
 * Schedules channel playback restart.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
SvTVContextScheduleAutoChannelRetry(QBWindowContext ctx_);

/**
 * Set channel playback error code
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] errorCode  error code
 */
void
SvTVContextSetChannelErrorCode(QBWindowContext ctx_, int errorCode);

/**
 * Set present/following event sources priority.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] priority  event sources priority
 **/
void
SvTVContextSetCurrentEventSourcesPriority(QBWindowContext ctx_, SvTVContextCurrentEventSourcesPriority priority);

/**
 * Shows OSD, it will automatically hide after some time.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] immediate false if OSD should be shown with an effect
 * @param[in] force     true if the show should be forced
 **/
void
QBTVOSDShow(QBWindowContext ctx_, bool immediate, bool force);

/**
 * Depending on request it eiter locks top OSD, so it will not
 * disappear automatically, or unlocks, so that it will.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] lock      true if top OSD should be locked
 * @param[in] force     true if the lock should be forced
 **/
void
QBTVOSDLock(QBWindowContext ctx_, bool lock, bool force);

/**
 * Hides top OSD (undoes effects of QBTVOSDShow()).
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] immediate false if OSD should be hidden with an effect
 * @param[in] force     true if the hide should be forced
 **/
void
QBTVOSDHide(QBWindowContext ctx_, bool immediate, bool force);

/**
 * Checks if top OSD is shown on screen.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff top OSD is on screen.
 **/
bool
QBTVOSDIsVisible(QBWindowContext ctx_);

/**
 * Show notification.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] immediate false if should be shown with an effect
 */
void QBTVOSDNotificationShow(QBWindowContext ctx_, bool immediate);

/**
 * Hide notification.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] immediate false if should be hidden with an effect
 */
void QBTVOSDNotificationHide(QBWindowContext ctx_, bool immediate);

/**
 * Hides the mini favorite list
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniFavoriteListHide(QBWindowContext ctx_);

/**
 * Shows the list of all playlists that a user can choose.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniFavoriteListShow(QBWindowContext ctx_);

/**
 * Checks if mini favorite list is shown on screen.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff mini favorite list is on screen
 **/
bool
QBTVMiniFavoriteListIsVisible(QBWindowContext ctx_);

/**
 * Shows the list of all channels from current playlist.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniChannelListShow(QBWindowContext ctx_);

/**
 * Used to obtain handle to channel list.
 *
 * @param[in] ctx_      TV Context handle
 * @return mini channel list handle if present, otherwise NULL
 **/
QBMiniChannelList
QBTVMiniChannelListGet(QBWindowContext ctx_);

/**
 * Hides the channel list
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniChannelListHide(QBWindowContext ctx_);

/**
 * Checks if channel list is shown on screen.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff channel list is on screen
 **/
bool
QBTVMiniChannelListIsVisible(QBWindowContext ctx_);

/**
 * Checks if recordings list is shown on screen.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff channel list is on screen
 **/
bool
QBTVMiniRecordingListIsVisible(QBWindowContext ctx_);

/**
 * Hides the recording list.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniRecordingListHide(QBWindowContext ctx_);

/**
 * Shows the list of all ongoing recordings.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniRecordingListShow(QBWindowContext ctx_);

/**
 * Shows a menu that allows user to select audio and subtitle tracks.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] submenu   tells which (if any) submenu should be opened automatically
 **/
void
QBTVLangMenuShow(QBWindowContext ctx_, QBLangMenuSubMenu submenu);

/**
 * Hides the lanugage menu.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVLangMenuHide(QBWindowContext ctx_);

/**
 * Checks if lanugage menu is shown on screen.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff channel list is on screen
 **/
bool
QBTVLangMenuIsVisible(QBWindowContext ctx_);

/**
 * Shows EPG listing for channel(s).
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniTVGuideShow(QBWindowContext ctx_);

/**
 * Hides EPG listing.
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVMiniTVGuideHide(QBWindowContext ctx_);

/**
 * Checks if EPG listing is visible.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff epg listing is visible
 **/
bool
QBTVMiniTVGuideIsVisible(QBWindowContext ctx_);

/**
 * Start displaying extended information.
 *
 * Get extended information about the content currently played from EPG and display it in a side pane.  If no extended
 * information is available, don't display anything.  Return true if the menu is displayed, false if not.
 *
 * @param[in]   ctx_    TV Context handle
 * @return      true if the extended info is displayed, false if not
 */
bool
QBTVExtendedInfoShow(QBWindowContext ctx_);

/**
 * Stop displaying extended information.
 * @param[in]   ctx_    TV Context handle
 **/
void
QBTVExtendedInfoHide(QBWindowContext ctx_);

/**
 * Checks if extended information is shown.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff extended information is shown
 **/
bool
QBTVExtendedInfoIsVisible(QBWindowContext ctx_);

/**
 * Shows a menu that allows to manage the selected recording, which may be,
 * for example, played or stopped. Usually used in conjuction with
 * recording list (@see QBTVMiniRecordingListShow()).
 *
 * @param[in] ctx_              TV Context handle
 * @param[in] recording         selected recording handle
 **/
void
QBTVRecordingOptionsShow(QBWindowContext ctx_, QBPVRRecording recording);

/**
 * Hides recording options menu.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVRecordingOptionsHide(QBWindowContext ctx_);

/**
 * Checks if recording options menu is shown.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff extended information is shown
 **/
bool
QBTVRecordingOptionsIsVisible(QBWindowContext ctx_);

/**
 * Shows the teletext browsing widget.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVTeletextShow(QBWindowContext ctx_);

/**
 * Hides the teletext browsing widget.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVTeletextHide(QBWindowContext ctx_);

/**
 * Checks if recording options menu is shown.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff extended information is shown
 **/
bool
QBTVTeletextIsVisible(QBWindowContext ctx_);

/**
 * Controls the transparency of teletext, either its background
 * is completely transparent or not.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] transparent true iff background should be transparent
 **/
void
QBTVTeletextSetTransparent(QBWindowContext ctx_, bool transparent);

/**
 * Checks if teletext is transparent (@see QBTVTeletextIsTransparent()).
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff teletext is transparent
 **/
bool
QBTVTeletextIsTransparent(QBWindowContext ctx_);

/**
 * Checks if side menu is shown.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff side menu is shown
 **/
bool
QBTVIsSideMenuShown(QBWindowContext ctx_);

/**
 * Shows a side menu and selects an option. The sidemenu is a Reminder Editor Pane
 * configured by TVReminderEditorPane.json.
 *
 * @param[in] ctx_      TV Context handle
 * @param[in] id        option id to focus
 * @param[in] type      type of sidemenu
 **/
void
QBTVShowSideMenu(QBWindowContext ctx_, SvString id, QBTVSideMenuType type);

/**
 * Hides the side menu.
 *
 * @param[in] ctx_      TV Context handle
 **/
void
QBTVHideSideMenu(QBWindowContext ctx_);

/**
 * Shows a popup that tells the user that it is not possible to play
 * because all tuners are in use and none is left for TV playback.
 *
 * @param[in] ctx_      TV Context handle
 **/
void QBTVTunerInfoShow(QBWindowContext ctx_);

/**
 * Hides the popup shown by QBTVTunerInfoShow().
 *
 * @param[in] ctx_      TV Context handle
 **/
void QBTVTunerInfoHide(QBWindowContext ctx_);

/**
 * Checks is currently played channel is being recorded.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff current channel is being recorded
 **/
bool svTVContextIsCurrentlyRecording(QBWindowContext ctx_);

/**
 * Requests that a one time recording (OTR) is started now and ends in 2 hours, unless
 * current recording is longer than 2 hours, in such case whole event is recorded.
 * Even if there are multiple events during the 2h period only one recording is made that
 * consists of all events. If recording is started in the middle of a event the recording will
 * not contain the missed part of the event.
 *
 * @param[in] ctx_      TV Context handle
 **/
void SvTVContextStartRecording(QBWindowContext ctx_);

/**
 * Stops the current recording.
 *
 * @param[in] ctx_      TV Context handle
 **/
void svTVContextStopRecording(QBWindowContext ctx_);

/**
 * Check if the NPvr recording can be started.
 *
 * @param[in] ctx_      TV Context handle
 * @return true if NPvr recording can be started
 **/
bool SvTVContextCanNPvrRecordingBeStarted(QBWindowContext ctx_);

/**
 * Shows a background image. This is should be used then there is no video content
 * (for example in radio channels) in order to show the user anything on the screen.
 * Usually the image is just a logo.
 *
 * @param[in] ctx_      TV Context handle
 **/
void QBTVBackgroundShow(QBWindowContext ctx_);

/**
 * Hides the background image.
 *
 * @param[in] ctx_      TV Context handle
 **/
void QBTVBackgroundHide(QBWindowContext ctx_);

/**
 * Checks if background image is visible.
 *
 * @param[in] ctx_      TV Context handle
 * @return true iff background image is visible
 **/
bool QBTVBackgroundIsVisible(QBWindowContext ctx_);

/**
 * Shows a popup that tells the problem occurred.
 *
 * @param[in] ctx_       TV Context handle
 */
void QBTVErrorShow(QBWindowContext ctx_);

/**
 * Hides the error popup.
 *
 * @param[in] ctx_       TV Context handle
 **/
void QBTVErrorHide(QBWindowContext ctx_);

/**
 * Shows a popup that tells the user that he cannot view current channel because he is
 * not entitled.
 *
 * @param[in] ctx_       TV Context handle
 **/
void QBTVNotEntitledShow(QBWindowContext ctx_);

/**
 * Hides the not entitled popup.
 *
 * @param[in] ctx_       TV Context handle
 **/
void QBTVNotEntitledHide(QBWindowContext ctx_);

/**
 * When a fatal error occures (playback is no longer possible, the player is unable
 * to handle the error) a popup informing the user of it is shown.
 *
 * @param[in] ctx_       TV Context handle
 **/
void QBTVFatalErrorShow(QBWindowContext ctx_);

/**
 * Hides the fatal error popup.
 *
 * @param[in] ctx_       TV Context handle
 **/
void QBTVFatalErrorHide(QBWindowContext ctx_);

/**
 * Shows an error popup with user definded information on the top of the screen.
 * Only one extra popup is allowed at the same time.
 * Popup will be destroyed automaticaly with window.
 *
 * @param[in] ctx_       TV Context handle
 * @param[in] title      Title of the popup
 * @param[in] content    Optional content of the popup
 */
void QBTVExtraErrorShow(QBWindowContext ctx_, const char* title, const char* content);

/**
 * Checks if extra error popup is currently visible.
 *
 * @param[in] ctx_       TV Context handle
 * @return               @c true if popup is visble or @c false if not
 */
bool QBTVExtraErrorIsVisible(QBWindowContext ctx_);

/**
 * Hides extra error popup.
 *
 * @param[in] ctx_       TV Context handle
 */
void QBTVExtraErrorHide(QBWindowContext ctx_);

/**
 * Checks if red button overlay is available for current channel.
 *
 * @param[in] ctx_       TV Context handle
 * @return true if red button overlay is available
 */
bool QBTVRedButtonOverlayIsAvailable(QBWindowContext ctx_);

/**
 * Checks if red button overlay is shown.
 *
 * @param[in] ctx_       TV Context handle
 * @return true if red button overlay is shown
 */
bool QBTVRedButtonOverlayIsVisible(QBWindowContext ctx_);

/**
 * Shows red button overlay.
 *
 * @param[in] ctx_       TV Context handle
 */
void QBTVRedButtonOverlayShow(QBWindowContext ctx_);

/**
 * Hides red button overlay.
 *
 * @param[in] ctx_       TV Context handle
 */
void QBTVRedButtonOverlayHide(QBWindowContext ctx_);

/**
 * Gets previous channel.
 *
 * @param[in] ctx_       TV Context handle
 * @return               previous channel.
 **/
SvTVChannel SvTVContextGetPreviousChannel(QBWindowContext ctx_);

/**
 * Gets playlist ID of the previous channel.
 *
 * @param[in] ctx_       TV Context handle
 * @return               list of previous channels.
 **/
SvString SvTVContextGetPreviousList(QBWindowContext ctx_);

// BEGIN RAL TimeShift cancel pop-up window
/**
 *
 * @}
 **/
void QBTVTimeShiftCancelPopUpShow(SvGenericObject listenerObj, QBWindowContext ctx_, unsigned int event, int channelNumber);

/**
 *
 * @}
 **/
bool QBTVTimeShiftCancelPopUpShowApplies(QBWindowContext ctx_);
// END RAL TimeShift cancel pop-up window

/**
 * Va al canal previo
 *
 * @param[in] ctx_       TV Context handle
 * @}
 **/
void QBTVBoldPlayPreviousChannel(QBWindowContext ctx_); // AMERELES #2206 Canal lineal que lleve a VOD

/**
 * @}
 **/

#endif
