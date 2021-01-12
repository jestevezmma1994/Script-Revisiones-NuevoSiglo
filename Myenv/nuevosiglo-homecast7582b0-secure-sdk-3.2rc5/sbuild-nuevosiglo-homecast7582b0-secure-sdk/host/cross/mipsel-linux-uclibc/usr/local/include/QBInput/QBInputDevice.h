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

#ifndef QB_INPUT_DEVICE_H_
#define QB_INPUT_DEVICE_H_

/**
 * @file QBInputDevice.h
 * @brief Input device interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBInput/QBInputQueue.h>
#include <QBAppKit/QBPropertiesMap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputDevice Input device interface
 * @ingroup QBInputCore
 * @{
 **/

/**
 * Device ID type (@c 0 means invalid value).
 **/
typedef unsigned int QBInputDeviceID;

/**
 * Values used in I/O event masks.
 **/
enum {
    /// event signalled once for each new device
    QBInputDevice_setup = 0x01,
    /// signals that driver can read from its file descriptor
    QBInputDevice_canRead = 0x02,
    /// signals that driver can write to its file descriptor
    QBInputDevice_canWrite = 0x04,
    /// signals that next polling period has finished
    QBInputDevice_timeExpired = 0x08,
    /// signals that device has been asked to do its I/O
    QBInputDevice_tryPoll = 0x10
};


/**
 * Get runtime type identification object representing
 * input device interface.
 *
 * @return input device interface
 **/
extern SvInterface QBInputDevice_getInterface(void);


/**
 * Input device interface.
 **/
typedef const struct QBInputDevice_ {
    /**
     * Return unique device ID.
     *
     * @param[in] self_  handle to an object implementing @ref QBInputDevice
     * @return           unique device ID, @c 0 on error
     **/
    QBInputDeviceID (*getID)(SvObject self_);

    /**
     * Return input device name.
     *
     * @param[in] self_  handle to an object implementing @ref QBInputDevice
     * @return           device name, @c NULL on error
     **/
    SvString (*getName)(SvObject self_);

    /**
     * Return input driver that created this device.
     *
     * @param[in] self_  handle to an object implementing @ref QBInputDevice
     * @return           handle to an object implementing @ref QBInputDriver
     *                   or @c NULL if device has not been created by a driver
     **/
    SvObject (*getDriver)(SvObject self_);

    /**
     * Return the mask identifying which event types can be reported
     * by this device.
     *
     * @param[in] self_  handle to an object implementing @ref QBInputDevice
     * @return           bitmask of QBInputEventType_* values
     **/
    unsigned int (*getEventTypes)(SvObject self_);

    /**
     * Check if new input events are available.
     *
     * This method is called once for each device with
     * @a mask == QBInputDevice::QBInputDevice_setup, and then when at least one condition
     * programmed by this device using QBInputServiceSetupDeviceIO() method
     * is true. The @a mask contains values identifying which
     * conditions are true.
     *
     * @param[in] self_  handle to an object implementing @ref QBInputDevice
     * @param[in] mask   bitmask of I/O conditions that are true
     * @param[in] dest   queue for new input events
     **/
    void (*performIO)(SvObject self_,
                      unsigned int mask,
                      QBInputQueue dest);
} *QBInputDevice;


/**
 * Get runtime type identification object representing
 * extended input device interface.
 *
 * @return extended input device interface
 **/
extern SvInterface QBInputDeviceExt_getInterface(void);


/**
 * Extended input device interface.
 **/
typedef const struct QBInputDeviceExt_ {
    /// super interface
    struct QBInputDevice_ super_;

    /**
     * Get input device's properties.
     *
     * @param[in] self_  handle to an object implementing @ref QBInputDeviceExt
     * @return           map of properties of the input device
     **/
    QBPropertiesMap (*getProperties)(SvObject self_);
} *QBInputDeviceExt;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
