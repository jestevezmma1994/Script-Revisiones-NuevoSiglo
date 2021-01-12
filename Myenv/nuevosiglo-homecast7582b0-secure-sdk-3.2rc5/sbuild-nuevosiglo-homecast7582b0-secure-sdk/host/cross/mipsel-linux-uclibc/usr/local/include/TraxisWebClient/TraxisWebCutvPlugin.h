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

#ifndef TRAXIS_WEB_CUTV_PLUGIN_H_
#define TRAXIS_WEB_CUTV_PLUGIN_H_

/**
 * @file TraxisWebCutvPlugin.h
 * @brief Traxis.Web catch-up TV plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <TraxisWebClient/TraxisWebSessionManager.h>

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvEPGDataLayer/SvEPGManager.h>


/**
 * @defgroup TraxisWebCutvPlugin Traxis.Web catch-up TV plugin class
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web catch-up TV plugin class.
 * @class TraxisWebCutvPlugin
 * @extends SvObject
 **/
typedef struct TraxisWebCutvPlugin_s *TraxisWebCutvPlugin;


/**
 * Get runtime type identification object
 * representing Traxis.Web catch-up TV plugin class.
 *
 * @return Traxis.Web catch-up TV plugin class
 **/
extern SvType
TraxisWebCutvPlugin_getType(void);

/**
 * Initialize Traxis.Web catch-up TV plugin instance.
 *
 * @memberof TraxisWebCutvPlugin
 *
 * @param[in] self        Traxis.Web catch-up TV plugin handle
 * @param[in] sessionManager Traxis.Web session manager handle
 * @param[in] epgManager  EPG manager handle
 * @param[out] errorOut   error info
 * @return                @a self or @c NULL in case of error
 **/
extern TraxisWebCutvPlugin
TraxisWebCutvPluginInit(TraxisWebCutvPlugin self,
                        TraxisWebSessionManager sessionManager,
                        SvEPGManager epgManager,
                        SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
