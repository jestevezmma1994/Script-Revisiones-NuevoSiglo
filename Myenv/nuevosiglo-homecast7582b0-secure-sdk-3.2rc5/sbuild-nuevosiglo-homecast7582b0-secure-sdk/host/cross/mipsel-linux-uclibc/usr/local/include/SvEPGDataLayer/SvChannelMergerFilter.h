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

#ifndef SVCHANNELMERGERFILTER_H
#define SVCHANNELMERGERFILTER_H

/**
 * @file SvChannelMergerFilter.h
 * @brief Channel merger filter interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup SvChannelMergersFilter filter for channel merger
 * @ingroup SvEPGDataLayer
 **/

/**
 * SvEPGExtendedChannelListListener interface.
 **/
typedef const struct SvChannelMergerFilter_ {
    /**
     * Notify that new channel has been found.
     *
     * @param[in] self_     handle to an object implementing @ref SvChannelMergersFilter
     * @param[in] channel   handle to an SvTVChannel object representing
     *                      newly found channel
     **/
    void (*channelFound)(SvObject self_,
                         SvTVChannel channel);

    /**
     * Notify that one of the channels is not available anymore.
     *
     * @param[in] self_     handle to an object implementing @ref SvChannelMergerFilter
     * @param[in] channel   handle to an SvTVChannel object representing
     *                      channel that has been lost
     **/
    void (*channelLost)(SvObject self_,
                        SvTVChannel channel);
    /**
     * Update state of channel list filter.
     *
     * @param[in] self_         handle to an object implementing @ref SvChannelMergerFilter
     * @param[in] pluginID      ID of plugin which has triggered state update
     * @return                  new filter state - true when enabled
     **/
    bool (*updateState)(SvObject self_, unsigned int pluginID);

    /**
     * Check if filter is enabled.
     *
     * @param[in] self_         handle to an object implementing @ref SvChannelMergerFilter
     * @return                  true when enabled
     **/
    bool (*isEnabled)(SvObject self_);

    /**
     * Check if channel is allowed.
     *
     * @param[in] self_         handle to an object implementing @ref SvChannelMergerFilter
     * @param[in] channelID     channel ID to check
     * @return                  true when channel is allowed
     **/
    bool (*isAllowed)(SvObject self_, SvValue channelID);
} *SvChannelMergerFilter;

/**
 * Get runtime type identification object representing
 * channel merger filter interface.
 *
 * @return SvChannelMergerFilter interface object
 **/
extern SvInterface
SvChannelMergerFilter_getInterface(void);

/** @} */

#endif // SVCHANNELMERGERFILTER_H
