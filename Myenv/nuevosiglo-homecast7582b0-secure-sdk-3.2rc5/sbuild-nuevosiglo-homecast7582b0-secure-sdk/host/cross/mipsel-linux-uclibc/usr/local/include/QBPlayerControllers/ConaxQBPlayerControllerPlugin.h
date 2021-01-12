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

#ifndef CONAX_QB_PLAYER_CONTROLLER_PLUGIN_H
#define CONAX_QB_PLAYER_CONTROLLER_PLUGIN_H

/**
 * @file ConaxQBPlayerControllerPlugin.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Cubiware Player Controller Conax Plugin API.
 *
 * Conax implementation generates conax id at device side and send to MW, so there
 * is no need to parse UseProduct call. Conax token is requested in different MW call.
 *
 * This plugin sets PlayReady type to Conax and handles EMM for VoDs.
 *
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>

/**
 * @defgroup ConaxQBPlayerControllerPlugin Cubiware MW Player Controller Conax Plugin
 * @ingroup QBPlayerControllerPlugins
 * @{
 **/

/**
 * ConaxQBPlayerControllerPlugin type
 */
typedef struct ConaxQBPlayerControllerPlugin_* ConaxQBPlayerControllerPlugin;

/**
 * Creates Conax QB player controllers plugin
 *
 * @param[in] qbMWProductUsageController MW product usage controller
 * @param[out] errorOut error info
 * @return created plugin or NULL
 */
ConaxQBPlayerControllerPlugin
ConaxQBPlayerControllerPluginCreate(QBMWProductUsageController qbMWProductUsageController, SvErrorInfo* errorOut);

/**
 * @}
 **/

#endif // CONAX_QB_PLAYER_CONTROLLER_PLUGIN_H
