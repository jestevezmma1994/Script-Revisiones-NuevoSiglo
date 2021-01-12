/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_DATA_PLUGIN_H_
#define SV_EPG_DATA_PLUGIN_H_

/**
 * @file SvEPGDataPlugin.h
 * @brief EPG data plugin interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvEPGDataLayer/SvEPGDataRequest.h>
#include <SvEPGDataLayer/SvEPGChannelList.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * @defgroup SvEPGDataPlugin EPG data plugin interface
 * @ingroup SvEPGDataLayer
 * @{
 **/


struct SvEPGDataPluginConfig_s
{
    int  maxChannelsCountPerRequest; /// submit requests for confirmed channels count separately
    int  epgDataExpirationTimeout;   /// 0 iff not expiring
    int  epgDataFailedRegetDelay;    /// 0 when want to use @a epgDataExpirationTimeout

    bool searchByCategoryEnabled;  /// true if plugin can handle searchich by category
};
typedef struct SvEPGDataPluginConfig_s  SvEPGDataPluginConfig;


/**
 * SvEPGDataPlugin interface.
 **/
typedef const struct SvEPGDataPlugin_ {
    /**
     * Get plugin name.
     *
     * @param[in] self_     EPG data plugin handle
     * @return              plugin name
     **/
    SvString (*getName)(SvGenericObject self_);

    /**
     * Get plugin configuration.
     *
     * @param[in] self_     EPG data plugin handle
     * @return              plugin configuration
     **/
    const SvEPGDataPluginConfig* (*getConfig)(SvGenericObject self_);

    /**
     * Get plugin's unique ID.
     *
     * @param[in] self_     EPG data plugin handle
     * @return              plugin's unique ID, @c -1 on error
     **/
    int (*getID)(SvGenericObject self_);

    /**
     * Set plugin's unique ID.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] pluginID  plugin's unique ID
     **/
    void (*setID)(SvGenericObject self_,
                  unsigned int pluginID);

    /**
     * Start EPG data plugin tasks.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] scheduler handle to the scheduler to be used by the plugin
     * @param[in] channelsList EPG channels list handle
     * @param[out] errorOut error info
     **/
    void (*start)(SvGenericObject self_,
                  SvScheduler scheduler,
                  SvEPGChannelList channelsList,
                  SvErrorInfo *errorOut);

    /**
     * Stop EPG data plugin tasks.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[out] errorOut error info
     **/
    void (*stop)(SvGenericObject self_,
                SvErrorInfo *errorOut);

    /**
     * Register new EPG request listener.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] listener  handle to an object implementing @ref SvEPGRequestListener
     * @param[out] errorOut error info
     **/
    void (*addListener)(SvGenericObject self_,
                        SvGenericObject listener,
                        SvErrorInfo *errorOut);

    /**
     * Unregister EPG request listener.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] listener  handle to a previously registered listener
     * @param[out] errorOut error info
     **/
    void (*removeListener)(SvGenericObject self_,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut);

    /**
     * Notify that new channel has been found.
     *
     * This method starts collecting and reporting EPG data for new channel from plugin.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] channel   newly found channel
     **/
    void (*channelFound)(SvGenericObject self_,
                         SvTVChannel channel);

    /**
     * Notify that one of the channels is not available anymore.
     *
     * This method stops collecting any EPG data for channel so no EPG events
     * will be reported from this plugin regarding channelLost
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] channel   channel that has been lost
     **/
    void (*channelLost)(SvGenericObject self_,
                        SvTVChannel channel);

    /**
     * Allocate new EPG data request.
     *
     * This method allocates new EPG data request. Plugin can choose whether
     * to create an instance of SvEPGDataRequest class, or some plugin-specific
     * class derived from SvEPGDataRequest.
     *
     * @param[in] self_     EPG data plugin handle
     * @return              EPG data request object, @c NULL in case of error
     **/
    SvEPGDataRequest (*createRequest)(SvGenericObject self_);

    /**
     * Submit EPG data request.
     *
     * This method asks EPG plugin to start processing EPG data request.
     * Notifications about all changes to request's state will be passed
     * to the @a listener.
     *
     * When this method leaves without reporting error, EPG data request
     * will be in one of two possible states: processing or finished.
     * In the latter case, EPG plugin signals that request is superfluous.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] request   EPG data request
     * @param[in] listener  handle to an object implementing @ref SvEPGRequestListener
     * @param[out] errorOut error info
     **/
    void (*submitRequest)(SvGenericObject self_,
                          SvEPGDataRequest request,
                          SvGenericObject listener,
                          SvErrorInfo *errorOut);

    /**
     * Cancel EPG data request.
     *
     * This method informs EPG plugin, that manager is no longer interested
     * with EPG data described by a particular data request. It is perfectly
     * safe if EPG plugin does not support cancelling requests; in such case
     * data delivered by cancelled request should be dropped by the entity
     * that issued the request.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] requestID ID of a previously submitted EPG data request
     * @param[out] errorOut error info
     **/
    void (*cancelRequest)(SvGenericObject self_,
                          SvValue requestID,
                          SvErrorInfo *errorOut);

    /**
     * Inform EPG data plugin that manager has dropped some EPG events.
     *
     * @param[in] self_     EPG data plugin handle
     * @param[in] timeRange time range of the dropped data
     * @param[in] channel   TV channel for which data was dropped
     * @param[in] channelRemoved @c true iff data was dropped because channel
     *                      no longer exists
     **/
    void (*dataDropped)(SvGenericObject self_,
                        const SvTimeRange *timeRange,
                        SvTVChannel channel,
                        bool channelRemoved);
} *SvEPGDataPlugin;


/**
 * Get runtime type identification object representing
 * EPG data plugin interface.
 *
 * @return SvEPGDataPlugin interface object
 **/
extern SvInterface
SvEPGDataPlugin_getInterface(void);

/**
 * @}
 **/


#endif
