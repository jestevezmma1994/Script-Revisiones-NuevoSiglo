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

#ifndef SV_EPG_TRAXISWEB_EPG_SEARCH_PLUGIN_H_
#define SV_EPG_TRAXISWEB_EPG_SEARCH_PLUGIN_H_

/**
 * @file TraxisWebEPGSearchPlugin.h
 * @brief Traxis.Web EPG search plugin
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>


/**
 * @defgroup TraxisWebEPGSearchPlugin Traxis.Web EPG search plugin class
 * @ingroup TraxisWebClient
 * @{
 **/

/**
 * Traxis.Web EPG search plugin class.
 * @class TraxisWebEPGSearchPlugin
 * @extends SvObject
 **/
typedef struct TraxisWebEPGSearchPlugin_s TraxisWebEPGSearchPlugin;


// FIXME: documentation!
typedef SvString (*TraxisWebEPGReverseChannelIdFn)(SvObject priv, SvString traxisChannelId);


/**
 * Create and initialize Traxis.Web EPG search plugin instance.
 *
 * @memberof TraxisWebEPGSearchPlugin
 *
 * @param[in] manager   Traxis.Web session manager handle
 * @param[out] errorOut error info
 * @return              newly created plugin or @c NULL in case of error
 **/
extern TraxisWebEPGSearchPlugin *
TraxisWebEPGSearchPluginCreate(TraxisWebSessionManager manager,
                               SvErrorInfo *errorOut);

/**
 * Setup function used by an EPG search plugin to map identifiers
 * of Traxis.Web channel objects to desired format.
 *
 * @memberof TraxisWebEPGSearchPlugin
 *
 * @param[in] self  Traxis.Web EPG search plugin handle
 * @param[in] func  function that maps channel identifiers
 * @param[in] prv   private context of @a func
 **/
extern void
TraxisWebEPGSearchPluginSetCustomReverseChannelIdFunction(TraxisWebEPGSearchPlugin* self,
                                                          TraxisWebEPGReverseChannelIdFn func,
                                                          SvObject prv);

/**
 * @}
 **/


#endif
