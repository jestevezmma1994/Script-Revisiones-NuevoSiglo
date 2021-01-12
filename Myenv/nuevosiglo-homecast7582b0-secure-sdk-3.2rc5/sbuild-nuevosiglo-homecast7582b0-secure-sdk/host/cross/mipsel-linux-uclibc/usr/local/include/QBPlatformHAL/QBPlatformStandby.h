/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_PLATFORM_STANDBY_H_
#define QB_PLATFORM_STANDBY_H_

/**
 * @file QBPlatformStandby.h Standby mode control API
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformStandby Standby mode control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Get standby capabilities
 *
 * @return      bitwise mask of QBStandbyCapability_* values
 **/

extern int
QBPlatformGetStandbyCapabilities(void);

/**
 * Switch the device into standby mode.
 *
 * FIXME: this function should return error code if platform
 * does not support standby! We should also have another
 * function to check if standby mode is supported.
 *
 * @note On success this function never returns.
 *
 * @param[in] wakeupDelay   time (in seconds) after which the device
 *                          power will be restored
 **/
extern void
QBPlatformSwitchToStandby(unsigned int wakeupDelay);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
