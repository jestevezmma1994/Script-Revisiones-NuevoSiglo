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

#ifndef QBVRCDEVICE_H_
#define QBVRCDEVICE_H_

/**
 * @file  QBVRCDevice.h
 * @brief Virtual RC input device class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBVRCDevice Virtual RC input device class
 * @ingroup  QBInputDrivers
 * @{
 **/

/**
 * Virtual RC input device class.
 * @class QBVRCDevice
 * @extends SvObject
 **/
typedef struct QBVRCDevice_ * QBVRCDevice;

/**
 * Get runtime type identification object representing
 * virtual RC input device class.
 *
 * @return virtual RC input device class
 **/
extern SvType
QBVRCDevice_getType(void);

/**
 * Initialize virtual RC input device.
 *
 * @memberof QBVRCDevice
 *
 * @param[in] self              virtual RC input device handle
 * @param[in] name              device name
 * @param[out] errorOut         error info
 * @return                      @a self or @c NULL in case of error
 **/
extern QBVRCDevice
QBVRCDeviceInit(QBVRCDevice self,
                SvString name,
                SvErrorInfo *errorOut);

/**
 * Handle virtual RC command.
 *
 * @memberof QBVRCDevice
 *
 * @param[in] self          virtual RC input device handle
 * @param[in] command       high-level input command for which will be
 *                          created input event with type keyTyped
 **/
extern void
QBVRCDeviceHandleCommand(QBVRCDevice self,
                         unsigned int command);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QBVRCDEVICE_H_ */
