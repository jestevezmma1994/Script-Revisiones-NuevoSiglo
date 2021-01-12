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

#ifndef ZRCPROFILE_HAL_H_
#define ZRCPROFILE_HAL_H_

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ZRCProfileHAL.h ZRCProfileHAL
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 *
 **/

/**
 * @defgroup ZRCProfileHAL ZRCProfileHAL HAL interface
 * @ingroup QBRf4ce
 * @{
 **/

/**
 * Hardware capabilities.
 **/
typedef struct {
    /**
     * @c true if hardware is present in the STB, @c false otherwise
     **/
    bool isHWPresent;
} ZRCProfileHALCapabilities;

/**
 * event type
 */
typedef enum ZRCProfileHALEventType_ {
    ZRCProfileHALEventType_keyDown = 1,   /**< key pressed */
    ZRCProfileHALEventType_keyRepeat = 2, /**< key repeat */
    ZRCProfileHALEventType_keyUp = 3,     /**< key released */
} ZRCProfileHALEventType;

/**
 * ZRC Profile device object
 *
 * Representing paired devices
 */
typedef struct ZRCProfileHALDevice_ {
    struct SvObject_ super_;
    uint8_t address[8];    /**< physical device's address */
    int devId;             /**< position id in device's pairing table */
    unsigned int vendorId; /**< device's vendor identification number */
} *ZRCProfileHALDevice;

/**
 * ZRC Profile key's events
 */
typedef struct ZRCProfileKeyEvent_ {
    int keyCode;                      /**< button key code */
    ZRCProfileHALEventType eventType; /**< event type (pressed, released, repeat) */
    ZRCProfileHALDevice device;       /**< ZRC device */
} ZRCProfileHALKeyEvent;

/**
 * Pairing callback function type
 * @param[in] callbackData arbitrary parameter for specific callback implementation use
 * @param[in] dev paired device
 */
typedef void (*devicePairedCallback)(void *callbackData, ZRCProfileHALDevice dev);

/**
 * Initialize ZRCProfileHAL module
 * @param[in] void
 * @return ZRC device SvType
 */
extern SvType ZRCProfileHALDevice_getType(void);

/**
 * Get ZRCProfileHAL capabilities.
 * @return capabilities
 **/
extern ZRCProfileHALCapabilities ZRCProfileHALGetCapabilities(void);

/**
 * Probe ZRC device module
 * @param[in] void
 * @return '0' on success '-1' in case of error
 */
extern int ZRCProfileHALProbe(void);

/**
 * Start pairing procedure
 * @param[in] callback pairing device callback method
 * @param[in] callbackData a parameter to be passed to the @param callback - possiby a self pointer
 * @return '0' on success '-1' in case of error
 */
extern int ZRCProfileHALStartPairing(devicePairedCallback callback, void *callbackData);

/**
 * Get paired devices array
 * @param[in] pointer to SvArray which will contain pairing devices list.
 *            If it points to @c NULL, a new array will be created.
 * @return '0' on success '-1' in case of error
 */
extern int ZRCProfileHALGetPairedDevices(SvArray *deviceList);

/**
 * Remove device from paired device's table
 * @param[in] device for remove
 * @return '0' on success '-1' in case of error
 */
extern int ZRCProfileHALUnpairDevice(ZRCProfileHALDevice device);

/**
 * Get ZRC Profile key event
 * @param[in] void
 * @return ZRCProfileHALKeyEvent
 */
extern ZRCProfileHALKeyEvent ZRCProfileHALGetZRCProfileKeyEvent(void);

/**
 * Get number of events
 * @param[in] void
 * @return number of registered events
 */
extern int ZRCProfileHALGetEventNumbers(void);

/**
 * Initialize ZRC Profile HAL
 * @param[in] void
 * @return '0' on success '-1' in case of error
 */
extern int ZRCProfileHALInit(void);

/**
 * Deinitialize ZRC Profile HAL
 * @param[in] void
 * @return '0' on success '-1' in case of error
 */
extern int ZRCProfileHALDeinit(void);

#ifdef __cplusplus
}
#endif

#endif /*ZRCPROFILE_HAL_H_*/
