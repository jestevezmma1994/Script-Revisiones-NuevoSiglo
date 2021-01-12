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

#ifndef TELEOSD_H_
#define TELEOSD_H_

/**
 * @file teleosd.h CubiTV TV OSD widget API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>

#include <CUIT/Core/types.h>
#include <QBInput/QBInputEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <Services/core/playlistManager.h>
#include <Services/core/QBTextRenderer.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <Utils/viewport.h>
#include <QBViewport.h>
#include <QBSubsManager.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Logic/EventsLogic.h>
#include <Logic/TVLogic.h>

/**
 * @defgroup CubiTVTVOSD CubiTV TV OSD widget
 * @ingroup CubiTV_widgets
 * @{
 **/

/**
 * Display info icons on OSD based on current channel and event.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] channel           used to obtain current channel metadata
 * @param[in] event             used to obtain current event metadata
 * @param[in] subsManager       used to obtain current subtitles metadata
 **/
extern void svTVOsdSetInfo(SvWidget tvOSD, SvTVChannel channel,  SvEPGEvent event, QBSubsManager subsManager);

/**
 * Display/hide recording status icon.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] isRecording       If @c true recording icon becomes visible, otherwise becomes hidden.
 */
extern void svTVOsdSetRecordingStatus(SvWidget tvOSD, bool isRecording);

/**
 * Display channel metadata on tvosd: name, number and thumbnail.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] channel           Channel to be displayed.
 * @param[in] isFavorite        @c true if @a channel is favorited
 */
extern void svTVOsdSetTVChannel(SvWidget tvOSD, SvTVChannel channel, bool isFavorite);

/**
 * Set list name displayed on tvosd.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] listName          Use listName instead of playlist name from playlist manager
 */
extern void svTVOsdSetListName(SvWidget tvOSD, SvString listName);

/**
 * Mute/Unmute tvosd.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] isMuted           If @c true tvosd should be muted, otherwise unmuted.
 */
extern void svTVOsdMute(SvWidget tvOSD, bool isMuted);

/**
 * Set value of volume displayed by tvosd.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] volume            New volume value (accepted values between 0 and 255).
 */
extern void svTVOsdSetVolume(SvWidget tvOSD, unsigned int volume);

/**
 * Handle input events
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] ie                ie event to be handled
 * @return                      information if event has been consumed
 */
extern bool svTVOSDInputEventHandler(SvWidget tvOSD, QBInputEvent ie);

/**
 * Set a callback to be called after setting channel number in the OSD
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] callback          callback to be set
 * @param[in] callbackData      callback data
 */
extern void svTVOSDSetChannelFrameCallback(SvWidget tvOSD, QBTVChannelFrameCallback callback, void *callbackData);

/**
 * Set current channel name displayed by tvosd.
 *
 * @param[in] tvOSD             Widget that will be affected by function call.
 * @param[in] name              New channel name.
 */
extern void svTVOsdSetChannelName(SvWidget tvOSD, SvString name);

/**
 * Set info icons by event.
 *
 * @param[in] tvOSD            Widget that will be affected by function call.
 * @param[in] event            New event.
 */
extern void svTVOsdSetInfoIconsByEvent(SvWidget tvOSD, SvEPGEvent event);

/**
 * Set info icons by subsManager
 *
 * @param[in] tvOSD            Widget that will be affected by function call.
 * @param[in] subsManager      handler to new subsManager.
 */
extern void svTVOsdSetInfoIconsBySubsManager(SvWidget tvOSD, QBSubsManager subsManager);

/**
 * Set info icons by Format
 * @param[in] tvOSD            Widget that will be affected by function call.
 * @param[in] format           new format.
 */
extern void SvTVOsdSetInfoIconsByFormat(SvWidget tvOSD, struct svdataformat *format);


/**
 * Create new tvosd. Tvosd should be attached on top of osd after being created.
 * Tvosd displays channel number, name and thumbnail, current time and volume.
 * It is created with Settings Manager. Parameters for SM:
 * - width, height - integer, required.
 * Representations of child widgets in SM are created by appending tvosd widget name by dot
 * character (".") and name of the child widget.
 * Child widgets:
 * - name - label widget, params for SM the same as in @see svLabelNewFromSM, additionally:
 *    - xOffset, yOffset - integer, required.
 * - number - label widget, params for SM the same as in @see svLabelNewFromSM, additionally:
 * - clock - params for SM:
 *    - xOffset, yOffset, width, height - integer, required,
 *    - bg - bitmap, required,
 *    - font - font, required,
 *    - textColor - color, required,
 *    - fontSize - integer, required,
 *    - iconXOffset, iconYOffset - integer, required.
 * - volume - gauge widget, params for SM the same as in @see svGaugeNewFromSM, additionally:
 *    - xOffset, yOffset - integer, required,
 *    - muted - bitmap, required.
 * -icon - params for SM:
 *    - xOffset, yOffset, width, height - integer, required,
 *    - tmpBg - bitmap, required.
 *
 * @param[in] app          CUIT application handle
 * @param[in] widgetName   widget's name in settings file
 * @param[in] textRenderer text renderer
 * @param[in] playlists    playlist manager
 * @param[in] viewport     viewport
 * @param[in] eventsLogic  events logic
 * @param[in] tvLogic      tv logic
 * @param[in] pc           parental control
 * @return                 new tv osd widged or @c NULL in case of error
 */
extern SvWidget
svTVOSDNew(SvApplication app,
           const char *widgetName,
           QBTextRenderer textRenderer,
           QBPlaylistManager playlists,
           QBViewport viewport,
           EventsLogic eventsLogic,
           QBTVLogic tvLogic,
           QBParentalControl pc);

/**
 * @}
 **/

#endif
