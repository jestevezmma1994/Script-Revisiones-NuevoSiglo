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

#ifndef HIBOXQBANYPLAYERCONTROLLERPLUGIN_H_
#define HIBOXQBANYPLAYERCONTROLLERPLUGIN_H_

/**
 * @file HiboxQBAnyPlayerControllerPlugin.h Hibox plugin for QBAnyPlayerController
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup HiboxQBAnyPlayerControllerPlugin Hibox plugin for Player Controller
 * @ingroup QBPlayerControllerPlugins
 * @{
 *
 * Hibox plugin for QBAnyPlayerController.
 **/

/**
 * Hibox plugin for QBAnyPlayerController.
 *
 * @class HiboxQBAnyPlayerControllerPlugin HiboxQBAnyPlayerControllerPlugin.h <Controllers/player/QBPlayerControllerPlugins/QHiboxQBAnyPlayerControllerPlugin.h>
 * @extends SvObject
 * @implements QBPlayerControllerPlugin
 * @implements SvHTTPClientListener
 *
 * This class implements QBPlayerControllerPlugin interface for Hibox
 * middleware server. It is responsible for getting playback url from Hibox
 * server and passing it to the player.
 *
 * Playback url is obtained from the answer to POST request sent to Hibox
 * server. Request URL is stored in QBConfig under they 'HIBOX_URL' key.
 * The answer to POST request is a JavaScript code containing among other
 * things, the object with upgradeUrl attribute.
 *
 * @note This plugin does not parse JavaScript code, playbackUrl is obtained
 *       by simple pattern matching.
 **/
typedef struct HiboxQBAnyPlayerControllerPlugin_ * HiboxQBAnyPlayerControllerPlugin;

/**
 * Create HiboxQBAnyPlayerControllerPlugin instance.
 *
 * @public @memberof HiboxQBAnyPlayerControllerPlugin
 *
 * @param[out] errorOut error info
 * @return              new HiboxQBAnyPlayerControllerPlugin instance or @c NULL in case of error
 **/
HiboxQBAnyPlayerControllerPlugin
HiboxQBAnyPlayerControllerPluginCreate(SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* HIBOXQBANYPLAYERCONTROLLERPLUGIN_H_ */
