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

#ifndef LATENSACCESPLUGIN_H_
#define LATENSACCESPLUGIN_H_

/**
 * @file LatensAccessPlugin.h
 * @brief Smartcard-based access plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/QBAccessPlugin.h>
#include <QBNetManager.h>


/**
 * @defgroup LatensAccessPlugin Smartcard-based access plugin class
 * @ingroup QBAccessController
 * @{
 **/

/**
 * Smartcard-based access plugin class.
 * @class LatensAccessPlugin
 * @extends SvObject
 **/
typedef struct LatensAccessPlugin_s *LatensAccessPlugin;


/**
 * Get runtime type identification object
 * representing smartcard-based access plugin class.
 *
 * @return smartcard-based access plugin class
 **/
extern SvType
LatensAccessPlugin_getType(void);

/**
 * Initialize Latens access plugin instance.
 *
 * @memberof LatensAccessPlugin
 *
 * @param[in] self      smartcard-based access plugin handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern LatensAccessPlugin
LatensAccessPluginInit(LatensAccessPlugin self, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* LATENSACCESPLUGIN_H_ */
