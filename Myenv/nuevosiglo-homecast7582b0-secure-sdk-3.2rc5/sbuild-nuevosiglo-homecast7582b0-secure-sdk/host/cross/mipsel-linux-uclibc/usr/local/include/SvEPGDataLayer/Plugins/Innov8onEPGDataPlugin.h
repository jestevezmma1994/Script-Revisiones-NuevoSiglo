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

#ifndef INNOV8ON_EPG_DATA_PLUGIN_H_
#define INNOV8ON_EPG_DATA_PLUGIN_H_

/**
 * @file Innov8onEPGDataPlugin.h
 * @brief Innov8on EPG data plugin class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvXMLRPCClient/SvXMLRPCServerInfo.h>


/**
 * @defgroup Innov8onEPGDataPlugin Innov8on EPG data plugin class
 * @ingroup Innov8onEPGPlugin
 * @{
 **/

/**
 * Innov8on EPG data plugin class.
 **/
typedef struct Innov8onEPGDataPlugin_ *Innov8onEPGDataPlugin;


/**
 * Get runtime type identification object
 * representing Innov8on EPG data plugin class.
 **/
extern SvType
Innov8onEPGDataPlugin_getType(void);

/**
 * Initialize Innov8on EPG data plugin instance.
 *
 * @param[in] self      Innov8on EPG data plugin handle
 * @param[in] serverInfo Innov8on server information
 * @param[out] errorOut error info
 * @return              self or @c NULL in case of error
 **/
extern Innov8onEPGDataPlugin
Innov8onEPGDataPluginInit(Innov8onEPGDataPlugin self,
                          SvXMLRPCServerInfo serverInfo,
                          SvErrorInfo *errorOut);

/**
 * Set Innov8on user ID.
 *
 * @param[in] self      Innov8on EPG data plugin handle
 * @param[in] userID    user ID
 * @param[out] errorOut error info
 **/
extern void
Innov8onEPGDataPluginSetUserID(Innov8onEPGDataPlugin self,
                               SvString userID,
                               SvErrorInfo *errorOut);

extern void
Innov8onEPGDataPluginSetChannelIdFlag(Innov8onEPGDataPlugin self,
                                      bool external,
                                      SvErrorInfo *errorOut);

/**
 * Set Innov8on XML RPC server info.
 *
 * @param[in] self          Innov8on EPG data plugin handle
 * @param[in] serverInfo    XML RPC server info
 * @param[out] errorOut     error info
 **/
extern void
Innov8onEPGDataPluginSetServerInfo(Innov8onEPGDataPlugin self,
                                   SvXMLRPCServerInfo serverInfo,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif