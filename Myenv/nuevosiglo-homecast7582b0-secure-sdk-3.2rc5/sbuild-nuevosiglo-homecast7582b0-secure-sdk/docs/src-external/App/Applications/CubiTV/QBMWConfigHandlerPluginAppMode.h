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

#ifndef QBMWCONFIGHANDLERPLUGINAPPMODE_H
#define QBMWCONFIGHANDLERPLUGINAPPMODE_H

/**
 * @file QBMWConfigHandlerPluginAppMode.h
 * @brief Cubiware MW config handler - App Mode Plugin API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>

/**
 * App Mode plugin name string.
 */
#define QBMW_CONFIG_HANDLER_PLUGIN_APPMODE  "QBMWConfigHandlerPluginAppMode"

/**
 * @defgroup QBMWConfigHandlerPluginAppMode Cubiware MW Config Handler App Mode Plugin
 * @ingroup QBMWConfigHandlerPlugins
 * @{
 *
 * This plugin checks whether stb mode received from middleware differs from
 * the one in config and saves it if needed.
 *
 **/

/**
 * @brief QBMWConfigHandlerPluginAppMode type
 */
typedef struct QBMWConfigHandlerPluginAppMode_* QBMWConfigHandlerPluginAppMode;

/**
 * Create App Mode QB MW config handler plugin
 *
 * @param[out] errorOut     error info
 * @return                  created plugin or NULL
 */
QBMWConfigHandlerPluginAppMode
QBMWConfigHandlerPluginAppModeCreate(SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif // QBMWCONFIGHANDLERPLUGINAPPMODE_H
