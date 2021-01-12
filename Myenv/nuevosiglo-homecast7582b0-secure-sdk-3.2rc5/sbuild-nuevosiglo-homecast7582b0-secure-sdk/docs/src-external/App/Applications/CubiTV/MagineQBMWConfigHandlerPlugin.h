/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2018 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef MAGINE_QB_MW_CONFIG_HANDLER_PLUGIN_H_
#define MAGINE_QB_MW_CONFIG_HANDLER_PLUGIN_H_

/**
 * @file MagineQBMWConfigHandlerPlugin.h
 * @brief Cubiware MW config handler - Magine Plugin API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>


/**
 * Magine plugin name string.
 */
#define QBMW_CONFIG_HANDLER_PLUGIN_MAGINE  "MagineQBMWConfigHandlerPlugin"

/**
 * @defgroup MagineQBMWConfigHandlerPlugin Cubiware MW Config Handler Magine Plugin
 * @ingroup QBMWConfigHandlerPlugins
 * @{
 **/

/**
 * @brief MagineQBMWConfigHandlerPlugin type
 */
typedef struct MagineQBMWConfigHandlerPlugin_ *MagineQBMWConfigHandlerPlugin;

/**
 * Create Magine QB MW config handler plugin
 *
 * @param[out] errorOut     error info
 * @return                  created plugin or NULL
 */
MagineQBMWConfigHandlerPlugin
MagineQBMWConfigHandlerPluginCreate(SvErrorInfo *errorOut);

/**
 * Get Magine Api Token as string
 *
 * @param[in] self      plugin handle
 * @return              Magine Api Token as string
 */
SvString
MagineQBMWConfigHandlerPluginGetMagineApiToken(MagineQBMWConfigHandlerPlugin self);

/**
 * Get Magine Secret as string
 *
 * @param[in] self      plugin handle
 * @return              Magine Secret as string
 */
SvString
MagineQBMWConfigHandlerPluginGetMagineSecret(MagineQBMWConfigHandlerPlugin self);

/**
 * Get Magine Device Id as string
 *
 * @param[in] self      plugin handle
 * @return              Magine Device Id as string
 */
SvString
MagineQBMWConfigHandlerPluginGetMagineDeviceId(MagineQBMWConfigHandlerPlugin self);

/**
 * @}
 **/

#endif // MAGINE_QB_MW_CONFIG_HANDLER_PLUGIN_H_
