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

/* App/Libraries/QBAccessController/QBAccessControllerTypes.h */

#ifndef QB_ACCESS_CONTROLLER_TYPES_H_
#define QB_ACCESS_CONTROLLER_TYPES_H_

/**
 * @file QBAccessControllerTypes.h
 * @brief Access Controller Data Types.
 **/

/**
 * @addtogroup QBAccessController
 * @{
 **/


/**
 * Access control codes.
 **/
typedef enum {
   QBAccess_granted    = 0,
   QBAccess_authenticationNotRequired = 0xfc,
   QBAccess_confirmationRequired   = 0xfd,
   QBAccess_authenticationRequired = 0xfe,
   QBAccess_denied     = 0xff
} QBAccessCode;

/**
 * Authentication status.
 **/
typedef enum {
   /// authentication status can not be determined
   QBAuthStatus_unknown = 0xff,
   /// authentication successful
   QBAuthStatus_OK = 0,
   /// authentication failed (display PIN dialog again)
   QBAuthStatus_invalid,
   QBAuthStatus_failed,
   /// authentication in progress
   QBAuthStatus_inProgress,
   /// authentication failed (do not display PIN dialog again)
   QBAuthStatus_rejected,
   /// authentication successful, but user is not authorised
   QBAuthStatus_notAuthorised,
   /// authentication failed (PIN is blocked)
   QBAuthStatus_blocked,
} QBAuthStatus;

/**
 * Password change status
 */
typedef enum {
   /// pasword change status can not be determined
   QBPasswordStatus_unknown,
   /// password chage successful
   QBPasswordStatus_OK,
   /// old password was invalid
   QBPasswordStatus_invalid,
   /// password change has failed
   QBPasswordStatus_failed,
   /// password change is in progress
   QBPasswordStatus_inProgress
} QBPasswordStatus;

/**
 * Authentication parameters.
 **/
typedef struct {
   struct {
      unsigned int minLength;
      unsigned int maxLength;
   } accountID;
   struct {
      unsigned int minLength;
      unsigned int maxLength;
   } password;
   unsigned int maxTries;
} QBAuthParams;


/**
 * @}
 **/

#endif
