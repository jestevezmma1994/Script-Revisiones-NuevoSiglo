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

#ifndef QB_PLATFORM_HDMI_OUTPUT_H_
#define QB_PLATFORM_HDMI_OUTPUT_H_

/**
 * @file QBPlatformHDMIOutput.h HDMI output control API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <QBPlatformHAL/QBPlatformCEC.h>
#include <QBPlatformHAL/QBPlatformEDID.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformHDMIOutput HDMI output control
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Check if there is any device connected to the HDMI port.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @param[out] isConnected @c true if something is connected to the HDMI port
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformHDMIOutputCheckConnection(unsigned int outputID,
                                    bool *isConnected);

/**
 * Get EDID information from the device connected to the HDMI port.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @return              copy of the EDID block (must be freed by the caller),
 *                      @c NULL in case of error
 **/
extern QBEDIDBlock *
QBPlatformHDMIOutputGetEDID(unsigned int outputID);

/**
 * Send CEC message over HDMI port.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @param[in] msg       CEC message to send
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformHDMIOutputSendCECMessage(unsigned int outputID,
                                   const QBCECMessage msg);

/**
 * HDMI hotplug callback function type.
 *
 * @param[in] prv       opaque pointer to callback function's private data
 * @param[in] ID        video output ID of the HDMI output
 * @param[in] isConnected @c true when device was connected to HDMI output,
 *                      @c false when device was disconnected
 **/
typedef void (*QBPlatformHDMIHotplugCallback)(void *prv,
                                              unsigned int outputID,
                                              bool isConnected);

/**
 * Register HDMI hotplug callback function.
 *
 * This function allows registering a callback that will be called
 * when device is connected to or disconnected from the HDMI port.
 *
 * @param[in] outputID  video output ID of the HDMI output
 * @param[in] callback  callback function, @c 0 to deregister
 * @param[in] prv       opaque pointer to callback function's private data
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformHDMIOutputSetHotplugCallback(unsigned int outputID,
                                       QBPlatformHDMIHotplugCallback callback,
                                       void *prv);

/**
 * Change video output mode of the HDMI output if the current mode
 * is not supported by the display device connected to HDMI output.
 *
 * This function reads EDID information from the display device
 * connected to HDMI output to check if current video output mode is supported
 * by this device. If current video output mode is not supported, it changes
 * output mode to the closest one that is supported.
 *
 * @param[in] outputID      video output ID of the HDMI output
 * @return                  @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformHDMIOutputApplyClosestMode(unsigned int outputID);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
