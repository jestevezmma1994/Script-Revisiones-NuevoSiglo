/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RCU_DEVICE_H_
#define QB_RCU_DEVICE_H_

/**
 * @file QBRCUDevice.h
 * @brief RCU input device class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBAppKit/QBPropertiesMap.h>
#include <QBInput/QBInputEvent.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBRCUDevice RCU input device class
 * @ingroup QBInputDrivers
 * @{
 **/

/**
 * RCU input device class.
 * @class QBRCUDevice
 * @extends SvObject
 **/
typedef struct QBRCUDevice_ *QBRCUDevice;


/**
 * Get runtime type identification object representing
 * RCU input device class.
 *
 * @return RCU input device class
 **/
extern SvType
QBRCUDevice_getType(void);

/**
 * Initialize RCU input device.
 *
 * @memberof QBRCUDevice
 *
 * @param[in] self      RCU input device handle
 * @param[in] name      device name
 * @param[in] address   RCU device address
 * @param[in] parentProperties properties of parent device (will be copied
 *                      to this device), @c NULL if device has no parent
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBRCUDevice
QBRCUDeviceInit(QBRCUDevice self,
                SvString name,
                unsigned int address,
                QBPropertiesMap parentProperties,
                SvErrorInfo *errorOut);

/**
 * Get RCU device address.
 *
 * @memberof QBRCUDevice
 *
 * @param[in] self      RCU input device handle
 * @return              device address as reported by RCU
 **/
extern unsigned int
QBRCUDeviceGetDeviceAddress(QBRCUDevice self);

/**
 * Handle RCU command.
 *
 * @memberof QBRCUDevice
 *
 * @param[in] self      RCU input device handle
 * @param[in] type      RCU button press event type
 * @param[in] cmd       raw RCU command
 **/
extern void
QBRCUDeviceHandleCommand(QBRCUDevice self,
                         QBInputEventType type,
                         unsigned int cmd);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
