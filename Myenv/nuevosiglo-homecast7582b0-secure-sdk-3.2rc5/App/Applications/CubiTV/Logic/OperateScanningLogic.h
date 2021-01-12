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

#ifndef QB_OPERATE_SCANNING_LOGIC_H_
#define QB_OPERATE_SCANNING_LOGIC_H_

/**
 * @file OperateScanningLogic.h
 * @brief Operate scanning logic API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>

#include <stdbool.h>
#include <main_decl.h>

/**
 * @defgroup QBOperateScanningLogic Operate scanning logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Logic for operate scanning - service responsible for scanning DVB channels.
 * @class QBOperateScanningLogic
 * @extends SvObject
 **/
typedef struct QBOperateScanningLogic_ *QBOperateScanningLogic;

/**
 * Operate scanning callback which will be called after background scanning is finished for any reason.
 *
 * @param[in] owner   handle to callback owner
 * @param[in] success @c true if background scanning finished, otherwise @c false
 **/
typedef void
(*QBOperateScanningBackgroundScanningFinishedCallback)(SvObject owner, bool success);

/**
 * Create operate scanning logic.
 *
 * @param[in] appGlobals handle to application globals
 * @return               created QBOperateScanningLogic object or @c NULL in case of error
 **/
extern QBOperateScanningLogic
QBOperateScanningLogicCreate(AppGlobals appGlobals) __attribute__((weak));

/**
 * Starts operate scanning when needed.
 *
 * @param[in] self                handle to operate scanning logic
 * @param[in] tunerNum            number of DVB tuner to use (find proper tuner when @c -1 is passed)
 * @param[in] tableVersionChanged tells if any DVB table's version changed
 * @param[in] callback            handle to function to call when scanning is finished (optional)
 * @param[in] callbackOwner       handle to callback owner object (required when callback function is defined)
 * @return                        @c true if scanning is needed, otherwise @c false
 *
 **/
extern bool
QBOperateScanningLogicScanningNeeded(QBOperateScanningLogic self,
                                     int tunerNum,
                                     bool tableVersionChanged,
                                     QBOperateScanningBackgroundScanningFinishedCallback callback,
                                     SvObject callbackOwner) __attribute__((weak));

/**
 * @}
 **/

#endif
