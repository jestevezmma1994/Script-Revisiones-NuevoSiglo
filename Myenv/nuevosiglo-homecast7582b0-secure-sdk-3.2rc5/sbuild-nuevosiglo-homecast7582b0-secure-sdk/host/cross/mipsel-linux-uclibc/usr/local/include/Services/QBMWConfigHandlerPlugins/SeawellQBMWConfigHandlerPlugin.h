/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SEAWELLINNOV8ONCONFIGHANDLERPLUGINS_H
#define SEAWELLINNOV8ONCONFIGHANDLERPLUGINS_H

/**
 * @file SeawellQBMWConfigHandlerPlugin.h
 * @brief Cubiware MW config handler - Seawell Plugin API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>

/**
 * Seawell plugin name string.
 */
#define QBMW_CONFIG_HANDLER_PLUGIN_SEAWELL  "SeawellQBMWConfigHandlerPlugin"

/**
 * @defgroup SeawellQBMWConfigHandlerPlugin Cubiware MW Config Handler Seawell Plugin
 * @ingroup QBMWConfigHandlerPlugins
 * @{
 *
 * This plugin expects to get information whether or not the Seawell channel authentication tokens
 * are sent in the channels data. It exposes that information to the interesed components.
 *
 **/


/**
 * @brief SeawellQBMWConfigHandlerPlugin type
 */
typedef struct SeawellQBMWConfigHandlerPlugin_* SeawellQBMWConfigHandlerPlugin;

/**
 * Create Seawell QB MW config handler plugin
 *
 * @param[out] errorOut     error info
 * @return                  created plugin or NULL
 */
SeawellQBMWConfigHandlerPlugin
SeawellQBMWConfigHandlerPluginCreate(SvErrorInfo *errorOut);

/**
 * Tell if the Seawell authentication tokens are available in the channel data.
 *
 * @param[in] self      plugin handle
 * @return              true if auth tokens are sent with the channel lineup
 */
extern bool
SeawellQBMWConfigHandlerPluginAuthTokensInLineupEnabled(SeawellQBMWConfigHandlerPlugin self);

/**
 * @}
 **/

#endif // SEAWELLINNOV8ONCONFIGHANDLERPLUGINS_H
