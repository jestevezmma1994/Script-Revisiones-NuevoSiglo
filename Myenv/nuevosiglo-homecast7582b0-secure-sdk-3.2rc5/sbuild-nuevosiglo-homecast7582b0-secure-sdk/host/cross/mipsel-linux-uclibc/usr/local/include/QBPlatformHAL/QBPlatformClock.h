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

#ifndef QB_PLATFORM_CLOCK_H_
#define QB_PLATFORM_CLOCK_H_

/**
 * @file QBPlatformClock.h Master clock control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformClock Master clock control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Change clock speed.
 *
 * This function modifies the speed of the master clock.
 * Amount of change is specified in ppm (parts per million) units.
 *
 * @param[in] ppm       amount of change
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformClockCorrectSpeed(int ppm);

/**
 * Get current value of the STC clock.
 *
 * @param[out] stc      value of the STC clock
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformClockGetSTC(uint64_t* stc);

/**
 * Set value of the STC clock.
 *
 * @param[in] stc       new value of the STC clock
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformClockSetSTC(uint64_t stc);

/**
 * Pause or unpause STC clock.
 *
 * @param[in] pause     @c true to pause STC clock, @c false to unpause
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformClockPauseSTC(bool pause);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
