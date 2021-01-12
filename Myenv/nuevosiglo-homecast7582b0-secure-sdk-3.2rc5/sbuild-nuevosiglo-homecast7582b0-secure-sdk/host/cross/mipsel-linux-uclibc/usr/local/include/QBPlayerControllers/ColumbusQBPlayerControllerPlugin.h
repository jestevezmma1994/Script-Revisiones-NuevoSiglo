/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef COLUMBUS_QB_PLAYER_CONTROLLER_PLUGIN_H
#define COLUMBUS_QB_PLAYER_CONTROLLER_PLUGIN_H

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvObject.h>

/**
 * @file ColumbusQBPlayerControllerPlugin.h
 * @brief Cubiware Player Controller Columbus Plugin API.
 *
 * Columbus plugin provides playready features (conax id and conax token) that will be completed
 * when "conax_contego_token" and "conax_contego_id" will come in the UseProduct answer.
 *
 * Columbus MW generates conax_id so there is a need to store it in device.
 * This plugin implements QBConaxPlayReadyHelperServiceInterface that provides
 * conaxId and conaxToken to conax playready implementation.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup ColumbusQBPlayerControllerPlugin Cubiware MW Player Controller Conax Plugin
 * @ingroup QBPlayerControllerPlugins
 * @{
 **/

/**
 * @brief ColumbusQBPlayerControllerPlugin Columbus plugin type.
 */
typedef struct ColumbusQBPlayerControllerPlugin_* ColumbusQBPlayerControllerPlugin;

/**
 * @brief ColumbusQBPlayerControllerPluginGetInstance returns static instance of ColumbusQBPlayerControllerPlugin.
 *
 * This instance implements QBConaxPlayReadyHelperServiceInterface so must be created only once.
 * @return Columbus plugin object or NULL in case of error.
 */
ColumbusQBPlayerControllerPlugin
ColumbusQBPlayerControllerPluginGetInstance(void);

/**
 * @brief ColumbusQBPlayerControllerPluginClenUp destroys static instance of Columbus plugin.
 */
void
ColumbusQBPlayerControllerPluginClenUp(void);

/**
 * @brief ColumbusQBPlayerControllerPluginSetHelper sets helper for Columbus plugin.
 *
 * @param[in] self_ self object
 * @param[in] helper object that implements QBConaxPlayReadyHelperServiceInterface
 * @param[out] errorOut pointer to SvErrorInfo
 */
void
ColumbusQBPlayerControllerPluginSetHelper(SvObject self_, SvObject helper, SvErrorInfo* errorOut);


/**
 * @}
 **/

#endif // COLUMBUS_QB_PLAYER_CONTROLLER_PLUGIN_H
