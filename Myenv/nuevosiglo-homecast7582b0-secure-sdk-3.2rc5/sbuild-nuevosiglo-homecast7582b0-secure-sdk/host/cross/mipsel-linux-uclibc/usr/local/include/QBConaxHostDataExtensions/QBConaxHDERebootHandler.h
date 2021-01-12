/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CONAX_HDE_REBOOT_HANDLER
#define QB_CONAX_HDE_REBOOT_HANDLER

/**
 * @file QBConaxHDERebootHandler.h Host Data Extensions handler for rebooting the box
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBConaxHDERebootHandler box rebooting Host Data Extensions handler
 * @ingroup CubiTV_services
 * @{
 */

#include <SvFoundation/SvString.h>
#include <SvCore/SvErrorInfo.h>

typedef struct QBConaxHDERebootHandler_t* QBConaxHDERebootHandler;

/**
 * Function responsible for invoking reboot
 * @param self_ QBCustomEMMHandler handler
 * @param reason reboot reason
 */
typedef void (*QBConaxHDERebootFunction)(SvObject self_, const char *reason);

/**
 * function for creating box rebooting handler object
 * @param[in] emmHandler EMM handler
 * @param[in] rebootFunction pointer to reboot function
 * @param[in] rebootHandlerFilePath path to file in which reboot info will be stored
 * @param[out] errorOut allocation error output
 */
QBConaxHDERebootHandler QBConaxHDERebootHandlerCreate(SvObject emmHandler, QBConaxHDERebootFunction rebootFunction, SvString rebootHandlerFilePath, SvErrorInfo* erroOut);

/**
 * @}
 **/

#endif //QB_CONAX_HDE_REBOOT_HANDLER
