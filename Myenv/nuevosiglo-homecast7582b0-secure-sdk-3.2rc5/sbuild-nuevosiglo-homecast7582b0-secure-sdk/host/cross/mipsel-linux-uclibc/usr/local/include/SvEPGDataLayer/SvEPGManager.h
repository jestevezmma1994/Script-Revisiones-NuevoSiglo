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

/* App/Libraries/SvEPGDataLayer/SvEPGManager.h */

#ifndef SV_EPG_MANAGER_H_
#define SV_EPG_MANAGER_H_

/**
 * @file SvEPGManager.h
 * @brief EPG Data Manager Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGManager EPG data manager class
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <time.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvEPGDataLayer/SvEPGDataWindow.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGEventsList.h>


/**
 * EPG manager class.
 * @class SvEPGManager SvEPGManager.h <SvEPGDataLayer/SvEPGManager.h>
 * @implements QBAsyncService
 * @extends SvObject
 **/
typedef struct SvEPGManager_ *SvEPGManager;

/**
 * EPG manager update parameter class.
 **/
typedef struct SvEPGManagerUpdateParams_ {
    struct SvObject_ super_;

    int64_t updateDelayMs;
    unsigned maxChannels;
} *SvEPGManagerUpdateParams;

/**
 * Create default instance of SvEPGManagerUpdateParams
 *
 * @param[in] self      EPG manager handle
 * @return              EPG update parameters handle, NULL in case of error
 */
extern SvEPGManagerUpdateParams
SvEPGManagerCreateUpdateParameters(SvEPGManager self);

/**
 * Get runtime type identification object
 * representing SvEPGManager class.
 **/
extern SvType
SvEPGManager_getType(void);

/**
 * Create EPG manager instance.
 *
 * @param[in] merger    channel merger
 * @param[out] errorOut error info
 * @return              created EPG manager, or @c NULL in case of error
 **/
extern SvEPGManager
SvEPGManagerCreate(SvObject merger,
                   SvErrorInfo *errorOut);

/**
 * Register new pair of EPG plugins in EPG manager.
 *
 * @param[in] self      EPG manager handle
 * @param[in] channelListPlugin handle to an object implementing @ref SvEPGChannelListPlugin
 * @param[in] dataPlugin handle to an object implementing @ref SvEPGDataPlugin
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerAddPlugin(SvEPGManager self,
                      SvGenericObject channelListPlugin,
                      SvGenericObject dataPlugin,
                      SvErrorInfo *errorOut);

/**
 * Unregister pair of EPG plugins.
 *
 * @param[in] self      EPG manager handle
 * @param[in] channelListPlugin previously registered channel list plugin or data plugin
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerRemovePlugin(SvEPGManager self,
                         SvGenericObject plugin,
                         SvErrorInfo *errorOut);

/**
 * Create new EPG events list.
 *
 * @param[in] self      EPG manager handle
 * @param[out] errorOut error info
 * @return              EPG events list handle, NULL in case of error
 **/
extern SvEPGEventsList
SvEPGManagerCreateEventsList(SvEPGManager self,
                             SvErrorInfo *errorOut);

/**
 * Get total number of EPG events stored by the manager.
 *
 * @param[in] self      EPG manager handle
 * @return              number of EPG events or @c -1 in case of error
 **/
extern ssize_t
SvEPGManagerGetEventsCount(SvEPGManager self);

/**
 * Get list of all known channels.
 *
 * @param[in] self      EPG manager handle
 * @return              channels list or @c NULL in case of error
 **/
extern SvEPGChannelList
SvEPGManagerGetChannelsList(SvEPGManager self);

/**
 * Set limit on number of EPG events that can be stored by the manager.
 *
 * @note EPG manager has no limit after initialization.
 *
 * @param[in] self      EPG manager handle
 * @param[in] maxCount  max number of EPG events that can be stored,
 *                      @c 0 to disable this limit
 * @param[out] errorOut error info
 * @return              previous setting or @c -1 in case of error
 **/
extern ssize_t
SvEPGManagerSetMaxEventsCount(SvEPGManager self,
                              size_t maxCount,
                              SvErrorInfo *errorOut);

/**
 * Set limit on number of EPG events per channel that can be stored by the manager.
 *
 * @note EPG manager has no limit after initialization.
 *
 * @param[in] self            EPG manager handle
 * @param[in] maxPerChannel   max number of EPG events per channel that can be stored
 * @param[in] start           start of the time range of EPG window for channel,
 *                            enabled when event-count per channel exceeds maximum
 * @param[in] end             end of the time range of EPG window for channel,
 *                            enabled when event-count per channel exceeds maximum
 *                            @c 0 to disable this limit
 * @param[out] errorOut       error info
 **/
extern void
SvEPGManagerSetMaxEventsPerChannel(SvEPGManager self,
                                   size_t maxPerChannel,
                                   int start,
                                   int end,
                                   SvErrorInfo *errorOut);

/**
 * Set range of EPG events that should be stored by the manager.
 *
 * @param[in] self      EPG manager handle
 * @param[in] start     start of the time range of allowed events,
 *                      relative to current time (in seconds)
 * @param[in] end       end of the time range of allowed events,
 *                      relative to current time (in seconds)
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerSetTimeLimit(SvEPGManager self,
                         int start, int end,
                         SvErrorInfo *errorOut);

/**
 * Get time range limit of EPG events that is used by manager.
 *
 * @param[in] self      EPG manager handle
 * @param[in] start     start of the time range of allowed events,
 *                      relative to current time (in seconds)
 * @param[in] end       end of the time range of allowed events,
 *                      relative to current time (in seconds)
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerGetTimeLimit(SvEPGManager self,
                         int *start, int *end,
                         SvErrorInfo *errorOut);

/**
 * Set channels update max count.
 *
 * @param[in] self              EPG manager handle
 * @param[in] maxChannelsCount  maximum channels count
 **/
extern void
SvEPGManagerSetChannelsUpdateMaxCount(SvEPGManager self,
                                      size_t maxChannelsCount);

/**
 * Tell the manager what to do when having too many events in the event cache.
 *
 * @param[in] self      EPG manager handle
 * @param[in] dynamicTimeLimit   true iff time-limits can be adjusted when event-count exceeds maximum
 */
extern void
SvEPGManagerSetOOMStrategy(SvEPGManager self,
                           bool dynamicTimeLimit,
                           bool droppingChunks);

/**
 * Set alignment of time-range-chunks in which the manager will try to keep it's data.
 *
 * @note default alignment is 3600 seconds
 *
 * @param[in] self      EPG manager handle
 * @param[in] timeRangeAlignment  in seconds
 **/
void
SvEPGManagerSetTimeRangeAlignment(SvEPGManager self,
                                  unsigned int timeRangeAlignment);

/**
 * Set EPG event filter.
 *
 * @param[in] self      EPG manager handle
 * @param[in] filter    handle to an object implementing SvEPGEventFilter
 *                      interface, NULL to remove existing filter
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerSetEventFilter(SvEPGManager self,
                           SvGenericObject filter,
                           SvErrorInfo *errorOut);

/**
 * Register new EPG channel list listener.
 *
 * @param[in] self      EPG manager handle
 * @param[in] listener  handle to an object implementing
 *                      SvEPGChannelListListener interface
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerAddChannelListListener(SvEPGManager self,
                                   SvGenericObject listener,
                                   SvErrorInfo *errorOut);

/**
 * Remove previously registered EPG channel list listener.
 *
 * @param[in] self      EPG manager handle
 * @param[in] listener  handle to an object implementing
 *                      SvEPGChannelListListener interface
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerRemoveChannelListListener(SvEPGManager self,
                                      SvGenericObject listener,
                                      SvErrorInfo *errorOut);

/**
 * Register new EPG cache monitor.
 *
 * @param[in] self      EPG manager handle
 * @param[in] monitor   handle to an object implementing @ref SvEPGCacheMonitor
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerAddCacheMonitor(SvEPGManager self,
                            SvGenericObject monitor,
                            SvErrorInfo *errorOut);

/**
 * Remove previously registered EPG cache monitor.
 *
 * @param[in] self      EPG manager handle
 * @param[in] monitor   handle to a cache monitor registered using
 *                      SvEPGManagerAddCacheMonitor()
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerRemoveCacheMonitor(SvEPGManager self,
                               SvGenericObject monitor,
                               SvErrorInfo *errorOut);

/**
 * Find channel with given ID.
 *
 * @param[in] self      EPG manager handle
 * @param[in] channelID channel ID
 * @param[out] errorOut error info
 * @return              TV channel handle, NULL in case of error
 **/
extern SvTVChannel
SvEPGManagerFindTVChannel(SvEPGManager self,
                          SvValue channelID,
                          SvErrorInfo *errorOut);

/**
 * Create new EPG data window.
 *
 * @param[in] self      EPG manager handle
 * @param[out] errorOut error info
 * @return              EPG data window handle, NULL in case of error
 **/
extern SvEPGDataWindow
SvEPGManagerCreateDataWindow(SvEPGManager self,
                             SvErrorInfo *errorOut);

/**
 * Create EPG iterator for specified channel data.
 *
 * @param[in] self      EPG manager handle
 * @param[in] channelID unique TV channel ID
 * @param[in] startTime initial position of the iterator
 * @return              new iterator (caller is responsible for
 *                      releasing it) or NULL in case of error
 **/
extern SvEPGIterator
SvEPGManagerCreateIterator(SvEPGManager self,
                           SvValue channelID,
                           time_t startTime);


/**
 * Checks if epg manager or its plugins are processing any channel changes
 * @param[in] self      EPG manager handle
 * @return              true iff any plugin or the epg manager is has
 *                      scheduled any channel change notifications
 **/
extern bool
SvEPGManagerIsPropagatingChannels(SvEPGManager self);

/**
 * Propagate notification about change in run-time properties
 * of one of TV channels.
 *
 * All EPGManager does is calling plugin method for propagating
 * channel change
 *
 * @param[in] self      EPG manager handle
 * @param[in] channel   TV channel handle
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerPropagateChannelChange(SvEPGManager self,
                                   SvTVChannel channel,
                                   SvErrorInfo *errorOut);

/**
 * Propagate notification about change in run-time properties
 * of EPG event data.
 *
 * @param[in] self      EPG manager handle
 * @param[in] channelID TV channel ID
 * @param[in] timeRange time range of changed events
 * @param[out] errorOut error info
 **/
extern void
SvEPGManagerPropagateEventsChange(SvEPGManager self,
                                  SvValue channelID,
                                  const SvTimeRange *timeRange,
                                  SvErrorInfo *errorOut);

extern bool
SvEPGManagerCheckIfRequestExists(SvEPGManager self,
                                 const SvTimeRange *timeRange,
                                 SvTVChannel channel);

extern unsigned int
SvEPGManagerGetTimeRangeAlignment(SvEPGManager self);

extern void
SvEPGManagerDontRequestEPGForChannelFromPlugin(SvEPGManager self,
                                               SvTVChannel channel,
                                               unsigned int pluginID,
                                               SvErrorInfo *errorOut);
/**
 * Set client specific update parameters.
 *
 * Used by EPG manager to use client specific optimizations.
 *
 * @param self          EPG manager handle
 * @param client        client handle
 * @param updateParams  update parameters handle
 */
extern void
SvEPGManagerSetUpdatesStrategyForClient(SvEPGManager self,
                                        SvObject client,
                                        SvEPGManagerUpdateParams updateParams);

/**
 * @}
 **/


#endif
