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

#ifndef QB_DEBUG_UTILS_H_
#define QB_DEBUG_UTILS_H_

/**
 * @file QBDebugUtils.h Debug functions for application connected with Generic objects.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>
#include <CUIT/Core/types.h>

/**
 * @brief Register hooks for generic objects. It only works if
 * SV_FOUNDATION_DEBUG was set.
**/
extern void QBDebugUtilsRegisterHooks(void);

/** @brief Number of milliseconds between two call of a fiber which print information about all generic objects*/
#define CUBITV_DEBUG_PRINT_OBJECTS_DELAY (60*1000)

/** @brief Number of milliseconds between two calls of a fiber which print information about all generic objects when allowed time for fiber passed*/
#define CUBITV_DEBUG_PRINT_OBJECTS_DELAY_AFTER_TIME_PASS (250)

/**
 * @brief Creates and starts a fiber which prints information about all generic objects currently stored in memory.
 * It only works if SV_FOUNDATION_DEBUG was set.
 *
 * @param [in] scheduler     handle to a scheduler used by fiber to schedule
 * @param [in] app           SvApplication handle
**/
extern void QBDebugUtilsListObjectsStart(SvScheduler scheduler, SvApplication app);

/**
 * @brief Destroys a fiber which prints information about all generic objects.
**/
extern void QBDebugUtilsListObjectsStop(void);

/**
 * Create and start a fiber which prints information about memory usage globally and per zone.
 *
 * @note To make this feature work set LD_PRELOAD=libzonetrace.so for CubiTV.
 *
 * @param [in] scheduler     handle to a scheduler to be used
 **/
extern void QBDebugUtilsStartMemoryTrace(SvScheduler scheduler);

/**
 * Destroy fiber started by QBDebugUtilsStartMemoryTrace().
 **/
extern void QBDebugUtilsStopMemoryTrace(void);

#endif // QB_DEBUG_UTILS_H_
