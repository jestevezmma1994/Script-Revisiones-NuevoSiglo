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

#ifndef QB_INPUT_SERVICE_H_
#define QB_INPUT_SERVICE_H_

/**
 * @file QBInputService.h
 * @brief Input service class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <fibers/c/fibers.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <QBInput/QBInputEvent.h>
#include <QBInput/QBInputQueue.h>
#include <QBInput/QBInputDevice.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputService Input service singleton class
 * @ingroup QBInputCore
 * @{
 **/

/**
 * Initalize QBInputService singleton.
 *
 * @param[out] errorOut error info
 **/
extern void QBInputServiceInitialize(SvErrorInfo *errorOut);

/**
 * Deinitialize QBInputService singleton.
 **/
extern void QBInputServiceCleanup(void);

/**
 * Stop QBInputService singleton.
 *
 * @param[out] errorOut error info
 **/
extern void QBInputServiceStop(SvErrorInfo *errorOut);

/**
 * Start QBInputService singleton.
 *
 * @param[in] scheduler handle to the scheduler that should be used
 *                      by the input service (@c NULL to use default one)
 * @param[out] errorOut error info
 **/
extern void QBInputServiceStart(SvScheduler scheduler,
                                SvErrorInfo *errorOut);

/**
 * Check if input service was started.
 *
 * @param[out] errorOut error info
 * @return              @c true if input service is running, @c false otherwise
 **/
extern bool QBInputServiceIsRunning(SvErrorInfo *errorOut);

/**
 * Get handle to a queue of input events produced by the input service.
 *
 * @param[out] errorOut error info
 * @return              handle to an input events queue
 **/
extern QBInputQueue QBInputServiceGetOutputQueue(SvErrorInfo *errorOut);

/**
 * Get time stamp of last input event received from any of the devices.
 *
 * This method returns time stamp of the last received input event.
 * Iff no input event was received, returned value equals zero;
 * to check it, compare with value returned by SvTimeGetZero().
 *
 * @param[out] errorOut error info
 * @return              timestamp of the last input event
 **/
extern SvTime QBInputServiceGetLastEventTime(SvErrorInfo *errorOut);

/**
 * List all registered input drivers.
 *
 * This method returns newly created array of all registered
 * input drivers.
 *
 * @note Caller is responsible for releasing returned array.
 *
 * @param[out] errorOut error info
 * @return              an array of input drivers, @c NULL on error
 **/
extern SvImmutableArray QBInputServiceListDrivers(SvErrorInfo *errorOut);

/**
 * List all registered input devices.
 *
 * This method returns newly created array of all registered
 * input devices.
 *
 * @note Caller is responsible for releasing returned array.
 *
 * @param[out] errorOut error info
 * @return              an array of input devices, @c NULL on error
 **/
extern SvImmutableArray QBInputServiceListDevices(SvErrorInfo *errorOut);

/**
 * Find input device with given device ID.
 *
 * @param[in] ID        unique device ID
 * @param[out] errorOut error info
 * @return              input device with given @a ID, @c NULL if not found
 **/
extern SvObject QBInputServiceGetDeviceByID(QBInputDeviceID ID,
                                            SvErrorInfo *errorOut);

/**
 * List all global input event filters.
 *
 * This method returns newly created array of all input filters
 * operating on the common input events queue, that collects input
 * events from all devices. Filters are returned in the same order
 * they are called to process events.
 *
 * @note Caller is responsible for releasing returned array.
 *
 * @param[out] errorOut error info
 * @return              an array of input filters, @c NULL on error
 **/
extern SvImmutableArray QBInputServiceListGlobalFilters(SvErrorInfo *errorOut);

/**
 * List input event filters for a particular input device.
 *
 * This method returns newly created array of all input filters operating
 * on the input events queue filled by a device with given @a deviceID.
 * Filters are returned in the same order they are called to process events.
 *
 * @note Caller is responsible for releasing returned array.
 *
 * @param[in] deviceID  unique input device ID
 * @param[out] errorOut error info
 * @return              an array of input filters, @c NULL on error
 **/
extern SvImmutableArray QBInputServiceListFiltersForDevice(QBInputDeviceID deviceID,
                                                           SvErrorInfo *errorOut);

/**
 * Add new service listener.
 *
 * @param[in] listener  handle to an object implementing
 *                      QBInputServiceListener interface
 * @param[out] errorOut error info
 **/
extern void QBInputServiceAddListener(SvObject listener,
                                      SvErrorInfo *errorOut);

/**
 * Remove previously added service listener.
 *
 * @param[in] listener  handle to the listener registered using
 *                      QBInputServiceAddListener()
 * @param[out] errorOut error info
 **/
extern void QBInputServiceRemoveListener(SvObject listener,
                                         SvErrorInfo *errorOut);

/**
 * Register new input driver.
 *
 * This method adds new driver to the pool of drivers managed by
 * the input service. In case of success, QBInputDriver::checkDevices()
 * will be called on driver object just after the registration.
 *
 * @param[in] driver    handle to an object implementing QBInputDriver interface
 * @param[out] errorOut error info
 **/
extern void QBInputServiceRegisterDriver(SvObject driver,
                                         SvErrorInfo *errorOut);


/**
 * Generate new unique input device ID.
 *
 * @return              new input device ID
 **/
extern QBInputDeviceID QBInputServiceGenerateDeviceID(void);

/**
 * Add new input device.
 *
 * This method adds new device to the pool of input devices handled by
 * the input service. After the registration, QBInputDevice::performIO()
 * method will be called once with mask == ::QBInputDevice_setup, enabling
 * the device to call QBInputServiceSetupDeviceIO() with correct settings.
 *
 * @param[in] device    handle to an object implementing QBInputDevice interface
 * @param[out] errorOut error info
 **/
extern void QBInputServiceAddDevice(SvObject device,
                                    SvErrorInfo *errorOut);

/**
 * Remove existing input device.
 *
 * @param[in] deviceID  unique input device ID
 * @param[out] errorOut error info
 **/
extern void QBInputServiceRemoveDevice(QBInputDeviceID deviceID,
                                       SvErrorInfo *errorOut);

/**
 * Setup I/O configuration of the input device.
 *
 * @param[in] deviceID  unique input device ID
 * @param[in] inFd      file descriptor device reads from (@c -1 to disable)
 * @param[in] outFd     file descriptor device writes to (@c -1 to disable)
 * @param[in] pollPeriod polling period in microseconds (@c 0 to disable polling)
 * @param[out] errorOut error info
 **/
extern void QBInputServiceSetupDeviceIO(QBInputDeviceID deviceID,
                                        int inFd, int outFd,
                                        unsigned int pollPeriod,
                                        SvErrorInfo *errorOut);

/**
 * Trigger I/O on the input device.
 *
 * @param[in] deviceID  unique input device ID
 * @param[out] errorOut error info
 **/
extern void QBInputServiceTriggerIO(QBInputDeviceID deviceID,
                                    SvErrorInfo *errorOut);

/**
 * Add new global input events filter.
 *
 * @param[in] filter    handle to an object implementing QBInputFilter interface
 * @param[in] nextFilter handle to an already registered filter, that should be
 *                      called just after @a filter (@c NULL to add @a filter as last one)
 * @param[out] errorOut error info
 **/
extern void QBInputServiceAddGlobalFilter(SvObject filter,
                                          SvObject nextFilter,
                                          SvErrorInfo *errorOut);

/**
 * Add new input events filter for a particular input device.
 *
 * @param[in] deviceID  unique input device ID
 * @param[in] filter    handle to an object implementing QBInputFilter interface
 * @param[in] nextFilter handle to an already registered filter, that should be
 *                      called just after @a filter (@c NULL to add @a filter as last one)
 * @param[out] errorOut error info
 **/
extern void QBInputServiceAddFilterForDevice(QBInputDeviceID deviceID,
                                             SvObject filter,
                                             SvObject nextFilter,
                                             SvErrorInfo *errorOut);

/**
 * Remove input events filter.
 *
 * @param[in] filter    handle to an object implementing QBInputFilter interface
 * @param[out] errorOut error info
 **/
extern void QBInputServiceRemoveFilter(SvObject filter,
                                       SvErrorInfo *errorOut);


/**
 * Remove input events filter for a single device.
 *
 * @param[in] deviceID  unique input device ID
 * @param[in] filter    handle to an object implementing QBInputFilter interface
 * @param[out] errorOut error info
 **/
extern void QBInputServiceRemoveFilterForDevice(QBInputDeviceID deviceID,
                                                SvObject filter,
                                                SvErrorInfo *errorOut);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
