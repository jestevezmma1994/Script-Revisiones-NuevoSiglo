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

/* App/Libraries/QBAccessController/QBAccessManager.h */

#ifndef QB_ACCESS_MANAGER_H_
#define QB_ACCESS_MANAGER_H_

/**
 * @file QBAccessManager.h
 * @brief Access Manager Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBAccessManager Access manager class
 * @ingroup QBAccessController
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <Services/QBAccessController/QBAccessControllerTypes.h>

/**
 * Access manager class
 * @class QBAccessManager QBAccessManager.h <QBAccessController/QBAccessManager.h>
 * @extends SvObject
 * @implements QBAsyncService
 * @implements QBInitializable
 *
 * This class expects to be initialized by @link QBInitializable @endlink
 * interface init method.
 **/
typedef struct QBAccessManager_t *QBAccessManager;


/**
 * Get runtime type identification object representing QBAccessManager class.
 *
 * @relates QBAccessManager
 *
 * @return QBAccessManager runtime type identification object
 **/
extern SvType
QBAccessManager_getType(void);

/**
 * Setup access plugin for given domain.
 *
 * @memberof QBAccessManager
 *
 * This method registers an access plugin in the access manager.
 * Access plugin implements actual access policy.
 *
 * By default, access manager is created with an instance of
 * QBStaticAccessPlugin configured to deny any access rights.
 *
 * @param[in] self          access manager handle
 * @param[in] plugin        handle to an object implementing QBAccessPlugin
 *                          interface
 * @param[in] domainName    plugin's domain name
 * @param[out] errorOut     error info
 **/
extern void
QBAccessManagerSetPluginByDomain(QBAccessManager self,
                                 SvGenericObject plugin,
                                 SvString domainName,
                                 SvErrorInfo *errorOut);

/**
 * Register new access manager listener.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self      access manager handle
 * @param[in] listener  handle to an object implementing
 *                      QBAccessManagerListener interface
 * @param[out] errorOut error info
 **/
extern void
QBAccessManagerAddListener(QBAccessManager self,
                           SvGenericObject listener,
                           SvErrorInfo *errorOut);

/**
 * Remove previously registered access manager listener.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self      access manager handle
 * @param[in] listener  handle to previously registered listener
 * @param[out] errorOut error info
 **/
extern void
QBAccessManagerRemoveListener(QBAccessManager self,
                              SvGenericObject listener,
                              SvErrorInfo *errorOut);

/**
 * Get authentication parameters.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[out] params       authentication parameters
 * @param[in] domainName    access control domain name
 * @param[out] errorOut     error info
 **/
extern void
QBAccessManagerGetAuthParamsByDomain(QBAccessManager self,
                                     QBAuthParams *params,
                                     SvString domainName,
                                     SvErrorInfo *errorOut);


/**
 * Set the validity time for the domain authentication.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[in] seconds       vailidity time
 * @param[out] errorOut     error info
 **/
extern void
QBAccessManagerSetAuthenticationValidityTime(QBAccessManager self,
                                             SvString domainName,
                                             int seconds,
                                             SvErrorInfo *errorOut);

/**
 * Invalidate authentication for the specified domain.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[out] errorOut     error info
 **/
extern void
QBAccessManagerInvalidateAuthentication(QBAccessManager self,
                                        SvString domainName,
                                        SvErrorInfo *errorOut);

/**
 * Invalidate authentication for all domains.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[out] errorOut     error info
 **/
void
QBAccessManagerInvalidateAllAuthentications(QBAccessManager self,
                                            SvErrorInfo *errorOut);
/**
 * Authenticate user.
 *
 * @memberof QBAccessManager
 *
 * This method authenticates user.
 * Authentication of previously authenticated user is revoked.
 * Authentication process can require network access and as such
 * is asynchronous and can take some undefined time to finish.
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[in] password      user password
 * @param[in] context       additional data for authentication, NULL if not used
 * @param[out] errorOut     error info
 * @return                  authentication status
 **/
extern QBAuthStatus
QBAccessManagerAuthenticate(QBAccessManager self,
                            SvString domainName,
                            SvString password,
                            SvGenericObject context,
                            SvErrorInfo *errorOut);

/**
 * Check authentication status.
 *
 * @memberof QBAccessManager
 *
 * This method revokes operations depending on status (OK or invalid).
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    domain that the operation concerns
 * @param[in] status        authentication status
 **/
extern void
QBAccessManagerOnAuthenticationStatusChange(QBAccessManager self,
                                            SvString domainName,
                                            QBAuthStatus status);

/**
 * Change password for user.
 *
 * @memberof QBAccessManager
 *
 * This method changes user's PIN for given access domain.
 * See QBAccessManagerAuthenticate().
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[in] password      current password
 * @param[in] newPassword   new password
 * @param[out] errorOut     error info
 * @return                  password change status
 **/
extern QBPasswordStatus
QBAccessManagerChangePassword(QBAccessManager self,
                              SvString domainName,
                              SvString password,
                              SvString newPassword,
                              SvErrorInfo *errorOut);

/**
 * Revoke current user authentication.
 *
 * @memberof QBAccessManager
 *
 * This method revokes current user authentication. If the authentication
 * process is in progress, it will be cancelled. If the authentication passed nothing happens.
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[out] errorOut     error info
 **/
extern void
QBAccessManagerRevokeAuthenticationByDomain(QBAccessManager self,
                                            SvString domainName,
                                            SvErrorInfo *errorOut);

/**
 * Get access rights to given access control domain.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domain        access control domain
 * @return                  access control code
 **/
extern QBAccessCode
QBAccessManagerCheckAccess(QBAccessManager self,
                           SvString domain);

/**
 * Check status of current authentication process.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[out] errorOut     error info
 * @return                  authentication status
 **/
extern QBAuthStatus
QBAccessManagerCheckAuthentication(QBAccessManager self,
                                   SvString domainName,
                                   SvErrorInfo *errorOut);

/**
 * Get status of authentication for the specified domain.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[out] errorOut     error info
 * @return                  authentication status
 **/
extern QBAuthStatus
QBAccessManagerGetAuthenticationStatus(QBAccessManager self,
                                       SvString domainName,
                                       SvErrorInfo *errorOut);

/**
 * Check status of current password change process.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 * @param[out] errorOut     error info
 * @return                  password change status
 **/
extern QBPasswordStatus
QBAccessManagerCheckPasswordChange(QBAccessManager self,
                                   SvString domainName,
                                   SvErrorInfo *errorOut);

/**
 * Requests password change without providing old password, the answer must be
 * immediate.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] acDomain      access domain for which the password needs to be reset
 * @param[in] newPassword   the password to reset to
 * @param[out] errorOut     error info
 * @return                  password change status
 **/
QBPasswordStatus
QBAccessManagerResetPassword(QBAccessManager self,
                             SvString acDomain,
                             SvString newPassword,
                             SvErrorInfo *errorOut);

/**
 * Access Manager may disallow too many attempts in PIN change. This function
 * returns how many attemps are left. By default all PINs are limited (see
 * QBAccessManagerSetPinAttemptsAsUnlimited()).
 *
 * @memberof QBAccessManager
 *
 * @param[in] self      access manager handle
 * @param[in] domain    access control domain
 * @return              @c -1 for unlimited, @c 0 for blocked,
 *                      greater than @c 0: number of attempts left
 **/
extern int
QBAccessManagerGetPinAttemptsLeft(QBAccessManager self, SvString domain);

/**
 * After pin checking has been blocked it returns for how many more seconds
 * it is blocked.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self    access manager handle
 * @return            amount of seconds before user may retry pin check
 **/
extern int
QBAccessManagerGetBlockPeriodLeft(QBAccessManager self);

/**
 * Forcibly resets the amount of pin attempts, so that the user can immediately
 * start again.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self    access manager handle
 **/
extern void
QBAccessManagerResetPinAttemptsLeft(QBAccessManager self);

/**
 * Requests that a domain does not limit pin attempts.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param[in] domainName    access control domain name
 **/
extern void
QBAccessManagerSetPinAttemptsAsUnlimited(QBAccessManager self,
                                         SvString domainName);

/**
 * Get authentication plugin used for given domain.
 *
 * @memberof QBAccessManager
 *
 * @param[in] self          access manager handle
 * @param [in] domainName   access control domain name.
 *                          If @c NULL or @c "DEFAULT" it will return default plugin
 * @return                  handle to an object implementing @ref QBAccessPlugin.
 *                          Returns @c NULL if domainName is unknown
 **/
extern SvGenericObject
QBAccessManagerGetDomainPlugin(QBAccessManager self,
                               SvString domainName);

/**
 * @}
 **/


#endif
