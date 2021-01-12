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

#ifndef QB_STATIC_ACCESS_PLUGIN_H_
#define QB_STATIC_ACCESS_PLUGIN_H_

/**
 * @file QBStaticAccessPlugin.h
 * @brief Static access plugin class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <Services/QBAccessController/QBAccessManager.h>
#include <Services/QBAccessController/QBAccessPlugin.h>


/**
 * @defgroup QBStaticAccessPlugin Static access plugin class
 * @ingroup QBAccessController
 * @{
 **/

/**
 * Static access plugin class.
 * @class QBStaticAccessPlugin
 * @extends SvObject
 **/
typedef struct QBStaticAccessPlugin_t *QBStaticAccessPlugin;


/**
 * Get runtime type identification object
 * representing static access plugin class.
 *
 * @return static access plugin class
 **/
extern SvType
QBStaticAccessPlugin_getType(void);

/**
 * Initialize static access plugin instance.
 *
 * @memberof QBStaticAccessPlugin
 *
 * @param[in] self      static access plugin handle
 * @param[in] allow     @c true to allow access to domains in the list
 *                      and deny to others, @c false to deny access
 *                      to domains in the list and allow to others
 * @param[in] domains   list of access control domain names to allow or
 *                      deny access to, according to @a allow flag
 *                      (@c NULL is treated as an empty list)
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBStaticAccessPlugin
QBStaticAccessPluginInit(QBStaticAccessPlugin self,
                         bool allow,
                         SvArray domains,
                         SvErrorInfo *errorOut);

/**
 * Initialize static access plugin instance.
 *
 * This method works just like QBStaticAccessPluginInit(),
 * but instead of an array of domain names it accepts a string
 * containing comma-separated domain names.
 *
 * @param[in] self      static access plugin handle
 * @param[in] allow     @c true to allow access to all domains in the list
 *                      and deny to others, @c false to deny access
 *                      to domains in the list and allow to others
 * @param[in] domains   list of access control domain names to allow or
 *                      deny access to, see QBStaticAccessPluginInit()
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBStaticAccessPlugin
QBStaticAccessPluginInitWithString(QBStaticAccessPlugin self,
                                   bool allow,
                                   const char *domains,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
