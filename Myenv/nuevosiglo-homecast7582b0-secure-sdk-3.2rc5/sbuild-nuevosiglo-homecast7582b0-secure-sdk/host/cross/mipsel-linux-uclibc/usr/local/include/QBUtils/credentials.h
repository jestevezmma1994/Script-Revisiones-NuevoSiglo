/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_UTILS_CREDENTIALS_H_
#define QB_UTILS_CREDENTIALS_H_

/**
 * @file credentials.h Utilities for controlling process credentials
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <sys/types.h>
#ifdef __linux__
# include <sys/capability.h>
#endif


/**
 * @defgroup QBUtils_credentials Controlling process credentials
 * @ingroup QBUtils
 * @{
 **/

/**
 * Write information about current process credentials to SvLog channel.
 *
 * @param[in] logPrefix prefix prepended to every logged line
 **/
extern void
QBUtilsPrintCredentials(const char *logPrefix);

#ifdef __linux__

/**
 * Change current thread credentials.
 *
 * This function changes credentials of the calling thread to the specified
 * values. You can specify new user ID and group ID as well as a list of
 * supplementary groups for the thread.
 *
 * When user ID changes from 0 to any other value, system will by default
 * clear all three sets of capabilities of the calling thread. This function
 * allows you to select capabilities that you want to keep (in permissive and
 * effective sets only) after changing user ID. You can also use this function
 * to drop individual capabilities when they are not needed anymore: in such
 * case pass @c -1 as @a uid and @a gid.
 *
 * @warning Please remember that credentials are thread-specific,
 * they are not process-wise! This function should be called as soon as
 * possible, before any threads are created, otherwise they will work with
 * original credentials of the process' main thread.
 *
 * @param[in] logPrefix     prefix prepended to every logged message
 * @param[in] uid           new user ID, @c -1 to keep current one
 * @param[in] gid           new group ID, @c -1 to keep current one
 * @param[in] supplementaryGroups an array of supplementary group identifiers
 * @param[in] supplementaryGroupsCount number of groups in @a supplementaryGroups array
 * @param[in] capabilities  an array of capabilities to retain after this operation
 * @param[in] capabilitiesCount number of capabilities in @a capabilities array
 * @return                  @c 0 on success, @c -1 in case of error
 **/
extern int
QBUtilsSetThreadCredentials(const char *logPrefix,
                            const uid_t uid,
                            const gid_t gid,
                            const gid_t *supplementaryGroups,
                            const unsigned int supplementaryGroupsCount,
                            const cap_value_t *capabilities,
                            const unsigned int capabilitiesCount);

#endif // __linux__

/**
 * @}
 **/


#endif
