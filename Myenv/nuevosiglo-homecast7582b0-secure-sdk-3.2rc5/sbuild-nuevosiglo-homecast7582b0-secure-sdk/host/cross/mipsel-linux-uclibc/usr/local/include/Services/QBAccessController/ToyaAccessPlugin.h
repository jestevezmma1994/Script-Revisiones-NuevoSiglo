/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef TOYA_ACCESS_PLUGIN_H_
#define TOYA_ACCESS_PLUGIN_H_

/**
 * @file ToyaAccessPlugin.h
 * @brief Toya access plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/QBAccessPlugin.h>
#include <QBNetManager.h>


/**
 * @defgroup ToyaAccessPlugin Toya access plugin class
 * @ingroup QBAccessController
 * @{
 **/

/**
 * Toya access plugin class.
 * @class ToyaAccessPlugin
 * @extends SvObject
 **/
typedef struct ToyaAccessPlugin_t *ToyaAccessPlugin;


/**
 * Get runtime type identification object
 * representing Toya access plugin class.
 *
 * @return Toya access plugin class
 **/
extern SvType
ToyaAccessPlugin_getType(void);

/**
 * Initialize Toya access plugin instance.
 *
 * @memberof ToyaAccessPlugin
 *
 * @param[in] self      Toya access plugin handle
 * @param[in] net_manager Network Manager instance
 * @param[in] timeout   in seconds, how long should an authentication be valid
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern ToyaAccessPlugin
ToyaAccessPluginInit(ToyaAccessPlugin self,
                     QBNetManager net_manager,
                     int timeout,
                     SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
