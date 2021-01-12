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

#ifndef QB_INPUT_EVENT_PLAYER_DEVICE_H_
#define QB_INPUT_EVENT_PLAYER_DEVICE_H_

/**
 * @file QBEventPlayerDevice.h
 * @brief Input event player device class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBEventPlayerDevice Input event player device class
 * @ingroup QBInputDrivers
 * @{
 **/

/**
 * Input event player device class.
 * @class QBEventPlayerDevice
 * @extends SvObject
 **/
typedef struct QBEventPlayerDevice_ *QBEventPlayerDevice;


/**
 * Get runtime type identification object representing
 * input event player device class.
 *
 * @return input event player device class
 **/
extern SvType
QBEventPlayerDevice_getType(void);

/**
 * Initialize input event player device.
 *
 * @memberof QBEventPlayerDevice
 *
 * @param[in] self      input event player device handle
 * @param[in] filePath  input file path
 * @param[in] eventsMask binary mask of input event types to play
 * @param[in] loop      @c true to play in loop
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBEventPlayerDevice
QBEventPlayerDeviceInitWithFile(QBEventPlayerDevice self,
                                const char *filePath,
                                unsigned int eventsMask,
                                bool loop,
                                SvErrorInfo *errorOut);


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
