/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLATFORM_LED_H_
#define QB_PLATFORM_LED_H_

/**
 * @file QBPlatformLED.h LEDs control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformLED LEDs control
 * @ingroup QBPlatformHAL
 * @{
 **/

/// LED brightness.
enum {
    /// LED turned off
    QBPlatformLEDBrightness_off = 0,
    /// LED turned on with min possible brightness
    QBPlatformLEDBrightness_min = 1,
    /// LED turned on with max possible brightness
    QBPlatformLEDBrightness_max = 255
};


/**
 * Get number of LEDs that can be controlled.
 *
 * @return              number of LEDs, @c -1 in case of error
 **/
extern int
QBPlatformGetLEDsCount(void);

/**
 * Get current LED brightness.
 *
 * @param[in] led       LED number
 * @return              current brightness level, @c -1 in case of error
 **/
extern int
QBPlatformGetLEDBrightness(unsigned int led);

/**
 * Set LED brightness.
 *
 * @param[in] led       LED number
 * @param[in] brightness desired LED brightness level
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformSetLEDBrightness(unsigned int led,
                           unsigned int brightness);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
