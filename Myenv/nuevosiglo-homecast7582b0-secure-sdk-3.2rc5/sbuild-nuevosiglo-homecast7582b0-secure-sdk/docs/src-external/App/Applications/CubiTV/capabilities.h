/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef CUBITV_CAPABILITIES_H_
#define CUBITV_CAPABILITIES_H_

/**
 * @file capabilities.h Controlling CubiTV process capabilites
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <sys/types.h>


/**
 * @defgroup CubiTV_capabilities Controlling CubiTV process capabilites
 * @ingroup CubiTV_utils
 **/

/**
 * Reduce process capabilities to minimum needed for CubiTV to work.
 *
 * This function changes user ID and group ID (all three of them: real, effective
 * and saved) of current process, keeping few selected capabilites:
 *  - CAP_SYS_BOOT: the right to use reboot() syscall,
 *  - CAP_SYS_TIME: the right to change system time with settimeofday() and similar syscalls,
 *  - CAP_SYS_NICE: the right to control process priorities and scheduling policy,
 *  - CAP_MKNOD: the right to create device nodes using mknod() syscall.
 *
 * @param[in] uid       new user ID
 * @param[in] gid       new group ID
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int QBDropRootPrivileges(uid_t uid, gid_t gid);

/**
 * Drop CAP_MKNOD capability.
 *
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int QBDropMkNodPrivileges(void);

/**
 * @}
 **/


#endif
