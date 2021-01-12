/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_THREAD_SUPPORT_H_
#define SV_THREAD_SUPPORT_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file SvThreadSupport.h
 * @brief Additional utilities for system threads
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvThreadSupport Additional utilities for system threads
 * @ingroup SvQuirks
 * @{
 **/

/**
 * Set name of the calling thread.
 *
 * @param[in] name      new name for current (calling) thread
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
SvSetThreadName(const char *name);

/**
 * Get thread stack size. On some platforms the minimal size of thread stack (PTHREAD_STACK_MIN)
 * is higher than we want to request and passing actual wanted value (lower than PTHREAD_STACK_MIN)
 * to pthread_attr_setstacksize() causes this function to fail.
 * Also it prevents making huge stacks - we made MAX 16MB.
 *
 * @param [in] minStackSize of the stack which we wish to have
 *
 * @return size of the stack adjusted to platform.
 **/
extern size_t
SvGetThreadStackSize(size_t minStackSize);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
