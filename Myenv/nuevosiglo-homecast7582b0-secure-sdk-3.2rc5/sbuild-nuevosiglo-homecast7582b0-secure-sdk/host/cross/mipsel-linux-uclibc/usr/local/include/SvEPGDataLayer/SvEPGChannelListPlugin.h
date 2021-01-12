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

#ifndef SV_EPG_CHANNEL_LIST_PLUGIN_H_
#define SV_EPG_CHANNEL_LIST_PLUGIN_H_

/**
 * @file SvEPGChannelListPlugin.h
 * @brief EPG channel list plugin interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvEPGDataLayer/SvEPGDataRequest.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * @defgroup SvEPGChannelListPlugin EPG channel list plugin interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

typedef void (*SvEPGChannelListPluginChannelFill)(void *self_, SvGenericObject plug, SvTVChannel channel);
typedef void (*SvEPGChannelListPluginChannelMetaFill)(void *fillData, SvTVChannel channel, bool found);

/**
 * SvEPGChannelListPlugin interface.
 **/
typedef const struct SvEPGChannelListPlugin_ {
    /**
     * Get plugin name.
     *
     * @param[in] self_  EPG channel list plugin handle
     * @return           plugin name
     **/
    SvString (*getName)(SvGenericObject self_);

    /**
     * Get plugin's unique ID.
     *
     * @param[in] self_  EPG channel list plugin handle
     * @return           plugin's unique ID, @c -1 on error
     **/
    int (*getID)(SvGenericObject self_);

    /**
     * Set plugin's unique ID.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[in] pluginID  plugin's unique ID
     **/
    void (*setID)(SvGenericObject self_,
                  unsigned int pluginID);

    /**
     * Start EPG channel list plugin tasks.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[in] scheduler handle to the scheduler to be used by the plugin
     * @param[out] errorOut error info
     **/
    void (*start)(SvGenericObject self_,
                  SvScheduler scheduler,
                  SvErrorInfo *errorOut);

    /**
     * Stop EPG channel list plugin tasks.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[out] errorOut error info
     **/
    void (*stop)(SvGenericObject self_,
                 SvErrorInfo *errorOut);

    /**
     * Register new EPG channel list listener.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[in] listener  handle to an object implementing @ref SvEPGChannelListListener
     * @param[in] highPriority @c true for high priority listeners (receive notifications before others)
     * @param[out] errorOut error info
     **/
    void (*addListener)(SvGenericObject self_,
                        SvGenericObject listener,
                        bool highPriority,
                        SvErrorInfo *errorOut);

    /**
     * Remove previously registered EPG channel list listener.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[in] listener  handle to a previously registered EPG channel list listener
     * @param[out] errorOut error info
     **/
    void (*removeListener)(SvGenericObject self_,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut);

    /**
     * Request an immediate update of the channels list.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[out] errorOut error info
     **/
    void (*update)(SvGenericObject self_,
                   SvErrorInfo *errorOut);

    /**
     * Propagate notification about change in run-time properties
     * of one of TV channels.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[in] channel   TV channel handle
     * @param[out] errorOut error info
     **/
    void (*propagateChannelChange)(SvGenericObject self_,
                                   SvTVChannel channel,
                                   SvErrorInfo *errorOut);

    /**
     * Check if plugin is processing / propagating channels.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @return              @c true iff plugin is processing/propagating channels
     */
    bool (*isPropagatingChannels)(SvGenericObject self_);

    /**
     * Restore list of channels from saved state.
     *
     * @param[in] self_     EPG channel list plugin handle
     * @param[in] channels  array of channels
     **/
    void (*restoreChannels)(SvGenericObject self_,
                            SvImmutableArray channels);

    /**
     * Set channel fill helper function.
     * This fill function can set or modify data for the new channel.
     *
     * @param[in] self_                 EPG channel list plugin handle
     * @param[in] channelFill           channel data fill helper
     * @param[in] channelFillData       channel data fill data pointer
     **/
    void (*setChannelFillHelper)(SvGenericObject self_,
                                 SvEPGChannelListPluginChannelFill channelFill,
                                 void* channelFillData);

    /**
     * Set channel metadata fill helper function.
     * This fill function can set or modify metadata for the new channel.
     *
     * @param[in] self_                 EPG channel list plugin handle
     * @param[in] channelMetaFill       channel meta fill helper
     * @param[in] channelMetaFillData   channel meta fill data pointer
     **/
    void (*setChannelMetaFillHelper)(SvGenericObject self_,
                                     SvEPGChannelListPluginChannelMetaFill channelMetaFill,
                                     void* channelMetaFillData);

} *SvEPGChannelListPlugin;


/**
 * Get runtime type identification object representing
 * EPG channel list plugin interface.
 *
 * @return SvEPGChannelListPlugin interface object
 **/
extern SvInterface
SvEPGChannelListPlugin_getInterface(void);

/**
 * @}
 **/


#endif
