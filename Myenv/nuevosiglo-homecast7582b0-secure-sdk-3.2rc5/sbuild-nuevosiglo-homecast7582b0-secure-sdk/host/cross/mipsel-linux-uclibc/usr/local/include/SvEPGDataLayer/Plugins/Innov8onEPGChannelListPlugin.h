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

#ifndef INNOV8ON_EPG_CHANNEL_LIST_PLUGIN_H_
#define INNOV8ON_EPG_CHANNEL_LIST_PLUGIN_H_

/**
 * @file Innov8onEPGChannelListPlugin.h
 * @brief Innov8on EPG channel list plugin class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURL.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>


/**
 * @defgroup Innov8onEPGChannelListPlugin Innov8on EPG channel list plugin class
 * @ingroup Innov8onEPGPlugin
 * @{
 **/

/**
 * Innov8on EPG channel list plugin class.
 **/
typedef struct Innov8onEPGChannelListPlugin_ *Innov8onEPGChannelListPlugin;

/**
 * Get runtime type identification object
 * representing Innov8on EPG channel list plugin class.
 **/
extern SvType
Innov8onEPGChannelListPlugin_getType(void);

/**
 * Initialize Innov8on EPG channel list plugin instance.
 *
 * @param[in] self      Innov8on EPG channel list plugin handle
 * @param[in] serverInfo Innov8on server information
 * @param[out] errorOut error info
 * @return              self or @c NULL in case of error
 **/
extern Innov8onEPGChannelListPlugin
Innov8onEPGChannelListPluginInit(Innov8onEPGChannelListPlugin self,
                                 SvXMLRPCServerInfo serverInfo,
                                 SvErrorInfo *errorOut);

/**
 * Enable service that is responsible for updating Seawell tokens
 * needed to play the channels.
 *
 * @param self          Innov8on EPG channel list plugin handle
 */
extern void
Innov8onEPGChannelListPluginEnableSeawellChannelTokenRefresher(Innov8onEPGChannelListPlugin self);

/**
 * Set thumbnails' base URL.
 *
 * @param[in] self      Innov8on EPG channel list plugin handle
 * @param[in] baseURL   base URL for relative thumbnails' URLs
 * @param[out] errorOut error info
 **/
extern void
Innov8onEPGChannelListPluginSetBaseURL(Innov8onEPGChannelListPlugin self,
                                       SvURL baseURL,
                                       SvErrorInfo *errorOut);

/**
 * Set Innov8on user ID.
 *
 * @param[in] self      Innov8on EPG channel list plugin handle
 * @param[in] userID    user ID
 * @param[out] errorOut error info
 **/
extern void
Innov8onEPGChannelListPluginSetUserID(Innov8onEPGChannelListPlugin self,
                                      SvString userID,
                                      SvErrorInfo *errorOut);

/**
 * Get Innov8on user ID
 * @param[in] self      Innov8on EPG channel list plugin handle
 * @param[out] errorOut error info
 * @return              user ID
 */
SvString
Innov8onEPGChannelListPluginGetUserID(Innov8onEPGChannelListPlugin self,
                                      SvErrorInfo *errorOut);


/**
 * Set Innov8on XML RPC server info.
 *
 * @param[in] self          Innov8on EPG channel list plugin handle
 * @param[in] serverInfo    XML RPC server info
 * @param[out] errorOut     error info
 **/
extern void
Innov8onEPGChannelListPluginSetServerInfo(Innov8onEPGChannelListPlugin self,
                                          SvXMLRPCServerInfo serverInfo,
                                          SvErrorInfo *errorOut);

/**
 * Set channel list refresh period.
 *
 * @param[in] self      Innov8on EPG channel list plugin handle
 * @param[in] period    channel list refresh period in milliseconds,
 *                      @c 0 to disable periodic updates
 * @param[out] errorOut error info
 **/
extern void
Innov8onEPGChannelListPluginSetRefreshPeriod(Innov8onEPGChannelListPlugin self,
                                             unsigned int period,
                                             SvErrorInfo *errorOut);

/**
 * Remove all channels.
 * @param[in] self      Innov8on EPG channel list plugin handle
 */
extern void
Innov8onEPGChannelListPluginDropAllChannels(Innov8onEPGChannelListPlugin self);

/**
 * Force a new request for channels
 * @params[in] self     Innov8on EPG channel list plugin handle
 */
extern void
Innov8onEPGChannelListPluginForceUpdate(Innov8onEPGChannelListPlugin self);

/*
 * Toggles whether the plugin should obtain only entitled channels or all available
 * @params[in] self     Innov8on EPG channel
 * @params[in] entitledOnly true iff the plugin should obtain only entitled channels
 */
extern void
Innov8onEPGChannelListPluginObtainEntitledOnly(Innov8onEPGChannelListPlugin self,
                                               bool entitledOnly);
/*
 * Toggles whether the plugin should obtain channels at all
 * @params[in] self     Innov8on EPG channel
 * @params[in] obtain true iff the plugin should obtain channels
 */
extern void
Innov8onEPGChannelListPluginToggleObtaining(Innov8onEPGChannelListPlugin self, bool obtain);


/**
 * @}
 **/


#endif
