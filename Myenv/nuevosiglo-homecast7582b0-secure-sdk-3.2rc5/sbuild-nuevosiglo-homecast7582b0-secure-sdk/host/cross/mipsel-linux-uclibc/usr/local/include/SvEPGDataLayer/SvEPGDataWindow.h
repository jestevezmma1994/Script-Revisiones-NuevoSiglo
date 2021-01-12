/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* App/Libraries/SvEPGDataLayer/SvEPGDataWindow.h */

#ifndef SV_EPG_WINDOW_H_
#define SV_EPG_WINDOW_H_

/**
 * @file SvEPGDataWindow.h
 * @brief EPG Data Window Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGDataWindow EPG data window class
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <time.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <SvEPGDataLayer/SvEPGIterator.h>


/**
 * EPG data window class.
 **/
typedef struct SvEPGDataWindow_ *SvEPGDataWindow;


/**
 * Get runtime type identification object
 * representing SvEPGDataWindow class.
 **/
extern SvType
SvEPGDataWindow_getType(void);

/**
 * Register EPG channel list listener.
 *
 * @param[in] self      EPG data window handle
 * @param[in] listener  handle to an object implementing
 *                      SvEPGChannelListListener and
 *                      SvEPGDataListener interfaces
 * @param[out] errorOut error info
 **/
extern void
SvEPGDataWindowAddListener(SvEPGDataWindow self,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut);

/**
 * Remove registered EPG channel list listener.
 *
 * @param[in] self      EPG data window handle
 * @param[in] listener  handle to the previously registered listener
 * @param[out] errorOut error info
 **/
extern void
SvEPGDataWindowRemoveListener(SvEPGDataWindow self,
                              SvGenericObject listener,
                              SvErrorInfo *errorOut);

/**
 * Setup EPG channel list view.
 *
 * @param[in] self      EPG data window handle
 * @param[in] view      handle to an object implementing SvEPGChannelView
 *                      interface, NULL to revert to default view
 * @param[out] errorOut error info
 **/
extern void
SvEPGDataWindowSetChannelView(SvEPGDataWindow self,
                              SvGenericObject view,
                              SvErrorInfo *errorOut);

/**
 * Get active EPG channel list view.
 *
 * @param[in] self      EPG data window handle
 * @return              current channel view (handle to an object
 *                      implementing SvEPGChannelView interface)
 **/
extern SvGenericObject
SvEPGDataWindowGetChannelView(SvEPGDataWindow self);

/**
 * Set looped mode for setting channels range.
 *
 * After setting loopedMode to true, SvEPGDataWindowSetChannelsRange()
 * will change behavior.
 * If loopedMode is set to false and firstChannel index + channelsCount is out
 * of channels range, last channels will be ignored.
 * If loopedMode is set to true, channels in the front of channel list will be set.
 * By default looped mode is disabled.
 *
 * @param[in] self          EPG data window handle
 * @param[in] loopedMode    should be looped mode enabled
 **/
extern void
SvEPGDataWindowSetLoopedMode(SvEPGDataWindow self,
                             bool loopedMode);

/**
 * Set width (time period) of the region-of-interest window.
 *
 * @param[in] self      EPG data window handle
 * @param[in] timeRange desired time range of EPG data window
 * @param[out] errorOut error info
 **/
extern void
SvEPGDataWindowSetTimeRange(SvEPGDataWindow self,
                            SvTimeRange timeRange,
                            SvErrorInfo *errorOut);

/**
 * Update EPG data that are in the given timerange.
 * Do not narrow ROI.
 *
 * @param[in] self      EPG data window handle
 * @param[in] timeRange desired time range of EPG data window
 * @param[out] errorOut error info
 **/
extern void
SvEPGDataWindowUpdateTimeRange(SvEPGDataWindow self,
                               SvTimeRange timeRange,
                               SvErrorInfo *errorOut);

/**
 * Set height (channels range) of the region-of-interest window.
 *
 * @param[in] self      EPG data window handle
 * @param[in] firstChannel index of the first channel according to current view
 * @param[in] channelsCount number of channels in the region-of-interest
 * @param[out] errorOut error info
 **/
extern void
SvEPGDataWindowSetChannelsRange(SvEPGDataWindow self,
                                unsigned int firstChannel,
                                unsigned int channelsCount,
                                SvErrorInfo *errorOut);

/**
  * Allows to control whether the data window will request refreshes
  * of EPG or not. By default it will.
  * @param[in] self     EPG data window handle
  * @param[in] passive  iff true data window doesn't request epg refresh
  **/
void
SvEPGDataWindowSetPassive(SvEPGDataWindow self,
                          bool passive);

/**
 * Create EPG iterator for event data of a specified TV channel.
 *
 * @param[in] self      EPG data window handle
 * @param[in] channelID unique TV channel ID
 * @param[in] startTime initial position of the iterator
 * @return              new iterator (caller is responsible for
 *                      releasing it) or NULL in case of error
 **/
extern SvEPGIterator
SvEPGDataWindowCreateIterator(SvEPGDataWindow self,
                              SvValue channelID,
                              time_t startTime);
/**
 * Invalidate EPG data for given EPG data window.
 *
 * @param[in] self          EPG data window handle
 * @param[out] errorOut     error info
 **/
extern void
SvEPGDataWindowInvalidateEvents(SvEPGDataWindow self,
                                SvErrorInfo *errorOut);

extern unsigned int
SvEPGDataWindowGetTimeRangeAlignment(SvEPGDataWindow self);
/**
 * @}
 **/

#endif
