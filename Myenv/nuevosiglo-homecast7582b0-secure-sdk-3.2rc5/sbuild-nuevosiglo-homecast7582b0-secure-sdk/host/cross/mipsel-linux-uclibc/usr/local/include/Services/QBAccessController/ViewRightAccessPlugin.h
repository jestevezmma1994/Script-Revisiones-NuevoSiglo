/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef VIEWRIGHTACCESPLUGIN_H_
#define VIEWRIGHTACCESPLUGIN_H_

/**
 * @file ViewRightAccessPlugin.h
 * @brief ViewRight access plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/QBAccessPlugin.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>

/**
 * @defgroup ViewRightAccessPlugin ViewRight access plugin class
 * @ingroup QBAccessController
 * @{
 **/

/**
 * ViewRight access plugin class.
 * @class ViewRightAccessPlugin
 * @extends SvObject
 **/
typedef struct ViewRightAccessPlugin_t *ViewRightAccessPlugin;


/**
 * Get runtime type identification object
 * representing ViewRight access plugin class.
 *
 * @return ViewRight access plugin class
 **/
extern SvType
ViewRightAccessPlugin_getType(void);

/**
 * Initialize ViewRight access plugin instance.
 *
 * @memberof ViewRightAccessPlugin
 *
 * @param[in] self      ViewRight access plugin handle
 * @param[in] timeout   in seconds, how long should an authentication be valid
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern ViewRightAccessPlugin
ViewRightAccessPluginInit(ViewRightAccessPlugin self,
                          int timeout,
                          SvErrorInfo *errorOut);

/**
 * @brief Add listener
 * @param[in] self      ViewRight access plugin handle
 * @param[in] listener  instance of object which implements ViewRightAccessPluginListener interface
 */
extern void
ViewRightAccessPluginAddListener(ViewRightAccessPlugin self, SvObject listener);

/**
 * @brief Remove listener
 * @param[in] self      ViewRight access plugin handle
 * @param[in] listener  instance of object which implements ViewRightAccessPluginListener interface
 */
extern void
ViewRightAccessPluginRemoveListener(ViewRightAccessPlugin self, SvObject listener);

/**
 * @}
 **/

#endif /* VIEWRIGHTACCESPLUGIN_H_ */
