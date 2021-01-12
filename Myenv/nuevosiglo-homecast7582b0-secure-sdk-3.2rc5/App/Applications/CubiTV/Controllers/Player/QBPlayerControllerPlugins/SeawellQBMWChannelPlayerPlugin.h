/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SEAWELLQBMWCHANNELPLAYERPLUGIN_H
#define SEAWELLQBMWCHANNELPLAYERPLUGIN_H

/**
 * @file SeawellQBMWChannelPlayerPlugin.h
 * @brief Cubiware MW Channel Player Controller Seawell Plugin API.
 *
 * Seawell tv channels auth_tokens (with validity time) can be sent along with
 * the channel lineup or in the UseProduct answer from the MW. Seawell plugin
 * caches UseProduct answers and reuses auth_token if it is still valid.
 *
 * If plugin has valid auth_token from lineup or from cache then it updates
 * content's URI and reports that playback can be started.
 *
 * When the UseProduct answer comes then it caches it.
 *
 * If it doesn't have valid auth_token from lineup then it checks if the cached
 * answer has valid token and uses it.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <main_decl.h>
#include <Services/core/QBMiddlewareManager.h>
#include <Services/QBMWConfigHandler.h>
#include <Services/QBMWConfigHandlerPlugins/SeawellQBMWConfigHandlerPlugin.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup SeawellQBMWChannelPlayerPlugin Cubiware MW Channel Player Controller Seawell Plugin
 * @ingroup QBPlayerControllerPlugins
 * @{
 **/


/**
 * @brief SeawellQBMWChannelPlayerPlugin type
 */
typedef struct SeawellQBMWChannelPlayerPlugin_* SeawellQBMWChannelPlayerPlugin;

/**
 * @brief SeawellQBMWChannelPlayerPluginCreate
 * Creates Seawell QB MW player controllers plugin
 * @param[in] qbMWProductUsageController    Middleware Product Usage Controller handle
 * @param[in] middlewareManager                Middleware Manager handle
 * @param[in] qbMWConfigHandler                Middleware Config handle
 * @param[in] dvbChannelList                Egg plugin DVB channel list
 * @param[in] ipChannelList                    Egg plugin IP channel list
 * @param[out] errorOut     error info
 * @return                  created plugin or NULL
 */
SeawellQBMWChannelPlayerPlugin
SeawellQBMWChannelPlayerPluginCreate(QBMWProductUsageController qbMWProductUsageController,
                                     QBMiddlewareManager middlewareManager,
                                     QBMWConfigHandler qbMWConfigHandler,
                                     SvGenericObject dvbChannelList,
                                     SvGenericObject ipChannelList,
                                     SvErrorInfo *errorOut);
/**
 * @}
 **/

#endif // SEAWELLQBMWCHANNELPLAYERPLUGIN_H
