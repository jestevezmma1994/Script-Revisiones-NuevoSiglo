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

#ifndef TRAXIS_WEB_EPG_DATA_PLUGIN_H_
#define TRAXIS_WEB_EPG_DATA_PLUGIN_H_

/**
 * @file TraxisWebEPGDataPlugin.h
 * @brief Traxis.Web EPG data plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <TraxisWebClient/TraxisWebSessionManager.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * @defgroup TraxisWebEPGDataPlugin Traxis.Web EPG data plugin class
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web EPG data plugin class.
 * @class TraxisWebEPGDataPlugin
 * @extends SvObject
 **/
typedef struct TraxisWebEPGDataPlugin_s *TraxisWebEPGDataPlugin;


/**
 * Configuration of the Traxis.Web EPG data plugin.
 **/
typedef struct TraxisWebEPGDataPluginInitConfig_s {
    /// subscription products refresh period in seconds, use @c 0 for default
    unsigned int productsRefreshPeriod;
    /// subscription products refresh period in seconds, used when previous refresh failed, use @c 0 for default
    unsigned int productsRefreshPeriodFailedRetry;

    /// events expiration timeout, use @c 0 for default
    unsigned int eventsExpirationTimeout;
    /// events refresh timeout, used when previous refresh failed, use @c 0 for default
    unsigned int eventsExpirationTimeoutFailedRetry;
} TraxisWebEPGDataPluginInitConfig;


/**
 * Get runtime type identification object
 * representing Traxis.Web EPG data plugin class.
 *
 * @return Traxis.Web EPG data plugin class
 **/
extern SvType
TraxisWebEPGDataPlugin_getType(void);

/**
 * Initialize Traxis.Web EPG data plugin instance.
 *
 * @memberof TraxisWebEPGDataPlugin
 *
 * @param[in] self        Traxis.Web EPG data plugin handle
 * @param[in] sessionManager Traxis.Web session manager handle
 * @param[in] initConfig  configuration of the EPG data plugin
 * @param[out] errorOut   error info
 * @return                @a self or @c NULL in case of error
 **/
extern TraxisWebEPGDataPlugin
TraxisWebEPGDataPluginInit(TraxisWebEPGDataPlugin self,
                           TraxisWebSessionManager sessionManager,
                           const TraxisWebEPGDataPluginInitConfig *initConfig,
                           SvErrorInfo *errorOut);

// FIXME: documentation!
typedef SvString (*TraxisWebEPGChannelIdFn)(SvObject priv, SvTVChannel channel);

/**
 * Registers a function that is called after receiving a channel and before
 * propagating it further. It is used to overwrite Traxis channel ID into customer
 * specific channel ID. For example, it may be used to ease matching Traxis channel
 * ID with DVB channel id.
 *
 * @param[in] self        Traxis.Web EPG data plugin handle
 * @param[in] func        function that is called
 * @param[in] priv        first argument to function containing its private data
 */
extern void
TraxisWebEPGDataPluginSetCustomChannelIdFunction(TraxisWebEPGDataPlugin self,
                                                 TraxisWebEPGChannelIdFn func,
                                                 SvObject priv);

/**
 * @}
 **/


#endif
