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

#ifndef SV_PROCESS_CONTEXT_H_
#define SV_PROCESS_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file SvProcessContext.h
 * @brief Utilities for getting information about current process
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvProcessContext Acquiring information about current process
 * @ingroup SvQuirks
 * @{
 **/

/**
 * Get value of the program counter (address in the code segment) from
 * saved process context.
 *
 * This function can be used in signal handler to get the address
 * of the instruction that caused an error. Saved context is passed
 * to the signal handler when registered using sigaction() with
 * SA_SIGINFO flag set.
 *
 * @param[in] ucontext  saved process context
 * @param[out] PC       program counter value
 * @return              @c 0 on success (@a PC contains valid value),
 *                      @c -1 in case of error
 **/
extern int
SvProcessContextGetPC(const void *ucontext, unsigned long long int *PC);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
