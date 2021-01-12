/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_STRIPE_H_
#define SV_EPG_STRIPE_H_

#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPG/SvEPGTextRenderer.h>
#include <SvEPG/SvEPGOverlay.h>
#include <SvEPG/SvEPGGrid.h>

/**
 * @file SvEPGStripe.h EPG Grid Stripe Widget API.
 **/

/**
 * @defgroup SvEPGStripe EPG grid stripe widget
 * @ingroup SvEPG
 * @{
 **/


/**
 * EPG stripe widget settings class.
 * @class SvEPGStripeSettings
 * @extends SvObject
 **/
typedef struct SvEPGStripeSettings_ *SvEPGStripeSettings;

/**
 * SvEPGStripeSettings class internals.
 * @private
 **/
struct SvEPGStripeSettings_ {
   struct SvObject_ super_; ///< super class

   SvObject displayer;
   char *widgetName;

   int height;          ///< stripe height in pixels
   int minWidth;        ///< stripe minimal width
   int spacing;         ///< event horizontal spacing
   int textPadding;     ///< event title horizontal padding
   double duration;     ///< duration of the activation/deactivation effect
   bool emptyMargins;
   bool alwaysVisibleIcons;     ///< rec and reminder icons in the visible area
   // text attributes
   SvFont font;         ///< font used for event titles
   int fontSize;
   SvColor textColor;
   SvColor focusTextColor;
   SvColor inactiveTextColor; ///< color for inactive channel
   // background
   /** bitmaps used as a background for inactive (unfocused) events */
   SvBitmap eventBg;
   /** bitmaps used as a background for active (focused) event */
   SvBitmap activeEventBg;
   SvColor bgColor;
   /*** background color for entire stripe when active */
   SvColor activeBgColor;

   int selectedRowHeightFactor;

   //detailed view
   struct {
      SvFont titleFont;
      int titleFontSize;
      int titleOffsetX, titleOffsetY;
      SvFont timeFont;
      int timeFontSize;
      int timeOffsetX, timeOffsetY;
      SvFont desctiptionFont;
      int desctiptionFontSize;
      int desctiptionOffsetX, desctiptionOffsetY;
      SvBitmap presentBg;
      SvColor presentColorBg;
      SvBitmap followingBg;
      SvColor followingColorBg;
   } detail;
};


/**
 * Load EPG stripe widget settings.
 *
 * This method loads settings of the EPG stripe widget from the
 * Settings Manager. It will read following parameters:
 *
 *  - height : integer, required,
 *  - spacing : integer, optional (default is 2),
 *  - textPadding : integer, optional (default is 0),
 *  - duration : double, optional (default is 0.0),
 *  - font : font, required,
 *  - fontSize : integer, required,
 *  - textColor : color, required,
 *  - inactiveTextColor : color, optional (default is textColor),
 *  - eventBg, activeEventBg, currentEventBg : bitmap, required,
 *  - bgColor, activeBgColor : color, optional (default: transparent).
 *
 * @param[in] widgetName   stripe widget name
 * @param[in] displayer    EPG displayer handle
 * @param[out] errorOut    error info
 * @return                 settings loaded from Settings Manager (released
 *                         by the caller), @c NULL in case of error
 **/
extern SvEPGStripeSettings
SvEPGStripeLoadSettings(const char *widgetName,
                        SvObject displayer,
                        SvErrorInfo *errorOut);

/**
 * Create new EPG stripe widget.
 *
 * This method creates grid header widget. It will display EPG events
 * for a specific channel, fetched from EPG data source.
 *
 * @param[in] app          CUIT application context
 * @param[in] settings     widget settings
 * @param[in] dataWindow   EPG data window handle
 * @param[in] renderer     EPG text renderer handle
 * @param[in] channel      TV channel to be shown
 * @param[in] channelIndex current index of the channel displayed in the stripe
 * @param[in] timeScale    number of pixels per one hour
 * @param[out] errorOut    error info
 * @return                 SvEPGStripe widget, NULL in case of error
 **/
extern SvWidget
SvEPGStripeNew(SvApplication app,
               SvEPGStripeSettings settings,
               SvEPGDataWindow dataWindow,
               SvEPGTextRenderer renderer,
               SvTVChannel channel,
               int channelIndex,
               unsigned int timeScale,
               bool detailed,
               SvErrorInfo *errorOut);

/**
 * Install or uninstall EPG overlay.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] overlay      EPG overlay: an object implementing
 *                         SvEPGOverlay interface (NULL to remove
 *                         previously installed one)
 **/
extern void
SvEPGStripeSetOverlay(SvWidget stripe,
                      SvObject overlay);

/**
 * Activate EPG stripe widget.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] active       @c true to set active
 * @param[in] timePoint    time point of event to be activated
 * @return                 a handle to the activated event,
 *                         @c NULL if event for @a timePoint can't be found
 **/
extern SvEPGEvent
SvEPGStripeSetActive(SvWidget stripe,
                     bool active,
                     time_t timePoint);

/**
 * Get active (focused) EPG event.
 *
 * @param[in] stripe       EPG stripe widget
 * @return                 a handle to the active event,
 *                         @c NULL if does not exist or in case of error
 **/
extern SvEPGEvent
SvEPGStripeGetActiveEvent(SvWidget stripe);

/**
 * Update current position.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] channelIndex current index of the channel displayed in the stripe
 * @param[in] now          current time
 * @return                 @c true if widgets has changed and needs redraw,
 *                         @c false otherwise
 **/
extern bool
SvEPGStripeSetCurrentTime(SvWidget stripe,
                          int channelIndex,
                          time_t now);

/**
 * Set time range.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] range        time range to be shown
 **/
extern void
SvEPGStripeSetTimeRange(SvWidget stripe,
                        SvTimeRange range);

/**
 * Set viewport dara.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] borderXPos   X position of the left viewport border
 * @param[in] width        viewport width
 **/
extern void
SvEPGStripeSetViewportData(SvWidget stripe,
                           int borderXPos,
                           int width);

/**
 * Set limit on displayed time range.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] start        start of the displayed range
 * @param[in] end          end if the displayed range
 **/
extern void
SvEPGStripeSetTimeLimit(SvWidget grid,
                        int start, int end);

/**
 * Update EPG event data.
 *
 * @param[in] stripe       EPG stripe widget
 * @param[in] range        time range of updated EPG data
 * @return                 @c true if active event has been set,
 *                         @c false otherwise
 **/
extern bool
SvEPGStripeUpdateEPGData(SvWidget stripe,
                         const SvTimeRange *range);

/**
 * Move focus (active event) to the previous event.
 *
 * @param[in] stripe       EPG stripe widget
 * @return                 a handle to the activated event,
 *                         @c NULL in case of error
 **/
extern SvEPGEvent
SvEPGStripeActivatePrev(SvWidget stripe);

/**
 * Move focus (active event) to the following event.
 *
 * @param[in] stripe       EPG stripe widget
 * @return                 a handle to the activated event,
 *                         @c NULL in case of error
 **/
extern SvEPGEvent
SvEPGStripeActivateNext(SvWidget stripe);

extern SvEPGEvent
SvEPGStripeSetTimePoint(SvWidget stripe, time_t timePoint);

extern SvEPGEvent
SvEPGStripeActivateLastFromTimeRange(SvWidget stripe, time_t start, time_t end);

extern SvEPGEvent
SvEPGStripeActivateFirstFromTimeRange(SvWidget stripe, time_t start, time_t end);

extern void
SvEPGStripeSetDescFunc(SvWidget stripe, SvEPGDescFunc func, void* target);

extern SvEPGEvent
SvEPGStripeGetLastFromTimeRange(SvWidget stripe, time_t start, time_t end);

/**
 * Update position of detailed widget.
 *
 * Active stripe hides events when detailed view is enabled and shows present/following widgets only.
 * Grid may shift view to adjust other events with present/following on active stripe.
 * Active stripe must be aware of that shit to display present/following widgets correctly.
 *
 * @param[in] stripe EPG stripe widget
 * @param[in] off_x  new offset of detail widget from the middle of the view.
 */
extern void
SvEPGStripeUpdateDetailedWidget(SvWidget stripe, int off_x);

/**
 * @}
 **/

#endif
