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

/* App/Libraries/QBAccessController/QBAccessPlugin.h */

#ifndef QB_ACCESS_PLUGIN_H_
#define QB_ACCESS_PLUGIN_H_

/**
 * @file QBAccessPlugin.h
 * @brief Access Plugin Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBAccessPlugin Access plugin interface
 * @ingroup QBAccessController
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <Services/QBAccessController/QBAccessManager.h>


/**
 * Get runtime type identification object representing
 * QBAccessPlugin interface.
 * @return Access plugin interface identification object
 **/
extern SvInterface
QBAccessPlugin_getInterface(void);


/**
 * QBAccessPlugin interface.
 **/
typedef struct QBAccessPlugin_t *QBAccessPlugin;
struct QBAccessPlugin_t {
   /**
    * Return plugin name.
    *
    * @param[in] self_     plugin handle
    * @return              plugin name, @c NULL on error
    **/
   SvString (*getName)(SvGenericObject self_);

   /**
    * Setup access plugin for work with access manager.
    *
    * @param[in] self_     plugin handle
    * @param[in] manager   access manager handle, @c NULL to signal that
    *                      plugin will be no longer used
    * @param[in] domain    plugin domain
    * @param[out] errorOut error info
    **/
   void (*setup)(SvGenericObject self_,
                 QBAccessManager manager,
                 SvString domain,
                 SvErrorInfo *errorOut);

   /**
    * Start plugin tasks.
    *
    * @param[in] self_     plugin handle
    * @param[in] scheduler handle to the scheduler that should be used
    *                      by the plugin if needed
    * @param[out] errorOut error info
    **/
   void (*start)(SvGenericObject self_,
                 SvScheduler scheduler,
                 SvErrorInfo *errorOut);

   /**
    * Stop EPG plugin tasks.
    *
    * @param[in] self_     plugin handle
    * @param[out] errorOut error info
    **/
   void (*stop)(SvGenericObject self_,
                SvErrorInfo *errorOut);

   /**
    * Get authentication parameters.
    *
    * @param[in] self_     plugin handle
    * @param[out] params   authentication parameters
    * @param[out] errorOut error info
    **/
   void (*getAuthParams)(SvGenericObject self_,
                         SvString domain,
                         QBAuthParams *params,
                         SvErrorInfo *errorOut);

   /**
    * Start authentication of a user.
    *
    * @param[in] self_     plugin handle
    * @param[in] domain    access control domain name
    * @param[in] password  user password
    * @param[in] context   additional data for authentication, NULL if not used
    * @param[out] errorOut error info
    * @return              authentication status
    **/
   QBAuthStatus (*authenticate)(SvGenericObject self_,
                                SvString domain,
                                SvString password,
                                SvGenericObject context,
                                SvErrorInfo *errorOut);

   /**
    * Start authentication of a user.
    *
    * @param[in] self_     plugin handle
    * @param[in] domain    access control domain name
    * @param[in] password  user password
    * @param[in] newPassword  new user password
    * @param[out] errorOut error info
    * @return              authentication status
    **/
   QBPasswordStatus (*changePassword)(SvGenericObject self_,
                                SvString domain,
                                SvString password,
                                SvString newPassword,
                                SvErrorInfo *errorOut);

   /**
    * Reset your password with a new one.
    *
    * @param[in] self_          plugin handle
    * @param[in] domain         access control domain name
    * @param[in] newPassword    new user Password
    * @param[out] errorOut      error info
    * @return                   result status
    **/
   QBPasswordStatus (*resetPassword)(SvGenericObject self_,
                                     SvString domain,
                                     SvString newPassword,
                                     SvErrorInfo *errorOut);

   /**
    * Cancel asynchronous authentication.
    *
    * @param[in] self_     plugin handle
    * @param[out] errorOut error info
    **/
   void (*cancelAuthentication)(SvGenericObject self_,
                                SvErrorInfo *errorOut);

   /**
    * Cancel asynchronous password change.
    *
    * @param[in] self_     plugin handle
    * @param[out] errorOut error info
    **/
   void (*cancelPasswordChange)(SvGenericObject self_,
                                SvErrorInfo *errorOut);

   /**
    * Check access to given access control domain.
    *
    * @param[in] self_     plugin handle
    * @param[in] domain    access control domain name
    * @return              access code
    **/
   QBAccessCode (*checkAccess)(SvGenericObject self_,
                               SvString domain);

   /**
    * Check status of current authenticaton process.
    *
    * @param[in] self_      plugin handle
    * @param[in] domain     access control domain name
    * @return authentication status
    **/
   QBAuthStatus (*checkAuthentication)(SvGenericObject self_,
                             SvString domain);

   /**
    * Check status of current password change process.
    *
    * @param[in] self_      plugin handle
    * @param[in] domain     access control domain name
    * @return password change status
    **/
   QBPasswordStatus (*checkPasswordChange)(SvGenericObject self_,
                             SvString domain);

};


/**
 * @}
 **/

#endif
